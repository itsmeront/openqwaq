#!/bin/bash

# Project OpenQwaq
#
# Copyright (c) 2005-20011, Teleplace, Inc., All Rights Reserved
#
# Redistributions in source code form must reproduce the above
# copyright and this condition.
#
# The contents of this file are subject to the GNU General Public
# License, Version 2 (the "License"); you may not use this file
# except in compliance with the License. A copy of the License is
# available at http://www.opensource.org/licenses/gpl-2.0.php.


#
# Sample robot-running script.
# To be installed in ~qwaq of Compute-Clound instances to allow remote launch.
#
# The arguments are positional:
#   	demon.sh    Forum-User-Name Forum-User-Password Server-DNS-Name Organization-Name Forum-Name
#
# To allow trampoline use via SSH, the underlying forum password
# is passed as an argument; this is not private, so only test user ids should be started this way.
#
# This can be run in a Linux setup if a Forums zip distribution,
# and its nested qwaqvm.tar.gz, have been unpacked into expected places.
#
# DHOME is where we expect to find ./bin/qwaqvm  and ./forums
# (DHOME is usually just HOME)
# Unzip a Telepace 3.0.10  or later into 
#	~/teleplace
# and unpack the unix VM into 
#	~/bin/qwaqvm
 
export DHOME=${DHOME:-~}

#-------------------------------------------------------------------------------
#
export VM="${DHOME}/bin/qwaqvm/bin/squeak -plugins ${DHOME}/bin/qwaqvm/lib/squeak/3.9-7"
export IMAGE=${DHOME}/teleplace/Teleplace.image
#
USAGE='Usage:  demon.sh username userpassword [server] [org] [forum]'
#-------------------------------------------------------------------------------

export USER=${1:? ${USAGE}}
export PASS=${2:? ${USAGE}}
export SERVER=${3:-qaserver.qwaq.com}
export ORG=${4:-Tests}
export FORUMKEY=${5:+forum}
export FORUMARG="${5}"

#-------------------------------------------------------------------------------

$VM -memory 512m -headless ${IMAGE} -noOGL -robot: idle \
	server ${SERVER} organization ${ORG} ${FORUMKEY} "${FORUMARG}" \
	user "${USER}" password "${PASS}" 
