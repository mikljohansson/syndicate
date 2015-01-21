#!/bin/sh
# Exit codes defined in sysexits.h, use from /etc/aliases (Postfix pipe transport) to pipe emails to the inventory module as:
# agent: "|/usr/local/synd/core/module/inventory/agent/pipe_agent_email.sh http://www.example.com/synd/inventory/agent/"

result=`/usr/bin/curl -f -s -w " %{http_code}" -H "Content-Type: text/xml" --data-binary @- $1`
if [ 0 != $? ] || [ 2 != `echo -n "$result" | head -n 1 | cut -b 1` ]; then
	case `echo -n "$result" | head -n 1 | cut -d " " -f 2` in
		400)
			exit 65		# EX_DATAERR, bounce with data format error
			;;
		403)
			exit 77		# EX_NOPERM, bounce with permission denied
			;;
		404)
			exit 67		# EX_NOUSER, bounce with addressee unknown
			;;
	esac
	exit 75				# EX_TEMPFAIL, defer delivery
fi
