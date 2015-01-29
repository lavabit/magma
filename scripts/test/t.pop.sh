#!/bin/bash

echo ""
tput setaf 6; echo "POP session:"; tput sgr0
echo ""
printf "USER ladar\r\nPASS test\r\nSTAT\r\nRETR 1\r\nQUIT\r\n" | nc localhost 8000
