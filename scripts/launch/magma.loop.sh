#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`

while true; do
	
	PID=`ps -ef | egrep "$MAGMA_DIST/src/.debug/magmad|$MAGMA_DIST/check/.check/magmad.check|/home/magma/magmad" | grep -v grep | awk -F' ' '{ print $2}'`
	if [ "$PID" = '' ]; then
		($MAGMA_DIST/src/.debug/magmad $MAGMA_DIST/res/config/magma.sandbox.config) &
	fi
	
	sleep 60

done
