#/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

cd src/.check/
make clean
make --keep-going --jobs=4 all

cd ../../check/.check/
make clean
make --keep-going --jobs=4 all
