#!/bin/bash

# Name: t.outbound.sh
# Author: Ladar Levison
#
# Description: Used for testing the SMTP outbound message handler.

DATE=`date`

echo ""
tput setaf 6; echo "Outbound SMTP request:"; tput sgr0
echo ""
printf "EHLO localhost\r\nAUTH PLAIN AGxhZGFyQGxhdmFiaXQuY29tAHRlc3Q=\r\nMAIL FROM: <ladar@lavabit.com>\r\n"\
"RCPT TO: <ladar@lavabit.com>\r\nDATA\r\nTo: ladar@lavabit.com\r\nFrom: ladar@lavabit.com\r\nSubject: Test @ $DATE\r\n\r\n"\
"Hello world!\r\nTesting outbound at $DATE\r\n\r\n.\r\nQUIT\r\n" | nc localhost 7000

