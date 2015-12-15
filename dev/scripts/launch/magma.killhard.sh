#!/bin/bash

# Name: magma.killhard.sh
# Author: Ladar Levison
#
# Description: Used for killing the magma daemon when it takes too long to shutdown normally.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

PID=`ps -ef | egrep "$MAGMA_DIST/magmad|$MAGMA_DIST/magmad.check|$MAGMA_DIST/dev/tools/mason/.debug/mason|/home/magma/magmad" | grep -v grep | awk -F' ' '{ print $2}'`
if [ "$PID" = '' ]; then
	echo "no magma instances to kill"
else
	kill -9 $PID
	PID=${PID//
/ }
	if [ $? -eq 0 ]; then
		sleep 1
	fi
	echo "magma process ids $PID killed"
fi
