#!/bin/bash

# Name: magma.loop.sh
# Author: Ladar Levison
#
# Description: Used for continuously launching the magma daemon process. Useful when the process continuously dies.

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`
fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../

MAGMA_DIST=`pwd`

while true; do
	
	PID=`ps -ef | egrep "$MAGMA_DIST/magmad|$MAGMA_DIST/magmad.check|/home/magma/magmad" | grep -v grep | awk -F' ' '{ print $2}'`
	if [ "$PID" = '' ]; then
		($MAGMA_DIST/magmad $MAGMA_DIST/sandbox/etc/magma.sandbox.config) &
	fi
	
	sleep 60

done
