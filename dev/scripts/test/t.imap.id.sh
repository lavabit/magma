#!/bin/bash

# Name: t.imap.id.sh
# Author: Ladar Levison
#
# Description: Used for testing IMAP ID command.

echo ""

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`       

# If the above logic doesn't find a process, then it's possible magma is running atop valgrind.
if [ -z "$PID" ]; then
	PID=`pidof valgrind`
	if [ ! -z "$PID" ]; then
		PID=`ps -ef | grep $PID | grep valgrind | grep -E "magmad|magmad.check" | awk -F' ' '{print $2}'`
	fi
fi

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "Magma process isn't running."; tput sgr0
	exit 2
fi

tput setaf 6; echo "IMAP ID Command:"; tput sgr0
echo ""
printf "A01 ID\r\nA04 LOGOUT\r\n" | nc localhost 9000
printf "A01 ID (\"program\" \"mrbig\")\r\nA04 LOGOUT\r\n" | nc localhost 9000
