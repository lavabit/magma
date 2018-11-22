#!/bin/bash

# Name: magma.mem.sh
# Author: Ladar Levison
#
# Description: Used for printing memory statistics on the magma and memcache daemons.

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

MAGMAHIT="no"
MYSQLHIT="no"
MEMCACHEDHIT="no"
ECLIPSEHIT="no"

# Print a couple of blank lines for separation
echo ""

# Magma Daemon Memory Usage

VMEM=`ps kstart_time auxw | egrep "$MAGMA_DIST/magmad|/usr/libexec/magmad" | grep -v "magmad.check" | grep -v grep | grep -v tail | grep -v dispatchd | grep -v scp | awk -F' ' '{print $5}' | tail -1`
RMEM=`ps kstart_time auxw | egrep "$MAGMA_DIST/magmad|/usr/libexec/magmad" | grep -v "magmad.check" | grep -v grep | grep -v tail | grep -v dispatchd | grep -v scp | awk -F' ' '{print $6}' | tail -1`
 
if [ "$VMEM" != '' ] || [ "$RMEM" != '' ]; then
	MAGMAHIT="yes"
	let "vmem = ($VMEM / 1024)"
	let "rmem = ($RMEM / 1024)"
	printf "%28.23s = %5.5s virtual megabytes %5.5s resident megabytes\n" "magmad" "$vmem" "$rmem"
fi
 
unset VMEM RMEM vmem rmem

# Magma Check Unit Tests Memory Usage 

VMEM=`ps kstart_time auxw | egrep "$MAGMA_DIST/magmad.check" | grep -v grep | grep -v tail | grep -v dispatchd | grep -v scp | awk -F' ' '{print $5}' | tail -1`
RMEM=`ps kstart_time auxw | egrep "$MAGMA_DIST/magmad.check" | grep -v grep | grep -v tail | grep -v dispatchd | grep -v scp | awk -F' ' '{print $6}' | tail -1`
 
if [ "$VMEM" != '' ] || [ "$RMEM" != '' ]; then
	MAGMAHIT="yes"
	let "vmem = ($VMEM / 1024)"
	let "rmem = ($RMEM / 1024)"
	printf "%28.23s = %5.5s virtual megabytes %5.5s resident megabytes\n" "magmad.check" "$vmem" "$rmem"
fi
 
unset VMEM RMEM vmem rmem

# MySQL Memory 

VMEM=`ps kstart_time auxw | grep "mysqld" | grep -v grep | awk -F' ' '{print $5}' | tail -1`
RMEM=`ps kstart_time auxw | grep "mysqld" | grep -v grep | awk -F' ' '{print $6}' | tail -1`

if [ "$VMEM" != '' ] || [ "$RMEM" != '' ]; then
	MYSQLHIT="yes"
	let "vmem = ($VMEM / 1024)"
	let "rmem = ($RMEM / 1024)"
	printf "%28.23s = %5.5s virtual megabytes %5.5s resident megabytes\n" "mysqld" "$vmem" "$rmem"
fi

unset VMEM RMEM vmem rmem

# Memcached Memory 

VMEM=`ps kstart_time auxw | grep "memcached" | grep -v grep | awk -F' ' '{print $5}' | tail -1`
RMEM=`ps kstart_time auxw | grep "memcached" | grep -v grep | awk -F' ' '{print $6}' | tail -1`

if [ "$VMEM" != '' ] || [ "$RMEM" != '' ]; then
	MEMCACHEDHIT="yes"
	let "vmem = ($VMEM / 1024)"
	let "rmem = ($RMEM / 1024)"
	printf "%28.23s = %5.5s virtual megabytes %5.5s resident megabytes\n" "memcached" "$vmem" "$rmem"
fi

unset VMEM RMEM vmem rmem

# Eclipse Memory 

VMEM=`ps kstart_time auxw | grep "java" | grep "eclipse" | awk -F' ' '{print $5}' | tail -1`
RMEM=`ps kstart_time auxw | grep "java" | grep "eclipse" | awk -F' ' '{print $6}' | tail -1`

if [ "$VMEM" != '' ] || [ "$RMEM" != '' ]; then
	ECLIPSEHIT="yes"
	let "vmem = ($VMEM / 1024)"
	let "rmem = ($RMEM / 1024)"
	printf "%28.23s = %5.5s virtual megabytes %5.5s resident megabytes\n" "eclipse" "$vmem" "$rmem"
fi

unset VMEM RMEM vmem rmem

if [ "$MAGMAHIT" == "no" ] || [ "$MYSQLHIT" == "no" ] || [ "$MEMCACHEDHIT" == "no" ] || [ "$ECLIPSEHIT" == "no" ]; then
	echo ""
fi

if [ "$MAGMAHIT" == "no" ]; then
	printf "%28.23s = $(tput setaf 1)%s$(tput sgr0)\n" "magmad and magmad.check" "not running"
fi

if [ "$MYSQLHIT" == "no" ]; then
	printf "%28.23s = $(tput setaf 1)%s$(tput sgr0)\n" "mysqld" "not running"
fi

if [ "$MEMCACHEDHIT" == "no" ]; then
	printf "%28.23s = $(tput setaf 1)%s$(tput sgr0)\n" "memcached" "not running"
fi

if [ "$ECLIPSEHIT" == "no" ]; then
	printf "%28.23s = $(tput setaf 1)%s$(tput sgr0)\n" "eclipse" "not running"
fi

echo ""

