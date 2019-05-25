#!/bin/bash

set -e

pushd ~/sneezymud-docker/sneezymud/_Setup-data/sql_data
for database in *; do
	pushd $database
	for i in *; do
		table=`echo $i | sed s/.sql//`
		docker exec sneezy mysqldump -hdb -u sneezy --password=password $database $table | sed 's/),(/),\n(/g' > $i
	done
	popd
done
popd
