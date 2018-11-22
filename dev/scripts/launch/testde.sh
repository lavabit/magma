#!/bin/bash

# Name: testde.sh
# Author: Ladar Levison
#
# Description: Used to run the decompression tester against a particular message.

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`
fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../

MAGMA_DIST=`pwd`

export MAGMA_LIBRARY=$MAGMA_DIST/magmad.so
export MAGMA_STORAGE=$MAGMA_DIST/sandbox/storage/local

$MAGMA_DIST/dev/tools/testde/testde $1
