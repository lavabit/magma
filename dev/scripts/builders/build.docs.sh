#/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

doxygen $MAGMA_DIST/scripts/builders/build.doxyfile
