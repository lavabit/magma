#!/bin/bash

# Name: mycon.sh
# Author: Ladar Levison
#
# Description: Used for quickly connecting to a MySQL database instance. This script uses the
# default username, password and schema provided by the bundled sandbox config file. Because 
# the sandbox uses default values which are easily discovered, it is important that you don't 
# use this script in a production environment without first altering the default values.

# Check and make sure mysqld is running before attempting a connection.
PID=`pidof mysqld`       

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "The MySQL server process isn't running."; tput sgr0
	exit 2
fi

# Check and make sure the mysql command line client has been installed.
which mysql &>/dev/null
if [ $? -ne 0 ]; then
	tput setaf 1; tput bold; echo "The mysql client command isn't available. It may need to be installed."; tput sgr0
	exit 1
fi

mysql -u mytool --password=aComplex1 Lavabit
