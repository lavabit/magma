#!/bin/bash

# Name: t.inbound.sh
# Author: Ladar Levison
#
# Description: Used for testing the SMTP inbound message handler.

DATE=`date`

echo ""
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
