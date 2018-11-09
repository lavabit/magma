#!/bin/bash

# Name: t.ciphers.sh
# Author: Ladar Levison
#
# Description: Used to periodically test the printf syntax, and ensure strings are being printed properly.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../
MAGMA_DIST=`pwd`


grep --recursive --line-number  "\\%\\*\\.s" $MAGMA_DIST/src/ $MAGMA_DIST/check/

# Flip the result codes, since finding a result fails this test.
if [ $? == 1 ]; then
  exit 0
fi

exit 1



