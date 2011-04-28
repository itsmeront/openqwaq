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
# kill all demons on instance hosts.
# DONT DO THIS ON HOSTS that also host routers / appservers /etc,
# only for listed EC2 qdemon ami instances!

for host in $(<instances); do ssh root@$host halt ; done

