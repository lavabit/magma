#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`

# add for suppressions --gen-suppressions=all
# self modifying code --smc-check=[none,stack,all]

valgrind --tool=memcheck --trace-children=yes --child-silent-after-fork=yes --run-libc-freeres=yes --demangle=yes --num-callers=50 \
--error-limit=no --show-below-main=no --undef-value-errors=yes --track-origins=yes --read-var-info=yes --smc-check=none --fullpath-after= \
--leak-check=yes --show-reachable=yes --leak-resolution=high --workaround-gcc296-bugs=no --partial-loads-ok=yes \
$MAGMA_DIST/src/.debug/magmad $MAGMA_DIST/res/config/magma.sandbox.config

##--suppressions=$MAGMA_DIST/res/config/magmad.supp
##--suppressions=/usr/local/lib64/valgrind/default.supp

#valgrind --tool=massif --heap=yes --stacks=yes --alloc-fn=mm_alloc \
#--trace-children=yes --child-silent-after-fork=yes --run-libc-freeres=yes --demangle=yes --num-callers=50 \
#--error-limit=no --show-below-main=no --max-stackframe=20000000 --read-var-info=yes \
#--suppressions=$MAGMA_DIST/res/config/magmad.supp \
#$MAGMA_DIST/src/.debug/magmad $MAGMA_DIST/res/config/magma.sandbox.config


#valgrind --tool=exp-ptrcheck --enable-sg-checks=yes  \
#--trace-children=yes --child-silent-after-fork=yes --run-libc-freeres=yes --demangle=yes --num-callers=50 --error-limit=no \
#--smc-check=all --trace-children=yes --child-silent-after-fork=yes --run-libc-freeres=yes --demangle=yes --num-callers=50 \
#--error-limit=no --show-below-main=no --max-stackframe=20000000 --read-var-info=yes \
#--suppressions=$MAGMA_DIST/res/config/magmad.supp \
#$MAGMA_DIST/src/.debug/magmad $MAGMA_DIST/res/config/magma.sandbox.config
