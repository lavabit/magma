#!/bin/bash


# diff -U 10 -Nprw 		ORIGINAL-FOLDER 		MODIFIED-FOLDER

if [ $# -ne 2 ]
then 
	echo
	echo "Usage: `basename $0` archive directory"
	echo
	exit 1
fi

if [ ! -e "$1" ] || [ ! -f "$1" ]
then 
	echo
	echo "$1 is not a valid filename"
	echo
	exit 1
fi

if [ ! -s "$1" ]
then 
	echo
	echo "$1 exists, but is empty"
	echo
	exit 1
fi

if [ ! -d "$2" ]
then 
	echo
	echo "$2 does not exist"
	echo
	exit 1
fi


if [ "`pwd`" != "$HOME/Lavabit/magmad.so" ]
then 
	echo 
	echo "script is designed to execute from the directory $HOME/Lavabit/magmad.so/"
	echo
	exit 1
fi

if [ ! -d "archives" ]
then 
	echo
	echo "archives directory does not exist"
	echo
	exit 1
fi

CMPTMPDIR=`mktemp -d /tmp/tmp.XXXXXXXXXXXXXXXXXXX`
tar xzf $1 -C $CMPTMPDIR
diff -U 10 -Npr $CMPTMPDIR/$2 $2 > "archives/`basename $2`.patch"
rm -rf $CMPTMPDIR

echo 
du -h archives/`basename $2`.patch
echo
