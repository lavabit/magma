#!/bin/bash

# Name: magma.run.sh
# Author: Ladar Levison
#
# Description: Used for launching the magmad daemon. 

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

$MAGMA_DIST/magmad $MAGMA_DIST/sandbox/etc/magma.sandbox.config
