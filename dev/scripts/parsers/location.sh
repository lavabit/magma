#/bin/bash

# Name: location.sh
# Author: Ladar Levison
#
# Description: Used to quickly update the file location inside the doxygen header.


# Build file location
PWD=`pwd`
FILE="$PWD$1" 
BASE=`basename $1`

# We can only generation function prototypes using code files
SUFFIX=`echo $BASE | awk -F'.' '{ print $NF }'`
if [ "$SUFFIX" != "c" ] && [ "$SUFFIX" != "h" ]; then
	 echo "location updates can only be performed on code files... not $SUFFIX ... $1" 1>&2
	 exit 1
fi

# We purposefully skip these files because they don't have a doxy description.
if [ $1 == "/magma/engine/status/build.h" ]; then
	echo "$1 skipped..."
	exit 0
fi

# Ensure the heading has a leading space.
/bin/sed -i -e "1 s/\/\*\*/\n&/g" $FILE

# See if we even need to update the file
grep "^ \* @file $1$" $FILE &> /dev/null
if [ "$?" == "0" ]; then
	echo "$1 is already correct..."
	exit 1
fi

# String replacement
cat $FILE | sed "s|^.*@file.*$| * @file $1|g" > $FILE.X

# Check for an output file 
if [ ! -f "$FILE.X" ]; then
	echo "location update for $1 failed..." 1>&2
	exit 1
fi 

# Check for the updated line 
grep "^ \* @file $1$" $FILE.X &> /dev/null
if [ "$?" == "0" ]; then
	mv -f "$FILE.X" $FILE
	tput setaf 2; echo "$1 updated..."; tput sgr0
else 
	echo "automatic location updating for file $1 failed..." 1>&2
	rm -f "$FILE.X"
fi


