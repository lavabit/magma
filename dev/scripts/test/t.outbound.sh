#!/bin/bash

# Name: t.outbound.sh
# Author: Ladar Levison
#
# Description: Used for testing the SMTP outbound message handler.

DATE=`date`

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

tput setaf 6; echo "Outbound SMTP request:"; tput sgr0
echo ""
printf "EHLO localhost\r\nAUTH PLAIN bWFnbWEAbWFnbWEAcGFzc3dvcmQ=\r\nMAIL FROM: <magma@lavabit.com>\r\n"\
"RCPT TO: <ladar@lavabit.com>\r\nDATA\r\nTo: ladar@lavabit.com\r\nFrom: magma@lavabit.com\r\nSubject: Test @ $DATE\r\n\r\n"\
"Hello world!\r\nTesting outbound at $DATE\r\n\r\n.\r\nQUIT\r\n" | nc localhost 7000

