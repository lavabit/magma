#!/bin/bash

# Clear the following tables since they either contain transient or generated data 
echo "TRUNCATE `Aliases`;" | mysql --batch -u mytool --password=aComplex1
echo "TRUNCATE `Signatures`;" | mysql --batch -u mytool --password=aComplex1
echo "TRUNCATE `Display`;" | mysql --batch -u mytool --password=aComplex1   
echo "TRUNCATE `Impressions`;" | mysql --batch -u mytool --password=aComplex1
echo "TRUNCATE `Transmitting`;" | mysql --batch -u mytool --password=aComplex1
echo "TRUNCATE `Receiving`;" | mysql --batch -u mytool --password=aComplex1
echo "TRUNCATE `Creation`;" | mysql --batch -u mytool --password=aComplex1

mysqldump --no-create-info=TRUE --order-by-primary=TRUE --force=FALSE --no-data=FALSE --tz-utc=TRUE --flush-privileges=FALSE \
--compress=FALSE --replace=FALSE --host=localhost --insert-ignore=FALSE --user=root --quote-names=TRUE --hex-blob=TRUE --complete-insert=FALSE \
--add-locks=TRUE --port=3306 --disable-keys=TRUE --delayed-insert=TRUE --create-options=TRUE --extended-insert=TRUE \
--delete-master-logs=FALSE --comments=TRUE --default-character-set=utf8 --max_allowed_packet=1G --flush-logs=FALSE --dump-date=TRUE \
--lock-tables=TRUE --allow-keywords=TRUE --events=FALSE --user mytool --password=aComplex1 --databases "Lavabit" \
> $HOME/Lavabit/magma.universe/sql/active/Data.sql 

mysqldump --no-create-info=TRUE --order-by-primary=TRUE --force=FALSE --no-data=FALSE --tz-utc=TRUE --flush-privileges=FALSE \
--compress=FALSE --replace=FALSE --host=localhost --insert-ignore=FALSE --user=root --quote-names=TRUE --hex-blob=TRUE --complete-insert=TRUE \
--add-locks=TRUE --port=3306 --disable-keys=TRUE --delayed-insert=TRUE --create-options=TRUE --skip-extended-insert=TRUE \
--delete-master-logs=FALSE --comments=TRUE --default-character-set=utf8 --max_allowed_packet=1G --flush-logs=FALSE --dump-date=TRUE \
--lock-tables=TRUE --allow-keywords=TRUE --events=FALSE --user mytool --password=aComplex1 --databases "Lavabit" \
> $HOME/Lavabit/magma.universe/sql/active/Full.sql 
