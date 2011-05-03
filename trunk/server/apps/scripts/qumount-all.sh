#!/bin/bash
# umount all QMS/QAS directories

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
log "Starting qumount-all"

MOUNTED_DIRS=$( cat /proc/mounts|grep "QF-"|awk '{print $2}' )
log "Mounted directories are: ${MOUNTED_DIRS}"

for dir in $MOUNTED_DIRS; do
    log "UNMOUNTING DIRECTORY: $dir"
    umount "$dir"
    if [ $? -ne 0 ]; then
      log "	Failed to unmount: ${dir}.  More than likely a process is holding the dir open."
    fi
done

# end
