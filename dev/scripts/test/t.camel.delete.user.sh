#!/bin/bash

# Name: t.camel.delete.user.sh
# Author: Ladar Levison
#
# Description: Used for testing the camelface delete user method.

API_PATH="http://localhost:10000/json"
read -d '' JSON <<EOF
{
    "id": 1,
    "method": "delete_user",
    "params": {
        "username": "$1"
    }
}
EOF

echo "Request:"
echo "$JSON"
echo "Response:"
curl --silent --data "$JSON" "$API_PATH"
echo ""