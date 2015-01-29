#!/bin/bash
printf "stats\r\nquit\r\n" | nc -w 120 localhost 6000
if [ "$?" != "0" ]; then
	sleep 5
	printf "stats\r\nquit\r\n" | nc -w 120 localhost 6000
fi
if [ "$?" != "0" ]; then
	echo "error fetching magma statistics"
fi 
