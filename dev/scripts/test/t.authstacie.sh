#!/bin/bash

# Name: t.authstacie.sh
# Author: Ladar Levison
#
# Description: Used for testing the STACIE authentication routines.

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

# Success (ladar/test)
tput setaf 6; echo "Valid STACIE login over POP:"; tput sgr0
echo ""
printf "USER stacie\r\nPASS password\r\nQUIT\r\n" | nc localhost 8000
echo ""

# Fail (ladar/password)
tput setaf 6; echo "Invalid STACIE login over POP:"; tput sgr0
echo ""
printf "USER stacie\r\nPASS password\r\nQUIT\r\n" | nc localhost 8000
echo ""
