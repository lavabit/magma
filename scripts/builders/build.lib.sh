#!/bin/bash

# Install Sphinx (python-sphinx package) to build Jansson HTML docs
# Install LTDL and create the clamav user/group so ClamAV will build correctly
# DSPAM will require the MySQL development (lib/headers) for a parallel build to complete
# Install libevent/memcached, and then add /usr/local/lib to the linker config path so memcached can be executed

# Enable the SELinux boolean allow_exestack or the shared object loader will fail
# Create the test@localhost mysql user and grant the account full access to the test schema
# The symbols.h file may need to be imported from the magma project for the object load test
# Note that some platforms may require setting the CFLAGS/CPPFLAGS environment variables to -Wold-style-cast when compiling memcached

# To generate a SLOC report for each project:
# cd $M_SOURCES; find  -maxdepth 1 -type d -printf '\n\n%P\n' -exec sloc --quiet --progress-rate=0 {} \; | grep -v "http://cloc.sourceforge.net"

M_ROOT=`pwd`
M_BUILD=`readlink -f $0`

# Set parent directory as project root by default (used to find scripts,
# bundled tarballs, patches, etc.)
if [ -z "$M_PROJECT_ROOT" ]; then M_PROJECT_ROOT=`readlink -f ..`; fi

. "$M_PROJECT_ROOT/scripts/builders/build.lib.params.sh"

mkdir -p "$M_LOGS"
mkdir -p "$M_SOURCES"
mkdir -p "$M_OBJECTS"

error() {
	if [ $? -ne 0 ]; then
		tput sgr0; tput setaf 1
		#printf "\n\n$COMMAND failed...\n\n";
		date +"%n%n$COMMAND failed at %r on %x%n%n"
		tput sgr0
		exit 1
	fi
}

# Confirm whether we really want to run this service
extract() {
	if [ -d "$M_SOURCES/$2" ]; then
		rm -rf "$M_SOURCES/$2"; error
	fi
	tar xzvf "$M_ARCHIVES/$1.tar.gz" -C "$M_SOURCES"; error
	mv "$M_SOURCES/$1" "$M_SOURCES/$2"; error
}

gd() {

	if [[ $1 == "gd-extract" ]]; then
		rm -f "$M_LOGS/gd.txt"; error
	elif [[ $1 != "gd-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/gd.txt"
	fi

	case "$1" in
		gd-extract)
			extract $GD "gd" &>> "$M_LOGS/gd.txt"
		;;
		gd-prep)
			cd "$M_SOURCES/gd"; error
			cat "$M_PATCHES/gd/"gd-2.0.33-freetype.patch | patch -s -p1 -b --suffix .freetype --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-2.0.34-multilib.patch | patch -s -p1 -b --suffix .mlib --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-loop.patch | patch -s -p1 -b --suffix .loop --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-2.0.35-overflow.patch | patch -s -p1 -b --suffix .overflow --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-2.0.34-sparc64.patch | patch -s -p1 -b --suffix .sparc64 --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-2.0.35-AALineThick.patch | patch -s -p1 -b --suffix .AALineThick --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-2.0.33-BoxBound.patch | patch -s -p1 -b --suffix .bb --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-2.0.34-fonts.patch | patch -s -p1 -b --suffix .fonts --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-2.0.35-time.patch | patch -s -p1 -b --suffix .time --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-2.0.35-security3.patch | patch -s -p1 -b --suffix .sec3 --fuzz=0; error
			cat "$M_PATCHES/gd/"gd-version.patch | patch -s -p1 -b --fuzz=0; error; error
			cat "$M_PATCHES/gd/"gd-sigcmp.patch | patch -s -p1 -b --fuzz=0; error; error
		;;
		gd-configure)
			cd "$M_SOURCES/gd"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure --without-xpm --without-fontconfig --without-x --with-png="$M_SOURCES/png" --with-jpeg="$M_SOURCES/jpeg" --with-freetype="$M_SOURCES/freetype" &>> "$M_LOGS/gd.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		gd-build)
			cd "$M_SOURCES/gd"; error
			make &>> "$M_LOGS/gd.txt"; error
		;;
		gd-check)
			cd "$M_SOURCES/gd"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/gd.txt"; error
		;;
		gd-check-full)
			cd "$M_SOURCES/gd"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/gd.txt"; error
		;;
		gd-clean)
			cd "$M_SOURCES/gd"; error
			make clean &>> "$M_LOGS/gd.txt"; error
		;;
		gd-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/gd.txt"; error
		;;
		gd-log)
			cat "$M_LOGS/gd.txt"; error
		;;
		gd)
			gd "gd-extract"
			gd "gd-prep"
			gd "gd-configure"
			gd "gd-build"
			gd "gd-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/gd.txt"

	return $?

}

png() {

	if [[ $1 == "png-extract" ]]; then
		rm -f "$M_LOGS/png.txt"; error
	elif [[ $1 != "png-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/png.txt"
	fi

	case "$1" in
		png-extract)
		extract $PNG "png" &>> "$M_LOGS/png.txt"
		;;
		png-prep)
			cd "$M_SOURCES/png"; error
			if [[ $PNG == "libpng-1.6.9" ]]; then
				cat "$M_PATCHES/png/"makefile-1.6.9.patch | patch -p1 --verbose &>> "$M_LOGS/png.txt"; error
			else
				cat "$M_PATCHES/png/"makefile-1.6.16.patch | patch -p1 --verbose &>> "$M_LOGS/png.txt"; error
			fi
		;;
		png-configure)
			cd "$M_SOURCES/png"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure &>> "$M_LOGS/png.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		png-build)
			cd "$M_SOURCES/png"; error
			make &>> "$M_LOGS/png.txt"; error
		;;
		png-check)
			cd "$M_SOURCES/png"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/png.txt"; error
		;;
		png-check-full)
			cd "$M_SOURCES/png"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/png.txt"; error
		;;
		png-clean)
			cd "$M_SOURCES/png"; error
			make clean &>> "$M_LOGS/png.txt"; error
		;;
		png-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/png.txt"
		;;
		png-log)
			cat "$M_LOGS/png.txt"; error
		;;
		png)
			png "png-extract"
			png "png-prep"
			png "png-configure"
			png "png-build"
			png "png-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/png.txt"

	return $?

}

lzo() {

	if [[ $1 == "lzo-extract" ]]; then
		rm -f "$M_LOGS/lzo.txt"; error
	elif [[ $1 != "lzo-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/lzo.txt"
	fi

	case "$1" in
		lzo-extract)
			extract $LZO "lzo" &>> "$M_LOGS/lzo.txt"
		;;
		lzo-prep)
			cd "$M_SOURCES/lzo"; error
		;;
		lzo-configure)
			cd "$M_SOURCES/lzo"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure --enable-shared &>> "$M_LOGS/lzo.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		lzo-build)
			cd "$M_SOURCES/lzo"; error
			make &>> "$M_LOGS/lzo.txt"; error
		;;
		lzo-check)
			cd "$M_SOURCES/lzo"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make test &>> "$M_LOGS/lzo.txt"; error
		;;
		lzo-check-full)
			cd "$M_SOURCES/lzo"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make test &>> "$M_LOGS/lzo.txt"; error
		;;
		lzo-clean)
			cd "$M_SOURCES/lzo"; error
			make clean &>> "$M_LOGS/lzo.txt"; error
		;;
		lzo-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/lzo.txt"; error
		;;
		lzo-log)
			cat "$M_LOGS/lzo.txt"; error
		;;
		lzo)
			lzo "lzo-extract"
			lzo "lzo-prep"
			lzo "lzo-configure"
			lzo "lzo-build"
			lzo "lzo-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/lzo.txt"

	return $?

}

jpeg() {

	if [[ $1 == "jpeg-extract" ]]; then
		rm -f "$M_LOGS/jpeg.txt"; error
	elif [[ $1 != "jpeg-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/jpeg.txt"
	fi

	case "$1" in
		jpeg-extract)
			extract $JPEG "jpeg" &>> "$M_LOGS/jpeg.txt"
		;;
		jpeg-prep)
			cd "$M_SOURCES/jpeg"; error
			cat "$M_PATCHES/jpeg/"jpeg-version.patch | patch -s -p1 -b --fuzz=3; error
		;;
		jpeg-configure)
			cd "$M_SOURCES/jpeg"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure &>> "$M_LOGS/jpeg.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		jpeg-build)
			cd "$M_SOURCES/jpeg"; error
			make &>> "$M_LOGS/jpeg.txt"; error
		;;
		jpeg-check)
			cd "$M_SOURCES/jpeg"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/jpeg.txt"; error
		;;
		jpeg-check-full)
			cd "$M_SOURCES/jpeg"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/jpeg.txt"; error
		;;
		jpeg-clean)
			cd "$M_SOURCES/jpeg"; error
			make clean &>> "$M_LOGS/jpeg.txt"; error
		;;
		jpeg-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/jpeg.txt"; error
		;;
		jpeg-log)
			cat "$M_LOGS/jpeg.txt"; error
		;;
		jpeg)
			jpeg "jpeg-extract"
			jpeg "jpeg-prep"
			jpeg "jpeg-configure"
			jpeg "jpeg-build"
			jpeg "jpeg-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/jpeg.txt"

	return $?

}

spf2() {

	if [[ $1 == "spf2-extract" ]]; then
		rm -f "$M_LOGS/spf2.txt"; error
	elif [[ $1 != "spf2-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/spf2.txt"
	fi

	case "$1" in
		spf2-extract)
			extract $SPF2 "spf2" &>> "$M_LOGS/spf2.txt"
		;;
		spf2-prep)
			cd "$M_SOURCES/spf2"; error
			cat "$M_PATCHES/spf2/"fix_variadic_macro_logging.patch | patch -p1 --verbose &>> "$M_LOGS/spf2.txt"; error
		;;
		spf2-configure)
			cd "$M_SOURCES/spf2"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"
			./configure &>> "$M_LOGS/spf2.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		spf2-build)
			cd "$M_SOURCES/spf2"; error
			make &>> "$M_LOGS/spf2.txt"; error
		;;
		spf2-check)
			cd "$M_SOURCES/spf2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/spf2.txt"; error
		;;
		spf2-check-full)
			cd "$M_SOURCES/spf2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/spf2.txt"; error
		;;
		spf2-clean)
			cd "$M_SOURCES/spf2"; error
			make clean &>> "$M_LOGS/spf2.txt"; error
		;;
		spf2-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/spf2.txt"; error
		;;
		spf2-log)
			cat "$M_LOGS/spf2.txt"; error
		;;
		spf2)
			spf2 "spf2-extract"
			spf2 "spf2-prep"
			spf2 "spf2-configure"
			spf2 "spf2-build"
			spf2 "spf2-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/spf2.txt"

	return $?

}

curl() {

	if [[ $1 == "curl-extract" ]]; then
		rm -f "$M_LOGS/curl.txt"; error
	elif [[ $1 != "curl-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/curl.txt"
	fi

	case "$1" in
		curl-extract)
			extract $CURL "curl" &>> "$M_LOGS/curl.txt"
		;;
		curl-prep)
			cd "$M_SOURCES/curl"; error
			cat "$M_PATCHES/curl/"skip_test172-version7.23.1.patch | patch -p1 --verbose &>> "$M_LOGS/curl.txt"; error
		;;
		curl-configure)
			# Note that if we don't include the debug configure option we can't run a check-full.
			cd "$M_SOURCES/curl"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure --enable-debug --enable-static=yes --without-librtmp --without-krb4 --without-krb5 --without-libssh2 --without-ca-bundle --without-ca-path --without-libidn \
				--disable-dict --disable-file --disable-ftp --disable-ftps --disable-gopher --disable-imap --disable-imaps --disable-pop3 --disable-pop3s \
				--disable-rtsp --disable-smtp --disable-smtps --disable-telnet --disable-tftp --disable-ldap --disable-ssh \
				--build=x86_64-redhat-linux-gnu --target=x86_64-redhat-linux-gnu \
				--with-pic --with-ssl="$M_SOURCES/openssl" --with-zlib="$M_SOURCES/zlib" &>> "$M_LOGS/curl.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		curl-build)
			cd "$M_SOURCES/curl"; error
			make &>> "$M_LOGS/curl.txt"; error
		;;
		curl-check)
			# The target 'check' is an alias for the targets 'test' and 'examples'
			cd "$M_SOURCES/curl"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make --jobs=4 examples &>> "$M_LOGS/curl.txt"; error; error
			make --jobs=4 test &>> "$M_LOGS/curl.txt"; error
		;;
		curl-check-full)

			# The target 'check' is an alias for the targets 'test' and 'examples'
			cd "$M_SOURCES/curl"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make --jobs=4 examples &>> "$M_LOGS/curl.txt"; error; error
			make --jobs=4 test &>> "$M_LOGS/curl.txt"; error
			make --jobs=4 test-full &>> "$M_LOGS/curl.txt"; error

			# To pass the torture test we'll need to recompile the library with all of the protocols enabled.
#			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
#			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
#			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
#
#			make distclean &>> "$M_LOGS/curl.txt";
#			./configure --enable-debug --enable-static=yes --without-librtmp --without-krb4 --without-krb5 --without-libssh2 --without-ca-bundle --without-ca-path --without-libidn \
#				--with-pic --with-ssl="$M_SOURCES/openssl" --with-zlib="$M_SOURCES/zlib" &>> "$M_LOGS/curl.txt"; error
#			make --jobs=4 &>> "$M_LOGS/curl.txt"; error
#			make --jobs=4 test-torture &>> "$M_LOGS/curl.txt"; error
#
#			# Assuming the torture test passed, rebuild the library using the default settings.
#			make distclean &>> "$M_LOGS/curl.txt";
#			curl configure
#			curl build
		;;
		curl-clean)
			cd "$M_SOURCES/curl"; error
			make clean &>> "$M_LOGS/curl.txt"; error
		;;
		curl-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/curl.txt"; error
		;;
		curl-log)
			cat "$M_LOGS/curl.txt"; error
		;;
		curl)
			curl "curl-extract"
			curl "curl-prep"
			curl "curl-configure"
			curl "curl-build"
			curl "curl-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/curl.txt"

	return $?

}

xml2() {

	if [[ $1 == "xml2-extract" ]]; then
		rm -f "$M_LOGS/xml2.txt"; error
	elif [[ $1 != "xml2-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/xml2.txt"
	fi

	case "$1" in
		xml2-extract)
			extract $XML2 "xml2" &>> "$M_LOGS/xml2.txt"
			tar xzvf "$M_ARCHIVES/$XMLTS.tar.gz" -C "$M_SOURCES/xml2" &>> "$M_LOGS/xml2.txt"; error
			tar --strip-components=1 -x -z -v -f "$M_ARCHIVES/$XML2TEST.tar.gz" -C "$M_SOURCES/xml2" &>> "$M_LOGS/xml2.txt"; error
		;;
		xml2-prep)
			cd "$M_SOURCES/xml2"; error
			rm -f test/att11; error
			rm -f result/att11.sax; error
			rm -f result/att11.sax2; error
			rm -f result/att11; error
			rm -f result/noent/att11; error
			rm -f result/att11.rdr; error
			rm -f result/att11.rde; error
		;;
		xml2-configure)
			cd "$M_SOURCES/xml2"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure --without-lzma --without-python --without-http --without-ftp &>> "$M_LOGS/xml2.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		xml2-build)
			cd "$M_SOURCES/xml2"; error
			make --jobs=2 &>> "$M_LOGS/xml2.txt"; error
		;;
		xml2-check)
			cd "$M_SOURCES/xml2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/xml2.txt"; error
		;;
		xml2-check-full)
			cd "$M_SOURCES/xml2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/xml2.txt"; error
			make -f Makefile.tests check &>> "$M_LOGS/xml2.txt"; error
		;;
		xml2-clean)
			cd "$M_SOURCES/xml2"; error
			make clean &>> "$M_LOGS/xml2.txt"; error
			make -f Makefile.tests clean &>> "$M_LOGS/xml2.txt"; error
		;;
		xml2-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/xml2.txt"; error
		;;
		xml2-log)
			cat "$M_LOGS/xml2.txt"; error
		;;
		xml2)
			xml2 "xml2-extract"
			xml2 "xml2-prep"
			xml2 "xml2-configure"
			xml2 "xml2-build"
			xml2 "xml2-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/xml2.txt"

	return $?

}

dkim() {

	if [[ $1 == "dkim-extract" ]]; then
		rm -f "$M_LOGS/dkim.txt"; error
	elif [[ $1 != "dkim-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/dkim.txt"
	fi

	case "$1" in
		dkim-extract)
			extract $DKIM "dkim" &>> "$M_LOGS/dkim.txt"
		;;
		dkim-prep)
			cd "$M_SOURCES/dkim"; error
		;;
		dkim-configure)
			cd "$M_SOURCES/dkim"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure --disable-filter --without-milter --without-sasl --without-gnutls --without-odbx --without-openldap --with-openssl &>> "$M_LOGS/dkim.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		dkim-build)
			cd "$M_SOURCES/dkim"; error
			make &>> "$M_LOGS/dkim.txt"; error
		;;
		dkim-check)
			cd "$M_SOURCES/dkim"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/dkim.txt"; error
		;;
		dkim-check-full)
			cd "$M_SOURCES/dkim"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/dkim.txt"; error
		;;
		dkim-clean)
			cd "$M_SOURCES/dkim"; error
			make clean &>> "$M_LOGS/dkim.txt"; error
		;;
		dkim-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/dkim.txt"; error
		;;
		dkim-log)
			cat "$M_LOGS/dkim.txt"; error
		;;
		dkim)
			dkim "dkim-extract"
			dkim "dkim-prep"
			dkim "dkim-configure"
			dkim "dkim-build"
			dkim "dkim-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/dkim.txt"

	return $?

}

zlib() {

	if [[ $1 == "zlib-extract" ]]; then
		rm -f "$M_LOGS/zlib.txt"; error
	elif [[ $1 != "zlib-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/zlib.txt"
	fi

	case "$1" in
		zlib-extract)
			extract $ZLIB "zlib" &>> "$M_LOGS/zlib.txt"
		;;
		zlib-prep)
			# Apply RHEL zlib prep steps.
			cd "$M_SOURCES/zlib"; error
			if [[ $ZLIB == "1.2.3" ]]; then
				chmod -Rf a+rX,u+w,g-w,o-w . &>> "$M_LOGS/zlib.txt"; error
				cat "$M_PATCHES/zlib/"zlib-1.2.3-autotools.patch | patch -p1 -b --suffix .atools --fuzz=0 &>> "$M_LOGS/zlib.txt"; error
				mkdir m4 &>> "$M_LOGS/zlib.txt"; error
				cat "$M_PATCHES/zlib/"minizip-1.2.3-malloc.patch | patch -p1 -b --suffix .mal --fuzz=0 &>> "$M_LOGS/zlib.txt"; error
				iconv -f windows-1252 -t utf-8 <ChangeLog >ChangeLog.tmp &>> "$M_LOGS/zlib.txt"; error
				mv ChangeLog.tmp ChangeLog &>> "$M_LOGS/zlib.txt"; error
				cp Makefile Makefile.old &>> "$M_LOGS/zlib.txt"; error
			fi
		;;
		zlib-configure)
			cd "$M_SOURCES/zlib"; error
			if [[ $ZLIB == "1.2.3" ]]; then
				export CFLAGS='-O2 -g3 -rdynamic -fPIC -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic'; error
				export CXXFLAGS='-O2 -g3 -rdynamic -fPIC -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic'; error
				export FFLAGS='-O2 -g3 -rdynamic -fPIC -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic -I/usr/lib64/gfortran/modules'; error
				autoreconf --install &>> "$M_LOGS/zlib.txt"; error
				./configure --build=x86_64-unknown-linux-gnu --host=x86_64-unknown-linux-gnu --target=x86_64-redhat-linux-gnu &>> "$M_LOGS/zlib.txt"; error
			else
				export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
				export FFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
				export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
				./configure --64 &>> "$M_LOGS/zlib.txt"; error
			fi
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		zlib-build)
			cd "$M_SOURCES/zlib"; error
			make &>> "$M_LOGS/zlib.txt"; error
		;;
		zlib-check)
			cd "$M_SOURCES/zlib"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/zlib.txt"; error
		;;
		zlib-check-full)
			cd "$M_SOURCES/zlib"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/zlib.txt"; error
		;;
		zlib-clean)
			cd "$M_SOURCES/zlib"; error
			make clean &>> "$M_LOGS/zlib.txt"; error
		;;
		zlib-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/zlib.txt"; error
		;;
		zlib-log)
			cat "$M_LOGS/zlib.txt"; error
		;;
		zlib)
			zlib "zlib-extract"
			zlib "zlib-prep"
			zlib "zlib-configure"
			zlib "zlib-build"
			zlib "zlib-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/zlib.txt"

	return $?

}

bzip2() {

	if [[ $1 == "bzip2-extract" ]]; then
		rm -f "$M_LOGS/bzip2.txt"; error
	elif [[ $1 != "bzip2-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/bzip2.txt"
	fi

	case "$1" in
		bzip2-extract)
			extract $BZIP2 "bzip2" &>> "$M_LOGS/bzip2.txt"
		;;
		bzip2-prep)
			# Apply RHEL bzip2 patches.
			cd "$M_SOURCES/bzip2"; error
			if [[ $ZLIB == "1.0.5" ]]; then
				chmod -Rf a+rX,u+w,g-w,o-w . &>> "$M_LOGS/bzip2.txt"; error
				cat "$M_PATCHES/bzip2/"bzip2-1.0.4-saneso.patch | patch -p1 -b --suffix .saneso --fuzz=0 &>> "$M_LOGS/bzip2.txt"; error
				cat "$M_PATCHES/bzip2/"bzip2-1.0.4-cflags.patch | patch -p1 -b --suffix .cflags --fuzz=0 &>> "$M_LOGS/bzip2.txt"; error
				cat "$M_PATCHES/bzip2/"bzip2-1.0.4-bzip2recover.patch | patch -p1 -b --suffix .bz2recover --fuzz=0 &>> "$M_LOGS/bzip2.txt"; error
			fi
		;;
		bzip2-configure)
			cd "$M_SOURCES/bzip2"; error
		;;
		bzip2-build)
			cd "$M_SOURCES/bzip2"; error
			make CC=gcc AR=ar RANLIB=ranlib 'CFLAGS=-O2 -g3 -fPIC -rdynamic -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic -D_FILE_OFFSET_BITS=64' &>> "$M_LOGS/bzip2.txt"; error
		;;
		bzip2-check)
			cd "$M_SOURCES/bzip2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/bzip2.txt"; error
		;;
		bzip2-check-full)
			cd "$M_SOURCES/bzip2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/bzip2.txt"; error
		;;
		bzip2-clean)
			cd "$M_SOURCES/bzip2"; error
			make clean &>> "$M_LOGS/bzip2.txt"; error
		;;
		bzip2-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/bzip2.txt"; error
		;;
		bzip2-log)
			cat "$M_LOGS/bzip2.txt"; error
		;;
		bzip2)
			bzip2 "bzip2-extract"
			bzip2 "bzip2-prep"
			bzip2 "bzip2-configure"
			bzip2 "bzip2-build"
			bzip2 "bzip2-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/bzip2.txt"

	return $?

}

dspam() {

	if [[ $1 == "dspam-extract" ]]; then
		rm -f "$M_LOGS/dspam.txt"; error
	elif [[ $1 != "dspam-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/dspam.txt"
	fi

	case "$1" in
		dspam-extract)
			extract $DSPAM "dspam" &>> "$M_LOGS/dspam.txt"
		;;
		dspam-prep)
			cd "$M_SOURCES/dspam"; error

			# Changes STATUS( to DSPAM_STATUS( in client.c, dspam.c and agent_shared.c in DSPAM the src folder.
			if [[ $DSPAM == "dspam-3.9.1-RC1" ]]; then
				cat "$M_PATCHES/dspam/"dspam_status_rename_3.9.0.RC1.patch | patch -p1 --verbose &>> "$M_LOGS/dspam.txt"; error
			else
				cat "$M_PATCHES/dspam/"dspam_status_rename_3.10.0.patch | patch -p3 --verbose &>> "$M_LOGS/dspam.txt"; error
			fi

			cat "$M_PATCHES/dspam/"dspam_version.patch | patch -p1 --verbose &>> "$M_LOGS/dspam.txt"; error
			cat "$M_PATCHES/dspam/"dspam_bufixes_3.10.1.patch | patch -p3 --verbose &>> "$M_LOGS/dspam.txt"; error
		;;
		dspam-configure)
			cd "$M_SOURCES/dspam"; error
			# Can't include because the library isn't there till after MySQL is compiled.
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"

			if [ ! -d "$M_SOURCES/mysql/libmysql/.libs/" ]; then
				MYSQL_LIB_PATH="/usr/lib64/mysql/"
				MYSQL_INC_PATH="/usr/include/mysql/"
			else
				MYSQL_LIB_PATH="$M_SOURCES/mysql/libmysql_r/.libs/"
				MYSQL_INC_PATH="$M_SOURCES/mysql/include/"
			fi

			./configure --enable-static --with-pic --enable-preferences-extension --enable-virtual-users \
			--with-storage-driver=mysql_drv --disable-trusted-user-security --disable-mysql4-initialization	\
			--with-mysql-includes=$MYSQL_INC_PATH --with-mysql-libraries=$MYSQL_LIB_PATH &>> "$M_LOGS/dspam.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		dspam-build)
			cd "$M_SOURCES/dspam"; error
			make &>> "$M_LOGS/dspam.txt"; error
		;;
		dspam-check)
			cd "$M_SOURCES/dspam"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/dspam.txt"; error
		;;
		dspam-check-full)
			cd "$M_SOURCES/dspam"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/dspam.txt"; error
		;;
		dspam-clean)
			cd "$M_SOURCES/dspam"; error
			make clean &>> "$M_LOGS/dspam.txt"; error
		;;
		dspam-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/dspam.txt"; error
		;;
		dspam-log)
			cat "$M_LOGS/dspam.txt"; error
		;;
		dspam)
			dspam "dspam-extract"
			dspam "dspam-prep"
			dspam "dspam-configure"
			dspam "dspam-build"
			dspam "dspam-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/dspam.txt"

	return $?

}

mysql() {

	if [[ $1 == "mysql-extract" ]]; then
		rm -f "$M_LOGS/mysql.txt"; error
	elif [[ $1 != "mysql-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/mysql.txt"
	fi

	case "$1" in
		mysql-extract)
			extract $MYSQL "mysql" &>> "$M_LOGS/mysql.txt"
		;;
		mysql-prep)
			cd "$M_SOURCES/mysql"; error
			touch libtoolT; error

		;;
		mysql-configure)
			cd "$M_SOURCES/mysql"; error
			export CFLAGS="-g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"
			export CXXFLAGS="-g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"
			export CPPFLAGS="-g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"

			./configure --with-pic --enable-thread-safe-client --with-readline --with-charset=latin1 --with-extra-charsets=all \
			--with-plugins=all &>> "$M_LOGS/mysql.txt"; error

			# According to the RHEL build spec, MySQL checks will fail without --with-big-tables,
			# And the "with-plugin" and "without-plugin options" do actually work; so we can ignore warnings about them...
			#./configure \
			#--with-mysqld-user="ladar" \
			#--with-extra-charsets=all --with-big-tables --with-pic --with-readline \
			#--enable-static --enable-shared --enable-largefile --enable-thread-safe-client --enable-local-infile \
			#--build=x86_64-redhat-linux-gnu --target=x86_64-redhat-linux-gnu --with-debug  &>> "$M_LOGS/mysql.txt"; error
			#
			#--with-plugins=innodb_plugin,myisam,heap,csv
			#--with-client-ldflags=-all-static --with-mysqld-ldflags=-all-static
			#mkdir working; error
			#--with-plugin-innobase --with-plugin-partition --without-plugin-innodb_plugin

			#--with-plugins=innobase,innodb_plugin,myisam,heap,csv &>> "$M_LOGS/mysql.txt"; error

			# TODO: Develop logic for detecting whether openssl and zlib are built ala DSPAM.
			#--with-zlib-dir="$M_SOURCES/zlib" --with-openssl="$M_SOURCES/openssl" &>> "$M_LOGS/mysql.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		mysql-build)
			cd "$M_SOURCES/mysql"; error
			make --jobs=4 &>> "$M_LOGS/mysql.txt"; error
		;;
		mysql-check)
			cd "$M_SOURCES/mysql"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make --jobs=2 test-fast &>> "$M_LOGS/mysql.txt"; error
		;;
		mysql-check-full)

			cd "$M_SOURCES/mysql"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error

			# The test suite will use this variable to offset the port numbers and prevent clashing.
			export MTR_BUILD_THREAD=86

			# test-full combines the test, test-nr and test-ps targets
			make clean &>> "$M_LOGS/mysql.txt"; error
			make --jobs=2 &>> "$M_LOGS/mysql.txt"; error
			make --jobs=2 test-full &>> "$M_LOGS/mysql.txt"; error

			# make clean &>> "$M_LOGS/mysql.txt"; error
			#make --jobs=2 &>> "$M_LOGS/mysql.txt"; error
			#make test-full-qa &>> "$M_LOGS/mysql.txt"; error
		;;
		mysql-clean)
			cd "$M_SOURCES/mysql"; error
			make clean &>> "$M_LOGS/mysql.txt"; error
		;;
		mysql-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/mysql.txt"; error
		;;
		mysql-log)
			cat "$M_LOGS/mysql.txt"; error
		;;
		mysql)
			mysql "mysql-extract"
			mysql "mysql-prep"
			mysql "mysql-configure"
			mysql "mysql-build"
			mysql "mysql-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/mysql.txt"

	return $?

}

geoip() {

	if [[ $1 == "geoip-extract" ]]; then
		rm -f "$M_LOGS/geoip.txt"; error
	elif [[ $1 != "geoip-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/geoip.txt"
	fi

	case "$1" in
		geoip-extract)
			extract $GEOIP "geoip" &>> "$M_LOGS/geoip.txt"
		;;
		geoip-prep)
			cd "$M_SOURCES/geoip"; error
		;;
		geoip-configure)
			cd "$M_SOURCES/geoip"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure &>> "$M_LOGS/geoip.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		geoip-build)
			cd "$M_SOURCES/geoip"; error
			make &>> "$M_LOGS/geoip.txt"; error
		;;
		geoip-check)
			cd "$M_SOURCES/geoip"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/geoip.txt"; error
		;;
		geoip-check-full)
			cd "$M_SOURCES/geoip"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/geoip.txt"; error
		;;
		geoip-clean)
			cd "$M_SOURCES/geoip"; error
			make clean &>> "$M_LOGS/geoip.txt"; error
		;;
		geoip-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/geoip.txt"; error
		;;
		geoip-log)
			cat "$M_LOGS/geoip.txt"; error
		;;
		geoip)
			geoip "geoip-extract"
			geoip "geoip-prep"
			geoip "geoip-configure"
			geoip "geoip-build"
			geoip "geoip-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/geoip.txt"

	return $?
}

clamav() {

	if [[ $1 == "clamav-extract" ]]; then
		rm -f "$M_LOGS/clamav.txt"; error
	elif [[ $1 != "clamav-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/clamav.txt"
	fi

	case "$1" in
		clamav-extract)
			extract $CLAMAV "clamav" &>> "$M_LOGS/clamav.txt"
		;;
		clamav-prep)
			# Patches cli_rarload() so it looks internally, and changes the name of cache_add() to cl_cache_add().
			cd "$M_SOURCES/clamav"; error
			if [[ $CLAMAV == "clamav-0.97" ]]; then
				cat "$M_PATCHES/clamav/"rarload_097.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error
				cat "$M_PATCHES/clamav/"cacheadd_097.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error
				cat "$M_PATCHES/clamav/"ulimit_097.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error
			elif [[ $CLAMAV =~ "clamav-0.97."[1-2] ]]; then
				cat "$M_PATCHES/clamav/"cacheadd_0971.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error
				cat "$M_PATCHES/clamav/"rarload_0971.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error
				cat "$M_PATCHES/clamav/"ulimit_0971.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error
				cat "$M_PATCHES/clamav/"shutdown_0972.patch | patch -p3 --verbose &>> "$M_LOGS/clamav.txt"; error
			elif [[ $CLAMAV =~ "clamav-0.98.1" ]]; then
				# Add the shutdown and clean up functions.
				cat "$M_PATCHES/clamav/"shutdown_0981.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error

				# Fix the rar library dynamic loading logic.
				cat "$M_PATCHES/clamav/"rarload_0971.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error

				# Fix reference conflict with OpenSSL over the name SHA256_CTX.
				SHA_FILES=`grep --files-with-matches --recursive SHA256_CTX *`; error
				sed -i -e "s/SHA256_CTX/CL_SHA256_CTX/g" $SHA_FILES; error
				unset SHA_FILES
			else
				# Add the shutdown and clean up functions and fix the rar library dynamic loading logic.
				cat "$M_PATCHES/clamav/"shutdown_rarload_0984.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error

				# Output the version number and not the git commit hash.
				cat "$M_PATCHES/clamav/"version_0984.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error
			fi

			# Fix reference conflict with libpng over the filename png.h.
			PNG_FILES=`grep --files-with-matches --recursive "png\\.h" *`; error
			sed -i -e "s/png\.h/clpng\.h/g" $PNG_FILES; error
			mv libclamav/png.h libclamav/clpng.h; error
			unset PNG_FILES

		;;
		clamav-configure)
			cd "$M_SOURCES/clamav"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2 -DGNU_SOURCE"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2 -DGNU_SOURCE"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2 -DGNU_SOURCE"

			# Note that at least in version 0.97 and 0.98, --disable-llvm breaks the unit tests.

			./configure --disable-llvm --enable-check --enable-static &>> "$M_LOGS/clamav.txt"; error

			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LD_LIBRARY_PATH

			if [[ $CLAMAV =~ "clamav-0.9"[7-8]"."[1-9] ]]; then
				# The check3_clamd.sh script will fail if LLVM is disabled.
				# Since were not currently using clamd, the offending check script is replaced.
				# The exit value 77 indicates the check was skipped.
				printf "\x23\x21/bin/bash\nexit 77\n" > "$M_SOURCES/clamav/unit_tests/check2_clamd.sh"; error
				printf "\x23\x21/bin/bash\nexit 77\n" > "$M_SOURCES/clamav/unit_tests/check3_clamd.sh"; error
				printf "\x23\x21/bin/bash\nexit 77\n" > "$M_SOURCES/clamav/unit_tests/check4_clamd.sh"; error
			fi
		;;
		clamav-build)
			cd "$M_SOURCES/clamav"; error
			make --jobs=2 &>> "$M_LOGS/clamav.txt"; error
		;;
		clamav-check)
			cd "$M_SOURCES/clamav"; error

			# Remove read perms for the accdenied file so the test works properly.
			if [[ -e "$M_SOURCES/clamav/unit_tests/accdenied" ]] && [[ ! -f "$M_SOURCES/clamav/unit_tests/accdenied" ]]; then
				chmod --changes u-r "$M_SOURCES/clamav/unit_tests/accdenied" &>> "$M_LOGS/clamav.txt"; error
			fi

			make check &>> "$M_LOGS/clamav.txt"; error

			# Add read perms to accdenied file so it can be checked into the version control repo.
			if [[ -e "$M_SOURCES/clamav/unit_tests/accdenied" ]] && [[ ! -f "$M_SOURCES/clamav/unit_tests/accdenied" ]]; then
				chmod --changes u+r "$M_SOURCES/clamav/unit_tests/accdenied" &>> "$M_LOGS/clamav.txt"; error
			fi
		;;
		clamav-check-full)
			cd "$M_SOURCES/clamav"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error

			# Remove read perms for the accdenied file so the test works properly.
			if [[ -e "$M_SOURCES/clamav/unit_tests/accdenied" ]] && [[ ! -f "$M_SOURCES/clamav/unit_tests/accdenied" ]]; then
				chmod --changes u-r "$M_SOURCES/clamav/unit_tests/accdenied" &>> "$M_LOGS/clamav.txt"; error
			fi

			# Reset the session limits.
			ulimit -f unlimited || ulimit -i 77233 || ulimit -l 64 || ulimit -m unlimited || ulimit -n 1024 || ulimit -q 819200 || ulimit -r 0 || ulimit -s 10240 || ulimit -c 0 || ulimit -d unlimited || ulimit -e 0 || ulimit -t unlimited || ulimit -u 77233 || ulimit -v unlimited || ulimit -x unlimited || ulimit -p 8

			make check VG=1 HG=1 &>> "$M_LOGS/clamav.txt"; error

			# Add read perms to accdenied file so it can be checked into the version control repo.
			if [[ -e "$M_SOURCES/clamav/unit_tests/accdenied" ]] && [[ ! -f "$M_SOURCES/clamav/unit_tests/accdenied" ]]; then
				chmod --changes u-r "$M_SOURCES/clamav/unit_tests/accdenied" &>> "$M_LOGS/clamav.txt"; error
			fi
		;;
		clamav-clean)
			cd "$M_SOURCES/clamav"; error
			make clean &>> "$M_LOGS/clamav.txt"; error
		;;
		clamav-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/clamav.txt"; error
		;;
		clamav-log)
			cat "$M_LOGS/clamav.txt"; error
		;;
		clamav)
			clamav "clamav-extract"
			clamav "clamav-prep"
			clamav "clamav-configure"
			clamav "clamav-build"
			clamav "clamav-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/clamav.txt"

	return $?

}

openssl() {

	if [[ $1 == "openssl-extract" ]]; then
		rm -f "$M_LOGS/openssl.txt"; error
	elif [[ $1 != "openssl-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/openssl.txt"
	fi

	case "$1" in
		openssl-extract)
			extract $OPENSSL "openssl" &>> "$M_LOGS/openssl.txt"
		;;
		openssl-prep)
			# OpenSSL 1.0.0b has a known bug
			# http://www.mail-archive.com/openssl-dev@openssl.org/msg28468.html
			# http://cvs.openssl.org/chngview?cn=19998
			cd "$M_SOURCES/openssl"; error
			if [[ $OPENSSL == "openssl-1.0.0b" ]]; then	cat "$M_PATCHES/openssl/1.0.0b_SSL_server_fix.patch" | patch -p1 --batch &>> "$M_LOGS/openssl.txt"; error; fi
		;;
		openssl-configure)
			# OpenSSL does not use environment variables to pickup additional compiler flags
			# The -d param specifies the creation of a debug build
			cd "$M_SOURCES/openssl"; error
			./config -d shared zlib no-dso no-asm --openssldir="$M_SOURCES/openssl" -g3 -rdynamic -fPIC -DPURIFY -D_FORTIFY_SOURCE=2 &>> "$M_LOGS/openssl.txt"; error
		;;
		openssl-build)
			cd "$M_SOURCES/openssl"; error
			make &>> "$M_LOGS/openssl.txt"; error
			make install_docs &>> "$M_LOGS/openssl.txt"; error
		;;
		openssl-check)
			cd "$M_SOURCES/openssl"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make test &>> "$M_LOGS/openssl.txt"; error
		;;
		openssl-check-full)
			cd "$M_SOURCES/openssl"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make test &>> "$M_LOGS/openssl.txt"; error
		;;
		openssl-clean)
			cd "$M_SOURCES/openssl"; error
			make clean &>> "$M_LOGS/openssl.txt"; error
		;;
		openssl-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/openssl.txt"; error
		;;
		openssl-log)
			cat "$M_LOGS/openssl.txt"; error
		;;
		openssl)
			openssl "openssl-extract"
			openssl "openssl-prep"
			openssl "openssl-configure"
			openssl "openssl-build"
			openssl "openssl-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/openssl.txt"

	return $?

}

jansson() {

	if [[ $1 == "jansson-extract" ]]; then
		rm -f "$M_LOGS/jansson.txt"; error
	elif [[ $1 != "jansson-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/jansson.txt"
	fi

	case "$1" in
		jansson-extract)
			extract $JANSSON "jansson" &>> "$M_LOGS/jansson.txt"
		;;
		jansson-prep)
			cd "$M_SOURCES/jansson"; error
			cat "$M_PATCHES/jansson/"jansson-version.patch | patch -p1 --batch &>> "$M_LOGS/jansson.txt"; error
			if [ -e "/usr/bin/sphinx-1.0-build" ]; then
				cat "$M_PATCHES/jansson/"jansson-sphinx.patch | patch -p3 --batch &>> "$M_LOGS/jansson.txt"; error
			fi
			echo "$M_PATCHES/jansson/"jansson-inlines.patch &>> "$M_LOGS/jansson.txt"; error
			cat "$M_PATCHES/jansson/"jansson-inlines.patch | patch -p1 --batch &>> "$M_LOGS/jansson.txt"; error
			echo "$M_PATCHES/jansson/"jansson-optional-params.patch &>> "$M_LOGS/jansson.txt"; error
			cat "$M_PATCHES/jansson/"jansson-optional-params.patch | patch -p1 --batch &>> "$M_LOGS/jansson.txt"; error
			echo "$M_PATCHES/jansson/"jansson-typeof-string.patch &>> "$M_LOGS/jansson.txt"; error
			cat "$M_PATCHES/jansson/"jansson-typeof-string.patch | patch -p1 --batch  &>> "$M_LOGS/jansson.txt"; error
		;;
		jansson-configure)
			cd "$M_SOURCES/jansson"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O2"
			./configure &>> "$M_LOGS/jansson.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		jansson-build)
			cd "$M_SOURCES/jansson"; error
			make &>> "$M_LOGS/jansson.txt"; error
			if [ -e "/usr/bin/sphinx-1.0-build" ] || [ -e "/usr/bin/sphinx-build" ]; then
				make html &>> "$M_LOGS/jansson.txt"; error
			fi
		;;
		jansson-check)
			cd "$M_SOURCES/jansson"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/jansson.txt"; error
		;;
		jansson-check-full)
			cd "$M_SOURCES/jansson"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/jansson.txt"; error
		;;
		jansson-clean)
			cd "$M_SOURCES/jansson"; error
			make clean &>> "$M_LOGS/jansson.txt"; error
		;;
		jansson-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/jansson.txt"; error
		;;
		jansson-log)
			cat "$M_LOGS/jansson.txt"; error
		;;
		jansson)
			jansson "jansson-extract"
			jansson "jansson-prep"
			jansson "jansson-configure"
			jansson "jansson-build"
			jansson "jansson-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/jansson.txt"

	return $?

}

freetype() {

	if [[ $1 == "freetype-extract" ]]; then
		rm -f "$M_LOGS/freetype.txt"; error
	elif [[ $1 != "freetype-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/freetype.txt"
	fi

	case "$1" in
		freetype-extract)
			extract $FREETYPE "freetype" &>> "$M_LOGS/freetype.txt"
		;;
		freetype-prep)
			cd "$M_SOURCES/freetype"; error
			cat "$M_PATCHES/freetype/"freetype-version.patch | patch -s -p1 -b --fuzz=0; error
		;;
		freetype-configure)
			cd "$M_SOURCES/freetype"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure &>> "$M_LOGS/freetype.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		freetype-build)
			cd "$M_SOURCES/freetype"; error
			make &>> "$M_LOGS/freetype.txt"; error
		;;
		freetype-check)
			cd "$M_SOURCES/freetype"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/freetype.txt"; error
		;;
		freetype-check-full)
			cd "$M_SOURCES/freetype"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/freetype.txt"; error
		;;
		freetype-clean)
			cd "$M_SOURCES/freetype"; error
			make clean &>> "$M_LOGS/freetype.txt"; error
		;;
		freetype-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/freetype.txt"; error
		;;
		freetype-log)
			cat "$M_LOGS/freetype.txt"; error
		;;
		freetype)
			freetype "freetype-extract"
			freetype "freetype-prep"
			freetype "freetype-configure"
			freetype "freetype-build"
			freetype "freetype-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/freetype.txt"

	return $?

}

memcached() {

	if [[ $1 == "memcached-extract" ]]; then
		rm -f "$M_LOGS/memcached.txt"; error
	elif [[ $1 != "memcached-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/memcached.txt"
	fi

	case "$1" in
		memcached-extract)
			extract $MEMCACHED "memcached" &>> "$M_LOGS/memcached.txt"
		;;
		memcached-prep)
			cd "$M_SOURCES/memcached"; error
			if [[ $MEMCACHED == "libmemcached-0.50" ]]; then
				cat "$M_PATCHES/memcached/951_950.diff" | patch -p0 -verbose &>> "$M_LOGS/memcached.txt"; error;
			elif [[ $MEMCACHED == "libmemcached-1.0.8" ]]; then
				# Add the shutdown and clean up functions.
				cat "$M_PATCHES/memcached/"srandom_fix.patch | patch -p1 --verbose &>> "$M_LOGS/memcached.txt"; error
			else
				# Fix memcached memory function unit tests.
				cat "$M_PATCHES/memcached/"memfunction_test_fix_1.0.18.patch | patch -p1 --verbose &>> "$M_LOGS/memcached.txt"; error
			fi
		;;
		memcached-configure)
			cd "$M_SOURCES/memcached"; error

			# If Dtrace or System Tap support is enabled, the libmemcached_probes.o file will need to be manually added to the shared object
			# since it doesn't appear to be included in the libmemcached.a archive file (as of v0.49).
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"

			# unset MEMCACHED_SERVERS
			# export GEARMAND_BINARY="/usr/local/sbin/gearmand"
			# export MEMCACHED_BINARY="/usr/local/bin/memcached"

			# Options used for 1.0.3+
			./configure --disable-silent-rules --disable-dtrace --disable-sasl --disable-libinnodb --disable-libevent --disable-mtmalloc \
			--enable-64bit --enable-largefile --enable-static --enable-shared --with-pic --with-debug \
			--with-memcached="/usr/local/bin/memcached" &>> "$M_LOGS/memcached.txt"; error

			# Options used for 1.0.2
			#./configure --disable-silent-rules --disable-dtrace --disable-sasl --disable-libinnodb --disable-libevent --disable-mtmalloc \
			#--enable-64bit --enable-largefile --enable-static --enable-shared \
			#--with-pic --with-debug --with-valgrind &>> "$M_LOGS/memcached.txt"; error

			# Options used for 0.51
			#./configure --disable-silent-rules --disable-dtrace --disable-sasl --disable-libinnodb --disable-libevent --enable-static &>> "$M_LOGS/memcached.txt"; error

			# An alternative and still experimental strategy for configuring the memcached library.
			# ./configure --disable-silent-rules --disable-sasl --enable-static --with-pic --with-debug --with-valgrind &>> "$M_LOGS/memcached.txt"; error

			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
			# unset GEARMAND_BINARY; unset MEMCACHED_BINARY
		;;
		memcached-build)
			cd "$M_SOURCES/memcached"; error
			make &>> "$M_LOGS/memcached.txt"; error
		;;
		memcached-check)
			cd "$M_SOURCES/memcached"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error

			# Doesn't appear to be necessary anymore...
			#rm -vf /tmp/memcached.pid* &>> "$M_LOGS/memcached.txt"; error

			# For some reason the included version of memcached is being used and causing the unit tests to fail, so overwrite the binary
			#cp /usr/local/bin/memcached "$M_SOURCES/memcached/memcached"

			make check &>> "$M_LOGS/memcached.txt"; error
		;;
		memcached-check-full)
			cd "$M_SOURCES/memcached"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error

			# Doesn't appear to be necessary anymore...
			#rm -vf /tmp/memcached.pid* &>> "$M_LOGS/memcached.txt"; error

			make check &>> "$M_LOGS/memcached.txt"; error
			make valgrind &>> "$M_LOGS/memcached.txt"; error
			make helgrind &>> "$M_LOGS/memcached.txt"; error
			make drd &>> "$M_LOGS/memcached.txt"; error
		;;
		memcached-clean)
			cd "$M_SOURCES/memcached"; error
			make clean &>> "$M_LOGS/memcached.txt"; error
		;;
		memcached-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/memcached.txt"; error
		;;
		memcached-log)
			cat "$M_LOGS/memcached.txt"; error
		;;
		memcached)
			memcached "memcached-extract"
			memcached "memcached-prep"
			memcached "memcached-configure"
			memcached "memcached-build"
			memcached "memcached-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/memcached.txt"

	return $?

}

tokyocabinet() {

	if [[ $1 == "tokyocabinet-extract" ]]; then
		rm -f "$M_LOGS/tokyocabinet.txt"; error
	elif [[ $1 != "tokyocabinet-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/tokyocabinet.txt"
	fi


	case "$1" in
		tokyocabinet-extract)
			extract $TOKYOCABINET "tokyocabinet" &>> "$M_LOGS/tokyocabinet.txt"
		;;
		tokyocabinet-prep)
			# Adds tctreegetboth() and tcndbgetboth().
			cd "$M_SOURCES/tokyocabinet" &>> "$M_LOGS/tokyocabinet.txt"; error
			cat "$M_PATCHES/tokyocabinet/"getboth.patch | patch -p1 --verbose &>> "$M_LOGS/tokyocabinet.txt"; error
			cat "$M_PATCHES/tokyocabinet/"tcndbdup.patch | patch -p3 --verbose &>> "$M_LOGS/tokyocabinet.txt"; error
			cat "$M_PATCHES/tokyocabinet/"fileopts.patch | patch -p3 --verbose &>> "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-configure)
			cd "$M_SOURCES/tokyocabinet"; error
			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			export CPPFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2"
			./configure &>> "$M_LOGS/tokyocabinet.txt"; error
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS
		;;
		tokyocabinet-build)
			cd "$M_SOURCES/tokyocabinet"; error
			make &>> "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-check)
			cd "$M_SOURCES/tokyocabinet"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-check-full)
			cd "$M_SOURCES/tokyocabinet"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			make check &>> "$M_LOGS/tokyocabinet.txt"; error
			make check-valgrind &>> "$M_LOGS/tokyocabinet.txt"; error
			make check-large &>> "$M_LOGS/tokyocabinet.txt"; error
			make check-compare &>> "$M_LOGS/tokyocabinet.txt"; error
			make check-thread &>> "$M_LOGS/tokyocabinet.txt"; error
			make check-race &>> "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-clean)
			cd "$M_SOURCES/tokyocabinet"; error
			make clean &>> "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-log)
			cat "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet)
			tokyocabinet "tokyocabinet-extract"
			tokyocabinet "tokyocabinet-prep"
			tokyocabinet "tokyocabinet-configure"
			tokyocabinet "tokyocabinet-build"
			tokyocabinet "tokyocabinet-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/tokyocabinet.txt"

	return $?

}

combine() {

	rm -f "$M_SO" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/gd" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/gd" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/gd" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/gd/.libs/libgd.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/png" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/png" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/png" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/png/.libs/libpng16.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/lzo" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/lzo" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/lzo" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/lzo/src/.libs/liblzo2.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/jpeg" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/jpeg" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/jpeg" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/jpeg/.libs/libjpeg.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/spf2" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/spf2" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/spf2" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/spf2/src/libspf2/.libs/libspf2.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/curl" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/curl" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/curl" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/curl/lib/.libs/libcurl.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/xml2" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/xml2" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/xml2" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/xml2/.libs/libxml2.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/dkim" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/dkim" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/dkim" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/dkim/libopendkim/.libs/libopendkim.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/zlib" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/zlib" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/zlib" &>> "$M_LOGS/combine.txt"; error

	if [[ $ZLIB == "1.2.3" ]]; then
		ar xv "$M_SOURCES/zlib/.libs/libz.a" &>> "$M_LOGS/combine.txt"; error
	else
		ar xv "$M_SOURCES/zlib/libz.a" &>> "$M_LOGS/combine.txt"; error
	fi

	rm -rf "$M_OBJECTS/bzip2" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/bzip2" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/bzip2" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/bzip2/libbz2.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/dspam" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/dspam" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/dspam" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/dspam/src/.libs/libdspam.a" &>> "$M_LOGS/combine.txt"; error
	#ar xv "$M_SOURCES/dspam/src/.libs/libmysql_drv.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/mysql" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/mysql" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/mysql" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/mysql/libmysql_r/.libs/libmysqlclient_r.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/geoip" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/geoip" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/geoip" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/geoip/libGeoIP/.libs/libGeoIP.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/clamav" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/clamav" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/clamav" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/clamav/libclamav/.libs/libclamav.a" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/clamav/libclamav/.libs/libclamunrar.a" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/clamav/libclamav/.libs/libclamunrar_iface.a" &>> "$M_LOGS/combine.txt"; error

	# Some systems don't appear to need this archive.
	ar xv "$M_SOURCES/clamav/libltdl/.libs/libltdlc.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/crypto" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/crypto" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/crypto" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/openssl/libcrypto.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/ssl" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/ssl" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/ssl" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/openssl/libssl.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/jansson" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/jansson" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/jansson" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/jansson/src/.libs/libjansson.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/freetype" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/freetype" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/freetype" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/freetype/objs/.libs/libfreetype.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/memcached" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/memcached" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/memcached" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/memcached/libmemcached/.libs/libmemcached.a" &>> "$M_LOGS/combine.txt"; error

	# If libmemcached is built with Dtrace support, this object file will need to be included.
	# cp "$M_SOURCES/memcached/libmemcached/libmemcached_probes.o" . &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/tokyocabinet" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/tokyocabinet" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/tokyocabinet" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/tokyocabinet/libtokyocabinet.a" &>> "$M_LOGS/combine.txt"; error

	gcc -Wl,-Bsymbolic -g3 -fPIC -rdynamic -shared -o "$M_SO" "$M_OBJECTS"/lzo/*.o "$M_OBJECTS"/zlib/*.o "$M_OBJECTS"/bzip2/*.o \
		"$M_OBJECTS"/geoip/*.o "$M_OBJECTS"/clamav/*.o "$M_OBJECTS"/tokyocabinet/*.o "$M_OBJECTS"/crypto/*.o "$M_OBJECTS"/ssl/*.o \
		"$M_OBJECTS"/mysql/*.o "$M_OBJECTS"/xml2/*.o "$M_OBJECTS"/spf2/*.o "$M_OBJECTS"/curl/*.o "$M_OBJECTS"/memcached/*.o \
		"$M_OBJECTS"/dkim/*.o "$M_OBJECTS"/dspam/*.o "$M_OBJECTS"/jansson/*.o "$M_OBJECTS"/png/*.o "$M_OBJECTS"/jpeg/*.o "$M_OBJECTS"/freetype/*.o \
		"$M_OBJECTS"/gd/*.o \
		-lm -lrt -ldl -lnsl -lresolv -lpthread -lstdc++ &>> "$M_LOGS/combine.txt"; error

	# Commands for creating a static version of the library.
	# rm -f "$M_ROOT/libmagmad.a"; error
	# ar -r "$M_ROOT/libmagmad.a" "$M_OBJECTS"/lzo/*.o "$M_OBJECTS"/zlib/*.o "$M_OBJECTS"/bzip2/*.o \
	#	"$M_OBJECTS"/clamav/*.o "$M_OBJECTS"/tokyocabinet/*.o "$M_OBJECTS"/crypto/*.o "$M_OBJECTS"/ssl/*.o \
	#	"$M_OBJECTS"/mysql/*.o "$M_OBJECTS"/xml2/*.o "$M_OBJECTS"/spf2/*.o "$M_OBJECTS"/curl/*.o \
	#	"$M_OBJECTS"/memcached/*.o "$M_OBJECTS"/dkim/*.o "$M_OBJECTS"/dspam/*.o "$M_OBJECTS"/jansson/*.o &>> "$M_LOGS/combine.txt"; error

	date +"%nFinished creating the shared object at %r on %x%n"
}

load() {

	echo ""
	echo "Checking shared object..."
	echo ""

	mkdir -p "$M_CHECK"; error
	cd "$M_CHECK"; error

	# Copy the current symbols file over.
	cat $M_SYM_FILE | egrep -v $M_SYM_SKIP > magma.open.symbols.h; error

	# Copy the source files for this test
	cp "$M_CHECK_SO_SOURCES/magma.open.check.h" .; error
	cp "$M_CHECK_SO_SOURCES/magma.open.check.c" .; error

	# Create a file with a function that assigns the original symbols to the dynamic version.
	echo "#include \"magma.open.check.h\"" > magma.open.symbols.c; error
	echo "#include \"magma.open.symbols.h\"" >> magma.open.symbols.c; error
	echo "void symbols_check(void) {" >> magma.open.symbols.c; error
	cat magma.open.symbols.h | grep -v "^$" | grep -v "#" | awk -F'(' '{print $2}' | tr -d "*" | sed 's/_d)//g' | awk '{ print $1 "_d = &" $1 ";"}' | grep -v "^_d = &;$" >> magma.open.symbols.c; error
	echo "}" >> magma.open.symbols.c; error

	# Because GeoIPDBDescription is an array of pointers it doesn't need the leading ampersand.
	sed -i -e "s/GeoIPDBDescription_d = &GeoIPDBDescription;/GeoIPDBDescription_d = GeoIPDBDescription;/g" magma.open.symbols.c; error

	# This function prototype is prefixed with macro in paraentheses which fools the default parsing rules.
	sed -i -e "s/SSL_COMP)_d = &SSL_COMP);/SSL_COMP_get_compression_methods_d = \&SSL_COMP_get_compression_methods;/g" magma.open.symbols.c; error

	# The name dkim_getsighdr_d is taken by the OpenDKIM library, so we had to break convention and use dkim_getsighdrx_d.
	sed -i -e "s/\&dkim_getsighdrx/\&dkim_getsighdr/g" magma.open.symbols.c; error

	# Compile the source files. If an error occurs at compile time it is probably because we have a mismatch somewhere.
	gcc -D_REENTRANT -D_GNU_SOURCE -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE $M_SYM_DIRS $M_SO -g3 -rdynamic -Wall -Wextra -Werror \
		-o magma.open.check magma.open.check.c magma.open.symbols.c -ldl

	# If errors are generated from invalid symbols, this should print out the specific lines that are invalid.
	if [ $? -ne 0 ]; then

		LNS=`gcc -D_REENTRANT -D_GNU_SOURCE -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE $M_SYM_DIRS $M_SO -g3 -rdynamic -Wall -Wextra -Werror \
			-o magma.open.check magma.open.check.c magma.open.symbols.c -ldl 2>&1 | grep "magma.open.symbols.c" | awk -F':' '{ print $2 }' | \
			grep "[0-9*]" | awk '{print $1 ", " }' | sort -gu | uniq | tr -d "\n" | sed "s/, $//g"`

		# Only output the symbol info we found lines to print.
		if [ "$LNS" != "" ]; then

		echo ""
		echo "printing invalid symbols..."
		echo "lines = " $LNS
		echo ""

		LNS=`gcc -D_REENTRANT -D_GNU_SOURCE -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE $M_SYM_DIRS $M_SO -g3 -rdynamic -Wall -Wextra -Werror \
			-o magma.open.check magma.open.check.c magma.open.symbols.c -ldl 2>&1 | grep "magma.open.symbols.c" | awk -F':' '{ print $2 }' | \
			grep "[0-9*]" | awk '{print $1 "p;" }' | sort -gu | uniq | tr -d "\n"`

		cat magma.open.symbols.c | sed -n "$LNS"; error

		fi

		echo ""
		exit 1
	fi

	# Execute the program to see if the library can be loaded successfully at run time.
	./magma.open.check "$M_SO"; error
}

combo() {

	date +"%nStarting $1 at %r on %x%n" &>> "$M_LOGS/build.txt"


	# The ClamAV checks will timeout if the system is under heavy load, so check that library all by itself first.
	if [[ $1 == "check" ]] || [[ $1 == "check-full" ]]; then
		($M_BUILD "clamav-$1") & CLAMAV_PID=$!
		wait $CLAMAV_PID; error

	else
		($M_BUILD "clamav-$1") & CLAMAV_PID=$!
	fi

	($M_BUILD "gd-$1") & GD_PID=$!
	($M_BUILD "png-$1") & PNG_PID=$!
	($M_BUILD "lzo-$1") & LZO_PID=$!
	($M_BUILD "jpeg-$1") & JPEG_PID=$!
	($M_BUILD "curl-$1") & CURL_PID=$!
	($M_BUILD "spf2-$1") & SPF2_PID=$!
	($M_BUILD "xml2-$1") & XML2_PID=$!
	($M_BUILD "dkim-$1") & DKIM_PID=$!
	($M_BUILD "zlib-$1") & ZLIB_PID=$!
	($M_BUILD "bzip2-$1") & BZIP2_PID=$!
	($M_BUILD "dspam-$1") & DSPAM_PID=$!
	($M_BUILD "mysql-$1") & MYSQL_PID=$!
	($M_BUILD "geoip-$1") & GEOIP_PID=$!
	($M_BUILD "openssl-$1") & OPENSSL_PID=$!
	($M_BUILD "jansson-$1") & JANSSON_PID=$!
	($M_BUILD "freetype-$1") & FREETYPE_PID=$!
	($M_BUILD "memcached-$1") & MEMCACHED_PID=$!
	($M_BUILD "tokyocabinet-$1") & TOKYOCABINET_PID=$!

	wait $GD_PID; error
	wait $PNG_PID; error
	wait $LZO_PID; error
	wait $JPEG_PID; error
	wait $CURL_PID; error
	wait $SPF2_PID; error
	wait $XML2_PID; error
	wait $DKIM_PID; error
	wait $ZLIB_PID; error
	wait $BZIP2_PID; error
	wait $DSPAM_PID; error
	wait $MYSQL_PID; error
	wait $GEOIP_PID; error
	wait $OPENSSL_PID; error
	wait $JANSSON_PID; error
	wait $FREETYPE_PID; error
	wait $MEMCACHED_PID; error
	wait $TOKYOCABINET_PID; error

	if [[ $1 != "check" ]] && [[ $1 != "check-full" ]]; then
		wait $CLAMAV_PID; error
	fi

	date +"%nFinished $1 at %r on %x%n"
	date +"%nFinished $1 at %r on %x%n" &>> "$M_LOGS/build.txt"
}

follow() {
	tail -n 0 -F "$M_LOGS/clamav.txt" "$M_LOGS/curl.txt" "$M_LOGS/dspam.txt" "$M_LOGS/jansson.txt" "$M_LOGS/memcached.txt" "$M_LOGS/openssl.txt" \
		"$M_LOGS/tokyocabinet.txt" "$M_LOGS/zlib.txt" "$M_LOGS/bzip2.txt" "$M_LOGS/dkim.txt" "$M_LOGS/geoip.txt" "$M_LOGS/lzo.txt" \
		"$M_LOGS/mysql.txt" "$M_LOGS/spf2.txt" "$M_LOGS/xml2.txt" "$M_LOGS/gd.txt" "$M_LOGS/png.txt" "$M_LOGS/jpeg.txt" "$M_LOGS/freetype.txt"
}

log() {
	cat "$M_LOGS/clamav.txt" "$M_LOGS/curl.txt" "$M_LOGS/dspam.txt" "$M_LOGS/jansson.txt" "$M_LOGS/memcached.txt" "$M_LOGS/openssl.txt" \
		"$M_LOGS/tokyocabinet.txt" "$M_LOGS/zlib.txt" "$M_LOGS/bzip2.txt" "$M_LOGS/dkim.txt" "$M_LOGS/geoip.txt" "$M_LOGS/lzo.txt" \
		"$M_LOGS/mysql.txt" "$M_LOGS/spf2.txt" "$M_LOGS/xml2.txt" "$M_LOGS/gd.txt" "$M_LOGS/png.txt" "$M_LOGS/jpeg.txt" "$M_LOGS/freetype.txt"
}

advance() {
	shift
	echo "$@"
}

status() {

	CPU=`iostat cpu | head -4 | tail -2`
	DISK=`iostat -m -x sda sdb sdc | tail -n +6 | awk '{print $1 "\t\t\t" $6 "\t" $7 "\t" $12}'`

	while true; do
		clear
		tput sgr0;  tput sgr 0 1; tput setaf 6; printf "\n# Commands\n\n"; tput sgr0
		ps --no-headers -C build -C build.sh -C build.lib -C build.lib.sh -o command:100,etime | grep -v status | cat - |
		while read line; do
			BASE=`echo "$line" | awk '{print $1}'`
			line=`eval "advance $line"`
			C=`basename "$BASE"`
			if [[ "$C" == "bash" ]]; then
				BASE=`echo "$line" | awk '{print $1}'`
				line=`eval "advance $line"`
				C=`basename "$BASE"`
			fi
			echo "$C $line"
		done
		tput sgr0;  tput sgr 0 1; tput setaf 6; printf "\n# Load\n\n"; tput sgr0
		uptime | sed "s/^ //"
		tput sgr0;  tput sgr 0 1; tput setaf 6; printf "\n# Processor\n\n"; tput sgr0
		echo "$CPU"
		tput sgr0;  tput sgr 0 1; tput setaf 6; printf "\n# Disk\n\n"; tput sgr0
		echo "$DISK"

		# Refresh the stats for the next loop; note that this takes 4 seconds to complete.
		CPU=`iostat cpu 4 2 | tail -5 | head -2`
		DISK=`iostat -m -x sda sdb sdc 4 2 | tail -3 | head -2 | awk '{print $1 "\t\t\t" $6 "\t" $7 "\t" $12}'`
	done
}

all() {
	rm -f "$M_LOGS/build.txt"; error
	date +"%nStarting at %r on %x%n"
	date +"Starting at %r on %x" &>> "$M_LOGS/build.txt"
	$M_BUILD "extract"
	$M_BUILD "prep"
	$M_BUILD "configure"
	$M_BUILD "build"

	# The DSPAM configure script requires the Magma version of MySQL to be compiled ahead of time for linkage, to accomodate we rebuild DSPAM at this
	# point and replace the version linked against the system's MySQL library/headers.
	$M_BUILD "dspam-configure"
	$M_BUILD "dspam-build"

	$M_BUILD "combine"
	$M_BUILD "load"
	$M_BUILD "check"
	date +"%nFinished at %r on %x%n"
	date +"Finished at %r on %x" &>> "$M_LOGS/build.txt"
}

# Store the command for failure messages
COMMAND="$@"

# Parent
if [[ "$PARENT" == "" ]]; then
	export PARENT="$BASHPID"
fi

# Setup
if [ ! -d "$M_SOURCES" ]; then mkdir "$M_SOURCES"; error; fi
if [ ! -d "$M_LOGS" ]; then mkdir "$M_LOGS"; error; fi
if [ ! -d "$M_OBJECTS" ]; then mkdir "$M_OBJECTS"; error; fi

# Aggregations
if [[ $1 == "extract" ]]; then combo "$1"
elif [[ $1 == "prep" ]]; then  combo "$1"
elif [[ $1 == "configure" ]]; then  combo "$1"
elif [[ $1 == "build" ]]; then combo "$1"
elif [[ $1 == "check" ]]; then combo "$1"
elif [[ $1 == "check-full" ]]; then combo "$1"
elif [[ $1 == "clean" ]]; then combo "$1"

# Libraries
elif [[ $1 =~ "gd" ]]; then (gd "$1") & GD_PID=$!; wait $GD_PID
elif [[ $1 =~ "png" ]]; then (png "$1") & PNG_PID=$!; wait $PNG_PID
elif [[ $1 =~ "lzo" ]]; then (lzo "$1") & LZO_PID=$!; wait $LZO_PID
elif [[ $1 =~ "jpeg" ]]; then (jpeg "$1") & JPEG_PID=$!; wait $JPEG_PID
elif [[ $1 =~ "curl" ]]; then (curl "$1") & CURL_PID=$!; wait $CURL_PID
elif [[ $1 =~ "spf2" ]]; then (spf2 "$1") & SPF2_PID=$!; wait $SPF2_PID
elif [[ $1 =~ "xml2" ]]; then (xml2 "$1") & XML2_PID=$!; wait $XML2_PID
elif [[ $1 =~ "dkim" ]]; then (dkim "$1") & DKIM_PID=$!; wait $DKIM_PID
elif [[ $1 =~ "zlib" ]]; then (zlib "$1") & ZLIB_PID=$!; wait $ZLIB_PID
elif [[ $1 =~ "bzip2" ]]; then (bzip2 "$1") & BZIP2_PID=$!; wait $BZIP2_PID
elif [[ $1 =~ "dspam" ]]; then (dspam "$1") & DSPAM_PID=$!; wait $DSPAM_PID
elif [[ $1 =~ "mysql" ]]; then (mysql "$1") & MYSQL_PID=$!; wait $MYSQL_PID
elif [[ $1 =~ "geoip" ]]; then (geoip "$1") & GEOIP_PID=$!; wait $GEOIP_PID
elif [[ $1 =~ "clamav" ]]; then (clamav "$1") & CLAMAV_PID=$!; wait $CLAMAV_PID
elif [[ $1 =~ "openssl" ]]; then (openssl "$1") & OPENSSL_PID=$!; wait $OPENSSL_PID
elif [[ $1 =~ "jansson" ]]; then (jansson "$1") & JANSSON_PID=$!; wait $JANSSON_PID
elif [[ $1 =~ "freetype" ]]; then (freetype "$1") & FREETYPE_PID=$!; wait $FREETYPE_PID
elif [[ $1 =~ "memcached" ]]; then (memcached "$1") & MEMCACHED_PID=$!; wait $MEMCACHED_PID
elif [[ $1 =~ "tokyocabinet" ]]; then (tokyocabinet "$1") & TOKYOCABINET_PID=$!; wait $TOKYOCABINET_PID

# Globals
elif [[ $1 == "combine" ]]; then combine
elif [[ $1 == "status" ]]; then status
elif [[ $1 == "follow" ]]; then follow
elif [[ $1 == "load" ]]; then load
elif [[ $1 == "log" ]]; then log
elif [[ $1 == "all" ]]; then all

# If follow were called tail it would create a keyword conflict, but we still want to be able to use tail on the command line.
elif [[ $1 == "tail" ]]; then follow

# Catchall
else
	echo ""
	echo $"  `basename $0` {gd|png|lzo|jpeg|curl|spf2|xml2|dkim|zlib|bzip2|dspam|mysql|geoip|clamav|openssl|freetype|memcached|tokyocabinet} and/or "
	echo $"  `basename $0` {extract|prep|configure|build|check|check-full|clean|tail|log} or "
	echo $"  `basename $0` {combine|load|follow|log|status|all}"
	echo ""
	exit 2
fi

# Beep the speaker 10 times to let us know when 'all' is done or 3 times for something else.
if [[ "$PARENT" == "$BASHPID" ]]; then

	if [[ $1 == "all" ]]; then
		NUMS="1 2 3 4 5 6 7 8 9 10"
	else
		NUMS="1 2 3"
	fi

	for i in $NUMS; do
	  printf "\a"; sleep 1
	done

fi

exit 0


