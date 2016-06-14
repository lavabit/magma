#!/bin/bash

# Name: t.pop.sh
# Author: Ladar Levison
#
# Description: Used for testing the POP protocol handler.

echo ""
tput setaf 6; echo "POP session [Should Fail]:"; tput sgr0
echo ""
printf "USER magma\r\nPASS password\r\nQUIT\r\n" | nc localhost 8000 | grep -E "^\+|^\-"
echo ""
tput setaf 6; echo "Secure POP session [Should Work]:"; tput sgr0
echo ""
printf "USER magma\r\nPASS password\r\nSTAT\r\RETR 1\r\nQUIT\r\n" | openssl s_client -quiet -connect localhost:8500 -quiet 2>&1 | grep -E "^\+|^\-"


