#!/bin/bash

set -e

pushd ~/sneezymud-docker/sneezymud/_Setup-data/sql_data
for database in *; do
	pushd $database
	for i in *; do
		table=`echo $i | sed s/.sql//`
		docker exec sneezy mysqldump --skip-extended-insert --password=password $database $table > $i
	done
	popd
done
popd
