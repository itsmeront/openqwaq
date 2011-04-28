#!/bin/bash

# Use 'mount --bind' to make directories available elsewhere 
# in the filesystem hierarchy.  Avoids mounting multiple times
# Fails if mount point is already used for a different mount source.  

# LIMITATIONS:
# May not work if directory paths have spaces in them.

# for logging
# Set debugging off=FALSE, on=anything else
debug="TRUE"
tilsFile="/home/openqwaq/server/apps/scripts/qutils.sh"

if [ ! -f $utilsFile ]; then
  log() {
    msg=$1
    echo "${myDate}: ${msg}"
  }
else
  . $utilsFile
fi

log "###################"
log "Starting qmount\n"
log "Args are: $*"
log ""

SRC=""
TGT=""

# Gather arguments
while getopts s:t:rw o
  do 
  case "$o" in
      s) SRC="$OPTARG";;
      t) TGT="$OPTARG";;
  esac
done

# Ensure that all necessary arguments have been provided
if [ "$SRC" = "" ]; then
    log "qmount.sh: Must specify source directory (use '-s <source-dir>')"
    exit 1
fi
if [ "$TGT" = "" ]; then
    log "qmount.sh: Must specify target directory (use '-t <target-dir>')"
    exit 1
fi

# Get the full paths of the arguments.  This allows the script to be used
# like: './qmount.sh -s ../../bin/foo-src -t ./path/to/mount/foo-target'
if [ ! -d "${SRC}" ]; then
    log "\nqmount.sh: Cannot obtain full source path: $SRC\n"
    exit 1
fi
if [ ! -d "${TGT}" ]; then
    log "\nqmount.sh: Cannot obtain full target path: $TGT\n"
    exit 1
fi

# You wouldn't think that anyone would do this, but I did...
# so now I test for it.
if [ "${SRC}" = "${TGT}" ]; then
    log "\nqmount.sh: ERROR: SOURCE and TARGET are equal\n"
    exit 1
fi

# Test if SOURCE mounted at TARGET; don't mount it twice
if [ "x`mount|grep \"${SRC}\"`" == "x" ]; then
	log "MOUNTING $SRC at $TGT"
	mount -o bind "$SRC" "$TGT"
	exit 0
else
	log "Source directory is already mounted.  Exiting."
	exit 0
fi

#end
