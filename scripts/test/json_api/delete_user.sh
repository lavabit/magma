#!/bin/bash

API_PATH="http://localhost:10000/json"
read -d '' JSON <<"EOF"
{
	"id": 1,
	"method": "delete_user",
	"params": {
		"username": "testuser1"
	}
}
EOF

echo "Request:"
echo "$JSON"
echo "Response:"
curl --silent --data "$JSON" "$API_PATH"
echo ""
