#!/bin/bash

# Name: t.imap.body.sh
# Author: Ladar Levison
#
# Description: Used for testing the IMAP BODY command.

echo ""
tput setaf 6; echo "IMAP session:"; tput sgr0
echo ""
nc localhost 9000 <<EOF
A01 LOGIN magma test
A02 LIST "" "*"
A03 SELECT Inbox
A04 FETCH 1:* (UID BODY[HEADER.FIELDS (SUBJECT DATE)])
A05 LOGOUT
EOF

