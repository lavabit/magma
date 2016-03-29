
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
#include <freetype.h>

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

/*
#define HAVE_ABI_CXA_DEMANGLE 1
#define HAVE_ACCESS 1
#define HAVE_AIO_H 1
#define HAVE_ALARM 1
#define HAVE_ALLOCA 1
#define HAVE_ALLOCA_H 1
#define HAVE_ANSIDECL_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_ARPA_NAMESER_H 1
#define HAVE_ASM_TERMBITS_H 1
#define HAVE_ASSERT_H 1
#define HAVE_ATEXIT 1
#define HAVE_ATOI 1
#define HAVE_ATOL 1
#define HAVE_BACKTRACE 1
#define HAVE_BACKTRACE_SYMBOLS 1
#define HAVE_BACKTRACE_SYMBOLS_FD 1
#define HAVE_BOOL 1
#define HAVE_BSEARCH 1
#define HAVE_BSS_START 1
#define HAVE_BZERO 1
#define HAVE_CEILF 1
#define HAVE_CHARSET_armscii8 1
#define HAVE_CHARSET_ascii 1
#define HAVE_CHARSET_big5 1
#define HAVE_CHARSET_cp1250 1
#define HAVE_CHARSET_cp1251 1
#define HAVE_CHARSET_cp1256 1
#define HAVE_CHARSET_cp1257 1
#define HAVE_CHARSET_cp850 1
#define HAVE_CHARSET_cp852 1
#define HAVE_CHARSET_cp866 1
#define HAVE_CHARSET_cp932 1
#define HAVE_CHARSET_dec8 1
#define HAVE_CHARSET_eucjpms 1
#define HAVE_CHARSET_euckr 1
#define HAVE_CHARSET_gb2312 1
#define HAVE_CHARSET_gbk 1
#define HAVE_CHARSET_geostd8 1
#define HAVE_CHARSET_greek 1
#define HAVE_CHARSET_hebrew 1
#define HAVE_CHARSET_hp8 1
#define HAVE_CHARSET_keybcs2 1
#define HAVE_CHARSET_koi8r 1
#define HAVE_CHARSET_koi8u 1
#define HAVE_CHARSET_latin1 1
#define HAVE_CHARSET_latin2 1
#define HAVE_CHARSET_latin5 1
#define HAVE_CHARSET_latin7 1
#define HAVE_CHARSET_macce 1
#define HAVE_CHARSET_macroman 1
#define HAVE_CHARSET_sjis 1
#define HAVE_CHARSET_swe7 1
#define HAVE_CHARSET_tis620 1
#define HAVE_CHARSET_ucs2 1
#define HAVE_CHARSET_ujis 1
#define HAVE_CHARSET_utf8 1
#define HAVE_CHMOD 1
#define HAVE_CHOWN 1
#define HAVE_C_INLINE
#define HAVE_CLOCK_GETTIME 1
#define HAVE_COMPRESS 1
#define HAVE_CRYPT 1
#define HAVE_CRYPT_H 1
#define HAVE_CTIME 1
#define HAVE_CTYPE_H 1
#define HAVE_CURSES_H 1
#define HAVE_CUSERID 1
#define HAVE_CXXABI_H 1
#define HAVE_DECL_BZERO 1
#define HAVE_DECL_FDATASYNC 1
#define HAVE_DECL_MADVISE 1
#define HAVE_DECL_NS_T_INVALID 1
#define HAVE_DECL_NS_T_SPF 0
#define HAVE_DECL_RES_NDESTROY 0
#define HAVE_DECL_RES_NINIT 1
#define HAVE_DECL_SHM_HUGETLB 1
#define HAVE_DECL_TGOTO 1
#define HAVE_DIFFTIME 1
#define HAVE_DIRENT_H 1
#define HAVE_DLERROR 1
#define HAVE_DLFCN_H 1
#define HAVE_DLOPEN
#define HAVE_ERRNO_H 1
#define HAVE_EVENT_BASE_FREE 1
#define HAVE_EVENT_BASE_GET_METHOD 1
#define HAVE_EVENT_BASE_NEW 1
#define HAVE_EXECINFO_H 1
#define HAVE_EXPLICIT_TEMPLATE_INSTANTIATION 1
#define HAVE_FCHMOD 1
#define HAVE_FCNTL 1
#define HAVE_FCNTL_H 1
#define HAVE_FDATASYNC 1
#define HAVE_FEDISABLEEXCEPT 1
#define HAVE_FENV_H 1
#define HAVE_FESETROUND 1
#define HAVE_FINITE 1
#define HAVE_FLOAT_H 1
#define HAVE_FLOCKFILE 1
#define HAVE_FLOORF 1
#define HAVE_FNMATCH_H 1
#define HAVE_FORK 1
#define HAVE_FPRINTF 1
#define HAVE_FPU_CONTROL_H 1
#define HAVE_FSTAT 1
#define HAVE_FSYNC 1
#define HAVE_FT2BUILD_H 1
#define HAVE_FTIME 1
#define HAVE_FTRUNCATE 1
#define HAVE_GCC_ATOMIC_BUILTINS 1
#define HAVE_GETADDRINFO
#define HAVE_GETCWD 1
#define HAVE_GETEGID 1
#define HAVE_GETENV 1
#define HAVE_GETEUID 1
#define HAVE_GETGID 1
#define HAVE_GETHOSTBYADDR_R 1
#define HAVE_GETHOSTBYNAME 1
#define HAVE_GETHOSTBYNAME_R 1
#define HAVE_GETHOSTBYNAME_R_GLIBC2_STYLE 1
#define HAVE_GETHOSTNAME 1
#define HAVE_GETLINE 1
#define HAVE_GETOPT_H 1
#define HAVE_GETOPT_LONG_ONLY 1
#define HAVE_GETPAGESIZE 1
#define HAVE_GETPASS 1
#define HAVE_GETPWNAM 1
#define HAVE_GETPWUID 1
#define HAVE_GETRLIMIT 1
#define HAVE_GETRUSAGE 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_GETUID 1
#define HAVE_GETWD 1
#define HAVE_GMTIME 1
#define HAVE_GMTIME_R 1
#define HAVE_GRP_H 1
#define HAVE_IB_ATOMIC_PTHREAD_T_GCC 1
#define HAVE_IB_GCC_ATOMIC_BUILTINS 1
#define HAVE_IB_PAUSE_INSTRUCTION 1
#define HAVE_ICONV 1
#define HAVE_ICONV_H 1
#define HAVE_ICONV_T_DEF 1
#define HAVE_IN_ADDR_T 1
#define HAVE_INDEX 1
#define HAVE_INET_NTOA 1
#define HAVE_INITGROUPS 1
#define HAVE_INTTYPES_H 1
#define HAVE_ISATTY 1
#define HAVE_ISINF 1
#define HAVE_ISNAN 1
#define HAVE_ISWCTYPE 1
#define HAVE_ISWLOWER 1
#define HAVE_ISWUPPER 1
#define HAVE_LANGINFO_CODESET
#define HAVE_LANGINFO_H 1
#define HAVE_LARGE_PAGES 1
#define HAVE_LIBDL 1
#define HAVE_LIBEVENT 1
#define HAVE_LIBFREETYPE 1
#define HAVE_LIBINTL_H 1
#define HAVE_LIBJPEG 1
#define HAVE_LIBM 1
#define HAVE_LIBMEMCACHED 1
#define HAVE_LIBMEMCACHEDUTIL 1
#define HAVE_LIBNSL 1
#define HAVE_LIBPNG 1
#define HAVE_LIBPTHREAD 1
#define HAVE_LIBZ 1
#define HAVE_LIMITS_H 1
#define HAVE_LOCALE_H 1
#define HAVE_LOCALTIME 1
#define HAVE_LOCALTIME_R 1
#define HAVE_LONGJMP 1
#define HAVE_LONG_LONG_INT 1
#define HAVE_LRAND48 1
#define HAVE_LSTAT 1
#define HAVE_MADVISE 1
#define HAVE_MALLINFO 1
#define HAVE_MALLOC 1
#define HAVE_MALLOC_H 1
#define HAVE_MATH_H 1
#define HAVE_MBRLEN
#define HAVE_MBRTOWC
#define HAVE_MBSRTOWCS
#define HAVE_MBSTATE_T
#define HAVE_MEMCMP 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMORY_H 1
#define HAVE_MEMSET 1
#define HAVE_MKDIR 1
#define HAVE_MKSTEMP 1
#define HAVE_MKTIME 1
#define HAVE_MLOCKALL 1
#define HAVE_MMAP 1
#define HAVE_MMAP64 1
#define HAVE_MODE_T
#define HAVE_MPROTECT 1
#define HAVE_MSG_DONTWAIT 1
#define HAVE_MSG_MORE 1
#define HAVE_MSG_NOSIGNAL 1
#define HAVE_MUNMAP 1
#define HAVE_MURMUR_HASH 1
#define HAVE_NETDB_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_NPTL 1
#define HAVE_OFF_T 1
#define HAVE_PATHS_H 1
#define HAVE_PERROR 1
#define HAVE_PNG_H 1
#define HAVE_POLL 1
#define HAVE_POLL_H 1
#define HAVE_POSIX_FALLOCATE 1
#define HAVE_POSIX_SIGNALS 1
#define HAVE_PREAD 1
#define HAVE_PRINTF 1
#define HAVE_PTHREAD 1
#define HAVE_PTHREAD_ATTR_GETSTACKSIZE 1
#define HAVE_PTHREAD_ATTR_SETSCHEDPARAM 1
#define HAVE_PTHREAD_ATTR_SETSCOPE 1
#define HAVE_PTHREAD_ATTR_SETSTACKSIZE 1
#define HAVE_PTHREAD_H 1
#define HAVE_PTHREAD_KEY_DELETE 1
#define HAVE_PTHREAD_RWLOCK_RDLOCK 1
#define HAVE_PTHREAD_SETSCHEDPARAM 1
#define HAVE_PTHREAD_SETSCHEDPRIO 1
#define HAVE_PTHREAD_SIGMASK 1
#define HAVE_PTHREAD_YIELD_ZERO_ARG 1
#define HAVE_PUTENV 1
#define HAVE_PWD_H 1
#define HAVE_QSORT 1
#define HAVE_QUERY_CACHE 1
#define HAVE_RAISE 1
#define HAVE_RCVTIMEO 1
#define HAVE_READDIR_R 1
#define HAVE_READLINK 1
#define HAVE_REALLOC 1
#define HAVE_REALPATH 1
#define HAVE_RE_COMP 1
#define HAVE_REGCOMP 1
#define HAVE_RENAME 1
#define HAVE_RESOLV_H 1
#define HAVE_RESTARTABLE_SYSCALLS 1
#define HAVE_RINT 1
#define HAVE_RMDIR 1
#define HAVE_RTREE_KEYS 1
#define HAVE_SCHED_H 1
#define HAVE_SCHED_YIELD 1
#define HAVE_SELECT 1
#define HAVE_SEMAPHORE_H 1
#define HAVE_SETENV 1
#define HAVE_SETJMP 1
#define HAVE_SETJMP_H 1
#define HAVE_SETLOCALE 1
#define HAVE_SHMAT 1
#define HAVE_SHMCTL 1
#define HAVE_SHMDT 1
#define HAVE_SHMGET 1
#define HAVE_SIGACTION 1
#define HAVE_SIGADDSET 1
#define HAVE_SIGEMPTYSET 1
#define HAVE_SIGHOLD 1
#define HAVE_SIGNAL 1
#define HAVE_SIGNAL_H 1
#define HAVE_SIGSET 1
#define HAVE_SIGSET_T 1
#define HAVE_SIGWAIT 1
#define HAVE_SIZE_T 1
#define HAVE_SLEEP 1
#define HAVE_SNDTIMEO 1
#define HAVE_SNPRINTF 1
#define HAVE_SOCKET 1
#define HAVE_SPATIAL 1
#define HAVE_SPRINTF 1
#define HAVE_SSCANF 1
#define HAVE_STAT 1
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STPCPY 1
#define HAVE_STRCASECMP 1
#define HAVE_STRCHR 1
#define HAVE_STRCOLL 1
#define HAVE_STRCSPN 1
#define HAVE_ST_RDEV 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRERROR_S 1
#define HAVE_STRFTIME 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRNCASECMP 1
#define HAVE_STRNDUP 1
#define HAVE_STRNLEN 1
#define HAVE_STRPBRK 1
#define HAVE_STRRCHR 1
#define HAVE_STRSIGNAL 1
#define HAVE_STRSPN 1
#define HAVE_STRSTR 1
#define HAVE_STRTOK_R 1
#define HAVE_STRTOL 1
#define HAVE_STRTOLL 1
#define HAVE_STRTOUL 1
#define HAVE_STRTOULL 1
#define HAVE_STRUCT_IN6_ADDR 1
#define HAVE_STRUCT_STAT_ST_RDEV 1
#define HAVE_STRUCT_TIMESPEC
#define HAVE_SYS_CDEFS_H 1
#define HAVE_SYS_DIR_H 1
#define HAVE_SYS_FILE_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_IPC_H 1
#define HAVE_SYSLOG_H 1
#define HAVE_SYS_MMAN_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_PRCTL_H 1
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_SHM_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIMEB_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UN_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_TCGETATTR 1
#define HAVE_TEMPNAM 1
#define HAVE_TERMCAP_H 1
#define HAVE_TERM_H 1
#define HAVE_TERMIO_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_TIME 1
#define HAVE_TIME_H 1
#define HAVE_TOWLOWER 1
#define HAVE_TOWUPPER 1
#define HAVE_TZNAME 1
#define HAVE_U_CHAR 1
#define HAVE_UCONTEXT_H 1
#define HAVE_UINT 1
#define HAVE_U_INT16_T 1
#define HAVE_UINT16_T 1
#define HAVE_U_INT32_T 1
#define HAVE_UINT32_T 1
#define HAVE_U_INT8_T 1
#define HAVE_UINT8_T 1
#define HAVE_ULONG 1
#define HAVE_UMASK 1
#define HAVE_UNISTD_H 1
#define HAVE_UTIME 1
#define HAVE_UTIME_H 1
#define HAVE_UTIME_NULL 1
#define HAVE_VA_COPY 1
#define HAVE_VFORK 1
#define HAVE_VFPRINTF 1
#define HAVE_VIO_READ_BUFF 1
#define HAVE_VISIBILITY 1
#define HAVE_VPRINTF 1
#define HAVE_VSNPRINTF 1
#define HAVE_VSPRINTF 1
#define HAVE_WCHAR_H 1
#define HAVE_WCHAR_T 1
#define HAVE_WCRTOMB
#define HAVE_WCSCOLL
#define HAVE_WCSDUP
#define HAVE_WCTYPE
#define HAVE_WCTYPE_H 1
#define HAVE_WCTYPE_T 1
#define HAVE_WCWIDTH
#define HAVE_WEAK_SYMBOL 1
#define HAVE_WORKING_FORK 1
#define HAVE_WORKING_VFORK 1
#define HAVE_ZLIB_H 1

 */

#endif
