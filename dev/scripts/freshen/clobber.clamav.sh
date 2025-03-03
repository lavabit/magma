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

# If the TERM environment variable is missing, then tput may trigger a fatal error.
if [[ -n "$TERM" ]] && [[ "$TERM" != "dumb" ]]; then
  export TPUT="tput"
else
  export TPUT="tput -Tvt100"
fi

# If the sandbox contains a CA bundle file, override the path to the CA file used by Freshclam.
cp -f  "$MAGMA_DIST/dev/virus/main.cvd" "$MAGMA_DIST/sandbox/virus/main.cvd"
cp -f  "$MAGMA_DIST/dev/virus/daily.cvd" "$MAGMA_DIST/sandbox/virus/daily.cvd"
cp -f  "$MAGMA_DIST/dev/virus/bytecode.cvd" "$MAGMA_DIST/sandbox/virus/bytecode.cvd"

# Done.
[[ -t 0 ]] && ${TPUT} setaf 2 || true ; printf "Done.\n" ; [[ -t 0 ]] && ${TPUT} sgr0 || true
