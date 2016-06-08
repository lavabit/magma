#!/bin/bash

# Name: runner.sh
# Author: Ladar Levison
#
# Description: Used to run the SMTP test client against the current working directory.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

$BASE/../../../dev/tools/runner/runner $1
