#!/bin/bash

# Name: check.callgrind.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad unit tests. The tests are executed atop the Valgrind/Callgrind utility.

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
#

#### Still not tracking forks/threads properly!!

valgrind --tool=callgrind \
--log-fd=1 \
--trace-children=yes \
--demangle=yes \
--num-callers=50 \
--error-limit=no \
--show-below-main=no \
--max-stackframe=20000000 \
--dsymutil=yes \
--fullpath-after= \
--read-var-info=yes \
--dump-line=yes \
--compress-strings=yes \
--compress-pos=yes \
--collect-systime=yes \
--collect-bus=yes \
--zero-before=main \
--dump-instr=yes \
--trace-jump=yes \
--collect-atstart=yes \
--instr-atstart=yes  \
--callgrind-out-file=$MAGMA_DIST/callgrind.out \
--suppressions=/usr/lib64/valgrind/default.supp \
--suppressions=$MAGMA_DIST/sandbox/etc/magma.suppressions \
$MAGMA_DIST/magmad $MAGMA_DIST/sandbox/etc/magma.sandbox.config 

export VPID=$!
export VLOG=$MAGMA_DIST/callgrind.out

wait $VPID

if [ -e "$VLOG" ] && [ -f "$VLOG" ]; then 
	#qcachegrind $VLOG &
	kcachegrind $VLOG &
fi
