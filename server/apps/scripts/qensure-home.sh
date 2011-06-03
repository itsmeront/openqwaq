#!/bin/bash
# PURPOSE:
#   Setup the "change root" environment for applications.

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

chrootMounts() {
  Q_HOMEDIR=$1

  mountedDirs=$( mount|grep ${Q_HOMEDIR} )

  # Check for the presence of mounted directories
  # Deals with 64b systems
  if [ "`uname -i`" == "x86_64" ]; then
    if [ "`echo $mountedDirs|grep "/lib64 on"`" == "" ]; then
      mount -o bind,ro /lib64 ${Q_HOMEDIR}/lib64
      log "chroot environment: mounted ${Q_HOMEDIR}/lib64"
    fi
    if [ "`echo $mountedDirs|grep "/usr/lib64 on"`" == "" ]; then
      mount -o bind,ro /usr/lib64 ${Q_HOMEDIR}/usr/lib64
      log "chroot environment: mounted ${Q_HOMEDIR}/usr/lib64"
    fi
  fi

  if [ "`echo $mountedDirs|grep "/usr/bin on"`" == "" ]; then
    mount -o bind,ro /usr/bin ${Q_HOMEDIR}/usr/bin
    log "chroot environment: mounted ${Q_HOMEDIR}/usr/bin"
  fi
  if [ "`echo $mountedDirs|grep "/proc on"`" == "" ]; then
    mount -o bind,ro /proc ${Q_HOMEDIR}/proc
    log "chroot environment: mounted ${Q_HOMEDIR}/proc"
  fi
  if [ "`echo $mountedDirs|grep "^/bin\ on"`" == "" ]; then
    mount -o bind,ro /bin ${Q_HOMEDIR}/bin
    log "chroot environment: mounted ${Q_HOMEDIR}/bin"
  fi
  if [ "`echo $mountedDirs|grep "^/opt\ on"`" == "" ]; then
    mount -o bind,ro /opt ${Q_HOMEDIR}/opt
    log "chroot environment: mounted ${Q_HOMEDIR}/opt"
  fi
  if [ "`echo $mountedDirs|grep "/usr/lib on"`" == "" ]; then
    mount -o bind,ro /usr/lib ${Q_HOMEDIR}/usr/lib
    log "chroot environment: mounted ${Q_HOMEDIR}/usr/lib"
  fi
  if [ "`echo $mountedDirs|grep "^/lib\ on"`" == "" ]; then
    mount -o bind,ro /lib ${Q_HOMEDIR}/lib
    log "chroot environment: mounted ${Q_HOMEDIR}/lib"
  fi
  if [ "`echo $mountedDirs|grep "/etc/pam.d on"`" == "" ]; then
    mount -o bind,ro /etc/pam.d ${Q_HOMEDIR}/etc/pam.d
    log "chroot environment: mounted ${Q_HOMEDIR}/etc/pam.d"
  fi
  if [ "`echo $mountedDirs|grep "/etc/gre.d on"`" == "" ]; then
    mount -o bind,ro /etc/gre.d ${Q_HOMEDIR}/etc/gre.d
    log "chroot environment: mounted ${Q_HOMEDIR}/etc/gre.d"
  fi
  if [ "`echo $mountedDirs|grep "/etc/security on"`" == "" ]; then
    mount -o bind,ro /etc/security ${Q_HOMEDIR}/etc/security
    log "chroot environment: mounted ${Q_HOMEDIR}/etc/security"
  fi
  if [ "`echo $mountedDirs|grep "/usr/share on"`" == "" ]; then
    mount -o bind,ro /usr/share ${Q_HOMEDIR}/usr/share
    log "chroot environment: mounted ${Q_HOMEDIR}/usr/share"
  fi
  if [ "`echo $mountedDirs|grep "/home/openqwaq/OpenQwaqApps on"`" == "" ]; then
    mount -o bind,ro /home/openqwaq/OpenQwaqApps ${Q_HOMEDIR}/OpenQwaqApps
    log "chroot environment: mounted ${Q_HOMEDIR}/OpenQwaqApps"
  fi
  if [ "`echo $mountedDirs|grep "/etc/fonts on"`" == "" ]; then
    mount -o bind,ro /etc/fonts ${Q_HOMEDIR}/etc/fonts
    log "chroot environment: mounted ${Q_HOMEDIR}/etc/fonts"
  fi
  if [ "`echo $mountedDirs|grep "/etc/pango on"`" == "" ]; then
    mount -o bind,ro /etc/pango ${Q_HOMEDIR}/etc/pango
    log "chroot environment: mounted ${Q_HOMEDIR}/etc/pango"
  fi
  if [ "`echo $mountedDirs|grep "/etc/gtk on"`" == "" ]; then
    mount -o bind,ro /etc/gtk ${Q_HOMEDIR}/etc/gtk
    log "chroot environment: mounted ${Q_HOMEDIR}/etc/gtk"
  fi
  if [ "`echo $mountedDirs|grep "/usr/libexec on"`" == "" ]; then
    mount -o bind,ro /usr/libexec ${Q_HOMEDIR}/usr/libexec
    log "chroot environment: mounted ${Q_HOMEDIR}/usr/libexec"
  fi
  if [ "`echo $mountedDirs|grep "^/dev\ on"`" == "" ]; then
    mount -o bind,ro /dev ${Q_HOMEDIR}/dev
    log "chroot environment: mounted ${Q_HOMEDIR}/dev"
  fi

} # End chrootMounts

chrootSkel() {
  Q_HOMEDIR=$1

  # Profile setup
  rootChownList="  ${Q_HOMEDIR}/.adobe/Acrobat/9.0/Preferences/reader_prefs ${Q_HOMEDIR}/etc/resolv.conf ${Q_HOMEDIR}/etc/group ${Q_HOMEDIR}/etc/hosts ${Q_HOMEDIR}/etc/localtime ${Q_HOMEDIR}/etc/nsswitch.conf ${Q_HOMEDIR}/etc/passwd ${Q_HOMEDIR}/etc/shadow ${Q_HOMEDIR}/tmp"

  # Need this from the running system
  cp /etc/resolv.conf ${Q_HOMEDIR}/etc/
  chmod 755 ${Q_HOMEDIR}/etc/resolv.conf

  #
  if [ ! -d ${Q_HOMEDIR}/tmp ]; then
    mkdir ${Q_HOMEDIR}/tmp
  fi

  for file in $rootList; do
    chown root.root $file
  done
  for file in $rootList; do
    chmod 755 $file
  done

  chmod 400 ${Q_HOMEDIR}/etc/shadow
  chmod 644 ${Q_HOMEDIR}/etc/passwd
  chmod 1777 ${Q_HOMEDIR}/tmp
  chmod og-rwx ${Q_HOMEDIR}/.vnc/passwd

} # end chrootSkel


log "#####################"
log "Starting qensure-home"
log "Args: $*"
log ""

if [ $# -lt 1 ] || [ $# -gt 1 ]; then
  log "qensure-home must be called with a single argument, the chroot environment directory.  Exiting"
  exit 1
fi
Q_HOMEDIR=$1

CHROOT_DIR="/home/openqwaq/server/apps/chroot"
if [ ! -d ${CHROOT_DIR} ]; then
  log "\n	Can not find the default profile directory!  Exiting!\n"
  exit 1
fi


if [ ! -d ${Q_HOMEDIR} ]; then
  cp -a ${CHROOT_DIR} ${Q_HOMEDIR}
  chown openqwaq:openqwaq ${Q_HOMEDIR}
  chmod 750 ${Q_HOMEDIR}

  # Build up the files and ownerships
  chrootSkel ${Q_HOMEDIR}
  # mount all the directories in the correct locations
  log "Populating chroot environment for ${Q_HOMEDIR}"
  chrootMounts ${Q_HOMEDIR}
else
  log "Appears a skel chroot environment is already available.  Building on that environment."
  log "Populating chroot environment for ${Q_HOMEDIR}"
  chrootMounts ${Q_HOMEDIR}
fi

log ""
log "END qensure-home.sh"
log ""
#end
