#/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

cd src/.debug/
make clean
make --keep-going --jobs=4 all
