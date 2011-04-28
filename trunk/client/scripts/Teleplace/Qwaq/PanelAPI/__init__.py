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

# Import shim to support legacy applications

try:
   from Teleplace.PanelAPI import *
except ImportError,e:
   raise ValueError("[Util]:Import Teleplace.PanelAPI error %s, current system search path is:%s" %(e, sys.path))
