#!/bin/bash

# Name: t.imap.purge.sh
# Author: Ladar Levison
#
# Description: Used for testing the IMAP EXPUNGE command.

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

tput setaf 6; echo "IMAP Purge Inbox Folder:"; tput sgr0
echo ""
printf "A01 LOGIN magma password\r\nA02 SELECT Inbox\r\nA03 STORE 1:* FLAGS.SILENT (\Deleted)\r\nA05 EXPUNGE\r\nA06 CLOSE\r\nA07 LOGOUT\r\n" | nc localhost 9000
