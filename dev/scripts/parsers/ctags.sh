#!/bin/bash

# Name: ctags.sh
# Author: Ladar Levison
#
# Description: Used to quickly produce a header file for the functions in the current directory.

# Check and make sure the ctags command line utility has been installed.
which ctags &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The ctags utility isn't available. It needs to be installed for this script to work."; tput sgr0
	exit 1
fi

# Check and make sure the clipit command line utility has been installed.
which parcellite &>/dev/null
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

# Detect a directory and output definitions for any c files it contains.
if [ -d "$1" ]; then
	export CTAGSMODE="DIRECTORY"
	echo "" | clipit
	tput setaf 2; tput bold; printf "\nThe following output has already been copied to the clipboard. So go paste it already.\n"; tput sgr0
	find "$1" -type f -name "*.c" -exec $0 {} \; | parcellite
	exit 0
fi

# Derive the folder, and file name from the first argument
DIR=`dirname $1`
FILE=`basename $1`

# We can only generation function prototypes using code files
SUFFIX=`echo $FILE | awk -F'.' '{ print $NF }'`
if [ "$SUFFIX" != "c" ] && [ "$SUFFIX" != "h" ]; then
	 echo "C function prototype generation can only be triggered when a code file is active in the editor or a folder has been selected that contains code." 1>&2
	 exit 1
fi

cd $DIR

# Possible c tags to ouptut:
#	c  classes
#	d  macro definitions
#	e  enumerators (values inside an enumeration)
#	f  function definitions
#	g  enumeration names
#	l  local variables [off]
#	m  class, struct, and union members
#	n  namespaces
#	p  function prototypes [off]
#	s  structure names
#	t  typedefs
#	u  union names
#	v  variable definitions
#	x  external and forward variable declarations [off] 

# Check the build logs for the compiler parameters to pass into protoize
#OPTS=`cat /home/ladar/Lavabit/.metadata/.plugins/org.eclipse.cdt.ui/*.build.log | grep $FILE | grep -v "Building file:" | grep -v "Finished building:" | \
#	awk -F'gcc' '{ print $2 }' | sed -e "s/ /\ \n/g" | sort | uniq | grep -v "^$" | egrep "^\-D|^\-I|^-std" | sed -e 's/\\"//g' | tr -d '\n'`

# Run ctags and print functions, prototypes and macro definitions
export COLS=1024

# Generate the Tags
# Detect files that should get special treatment.
if [[ "$FILE" == "print.c" ]]; then
	TAGS=`ctags --c-kinds=f -x "$FILE" | grep -v -E "^START\_TEST" | grep -v "va\_list" | sed -e "s/^.*print\.c\s*\\(.*\\) {/\\1 __attribute__((format (printf, 2, 3)))\\;/g"`
	VTAGS=`ctags --c-kinds=f -x "$FILE" | grep -v -E "^START\_TEST" | grep "va\_list" | sed -e "s/^.*print\.c\s*\\(.*\\) {/\\1;/g"`
	TAGS=`printf "$TAGS\n$VTAGS"`
else
	SPLIT=`echo "$FILE" | /bin/sed -e "s/\\([\.\_]\\)/\/\/\\1/g" | tr '/' '\\\\'`
	TAGS=`ctags --c-kinds=fp -x "$FILE" | grep -v -E "^START\_TEST" | awk -F"$SPLIT" '{print $2}' | sed -e "s/ {/;/g" | sed -e "s/^[ ]\+//g" | sed -e "s/^[A-Za-z0-9\_]\+[ ]\+\*\?/&	/g" | sed -e "s/\t/|/g" | sed -e "s/| /|/g" | column -s '|' -t`
	TAGS+=`echo ""`
fi

# Put the output into the clipboard automatically, and don't bother with the banner if were processing an entire directory.
if [[ $CTAGSMODE == "DIRECTORY" ]]; then
	printf "\\n/// $FILE\\n$TAGS\\n"
else
	printf "/// $FILE\n$TAGS\n" | parcellite	
	printf "\n"
	tput setaf 2; tput bold; echo "The following output has already been copied to the clipboard. So go paste it already."; tput sgr0
	printf "\n/// $FILE\n$TAGS\n\n"
fi

exit 0
