#!/bin/bash

# Name: check.gprof.sh
# Author: Ladar Levison
#
# Description: Used for profiling the magmad unit tests. The tests are executed using the gprof profiler.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

if [ ! -e "$MAGMA_DIST/magmad.check.gprof" ] || [ ! -f "$MAGMA_DIST/magmad.check.gprof" ]; then
	tput setaf 1; tput bold; echo "The magmad.check.gprof binary doesn't exist. Please compile it first."; tput sgr0
	exit 1
fi
GMON_OUT_PREFIX="$MAGMA_DIST/magmad.check.gprof.out" $MAGMA_DIST/magmad.check.gprof $MAGMA_DIST/sandbox/etc/magma.sandbox.config & MAGMA_PID=$!
wait $MAGMA_PID

if [ -e "$MAGMA_DIST/magmad.check.gprof.out.$MAGMA_PID" ] && [ -f "$MAGMA_DIST/magmad.check.gprof.out.$MAGMA_PID" ]; then
	gprof $MAGMA_DIST/magmad.check.gprof $MAGMA_DIST/magmad.check.gprof.out.$MAGMA_PID
fi
