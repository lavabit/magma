#/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../src/

MAGMA_ROOT=`pwd`

rm -f $MAGMA_ROOT/engine/status/build.h

COMMIT_ID=`git log --format="%H" -n 1 | cut -c33-40`
printf "#define MAGMA_BUILD \"6.0.1-"$COMMIT_ID"\"\n" > $MAGMA_ROOT/engine/status/build.h
date +'#define MAGMA_STAMP "%Y%m%d.%H%M"' >> $MAGMA_ROOT/engine/status/build.h

sleep 1
