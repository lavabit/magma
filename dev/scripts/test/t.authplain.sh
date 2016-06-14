#!/bin/bash

# Name: t.authplain.sh
# Author: Ladar Levison
#
# Description: Used for testing the AUTH PLAIN method.

echo ""

# Success (magma/password)
tput setaf 6; echo "Valid AUTH PLAIN requests:"; tput sgr0
echo ""
printf "AUTH PLAIN bWFnbWEAbWFnbWEAcGFzc3dvcmQ=\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH PLAIN\r\nbWFnbWEAbWFnbWEAcGFzc3dvcmQ=\r\nQUIT\r\n" | nc localhost 7000
echo ""

# Fail (magma/invalidpassword)
tput setaf 6; echo "Invalid AUTH PLAIN requests:"; tput sgr0
echo ""
printf "AUTH PLAIN bWFnbWEAbWFnbWEAaW52YWxpZHBhc3N3b3Jk\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH PLAIN\r\nbWFnbWEAbWFnbWEAaW52YWxpZHBhc3N3b3Jk\r\nQUIT\r\n" | nc localhost 7000
echo ""
