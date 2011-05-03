#!/bin/bash
# qkill-qms.sh looks for a rogue process (using 'ps' and 'grep') that
# are running on the specified display.  If it finds one, it kills it.
# Dead.
# 
# I was considering figuring out which user had run it, and killing
# it by 'logging in' as that user and using 'vncserver -kill', but:
#    - I don't see a benefit
#    - for some weird reason, some of these processes don't show a
#      username in 'ps', just a uid.
# In some cases, it would be possible to simply remember which user
# execed vncserver.  But not always, eg: when 'RestartForums.sh' is
# invoked.  See 'qkill-qms-old.sh' for the remnants/beginnings of 
# such an approach.

# for logging
# Set debugging off=FALSE, on=anything else
debug="TRUE"
utilsFile="/home/openqwaq/server/apps/scripts/qutils.sh"

if [ ! -f $utilsFile ]; then
  log() {
    msg=$1
    echo "${myDate}: ${msg}"
  }
else
  . $utilsFile
fi

log "###############"
log "Starting qkill-qms"
log "Args: $*"
log ""

# Make sure that the necessary argument is provided
Q_DISPLAY=""
while getopts d: o
  do 
  case "$o" in
      d) Q_DISPLAY="$OPTARG";; 
  esac
done
if [ "$Q_DISPLAY" = "" ]; then
    log "USAGE: $0 -d <display-integer>"
    exit 1
fi

# Get rid of any rouges...
pkill -9 -f "firefox-bin -P QwaqProfile-$Q_DISPLAY" > /dev/null 2>&1
pkill -9 -f "acroread --screen=$Q_DISPLAY" > /dev/null 2>&1
officePIDs=`ps aux|grep -v grep|grep "ooffice/users/screen${Q_DISPLAY}"|grep soffice.bin|awk '{print $2}'`
for pid in $officePIDs; do
  log "qkill-qms: destroy ${pid}"
  kill -9 $pid > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    log "Could not kill process $pid. Will try again."
    sleep 1
    kill -9 $pid > /dev/null 2>&1
    if [ $? -ne 0 ]; then
      log "   Failed to kill off ${pid}! Error is $?"
    fi
  fi
done

pkill -f "ooffice/users/screen$Q_DISPLAY" > /dev/null 2>&1
pkill -f "Xvnc :$Q_DISPLAY" > /dev/null 2>&1
pkill -f "Xvnc :$Q_DISPLAY" > /dev/null 2>&1

log "End qkill-qms"
log ""
# end
