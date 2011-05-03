#!/bin/bash
# Unmount all directories mounted for the specified user account

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

log "#################"
log "Starting qumount"
log "Args: $*"

ACCOUNT=""
# Gather arguments
while getopts a: o
  do 
  case "$o" in
      a) ACCOUNT="$OPTARG";;
  esac
done

if [ "$ACCOUNT" = "" ]; then
    log "qumount.sh: Must specify account name (use '-a <account-name>')"
    exit 1
fi

# passed only the account name, figure out where it is mounted
MOUNTED_DIRS=$( cat /proc/mounts|grep $ACCOUNT|awk '{print $2}' )
declare -a probList

for dir in $MOUNTED_DIRS; do
    log "UNMOUNTING DIRECTORY: $dir"
    umount "$dir" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
      log "	FAILED to unmount ${dir}.  A another attempt will be made."
      probList=( "${probList[@]}" "${dir}" )
    fi
done


if [ "x${probList[@]}" != "" ]; then
  # Need time for applications to quit
  sleep 5
  for pDir in "${probList[@]}"; do
    log "UNMOUNTING DIRECTORY (second pass): ${pDir}"
    umount "$pDir" >> $logFile 2>&1
    if [ $? -ne 0]; then
      log "	FAILED second time to umount ${pDir}! Giving up.  Verify that this directory is not held up by an application."
    fi
  done
fi 
# end
