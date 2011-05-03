#!/bin/bash

if [ $# -ne 2 ] || [ "${1}" != "service" ] || [ "${2}" == "" ]; then
  echo "This command can not be called directly. Use \"/sbin/service openqwaq [start][stop]\". Exiting! "
  exit 1
fi

serverDir="${2}"

${serverDir}/bin/StopOpenQwaq.sh service ${serverDir}

echo "VM Locale (locale | grep LC_CTYPE); we expect xy_XY.UTF-8"
/usr/bin/locale | /bin/grep LC_CTYPE

echo "Launching Service Provider services"
echo "Launching Router services"
echo "Launching Web services"
echo "Launching Proxy services"
${serverDir}/bin/forums/RunServer.sh ${serverDir}/conf/server.conf -vncPort: 5999 -startServiceProvider -startRouterServices -startWebServices -startWebcastServices -startClientProxyServices &

echo "Launching Application services"
${serverDir}/bin/forums/RunServer.sh ${serverDir}/conf/server.conf -vncPort: 5998 -startAppServices &

echo "Launching Video services"
${serverDir}/bin/forums/RunServer.sh ${serverDir}/conf/server.conf -vncPort: 5997 -startVideoServices &

#echo "Launching Broadcast services"
#${serverDir}/bin/forums/RunServer.sh ${serverDir}/conf/server.conf -vncPort: 5996 -startBroadcastServices &
