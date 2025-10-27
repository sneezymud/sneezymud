#!/bin/bash

set -e

pushd ~/sneezymud-docker/sneezymud/_Setup-data/sql_data
for database in *; do
	pushd $database
	for i in *; do
		table=`echo $i | sed s/.sql//`
		# Use mariadb-dump for consistency with modern MariaDB containers
		# Remove timestamp comments to avoid unnecessary diffs
		docker exec sneezy-db mariadb-dump -h sneezy-db -u sneezy --password=password $database $table | \
			sed 's/),(/),\n(/g' | \
			grep -v '^-- Dump completed on' > $i
	done
	popd
done
popd
