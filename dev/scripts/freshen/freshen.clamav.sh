#/bin/bash

# Name: freshen.clamav.sh
# Author: Ladar Levison
#
# Description: Used to update the Clam Antivirus Signatures using the bundled freshclam
# binary and the sandbox config file. Note the update signatures will be placed in the
# sandbox environment.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

export MAGMA_DIST=`pwd`

export LD_LIBRARY_PATH=$MAGMA_DIST/lib/sources/clamav/libclamav/.libs/ 
lib/sources/clamav/freshclam/.libs/freshclam --user $USER --datadir=sandbox/virus --config-file=sandbox/etc/freshclam.conf
printf "\n\n"
find sandbox/virus/ \( -type f -name "*.cvd" -print0 \) -or \( -type f -name "*.cld" -print0 \) | xargs -0 --replace={} bash -c '$MAGMA_DIST/lib/sources/clamav/sigtool/sigtool --info={} | grep -v Builder | grep -v MD5 | grep -v "Digital signature" ; printf "\n"'


