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

# Panel.py: Teleplace Panel interface

from Teleplace.Proxy import Proxy
from Graphics import QColor
from Q3D import *
import sys

class QPanel(object):
    """QPanel represents a facade to the actual Teleplace panel"""

    def __init__(self):
        """Construct the objectDictionary """
        self.objectDictionary = {}

    # ---------------------------------------------------------#
    # Replicated storage:
    # All the attributes stored in the panel are replicated across
    # the participants. Setting these values is *not* immediate though,
    # the values are only set after the round-trip completes (indicated
    # by the accompanying change event).
    #

    def __getitem__(self, key):
        """Implementer of the return value for myValue = self.panel["myKey"]"""
        rval = Proxy.send(self, "getItem", key)
        if(type(rval)==tuple): #is this a tuple?
            if(len(rval)==3): #of length 3?
                if(rval[0]=="__sharedObject__" or rval[0]=="__3DObject__"): #a shared Object?
                    if(self.objectDictionary.has_key(rval[2])): #already loaded?
                        rval = self.objectDictionary[rval[2]]
                    else:
                        rval =  eval(rval[1])(self, rval[2])
        return rval

    def __setitem__(self, key, value):
        """Implementer of the setter for self.panel["myKey"] = myValue """
        Proxy.send(self, "setItem", key, value)

    def __delitem__(self, key):
        """Implementer of the delete operation for del self.panel["myKey"]"""
        Proxy.send(self, "delItem", key)

    def append(self, key, value):
        """Implementer of self.panel["myKey"].append(value)"""
        Proxy.send(self, "appendItem", key, value)

    # ---------------------------------------------------------#
    # User related functions:
    #

    def getUsers(self):
        """Retrieves all the user IDs in the space"""
        return Proxy.send(self, "getUsers")

    def getUserID(self):
        """Retrieves the ID for the local user"""
        return Proxy.send(self, "getUserID")

    def getBaseUserID(self):
        """Retrieve the _base_ user - usually the guy that created the doc"""
        return Proxy.send(self, "getBaseUserID")
    
    def getBaseUserName(self):
        """Retrive the _base_ user name - the guy that created and is running the doc"""
        return Proxy.send(self, "getBaseUserName")
   
    def getUserColor(self, uid):
        """Retrieves the color for a user"""
        return QColor.fromRGBA(Proxy.send(self, "getUserColor", uid))

    def getUserName(self, uid):
        """Retrieves the preferred screen name for a user"""
        return Proxy.send(self, "getUserName", uid)

    def getUserLogin(self, uid):
        """Retrieves the login string for a user"""
        return Proxy.send(self, "getUserLogin", uid)
    
    def getServiceURL(self, uid):
        self.initialize3D()
        return Proxy.send(self, "get", "___rootObject___", "serviceURL")

    def getCurrentRoomName(self):
        return Proxy.send(self, "getCurrentRoomName")

    def getCurrentOrgName(self):
        return Proxy.send(self, "getCurrentOrgName")
    
    def abort(self, abortMessage = 'Application aborted'):
        """abort the application, include an abort message"""
        Proxy.send(self, "abort", abortMessage)
        sys.exit("Python application intentionally aborted.")

    def setStartupOwner(self):
        """only this user running the app can run this app in the future"""
        Proxy.send(self, "setStartupOwner")
        
    def setRelaunch(self):
        """if the user running the app exits, try to relaunch with another user"""
        Proxy.send(self, "setRelaunch")

    # ---------------------------------------------------------#
    # Panel properties:
    # Title, width, height etc.
    #

    def setTitle(self, title):
        """Set the title string of this panel - this is deprecated"""
        Proxy.send(self, "setPanelTitle", title)

    def getWidth(self):
        """Query about the current panel width"""
        return Proxy.send(self, "getPanelWidth")

    def getHeight(self):
        """Query about the current panel height"""
        return Proxy.send(self, "getPanelHeight")

    def setExtent(self, w, h):
        """Set the panel width and height"""
        Proxy.send(self, "setPanelExtent", w, h)

    def apiVersion(self):
        """Retrieves the Panel API version which is the version string of the Teleplace Client. """
        return Proxy.send(self, "apiVersion")

    def println(self, obj, replicated = False):
        """Prints obj into the Teleplace transcript"""
        Proxy.send(self, "println", str(obj),replicated)
        
    def exitApplication(self, replicated = False):
        """Don't do this, and NEVER do this with replicated = TRUE. I will hunt you down if you do."""
        Proxy.send(self, "exitApplication", replicated)
        
    def setLogging(self, bool):
        """Causes canvas messages to print to transcript"""
        Proxy.send(self, "setLogging", bool)

    def signalUserEvent(self, eventName, eventArg):
        """Sends a replicated user event with an optional argument"""
        Proxy.send(self, "signalUserEvent", eventName, eventArg)
        
    def getPanelVisible(self):
        """Returns the panels visibility - True is visible, False is not visible"""
        return Proxy.send(self, "getPanelVisible")
    
    def setPanelVisible(self, bool):
        """Sets the panels visibility - True is visible, False is not visible"""
        Proxy.send(self, "setPanelVisible", bool)

    def getKeyValue(self):
        return Proxy.send(self, "getKeyValue")
    
    def setKeyValue(self, kv):
        Proxy.send(self, "setKeyValue", kv)
        
    def getDocumentName(self):
        return Proxy.send(self, "getDocName")

    # ---------------------------------------------------------#
    # Timer support
    #

    def startTimer(self, msecs, callback):
        """Starts a timer with the given number of msecs base"""
        return Proxy.send(self, "startTimer", msecs, callback)

    def stopTimer(self, handle):
        """Stops a timer with the given handle"""
        Proxy.send(self, "stopTimer", handle)

    def alert(self, text):
        """signals an alert with a message"""
        Proxy.send(self, "alert", text)

    # ---------------------------------------------------------#
    # Document (up- and download) API:
    #

    def requestDownload(self, fileName, callback):
        """Requests the download of a shared document."""
        return Proxy.send(self, "requestDownload", fileName, callback)

    def requestUpload(self, fileName, data, callback):
        """Requests the upload of a shared document"""
        Proxy.send(self, "requestUpload", fileName, data, callback)

    def requestStatus(self, handle):
        """Answers the current status of the request"""
        return Proxy.send(self, "requestStatus", handle)

    def requestData(self, handle):
        """Returns the data from the request"""
        return Proxy.send(self, "requestData", handle)

    def requestDone(self, handle):
        """Invalidates the request"""
        Proxy.send(self, "requestDone", handle)

    def isLoaded(self):
        """has the associated data been loaded already?"""
        return Proxy.send(self, "isLoaded")

    def saveForum(self):
        """save the forum now - historical"""
        Proxy.send(self, "saveForum")

    def saveTeleplace(self):
        """save the Teleplace now"""
        Proxy.send(self, "saveForum")
    # ---------------------------------------------------------#
    # Standard File dialog access
    #

    def findFile(self, userID, fileDesc, fileExt, dialogLabel, callback):
        """ Find a file and return the file name
        fileDesc - describe the file type
        fileExt - a string with all suffexes, separated by ;
        dialogLabel - a string with the label for the file dialog"""
        return Proxy.send(self, "findFile", userID, fileDesc, fileExt, dialogLabel, callback)

    def saveFile(self, userID, data, fileDesc, fileExt, dialogLabel, callback):
        """ Find a file and return the file name
        fileDesc - describe the file type
        fileExt - a string with all suffexes, separated by ;
        dialogLabel - a string with the label for the file dialog"""
        self['__saveFileData__'] = data
        return Proxy.send(self, "saveFile", userID, fileDesc, fileExt, dialogLabel, callback)
    
    def saveFilePath(self, userID, path, data, fileDesc, fileExt, dialogLabel, callback):
        """ Find a file and return the file name
        fileDesc - describe the file type
        fileExt - a string with all suffexes, separated by ;
        dialogLabel - a string with the label for the file dialog"""
        self['__saveFileData__'] = data
        return Proxy.send(self, "saveFilePath", userID, path, fileDesc, fileExt, dialogLabel, callback)
    
    def loadFile(self, fileName):
        return Proxy.send(self, "getWithArgs", "getFileData:", fileName)

    def launchBrowser(self, userID, url):
        """ launch a web browser for the specified user"""
        Proxy.send(self, "launchBrowser", url, userID)
        
    # ---------------------------------------------------------#
    # File directory access
    #

    def getCacheDirectory(self):
        """return the cache directory """
        return Proxy.send(self, "getCacheDirectory")
    
    def getDefaultDirectory(self):
        """return the Teleplace directory """
        return Proxy.send(self, "getDefaultDirectory")
    
    def getHomeDirectory(self):
        """return the users home directory """
        return Proxy.send(self, "getHomeDirectory")
    
    def getLogDirectory(self):
        """return the Teleplace log directory """
        return Proxy.send(self, "getLogDirectory")
    
    def getRootDirectory(self):
        """return the system root directory """
        return Proxy.send(self, "getRootDirectory")
    
    def getPathNameDelimiter(self):
        """return the path delimiter - a '/' on Mac and Linux, a '\' on Windows """
        return Proxy.send(self, "getPathNameDelimiter")

        
    # ---------------------------------------------------------#
    # Menu Access:
    #
    def showMenu(self, menuList):
        """Create a pop-up menu list"""
        Proxy.send(self, "showMenu", menuList)  
        
    # ---------------------------------------------------------#
    # 3D API:
    #

    def initialize3D(self):
        """Sets up the 3D world, including 3D dictionaries, a base group, and
        access to the panel as a 3D object"""
        if self.objectDictionary.has_key("___rootObject___") is False:
            QBaseGroup(self)
            QPanelGroup(self)
            QRootGroup(self)
            Proxy.send(self, "initialize3D")

    def allowRootAccess(self):
        """ Indicates whether you or your application has access to the root frame
        of the Teleplace. Defaults to True """
        return Proxy.send(self,"allowRootAccess")

    def flush3D(self):
        """Force all pending messages to be sent"""
        Proxy.send(self, "flush3D")

    def generateName(self):
        """ generates a new unique name using TObjectID"""
        return Proxy.send(self,"generateName")

    def baseGroup(self):
        """ Return the base group object """
        return self.objectDictionary["___baseObject___"]

    def panelGroup(self):
        """ Return the actual panel containing this Python app """
        return self.objectDictionary["___panelObject___"]
    
    def rootGroup(self):
        """ Return the root object or TSpace of the Teleplace that
        this application is inside of """
        return self.objectDictionary["___rootObject___"]
    
    def constructC3X(self, c3xData):
        """construct a 3D object from a C3X data stream"""
        id = Proxy.send(self, "setC3X", c3xData)
        return QAnonymous(self,id)
    
    # ---------------------------------------------------------#
    # Chatbot API: send and receive message via the text chat
    #

    def sendChat(self, who, message):
        """ send a chat message to the text chat window
        message is a text string message
        who is a text string describing who the message is from
        """
        Proxy.send(self, "sendChat", who, message)
        
    def registerChat(self):
        """ register with the text chat window so that it will 
        forward messages to me """
        Proxy.send(self,"registerChat")
