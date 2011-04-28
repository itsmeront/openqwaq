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

# EntryPoints.py: The main file for a Python Panel App

# The following are the entry points that are actually used by Teleplace
# All of these are forwarded to their counterparts in our application.
# The app is initialized in handleCreateEvent() which needs to be defined in
# the __init__.py file of the application:
#
# def handleCreateEvent(panel):
#     return the appAppCS.handleCreateEvent(panel)

import PanelApp

def handleNewUserEvent(panel, id):
    """Passes the id of a new user - allows us to redraw as necessary"""
    return PanelApp.QPanelApp.handleNewUserEvent(panel, id)

def handleAvatarEnterEvent(panel, id):
    """Passes the id of a new user when avatar actually appears"""
    return PanelApp.QPanelApp.handleAvatarEnterEvent(panel, id)

def handleAvatarLeaveEvent(panel, id):
    """Passes the id of a avatar user when avatar is removed"""
    return PanelApp.QPanelApp.handleAvatarLeaveEvent(panel, id)

def handleDestroyEvent(panel):
    """Passes the destroy event to the app"""
    return PanelApp.QPanelApp.handleDestroyEvent(panel)

def handleKeyboardEvent3D(panel, type, stamp, buttons, id,  key, name):
    """Passes the keyboard event to the app""" 
    return PanelApp.QPanelApp.handleKeyboardEvent3D(panel, type, stamp, buttons, id, key, name)

def handleKeyboardEvent(panel, type, stamp, buttons, id,  key, widgetID):
    """Passes the keyboard event to the app""" 
    return PanelApp.QPanelApp.handleKeyboardEvent(panel, type, stamp, buttons, id, key, widgetID)

def handleMouseEvent(panel, type, stamp, buttons, id, x, y, widgetID):
    """Passes the mouse event to the app"""
    return PanelApp.QPanelApp.handleMouseEvent(panel, type, stamp, buttons, id, x, y, widgetID)

def handlePointerEvent(panel, type, stamp, buttons, id, target, frame, px, py, pz, nx,ny, nz):
    """Passes the mouse event to the app"""
    return PanelApp.QPanelApp.handlePointerEvent(panel, type, stamp, buttons, id, target, frame, (px,py,pz), (nx,ny,nz))

def handleFloorEvent(panel, type, stamp, buttons, id, target, frame, px, py, pz, nx,ny, nz):
    """Passes the mouse event to the app"""
    return PanelApp.QPanelApp.handleFloorEvent(panel, type, stamp, buttons, id, target, frame, (px,py,pz), (nx,ny,nz))

def handleChangeEvent(panel, attrName, attrValue, userID):
    """Passes the change event to the app"""
    return PanelApp.QPanelApp.handleChangeEvent(panel, attrName, attrValue, userID)

def handleSaveEvent(panel):
    """Passes a save event to the app"""
    return PanelApp.QPanelApp.handleSaveEvent(panel)

def handleLoadComplete(panel, success):
    """Alert the app that the data is now good"""
    PanelApp.QPanelApp.handleLoadComplete(panel, success)
    
def handleUserEvent(panel, eventName, eventArg):
    """Passes the timer event to the app"""
    return PanelApp.QPanelApp.handleUserEvent(panel, eventName, eventArg)

def handleCallbackEvent(panel, callbackName, eventArg):
    """Performs a generic named callback"""
    return PanelApp.QPanelApp.handleCallbackEvent(panel, callbackName, eventArg)

def handleMessageEvent(panel, message, arguments):
    """Performs a generic named callback"""
    return PanelApp.QPanelApp.handleMessageEvent(panel, message, arguments)

def handleMenuEvent(panel, menuItem):
    """A menu item was selected"""
    return PanelApp.QPanelApp.handleMenuEvent(panel, menuItem)

def handleChatEvent(panel, mssg):
    """A menu item was selected"""
    return PanelApp.QPanelApp.handleChatEvent(panel, mssg)

def handleWidgetChanged(panel, widgetID, key, userID):
    """A widget changed event."""
    return PanelApp.QPanelApp.handleWidgetChanged(panel, widgetID, key, userID)
