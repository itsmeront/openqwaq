#!/bin/bash

# qkillall-qms.sh iterates through the specified range of display numbers, 
# and kills all spawned apps.

#for logging
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
log "Starting qkillall-qms"
log ""

LOW_SCREEN=10
HIGH_SCREEN=60
echo "Killing spawned QMS processes on all displays (firefox, acroread, etc.)"
for ((DISPLAY_NUM=LOW_SCREEN; DISPLAY_NUM <= HIGH_SCREEN; DISPLAY_NUM++)); do
    log "   killing processes on display $DISPLAY_NUM"
    `dirname $0`/qkill-qms.sh -d $DISPLAY_NUM &
done

