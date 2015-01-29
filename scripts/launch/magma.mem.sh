#!/bin/bash

# Magma Daemon Memory 

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`


HIT="no"

VMEM=`ps kstart_time auxw | egrep "$MAGMA_DIST/src/.debug/magmad" | grep -v grep | grep -v tail | grep -v dispatchd | grep -v scp | awk -F' ' '{print $5}' | tail -1`
RMEM=`ps kstart_time auxw | egrep "$MAGMA_DIST/src/.debug/magmad" | grep -v grep | grep -v tail | grep -v dispatchd | grep -v scp | awk -F' ' '{print $6}' | tail -1`
 
if [ "$VMEM" != '' ] || [ "$RMEM" != '' ]; then
	HIT="yes"
	let "vmem = ($VMEM / 1024)"
	let "rmem = ($RMEM / 1024)"
	echo "mamgma = $vmem virtual megabytes / $rmem resident megabytes"
fi
 
unset VMEM RMEM vmem rmem

# Magma Check Memory 

VMEM=`ps kstart_time auxw | egrep "$MAGMA_DIST/check/.check/magmad.check" | grep -v grep | grep -v tail | grep -v dispatchd | grep -v scp | awk -F' ' '{print $5}' | tail -1`
RMEM=`ps kstart_time auxw | egrep "$MAGMA_DIST/check/.check/magmad.check" | grep -v grep | grep -v tail | grep -v dispatchd | grep -v scp | awk -F' ' '{print $6}' | tail -1`
 
if [ "$VMEM" != '' ] || [ "$RMEM" != '' ]; then
	HIT="yes"
	let "vmem = ($VMEM / 1024)"
	let "rmem = ($RMEM / 1024)"
	echo "mamgmad.check = $vmem virtual megabytes / $rmem resident megabytes"
fi
 
unset VMEM RMEM vmem rmem

if [ "$HIT" == "no" ]; then
	echo "magmad and magmad.check = not running"
fi

# Memcached Memory 

VMEM=`ps kstart_time auxw | grep "memcached" | grep -v grep | awk -F' ' '{print $5}' | tail -1`
RMEM=`ps kstart_time auxw | grep "memcached" | grep -v grep | awk -F' ' '{print $6}' | tail -1`

if [ "$VMEM" != '' ] || [ "$RMEM" != '' ]; then
	HIT="yes"
	let "vmem = ($VMEM / 1024)"
	let "rmem = ($RMEM / 1024)"
	echo "memcached = $vmem virtual megabytes / $rmem resident megabytes"
fi



