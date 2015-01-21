#!/bin/bash

while getopts d:fp: name; do
	case "$name" in
		d)
			delete="$OPTARG"
			;;
		f)
			force=1
			;;
		p)
			version="$OPTARG"
			;;
	esac
done
shift $(($OPTIND - 1))

if [ -z "$1" ] || [ "--help" == "$1" ]; then
	echo >&2 "Packages SVN snapshots into releses like program-1.0.0.tar.gz"
	echo >&2 "  Usage: svn_package.sh [OPTION]... SVNBASEURI OUTPUTDIR"
	echo >&2 "  Example: svn_package.sh -f -p 20060218 -d -p 20060217 http://svn.synd.info/synd/extensions /home/info_synd/www/htdocs/downloads/snaps/"
	echo >&2 
	echo >&2 "Optional arguments"
	echo >&2 "  -d	Remove package of this version"
	echo >&2 "  -f	Overwrite packages that already exists"
	echo >&2 "  -p	Append specific version or date to package"
	exit 1
fi

if [ ! -d "$2" ]; then 
	echo "Target directory '$2' does not exist"
	exit 1
fi

projects=/tmp/svn_package-`date +%G%m%d-%H%M%S`
svn checkout "$1" $projects >/dev/null

for name in `ls $projects`; do
	if [ -e "$projects/$name/buildconf" ]; then
		cd "$projects/$name"
		. buildconf >/dev/null 2>&1
		
		if [ ! -e "$projects/$name/configure" ]; then
			echo >&2 "$projects/$name/buildconf failed to create '$projects/$name/configure'"
			continue
		fi
		
		rm -rf autom4te.cache config.log config.status libtool Makefile
	fi

	if [ -z "$version" ]; then
		snapshot="$name"
	else
		snapshot="$name-$version"
		mv "$projects/$name" "$projects/$snapshot"
	fi
	
	if [ -z "$force" ] && [ -e "$2/$snapshot.tar.gz" ]; then
		echo >&2 "Package '$2/$snapshot.tar.gz' exists, skipping ..."
		continue
	fi
	
	tar -C$projects -czf "$2/$snapshot.tar.gz" "$snapshot"
	
	if [ ! -e "$2/$snapshot.tar.gz" ]; then
		echo >&2 "Failed to create '$2/$snapshot.tar.gz'"
		continue
	fi
	
	if [ -n "$delete" ]; then
		rm -f "$2/$name-$delete.tar.gz"
	fi
done

rm -rf $projects
