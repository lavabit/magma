#!/bin/bash

export COOKIES=`mktemp`
export API_PATH="http://localhost:10000/json"

submit() {
	# Submit using cURL
	# To print the server supplied HTTP headers add --include
	#export OUTPUT=`curl --silent --cookie "$COOKIES" --cookie-jar "$COOKIES" --data "$1" "$API_PATH"`
	curl --verbose --cookie "$COOKIES" --cookie-jar "$COOKIES" --data "$1" "$API_PATH"
}

submit "{\"id\":1,\"method\":\"auth\",\"params\":{\"username\":\"princess\",\"password\":\"test\"}}"

rm -f "$COOKIES"
