#/bin/bash

# Name: runner.sh
# Author: Ladar Levison
#
# Description: Used to run the SMTP test client against the current working directory.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

$MAGMA_DIST/dev/tools/runner/.debug/runner $1
