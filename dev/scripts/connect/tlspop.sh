#/bin/bash

# Name: tlspop.sh
# Author: Ladar Levison
#
# Description: Used for quickly connecting to a magmad instance. This script sets up a TLS  
# connection using the POP port specified in bundled sandbox config file and the STARTTLS 
# command. Because the sandbox environment uses non-standard ports, we only suggest using 
# this script during development and testing.

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`       

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "Magma process isn't running."; tput sgr0
	exit 2
fi

openssl s_client -starttls pop3 -connect localhost:8000
