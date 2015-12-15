#!/bin/bash

# Name: magma.loop.sh
# Author: Ladar Levison
#
# Description: Used for continuously launching the magma daemon process. Useful when the process continuously dies.


LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

while true; do
	
	PID=`ps -ef | egrep "$MAGMA_DIST/magmad|$MAGMA_DIST/magmad.check|/home/magma/magmad" | grep -v grep | awk -F' ' '{ print $2}'`
	if [ "$PID" = '' ]; then
		($MAGMA_DIST/magmad $MAGMA_DIST/sandbox/etc/magma.sandbox.config) &
	fi
	
	sleep 60

done
