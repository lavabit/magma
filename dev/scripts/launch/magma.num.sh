#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`


ps -ef | egrep "$MAGMA_DIST/src/.debug/magmad|$MAGMA_DIST/check/.check/magmad.check|$MAGMA_DIST/tools/mason/.debug/mason" | grep -v grep
