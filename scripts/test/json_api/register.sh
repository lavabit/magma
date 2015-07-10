#!/bin/bash

API_PATH="http://localhost:10000/json"
read -d '' JSON <<"EOF"
{
	"id": 1,
	"method": "register",
	"params": {
		"username": "testuser1",
		"password": "testpassword",
		"password_verification": "testpassword"
	}
}
EOF

echo "Request:"
echo "$JSON"
echo "Response:"
curl --silent --data "$JSON" "$API_PATH"
echo ""
