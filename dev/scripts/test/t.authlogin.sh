#!/bin/bash

# Name: t.authlogin.sh
# Author: Ladar Levison
#
# Description: Used for testing the AUTH LOGIN method.

echo ""

# Success (magma/password)
tput setaf 6; echo "Valid AUTH LOGIN requests:"; tput sgr0
echo ""
printf "AUTH LOGIN bWFnbWE=\r\ncGFzc3dvcmQ=\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH LOGIN\r\nbWFnbWE=\r\ncGFzc3dvcmQ=\r\nQUIT\r\n" | nc localhost 7000
echo ""

# Fail (magma/invalidpassword)
tput setaf 6; echo "Invalid AUTH LOGIN requests:"; tput sgr0
echo ""
printf "AUTH LOGIN bWFnbWE=\r\naW52YWxpZHBhc3N3b3Jk\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH LOGIN\r\nbWFnbWE=\r\naW52YWxpZHBhc3N3b3Jk\r\nQUIT\r\n" | nc localhost 7000
echo ""
