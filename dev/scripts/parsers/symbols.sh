#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

if [ "$*" == "" ]; then
	INPUT=`cat $MAGMA_DIST/src/providers/symbols.h | egrep ".*(\*.*\_d).*"`
else
	INPUT=`echo "$*"`
fi

echo "$INPUT" | sed -e "s/\\(.*\\)(\*\\(.*\\)\_d)\\(.*\\)/\\2/g" | awk '{ print "{ .name = \"" $1 "\", .pointer = (void *)&" $1 "_d }," }' | \
	sed -e "s/\"dkim_getsighdrx\"/\"dkim_getsighdr\"/g"
