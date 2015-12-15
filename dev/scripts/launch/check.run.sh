#!/bin/bash

# Name: check.run.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad unit tests.


LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

$MAGMA_DIST/magmad.check $MAGMA_DIST/sandbox/etc/magma.sandbox.config
