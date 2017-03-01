#!/bin/bash

# Name: testde.sh
# Author: Ladar Levison
#
# Description: Used to run the decompression tester against a particular message.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

export MAGMA_LIBRARY=$MAGMA_DIST/magmad.so
export MAGMA_STORAGE=$MAGMA_DIST/sandbox/storage/local

$MAGMA_DIST/dev/tools/testde/testde $1
