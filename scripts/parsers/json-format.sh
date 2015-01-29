#!/bin/bash

# Derive the folder, and file name from the first argument
DIR=`dirname $1`
FILE=`basename $1`
cd $DIR

# We can only generation function prototypes using code files
SUFFIX=`echo $FILE | awk -F'.' '{ print $NF }'`
if [ "$SUFFIX" != "json" ]; then
	 echo "json formatter can only be used on json files..." 1>&2
	 exit 1
fi

# Run the formatter
/bin/cat "$FILE" | /usr/bin/json_reformat > "$FILE.X"

# Check for an output file 
if [ ! -f "$FILE.X" ]; then
	echo "formatting of $1 failed..." 1>&2
	exit 1
fi 

# Replace the original
/bin/mv -f "$FILE.X" "$FILE"