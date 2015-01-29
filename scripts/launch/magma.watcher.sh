#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`

while true; do
	
	# Watch for resource changes.
	/usr/bin/inotifywait --quiet --recursive --event modify $MAGMA_DIST/res/pages  $MAGMA_DIST/res/templates
	
	# Resources modified, so send the signal.
	PID=`ps -ef | egrep "$MAGMA_DIST/src/.debug/magmad|$MAGMA_DIST/check/.check/magmad.check|$MAGMA_DIST/tools/mason/.debug/mason|/home/src/magmad" | grep -v grep | awk -F' ' '{ print $2}'`
	if [ "$PID" != '' ]; then
		PID=${PID//
/ }
		kill -SIGHUP $PID
		echo "magma process ids $PID given the refresh signal"
	fi
	
done
