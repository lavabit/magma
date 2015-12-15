#/bin/bash

# Name: magma.kill.sh
# Author: Ladar Levison
#
# Description: Used for signaling the magma daemon that it is time to shutdown.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

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
