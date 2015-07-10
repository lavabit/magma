#!/bin/bash

API_PATH="http://localhost:10000/json"
read -d '' JSON <<EOF
{
    "id": 1,
    "method": "auth",
    "params": {
        "username": "$1",
        "password": "$2"
    }
}
EOF

echo "Request:"
echo "$JSON"
echo "Response:"
curl --silent --data "$JSON" "$API_PATH"
echo ""
