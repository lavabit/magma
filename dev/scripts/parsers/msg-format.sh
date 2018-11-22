#!/bin/bash

# Name: msg-format.sh
# Author: Ladar Levison
#
# Description: Used to base64 encode, and then format an email message as a header
# so it can be added to the magma unit test corpus. The output is automatically 
# added to the check/magma/data/ folder.

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
ORIG=`pwd`

cd $BASE/../../../

# Make sure we were passed a path to an email message.
if [ $# -ne 1 ]; then
	printf "\nUsage: `basename $0` message.eml\n\n"
	exit 2
fi	

# Then make sure the path provided actually exists.
if [ ! -f $ORIG/$1 ]; then
	printf "\nUsage: `basename $0` message.eml\n\n"
	tput setaf 1
	printf "Invalid filename. { $1 }\n\n"
	tput sgr0
	exit 3
fi

# Find the first available header number.
COUNTER=1
until [ ! -f check/magma/data/message.$COUNTER.h ]; do
	let COUNTER+=1
done

# Store the last used number for later.
let PREVIOUS=$COUNTER-1

# Setup the path to the output header.
OUTPUT="check/magma/data/message.$COUNTER.h"

# Output the preprocessor heading.
printf "\n#ifndef CHECK_MESSAGE_${COUNTER}_H\n#define CHECK_MESSAGE_${COUNTER}_H\n\n#define MESSAGE_${COUNTER} " > $OUTPUT 

# Encode the message into base64 and add the result to the output header. 
base64 $ORIG/$1 | sed -e "s/^/\"/g" | sed -e "s/$/\" \\\/g" | head --bytes=-3 >> $OUTPUT

# Finish up the preprocessor directives.
printf "\n\n#endif\n\n" >> $OUTPUT

# Add an include to the source file.
sed -i -e "s/^\(\#include \"message.$PREVIOUS.h\"\)$/\1\n\#include \"message.$COUNTER.h\"/g" check/magma/data/data_check.c

# Add a reference to the variable length array.
sed -i -e "s/^\(\tNULLER(MESSAGE_$PREVIOUS)\)$/\1,\n\tNULLER(MESSAGE_$COUNTER)/g" check/magma/data/data_check.c

# Output a confirmation message.
tput setaf 2
printf "\nThe provided message was written to `basename $OUTPUT`\n\n"
tput sgr0



         

