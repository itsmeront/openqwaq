# Teleplace, Inc 2010

# Basic log function
log() {

  myDate=`date +%Y%m%d-%H.%M.%S`
  myLogDate=`date +%Y.%m.%d`
  msg=$1

  if [ "$debug" != "FALSE" ]; then
    if [ -z "$logFile" ]; then
      logFile="/home/openqwaq/server/logs/QCmdScripts-${myLogDate}.log"
    fi
    if [ ! -f "$logFile" ]; then
      touch "$logFile" > /dev/null 2>&1
      chown openqwaq.openqwaq "${logFile}" > /dev/null 2>&1
      chmod 750 "${logFile}" > /dev/null 2>&1
      if [ $? -ne 0 ]; then
        echo "  Could not create file $logFile."
        echo "  Defaulting to terminal echos."
        debug="FALSE"
      fi
    else
      touch "$logFile" > /dev/null 2>&1
      chown openqwaq.openqwaq "${logFile}" > /dev/null 2>&1
      chmod 750 "${logFile}" > /dev/null 2>&1
      if [ $? -ne 0 ]; then
        echo "  Could append to $logFile."
        echo "  Defaulting to terminal echos."
        debug="FALSE"
      fi
    fi
  fi

  if [ "$debug" != "FALSE" ]; then
    echo "${myDate}: ${msg}" >> $logFile 2>&1
  else
    echo "${myDate}: ${msg}"
  fi

}
