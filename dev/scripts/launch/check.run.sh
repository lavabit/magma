#!/bin/bash

# Name: check.run.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad unit tests.

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

if [ $# == 1 ]; then
$MAGMA_DIST/magmad.check --check $1 $MAGMA_DIST/sandbox/etc/magma.sandbox.config
else
$MAGMA_DIST/magmad.check $MAGMA_DIST/sandbox/etc/magma.sandbox.config
fi
