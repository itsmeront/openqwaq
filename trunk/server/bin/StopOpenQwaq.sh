#!/bin/bash

if [ $# -ne 2 ] || [ "${1}" != "service" ] || [ "${2}" == "" ]; then
  echo "This command can not be called directly. Use \"/sbin/service openqwaq [start][stop]\". Exiting! "
  exit 1
fi

serverDir="${2}"

echo "    Killing all server scripts"
runServerPids=`ps aux|grep RunServer|grep ${serverDir}|grep -v grep|awk '{print $2}'`
for pid in $runServerPids; do
  kill -9 $pid
done

echo "    Killing all server instances"
serverPids=`ps aux|grep squeak|grep ${serverDir}|grep -v grep|awk '{print $2}'`
for pid in $serverPids; do
  kill -1 $pid > /dev/null
done


#HERE=`dirname $0`
echo "	Killing all QMS apps"
${serverDir}/apps/scripts/qkillall-qms >> /dev/null
${serverDir}/apps/scripts/qumount-all
