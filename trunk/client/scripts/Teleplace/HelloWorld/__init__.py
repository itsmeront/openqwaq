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

# __init__.py: The main file for a Python Panel App

# Import our app (this also imports the panel api)
from HelloWorld import *

# Create the right kind of panel app
def handleCreateEvent(panel):
    return HelloWorld.handleCreateEvent(panel)
