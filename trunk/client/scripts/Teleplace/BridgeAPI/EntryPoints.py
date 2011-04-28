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

# EntryPoints.py: The main file for a Python Bridge App

# The following are the entry points that are actually used by Teleplace
# All of these are forwarded to their counterparts in our application.
# The app is initialized in handleCreateEvent() which needs to be defined in
# the __init__.py file of the application:
#
# def handleCreateEvent(panel):
#     return MyBridgeApp.handleCreateEvent(panel)

import BridgeApp

def handleCreateEvent(bridge, id):
    """Passes the id of a new user - allows us to redraw as necessary"""
    return BridgeApp.QBridgeApp.handleCreateEvent(bridge, id)

def handleRunEvent(bridge, id):
    return BridgeApp.QBridgeApp.handleRunEvent(bridge, id)

def handleDestroyEvent(bridge):
    """Passes the destroy event to Scribble"""
    return BridgeApp.QBridgeApp.handleDestroyEvent(bridge)

