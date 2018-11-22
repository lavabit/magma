#!/bin/bash

# Name: magma.run.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad daemon. 

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

$MAGMA_DIST/magmad $MAGMA_DIST/sandbox/etc/magma.sandbox.config
