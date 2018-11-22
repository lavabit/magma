#!/bin/bash

# Name: check.pprof.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad unit tests. The tests are executed using the gperftools CPU profiler.

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


if [ -e "$MAGMA_DIST/magmad.check.pprof.out" ] && [ -f "$MAGMA_DIST/magmad.check.pprof.out" ]; then
	rm --force "$MAGMA_DIST/magmad.check.pprof.out"
fi

CPUPROFILE="$MAGMA_DIST/magmad.check.pprof.out" $MAGMA_DIST/magmad.check.pprof $MAGMA_DIST/sandbox/etc/magma.sandbox.config

if [ -e "$MAGMA_DIST/magmad.check.pprof.out" ] && [ -f "$MAGMA_DIST/magmad.check.pprof.out" ]; then
	pprof -gv $MAGMA_DIST/magmad.check.pprof $MAGMA_DIST/magmad.check.pprof.out &
fi
