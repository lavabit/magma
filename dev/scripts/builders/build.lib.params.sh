#!/bin/bash

GD="libgd-2.2.5"
LZO="lzo-2.10"
PNG="libpng-1.6.37"
CURL="curl-7.70.0"
SPF2="libspf2-1.2.10"
XML2="libxml2-2.9.10"
DKIM="opendkim-2.10.3"
ZLIB="zlib-1.2.11"
JPEG="jpeg-9d"
PCRE="pcre2-10.31"
BZIP2="bzip2-1.0.8"
DSPAM="dspam-3.10.2"
GEOIP="GeoIP-1.4.8"
CLAMAV="clamav-0.102.3"
MARIADB="mariadb-connector-c-2.3.7"
CHECKER="check-0.11.0"
OPENSSL="openssl-1.0.2u"
GOOGTAP="gtest-tap-listener-0.5"
GOOGTEST="googletest-release-1.7.0"
JANSSON="jansson-2.2.1"
UTF8PROC="utf8proc-2.5.0"
FREETYPE="freetype-2.10.2"
MEMCACHED="libmemcached-1.0.18"
TOKYOCABINET="tokyocabinet-1.4.48"

XMLTS="xmlts20130923"
XML2TEST="libxml2-tests-2.9.10"
UTF8PROCTEST="utf8proc-data-2.5.0"

# Quickly add temporary compiler/linker flags to all builds, using these variables,
# or to a single build using an environment variable.
M_CFLAGS="$M_CFLAGS"
M_FFLAGS="$M_FFLAGS"
M_LDFLAGS="$M_LDFLAGS"
M_CPPFLAGS="$M_CPPFLAGS"
M_CXXFLAGS="$M_CXXFLAGS"

# The static archive. This file will only be created if specifically requested.
M_AR="$M_PROJECT_ROOT/magmad.a"

# The shared object output file. If the above flags are modified, it's a good idea to modified this file name.
M_SO="$M_PROJECT_ROOT/magmad.so"

# Calculate the relative path to various folders.
M_LOGS="$M_ROOT/logs"
M_ARCHIVES="$M_ROOT/archives"
M_PATCHES="$M_ROOT/patches"
M_SOURCES="$M_ROOT/sources"
M_OBJECTS="$M_ROOT/objects"
M_CHECK="$M_ROOT/check"
M_LOCAL="$M_ROOT/local"

M_BNPATH="$M_LOCAL/bin/"
M_LDPATH="$M_LOCAL/lib/"
M_PKGPATH="$M_LDPATH/pkgconfig/"

# Where the symbols.h file can be found.
M_SYM_FILE="$M_PROJECT_ROOT/src/providers/symbols.h"

# The following symbols are not in defined in any of the public header files.
M_SYM_SKIP="tcndbgetboth|my_once_free|lt_dlexit"

# The paths searched for all of the included header files.
M_SYM_INCLUDES="-I$M_LOCAL/include -I$M_LOCAL/include/curl -I$M_LOCAL/include/dspam -I$M_LOCAL/include/freetype2 \
-I$M_LOCAL/include/freetype2/freetype -I$M_LOCAL/include/libmemcached -I$M_LOCAL/include/libxml2 -I$M_LOCAL/include/spf2 \
-I$M_LOCAL/include/lzo -I$M_LOCAL/include/mariadb -I$M_LOCAL/include/opendkim -I$M_LOCAL/include/openssl"

