#!/bin/bash

# Name: json-format.sh
# Author: Ladar Levison
#
# Description: Used to quickly reformat JSON files.

# Check and make sure the json_reformat command line utility has been installed.
which json_reformat &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The json_reformat utility isn't available. It needs to be installed for this script to work."; tput sgr0
	exit 1
fi

# Derive the folder, and file name from the first argument.
DIR=`dirname $1`
FILE=`basename $1`
cd $DIR

# We can only generation function prototypes using JSON files.
SUFFIX=`echo $FILE | awk -F'.' '{ print $NF }'`
if [ "$SUFFIX" != "json" ]; then
	 echo "json formatter can only be used on json files..." 1>&2
	 exit 1
fi

# Run the formatter
cat "$FILE" | json_reformat > "$FILE.X"

# Check for an output file 
if [ ! -f "$FILE.X" ]; then
	echo "formatting of $1 failed..." 1>&2
	exit 1
fi 

# Replace the original
mv -f "$FILE.X" "$FILE"