#!/bin/bash

# Name: build.lib.sh
# Author: Ladar Levison
#
# Description: Used to compile the external dependencies required by magma, and combine them into the magmad.so shared object file.

# Preliminary Dependency Matrix

# bzip2 -:-
# checker -:-
# clamav -:- zlib bzip pcre libxml2
# curl -:- zlib openssl
# dkim -:- openssl
# dspam -:- mysql
# freetype -:- zlib bzip2 png
# gd -:- zlib png jpeg freetype
# geoip -:- zlib
# googtap -:-
# googtest -:-
# jansson -:-
# jpeg -:-
# lzo -:-
# memcached -:-
# mysql -:- zlib openssl
# openssl -:- zlib
# pcre -:-
# png -:- zlib
# spf2 -:-
# tokyocabinet -:- zlib bzip2
# utf8proc -:- curl
# xml2 -:- zlib
# zlib -:-

# Install Sphinx (python-sphinx package) to build Jansson HTML docs

# Enable the SELinux boolean allow_exestack or the shared object loader will fail
# The symbols.h file may need to be imported from the magma project for the object load test
# Note that some platforms may require setting the CFLAGS/CPPFLAGS environment variables to -Wold-style-cast when compiling memcached

# To generate a SLOC report for each project:
# cd $M_SOURCES; find  -maxdepth 1 -type d -printf '\n\n%P\n' -exec sloc --quiet --progress-rate=0 {} \; | grep -v "http://cloc.sourceforge.net"

LINK=`readlink -f $0`
BASE=`dirname $LINK`
M_BUILD=`readlink -f $0`

cd $BASE/../../../lib/

M_ROOT=`pwd`

# Set parent directory as project root by default (used to find scripts,
# bundled tarballs, patches, etc.)
if [ -z "$M_PROJECT_ROOT" ]; then
	M_PROJECT_ROOT=`readlink -f ..`
fi

# Read in the build parameters.
. "$M_PROJECT_ROOT/dev/scripts/builders/build.lib.params.sh"

# This undocumented environment variables makes it easy to use the "all" command line option, but skip still the 
# normal dependency checks, this making the build process faster. The QUICK option is undocumented, because in general, 
# developers interacting directly with the build.lib.sh script, should be running the "check" step to ensure
# they have a valid, and properly configured, and functional platform capable of running magma.
if [ -z "$QUICK" ] || [ "$QUICK" != "yes" ]; then
	QUICK="no"	
fi

error() {
	if [ $? -ne 0 ]; then
		tput sgr0; tput setaf 1; date +"%n%n$COMMAND failed at %r on %x%n%n";	tput sgr0
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
			
			if [[ $GD == "gd-2.0.35" ]]; then
				
				# A stack of patches needed to fix a variety of bugs in the neglected 2.0.X series. 
				cat "$M_PATCHES/gd/"gd-2.0.33-freetype.patch | patch -s -p1 -b --suffix .freetype --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-2.0.34-multilib.patch | patch -s -p1 -b --suffix .mlib --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-loop.patch | patch -s -p1 -b --suffix .loop --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-2.0.35-overflow.patch | patch -s -p1 -b --suffix .overflow --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-2.0.34-sparc64.patch | patch -s -p1 -b --suffix .sparc64 --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-2.0.35-AALineThick.patch | patch -s -p1 -b --suffix .AALineThick --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-2.0.33-BoxBound.patch | patch -s -p1 -b --suffix .bb --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-2.0.34-fonts.patch | patch -s -p1 -b --suffix .fonts --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-2.0.35-time.patch | patch -s -p1 -b --suffix .time --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-2.0.35-security3.patch | patch -s -p1 -b --suffix .sec3 --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-version.patch | patch -s -p1 -b --fuzz=0 &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd-sigcmp.patch | patch -s -p1 -b --fuzz=0 &>> "$M_LOGS/gd.txt" ; error				
			else
				
				# Of the patches above, these are the only ones still applicable to the 2.2.X series. They have
				# been updated, so an equivalent change is made to a 2.2.X release.
				cat "$M_PATCHES/gd/"sparc_cflags_2.2.5.patch | patch -p1 --verbose &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"gd_gif_loop_2.2.5.patch | patch -p1 --verbose &>> "$M_LOGS/gd.txt" ; error
				cat "$M_PATCHES/gd/"default_fontpath_2.2.25.patch | patch -p1 --verbose &>> "$M_LOGS/gd.txt" ; error
				
				# The bounding box patch is currently broken. 
				# cat "$M_PATCHES/gd/"bounding_box_2.2.5.patch | patch -p1 --verbose &>> "$M_LOGS/gd.txt" ; error
			fi
		;;
		gd-build)
			cd "$M_SOURCES/gd"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before gd.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libpng.so ] || [ ! -f "$M_LDPATH"/libpng.a ] || [ ! -f "$M_PKGPATH"/libpng.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build png before gd.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libjpeg.so ] || [ ! -f "$M_LDPATH"/libjpeg.a ] || [ ! -f "$M_PKGPATH"/libjpeg.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build jpeg before gd.\n"; tput sgr0
				return 3
			fi
			
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			# We need to override the PNG flags, otherwise the system include files/libraries might be used by mistake.
			export PKG_CONFIG_LIBDIR="$M_PKGPATH" 
			export LIBPNG_LIBS=" `pkg-config --libs libpng` "
			export LIBPNG_CFLAGS=" `pkg-config --cflags libpng` "
			unset PKG_CONFIG_LIBDIR
			
			# We need to override the JPEG flags, otherwise the system include files/libraries might be used by mistake.
			export PKG_CONFIG_LIBDIR="$M_PKGPATH"
			export LIBJPEG_LIBS=" `pkg-config --libs libjpeg` "
			export LIBJPEG_CFLAGS=" `pkg-config --cflags libjpeg` "
			unset PKG_CONFIG_LIBDIR
			
			./configure --without-x --without-xpm --without-tiff --without-fontconfig \
				--with-png="$M_LOCAL" --with-jpeg="$M_LOCAL" --with-freetype="$M_LOCAL" \
				--prefix="$M_LOCAL" &>> "$M_LOGS/gd.txt"; error
				
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS; unset LIBPNG_LIBS; unset LIBPNG_CFLAGS; unset LIBJPEG_LIBS; unset LIBJPEG_CFLAGS

			make --jobs=4 &>> "$M_LOGS/gd.txt"; error
			make install &>> "$M_LOGS/gd.txt"; error
		;;
		gd-check)
			cd "$M_SOURCES/gd"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/gd.txt"; error
		;;
		gd-check-full)
			cd "$M_SOURCES/gd"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
			cat "$M_PATCHES/png/"version_script_1.6.5.patch | patch -p1 --verbose &>> "$M_LOGS/gd.txt" ; error
		;;
		png-build)
			cd "$M_SOURCES/png"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before png.\n"; tput sgr0
				return 3
			fi
			
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			./configure --prefix="$M_LOCAL" &>> "$M_LOGS/png.txt"; error

			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS

			make --jobs=4 &>> "$M_LOGS/png.txt"; error
			make install &>> "$M_LOGS/png.txt"; error
		;;
		png-check)
			cd "$M_SOURCES/png"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/png.txt"; error
		;;
		png-check-full)
			cd "$M_SOURCES/png"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
		lzo-build)
			cd "$M_SOURCES/lzo"; error
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			./configure --enable-shared --prefix="$M_LOCAL" &>> "$M_LOGS/lzo.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS

			make --jobs=4 &>> "$M_LOGS/lzo.txt"; error
			make install &>> "$M_LOGS/lzo.txt"; error
		;;
		lzo-check)
			cd "$M_SOURCES/lzo"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make test &>> "$M_LOGS/lzo.txt"; error
		;;
		lzo-check-full)
			cd "$M_SOURCES/lzo"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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

pcre() {

	if [[ $1 == "pcre-extract" ]]; then
		rm -f "$M_LOGS/pcre.txt"; error
	elif [[ $1 != "pcre-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/pcre.txt"
	fi

	case "$1" in
		pcre-extract)
			extract $PCRE "pcre" &>> "$M_LOGS/pcre.txt"
		;;
		pcre-prep)
			cd "$M_SOURCES/pcre"; error
		;;
		pcre-build)
			cd "$M_SOURCES/pcre"; error
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			./configure --prefix="$M_LOCAL" &>> "$M_LOGS/pcre.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS

			make --jobs=4 &>> "$M_LOGS/pcre.txt"; error
			make install &>> "$M_LOGS/pcre.txt"; error
		;;
		pcre-check)
			cd "$M_SOURCES/pcre"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/pcre.txt"; error
		;;
		pcre-check-full)
			cd "$M_SOURCES/pcre"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/pcre.txt"; error
		;;
		pcre-clean)
			cd "$M_SOURCES/pcre"; error
			make clean &>> "$M_LOGS/pcre.txt"; error
		;;
		pcre-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/pcre.txt"; error
		;;
		pcre-log)
			cat "$M_LOGS/pcre.txt"; error
		;;
		pcre)
			pcre "pcre-extract"
			pcre "pcre-prep"
			pcre "pcre-build"
			pcre "pcre-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/pcre.txt"

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
		jpeg-build)
			cd "$M_SOURCES/jpeg"; error
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			./configure --prefix="$M_LOCAL" &>> "$M_LOGS/jpeg.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS

			make --jobs=4 &>> "$M_LOGS/jpeg.txt"; error
			make install &>> "$M_LOGS/jpeg.txt"; error
		;;
		jpeg-check)
			cd "$M_SOURCES/jpeg"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/jpeg.txt"; error
		;;
		jpeg-check-full)
			cd "$M_SOURCES/jpeg"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
			cat "$M_PATCHES/spf2/"remove-libreplace.patch | patch -p1 --verbose &>> "$M_LOGS/spf2.txt"; error
			cat "$M_PATCHES/spf2/"autoreconf-on-el6.patch | patch -p1 --verbose &>> "$M_LOGS/spf2.txt"; error
			cat "$M_PATCHES/spf2/"expand_hostname_buffer.patch | patch -p1 --verbose &>> "$M_LOGS/spf2.txt"; error
			cat "$M_PATCHES/spf2/"fix_use_after_free.patch | patch -p1 --verbose &>> "$M_LOGS/spf2.txt"; error
			cat "$M_PATCHES/spf2/"handle_redirect_mechanism.patch | patch -p1 --verbose &>> "$M_LOGS/spf2.txt"; error
			cat "$M_PATCHES/spf2/"return_reason_for_dns_failure.patch | patch -p1 --verbose &>> "$M_LOGS/spf2.txt"; error
		;;
		spf2-build)
			cd "$M_SOURCES/spf2"; error
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			./configure --prefix="$M_LOCAL" &>> "$M_LOGS/spf2.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS

			make --jobs=4 &>> "$M_LOGS/spf2.txt"; error
			make install &>> "$M_LOGS/spf2.txt"; error
		;;
		spf2-check)
			cd "$M_SOURCES/spf2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/spf2.txt"; error
		;;
		spf2-check-full)
			cd "$M_SOURCES/spf2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
		curl-build)
			# Note that if we don't include the debug configure option we can't run a check-full.
			cd "$M_SOURCES/curl"; error

			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before curl.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libcrypto.so ] || [ ! -f "$M_LDPATH"/libcrypto.a ] || [ ! -f "$M_PKGPATH"/libcrypto.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build openssl before curl.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libssl.so ] || [ ! -f "$M_LDPATH"/libssl.a ] || [ ! -f "$M_PKGPATH"/libssl.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build openssl before curl.\n"; tput sgr0
				return 3
			fi

			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"

			./configure --enable-debug --enable-static=yes \
				--without-librtmp --without-krb4 --without-krb5 --without-libssh2 \
				--without-ca-bundle --without-ca-path --without-libidn \
				--disable-file --disable-ftp --disable-ftps --disable-gopher \
				--disable-imap --disable-imaps --disable-pop3 --disable-pop3s \
				--disable-rtsp --disable-smtp --disable-smtps --disable-telnet \
				--disable-tftp --disable-ldap --disable-ssh --disable-dict \
				--build=x86_64-redhat-linux-gnu --target=x86_64-redhat-linux-gnu --with-pic \
				--with-ssl="$M_SOURCES/openssl" --with-zlib="$M_SOURCES/zlib" \
				--prefix="$M_LOCAL" &>> "$M_LOGS/curl.txt"; error

			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS

			make --jobs=4 &>> "$M_LOGS/curl.txt"; error
			make install &>> "$M_LOGS/curl.txt"; error
		;;
		curl-check)
			# The target 'check' is an alias for the targets 'test' and 'examples'
			cd "$M_SOURCES/curl"; error
			
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			
			# To avoid having curl run all of its tests using valgrind, we pass the '-n' option to the runtests.pl script. 
			sed -i -e "s/^\(TEST = .*runtests.pl\).*/\1 -n/g" tests/Makefile
			
			make --jobs=4 examples &>> "$M_LOGS/curl.txt"; error
			make --jobs=4 test &>> "$M_LOGS/curl.txt"; error
		;;
		curl-check-full)

			# The target 'check' is an alias for the targets 'test' and 'examples'
			cd "$M_SOURCES/curl"; error
			
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			
			# Since valgrind may have been disabled above, we ensure the -n flag is removed here. 
			sed -i -e "s/^\(TEST = .*runtests.pl\).*/\1/g" tests/Makefile
			
			make --jobs=4 examples &>> "$M_LOGS/curl.txt"; error
			make --jobs=4 test &>> "$M_LOGS/curl.txt"; error
			make --jobs=4 test-full &>> "$M_LOGS/curl.txt"; error
			# make --jobs=4 test-torture &>> "$M_LOGS/curl.txt"; error

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
		xml2-build)
			cd "$M_SOURCES/xml2"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before xml2.\n"; tput sgr0
				return 3
			fi
			
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			./configure --without-lzma --without-python --without-http --without-ftp --with-zlib=$M_SOURCES/zlib \
				--prefix="$M_LOCAL" &>> "$M_LOGS/xml2.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS

			make --jobs=4 &>> "$M_LOGS/xml2.txt"; error
			make install &>> "$M_LOGS/xml2.txt"; error
		;;
		xml2-check)
			cd "$M_SOURCES/xml2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/xml2.txt"; error
		;;
		xml2-check-full)
			cd "$M_SOURCES/xml2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
			cat "$M_PATCHES/dkim/"opendkim.ticket226.patch | patch -p1 --verbose &>> "$M_LOGS/dkim.txt"; error
			cat "$M_PATCHES/dkim/"opendkim_headers_2.10.3.patch | patch -p1 --verbose &>> "$M_LOGS/dkim.txt"; error
		;;
		dkim-build)
			cd "$M_SOURCES/dkim"; error
			
			if [ ! -f "$M_LDPATH"/libcrypto.so ] || [ ! -f "$M_LDPATH"/libcrypto.a ] || [ ! -f "$M_PKGPATH"/libcrypto.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build openssl before dkim.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libssl.so ] || [ ! -f "$M_LDPATH"/libssl.a ] || [ ! -f "$M_PKGPATH"/libssl.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build openssl before dkim.\n"; tput sgr0
				return 3
			fi
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH -L$M_LDPATH/engines/ -Wl,-rpath,$M_LDPATH/engines/ $M_LDFLAGS"
				
			./configure \
				--disable-filter --without-milter --without-sasl --without-gnutls --without-odbx \
				--without-openldap --with-openssl="$M_SOURCES/openssl" \
				--prefix="$M_LOCAL" &>> "$M_LOGS/dkim.txt"; error
				
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS

			make --jobs=4 &>> "$M_LOGS/dkim.txt"; error
			make install &>> "$M_LOGS/dkim.txt"; error
		;;
		dkim-check)
			cd "$M_SOURCES/dkim"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/dkim.txt"; error
		;;
		dkim-check-full)
			cd "$M_SOURCES/dkim"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
			cd "$M_SOURCES/zlib"; error
		;;
		zlib-build)
			cd "$M_SOURCES/zlib"; error

			export CFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export FFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_FFLAGS"
			export CXXFLAGS="-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			
			./configure --prefix="$M_LOCAL" --64 &>> "$M_LOGS/zlib.txt"; error
		
			unset CFLAGS; unset CXXFLAGS; unset FFLAGS

			make &>> "$M_LOGS/zlib.txt"; error
			make install &>> "$M_LOGS/zlib.txt"; error

			# Fool Autotools checks into thinking this is a normal OpenSSL install (e.g., clamav)
			ln -s `pwd` lib
			ln -s `pwd` include
		;;
		zlib-check)
			cd "$M_SOURCES/zlib"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/zlib.txt"; error
		;;
		zlib-check-full)
			cd "$M_SOURCES/zlib"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
			cd "$M_SOURCES/bzip2"; error
			
			chmod -Rf a+rX,u+w,g-w,o-w . &>> "$M_LOGS/bzip2.txt" ; error
			
			# We use slightly different patches depending on the bzip2 version. These patches were largely 
			# ported from Red Hat.
			if [[ $BZIP2 == "bzip2-1.0.5" ]]; then
				cat "$M_PATCHES/bzip2/"bzip2-1.0.4-saneso.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
				cat "$M_PATCHES/bzip2/"bzip2-1.0.4-cflags.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			elif [[ $BZIP2 == "bzip2-1.0.6" ]]; then
				cat "$M_PATCHES/bzip2/"saneso_1.0.6.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
				cat "$M_PATCHES/bzip2/"cflags_1.0.6.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
				cat "$M_PATCHES/bzip2/"bzip2recover_cve_20163189.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			fi
		
			# These patches apply every version since 1.0.4, and were largely ported from the Gentoo repository.
			cat "$M_PATCHES/bzip2/"bzip2-1.0.4-bzip2recover.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			cat "$M_PATCHES/bzip2/"bzip2-1.0.4-makefile-flags.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			cat "$M_PATCHES/bzip2/"bzip2-1.0.4-man-links.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			cat "$M_PATCHES/bzip2/"bzip2-1.0.6-progress.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			cat "$M_PATCHES/bzip2/"bzip2-1.0.4-posix-shell.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error			
			cat "$M_PATCHES/bzip2/"bzip2-1.0.6-mingw.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			cat "$M_PATCHES/bzip2/"bzip2-1.0.6-out-of-tree-build.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			
			# These patches are a mess, but were painfully ported from the Debian bzip2 repository. 
			cat "$M_PATCHES/bzip2/"man_formatting_1.0.6.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			cat "$M_PATCHES/bzip2/"make_modernize_1.0.6.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
			cat "$M_PATCHES/bzip2/"man_path_1.0.6.patch | patch -p1 --verbose &>> "$M_LOGS/bzip2.txt" ; error
		;;
		bzip2-build)
			cd "$M_SOURCES/bzip2"; error
			make CC=gcc AR=ar RANLIB=ranlib CFLAGS='-g3 -fPIC -rdynamic -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic -D_FILE_OFFSET_BITS=64' &>> "$M_LOGS/bzip2.txt" ; error
			make PREFIX="$M_LOCAL" install &>> "$M_LOGS/bzip2.txt" ; error
		;;
		bzip2-check)
			cd "$M_SOURCES/bzip2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/bzip2.txt" ; error
		;;
		bzip2-check-full)
			cd "$M_SOURCES/bzip2"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/bzip2.txt" ; error
		;;
		bzip2-clean)
			cd "$M_SOURCES/bzip2"; error
			make clean &>> "$M_LOGS/bzip2.txt" ; error
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
				cat "$M_PATCHES/dspam/"dspam_status_rename_3.10.x.patch | patch -p3 --verbose &>> "$M_LOGS/dspam.txt"; error
			fi

			cat "$M_PATCHES/dspam/"dspam_version.patch | patch -p1 --verbose &>> "$M_LOGS/dspam.txt"; error
			cat "$M_PATCHES/dspam/"dspam_headers_3.10.2.patch | patch -p1 --verbose &>> "$M_LOGS/dspam.txt"; error
		;;
		dspam-build)
			cd "$M_SOURCES/dspam"; error
			
			if [ ! -f "$M_LDPATH"/mysql/libmysqlclient_r.so ] || [ ! -f "$M_LDPATH"/mysql/libmysqlclient_r.a ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build mysql before dspam.\n"; tput sgr0
				return 3
			fi
			
			export LDFLAGS="-L$M_LDPATH/mysql -L$M_LDPATH -Wl,-rpath,$M_LDPATH/mysql -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			export LD_LIBRARY_PATH="$M_LDPATH"

			./configure --enable-static --with-pic --enable-preferences-extension --enable-virtual-users \
				--with-storage-driver=mysql_drv --disable-trusted-user-security --disable-mysql4-initialization	\
				--with-mysql-includes="$M_LOCAL/include/mysql/" --with-mysql-libraries="$M_LOCAL/lib/mysql/" \
				--prefix="$M_LOCAL" &>> "$M_LOGS/dspam.txt"; error
				
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS; unset LD_LIBRARY_PATH

			make &>> "$M_LOGS/dspam.txt"; error
			make install &>> "$M_LOGS/dspam.txt"; error

			# I don't know my mysql_drv.h doesn't get copied to the include directory, so the quickest workaround is
			# is to copy it over ourselves.
			cp "$M_SOURCES/dspam/src/mysql_drv.h" "$M_LOCAL/include/dspam/"
		;;
		dspam-check)
			cd "$M_SOURCES/dspam"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/dspam.txt"; error
		;;
		dspam-check-full)
			cd "$M_SOURCES/dspam"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
			cat "$M_PATCHES/mysql/"mysql-test-run.patch | patch -p1 --verbose &>> "$M_LOGS/mysql.txt"; error
		;;
		mysql-build)
			cd "$M_SOURCES/mysql"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before mysql.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libcrypto.so ] || [ ! -f "$M_LDPATH"/libcrypto.a ] || [ ! -f "$M_PKGPATH"/libcrypto.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build openssl before mysql.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libssl.so ] || [ ! -f "$M_LDPATH"/libssl.a ] || [ ! -f "$M_PKGPATH"/libssl.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build openssl before mysql.\n"; tput sgr0
				return 3
			fi
			
			# The MySQL code (as of version 5.1.73) includes uses invalid cast operations. As a result the 
			# -fpermissive command line option is required for compilation.
			printf "\n#include <stdlib.h>\n\nint main(int argc, char *argv[]) { return 0; }\n\n" | gcc -o /dev/null -x c++ -fpermissive - &> /dev/null
			if [ $? -eq 0 ]; then
				M_EXTRA="-fpermissive"
			else
				M_EXTRA=""
			fi
			
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O -Wno-narrowing $M_EXTRA $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O -Wno-narrowing $M_EXTRA $M_CPPFLAGS"

			# According to the RHEL build spec, MySQL checks will fail without --with-big-tables,
			./configure --with-pic --enable-thread-safe-client --with-readline --with-charset=latin1 \
				--with-extra-charsets=all --with-plugins=all --with-ssl="$M_SOURCES/openssl" \
				--prefix="$M_LOCAL" &>> "$M_LOGS/mysql.txt"; error

			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS; unset M_EXTRA

			make --jobs=4 &>> "$M_LOGS/mysql.txt"; error
			make install &>> "$M_LOGS/mysql.txt"; error
		;;
		mysql-check)
			cd "$M_SOURCES/mysql"; error
			
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
	
			make --jobs=4 test-fast &>> "$M_LOGS/mysql.txt"; error
		;;
		mysql-check-full)

			cd "$M_SOURCES/mysql"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error

			# The test suite will use this variable to offset the port numbers and prevent clashing.
			export MTR_BUILD_THREAD=86

			# test-full combines the test, test-nr and test-ps targets
			make clean &>> "$M_LOGS/mysql.txt"; error
			make --jobs=4 &>> "$M_LOGS/mysql.txt"; error
			make --jobs=4 test-full &>> "$M_LOGS/mysql.txt"; error

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
		geoip-build)
			cd "$M_SOURCES/geoip"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before geoip.\n"; tput sgr0
				return 3
			fi
			
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			./configure --prefix="$M_LOCAL" &>> "$M_LOGS/geoip.txt"; error

			make &>> "$M_LOGS/geoip.txt"
			if [ $? -ne 0 ]; then
				# Fix for Ubuntu 12.04 LTS
				echo "build failed... retrying with libtoolize" > "$M_LOGS/geoip.txt"
				libtoolize -f &>> "$M_LOGS/geoip.txt"; error
				./configure &>> "$M_LOGS/geoip.txt"; error
				make &>> "$M_LOGS/geoip.txt"; error
			fi
			make install &>> "$M_LOGS/geoip.txt"
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS
		;;
		geoip-check)
			cd "$M_SOURCES/geoip"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/geoip.txt"; error
		;;
		geoip-check-full)
			cd "$M_SOURCES/geoip"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
			elif [[ $CLAMAV =~ "clamav-0.98.7" ]]; then
				
				# Add the shutdown and clean up functions and fix the rar library dynamic loading logic.
				cat "$M_PATCHES/clamav/"shutdown_rarload_0984.patch | patch -p1 --fuzz=100 --verbose &>> "$M_LOGS/clamav.txt"; error

				# Output the version number and not the git commit hash.
				cat "$M_PATCHES/clamav/"version_0984.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error

				# Fix the zlib version check, so that 1.2.10+ doesn't trigger a spurious error.
				cat "$M_PATCHES/clamav/"zlib_check_0992.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error
			else
				# Add the shutdown and clean up functions and fix the rar library dynamic loading logic.
				cat "$M_PATCHES/clamav/"shutdown_rarload_01001.patch | patch -p1 --fuzz=100 --verbose &>> "$M_LOGS/clamav.txt"; error

				# Output the version number and not the git commit hash.
				cat "$M_PATCHES/clamav/"version_0984.patch | patch -p1 --verbose &>> "$M_LOGS/clamav.txt"; error

			fi

			# Fix reference conflict with libpng over the filename png.h.
			PNG_FILES=`grep --files-with-matches --recursive "png\\.h" *`; error
			sed -i -e "s/png\.h/clpng\.h/g" $PNG_FILES; error
			mv libclamav/png.h libclamav/clpng.h; error
			unset PNG_FILES

		;;
		clamav-build)
			cd "$M_SOURCES/clamav"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before clamav.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libpcre2-8.so ] || [ ! -f "$M_LDPATH"/libpcre2-8.a ] || [ ! -f "$M_PKGPATH"/libpcre2-8.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build pcre before pcre.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libxml2.so ] || [ ! -f "$M_LDPATH"/libxml2.a ] || [ ! -f "$M_PKGPATH"/libxml-2.0.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build xml2 before clamav.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libbz2.so ] || [ ! -f "$M_LDPATH"/libbz2.a ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build bzip2 before clamav.\n"; tput sgr0
				return 3
			fi
			
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -DGNU_SOURCE $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -DGNU_SOURCE $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -DGNU_SOURCE $M_CPPFLAGS"

			# --disable-mempool
			./configure  \
				--enable-check --enable-static --enable-shared --disable-llvm --disable-silent-rules \
				--with-openssl="$M_LOCAL" --with-zlib="$M_LOCAL" --with-xml="$M_LOCAL" --with-libcurl="$M_LOCAL" \
				--with-pcre="$M_LOCAL" --with-systemdsystemunitdir="no" \
				--with-libbz2-prefix="$M_LOCAL" --with-libcheck-prefix="$M_LOCAL" \
				--prefix="$M_LOCAL" --exec-prefix="$M_LOCAL" --libdir="$M_LOCAL/lib" &>> "$M_LOGS/clamav.txt"; error

			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS

			if [[ $CLAMAV =~ "clamav-0.9"[7-9]"."[1-9] ]]; then
				# The check3_clamd.sh script will fail if LLVM is disabled.
				# Since we are not currently using clamd, the offending check script is replaced.
				# The exit value 77 indicates the check was skipped.
				printf "\x23\x21/bin/bash\nexit 77\n" > "$M_SOURCES/clamav/unit_tests/check2_clamd.sh"; error
				printf "\x23\x21/bin/bash\nexit 77\n" > "$M_SOURCES/clamav/unit_tests/check3_clamd.sh"; error
				printf "\x23\x21/bin/bash\nexit 77\n" > "$M_SOURCES/clamav/unit_tests/check4_clamd.sh"; error
			fi

			make --jobs=4 &>> "$M_LOGS/clamav.txt"; error
			make install &>> "$M_LOGS/clamav.txt"; error
		;;
		clamav-check)
			cd "$M_SOURCES/clamav"; error

			# Remove read perms for the accdenied file so the test works properly.
			if [[ -e "$M_SOURCES/clamav/unit_tests/accdenied" ]] && [[ ! -f "$M_SOURCES/clamav/unit_tests/accdenied" ]]; then
				chmod --changes u-r "$M_SOURCES/clamav/unit_tests/accdenied" &>> "$M_LOGS/clamav.txt"; error
			fi

			# Increase the default test timeout to 20 minutes.
			export T="1200"

			make check &>> "$M_LOGS/clamav.txt"; error

			unset T

			# Add read perms to accdenied file so it can be checked into the version control repo.
			if [[ -e "$M_SOURCES/clamav/unit_tests/accdenied" ]] && [[ ! -f "$M_SOURCES/clamav/unit_tests/accdenied" ]]; then
				chmod --changes u+r "$M_SOURCES/clamav/unit_tests/accdenied" &>> "$M_LOGS/clamav.txt"; error
			fi
		;;
		clamav-check-full)
			cd "$M_SOURCES/clamav"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error

			# Remove read perms for the accdenied file so the test works properly.
			if [[ -e "$M_SOURCES/clamav/unit_tests/accdenied" ]] && [[ ! -f "$M_SOURCES/clamav/unit_tests/accdenied" ]]; then
				chmod --changes u-r "$M_SOURCES/clamav/unit_tests/accdenied" &>> "$M_LOGS/clamav.txt"; error
			fi

			# Reset the session limits.
			ulimit -f unlimited || ulimit -i 77233 || ulimit -l 64 || ulimit -m unlimited || ulimit -n 1024 || ulimit -q 819200 || ulimit -r 0 || ulimit -s 10240 || ulimit -c 0 || ulimit -d unlimited || ulimit -e 0 || ulimit -t unlimited || ulimit -u 77233 || ulimit -v unlimited || ulimit -x unlimited || ulimit -p 8

			# Increase the default test timeout to 1 hour.
			export T="3600"

			make check VG=1 HG=1 &>> "$M_LOGS/clamav.txt"; error

			unset T

			# Add read permissions to accdenied file so it can be checked into the version control repo.
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

checker() {

	if [[ $1 == "checker-extract" ]]; then
		rm -f "$M_LOGS/checker.txt"; error
	elif [[ $1 != "checker-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/checker.txt"
	fi

	case "$1" in
		checker-extract)
			extract $CHECKER "checker" &>> "$M_LOGS/checker.txt"
		;;
		checker-prep)
			cd "$M_SOURCES/checker"; error
			# The automake requirement is changed from 1.11.2 to 1.11.1 so libcheck will build on CentOS 6. Systems with
			# non-POSIX archivers might require the AM_PROG_AR macro to work, which was added to automake in version 1.11.2.
			# For those systems, this patch might break things in unpredictable ways, assuming that automake is indeed 1.11.1.
			cat "$M_PATCHES/checker/"checker-automake-version.patch | patch -p1 --verbose &>> "$M_LOGS/checker.txt"; error
			# Valgrind will complain about uninitialized bytes if we don't memset the timer variable before using it.
			cat "$M_PATCHES/checker/"checker-timer-memset.patch | patch -p1 --verbose &>> "$M_LOGS/checker.txt"; error

		;;
		checker-build)
			cd "$M_SOURCES/checker"; error
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O0 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O0 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O0 $M_CPPFLAGS"

			autoreconf --install &>> "$M_LOGS/checker.txt"; error
			./configure --disable-subunit --enable-timer-replacement --enable-snprintf-replacement \
				--enable-fork --enable-timeout-tests --prefix="$M_LOCAL" &>> "$M_LOGS/checker.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS

			make &>> "$M_LOGS/checker.txt"; error
			make install &>> "$M_LOGS/checker.txt"; error

		;;
		checker-check)
			cd "$M_SOURCES/checker"; error
			
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			
			make --jobs=4 check &>> "$M_LOGS/checker.txt"; error
		;;
		checker-check-full)
			cd "$M_SOURCES/checker"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/checker.txt"; error
		;;
		checker-clean)
			cd "$M_SOURCES/checker"; error
			make clean &>> "$M_LOGS/checker.txt"; error
		;;
		checker-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/checker.txt"; error
		;;
		checker-log)
			cat "$M_LOGS/checker.txt"; error
		;;
		checker)
			checker "checker-extract"
			checker "checker-prep"
			checker "checker-build"
			checker "checker-check"
		;;
		*)
			printf "\nUnrecognized request.\n $1"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/checker.txt"

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
			cd "$M_SOURCES/openssl"; error
			if [[ $OPENSSL =~ "openssl-1.0.2" ]]; then
				cat "$M_PATCHES/openssl/"1.0.2_curve25519_ed25519.patch | patch -p1 --verbose &>> "$M_LOGS/openssl.txt"; error
			fi
		;;
		openssl-build)
			# OpenSSL does not use environment variables to pickup additional compiler flags
			# The -d param specifies the creation of a debug build
			# See here for reasoning behind openssl-specific linker flags:
			# https://mta.openssl.org/pipermail/openssl-users/2015-April/001053.html
			cd "$M_SOURCES/openssl"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before openssl.\n"; tput sgr0
				return 3
			fi
			
			grep -E "CentOS Linux release 7|Red Hat Enterprise.*release 7" /etc/system-release >& /dev/null
			if [ $? == 0 ]; then
					export CONFIGOPTS='-fno-merge-debug-strings '
			fi
			
			./config -d shared zlib no-asm --prefix="$M_LOCAL" --openssldir="share" --libdir="lib" \
				-I"$M_LOCAL/include/" -O $CONFIGOPTS -g3 -rdynamic -fPIC -DPURIFY -D_FORTIFY_SOURCE=2 \
				-L"$M_LOCAL/lib/" -Wl,-rpath,"$M_LOCAL/lib/" &>> "$M_LOGS/openssl.txt"; error

			make depend &>> "$M_LOGS/openssl.txt"; error
			make &>> "$M_LOGS/openssl.txt"; error
			make install &>> "$M_LOGS/openssl.txt"; error
			
			# Fool autotools checks into thinking this is a normal OpenSSL install (e.g., ClamAV)
			ln -s `pwd` lib
		;;
		openssl-check)
			cd "$M_SOURCES/openssl"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make test &>> "$M_LOGS/openssl.txt"; error
		;;
		openssl-check-full)
			cd "$M_SOURCES/openssl"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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

googtap() {

	if [[ $1 == "googtap-extract" ]]; then
		rm -f "$M_LOGS/googtap.txt"; error
	elif [[ $1 != "googtap-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/googtap.txt"
	fi

	case "$1" in
		googtap-extract)
			extract $GOOGTAP "googtap" &>> "$M_LOGS/googtap.txt"
		;;
		googtap-prep)
			cd "$M_SOURCES/googtap"; error
		;;
		googtap-build)
			cd "$M_SOURCES/googtap"; error
		;;
		googtap-check)
			cd "$M_SOURCES/googtap"; error
		;;
		googtap-check-full)
			cd "$M_SOURCES/googtap"; error
		;;
		googtap-clean)
			cd "$M_SOURCES/googtap"; error
		;;
		googtap-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/googtap.txt"; error
		;;
		googtap-log)
			cat "$M_LOGS/googtap.txt"; error
		;;
		googtap)
			googtap "googtap-extract"
			googtap "googtap-prep"
			googtap "googtap-build"
			googtap "googtap-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/googtap.txt"

	return $?
}

googtest() {

	if [[ $1 == "googtest-extract" ]]; then
		rm -f "$M_LOGS/googtest.txt"; error
	elif [[ $1 != "googtest-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/googtest.txt"
	fi

	case "$1" in
		googtest-extract)
			extract $GOOGTEST "googtest" &>> "$M_LOGS/googtest.txt"
		;;
		googtest-prep)
			cd "$M_SOURCES/googtest"; error
		;;
		googtest-build)
			cd "$M_SOURCES/googtest"; error
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O $M_CXXFLAGS"

			autoreconf --install &>> "$M_LOGS/googtest.txt"; error
			./configure --prefix="$M_LOCAL" &>> "$M_LOGS/googtest.txt"; error
			
			unset CFLAGS; unset CXXFLAGS

			make &>> "$M_LOGS/googtest.txt"; error
		;;
		googtest-check)
			cd "$M_SOURCES/googtest"; error
			make check &>> "$M_LOGS/googtest.txt"; error

			mkdir build && cd build; error
			cmake -Dgtest_build_samples=ON "$M_SOURCES/googtest" &>> "$M_LOGS/googtest.txt"; error
			make &>> "$M_LOGS/googtest.txt"; error
			./sample1_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample2_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample3_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample4_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample5_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample6_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample7_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample8_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample9_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample10_unittest &>> "$M_LOGS/googtest.txt"; error
		;;
		googtest-check-full)
			cd "$M_SOURCES/googtest"; error
			make check &>> "$M_LOGS/googtest.txt"; error

			mkdir build && cd build; error
			cmake -Dgtest_build_samples=ON "$M_SOURCES/googtest" &>> "$M_LOGS/googtest.txt"; error
			make &>> "$M_LOGS/googtest.txt"; error
			./sample1_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample2_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample3_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample4_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample5_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample6_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample7_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample8_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample9_unittest &>> "$M_LOGS/googtest.txt"; error
			./sample10_unittest &>> "$M_LOGS/googtest.txt"; error
		;;
		googtest-clean)
			cd "$M_SOURCES/googtest"; error
			make clean &>> "$M_LOGS/googtest.txt"; error
		;;
		googtest-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/googtest.txt"; error
		;;
		googtest-log)
			cat "$M_LOGS/googtest.txt"; error
		;;
		googtest)
			googtest "googtest-extract"
			googtest "googtest-prep"
			googtest "googtest-build"
			googtest "googtest-check"
		;;
		*)
			printf "\nUnrecognized request.\n"
			exit 2
		;;
	esac

	date +"Finished $1 at %r on %x"
	date +"%n%nFinished $1 at %r on %x%n%n" &>> "$M_LOGS/googtest.txt"

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
		jansson-build)
			cd "$M_SOURCES/jansson"; error
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 -O $M_CPPFLAGS"
			
			./configure --prefix="$M_LOCAL" &>> "$M_LOGS/jansson.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS

			make &>> "$M_LOGS/jansson.txt"; error
			if [ -e "/usr/bin/sphinx-1.0-build" ] || [ -e "/usr/bin/sphinx-build" ]; then
				make html &>> "$M_LOGS/jansson.txt"; error
			fi
			make install &>> "$M_LOGS/jansson.txt"; error
		;;
		jansson-check)
			cd "$M_SOURCES/jansson"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/jansson.txt"; error
		;;
		jansson-check-full)
			cd "$M_SOURCES/jansson"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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
		;;
		freetype-build)
			cd "$M_SOURCES/freetype"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before freetype.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libpng.so ] || [ ! -f "$M_LDPATH"/libpng.a ] || [ ! -f "$M_PKGPATH"/libpng.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build png before freetype.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libbz2.so ] || [ ! -f "$M_LDPATH"/libbz2.a ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build bzip2 before freetype.\n"; tput sgr0
				return 3
			fi
			
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			# We need to override the PNG flags, otherwise the system include files/libraries might be used by mistake.
			export PKG_CONFIG_LIBDIR="$M_PKGPATH"
			export LIBPNG_LIBS=" `pkg-config --libs libpng` "
			export LIBPNG_CFLAGS=" `pkg-config --cflags libpng` "
			unset PKG_CONFIG_LIBDIR
			
			./configure --prefix="$M_LOCAL" --without-harfbuzz &>> "$M_LOGS/freetype.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS; unset LIBPNG_LIBS; unset LIBPNG_CFLAGS

			make --jobs=4 &>> "$M_LOGS/freetype.txt"; error
			make install &>> "$M_LOGS/freetype.txt"; error
		;;
		freetype-check)
			cd "$M_SOURCES/freetype"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/freetype.txt"; error
		;;
		freetype-check-full)
			cd "$M_SOURCES/freetype"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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

utf8proc() {

	if [[ $1 == "utf8proc-extract" ]]; then
		rm -f "$M_LOGS/utf8proc.txt"; error
	elif [[ $1 != "utf8proc-log" ]]; then
		date +"%n%nStarted $1 at %r on %x%n%n" &>> "$M_LOGS/utf8proc.txt"
	fi

	case "$1" in
		utf8proc-extract)
			extract $UTF8PROC "utf8proc" &>> "$M_LOGS/utf8proc.txt"
			tar xzvf "$M_ARCHIVES/$UTF8PROCTEST.tar.gz" --directory="$M_SOURCES/utf8proc/data" &>> "$M_LOGS/utf8proc.txt"; error
		;;
		utf8proc-prep)
			cd "$M_SOURCES/utf8proc"; error
			if [[ $UTF8PROC =~ "utf8proc-1.3.1" ]]; then
				cat "$M_PATCHES/utf8proc/"utf8proc.release.version.patch | patch -p1 --verbose &>> "$M_LOGS/utf8proc.txt"; error
			fi
		;;
		utf8proc-build)
			cd "$M_SOURCES/utf8proc"; error
			
			if [ ! -f "$M_LDPATH"/libcurl.so ] || [ ! -f "$M_LDPATH"/libcurl.a ] || [ ! -f "$M_PKGPATH"/libcurl.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build curl before utf8proc.\n"; tput sgr0
				return 3
			fi
			
			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			
			make prefix="$M_LOCAL" &>> "$M_LOGS/utf8proc.txt"; error
			make prefix="$M_LOCAL" install &>> "$M_LOGS/utf8proc.txt"; error
			
			unset CFLAGS;
		;;
		utf8proc-check)
			cd "$M_SOURCES/utf8proc"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/utf8proc.txt"; error
		;;
		utf8proc-check-full)
			cd "$M_SOURCES/utf8proc"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/utf8proc.txt"; error
		;;
		utf8proc-clean)
			cd "$M_SOURCES/utf8proc"; error
			make clean &>> "$M_LOGS/utf8proc.txt"; error
		;;
		utf8proc-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/utf8proc.txt"; error
		;;
		utf8proc-log)
			cat "$M_LOGS/utf8proc.txt"; error
		;;
		utf8proc)
			utf8proc "utf8proc-extract"
			utf8proc "utf8proc-prep"
			utf8proc "utf8proc-build"
			utf8proc "utf8proc-check"
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
				# Disable portable instruction executables so profiling capable builds work properly. Technically this is only
				# needed so "-pg" can be used, even though the "-pg" flag isn't enabled by default.
				cat "$M_PATCHES/memcached/"configure_1.0.18.patch | patch -p1 --verbose &>> "$M_LOGS/memcached.txt"; error
			fi
		;;
		memcached-build)
			cd "$M_SOURCES/memcached"; error

			# The libmemcached code (as of version 1.0.18) includes comparisons between integers and pointers. This violates 
			# the ISO C++ standard, which means conformant compilers will fail without the -fpermissive command line option.
			printf "\n#include <stdlib.h>\n\nint main(int argc, char *argv[]) { return 0; }\n\n" | gcc -o /dev/null -x c++ -fpermissive - &> /dev/null
			if [ $? -eq 0 ]; then
				M_EXTRA="-fpermissive"
			else
				M_EXTRA=""
			fi

			export CFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_EXTRA $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_EXTRA $M_CPPFLAGS"

			# Recent versions of gcc require libmemcached to be explicitly linked with libm.so and libstdc++.so, and configure
			# doesn't appear to include these libraries automatically.
			export LIBS="-lm -lstdc++"

			# For some reason, the unit tests will fail if this environment variable is configured.
			unset MEMCACHED_SERVERS

			# export GEARMAND_BINARY="/usr/local/sbin/gearmand"
			# export MEMCACHED_BINARY="/usr/local/bin/memcached"
			# --with-memcached="/usr/local/bin/memcached"
			
			# Options used for 1.0.3+
			./configure --disable-silent-rules --disable-dtrace --disable-sasl --enable-static --enable-shared --with-pic \
				--prefix="$M_LOCAL" &>> "$M_LOGS/memcached.txt"; error

			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LIBS; unset M_EXTRA
			# unset GEARMAND_BINARY; unset MEMCACHED_BINARY

			make --jobs=4 &>> "$M_LOGS/memcached.txt"; error
			make install &>> "$M_LOGS/memcached.txt"; error
		;;
		memcached-check)
			cd "$M_SOURCES/memcached"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error

			# For some reason, the unit tests will fail when using this environment variable to find the memcached server.
			unset MEMCACHED_SERVERS

			# Doesn't appear to be necessary anymore...
			#rm -vf /tmp/memcached.pid* &>> "$M_LOGS/memcached.txt"; error

			# For some reason the included version of memcached is being used and causing the unit tests to fail, so overwrite the binary
			#cp /usr/local/bin/memcached "$M_SOURCES/memcached/memcached"

			make check &>> "$M_LOGS/memcached.txt"; error
		;;
		memcached-check-full)
			cd "$M_SOURCES/memcached"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error

			# For some reason, the unit tests will fail when using this environment variable to find the memcached server.
			unset MEMCACHED_SERVERS

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
			cd "$M_SOURCES/tokyocabinet" &>> "$M_LOGS/tokyocabinet.txt"; error
			
			# Adds tctreegetboth() and tcndbgetboth().
			cat "$M_PATCHES/tokyocabinet/"getboth.patch | patch -p1 --verbose &>> "$M_LOGS/tokyocabinet.txt"; error
			cat "$M_PATCHES/tokyocabinet/"tcndbdup.patch | patch -p3 --verbose &>> "$M_LOGS/tokyocabinet.txt"; error
			cat "$M_PATCHES/tokyocabinet/"fileopts.patch | patch -p3 --verbose &>> "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-build)
			cd "$M_SOURCES/tokyocabinet"; error
			
			if [ ! -f "$M_LDPATH"/libz.so ] || [ ! -f "$M_LDPATH"/libz.a ] || [ ! -f "$M_PKGPATH"/zlib.pc ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build zlib before tokyocabinet.\n"; tput sgr0
				return 3
			fi
			
			if [ ! -f "$M_LDPATH"/libbz2.so ] || [ ! -f "$M_LDPATH"/libbz2.a ]; then
				tput sgr0; tput setaf 3; printf "\nPlease build bzip2 before tokyocabinet.\n"; tput sgr0
				return 3
			fi
			
			export LDFLAGS="-L$M_LDPATH -Wl,-rpath,$M_LDPATH $M_LDFLAGS"
			export CFLAGS="$M_SYM_INCLUDES-fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CFLAGS"
			export CXXFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CXXFLAGS"
			export CPPFLAGS="$M_SYM_INCLUDES -fPIC -g3 -rdynamic -D_FORTIFY_SOURCE=2 $M_CPPFLAGS"
			
			./configure --prefix="$M_LOCAL" &>> "$M_LOGS/tokyocabinet.txt"; error
			
			unset CFLAGS; unset CXXFLAGS; unset CPPFLAGS; unset LDFLAGS

			make --jobs=4 &>> "$M_LOGS/tokyocabinet.txt"; error
			make install &>> "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-check)
			cd "$M_SOURCES/tokyocabinet"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
			make check &>> "$M_LOGS/tokyocabinet.txt"; error
		;;
		tokyocabinet-check-full)
			cd "$M_SOURCES/tokyocabinet"; error
			export LD_LIBRARY_PATH="$M_LDPATH"; error
			export PATH="$M_BNPATH:$PATH"; error
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

	case "$1" in
		combine-tail)
			tail --lines=30 --follow=name --retry "$M_LOGS/combine.txt"; error
			exit 0
		;;
		combine-log)
			cat "$M_LOGS/combine.txt"; error
			exit 0
		;;
		combine-static)
			printf "Creating the static archive... "
		;;
		*)
			rm -f "$M_SO" &>> "$M_LOGS/combine.txt"; error
			printf "Creating the shared object... "
		;;
	esac


	if [[ ! -f "$M_SOURCES/gd/src/.libs/libgd.a" || \
		! -f "$M_SOURCES/png/.libs/libpng16.a" || \
		! -f "$M_SOURCES/lzo/src/.libs/liblzo2.a" || \
		! -f "$M_SOURCES/pcre/.libs/libpcre2-8.a" || \
		! -f "$M_SOURCES/jpeg/.libs/libjpeg.a" || \
		! -f "$M_SOURCES/spf2/src/libspf2/.libs/libspf2.a" || \
		! -f "$M_SOURCES/curl/lib/.libs/libcurl.a" || \
		! -f "$M_SOURCES/xml2/.libs/libxml2.a" || \
		! -f "$M_SOURCES/dkim/libopendkim/.libs/libopendkim.a" || \
		! -f "$M_SOURCES/zlib/libz.a" || \
		! -f "$M_SOURCES/bzip2/libbz2.a" || \
		! -f "$M_SOURCES/dspam/src/.libs/libdspam.a" || \
		! -f "$M_SOURCES/mysql/libmysql_r/.libs/libmysqlclient_r.a" || \
		! -f "$M_SOURCES/geoip/libGeoIP/.libs/libGeoIP.a" || \
		! -f "$M_SOURCES/clamav/libclamav/.libs/libclamav.a" || \
		! -f "$M_SOURCES/clamav/libclamav/.libs/libclamunrar.a" || \
		! -f "$M_SOURCES/clamav/libclamav/.libs/libclamunrar_iface.a" || \
		! -f "$M_SOURCES/clamav/libclamav/libmspack-0.5alpha/.libs/libclammspack.a" || \
		! -f "$M_SOURCES/clamav/libltdl/.libs/libltdlc.a" || \
		! -f "$M_SOURCES/openssl/libcrypto.a" || \
		! -f "$M_SOURCES/openssl/libssl.a" || \
		! -f "$M_SOURCES/jansson/src/.libs/libjansson.a" || \
		! -f "$M_SOURCES/freetype/objs/.libs/libfreetype.a" || \
		! -f "$M_SOURCES/utf8proc/libutf8proc.a" || \
		! -f "$M_SOURCES/memcached/libmemcached/.libs/libmemcached.a" || \
		! -f "$M_SOURCES/tokyocabinet/libtokyocabinet.a" ]]; then
		printf " a required dependency is missing. \n\nCreate the dependencies by running \"build.lib.sh all\" and then try again."
		tput sgr0; tput setaf 1
		date +"%n%nFinished $COMMAND failed at %r on %x%n"
		tput sgr0
		exit 1
	fi

	rm -rf "$M_OBJECTS/gd" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/gd" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/gd" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/gd/src/.libs/libgd.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/png" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/png" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/png" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/png/.libs/libpng16.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/lzo" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/lzo" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/lzo" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/lzo/src/.libs/liblzo2.a" &>> "$M_LOGS/combine.txt"; error
	
	rm -rf "$M_OBJECTS/pcre" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/pcre" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/pcre" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/pcre/.libs/libpcre2-8.a" &>> "$M_LOGS/combine.txt"; error
	
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
	ar xv "$M_SOURCES/zlib/libz.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/bzip2" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/bzip2" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/bzip2" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/bzip2/libbz2.a" &>> "$M_LOGS/combine.txt"; error

	rm -rf "$M_OBJECTS/dspam" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/dspam" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/dspam" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/dspam/src/.libs/libdspam.a" &>> "$M_LOGS/combine.txt"; error

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
	ar xv "$M_SOURCES/clamav/libltdl/.libs/libltdlc.a" &>> "$M_LOGS/combine.txt"; error
	
	mkdir "$M_OBJECTS/clamav/unrar" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/clamav/unrar" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/clamav/libclamav/.libs/libclamunrar.a" &>> "$M_LOGS/combine.txt"; error
	
	mkdir "$M_OBJECTS/clamav/unrar_iface" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/clamav/unrar_iface" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/clamav/libclamav/.libs/libclamunrar_iface.a" &>> "$M_LOGS/combine.txt"; error
	
	mkdir "$M_OBJECTS/clamav/mspack" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/clamav/mspack" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/clamav/libclamav/libmspack-0.5alpha/.libs/libclammspack.a" &>> "$M_LOGS/combine.txt"; error

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

	rm -rf "$M_OBJECTS/utf8proc" &>> "$M_LOGS/combine.txt"; error
	mkdir "$M_OBJECTS/utf8proc" &>> "$M_LOGS/combine.txt"; error
	cd "$M_OBJECTS/utf8proc" &>> "$M_LOGS/combine.txt"; error
	ar xv "$M_SOURCES/utf8proc/libutf8proc.a" &>> "$M_LOGS/combine.txt"; error

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

	case "$1" in
		combine-static)
			# Commands for creating a static version of the library.
			rm -f "$M_AR"; error
			ar rs "$M_AR" \
				"$M_OBJECTS"/lzo/*.o "$M_OBJECTS"/zlib/*.o \
				"$M_OBJECTS"/bzip2/*.o "$M_OBJECTS"/pcre/*.o \
				"$M_OBJECTS"/clamav/*.o "$M_OBJECTS"/clamav/unrar/*.o \
				"$M_OBJECTS"/clamav/unrar_iface/*.o "$M_OBJECTS"/clamav/mspack/*.o \
			 	"$M_OBJECTS"/tokyocabinet/*.o "$M_OBJECTS"/crypto/*.o "$M_OBJECTS"/ssl/*.o \
				"$M_OBJECTS"/mysql/*.o "$M_OBJECTS"/xml2/*.o "$M_OBJECTS"/spf2/*.o "$M_OBJECTS"/geoip/*.o \
				"$M_OBJECTS"/curl/*.o "$M_OBJECTS"/memcached/*.o "$M_OBJECTS"/utf8proc/*.o \
				"$M_OBJECTS"/png/*.o "$M_OBJECTS"/jpeg/*.o "$M_OBJECTS"/freetype/*.o "$M_OBJECTS"/gd/*.o \
				"$M_OBJECTS"/dkim/*.o "$M_OBJECTS"/dspam/*.o "$M_OBJECTS"/jansson/*.o &>> "$M_LOGS/combine.txt"; error
			date +"%n%nFinished creating the static archive at %r on %x%n"
		;;
		*)
			rm -f "$M_SO"
			gcc -Wl,-Bsymbolic -g3 -fPIC -rdynamic -shared -o "$M_SO" \
				"$M_OBJECTS"/lzo/*.o "$M_OBJECTS"/zlib/*.o \
				"$M_OBJECTS"/bzip2/*.o "$M_OBJECTS"/pcre/*.o \
				"$M_OBJECTS"/clamav/*.o "$M_OBJECTS"/clamav/unrar/*.o \
				"$M_OBJECTS"/clamav/unrar_iface/*.o "$M_OBJECTS"/clamav/mspack/*.o \
			 	"$M_OBJECTS"/tokyocabinet/*.o "$M_OBJECTS"/crypto/*.o "$M_OBJECTS"/ssl/*.o \
				"$M_OBJECTS"/mysql/*.o "$M_OBJECTS"/xml2/*.o "$M_OBJECTS"/spf2/*.o "$M_OBJECTS"/geoip/*.o \
				"$M_OBJECTS"/curl/*.o "$M_OBJECTS"/memcached/*.o "$M_OBJECTS"/utf8proc/*.o \
				"$M_OBJECTS"/png/*.o "$M_OBJECTS"/jpeg/*.o "$M_OBJECTS"/freetype/*.o "$M_OBJECTS"/gd/*.o \
				"$M_OBJECTS"/dkim/*.o "$M_OBJECTS"/dspam/*.o "$M_OBJECTS"/jansson/*.o \
				-lm -lrt -ldl -lnsl -lresolv -lpthread -lstdc++ &>> "$M_LOGS/combine.txt"; error
			date +"%n%nFinished creating the shared object at %r on %x%n"
		;;
	esac

	if [ "$MASTER" == "yes" ]; then
		echo ""
	fi

	exit 0
	
}

load() {

	if [ "$MASTER" == "yes" ]; then
		echo ""
	fi
	
	printf "Checking the shared object... "
	
	if [ ! -f "$M_SO" ]; then
		printf " the magmad.so file is missing.\n\nCreate the shared object by running \"build.lib.sh all\" and then try again."
		tput sgr0; tput setaf 1
		date +"%n%nFinished $COMMAND failed at %r on %x%n%n"
		tput sgr0
		exit 1
	fi

	mkdir -p "$M_CHECK"; error
	cd "$M_CHECK"; error

	# Copy the current symbols file over.
	cat $M_SYM_FILE | egrep -v $M_SYM_SKIP > magma.open.symbols.h; error
	
	# Create a file with a function that assigns the original symbols to the dynamic version.
	echo "#include \"magma.open.check.h\"" > magma.open.symbols.c; error
	echo "#include \"magma.open.symbols.h\"" >> magma.open.symbols.c; error
	cat magma.open.symbols.h | \
		grep "extern" | \
		sed "s/extern //" | \
		sed "s/;/ = NULL;/g" \
		>> magma.open.symbols.c; error
	echo "const char * symbols_check(void *magma) {" >> magma.open.symbols.c; error
	cat magma.open.symbols.h | \
		grep "extern" | \
		awk -F'(' '{print $2}' | \
		grep -v '^$' | \
		tr -d '*' | sed 's/_d)//' | \
		sed 's/SSL_COMP)/SSL_COMP_get_compression_methods/g' | \
		awk '{ print "if ((*(void **)&(" $1 "_d) = dlsym(magma, \"" $1 "\")) == NULL) return \"" $1 "\";"}' >> magma.open.symbols.c; error
	echo "return NULL;" >> magma.open.symbols.c; error
	echo "}" >> magma.open.symbols.c; error

	## Because GeoIPDBDescription is an array of pointers it doesn't need the leading ampersand.
	#sed -i -e "s/GeoIPDBDescription_d = &GeoIPDBDescription;/GeoIPDBDescription_d = GeoIPDBDescription;/g" magma.open.symbols.c; error

	## This function prototype is prefixed with a macro in parentheses which fools the default parsing rules.
	#sed -i -e "s/SSL_COMP)/SSL_COMP_get_compression_methods/g" magma.open.symbols.c; error

	# The name dkim_getsighdr_d is taken by the OpenDKIM library, so we had to break convention and use dkim_getsighdrx_d.
	sed -i -e "s/\"dkim_getsighdrx\"/\"dkim_getsighdr\"/g" magma.open.symbols.c; error

	# Compile the source files. If an error occurs at compile time it is probably because we have a mismatch somewhere.
	gcc -D_REENTRANT -D_GNU_SOURCE -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE $M_SYM_INCLUDES $M_SO \
		-g3 -rdynamic -Wall -Wextra -Werror -o magma.open.check magma.open.check.c magma.open.symbols.c -ldl; error

	# If errors are generated from invalid symbols, this should print out the specific lines that are invalid.
	if [ $? -ne 0 ]; then

		LNS=`gcc -D_REENTRANT -D_GNU_SOURCE -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE $M_SYM_INCLUDES $M_SO -g3 -rdynamic -Wall -Wextra -Werror \
			-o magma.open.check magma.open.check.c magma.open.symbols.c -ldl 2>&1 | grep "magma.open.symbols.c" | awk -F':' '{ print $2 }' | \
			grep "[0-9*]" | awk '{print $1 ", " }' | sort -gu | uniq | tr -d "\n" | sed "s/, $//g"`

		# Only output the symbol info we found lines to print.
		if [ "$LNS" != "" ]; then

			printf "printing invalid symbols...\n"
			echo "lines = " $LNS
			echo ""

			LNS=`gcc -D_REENTRANT -D_GNU_SOURCE -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE $M_SYM_INCLUDES $M_SO -g3 -rdynamic -Wall -Wextra -Werror \
				-o magma.open.check magma.open.check.c magma.open.symbols.c -ldl 2>&1 | grep "magma.open.symbols.c" | awk -F':' '{ print $2 }' | \
				grep "[0-9*]" | awk '{print $1 "p;" }' | sort -gu | uniq | tr -d "\n"`

			cat magma.open.symbols.c | sed -n "$LNS"; error

		fi

		echo ""
		exit 1
	fi

	# Execute the program to see if the library can be loaded successfully at run time.
	./magma.open.check "$M_SO"; error
	echo ""
	return 0

}

keys() {

	printf "Fixing the permissions for the DIME, DKIM and TLS keys in the magma sandbox...\n"
	if [ "$MASTER" == "yes" ]; then
		echo ""
	fi

	chmod 600 "$M_PROJECT_ROOT/sandbox/etc/tls.localhost.localdomain.pem"; error
	chmod 600 "$M_PROJECT_ROOT/sandbox/etc/dkim.localhost.localdomain.pem"; error
	chmod 600 "$M_PROJECT_ROOT/sandbox/etc/dime.localhost.localdomain.key"; error
	chmod 600 "$M_PROJECT_ROOT/sandbox/etc/dime.localhost.localdomain.signet"; error

	# Tell git to skip checking for changes to these SQL files, but we only do this if git is on the system and the files
	# are stored inside a repo.
	GIT_IS_AVAILABLE=`which git &> /dev/null && git log &> /dev/null && echo 1`
	if [[ "$GIT_IS_AVAILABLE" == "1" ]]; then
		git update-index --skip-worktree "$M_PROJECT_ROOT/sandbox/etc/tls.localhost.localdomain.pem"
		git update-index --skip-worktree "$M_PROJECT_ROOT/sandbox/etc/dkim.localhost.localdomain.pub"
		git update-index --skip-worktree "$M_PROJECT_ROOT/sandbox/etc/dime.localhost.localdomain.key"
		git update-index --skip-worktree "$M_PROJECT_ROOT/sandbox/etc/dime.localhost.localdomain.signet"
	fi
	
}

generate() {

	printf "Generating DIME, DKIM and TLS keys for the magma sandbox...\n"

	# Generate a DKIM private key.
	"$M_LOCAL/bin/"openssl genrsa -out "$M_PROJECT_ROOT/sandbox/etc/dkim.localhost.localdomain.pem" 2048 2>&1 >& /dev/null

	# Derive the DKIM public DNS record based on the generated key.
	"$M_LOCAL/bin/"openssl rsa -in "$M_PROJECT_ROOT/sandbox/etc/dkim.localhost.localdomain.pem" -pubout -outform PEM 2> /dev/null | \
	sed -r "s/-----BEGIN PUBLIC KEY-----$//" | sed -r "s/-----END PUBLIC KEY-----//" | tr -d [:space:] | \
	awk "{ print \"bazinga._domainkey IN TXT \\\"v=DKIM1; k=rsa; p=\" substr(\$1, 1, 208) \"\\\" \\\"\" substr(\$1, 209) \"\\\" ; ----- DKIM bazinga key\" }"  > \
	"$M_PROJECT_ROOT/sandbox/etc/dkim.localhost.localdomain.pub" ; error

	# The TLS private key and a self-signed certificate.
	"$M_LOCAL/bin/"openssl req -x509 -nodes -batch -days 1826 -newkey rsa:4096 \
	-keyout "$M_PROJECT_ROOT/sandbox/etc/tls.localhost.localdomain.pem" \
	-out "$M_PROJECT_ROOT/sandbox/etc/tls.localhost.localdomain.pem" >& /dev/null ; error

	keys
	
}

combo() {

	date +"%nStarting $1 at %r on %x%n" &>> "$M_LOGS/build.txt"

	# If compiling, then proceed sequentially to ensure dependencies are compiled in order.
	if [[ $1 == "build" ]]; then

		# These libraries don't have any pre-requisites or dependencies.
		($M_BUILD "zlib-$1") & ZLIB_PID=$!
		($M_BUILD "bzip2-$1") & BZIP2_PID=$!
		($M_BUILD "jpeg-$1") & JPEG_PID=$!
		($M_BUILD "lzo-$1") & LZO_PID=$!
		($M_BUILD "pcre-$1") & PCRE_PID=$!
		($M_BUILD "spf2-$1") & SPF2_PID=$!
		($M_BUILD "checker-$1") & CHECKER_PID=$!
		($M_BUILD "jansson-$1") & JANSSON_PID=$!
		($M_BUILD "memcached-$1") & MEMCACHED_PID=$!	
		($M_BUILD "googtest-$1") & GOOGTEST_PID=$!
		($M_BUILD "googtap-$1") & GOOGTAP_PID=$!
		
		# These libraries require zlib, but nothing else.
		wait $ZLIB_PID; error
		
		($M_BUILD "png-$1") & PNG_PID=$!
		($M_BUILD "xml2-$1") & XML2_PID=$!
		($M_BUILD "openssl-$1") & OPENSSL_PID=$!
		($M_BUILD "geoip-$1") & GEOIP_PID=$!
		
		# These libraries require zlib (above) and openssl.
		wait $OPENSSL_PID; error
		
		($M_BUILD "curl-$1") & CURL_PID=$!
		($M_BUILD "dkim-$1") & DKIM_PID=$!
		($M_BUILD "mysql-$1") & MYSQL_PID=$!
		
		# These libraries require zlib (above), bzip2, png and jpeg.
		wait $PNG_PID; error
		wait $JPEG_PID; error
		wait $BZIP2_PID; error
		
		($M_BUILD "freetype-$1") & FREETYPE_PID=$!
		($M_BUILD "tokyocabinet-$1") & TOKYOCABINET_PID=$!
		
		# ClamAV requires zlib (above) and bzip (above), pcre and xml2.
		wait $XML2_PID; error
		wait $PCRE_PID; error
		
		($M_BUILD "clamav-$1") & CLAMAV_PID=$!
		
		# The gd library requires zlib (above), png (above), jpeg (above) and freetype.
		wait $FREETYPE_PID; error
		
		($M_BUILD "gd-$1") & GD_PID=$!
		
		# The dspam library requires mysql.
		wait $MYSQL_PID; error
		
		($M_BUILD "dspam-$1") & DSPAM_PID=$!
		
		# The utf8proc library requires curl.
		wait $CURL_PID; error
		
		($M_BUILD "utf8proc-$1") & UTF8PROC_PID=$!
		
		# Wait on any remaining build jobs.
		wait $GD_PID; error
		wait $LZO_PID; error
		wait $SPF2_PID; error
		wait $DKIM_PID; error
		wait $GEOIP_PID; error
		wait $DSPAM_PID; error
		wait $CLAMAV_PID; error
		wait $CHECKER_PID; error
		wait $GOOGTAP_PID; error
		wait $JANSSON_PID; error
		wait $GOOGTEST_PID; error
		wait $UTF8PROC_PID; error
		wait $FREETYPE_PID; error
		wait $MEMCACHED_PID; error
		wait $TOKYOCABINET_PID; error

	else

		# If this isn't a build, then we kick off everything in parallel. 
		($M_BUILD "clamav-$1") & CLAMAV_PID=$!
		($M_BUILD "curl-$1") & CURL_PID=$!
		($M_BUILD "mysql-$1") & MYSQL_PID=$!
		($M_BUILD "gd-$1") & GD_PID=$!
		($M_BUILD "png-$1") & PNG_PID=$!
		($M_BUILD "lzo-$1") & LZO_PID=$!
		($M_BUILD "pcre-$1") & PCRE_PID=$!
		($M_BUILD "jpeg-$1") & JPEG_PID=$!
		($M_BUILD "spf2-$1") & SPF2_PID=$!
		($M_BUILD "xml2-$1") & XML2_PID=$!
		($M_BUILD "dkim-$1") & DKIM_PID=$!
		($M_BUILD "zlib-$1") & ZLIB_PID=$!
		($M_BUILD "bzip2-$1") & BZIP2_PID=$!
		($M_BUILD "dspam-$1") & DSPAM_PID=$!
		($M_BUILD "geoip-$1") & GEOIP_PID=$!
		($M_BUILD "checker-$1") & CHECKER_PID=$!
		($M_BUILD "openssl-$1") & OPENSSL_PID=$!
		($M_BUILD "googtest-$1") & GOOGTEST_PID=$!
		($M_BUILD "googtap-$1") & GOOGTAP_PID=$!
		($M_BUILD "jansson-$1") & JANSSON_PID=$!
		($M_BUILD "freetype-$1") & FREETYPE_PID=$!
		($M_BUILD "utf8proc-$1") & UTF8PROC_PID=$!
		($M_BUILD "memcached-$1") & MEMCACHED_PID=$!
		($M_BUILD "tokyocabinet-$1") & TOKYOCABINET_PID=$!

		wait $GD_PID; error
		wait $PNG_PID; error
		wait $LZO_PID; error
		wait $PCRE_PID; error
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
		wait $CLAMAV_PID; error
		wait $CHECKER_PID; error
		wait $OPENSSL_PID; error
		wait $GOOGTEST_PID; error
		wait $GOOGTAP_PID; error
		wait $JANSSON_PID; error
		wait $UTF8PROC_PID; error
		wait $FREETYPE_PID; error
		wait $MEMCACHED_PID; error
		wait $TOKYOCABINET_PID; error

	fi

	date +"%nFinished $1 at %r on %x%n"
	date +"%nFinished $1 at %r on %x%n" &>> "$M_LOGS/build.txt"
}

follow() {
	# Note that the build.txt and combo.txt log files are intentionally excluded from this list because they don't belong to a bundled package file.
	tail -n 0 -F "$M_LOGS/clamav.txt" "$M_LOGS/curl.txt" "$M_LOGS/dspam.txt" "$M_LOGS/jansson.txt" "$M_LOGS/memcached.txt" "$M_LOGS/openssl.txt" \
		"$M_LOGS/tokyocabinet.txt" "$M_LOGS/zlib.txt" "$M_LOGS/bzip2.txt" "$M_LOGS/dkim.txt" "$M_LOGS/geoip.txt" "$M_LOGS/lzo.txt" \
		"$M_LOGS/mysql.txt" "$M_LOGS/spf2.txt" "$M_LOGS/xml2.txt" "$M_LOGS/gd.txt" "$M_LOGS/png.txt" "$M_LOGS/jpeg.txt" "$M_LOGS/freetype.txt" \
		"$M_LOGS/utf8proc.txt" "$M_LOGS/checker.txt" "$M_LOGS/pcre.txt"
}

log() {
	# Note that the build.txt and combo.txt log files are intentionally excluded from this list because they don't belong to a bundled package file.
	cat "$M_LOGS/clamav.txt" "$M_LOGS/curl.txt" "$M_LOGS/dspam.txt" "$M_LOGS/jansson.txt" "$M_LOGS/memcached.txt" "$M_LOGS/openssl.txt" \
		"$M_LOGS/tokyocabinet.txt" "$M_LOGS/zlib.txt" "$M_LOGS/bzip2.txt" "$M_LOGS/dkim.txt" "$M_LOGS/geoip.txt" "$M_LOGS/lzo.txt" \
		"$M_LOGS/mysql.txt" "$M_LOGS/spf2.txt" "$M_LOGS/xml2.txt" "$M_LOGS/gd.txt" "$M_LOGS/png.txt" "$M_LOGS/jpeg.txt" "$M_LOGS/freetype.txt" \
		"$M_LOGS/utf8proc.txt" "$M_LOGS/checker.txt" "$M_LOGS/pcre.txt"
}

advance() {
	shift
	echo "$@"
}

status() {

	CPU=`iostat cpu | head -4 | tail -2`
	DISK=`iostat -m -x sda sdb sdc vda vdb vdc | tail -n +6 | sed "s/Device:/device:/" | awk '{print $1 "\t  " $6 "\t" $7 "\t" $14}'`

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
		uptime | sed "s/^.*load average://" | awk -F',' '{print "avg-load: " $1 ", " $2 ", " $3 }'
		tput sgr0;  tput sgr 0 1; tput setaf 6; printf "\n# Processor\n\n"; tput sgr0
		echo "$CPU"
		tput sgr0;  tput sgr 0 1; tput setaf 6; printf "\n# Disk\n\n"; tput sgr0
		echo "$DISK"

		# Refresh the stats for the next loop; note that this takes 4 seconds to complete.
		CPU=`iostat cpu 4 2 | tail -5 | head -2`
		DISK=`iostat -m -x sda sdb sdc vda vdb vdc 4 2 | tail -3 | head -2 | sed "s/Device:/device:/"`
		
		# If the status code isn't 0, it means iostat hasn't been installed.
		if [ $? != 0 ]; then
			sleep 1;
		fi
	done
}

all() {
	rm -f "$M_LOGS/build.txt"; error
	date +"%nStarting at %r on %x%n"
	date +"Starting at %r on %x" &>> "$M_LOGS/build.txt"
	$M_BUILD "extract"; error
	$M_BUILD "prep"; error
	$M_BUILD "build"; error
	$M_BUILD "combine"; error
	$M_BUILD "load"; error
	$M_BUILD "keys"; error
	
	# Quick builds don't run the dependency checks, and/or unit tests.
	if [ "$QUICK" != "yes" ]; then
		$M_BUILD "check"
	fi
	
	date +"Finished at %r on %x%n"
	date +"Finished at %r on %x" &>> "$M_LOGS/build.txt"
}

# Store the command for failure messages
COMMAND="$@"

# Parent
if [[ "$PARENT" == "" ]]; then
	MASTER="yes"
	export PARENT="$BASHPID"
fi

# Setup
if [ ! -d "$M_SOURCES" ]; then mkdir "$M_SOURCES"; error; fi
if [ ! -d "$M_OBJECTS" ]; then mkdir "$M_OBJECTS"; error; fi
if [ ! -d "$M_LOCAL" ]; then mkdir "$M_LOCAL"; error; fi
if [ ! -d "$M_LOGS" ]; then mkdir "$M_LOGS"; error; fi

# Aggregations
if [[ $1 == "extract" ]]; then combo "$1"
elif [[ $1 == "prep" ]]; then  combo "$1"
elif [[ $1 == "build" ]]; then combo "$1"
elif [[ $1 == "check" ]]; then combo "$1"
elif [[ $1 == "check-full" ]]; then combo "$1"
elif [[ $1 == "clean" ]]; then combo "$1"

# Libraries
elif [[ $1 =~ "gd" ]]; then gd "$1"
elif [[ $1 =~ "png" ]]; then png "$1"
elif [[ $1 =~ "lzo" ]]; then lzo "$1"
elif [[ $1 =~ "pcre" ]]; then pcre "$1"
elif [[ $1 =~ "jpeg" ]]; then jpeg "$1"
elif [[ $1 =~ "curl" ]]; then curl "$1"
elif [[ $1 =~ "spf2" ]]; then spf2 "$1"
elif [[ $1 =~ "xml2" ]]; then xml2 "$1"
elif [[ $1 =~ "dkim" ]]; then dkim "$1"
elif [[ $1 =~ "zlib" ]]; then zlib "$1"
elif [[ $1 =~ "bzip2" ]]; then bzip2 "$1"
elif [[ $1 =~ "dspam" ]]; then dspam "$1"
elif [[ $1 =~ "mysql" ]]; then mysql "$1"
elif [[ $1 =~ "geoip" ]]; then geoip "$1"
elif [[ $1 =~ "clamav" ]]; then clamav "$1"
elif [[ $1 =~ "checker" ]]; then checker "$1"
elif [[ $1 =~ "openssl" ]]; then openssl "$1"
elif [[ $1 =~ "googtap" ]]; then googtap "$1"
elif [[ $1 =~ "googtest" ]]; then googtest "$1"
elif [[ $1 =~ "jansson" ]]; then jansson "$1"
elif [[ $1 =~ "freetype" ]]; then freetype "$1"
elif [[ $1 =~ "utf8proc" ]]; then utf8proc "$1"
elif [[ $1 =~ "memcached" ]]; then memcached "$1"
elif [[ $1 =~ "tokyocabinet" ]]; then tokyocabinet "$1"

# Globals
elif [[ $1 =~ "combine" ]]; then combine "$1"
elif [[ $1 == "generate" ]]; then generate
elif [[ $1 == "status" ]]; then status
elif [[ $1 == "follow" ]]; then follow
elif [[ $1 == "load" ]]; then load
elif [[ $1 == "keys" ]]; then keys
elif [[ $1 == "log" ]]; then log
elif [[ $1 == "all" ]]; then all

# If follow were called tail it would create a keyword conflict, but we still want to be able to use tail on the command line.
elif [[ $1 == "tail" ]]; then follow

# Catchall
else
	echo ""
	echo " Libraries"
	echo $"  `basename $0` {gd|png|lzo|pcre|jpeg|curl|spf2|xml2|dkim|zlib|bzip2|dspam|mysql|geoip|clamav|checker|openssl|freetype|utf8proc|memcached|tokyocabinet} and/or "
	echo ""
	echo " Stages (which may be combined via a dash with the above)"
	echo $"  `basename $0` {extract|prep|build|check|check-full|clean|tail|log} or "
	echo ""
	echo " Global Commands"
	echo $"  `basename $0` {combine|load|keys|generate|follow|log|status|all}"
	echo ""
	echo " Please specify a library, a stage, a global command or a combination of library and stage."
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
