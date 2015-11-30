#!/bin/bash


LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

export MAGMA_DIST=`pwd`

export LD_LIBRARY_PATH=$MAGMA_DIST/lib/sources/clamav/libclamav/.libs/ 
lib/sources/clamav/freshclam/.libs/freshclam --user $USER --datadir=res/virus --config-file=res/config/freshclam.conf
printf "\n\n"
find res/virus/ \( -type f -name "*.cvd" -print0 \) -or \( -type f -name "*.cld" -print0 \) | xargs -0 --replace={} bash -c '$MAGMA_DIST/lib/sources/clamav/sigtool/sigtool --info={} | grep -v Builder | grep -v MD5 | grep -v "Digital signature" ; printf "\n"'


