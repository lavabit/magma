#!/bin/bash

# Name: t.pop.sh
# Author: Ladar Levison
#
# Description: Used for testing the POP protocol handler.

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

tput setaf 6; echo "POP session [Should Fail]:"; tput sgr0
echo ""
printf "USER magma\r\nPASS password\r\nQUIT\r\n" | nc localhost 8000 | grep -E "^\+|^\-"
echo ""
tput setaf 6; echo "Secure POP session [Should Work]:"; tput sgr0
echo ""
printf "USER magma\r\nPASS password\r\nSTAT\r\RETR 1\r\nQUIT\r\n" | openssl s_client -quiet -connect localhost:8500 -quiet 2>&1 | grep -E "^\+|^\-"


