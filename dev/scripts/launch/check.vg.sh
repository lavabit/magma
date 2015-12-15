#!/bin/bash

# Name: check.vg.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad unit tests. The tests are executed atop the Valgrind/Memcheck utility.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

# add for suppressions --gen-suppressions=all
# self modifying code --smc-check=[none,stack,all]
#

valgrind --tool=memcheck \
--trace-children=yes \
--child-silent-after-fork=yes \
--run-libc-freeres=yes \
--demangle=yes \
--num-callers=50 \
--error-limit=no \
--show-below-main=no \
--undef-value-errors=yes \
--track-origins=yes \
--read-var-info=yes \
--smc-check=none \
--fullpath-after= \
--leak-check=yes \
--show-reachable=yes \
--leak-resolution=high \
--workaround-gcc296-bugs=no \
--partial-loads-ok=yes \
--suppressions=$MAGMA_DIST/sandbox/etc/magma.suppressions \
$MAGMA_DIST/magmad.check $MAGMA_DIST/sandbox/etc/magma.sandbox.config
