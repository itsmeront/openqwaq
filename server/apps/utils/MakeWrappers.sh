#!/bin/bash

if test $# -ne 1; then
    echo "Usage: sudo $0 </full/path/to/scripts>"
    exit 1
fi

# We assume that these are absolute paths.
TARGET=$1

# The wrapper names should not have any file suffix (eg: .out);
# we rely on this later in the script.
WRAPPER_NAMES="qensure-home qlaunch-qms qkill-qms qmount qumount qumount-all qkillall-qms"

# Generate C wrappers for the scripts
export INSTALL_DIR=$TARGET
for name in $WRAPPER_NAMES; do
    echo Generating setuid wrapper for $name
    export WRAPPER_NAME=$name
    make -s $name
    if test $? -ne 0; then
	echo "      Error during compilation of wrapper: $name"
    fi
 
done

cd $TARGET
for name in $WRAPPER_NAMES; do
    echo "Updating ownership for $name[.sh]"
    chown root:openqwaq $name
    chmod 4550 $name
    chown root:openqwaq $name.sh
    chmod 0550 $name.sh
done
