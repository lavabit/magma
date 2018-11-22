#!/bin/bash

# Name: magma.watcher.sh
# Author: Ladar Levison
#
# Description: Used to watch the static web resources folders and signal the magma process whenever 
# any of the files are updated. The HUP signal will trigger a reload of the data from disk.

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

# Check and make sure the inotifywait command line utility has been installed.
which inotifywait &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The inotifywait utility isn't available. It needs to be installed for this script to work."; tput sgr0
	exit 1
fi

while true; do
	
	# Watch for resource changes.
	inotifywait --quiet --recursive --event modify $MAGMA_DIST/src/  $MAGMA_DIST/check/
		
	# Check whether make or gcc are already running before attempting to build the project.
	PID=`pidof make gcc`       
	
	if [ ! -z "$PID" ]; then
		tput setaf 1; tput bold; echo "A compiler is already running..."; tput sgr0
	else
		make --silent --keep-going --jobs=1 all > /dev/null
	fi
done
