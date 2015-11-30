#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

# add for suppressions --gen-suppressions=all
# self modifying code --smc-check=[none,stack,all]

valgrind --tool=helgrind --trace-children=yes --child-silent-after-fork=yes --run-libc-freeres=yes \
--demangle=yes --num-callers=50 --error-limit=no --show-below-main=no --max-stackframe=20000000 --dsymutil=yes --fullpath-after= --read-var-info=yes \
--suppressions=$MAGMA_DIST/res/config/magma.supp --suppressions=/usr/lib64/valgrind/default.supp \
--track-lockorders=yes --history-level=full --conflict-cache-size=1000000 \
$MAGMA_DIST/src/.debug/magmad $MAGMA_DIST/res/config/magma.sandbox.config

