#!/bin/bash

# Name: prototypes.sh
# Author: Ladar Levison
#
# Description: Used to quickly produce a header file for the functions in the current directory. 
# This script uses protoize instead of ctags. The ctags script is generally preferred over this version.

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../

MAGMA_DIST=`pwd`

# Derive the folder, and file name from the first argument
DIR=`dirname $1`
FILE=`basename $1`
cd $DIR

# Check and make sure the protoize command line utility has been installed.
which protoize &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The protoize utility isn't available. It needs to be installed for this script to work."; tput sgr0
	exit 1
fi

# Check and make sure the clipit command line utility has been installed.
which clipit &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The clipit utility isn't available. It needs to be installed for this script to work."; tput sgr0
	exit 1
fi

# Check and make sure the sed command line utility has been installed.
which sed &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The sed utility isn't available. It needs to be installed for this script to work."; tput sgr0
	exit 1
fi

# Check and make sure the awk command line utility has been installed.
which awk &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The awk utility isn't available. It needs to be installed for this script to work."; tput sgr0
	exit 1
fi



# We can only generation function prototypes using code files
SUFFIX=`echo $FILE | awk -F'.' '{ print $NF }'`
if [ "$SUFFIX" != "c" ] && [ "$SUFFIX" != "h" ]; then
	 echo "prototype generation can only be performed on code files..." 1>&2
	 exit 1
fi

# Check the build logs for the compiler parameters to pass into protoize
OPTS=`cat $MAGMA_DIST/../.metadata/.plugins/org.eclipse.cdt.ui/*.build.log | grep $FILE | grep -v "Building file:" | grep -v "Finished building:" | \
	awk -F'gcc' '{ print $2 }' | sed -e "s/ /\ \n/g" | sort | uniq | grep -v "^$" | egrep "^\-D|^\-I|^-std" | sed -e 's/\\"//g' | tr -d '\n'`

# Run protoize
### ${OPTS//-Werror /}
### OPTS="$OPTS -fsyntax-only"

###
### UNCOMMENT FOR DEBUGING
###
### printf "\n\nprotoize -q -n -c \"%s\" %s\n\n" "$OPTS" "$FILE" 1>&2
###
###

# Protoize
protoize -q -n -c "$OPTS" $FILE 

# Check for an output file 
if [ ! -f "$FILE.X" ]; then
	echo "prototype generation for $1 failed..." 1>&2
	exit 1
fi 

# Remove the space between the function name and the opening parameter paren.
sed -e "s/\ [\(]\\([A-Za-z]\\)/\(\\1/" -i $FILE.X

# Output Filename
echo "/// $FILE"

# Extract the function prototypes found in the file were concerned with and sort the output by function name.
cat $FILE.X | grep $FILE | columns -S " " -s | awk -F'/' '{print $3 }' | awk -F'extern ' '{print $2}' | sed -e "s/^[a-zA-Z0-9_]*\ \*/& /"

# Remove the protoize file
rm $FILE.X
