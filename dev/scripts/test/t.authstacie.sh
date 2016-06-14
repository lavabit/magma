#!/bin/bash

# Name: t.authstacie.sh
# Author: Ladar Levison
#
# Description: Used for testing the STACIE authentication routines.

echo ""

# Success (ladar/test)
tput setaf 6; echo "Valid STACIE login over POP:"; tput sgr0
echo ""
printf "USER stacie\r\nPASS password\r\nQUIT\r\n" | nc localhost 8000
echo ""

# Fail (ladar/password)
tput setaf 6; echo "Invalid STACIE login over POP:"; tput sgr0
echo ""
printf "USER stacie\r\nPASS password\r\nQUIT\r\n" | nc localhost 8000
echo ""
