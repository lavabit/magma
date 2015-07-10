#!/bin/bash

API_PATH="http://localhost:10000/json"
JSON="{\
	\"id\": 1,\
	\"method\": \"delete_user\",\
	\"params\": {\
		\"username\":\"testuser1\"\
	}\
}"
curl --silent --data "$JSON" "$API_PATH"
