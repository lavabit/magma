#!/bin/bash

# Name: t.camel.edit.config.sh
#
# Description: Used for testing the camelface edit config user method.

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

API_PATH="http://localhost:10000/portal/camel"
read -d '' JSON <<EOF
{
    "id": 1,
    "method": "config.edit",
    "params": {
        "$2": $3
    }
}
EOF

echo "Request:"
echo "$JSON"
echo "Response:"
curl --silent --cookie "$1" --data "$JSON" "$API_PATH"
echo ""
