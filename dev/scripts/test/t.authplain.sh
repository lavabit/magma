#/bin/bash

# Name: t.authplain.sh
# Author: Ladar Levison
#
# Description: Used for testing the AUTH PLAIN method.

echo ""

# Success (ladar/test)
tput setaf 6; echo "Valid AUTH PLAIN requests:"; tput sgr0
echo ""
printf "AUTH PLAIN AGxhZGFyQGxhdmFiaXQuY29tAHRlc3Q=\r\nQUIT\r\n" | nc localhost 7000
echo ""
printf "AUTH PLAIN\r\nAGxhZGFyQGxhdmFiaXQuY29tAHRlc3Q=\r\nQUIT\r\n" | nc localhost 7000
echo ""

# Fail (ladar/password)
tput setaf 6; echo "Invalid AUTH PLAIN requests:"; tput sgr0
echo ""
printf "AUTH PLAIN AGxhZGFyQGxhdmFiaXQuY29tAHBhc3N3b3Jk\r\nQUIT\r\n" | nc localhost 7000
echo "" 
printf "AUTH PLAIN\r\nAGxhZGFyQGxhdmFiaXQuY29tAHBhc3N3b3Jk\r\nQUIT\r\n" | nc localhost 7000
echo ""