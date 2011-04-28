#!/bin/bash

# Location of backup directory
backupDir="/home/openqwaq/backup"
if [ ! -d "{$backupDir}" ]; then
  mkdir -p "${backupDir}"
fi

# Location of backup logs
backupLogDir="/home/openqwaq/server/logs/backup"
if [ ! -d "${backupLogDir}" ]; then
  mkdir "${backupLogDir}"
fi

# Number of days to keep back
keep=7

# DB root user and password
user=root
passwd=openqwaq

# How many backup currently exist
declare -i numBackups
numBackups=`ls ${backupDir}/OpenQwaqDB*.tbz2|wc -l`
if [ $? -ne 0 ]; then
  numBackups=0
fi

cleanDBBackup() {
  rmBackup=`find "${backupDir}/OpenQwaqDB-*.tbz2" -mtime +1`
  
}

dbRunState=`ps aux|grep mysqld|grep -v grep`
if [ "x`ps aux|grep mysqld|grep -v grep`" != "x" ]; then
  declare -i remainDB
  echo "Numbackups: $numBackups"
  remainDB=$(($numBackups - $keep))
  echo "Remaining DBs: $remainDB"
  while [ $remainDB -ge 0 ]; do
    cleanDBBackup
    remainDB=$(($numBackups - $keep))
  done

  myDate=`date +%Y-%m-%d`
  
  # Becuaes we are using InnoDB, we have to dump the db as a single transaction
  # This operation can be expensive especially on larger databases, so set
  # the backup time appropriately
  mysqldump --all-databases --flush-logs --single-transaction -u$user -p$passwd > "${backupDir}/OpenQwaqDB-${myDate}.sql"
  tar jcspf "${backupDir}/OpenQwaqDB-${myDate}.sql.tbz2" "${backupDir}/OpenQwaqDB-${myDate}.sql"

  if [ $? -eq 0 ]; then
    rm -f "${backupDir}/OpenQwaqDB-${myDate}.sql"
  fi

fi # end backup routine

# end
