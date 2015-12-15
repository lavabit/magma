#/bin/bash

# Name: t.imap.purge.sh
# Author: Ladar Levison
#
# Description: Used for testing the IMAP EXPUNGE command.

echo ""
tput setaf 6; echo "IMAP Purge Inbox Folder:"; tput sgr0
echo ""
printf "A01 LOGIN magma test\r\nA02 SELECT Inbox\r\nA03 STORE 1:* FLAGS.SILENT (\Deleted)\r\nA05 EXPUNGE\r\nA06 CLOSE\r\nA07 LOGOUT\r\n" | nc localhost 9000
