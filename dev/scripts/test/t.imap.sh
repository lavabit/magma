#!/bin/bash

echo ""
tput setaf 6; echo "IMAP session:"; tput sgr0
echo ""
printf "A01 LOGIN ladar test\r\nA02 LIST \"\" \"*\"\r\nA03 SELECT Inbox\r\nA04 FETCH 1:* RFC822\r\nA05 LOGOUT\r\n" | nc localhost 9000
