#!/bin/bash

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../../

MAGMA_DIST=`pwd`

cd /home/ladar/Lavabit/magma.universe/sandbox
export LOGFILE=/home/ladar/Lavabit/magma.universe/docs/benchmark/mason/results.txt
export LD_LIBRARY_PATH=/home/ladar/Lavabit/magma.so/sources/bzip2:/home/ladar/Lavabit/magma.so/sources/zlib:/home/ladar/Lavabit/magma.so/sources/tokyocabinet:/magma.so/sources/lzo/src/.libs

mason() {
	echo "--------------------------------------------------------------------" &>> $LOGFILE
	date +"%nStarting at %r on %x%n"&>> $LOGFILE
	sync; echo 3 > /proc/sys/vm/drop_caches; 
	su ladar -l -c "LD_LIBRARY_PATH=$LD_LIBRARY_PATH nice -n -15 /usr/bin/time -v ~ladar/Lavabit/magma.tools/mason/.debug/mason clean flush async $* &>> $LOGFILE"
	echo "--------------------------------------------------------------------" &>> $LOGFILE
	ls -alb storage/mason.dat  &>> $LOGFILE
	ls -alk storage/mason.dat &>> $LOGFILE 
	ls -alh storage/mason.dat &>> $LOGFILE
}

mason
mason gzip
mason bzip
mason lzo1
mason lzo999
