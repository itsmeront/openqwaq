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


# Runs a bunch of remote-demon.sh batches of demon clients
# for a list of compute cloud instances.
#
# Reads the hosts out of ./instances,
# which should have one ECS AMI instance (domain name) per line.
#
export USAGE='batch.sh num-demons-per-host [starting-demon-number] [forum-host] [forum-organization] [forum]'
# 
#
if [ ${#@} -eq 0 ]; then 
	echo $USAGE
	exit 0
fi

TESTHOSTS=( $(< ./instances) )

# These are exported to remote-demon.sh
#
export HOST=${3:-${HOST:-qaserver.teleplace.com}}
export ORG=${4:-${ORG:-Tests}}
export FORUM=${5:-${FORUM:+${FORUM}}}

NHOSTS=${#TESTHOSTS[@]}
NDEMONS_PER_HOST=${1:-1}
USTART=${2:-2}

if [ $NHOSTS -eq 0 ]; then 
	echo "Hosts lists ./instances contains no hosts."
	exit -1
fi

# Make sure the numbers are numbers
export LOGINBASE=${LOGINBASE:-'extra%.3d'}
NDEMONS_PER_HOST=$((NDEMONS_PER_HOST+0))
USTART=$((USTART+0))

echo "Sending $((NDEMONS_PER_HOST * NHOSTS)) demons (${LOGINBASE}${USTART}...) to $HOST/$ORG"
echo `date` > ./roster.txt
N=0
while [ $N -ne $NDEMONS_PER_HOST ]
do
	M=0	
	while [ $M -lt $NHOSTS ] 
	do
		TN=$((M*NDEMONS_PER_HOST + N + USTART))
		echo Test user $TN on ${TESTHOSTS[$M]} >> ./roster.txt
		sh ./remote-demon.sh ${TESTHOSTS[$M]} $TN
		M=$((M+1))
		sleep ${HOSTGAP:-10}	# Between hosts
	done
	N=$((N+1))
done
