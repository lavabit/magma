#!/bin/bash

DATE=`date`

echo ""
tput setaf 6; echo "Inbound SMTP request:"; tput sgr0
printf "EHLO localhost\r\nMAIL FROM: <magma@lavabit.com>\r\n"\
"RCPT TO: <magma@lavabit.com>\r\nDATA\r\nTo: magma@lavabit.com\r\nFrom: magma@lavabit.com\r\nSubject: Test @ $DATE\r\n\r\n"\
"Hello world!\r\nTesting outbound at $DATE\r\n\r\n.\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "EHLO localhost\r\nMAIL FROM: <ladar@lavabit.com>\r\n"\
"RCPT TO: <ladar@lavabit.com>\r\nDATA\r\nTo: ladar@lavabit.com\r\nFrom: ladar@lavabit.com\r\nSubject: Test @ $DATE\r\n\r\n"\
"Hello world!\r\nTesting outbound at $DATE\r\n\r\n.\r\nQUIT\r\n" | nc localhost 7000