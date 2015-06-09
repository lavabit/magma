# Note that this has not been extensively tested with various autoconf
# versions.  TODO - determine the minimum autoconf version we will support
AC_PREREQ([2.68])

AC_INIT([magma], [0.1])

# Look for macros in the m4 directory
AC_CONFIG_MACRO_DIR([m4])

# Try to find libcheck using pkg-config
PKG_CHECK_MODULES([CHECK], [check >= 0.9])

# Sanity check to make sure the specified srcdir contains magma.h
AC_CONFIG_SRCDIR([src/magma.h])

# Auxiliary autotools files go here
AC_CONFIG_AUX_DIR([build-aux])

# Note that this has not been extensively tested with various automake
# versions.  TODO - determine the minimum automake version we will support.
#
# The foreign option prevents automake from requiring the GNU-specific project
# files like AUTHORS, COPYING, etc.
#
# The subdir-objects option allows source files in subdirectories.
#
# nostdinc disables including srcdir (-I.)
AM_INIT_AUTOMAKE([foreign subdir-objects nostdinc])

# Instruct configure to create Makefile using Makefile.am
AC_CONFIG_FILES([Makefile])

# Find and probe C compiler (use "current" standard)
AC_PROG_CC_STDC

# Find ranlib
AC_PROG_RANLIB

# Define _GNU_SOURCE
AC_GNU_SOURCE

# Handle pthreads (See
# http://stackoverflow.com/questions/17055279/autotools-for-pthreads-not-setting-correct-linker-flags)
AX_PTHREAD([
			AC_DEFINE([HAVE_PTHREAD],[1], [Define if you have pthreads.])
			LIBS="$PTHREAD_LIBS $LIBS"
			CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
			LDFLAGS="$LDFLAGS $PTHREAD_CFLAGS"
			CC="$PTHREAD_CC"], [])

AC_DEFINE(HAVE_NS_TYPE)

AC_OUTPUT
