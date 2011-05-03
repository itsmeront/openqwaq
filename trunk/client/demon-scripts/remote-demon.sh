#/!bin/bash

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


# Runs demons on (typically) AMI (Amazon EC2) instance hosts
# by running the ~root/demon.sh    installed on the instance.
# 
# For the forum user login passed to the QwaqForums client,
# this uses the canonical test user IDs with matching passwords
#
# EG:
# remote-demon.sh ec2-67-202-18-74.compute-1.amazonaws.com  1 3
# to log in users test1..test3 on that (dynamic) ami instance.
#
# (batch.sh is normally used to run this.)
#

export USAGE='Usage: remote-demon.sh Demon-Instance-Host TestUserStartingIndex NumTestUsers'
export INSTANCE=${1:? $USAGE}
export STARTUSER=$((${2:? $USAGE}))
export NUSERS=${3:-1} 	

# Where do you want to go today? 
# These can be set in the environment (and the batch.sh uses that.)
#
HOST=${HOST:-qaserver.teleplace.com}
ORG=${ORG:-Tests}
FORUM=${FORUM:+${FORUM}}   ## I.e remains unset; demon chooses first in list."
export LOGINBASE=${LOGINBASE:-'extra%.3d'}

# Args are demon host, test user number
function invoke() {
	LOGN=`printf "${LOGINBASE}\n" ${2}` ;
	ssh -f root@$1 ./demon.sh ${LOGN}@netjam.org extra $HOST "'"$ORG"'" "'"$FORUM"'" ;
}

USER=$STARTUSER
END=$((STARTUSER+NUSERS))

echo "Logging $NUSERS demons (${LOGINBASE}${USER}...) into $HOST/$ORG ${FORUM:+forum} $FORUM"

while [ $USER -lt $END ] 
do
	invoke ${INSTANCE} $USER 
	USER=$((USER+1))
done
