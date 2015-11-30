#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`

export MAGMA_LIBRARY=$MAGMA_DIST/lib/magmad.so
export MAGMA_STORAGE=$MAGMA_DIST/res/servers/local

$MAGMA_DIST/tools/testde/.debug/testde $1
