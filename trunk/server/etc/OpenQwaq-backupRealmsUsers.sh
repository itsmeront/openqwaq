#!/bin/bash

# do an rsync of /home/openqwaq/realms and /home/openqwaq/users to backup
# directory /home/openqwaq/backup
# Backup src -> target

# Location of backup directory
backupDir="/home/openqwaq/backup"

# Location of backup logs
backupLog="${backupDir}/Logs"

# Log names
backRealmsLog="${backupLog}/backupRealms.log.${myDate}"
backUsersLog="${backupLog}/backupUsers.log.${myDate}"

# Src realms and users directories
backupRealmsSrc="/home/openqwaq/realms/"
backupUsersSrc="/home/openqwaq/users/"

# Target realms and users directories
backupRealmsTarget="${backupDir}/realms/"
backupUsersTarget="${backupDir}/Users/"

# Who should get mails
mailRcpts="user@openqwaq.com"

myDate=`date +%Y%m%d-%H%M%S`

# Begin
# Validate backuip dir exists, create it if needed.
if [ ! -d "${backupDir} ]; then
  mkdir -p "${backupDir}"
if [ -f $backRealmsLog ]; then
  rm $backRealmsLog
  touch $backRealmsLog
fi

dateNow=`date +%Y%m%d-%H%M%S`
echo "Start time: ${dateNow}" >> ${backRealmsLog}

rsync -avulzt --exclude='/home/openqwaq/realms/server.xml' ${backupRealmsSrc} ${backupRealmsTarget} >> $backRealmsLog 2>&1

# Stop time
dateNow=`date +%Y%m%d-%H%M%S`
echo "---------------" >> $backRealmsLog
echo "Stop time: ${dateNow}"  >> $backRealmsLog

if [ $? -ne 0 ]; then
  mail ${mailRcpts} -s "There was an issue backing up ${backupRealmsSrc}" < $backRealmsLog
fi

if [ -f $backUsersLog ]; then
  rm $backUsersLog
  touch $backUsersLog
fi
dateNow=`date +%Y%m%d-%H%M%S`
echo "Start time: ${dateNow}" >> ${backUsersLog}

rsync -avulzt ${backupUsersSrc} ${backupUsersTarget} >> ${backUsersLog} 2>&1

# Finish time
dateNow=`date +%Y%m%d-%H%M%S`
echo "---------------------" >> ${backUsersLog}
echo "Stop time: ${dateNow}" >> ${backUsersLog}

if [ $?  -ne 0 ]; then
  mail ${mailRcpts} -s "There was an issue backing up ${backupUsersSrc}" < ${backUsersLog}
fi

# end
