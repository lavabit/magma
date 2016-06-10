
/**
 * @file /magma/magma.h
 *
 * @brief The global include file. This header includes both system headers and Magma module headers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_H
#define MAGMA_H

#define __USE_GNU

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
#include <sys/utsname.h>
#include <sys/epoll.h>
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

// SPF
#include <spf.h>
#include <spf_dns_zone.h>

// ClamAV
#include <clamav.h>

// MySQL
#include <mysql.h>

// OpenSSL
#include <openssl/conf.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/engine.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/ec.h>
#include <openssl/dh.h>
#include <openssl/err.h>

// LZO
#include <lzo/lzoconf.h>
#include <lzo/lzodefs.h>
#include <lzo/lzoutil.h>
#include <lzo/lzo1x.h>

// XML2
#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/valid.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlerror.h>

// ZLIB
#include <zlib.h>

// BZIP
#include <bzlib.h>

// TOKYO
#include <tcutil.h>
#include <tcadb.h>
#include <tchdb.h>
#include <tcbdb.h>

// Memcached
#include <libmemcached/memcached.h>

// DKIM
#define lint
#include <dkim.h>
#undef lint

// DSPAM
#define CONFIG_DEFAULT ""
#define LOGDIR "~/"
#include <libdspam.h>
#include <mysql_drv.h>

// Jansson
#include <jansson.h>

// GD
#include <gd.h>

// PNG
#include <png.h>

// JPEG
#include <jpeglib.h>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

// UTF8
#include <utf8proc.h>

#include "core/core.h"
#include "providers/providers.h"
#include "engine/engine.h"
#include "network/network.h"
#include "objects/objects.h"
#include "servers/servers.h"
#include "web/web.h"

#include "queries.h"

extern magma_t magma;

extern __thread char threadBuffer[1024];
#define bufptr (char *)&(threadBuffer)
#define buflen sizeof(threadBuffer)

#endif
