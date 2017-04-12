#!/bin/bash

export SEED=$(head -1 /dev/urandom | od -N 16 | awk '{ print $2 }')
export RANDOM=$SEED

export IDNUM="1"
export USERID="magma"
export PASSWORD="password"
export COOKIES=`mktemp`
export SQLHOST="localhost"
export CAMELHOST="https://localhost:10500"
export CAMELPATH="$CAMELHOST/portal/camel"

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`

# If the above logic doesn't find a process, then it's possible magma is running atop valgrind.
if [ -z "$PID" ]; then
	PID=`pidof valgrind`
	if [ -z "$PID" ]; then
		PID=`ps -ef | grep $PID | grep valgrind | grep -E "magmad|magmad.check" | awk -F' ' '{print $2}'`
	fi
fi

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "Magma process isn't running."; tput sgr0
	exit 2
fi

wget --quiet --no-check-certificate --retry-connrefused --connect-timeout=1 --waitretry=1 --tries=0 --output-document=/dev/null "$CAMELHOST"
if [ $? != 0 ]; then
	tput setaf 1; tput bold; echo "tired of waiting on the magma daemon start"; tput sgr0
	exit
fi

submit() {

	# Client submission in yellow, server responses in red.
	tput setaf 3; tput bold; echo "$1";

	# Submit using cURL
	# To print the server supplied HTTP headers add --include

	export OUTPUT=`curl --insecure --silent --cookie "$COOKIES" --cookie-jar "$COOKIES" --data "$1" "$CAMELPATH"`

	# Submit using wget
	# To print the server supplied HTTP headers add --server-response

	#export OUTPUT=`wget --no-check-certificate --load-cookies="$COOKIES" --save-cookies="$COOKIES" --quiet --output-document=- --post-data="$1" "$CAMELPATH"`

	if [[ "$OUTPUT" =~ "\"result\":" ]] && [[ ! "$OUTPUT" =~ "\"failed\"" ]]; then
		tput setaf 2; tput bold; echo $OUTPUT
	else
		tput setaf 1; tput bold; echo $OUTPUT
	fi

	tput sgr0; echo ""
	export IDNUM=`expr $IDNUM + 1`
}

foldernum() {
	unset FOLDERNUM
	export FOLDERNUM=`echo "SELECT foldernum FROM Users LEFT JOIN Folders ON Users.usernum = Folders.usernum WHERE Users.userid = '$USERID' AND Folders.foldername = '$1';" | mysql --disable-tee --skip-column-names -u mytool -h "$SQLHOST" --password=aComplex1 Lavabit`
}

messagenums() {
	unset SOURCENUM; unset MESSAGENUM
	export SOURCENUM=`echo "SELECT foldernum FROM Users LEFT JOIN Folders ON Users.usernum = Folders.usernum WHERE Users.userid = '$USERID' AND Folders.foldername = '$1';" | mysql --disable-tee --skip-column-names -u mytool -h "$SQLHOST" --password=aComplex1 Lavabit`
	export MESSAGENUM=`echo "SELECT messagenum FROM Users LEFT JOIN Messages ON Users.usernum = Messages.usernum LEFT JOIN Folders ON Messages.foldernum = Folders.foldernum WHERE Users.userid = '$USERID' AND Folders.foldername = '$1' ORDER BY RAND() LIMIT $2;" | mysql --disable-tee --skip-column-names -u mytool -h "$SQLHOST" --password=aComplex1 Lavabit | tr '\n' ',' | sed -e 's/\,$//g'`
}

contactnums() {
	unset SOURCENUM; unset CONTACTNUM
	export SOURCENUM=`echo "SELECT foldernum FROM Users LEFT JOIN Folders ON Users.usernum = Folders.usernum WHERE Users.userid = '$USERID' AND Folders.foldername = '$1';" | mysql --disable-tee --skip-column-names -u mytool -h "$SQLHOST" --password=aComplex1 Lavabit`
	export CONTACTNUM=`echo "SELECT contactnum FROM Users LEFT JOIN Contacts ON Users.usernum = Contacts.usernum LEFT JOIN Folders ON Contacts.foldernum = Folders.foldernum WHERE Users.userid = '$USERID' AND Folders.foldername = '$1' ORDER BY RAND() LIMIT $2;" | mysql --disable-tee --skip-column-names -u mytool -h "$SQLHOST" --password=aComplex1 Lavabit | tr '\n' ',' | sed -e 's/\,$//g'`
}


echo ""
tput setaf 6; echo "Camel requests:"; tput sgr0
echo ""

submit "{\"id\":$IDNUM,\"method\":\"auth\",\"params\":{\"username\":\"$USERID\",\"password\":\"$PASSWORD\"}}"
submit "{\"id\":$IDNUM,\"method\":\"config.edit\",\"params\":{\"key\":\"value\"}}"
submit "{\"id\":$IDNUM,\"method\":\"config.load\"}"
submit "{\"id\":$IDNUM,\"method\":\"config.edit\",\"params\":{\"key\":null}}"
submit "{\"id\":$IDNUM,\"method\":\"config.load\"}"
submit "{\"id\":$IDNUM,\"method\":\"config.edit\",\"params\":{\"key.$RANDOM\":\"$RANDOM\"}}"
submit "{\"id\":$IDNUM,\"method\":\"folders.add\",\"params\":{\"context\":\"contacts\",\"name\":\"Flight Crew\"}}"
submit "{\"id\":$IDNUM,\"method\":\"folders.list\",\"params\":{\"context\":\"contacts\"}}"
foldernum "Flight Crew"; submit "{\"id\":$IDNUM,\"method\":\"contacts.add\",\"params\":{\"folderID\":$FOLDERNUM, \"contact\":{\"name\":\"Jenna\", \"email\":\"jenna@jameson.com\"}}}"
contactnums "Flight Crew" "1"; submit "{\"id\":$IDNUM,\"method\":\"contacts.copy\",\"params\":{\"sourceFolderID\":$SOURCENUM, \"targetFolderID\":$SOURCENUM, \"contactID\": $CONTACTNUM }}"
foldernum "Flight Crew"; submit "{\"id\":$IDNUM,\"method\":\"contacts.list\",\"params\":{\"folderID\":$FOLDERNUM }}"
contactnums "Flight Crew" "1"; submit "{\"id\":$IDNUM,\"method\":\"contacts.edit\",\"params\":{\"folderID\":$FOLDERNUM, \"contactID\":$CONTACTNUM, \"contact\":{\"name\":\"Jenna Marie Massoli\", \"email\":\"jenna+private-chats@jameson.com\"}}}"
contactnums "Flight Crew" "1"; submit "{\"id\":$IDNUM,\"method\":\"contacts.load\",\"params\":{\"folderID\":$SOURCENUM, \"contactID\":$CONTACTNUM }}"
contactnums "Flight Crew" "1"; submit "{\"id\":$IDNUM,\"method\":\"contacts.edit\",\"params\":{\"folderID\":$FOLDERNUM, \"contactID\":$CONTACTNUM, \"contact\":{\"name\":\"Jenna\", \"email\":\"jenna@jameson.com\", \"phone\":\"2145551212\", \"notes\":\"The Tuesday night hottie!\"}}}"
contactnums "Flight Crew" "1"; submit "{\"id\":$IDNUM,\"method\":\"contacts.load\",\"params\":{\"folderID\":$SOURCENUM, \"contactID\":$CONTACTNUM }}"
submit "{\"id\":$IDNUM,\"method\":\"folders.add\",\"params\":{\"context\":\"contacts\",\"name\":\"Lovers\"}}"
foldernum "Lovers"; contactnums "Flight Crew" "1"; submit "{\"id\":$IDNUM,\"method\":\"contacts.move\",\"params\":{ \"contactID\":$CONTACTNUM, \"sourceFolderID\":$SOURCENUM, \"targetFolderID\":$FOLDERNUM }}"
foldernum "Flight Crew"; submit "{\"id\":$IDNUM,\"method\":\"contacts.list\",\"params\":{\"folderID\":$FOLDERNUM }}"
foldernum "Lovers"; submit "{\"id\":$IDNUM,\"method\":\"contacts.list\",\"params\":{\"folderID\":$FOLDERNUM }}"
contactnums "Flight Crew" "1"; submit "{\"id\":$IDNUM,\"method\":\"contacts.remove\",\"params\":{\"folderID\":$SOURCENUM, \"contactID\":$CONTACTNUM }}"
contactnums "Lovers" "1"; submit "{\"id\":$IDNUM,\"method\":\"contacts.remove\",\"params\":{\"folderID\":$SOURCENUM, \"contactID\":$CONTACTNUM }}"
foldernum "Lovers"; submit "{\"id\":$IDNUM,\"method\":\"contacts.list\",\"params\":{\"folderID\":$FOLDERNUM }}"
foldernum "Flight Crew"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"contacts\",\"folderID\":$FOLDERNUM }}"
foldernum "Lovers"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"contacts\",\"folderID\":$FOLDERNUM }}"
submit "{\"id\":$IDNUM,\"method\":\"cookies\"}"
submit "{\"id\":$IDNUM,\"method\":\"alert.list\"}"
submit "{\"id\":$IDNUM,\"method\":\"alert.acknowledge\",\"params\":[1,7,13]}"
submit "{\"id\":$IDNUM,\"method\":\"alert.list\"}"
submit "{\"id\":$IDNUM,\"method\":\"folders.list\",\"params\":{\"context\":\"mail\"}}"
submit "{\"id\":$IDNUM,\"method\":\"folders.list\",\"params\":{\"context\":\"settings\"}}"
submit "{\"id\":$IDNUM,\"method\":\"folders.list\",\"params\":{\"context\":\"help\"}}"
submit "{\"id\":$IDNUM,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Camel\"}}"
foldernum "Camel"; submit "{\"id\":$IDNUM,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"parentID\":$FOLDERNUM,\"name\":\"Toe\"}}"
foldernum "Toe"; submit "{\"id\":$IDNUM,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"parentID\":$FOLDERNUM,\"name\":\"Rocks\"}}"
foldernum "Rocks"; submit "{\"id\":$IDNUM,\"method\":\"folders.rename\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM,\"name\":\"Dames.Rock\"}}"
foldernum "Dames"; submit "{\"id\":$IDNUM,\"method\":\"folders.rename\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM,\"name\":\"Clams\"}}"
foldernum "Rock"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM }}"
foldernum "Clams"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM }}"
foldernum "Toe"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM }}"
foldernum "Camel"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM }}"
submit "{\"id\":$IDNUM,\"method\":\"aliases\"}"
submit "{\"id\":$IDNUM,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Duplicate\"}}"
foldernum "Duplicate"; messagenums "Inbox" "100"; submit "{\"id\":$IDNUM,\"method\":\"messages.copy\",\"params\":{\"messageIDs\": [$MESSAGENUM], \"sourceFolderID\":$SOURCENUM, \"targetFolderID\":$FOLDERNUM }}"
foldernum "Duplicate"; messagenums "Inbox" "100"; submit "{\"id\":$IDNUM,\"method\":\"messages.copy\",\"params\":{\"messageIDs\": [$MESSAGENUM], \"sourceFolderID\":$SOURCENUM, \"targetFolderID\":$FOLDERNUM }}"
foldernum "Duplicate"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM }}"
submit "{\"id\":$IDNUM,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Duplicate\"}}"
messagenums "Inbox" "1"; submit "{\"id\":$IDNUM,\"method\":\"messages.load\",\"params\":{\"messageID\": $MESSAGENUM, \"folderID\":$SOURCENUM, \"sections\": [\"meta\", \"source\", \"security\", \"server\", \"header\", \"body\", \"attachments\" ]}}"
foldernum "Duplicate"; messagenums "Inbox" "100"; submit "{\"id\":$IDNUM,\"method\":\"messages.copy\",\"params\":{\"messageIDs\": [$MESSAGENUM], \"sourceFolderID\":$SOURCENUM, \"targetFolderID\":$FOLDERNUM }}"
foldernum "Duplicate"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM }}"
messagenums "Inbox" "100"; submit "{\"id\":$IDNUM,\"method\":\"messages.flag\",\"params\":{\"action\":\"add\", \"flags\":[\"flagged\"], \"messageIDs\": [$MESSAGENUM], \"folderID\":$SOURCENUM }}"
messagenums "Inbox" "100"; submit "{\"id\":$IDNUM,\"method\":\"messages.tags\",\"params\":{\"action\":\"add\", \"tags\":[\"girlie\",\"girlie-$RANDOM\"], \"messageIDs\": [$MESSAGENUM], \"folderID\":$SOURCENUM }}"
messagenums "Inbox" "100"; submit "{\"id\":$IDNUM,\"method\":\"messages.flag\",\"params\":{\"action\":\"list\", \"messageIDs\": [$MESSAGENUM], \"folderID\":$SOURCENUM }}"
messagenums "Inbox" "100"; submit "{\"id\":$IDNUM,\"method\":\"messages.tags\",\"params\":{\"action\":\"list\", \"messageIDs\": [$MESSAGENUM], \"folderID\":$SOURCENUM }}"
foldernum "Inbox"; submit "{\"id\":$IDNUM,\"method\":\"messages.list\",\"params\":{\"folderID\":$FOLDERNUM }}"
foldernum "Inbox"; submit "{\"id\":$IDNUM,\"method\":\"folders.tags\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM }}"
submit "{\"id\":$IDNUM,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Mover\"}}"
foldernum "Mover"; messagenums "Inbox" "1000"; submit "{\"id\":$IDNUM,\"method\":\"messages.move\",\"params\":{\"messageIDs\": [$MESSAGENUM], \"sourceFolderID\": $SOURCENUM, \"targetFolderID\":$FOLDERNUM }}"
messagenums "Mover" "1000"; submit "{\"id\":$IDNUM,\"method\":\"messages.remove\",\"params\":{\"folderID\":$SOURCENUM,\"messageIDs\":[$MESSAGENUM]}}"
foldernum "Mover"; submit "{\"id\":$IDNUM,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":$FOLDERNUM }}"


# submit "{\"id\":$IDNUM,\"method\":\"messages.load\",\"params\":[{\"messageID\": 0, \"section\": [\"header, body, attachments\"]}, {\"messageID\": 0, \"section\": [\"info\"]}]}"
# submit "{\"id\":$IDNUM,\"method\":\"messages.tags\",\"params\":{\"messageIDs\": [142, 154, 156], \"tag\": [\"javascript\", \"magma\"]}}"


# submit "{\"id\":$IDNUM,\"method\":\"search\",\"params\":{\"searchin\": 0, \"queries\": [{ \"field\": \"from\", \"filter\": \"contains\", \"query\": \"Paul Grahm\" }, { \"field\": \"date\", \"range\": { \"from\": 23112342342, \"to\": 2342342343 } }]}}"
# submit "{\"id\":$IDNUM,\"method\":\"scrape.add\",\"params\":{\"messageID\": 203, \"id\": 59, \"name\": \"John Doe\", \"email\": \"jdoe@example.com\"}}"
# submit "{\"id\":$IDNUM,\"method\":\"scrape\",\"params\":{\"messageID\": 203}}"
# submit "{\"id\":$IDNUM,\"method\":\"attachments.add\",\"params\":{\"composeID\": 45, \"filename\": \"script.js\"}}"
# submit "{\"id\":$IDNUM,\"method\":\"attachments.progress\",\"params\":{\"attachmentID\": 34}}"
# submit "{\"id\":$IDNUM,\"method\":\"attachments.remove\",\"params\":{\"attachmentID\": 34}}"
# submit "{\"id\":$IDNUM,\"method\":\"contacts.edit\",\"params\":{\"contactID\": 58, \"name\": \"Tim Burns-Lee\", \"email\": \"tim@cern.org\"}}"
# submit "{\"id\":$IDNUM,\"method\":\"contacts.list\",\"params\":{\"folderID\": 3}}"
# submit "{\"id\":$IDNUM,\"method\":\"contacts.load\",\"params\":{\"contactID\": 88}}"
# submit "{\"id\":$IDNUM,\"method\":\"contacts.remove\",\"params\":{\"contactID\": 88}}"
# submit "{\"id\":$IDNUM,\"method\":\"ad\",\"params\":{\"context\":\"loading\"}}"
submit "{\"id\":$IDNUM,\"method\":\"logout\"}"

rm -f "$COOKIES"


