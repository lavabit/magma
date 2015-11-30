
/**
 * @file /mason/mason.h
 *
 * @brief  The data file mason.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/07/27 01:43:24 $
 * $Revision: 4103e46b928680d472921cec918c5e7f8e0698d1 $
 *
 */

#ifndef MASON_H
#define MASON_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
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
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>
#include <regex.h>
#include <search.h>
#include <semaphore.h>

#include <tcadb.h>
#include <tcbdb.h>
#include <tcfdb.h>
#include <tchdb.h>
#include <tctdb.h>
#include <tcutil.h>

#include <lzoconf.h>
#include <lzo1x.h>

#include <zlib.h>

#include <bzlib.h>

uint32_t hash_adler32(char *buffer, size_t length);

size_t lzo_compress(void **block, size_t length);
size_t gzip_compress(void **block, size_t length);
size_t bzip_compress(void **block, size_t length);

size_t lzo_decompress(void **block, size_t length, size_t uncompressed);
size_t gzip_decompress(void **block, size_t length, size_t uncompressed);
size_t bzip_decompress(void **block, size_t length, size_t uncompressed);

#endif

