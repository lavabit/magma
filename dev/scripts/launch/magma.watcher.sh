#!/bin/bash

# Name: magma.watcher.sh
# Author: Ladar Levison
#
# Description: Used to watch the static web resources folders and signal the magma process whenever 
# any of the files are updated. The HUP signal will trigger a reload of the data from disk.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

# Check and make sure the inotifywait command line utility has been installed.
which inotifywait &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The inotifywait utility isn't available. It needs to be installed for this script to work."; tput sgr0
	exit 1
fi

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`       

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "Magma process isn't running."; tput sgr0
	exit 2
fi

while true; do
	
	# Watch for resource changes.
	inotifywait --quiet --recursive --event modify $MAGMA_DIST/res/pages  $MAGMA_DIST/res/templates
	
	# Resources modified, so send the signal.
	PID=`ps -ef | egrep "$MAGMA_DIST/magmad|$MAGMA_DIST/magmad.check|$MAGMA_DIST/tools/mason/.debug/mason|/home/src/magmad" | grep -v grep | awk -F' ' '{ print $2}'`
	if [ "$PID" != '' ]; then
		PID=${PID//
/ }
		kill -SIGHUP $PID
		echo "magma process ids $PID given the refresh signal"
	fi
	
done
