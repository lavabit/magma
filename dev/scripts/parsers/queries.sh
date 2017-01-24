#!/bin/bash

# Name: queries.sh
# Author: Ladar Levison
#
# Description: Used to quickly output the second copy of the queries file.

# Queries + Stmts Init

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_DIST=`pwd`

QUERIES=`printf "#define QUERIES_INIT\t\t\t\t\t"; cat $MAGMA_DIST/src/queries.h | grep "\#define" | egrep -v "MAGMA_DATA_QUERIES_H|INIT" | grep -v "//" | awk -F' ' '{ print $2 }' | egrep "^[A-Z_]+$" | awk -F' ' '{ print "\t\t\t\t\t\t\t\t\t\t\t" $1 ", \\\\" }' | head --bytes=-4 | tail --bytes=+11`
STMTS=`printf "\n\n#define STMTS_INIT\t\t\t\t\t"; cat $MAGMA_DIST/src/queries.h | grep "\#define" | egrep -v "MAGMA_DATA_QUERIES_H|INIT" | grep -v "//" | awk -F' ' '{ print $2 }' | egrep "^[A-Z_]+$" | awk -F' ' '{ print "\t\t\t\t\t\t\t\t\t\t\t**" tolower($1) ", \\\\" }' | head --bytes=-4 | tail --bytes=+10`

#printf "$QUERIES$STMTS\n" | xclip -in -selection clipboard
printf "$QUERIES$STMTS\n" | clipit
printf "$QUERIES\n\n$STMTS"
