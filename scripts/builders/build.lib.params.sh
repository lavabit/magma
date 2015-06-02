#!/bin/bash

GD="gd-2.0.35"
LZO="lzo-2.09"
PNG="libpng-1.6.16"
CURL="curl-7.23.1"
SPF2="libspf2-1.2.10"
XML2="libxml2-2.9.1"
DKIM="opendkim-2.9.0"
ZLIB="zlib-1.2.8"
JPEG="jpeg-9a"
BZIP2="bzip2-1.0.6"
DSPAM="dspam-3.10.1"
MYSQL="mysql-5.1.73"
GEOIP="GeoIP-1.4.8"
CLAMAV="clamav-0.98.4"
OPENSSL="openssl-1.0.1l"
JANSSON="jansson-2.2.1"
FREETYPE="freetype-2.5.3"
MEMCACHED="libmemcached-1.0.18"
TOKYOCABINET="tokyocabinet-1.4.48"

XMLTS="xmlts20130923"
XML2TEST="libxml2-tests-2.9.0"

M_SO="$M_ROOT/magmad.so"
M_LOGS="$M_ROOT/logs"
M_ARCHIVES="$M_PROJECT_ROOT/lib/archives"
M_PATCHES="$M_PROJECT_ROOT/lib/patches"
M_SOURCES="$M_ROOT/sources"
M_OBJECTS="$M_ROOT/objects"
M_CHECK="$M_PROJECT_ROOT/lib/check"

# Where the symbols.h file can be found
M_SYM_FILE="$M_PROJECT_ROOT/src/providers/symbols.h"

# The following symbols are not in defined in any of the public header files
M_SYM_SKIP="tcndbgetboth|my_once_free|lt_dlexit"

# The paths searched for all of the included header files
M_SYM_DIRS="-I$M_SOURCES/clamav/libclamav -I$M_SOURCES/mysql/include -I$M_SOURCES/openssl/include -I$M_SOURCES/tokyocabinet \
-I$M_SOURCES/spf2/src/include -I$M_SOURCES/xml2/include/libxml -I$M_SOURCES/xml2/include -I$M_SOURCES/lzo/include/lzo -I$M_SOURCES/lzo/include \
-I$M_SOURCES/bzip2 -I$M_SOURCES/zlib -I$M_SOURCES/curl/include/curl -I$M_SOURCES/curl/include -I$M_SOURCES/memcached -I$M_SOURCES/geoip/libGeoIP \
-I$M_SOURCES/dkim/libopendkim -I$M_SOURCES/dspam/src/ -I$M_SOURCES/jansson/src/ -I$M_SOURCES/gd -I$M_SOURCES/png -I$M_SOURCES/jpeg \
-I$M_SOURCES/freetype/include/freetype -I$M_SOURCES/freetype/include"

M_LDPATH="$M_SOURCES/curl/lib/.libs/:$M_SOURCES/memcached/libmemcached/.libs/:$M_SOURCES/spf2/src/libspf2/.libs/:\
$M_SOURCES/tokyocabinet/:$M_SOURCES/lzo/src/.libs/:$M_SOURCES/openssl/engines/:$M_SOURCES/openssl/:$M_SOURCES/clamav/libclamav/.libs/:\
$M_SOURCES/mysql/libmysql/.libs/:$M_SOURCES/xml2/.libs/:$M_SOURCES/xml2/python/.libs/:$M_SOURCES/geoip/libGeoIP/.libs/:\
$M_SOURCES/dkim/libopendkim/.libs/:$M_SOURCES/dspam/src/.libs/:$M_SOURCES/jansson/src/.libs/:$M_SOURCES/gd/.libs/:$M_SOURCES/png/.libs/:\
$M_SOURCES/jpeg/.libs/:$M_SOURCES/freetype/objs/.libs/"
