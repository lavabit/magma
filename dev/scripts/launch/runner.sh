#!/bin/bash

# Name: runner.sh
# Author: Ladar Levison
#
# Description: Used to run the SMTP test client against the current working directory.

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

$BASE/../../../dev/tools/runner/runner $1
