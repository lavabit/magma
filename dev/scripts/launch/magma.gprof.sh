#!/bin/bash

# Name: check.gprof.sh
# Author: Ladar Levison
#
# Description: Used for profiling the magma daemon. The daemon is executed using the gprof profiler.

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

if [ ! -e "$MAGMA_DIST/magmad.gprof" ] || [ ! -f "$MAGMA_DIST/magmad.gprof" ]; then
	tput setaf 1; tput bold; echo "The magmad.gprof binary doesn't exist. Please compile it first."; tput sgr0
	exit 1
fi

GMON_OUT_PREFIX="$MAGMA_DIST/magmad.check.gprof.out" $MAGMA_DIST/magmad.check.gprof $MAGMA_DIST/sandbox/etc/magma.sandbox.config & MAGMA_PID=$!
wait $MAGMA_PID

if [ -e "$MAGMA_DIST/magmad.check.gprof.out.$MAGMA_PID" ] && [ -f "$MAGMA_DIST/magmad.check.gprof.out.$MAGMA_PID" ]; then
	gprof $MAGMA_DIST/magmad.check.gprof $MAGMA_DIST/magmad.check.gprof.out.$MAGMA_PID
fi
