#!/bin/bash

# Name: t.outbound.sh
# Author: Ladar Levison
#
# Description: Used for testing the SMTP outbound message handler.

DATE=`date`

echo ""
tput setaf 6; echo "Outbound SMTP request:"; tput sgr0
echo ""
printf "EHLO localhost\r\nAUTH PLAIN bWFnbWEAbWFnbWEAcGFzc3dvcmQ=\r\nMAIL FROM: <magma@lavabit.com>\r\n"\
"RCPT TO: <ladar@lavabit.com>\r\nDATA\r\nTo: ladar@lavabit.com\r\nFrom: magma@lavabit.com\r\nSubject: Test @ $DATE\r\n\r\n"\
"Hello world!\r\nTesting outbound at $DATE\r\n\r\n.\r\nQUIT\r\n" | nc localhost 7000

