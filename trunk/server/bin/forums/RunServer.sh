#!/bin/bash

HERE=`dirname $0`
VMPATH=${HERE}/../qwaqvm

ETH0=`/sbin/ifconfig eth0 | grep 'inet addr' | cut -c21-`

# Server must never be run by anyone other than openqwaq (1234)?
UserID=`id -u openqwaq`
myID=`id -u`
if [ "$myID" != "$UserID" ]; then
   echo ""
   echo "	Only user openqwaq can run OpenQwaq Server!"
   echo "	 The UID ${myID} is incorrect.  Exiting!"
   exit
fi

# Starts up the forums server.

if test "$1" = ""; then
    echo "Usage: $0 configfile"
    exit
fi

# We want shared documents created by the service to be 
#    -rw-rw-r-- == 664
# so that they are writeable by group 'openqwaq'.
# The appserver runs OpenQwaq MultiShare processes in a per-teleplace
# proxy account with group 'openqwaq', and those processes need
# to be able to write the shared files.
umask 002

while true; do
    OUTFILE=${HERE}/../../logs/`basename $1 .conf`.`date +%F`.out
    echo `date` "Starting OpenQwaq server component: $*" >> $OUTFILE
    $VMPATH/squeak -plugins $VMPATH/plugins -vm-display-null \
	-mmap 512M ${HERE}/server.image \
        -defaultServerName: "$ETH0" \
        -configFile: $* >> $OUTFILE
    sleep 5
done
