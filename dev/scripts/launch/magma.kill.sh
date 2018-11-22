#!/bin/bash

# Name: magma.kill.sh
# Author: Ladar Levison
#
# Description: Used for signaling the magma daemon that it is time to shutdown.

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

PID=`ps -ef | egrep "$MAGMA_DIST/magmad|$MAGMA_DIST/magmad.check|$MAGMA_DIST/dev/tools/mason/.debug/mason|/home/magma/magmad" | grep -v grep | awk -F' ' '{ print $2}'`
if [ "$PID" = '' ]; then
	echo "no magma instances to kill"
else
	PID=${PID//
/ }
	kill $PID
	echo "magma process ids $PID signaled"
#	killall --quiet --wait --regexp "$HOME/Lavabit/magma/.debug/magmad|$HOME/Lavabit/magma.check/.check/magmad.check|$HOME/Lavabit/magma.tools/mason/.debug/mason"

fi
