#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

$MAGMA_DIST/magmad $MAGMA_DIST/res/config/magma.sandbox.config
