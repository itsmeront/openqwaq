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

# BridgeApp.py: A Teleplace Bridge Application

from Bridge import *
from EntryPoints import *
from BridgeApp import *

# The AppRegistry bridges to apps
AppRegistry = {}

class QBridgeApp(object):
    @classmethod
    def handleCreateEvent(appClass, bridge):
        """Registers a bridge app and dispatches the create event"""
        app = appClass(bridge)
        AppRegistry[bridge] = app
        app.onCreate()

    @classmethod
    def handleDestroyEvent(appClass, bridge):
        """Deregisters and dispatches a destroy event to a registered app"""
        app = AppRegistry[bridge]
        del AppRegistry[bridge]
        app.onDestroy()


    @classmethod
    def handleRunEvent(appClass, bridge):
        """Deregisters and dispatches a destroy event to a registered app"""
        app = AppRegistry[bridge]
        del AppRegistry[bridge]
        app.onRun()

    @classmethod
    def handleCallbackEvent(appClass, bridge, callback, eventArg):
        """A callback return"""
        app = AppRegistry[bridge]
        callback(eventArg)

        
    # -------------------------------------------------------------- #

    def __init__(self, bridge):
        """Initialized the bridge application"""
        # the bridge itself constructed by Teleplace
        self.bridge = bridge
        #api version from bridge API
        self.apiVersion = bridge.apiVersion()
        #returns id of the local user
        self.userID = bridge.getUserID()

    def println(self, obj, replicated = False):
        """Prints an object in the Teleplace transcript"""
        self.bridge.println(obj, replicated)

    def startTimer(self, msecs, callback):
        """Starts a timer with the given number of msecs base"""
        return self.bridge.startTimer(msecs, callback)

    def stopTimer(self, handle):
        """Stops a timer with the given handle"""
        self.bridge.stopTimer(handle)

    # -------------------------------------------------------------- #

    def onCreate(self):
        """The handler for a create event"""
        pass

    def onDestroy(self):
        """The handler for a destroy event"""
        pass

    def onRun(self):
        """The handler for the running event"""
        pass
        
