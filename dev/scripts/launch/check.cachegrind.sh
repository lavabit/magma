#!/bin/bash

# Name: check.cachegrind.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad unit tests. The tests are executed atop the Valgrind/Cachegrind utility.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

# add for suppressions --gen-suppressions=all
# self modifying code --smc-check=[none,stack,all]
#

valgrind --tool=cachegrind \
--trace-children=yes \
--child-silent-after-fork=yes \
--run-libc-freeres=yes \
--demangle=yes \
--num-callers=50 \
--error-limit=no \
--show-below-main=no \
--max-stackframe=20000000 \
--dsymutil=yes \
--fullpath-after= \
--read-var-info=yes \
--cache-sim=yes \
--branch-sim=no \
--cachegrind-out-file=$MAGMA_DIST/cachegrind.out.%p \
--suppressions=/usr/lib64/valgrind/default.supp \
--suppressions=$MAGMA_DIST/sandbox/etc/magma.suppressions \ 
$MAGMA_DIST/magmad.check $MAGMA_DIST/sandbox/etc/magma.sandbox.config
