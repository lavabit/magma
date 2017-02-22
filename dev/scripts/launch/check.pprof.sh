#!/bin/bash

# Name: check.pprof.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad unit tests. The tests are executed using the gperftools CPU profiler.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

CPUPROFILE="$MAGMA_DIST/magmad.check.pprof.out" $MAGMA_DIST/magmad.check.pprof $MAGMA_DIST/sandbox/etc/magma.sandbox.config

if [ -e "$MAGMA_DIST/magmad.check.pprof.out" ] && [ -f "$MAGMA_DIST/magmad.check.pprof.out" ]; then
	pprof -gv $MAGMA_DIST/magmad.check.pprof $MAGMA_DIST/magmad.check.pprof.out &
fi
