#!/bin/bash

# Name: t.authplain.sh
# Author: Ladar Levison
#
# Description: Used for testing the AUTH PLAIN method.

echo ""

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`       

# If the above logic doesn't find a process, then it's possible magma is running atop valgrind.
if [ -z "$PID" ]; then
	PID=`pidof valgrind`
	if [ -z "$PID" ]; then
		PID=`ps -ef | grep $PID | grep valgrind | grep -E "magmad|magmad.check" | awk -F' ' '{print $2}'`
	fi
fi

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "Magma process isn't running."; tput sgr0
	exit 2
fi

# Success (magma/password)
tput setaf 6; echo "Valid AUTH PLAIN requests:"; tput sgr0
echo ""
printf "AUTH PLAIN bWFnbWEAbWFnbWEAcGFzc3dvcmQ=\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH PLAIN\r\nbWFnbWEAbWFnbWEAcGFzc3dvcmQ=\r\nQUIT\r\n" | nc localhost 7000
echo ""

# Fail (magma/invalidpassword)
tput setaf 6; echo "Invalid AUTH PLAIN requests:"; tput sgr0
echo ""
printf "AUTH PLAIN bWFnbWEAbWFnbWEAaW52YWxpZHBhc3N3b3Jk\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH PLAIN\r\nbWFnbWEAbWFnbWEAaW52YWxpZHBhc3N3b3Jk\r\nQUIT\r\n" | nc localhost 7000
echo ""
