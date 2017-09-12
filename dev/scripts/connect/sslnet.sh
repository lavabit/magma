#!/bin/bash

# Name: sslnet.sh
# Author: Ladar Levison
#
# Description: Used for quickly connecting to a magmad instance. This script sets up a TLS  
# connection to the SMTPS port specified in bundled sandbox config file. Because the sandbox 
# environment uses non-standard ports, we only suggest using this script during development 
# and testing.

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

# The -ign_eof option is to prevent renegotiation whenever a line begins with R.
openssl s_client -ign_eof -connect localhost:7500

