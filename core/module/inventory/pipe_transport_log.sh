#!/bin/bash
# Pipes a syslog-ng FIFO buffer or log file to curl, reads data 
# from STDID
#
# Example syslog-ng configuration (eg. /etc/syslog-ng/syslog-ng.conf)
#
# 	destination d_vmpsd_fifo {
# 		pipe("/var/log/vmpsd.fifo");
# 	}
#
# 	filter f_vmpsd {
# 		program(vmpsd);
# 	};
#
# 	log {
# 		source(s_local);
# 		filter(f_vmpsd);
# 		destination(d_vmpsd_fifo);
# 	};
#
# @access		public
# @package		synd.core.module
# @link			http://www.balabit.com/products/syslog-ng/	

buffer=""
count=100
interval=900

while getopts l:i:d name; do
	case "$name" in
		l)
			if [ -z "$OPTARG" ]; then
				echo >&2 "Error: Parameter '$OPTARG' must be a positive integer"
				exit 1
			fi
			count=$OPTARG
			;;
		i)
			if [ -z "$OPTARG" ]; then
				echo >&2 "Error: Parameter '$OPTARG' must be a positive integer"
				exit 1
			fi
			interval=$OPTARG
			;;
		d)
			debug=1
			;;
	esac
done
shift $(($OPTIND - 1))

if [ -z "$1" ]; then
	echo >&2 "Usage: pipe_transport_log.sh [OPTION]... URI"
	echo >&2 "Pipes syslog-ng FIFO buffer or log file to curl, reads data from STDID"
	echo >&2 "Example: pipe_transport_log.sh http://www.example.com/synd/inventory/vmps.log < vmpsd.fifo"
	echo >&2 
	echo >&2 "Optional arguments"
	echo >&2 "  -l	Specify number of lines to buffer before invoking curl (default 100 lines)"
	echo >&2 "  -i	Specify the maximum number of seconds before invoking curl (default 900 seconds)"
	echo >&2 "  -d	Enable debugging output"
	exit 1
fi

which curl > /dev/null 2>&1
if [ 0 != $? ]; then
	echo >&2 "Error: Make sure 'curl' is installed and in your \$PATH ($PATH)"
	exit 1
fi

send() {
	if [ -z "$debug" ]; then
		curl -f -s -k -H "Content-Type: text/plain" --data-binary "$2" "$1" > /dev/null
	else
		echo "$2"
		curl -s -k -H "Content-Type: text/plain" --data-binary "$2" "$1"
	fi
}

i=1
ts1=`date +%s`

while read l; do
	if [ -n "$buffer" ]; then
		buffer="$buffer
$l"
	else
		buffer=$l
	fi
	
	ts2=`date +%s`
	
	if [ $i -lt $count ] && [ $(($ts1 + $interval)) -lt $ts2 ]; then
		let i++
	else
		send "$1" "$buffer"
		buffer=""
		i=1
		ts1=ts2
	fi
done

if [ -n "$buffer" ]; then
	send "$1" "$buffer"
fi
