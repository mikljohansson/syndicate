#!/bin/sh
# Exit codes defined in sysexits.h, use from /etc/aliases (Postfix 
# pipe transport) to pipe emails to the issue module as:
#
# issues: "|/usr/local/synd/core/module/issue/pipe_issue_mail.sh http://www.example.com/synd/issue/mail/project.123/"
#
# The receiving script must output "200" to indicate it has accepted
# delivery of the email.

# Pipe the mail straight to curl by default
result=`/usr/bin/curl -f -s -k --location-trusted -w " %{http_code}" -H "Content-Type: text/plain" --data-binary @- "$1"`

# This example would filter the email though a locally trained 
# spamassassin filter (/var/spamd/local). The issue module will 
# discard messages having the "X-Spam-Status" set to "Yes" sent
# from addresses the system haven't seen before

#result=`spamc | /usr/bin/curl -f -s -k --location-trusted -w " %{http_code}" -H "Content-Type: text/plain" --data-binary @- "$1"`

if [ 0 != $? ] || [ "2" != "`echo -n \"$result\" | head -n 1 | cut -b 1`" ]; then
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
