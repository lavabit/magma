#!/bin/bash

# Name: t.authlogin.sh
# Author: Ladar Levison
#
# Description: Used for testing the AUTH LOGIN method.

echo ""

# Success (ladar/test)
tput setaf 6; echo "Valid AUTH LOGIN requests:"; tput sgr0
echo ""
printf "AUTH LOGIN bGFkYXI=\r\ndGVzdA==\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH LOGIN\r\nbGFkYXI=\r\ndGVzdA==\r\nQUIT\r\n" | nc localhost 7000
echo ""

# Fail (ladar/password)
tput setaf 6; echo "Invalid AUTH LOGIN requests:"; tput sgr0
echo ""
printf "AUTH LOGIN bGFkYXI=\r\ncGFzc3dvcmQ=\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH LOGIN\r\nbGFkYXI=\r\ncGFzc3dvcmQ=\r\nQUIT\r\n" | nc localhost 7000
echo ""
