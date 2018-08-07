#!/bin/bash

# Name: t.camel.delete.user.sh
# Author: Ladar Levison
#
# Description: Used for testing the camelface register user method.

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`       
API_PATH="http://localhost:10000/json"

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

if [ $# != 3 ] && [ $# != 4 ]; then
	CMD=`basename $0`
	printf "\n\t$CMD username password password_confirmation [plan]\n\n"
	exit 1
fi

if [ $# == 4 ]; then
read -d '' JSON <<-EOF
{
    "id": 1,
    "method": "register",
    "params": {
        "username": "$1",
        "password": "$2",
        "password_verification": "$3",
        "plan": "$4"
    }
}
EOF
else
read -d '' JSON <<-EOF
{
    "id": 1,
    "method": "register",
    "params": {
        "username": "$1",
        "password": "$2",
        "password_verification": "$3"
    }
}
EOF
fi

echo "Request:"
echo "$JSON"
echo "Response:"
curl --silent --data "$JSON" "$API_PATH"
echo ""
