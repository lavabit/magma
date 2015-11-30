#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

# add for suppressions --gen-suppressions=all
# self modifying code --smc-check=[none,stack,all]
#

valgrind --tool=cachegrind --trace-children=yes --child-silent-after-fork=yes --run-libc-freeres=yes --demangle=yes --num-callers=50 --error-limit=no \
--show-below-main=no --max-stackframe=20000000 --dsymutil=yes --fullpath-after= --read-var-info=yes \
--cache-sim=yes --branch-sim=no --cachegrind-out-file=$HOME/cachegrind.out.%p \
--suppressions=$MAGMA_DIST/res/config/magma.supp --suppressions=/usr/lib64/valgrind/default.supp \
$MAGMA_DIST/check/.check/magmad.check $MAGMA_DIST/res/config/magma.sandbox.config
