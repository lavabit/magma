#!/bin/bash

# Name: schema.dump.sh
# Author: Ladar Levison
#
# Description: Used for quickly updating the SQL scripts used to initialize a MySQL database
# suitable for use by the magma daemon. This script should be run whenever the schema has been 
# altered, or whenever the seed data used by the sandbox environment needs updating.

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=`readlink -f $BASH_SOURCE`
else
  export CMD=`readlink -f $0`
fi

# Cross Platform Base Directory Discovery
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null

cd $BASE/../../../

MAGMA_DIST=`pwd`

MYSQL_USER=${MYSQL_USER:-"mytool"}
MYSQL_PASSWORD=${MYSQL_PASSWORD:-"aComplex1"}
MYSQL_SCHEMA=${MYSQL_SCHEMA:-"Sandbox"}

# If the TERM environment variable is missing, then tput may trigger a fatal error.
if [[ -n "$TERM" ]] && [[ "$TERM" != "dumb" ]]; then
  export TPUT="tput"
else
  export TPUT="tput -Tvt100"
fi

# Check and make sure mysqld is running before attempting a connection.
PID=`pidof mysqld`       

if [ -z "$PID" ]; then
	${TPUT} setaf 1; ${TPUT} bold; echo "The MySQL server process isn't running."; ${TPUT} sgr0
	exit 2
fi

# Check and make sure the mysql command line client has been installed.
which mysql &>/dev/null
if [ $? -ne 0 ]; then
	${TPUT} setaf 1; ${TPUT} bold; echo "The mysql client command isn't available. It may need to be installed."; ${TPUT} sgr0
	exit 1
fi

# Check and make sure the mysqldump command line utility has been installed.
which mysql &>/dev/null
if [ $? -ne 0 ]; then
	${TPUT} setaf 1; ${TPUT} bold; echo "The mysqldump command line utility isn't available. It may need to be installed."; ${TPUT} sgr0
	exit 1
fi

# Clear the following tables since they either contain transient or generated data 
echo "TRUNCATE `Aliases`;" | mysql --batch -u $MYSQL_USER --password="$MYSQL_PASSWORD" "$MYSQL_SCHEMA"
echo "TRUNCATE `Signatures`;" | mysql --batch -u $MYSQL_USER --password="$MYSQL_PASSWORD" "$MYSQL_SCHEMA"
echo "TRUNCATE `Display`;" | mysql --batch -u $MYSQL_USER --password="$MYSQL_PASSWORD" "$MYSQL_SCHEMA"
echo "TRUNCATE `Impressions`;" | mysql --batch -u $MYSQL_USER --password="$MYSQL_PASSWORD" "$MYSQL_SCHEMA"
echo "TRUNCATE `Transmitting`;" | mysql --batch -u $MYSQL_USER --password="$MYSQL_PASSWORD" "$MYSQL_SCHEMA"
echo "TRUNCATE `Receiving`;" | mysql --batch -u $MYSQL_USER --password="$MYSQL_PASSWORD" "$MYSQL_SCHEMA"
echo "TRUNCATE `Creation`;" | mysql --batch -u $MYSQL_USER --password="$MYSQL_PASSWORD" "$MYSQL_SCHEMA"

mysqldump --no-create-info=TRUE --order-by-primary=TRUE --force=FALSE --no-data=FALSE --tz-utc=TRUE --flush-privileges=FALSE \
--compress=FALSE --replace=FALSE --host=localhost --insert-ignore=FALSE --user=root --quote-names=TRUE --hex-blob=TRUE --complete-insert=FALSE \
--add-locks=TRUE --port=3306 --disable-keys=TRUE --delayed-insert=TRUE --create-options=TRUE --extended-insert=TRUE \
--delete-master-logs=FALSE --comments=TRUE --default-character-set=utf8 --max_allowed_packet=1G --flush-logs=FALSE --dump-date=TRUE \
--lock-tables=TRUE --allow-keywords=TRUE --events=FALSE --user $MYSQL_USER --password="$MYSQL_PASSWORD" --databases "$MYSQL_SCHEMA" \
> res/sql/Data.sql 

mysqldump --no-create-info=TRUE --order-by-primary=TRUE --force=FALSE --no-data=FALSE --tz-utc=TRUE --flush-privileges=FALSE \
--compress=FALSE --replace=FALSE --host=localhost --insert-ignore=FALSE --user=root --quote-names=TRUE --hex-blob=TRUE --complete-insert=TRUE \
--add-locks=TRUE --port=3306 --disable-keys=TRUE --delayed-insert=TRUE --create-options=TRUE --skip-extended-insert=TRUE \
--delete-master-logs=FALSE --comments=TRUE --default-character-set=utf8 --max_allowed_packet=1G --flush-logs=FALSE --dump-date=TRUE \
--lock-tables=TRUE --allow-keywords=TRUE --events=FALSE --user $MYSQL_USER --password="$MYSQL_PASSWORD" --databases "$MYSQL_SCHEMA" \
> res/sql/Full.sql 
