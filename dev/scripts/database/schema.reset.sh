#!/bin/bash

# Name: schema.reset.sh
# Author: Ladar Levison
#
# Description: Used for quickly reinitializing the MySQL database used by the by the 
# magma daemon. This script should be only be run in a development environment. 
# It may be used at the user's discretion against a development sandbox.

LINK=`readlink -f $0`
BASE=`dirname $LINK`

cd $BASE/../../../

MAGMA_RES_SQL="res/sql/" 
MAGMA_RES_TANKS="sandbox/storage/tanks/"
MAGMA_RES_STORAGE="sandbox/storage/local/0/"

case $# in
	0) 
    	echo "Using the default sandbox values for the MySQL username, password and schema name."
		MYSQL_USER=${MYSQL_USER:-"mytool"}
		MYSQL_PASSWORD=${MYSQL_PASSWORD:-"aComplex1"}
		MYSQL_SCHEMA=${MYSQL_SCHEMA:-"Sandbox"}
	;;
	*!3*)
		echo "Initialize the MySQL database used by the magma daemon."
		echo ""
		echo "Usage:    $0 \<mysql_user\> \<mysql_password\> \<mysql_schema\>"
		echo "Example:  $0 magma volcano Sandbox"
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

if [ ! -d $MAGMA_RES_TANKS ]; then
	echo "The storage tank directory appears to be missing. It will be created. { path = $MAGMA_RES_TANKS }"
	mkdir --parents $MAGMA_RES_TANKS
fi

if [ ! -d $MAGMA_RES_STORAGE ]; then
	echo "The local storage directory appears to be missing. It will be created. { path = $MAGMA_RES_STORAGE }"
fi

# Generate a Start.sql file using the user-provided schema name.
cat <<-EOF > $MAGMA_RES_SQL/Start.sql
DROP DATABASE IF EXISTS \`${MYSQL_SCHEMA}\`;
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

# Add -vvv to the mysql command line option when the batch fails to assist in troubleshooting.
cat $MAGMA_RES_SQL/Start.sql \
	$MAGMA_RES_SQL/Schema.sql \
	$MAGMA_RES_SQL/Migration.sql \
	$MAGMA_RES_SQL/Hostname.sql \
	$MAGMA_RES_SQL/Version.sql \
	$MAGMA_RES_SQL/Data.sql \
	$MAGMA_RES_SQL/Finish.sql \
| mysql --batch -u "${MYSQL_USER}" --password="${MYSQL_PASSWORD}"

# Remove the storage tanks.
rm --force "$MAGMA_RES_TANKS/system.data"
rm --force "$MAGMA_RES_TANKS/tank.1.data"
rm --force "$MAGMA_RES_TANKS/tank.2.data"
rm --force "$MAGMA_RES_TANKS/tank.3.data"
rm --force "$MAGMA_RES_TANKS/tank.4.data"

# Remove the local storage folder.
rm --recursive --force "$MAGMA_RES_STORAGE"
mkdir --parents "$MAGMA_RES_STORAGE"


