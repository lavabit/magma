#!/bin/bash

# Name: magma.helgrind.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad daemon. The daemon is launched using the Valgrind/Helgrind utility.

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../

MAGMA_DIST=`pwd`

# add for suppressions --gen-suppressions=all
# self modifying code --smc-check=[none,stack,all]

valgrind --tool=helgrind \
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
--track-lockorders=yes \
--history-level=full \
--conflict-cache-size=1000000 \
--suppressions=$MAGMA_DIST/sandbox/etc/magma.suppressions \
--suppressions=/usr/lib64/valgrind/default.supp \
$MAGMA_DIST/magmad $MAGMA_DIST/sandbox/etc/magma.sandbox.config

