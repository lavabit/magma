#!/bin/bash

# Name: t.ciphers.sh
# Author: Ladar Levison
#
# Description: Used to periodically test the printf syntax, and ensure strings are being printed properly.

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../
MAGMA_DIST=`pwd`

grep --recursive --line-number  "\\%\\*\\.s" $MAGMA_DIST/src/ $MAGMA_DIST/check/

# Flip the result codes, since finding a result fails this test.
if [ $? == 1 ]; then
  exit 0
fi

exit 1



