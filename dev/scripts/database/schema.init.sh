#!/bin/bash

# Name: schema.init.sh
# Author: Ladar Levison
#
# Description: Used for quickly initializing the MySQL database used by the by the 
# magma daemon. This script should be only be run once in a production environment. 
# It may be used at the user's discretion against a sandbox environment.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_RES_SQL="res/sql/" 

case $# in
	0) 
    	echo "Using the default sandbox values for the MySQL username, password and schema name."
		MYSQL_USER=${MYSQL_USER:-"mytool"}
		MYSQL_PASSWORD=${MYSQL_PASSWORD:-"aComplex1"}
		MYSQL_SCHEMA=${MYSQL_SCHEMA:-"Magma"}
	;;
	*!3*)
		echo "Initialize the MySQL database used by the magma daemon."
		echo ""
		echo "Usage:    $0 \<mysql_user\> \<mysql_password\> \<mysql_schema\>"
		echo "Example:  $0 magma volcano Magma"
		echo ""
		exit 1
	;;
esac

if [ -z "$MYSQL_USER" ]; then
	if [ -z "$1" ]; then
		echo "Please pass in the MySQL username."
		exit 1
	else
		MYSQL_USER="$1"
	fi
fi

if [ -z "$MYSQL_PASSWORD" ]; then
	if [ -z "$2" ]; then
		echo "Please pass in the MySQL password."
		exit 1
	else
		MYSQL_PASSWORD="$2"
	fi
fi

if [ -z "$MYSQL_SCHEMA" ]; then
	if [ -z "$3" ]; then
		echo "Please pass in the MySQL schema name."
		exit 1
	else
		MYSQL_SCHEMA="$3"
	fi
fi

if [ ! -d $MAGMA_RES_SQL ]; then
	echo "The SQL scripts directory appears to be missing. { path = $MAGMA_RES_SQL }"
	exit 1
fi

# Generate a Start.sql file using the user-provided schema name.
cat <<-EOF > $MAGMA_RES_SQL/Start.sql
CREATE DATABASE IF NOT EXISTS \`${MYSQL_SCHEMA}\`;
USE \`${MYSQL_SCHEMA}\`;

SET SESSION sql_mode = 'ALLOW_INVALID_DATES';
SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
EOF

if [ -z "$HOSTNAME" ]; then
	HOSTNAME=$(hostname)
fi

# Generate the Hostname.sql file using the system host name.
echo "INSERT INTO \`Hosts\` (\`hostnum\`, \`hostname\`, \`timestamp\`) VALUES (1,'$HOSTNAME',NOW());" > $MAGMA_RES_SQL/Hostname.sql

# Generate Version.sql file with the correct version number.
MAGMA_VERSION=`grep --extended-regexp "^PACKAGE_VERSION" Makefile | awk --field-separator='=' '{print \$2}' | awk --field-separator='.' '{print \$1}' | tr --delete [:blank:]`
echo "INSERT INTO \`Host_Config\` (\`hostnum\`, \`application\`, \`name\`, \`value\`, \`timestamp\`) VALUES (NULL,'magmad','magma.version','$MAGMA_VERSION',NOW());" > $MAGMA_RES_SQL/Version.sql

# Tell git to skip checking for changes to these SQL files, but we only do this if git is on the system and the files
# are stored inside a repo.
GIT_IS_AVAILABLE=`which git &> /dev/null && git log &> /dev/null && echo 1`
if [[ "$GIT_IS_AVAILABLE" == "1" ]]; then
	git update-index --skip-worktree "$MAGMA_RES_SQL/Start.sql"
	git update-index --skip-worktree "$MAGMA_RES_SQL/Version.sql"
	git update-index --skip-worktree "$MAGMA_RES_SQL/Hostname.sql"
fi

# Attempt the schema reset silently first.
cat $MAGMA_RES_SQL/Start.sql \
	$MAGMA_RES_SQL/Schema.sql \
	$MAGMA_RES_SQL/Migration.sql \
	$MAGMA_RES_SQL/Hostname.sql \
	$MAGMA_RES_SQL/Version.sql \
	$MAGMA_RES_SQL/Finish.sql \
| mysql --batch --user="${MYSQL_USER}" --password="${MYSQL_PASSWORD}" &> /dev/null

# if the query fails we run it again, verbosely, and print the last 20 lines output, so we know what caused the error.
if [ $? != 0 ]; then
	printf "\n"
	cat $MAGMA_RES_SQL/Start.sql \
	$MAGMA_RES_SQL/Schema.sql \
	$MAGMA_RES_SQL/Migration.sql \
	$MAGMA_RES_SQL/Hostname.sql \
	$MAGMA_RES_SQL/Version.sql \
	$MAGMA_RES_SQL/Finish.sql \
	| mysql --verbose --user="${MYSQL_USER}" --password="${MYSQL_PASSWORD}" | tail -4
	tput setaf 1; printf "Schema Reset Failed\n\n"; tput sgr0
	exit 1
fi

# Done.
tput setaf 2; printf "Done.\n"; tput sgr0
