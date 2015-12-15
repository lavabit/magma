#/bin/bash

# Name: magma.num.sh
# Author: Ladar Levison
#
# Description: Used for quickly listing the number of magma processes currently running.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`


ps -ef | egrep "$MAGMA_DIST/magmad|$MAGMA_DIST/magmad.check|$MAGMA_DIST/dev/tools/mason/.debug/mason" | grep -v grep
