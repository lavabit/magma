#!/bin/bash

# Name: location.sh
# Author: Ladar Levison
#
# Description: Used to quickly update the file location inside the doxygen header.


# Build file location
PWD=`pwd`
FILE="$PWD/$1" 
BASE=`basename $1`

SUFFIX=`echo $BASE | awk -F'.' '{ print $NF }'`
if [ "$SUFFIX" != "c" ] && [ "$SUFFIX" != "h" ]; then
	tput setaf 3;  echo "location updates can only be performed on code files... not $SUFFIX ... $1" 1>&2; tput sgr0
	exit 1
fi

# Ensure the heading has a leading space.
sed -i -e "1 s/\/\*\*/\n&/g" $FILE

# Tweak the actual file path so "/magma/check/" is simply "check" and "/magma/src" is simply "/magma/" inside the doxygen comment headers.
PRETTY=`echo $1 | sed "s/\/magma\/check\//\/check\//g" | sed "s/\/magma\/src\//\/magma\//g"`

# See if we even need to update the file
grep "^ \* @file $PRETTY$" $FILE &> /dev/null
if [ "$?" == "0" ]; then
	echo "$1 is already correct..."
	exit 1
fi

# String replacement
cat $FILE | sed "s|^.*@file.*$| * @file $PRETTY|g" > $FILE.X

# Check for an output file 
if [ ! -f "$FILE.X" ]; then
	tput setaf 1; echo "location update for $1 failed..." 1>&2; tput sgr0
	exit 1
fi 

# Check for the updated line 
grep "^ \* @file $PRETTY$" $FILE.X &> /dev/null
if [ "$?" == "0" ]; then
	mv -f "$FILE.X" $FILE
	tput setaf 2; echo "$1 updated..."; tput sgr0
else 
	tput setaf 1; echo "automatic location updating for file $1 failed..." 1>&2; tput sgr0
	rm -f "$FILE.X"
fi


