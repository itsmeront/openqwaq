#!/bin/bash
MAILRCPT=""
LOGDIR="/home/qwaq/logs"
LOGFILES="$LOGDIR/activity.*.log"

if [ "$MAILRCPT" == "" ]; then
    echo "No recipients given; exiting..."
    exit 0
fi

# Today's log file is left out since it is not complete
today=`date +%F`
todayFile="$LOGDIR/activity.$today.log"

# In case we're on Ubuntu, etc, where mail is /usr/bin/mail..."
MAILCMD=/bin/mail
if test -x /bin/mail ; then  
    MAILCMD=/bin/mail
elif test -x /usr/bin/mail ; then 
    MAILCMD=/usr/bin/mail
fi

for logFile in $LOGFILES; do
    base=`basename $logFile .log`
    mailFile="$LOGDIR/$base.sent"
    if test ! -e $mailFile; then
	# Filter out the entries we don't want from the logs
	cat $logFile | grep -v "<Action>get</Action>" | grep -v "<Action>put</Action>" > $mailFile
	$MAILCMD -s "Qwaq Forums Log: $base" $MAILRCPT < $mailFile
	echo $mailFile
    fi
done
