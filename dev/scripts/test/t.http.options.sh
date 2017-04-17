#!/bin/bash

# Name: t.http.options.sh
# Author: Ladar Levison
#
# Description: Used for testing HTTP OPTIONS method.

echo ""

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`       

# If the above logic doesn't find a process, then it's possible magma is running atop valgrind.
if [ -z "$PID" ]; then
	PID=`pidof valgrind`
	if [ ! -z "$PID" ]; then
		PID=`ps -ef | grep $PID | grep valgrind | grep -E "magmad|magmad.check" | awk -F' ' '{print $2}'`
	fi
fi

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "Magma process isn't running."; tput sgr0
	exit 2
fi

tput setaf 6; echo "HTTP OPTIONS Method:"; tput sgr0
echo ""
printf "OPTIONS /portal/camel HTTP/1.1\r\nHost: localhost.lavabit.com:10000\r\nUser-Agent: Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.17) "\
"Gecko/20110421 Red Hat/3.6.17-1.el6_0 Firefox/3.6.17\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"\
"Accept-Language: en-us,en;q=0.5\r\nAccept-Encoding: gzip,deflate\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"\
"Keep-Alive: 115\r\nConnection: keep-alive\r\nOrigin: http://localhost:10000\r\nAccess-Control-Request-Method: POST\r\n\r\n" | nc localhost 10000 
