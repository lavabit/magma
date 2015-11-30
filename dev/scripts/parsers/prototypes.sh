#/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

pushd $BASE/../../

MAGMA_DIST=`pwd`

popd

# Derive the folder, and file name from the first argument
DIR=`dirname $1`
FILE=`basename $1`
cd $DIR

# We can only generation function prototypes using code files
SUFFIX=`echo $FILE | awk -F'.' '{ print $NF }'`
if [ "$SUFFIX" != "c" ] && [ "$SUFFIX" != "h" ]; then
	 echo "prototype generation can only be performed on code files..." 1>&2
	 exit 1
fi

# Check the build logs for the compiler parameters to pass into protoize
OPTS=`cat $MAGMA_DIST/.metadata/.plugins/org.eclipse.cdt.ui/*.build.log | grep $FILE | grep -v "Building file:" | grep -v "Finished building:" | \
	awk -F'gcc' '{ print $2 }' | sed -e "s/ /\ \n/g" | sort | uniq | grep -v "^$" | egrep "^\-D|^\-I|^-std" | sed -e 's/\\"//g' | tr -d '\n'`

# Run protoize
### ${OPTS//-Werror /}
### OPTS="$OPTS -fsyntax-only"

###
### UNCOMMENT FOR DEBUGING
###
### printf "\n\n/usr/bin/protoize -q -n -c \"%s\" %s\n\n" "$OPTS" "$FILE" 1>&2
###
###

# Protoize
/usr/bin/protoize -q -n -c "$OPTS" $FILE 

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
cat $FILE.X | grep $FILE | /usr/bin/columns -S " " -s | awk -F'/' '{print $3 }' | awk -F'extern ' '{print $2}' | sed -e "s/^[a-zA-Z0-9_]*\ \*/& /"

# Remove the protoize file
rm $FILE.X
