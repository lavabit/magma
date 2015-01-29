#!/bin/bash

echo ""
tput setaf 6; echo "IMAP session:"; tput sgr0
echo ""
nc localhost 9000 <<EOF
A01 LOGIN ladar test
A02 LIST "" "*"
A03 SELECT Inbox
A04 FETCH 1:* (UID BODY[HEADER.FIELDS (SUBJECT DATE)])
A05 LOGOUT
EOF

