#!/bin/bash

# Support the all and clean keywords.
# Meant to avoid some re-compilation. Not modified for general paths
# Incomplete and inactive

LINK=`readlink -f $0`
BASE=`dirname $LINK`

pushd $BASE/../../

if [[ "$1" == "all" ]]; then
	make -k -j4 all
elif [[ "$1" == "clean" ]]; then
	make -k -j4 clean
elif [ ! -e "$1" ] || [ ! -f "$1" ]; then 
	FILES=`cd /home/ladar/Lavabit/magma/; find . -name "*.c" -newer /home/ladar/Lavabit/magma/.debug/quick.stamp -print | sed -e 's/\.c$/.o/g'`
	echo $FILES
	if [[ "$FILES" != "" ]]; then
		cd /home/ladar/Lavabit/magma/.debug/; make -k -j4 $FILES
	fi
else
	FILES=`echo $1 | sed -e 's/\/magma\/\(.*\)\.c$/.\/\\1.o/g'; cd /home/ladar/Lavabit/magma/; find . -name "*.c" -newer /home/ladar/Lavabit/magma/.debug/quick.stamp -print | sed -e 's/\.c$/.o/g'`
	echo $FILES
	if [[ "$FILES" != "" ]]; then
		cd /home/ladar/Lavabit/magma/.debug/; make -k -j4 $FILES
	fi
fi

# Touch the timestamp.
if [ "$?" == "0" ]; then 
	touch /home/ladar/Lavabit/magma/.debug/quick.stamp
fi
