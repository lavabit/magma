#!/bin/bash

# Name: check.helgrind.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad unit tests. The tests are executed atop the Valgrind/Massif utility.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

# add for suppressions --gen-suppressions=all
# self modifying code --smc-check=[none,stack,all]
#

valgrind --tool=massif \
--log-fd=1 \
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
--heap=yes \
--heap-admin=8 \
--stacks=no \
--depth=30 \
--threshold=1.0 \
--peak-inaccuracy=1.0 \
--time-unit=i \
--detailed-freq=10 \
--max-snapshots=100 \
--massif-out-file=$MAGMA_DIST/massif.out.%p \
--suppressions=/usr/lib64/valgrind/default.supp \
--suppressions=$MAGMA_DIST/sandbox/etc/magma.suppressions \
$MAGMA_DIST/magmad.check $MAGMA_DIST/sandbox/etc/magma.sandbox.config
