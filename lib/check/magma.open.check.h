

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
#include <ftw.h>
#include <search.h>
#include <semaphore.h>
#include <sys/mman.h>

// GNU Lib C
#include <gnu/libc-version.h>

// SPF
#include <spf.h>
#include <spf_dns_zone.h>

// ClamAV
#include <clamav.h>
//#include <ltdl.h> // ltdlexit

// MySQL
#include <mysql.h>

// OpenSSL
#include <openssl/conf.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/engine.h>
#include <openssl/ec.h>
#include <openssl/err.h>

// LZO
#include <lzoconf.h>
#include <lzodefs.h>
#include <lzoutil.h>
#include <lzo1x.h>

// XML2
#include <xmlmemory.h>
#include <tree.h>
#include <valid.h>
#include <xpath.h>
#include <xpathInternals.h>
#include <parserInternals.h>
#include <xmlerror.h>

// ZLIB
#include <zlib.h>

// BZIP
#include <bzlib.h>

// TOKYO
#include <tcutil.h>
#include <tcadb.h>
#include <tchdb.h>
#include <tcbdb.h>

// cURL
#include <curl.h>

// Memcached
#include <libmemcached/memcached.h>

// Geo IP
#include <GeoIP.h>
#include <GeoIPCity.h>
#include <GeoIP_internal.h>

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
