#!/bin/bash

# Name: freshen.clamav.sh
# Author: Ladar Levison
#
# Description: Used to update the Clam Antivirus Signatures using the bundled freshclam
# binary and the sandbox config file. Note the update signatures will be placed in the
# sandbox environment.

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`
fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../

export MAGMA_DIST=`pwd`
export LD_LIBRARY_PATH=$MAGMA_DIST/lib/sources/clamav/libclamav/.libs/:$MAGMA_DIST/lib/sources/clamav/libfreshclam/.libs/

# If the sandbox contains a CA bundle file, override the path to the CA file used by Freshclam.
if [ -f $MAGMA_DIST/sandbox/etc/ca-bundle.crt ]; then
  lib/sources/clamav/freshclam/.libs/freshclam --user $USER --ca=$MAGMA_DIST/sandbox/etc/ca-bundle.crt --datadir=$MAGMA_DIST/sandbox/virus --config-file=$MAGMA_DIST/sandbox/etc/freshclam.conf
else
  lib/sources/clamav/freshclam/.libs/freshclam --user $USER --datadir=$MAGMA_DIST/sandbox/virus --config-file=$MAGMA_DIST/sandbox/etc/freshclam.conf
fi

# Print statistics for all of the ClamAV database files.
printf "\n\n"
find sandbox/virus/ \( -type f -name "*.cvd" -print0 \) -or \( -type f -name "*.cld" -print0 \) | xargs -0 --replace={} bash -c '$MAGMA_DIST/lib/sources/clamav/sigtool/sigtool --info={} | grep -v Builder | grep -v MD5 | grep -v "Digital signature" ; printf "\n"'


