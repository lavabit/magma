#/bin/bash

# Name: magma.stat.sh
# Author: Ladar Levison
#
# Description: Used for quickly printing the internal magma statistics using the molten protocol.

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`       

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "Magma process isn't running."; tput sgr0
	exit 2
fi

printf "stats\r\nquit\r\n" | nc -w 120 localhost 6000
if [ "$?" != "0" ]; then
	sleep 5
	printf "stats\r\nquit\r\n" | nc -w 120 localhost 6000
fi
if [ "$?" != "0" ]; then
	echo "error fetching magma statistics"
fi 
