#include "dime/common/network.h"
#include "dime/common/error.h"

int connect_host(const char *hostname, unsigned short port, int force_family) {
    PUBLIC_FUNC_IMPL(connect_host, hostname, port, force_family);
}
