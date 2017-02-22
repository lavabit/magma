#!/bin/bash

# Name: magma.pprof.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad daemon. The daemon is run atop the gperftools CPU profiler.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

CPUPROFILE="$MAGMA_DIST/magmad.pprof.out" $MAGMA_DIST/magmad.pprof $MAGMA_DIST/sandbox/etc/magma.sandbox.config

if [ -e "$MAGMA_DIST/magmad.pprof.out" ] && [ -f "$MAGMA_DIST/magmad.pprof.out" ]; then
	pprof -gv $MAGMA_DIST/magmad.pprof $MAGMA_DIST/magmad.pprof.out &
fi
