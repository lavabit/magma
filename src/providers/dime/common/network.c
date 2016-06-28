#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include <errno.h>

#include "dime/common/network.h"
#include "dime/common/misc.h"

//the timeout value for connection attempts, in seconds
#define CONNECT_TIMEOUT 5

/**
 * @brief
 *  Connect to a host/tcp port in an address-independent manner, and return a
 *  file descriptor.
 * @param hostname
 *  the hostname of the server to connect to.
 * @param port
 *  the TCP port number to which the connection should be made.
 * @param force_family
 *  an optional address family to force the connection to (AF_INET or
 *  AF_INET6), or 0 to ignore).
 * @return
 *  -1 on general failure or the file descriptor of the socket connection on
 *  success.
 */
int
_connect_host(
    char const *hostname,
    unsigned short port,
    int force_family)
{
    struct addrinfo hints, *address;
    char pstr[16];
    int result, fd = -1;

    if (!hostname || !port) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    memset(pstr, 0, sizeof(pstr));
    snprintf(pstr, sizeof(pstr), "%u", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = force_family ? force_family : AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((result = getaddrinfo(hostname, pstr, &hints, &address))) {
        RET_ERROR_INT_FMT(
            ERR_UNSPEC,
            "failed to resolve host address: %s",
            gai_strerror(result));
    }

    for (struct addrinfo *aptr = address; aptr; aptr = aptr->ai_next) {

        // Shouldn't happen, but paranoia never hurt anybody.
        if (force_family && (aptr->ai_family != force_family)) {
            continue;
        }

        if ((fd = socket(
                aptr->ai_family,
                aptr->ai_socktype,
                aptr->ai_protocol))
            < 0)
        {
            continue;
        } else if (
            _connect_timeout(
                fd,
                aptr->ai_addr,
                aptr->ai_addrlen)
            > 0)
        {
            _dbgprint(
                3,
                "Established TCP connection (%s) to %s:%s.\n",
                (aptr->ai_family == AF_INET ? "IPV4" : "IPV6"),
                hostname,
                pstr);
            break;
        }

        close(fd);
        fd = -1;
    }

    freeaddrinfo(address);

    if (fd < 0) {
        RET_ERROR_INT(ERR_UNSPEC, NULL);
    }

    return fd;
}

/**
 * @brief
 *  Attempt a TCP connection with a time-out mechanism.
 * @param fd
 *  the open socket descriptor across which the connection will be attempted.
 * @param addr
 *  a pointer to the prepared sockaddr structure that is the target of the
 *  connection.
 * @param addrlen
 *  the size of the supplied sockaddr structure.
 * @return
 *  -1 on general error, 0 if the connection failed or timed out, and 1 on
 *  success.
 */
int
_connect_timeout(
    int fd,
    struct sockaddr const *addr,
    socklen_t addrlen)
{
    fd_set fds;
    struct timeval tv;
    socklen_t slen;
    int serr, oflags;

    if (!addr || !addrlen) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if ((oflags = fcntl(fd, F_GETFL, NULL)) < 0) {
        PUSH_ERROR_SYSCALL("fcntl");
        RET_ERROR_INT(ERR_UNSPEC, "unable to get socket blocking mode");
    }

    if (fcntl(fd, F_SETFL, oflags | O_NONBLOCK) < 0) {
        PUSH_ERROR_SYSCALL("fcntl");
        RET_ERROR_INT(ERR_UNSPEC, "unable to set non-blocking mode on socket");
    }

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = CONNECT_TIMEOUT;

    if (((connect(fd, addr, addrlen)) < 0) && (errno != EINPROGRESS)) {
        PUSH_ERROR_SYSCALL("connect");
        RET_ERROR_INT(ERR_UNSPEC, "unable to establish connection to host");
    }

    switch (select(fd + 1, NULL, &fds, NULL, &tv)) {

    case -1:
        PUSH_ERROR_SYSCALL("select");
        RET_ERROR_INT(ERR_UNSPEC, "select operation on socket failed");
        break;
    case 0:
        return 0;
        break;
    case 1:
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "select operation returned an unexpected value");
        break;

    }

    slen = sizeof(serr);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &serr, &slen) < 0) {
        PUSH_ERROR_SYSCALL("getsockopt");
        RET_ERROR_INT(ERR_UNSPEC, "attempt to get socket error flag failed");
    }

    fcntl(fd, F_SETFL, oflags);

    if (!serr) {
        return 1;
    }

    return 0;
}
