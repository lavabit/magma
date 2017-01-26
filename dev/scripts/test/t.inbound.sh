#!/bin/bash

# Name: t.inbound.sh
# Author: Ladar Levison
#
# Description: Used for testing the SMTP inbound message handler.

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

tput setaf 6; echo "Inbound SMTP request:"; tput sgr0
printf "EHLO localhost\r\nMAIL FROM: <magma@lavabit.com>\r\n"\
"RCPT TO: <magma@lavabit.com>\r\nDATA\r\nTo: magma@lavabit.com\r\nFrom: magma@lavabit.com\r\nSubject: Test @ $DATE\r\n\r\n"\
"Hello world!\r\nTesting outbound at $DATE\r\n\r\n.\r\nQUIT\r\n" | nc localhost 7000

echo ""
tput setaf 6; echo "Inbound STARTTLS SMTP request:"; tput sgr0
printf "EHLO localhost\nMAIL FROM: <magma@lavabit.com>\n"\
"RCPT TO: <magma@lavabit.com>\nDATA\nTo: magma@lavabit.com\nFrom: magma@lavabit.com\nSubject: Test @ $DATE\n\n"\
"Hello world!\nTesting outbound at $DATE\n\n.\nQUIT\n" | openssl s_client -quiet -starttls smtp -crlf -connect localhost:7000

echo ""
tput setaf 6; echo "Inbound SMTP over TLS request:"; tput sgr0
printf "EHLO localhost\nMAIL FROM: <magma@lavabit.com>\n"\
"RCPT TO: <magma@lavabit.com>\nDATA\nTo: magma@lavabit.com\nFrom: magma@lavabit.com\nSubject: Test @ $DATE\n\n"\
"Hello world!\nTesting outbound at $DATE\n\n.\nQUIT\n" | openssl s_client -quiet -crlf -connect localhost:7500
echo ""
