#!/bin/bash

if [ -z "$1" ] || [ "--help" == "$1" ]; then
	echo >&2 "Dumps the databases of a MySQL installation into SQL files"
	echo >&2 "  Usage: backup_mysql.sh [OPTION]... DATADIR OUTPUTDIR"
	echo >&2 "  Example: backup_mysql.sh /var/lib/mysql /backup/mysql"
	exit 1
fi

if [ ! -d "$1" ]; then
	echo "MySQL data directory '$1' does not exist" 
	exit 1
fi

if [ ! -d "$2" ]; then
	echo "Backup directory '$2' does not exist" 
	exit 1
fi

target="$2/`date +%G%m%d-%H%M`"
if [ ! -d $target ]; then
	mkdir $target
fi

for f in `find $1/* -type d`; do
	name=`basename $f`
	if [ "." == "${name:0:1}" ]; then
		continue
	fi

	mysqldump -K --single-transaction $name > "$target/$name.sql"

	if [ 0 == $? ]; then
		bzip2 --best -z "$target/$name.sql"
	fi
done
