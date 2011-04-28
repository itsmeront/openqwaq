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

# PanelApp.py: A Teleplace Panel Application

from Graphics import *
from Events import *
from Panel import *
from PanelApp import *
from EntryPoints import *

# The AppRegistry panels to apps
AppRegistry = {}

class QPanelApp(object):
    
    @classmethod    
    def handleCreateEvent(appClass, panel):
        """Registers a panel app and dispatches the create event"""
        app = appClass(panel)
        AppRegistry[panel] = app
        app.onCreate()

    @classmethod
    def handleNewUserEvent(appClass, panel, id):
        """A new user is showing up"""
        app = AppRegistry[panel]
        return app.onNewUser(id)

    @classmethod
    def handleAvatarEnterEvent(appClass, panel, id):
        """A new user is showing up"""
        app = AppRegistry[panel]
        return app.onAvatarEnter(id)    

    @classmethod
    def handleAvatarLeaveEvent(appClass, panel, id):
        """A new user is showing up"""
        app = AppRegistry[panel]
        return app.onAvatarLeave(id)    
    
    @classmethod
    def handleDestroyEvent(appClass, panel):
        """Deregisters and dispatches a destroy event to a registered app"""
        app = AppRegistry[panel]
        del AppRegistry[panel]
        app.onDestroy()

    @classmethod
    def handleMouseEvent(appClass, panel, type, stamp, buttons, id, x, y, widgetID):
        """Dispatches a mouse event to a registered app"""
        app = AppRegistry[panel]
        if(widgetID is None):
            myWidget = None
        else:
            myWidget = app.panel.objectDictionary[widgetID]
        event = QMouseEvent(type, stamp, buttons, id, (x,y), myWidget)
        if(app.userFilter is not None):
            if(id not in app.userFilter): return False
        {
        "mouseMove" : app.onMouseMove,
        "mouseDown" : app.onMouseDown,
        "mouseUp" : app.onMouseUp,
        "mouseOver" : app.onMouseOver,
        "mouseEnter" : app.onMouseEnter,
        "mouseLeave" : app.onMouseLeave
        }[type](event)

    @classmethod
    def handleKeyboardEvent(appClass, panel, type, stamp, buttons, id, key, widgetID):
        """Dispatches a keyboard event to a registered app"""
        app = AppRegistry[panel]
        if(app.userFilter is not None):
            if(id not in app.userFilter): return False
        if(widgetID is None):
            myWidget = None
        else:
            myWidget = app.panel.objectDictionary[widgetID]
        event = QKeyboardEvent(type, stamp, buttons, id, key, None, myWidget)
        {
        "keyStroke" : app.onKeyStroke,
        "keyDown" : app.onKeyDown,
        "keyUp" : app.onKeyUp
        }[type](event)

    @classmethod
    def handleKeyboardEvent3D(appClass, panel, type, stamp, buttons, id, key, name):
        """Dispatches a keyboard event to a registered app"""
        app = AppRegistry[panel]
        if(app.userFilter is not None):
            if(id not in app.userFilter): return False
        if(name is None):
            myObject = None
        else:
            myObject = app.panel.objectDictionary[name]
        event = QKeyboardEvent(type, stamp, buttons, id, key, myObject, None)
        {
        "keyStroke" : app.onKeyStroke3D,
        "keyDown" : app.onKeyDown3D,
        "keyUp" : app.onKeyUp3D
        }[type](event)


    @classmethod
    def handlePointerEvent(appClass, panel, type, stamp, buttons, userID, targetID, frameID, point, normal):
        """Dispatches a pointer event to a registered app"""
        app = AppRegistry[panel]
        if(app.userFilter is not None):
            if(id not in app.userFilter): return False
        if(targetID is None):
            myObject = None
        else:
            if(app.panel.objectDictionary.has_key(targetID)):
                myObject = app.panel.objectDictionary[targetID]
            else:
                myObject = QAnonymous(app.panel, targetID)
        if(frameID is not None):
            if(app.panel.objectDictionary.has_key(frameID)):
                myHitObject = app.panel.objectDictionary[frameID]
            else:
                myHitObject = QAnonymous(app.panel, frameID)
        else: myHitObject = myObject
        event = QPointerEvent(type, stamp, buttons, userID, myObject, myHitObject, point, normal)
        {
        'pointerMove' : app.onPointerMove,
        'pointerDown' : app.onPointerDown,
        'pointerUp' : app.onPointerUp,
        'pointerEnter' : app.onPointerEnter,
        'pointerOver' : app.onPointerOver,
        'pointerLeave' : app.onPointerLeave
        }[type](event)

    @classmethod
    def handleFloorEvent(appClass, panel, type, stamp, buttons, userID, targetID, frameID, point, normal):
        """Dispatches a floor event to a registered app"""
        app = AppRegistry[panel]

        if(targetID is None):
            myObject = None
        else:
            if(app.panel.objectDictionary.has_key(targetID)):
                myObject = app.panel.objectDictionary[targetID]
            else:
                myObject = QAnonymous(app.panel, targetID)
        if(frameID is not None):
            if(app.panel.objectDictionary.has_key(frameID)):
                myHitObject = app.panel.objectDictionary[frameID]
            else:
                myHitObject = QAnonymous(app.panel, frameID)
        else: myHitObject = myObject
        event = QPointerEvent(type, stamp, buttons, userID, myObject, myHitObject, point, normal)
        {
        'floorEnter' : app.onFloorEnter,
        'floorExit' : app.onFloorExit,
        }[type](event)
        
    @classmethod
    def handleChangeEvent(appClass, panel, attrName, attrValue, userID):
        """Dispatches a change event to a registered app"""
        app = AppRegistry[panel]
        app.onChangeEvent(attrName, attrValue, userID)

    @classmethod
    def handleSaveEvent(appClass, panel):
        """Dispatches a change event to a registered app"""
        app = AppRegistry[panel]
        app.onSaveEvent()

    @classmethod
    def handleLoadComplete(appClass, panel, success):
        """Dispatches a change event to a registered app"""
        app = AppRegistry[panel]
        app.onLoadSuccessEvent(success)
        if(success):
            app.onLoadEvent()

    @classmethod
    def handleUserEvent(appClass, panel, eventName, eventArg):
        """Dispatches a user event to a registered app"""
        app = AppRegistry[panel]
        app.onUserEvent(eventName, eventArg)

    @classmethod
    def handleCallbackEvent(appClass, panel, callback, eventArg):
        """A new user is showing up"""
        app = AppRegistry[panel]
        callback(eventArg)

    @classmethod
    def handleMessageEvent(appClass, panel, message, messageArgs):
        """Dispatches a message event to a registered app"""
        app = AppRegistry[panel]
        app.onMessageEvent(message, messageArgs)

    @classmethod
    def handleMenuEvent(appClass, panel, item):
        """A menu item is selected"""
        app = AppRegistry[panel]
        app.onMenuEvent(item)

    @classmethod
    def handleChatEvent(appClass, panel, mssg):
        """A chat message has been received"""
        app = AppRegistry[panel]
        app.onReceiveTextChat(mssg[0], mssg[1], mssg[2])

    @classmethod
    def handleWidgetChanged(appClass, panel, widgetID, key, userID):
        """A widget change event"""
        app = AppRegistry[panel]
        if(widgetID is None):
            myWidget = None
        else:
            if app.panel.objectDictionary.has_key(widgetID):
                myWidget = app.panel.objectDictionary[widgetID]
                myWidget.onWidgetChanged(key, userID)
            else: return
        app.onWidgetChanged(myWidget, key, userID)
        
    # -------------------------------------------------------------- #

    def __init__(self, panel):
        """Initialized the panel application"""
        # the panel itself constructed by Teleplace
        self.panel = panel
        # a canvas interface to the panel
        self.canvas = QCanvas(panel, "__mainCanvas__")
        #api version from Panel API
        self.apiVersion = panel.apiVersion()
        #returns id of the local user
        self.userID = panel.getUserID()
        #returns screen name of local user
        self.userName  = panel.getUserName(self.userID)
        #returns the avatar color of local user
        self.userColor = panel.getUserColor(self.userID)
        #sets the initial extent to the current size of the panel
        self.extent = (panel.getWidth(), panel.getHeight())
        #sets the initial userFilter to None so that anyone can play
        self.userFilter = None

    def println(self, obj, replicated = False):
        """Prints an object in the Teleplace transcript"""
        self.panel.println(obj, replicated)

    def setTitle(self, title):
        """Set the title string of this app"""
        self.panel.setTitle(title)

    def startTimer(self, msecs, callback):
        """Starts a timer with the given number of msecs base"""
        return self.panel.startTimer(msecs, callback)

    def stopTimer(self, handle):
        """Stops a timer with the given handle"""
        self.panel.stopTimer(handle)

    def setExtent(self, x, y):
        """Sets and caches the extent of the panel"""
        self.extent = (x, y)
        self.panel.setExtent(x, y)

    def getExtent(self):
        """Answers the (cached) panel extent"""
        return self.extent

    def setKeyValue(self, kv):
        """set the external key value for Py2Py communication"""
        self.panel.setKeyValue(kv)
        
    def setUserFilter(self, uf):
        """set the user filter. None will clear it. Ensure it is a sequence otherwise."""
        if uf is None: 
            userFilter = None
            return
        "are we a sequence? make sure it is."
        if getattr(uf, "__iter__", False):
            userFilter = (uf,)
        else:
            userFilter = uf
        
    # -------------------------------------------------------------- #

    def onCreate(self):
        """The handler for a create event"""
        pass

    def onDestroy(self):
        """The handler for a destroy event"""
        pass

    def onMouseDown(self, event):
        """The handler for a mouseDown event"""
        pass

    def onMouseMove(self, event):
        """The handler for a mouseMove event"""
        pass

    def onMouseOver(self, event):
        """The handler for the mouseOver event"""
        pass

    def onMouseUp(self, event):
        """The handler for a mouseUp event"""
        pass

    def onMouseEnter(self, event):
        """The handler for the mouseEnter event"""
        pass

    def onMouseLeave(self, event):
        """The handler for the mouseLeave event"""
        pass

    def onKeyDown(self, event):
        """The handler for a keyDown event"""
        pass

    def onKeyStroke(self, event):
        """The handler for a keyStroke event"""
        pass

    def onKeyUp(self, event):
        """The handler for a keyUp event"""
        pass

    def onKeyDown3D(self, event):
        """The handler for a keyDown event"""
        pass

    def onKeyStroke3D(self, event):
        """The handler for a keyStroke event"""
        pass

    def onKeyUp3D(self, event):
        """The handler for a keyUp event"""
        pass

    def onPointerDown(self, event):
        """The handler for a pointerDown event"""
        pass

    def onPointerMove(self, event):
        """The handler for a pointerMove event"""
        pass

    def onPointerUp(self, event):
        """The handler for a pointerUp event"""
        pass

    def onPointerEnter(self, event):
        """The handler for a pointerDown event"""
        pass

    def onPointerOver(self, event):
        """The handler for a pointerMove event"""
        pass

    def onPointerLeave(self, event):
        """The handler for a pointerUp event"""
        pass
        
    def onFloorEnter(self, event):
        """The handler for a floorEnter event"""
        pass
        
    def onFloorExit(self, event):
        """The handler for a floorExit event"""
        pass

    def onChangeEvent(self, attrName, attrValue):
        """The handler for a change event"""
        pass

    def onSaveEvent(self):
        """The handler for a save event"""
        pass

    def onNewUser(self, id):
        """The handler for a new user event"""
        pass

    def onAvatarEnter(self, id):
        """The handler for an avatar entering or re-entering a space"""
        pass

    def onAvatarLeave(self, id):
        """The handler for an avatar leaving a space"""
        pass
        
    def onMessageEvent(self, message, arguments):
        """The handler for a remote message event"""
        pass
        
    def onMenuEvent(self, menuItem):
        """The handler for a pop-up menu event - see PanelApp.showMenu()"""
        pass
    
    def onLoadEvent(self):
        """ The handler for informing the app that the data is ready to access.
        This method is still called, but new Python applications should use
        onLoadSuccessEvent(success)."""
        pass
    
    def onLoadSuccessEvent(self, success):
        """ The handler for informing the app that the data is ready to access.
        This is the new event and should be used by your application."""
        pass
        
    def onReceiveTextChat(self, who, message, timeStamp):
        """ If we register for text chat messages, this method will be called
        when a new text chat message is received by this user.
        who - the user who initiated the text chat
        message - the actual message
        timeStamp - when the message was received"""
        pass

    def onWidgetChanged(self, myWidget, key, userID):
        """ When a widget recieves a changed message, both the panel app and
        the widget recieve the onWidgetChanged message"""
        pass

    # -------------------------------------------------------------- #

    def signalUserEvent(self, eventName, eventArg):
        """The handler for a user event"""
        self.panel.signalUserEvent(eventName, eventArg)

    def onUserEvent(self, eventName, eventArg):
        """The handler for a user event"""
        pass

    # -------------------------------------------------------------- #

    def requestDownload(self, filePath, callback):
        """Requests the download of a shared document."""
        return self.panel.requestDownload(filePath, callback)

    def requestUpload(self, filePath, data, callback):
        """Requests the upload of a shared document"""
        return self.panel.requestUpload(filePath, data, callback)

    def requestStatus(self, handle):
        """Answers the current status of the request"""
        return self.panel.requestStatus(handle)

    def requestData(self, handle):
        """Returns the data from the request"""
        return self.panel.requestData(handle)

    def requestDone(self, handle):
        """Invalidates the request"""
        return self.panel.requestDone(handle)
    
    def showMenu(self, menuList):
        """Create a pop-up menu list"""
        self.panel.showMenu(menuList)
