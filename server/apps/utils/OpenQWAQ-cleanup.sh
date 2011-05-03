#!/bin/bash

# Use this script to clean up the chroot environment
# This script will umount all chroot mounted directories and then completely
# remove the chroot environment.  If an umount fails, the directory will NOT
# be removed (preventing loss of data).

# assume chroot directory
chrootDir="/home/openqwaq/tmp"

#
usage () {
  echo -n "This script can be used to clean up the chroot environment directories."
  echo -n "  You must run this script as root or through sudo."
  echo "   This script accepts two options: "
  echo  ""
  echo "	-d	Base directory for the chroot environement directories."
  echo "		Defaults to /var/tmp."
  echo ""
  echo "	-n	Name of the chroot environment directory.  Use this to"
  echo "		specify a single directory to remove. Defaults to all "
  echo "		chroot directories found in the base directory."
  echo ""
  echo "Exiting"
  echo ""
  exit 1
}


chrootName="QF-*"
dirList=""

# Make sure this is run as root or root equiv
if [ `id -u` -ne 0 ]; then
  echo "	This script must be run as root or through sudo."
  usage
fi

# Parse options
while getopts ":d:n:" Opt
  do
   case $Opt in
     d)	chrootDir="$OPTARG"
	echo "	Base directory is ${chrootDir}"
	;;
     n) chrootName="$OPTARG"
	echo "Cleaning up directory ${chrootName}"
	;;
     *) usage;;
   esac
done

# Get list of chroot environments
dirList=`ls -d ${chrootDir}/${chrootName} 2>/dev/null`
if [ ! -z "$dirList" ]; then
 for dir in $dirList
   do
    error=""
    echo "   Processing ${dir}..."

    # This is pretty ugly, but alas, I have not found anything else yet 
    # that works :(
    mnts=`cat /proc/mounts|grep ${dir}|awk '{print $2}'`

    if [ "$mnts" != "" ]; then
      for dir in $mnts; do
	echo "	Unmounting ${dir}"
        umount $dir

    	if [ $? -ne 0 ]; then
	  echo "		ERROR: There was an error processing umounts for directory ${dir}.  Skipping."
	  error="FAIL"
	else
 	  echo "	Successfully completed umounting for ${dir}"
	fi
      done
     if [ "$error" != "FAIL" ]; then
       echo "     Removing directory ${dir}"
       su openqwaq -c "rm -rf ${dir}"
     else
       echo "	Failed to compleetely remove mounts in directory ${dir}. Not removing the chroot environment."
     fi
    else
     # let's remove the dir now
     echo "	Removing directory ${dir}"
     su openqwaq -c "rm -rf ${dir}"
    fi
 done
else
  echo "	Found no chroot environments in directory ${chrootDir}."
  exit
fi

echo "Completed cleanup process."

# end
