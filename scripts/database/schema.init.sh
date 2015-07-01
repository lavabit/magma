#!/bin/bash

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../

MAGMA_DIST=`pwd`

readonly PROGNAME=$(basename $0)

usage () {
	cat <<- EOF
	Usage: $PROGNAME <mysql_user> <mysql_password> <mysql_schema>
	
	Resets the database to factory defaults
	
	Example: $PROGNAME magma volcano Lavabit
	
	EOF
}

if [ -z "$MYSQL_USER" ]; then
	if [ -z "$1" ]; then
		usage
		echo "Please pass in the MySQL Username"
		exit 1
	else
		MYSQL_USER="$1"
	fi
fi

if [ -z "$MYSQL_PASSWORD" ]; then
	#if [ -z "$2" ]; then
	#	usage
	#	echo "Please pass in the MySQL Password for $MYSQL_USER"
	#	exit 1
	#else
		MYSQL_PASSWORD="$2"
	#fi
fi

if [ -z "$MYSQL_SCHEMA" ]; then
	if [ -z "$3" ]; then
		usage
		echo "Please pass in the MySQL Schema"
		exit 1
	else
		MYSQL_SCHEMA="$3"
	fi
fi

SQL="./res/sql/"

if [ ! -d "$SQL" ]; then
	echo "Can't find directory with *.sql files: $SQL"
	exit 1
fi

# Generate Start.sql from the user-provided Schema
echo "DROP DATABASE IF EXISTS \`${MYSQL_SCHEMA}\`;
CREATE DATABASE IF NOT EXISTS \`${MYSQL_SCHEMA}\`;
USE \`${MYSQL_SCHEMA}\`;

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;" > $SQL/Start.sql

if [ -z "$HOSTNAME" ]; then
	HOSTNAME=$(hostname)
fi

# Generate Hostname.sql with the system's Hostname
echo "UPDATE Hosts SET hostname = '$HOSTNAME' WHERE hostnum = 1;" > $SQL/Hostname.sql

cat $SQL/Start.sql \
	$SQL/Schema.sql \
	$SQL/Data.sql \
	$SQL/Migration.sql \
	$SQL/Finish.sql \
	$SQL/Hostname.sql \
	| mysql --batch -u ${MYSQL_USER} --password=${MYSQL_PASSWORD}
