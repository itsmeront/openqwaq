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


# Partner to batch -
# push ../update.tar to all the listed instances,
# cd into forums, and untar.
#
# Relies on the instances being able to ssh/scp to one another via keys,
# which is in my base image ami-0f719066 


first=1
for host in $(<instances); do 
	if [  $first -eq 1 ]; then 
		firsthost=$host
		scp ../update.tar  root@${host}:
		ssh root@${host} '(cd teleplace; yes | tar -xvf ../update.tar)' 
		first=0
	else
		ssh root@${firsthost} "scp update.tar root@${host}:"
		ssh root@${host} '(cd teleplace; yes | tar -xvf ../update.tar; rm ../update.tar)'
	fi
done

ssh root@${firsthost} 'rm update.tar'
