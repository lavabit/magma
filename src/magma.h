
/**
 * @file /magma/magma.h
 *
 * @brief The global include file. This header includes both system headers and Magma module headers.
 */

#ifndef MAGMA_H
#define MAGMA_H

#define __USE_GNU

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/prctl.h>
#include <sys/epoll.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>
#include <regex.h>
#include <ftw.h>
#include <search.h>
#include <semaphore.h>
#include <sys/mman.h>

// GNU C Library
#include <gnu/libc-version.h>

#include "providers/symbols.h"

#include "core/core.h"
#include "providers/providers.h"
#include "engine/engine.h"
#include "network/network.h"
#include "objects/objects.h"
#include "servers/servers.h"
#include "web/web.h"

#include "queries.h"

extern magma_t magma;
/*
extern __thread char threadBuffer[1024];
#define bufptr (char *)&(threadBuffer)
#define buflen sizeof(threadBuffer)
*/
#endif
