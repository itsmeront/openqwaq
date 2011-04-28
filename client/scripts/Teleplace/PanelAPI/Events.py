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

# Events.py: Basic event types
from VirtualKey import *

class QPanelEvent(object):
    """ QPanelEvent is an abstract superclass for panel events """
    __slots__ = 'type', 'stamp', 'id'
    def __init__(self, type, stamp, id):
        self.type = type
        self.stamp = stamp
        self.id = id

    def __str__(self):
        return self.__class__.__name__ + "(" + str(self.type) + ")"

class QMouseEvent(QPanelEvent):
    """ QMouseEvent: An event representing mouse input """
    __slots__ = 'buttons', 'position', 'widget'
    def __init__(self, type, stamp, buttons, id, position, widget):
        QPanelEvent.__init__(self, type, stamp, id)
        self.buttons = buttons
        self.position = position
        self.widget = widget
        
    def isLeftButton(self):
        return VK_XBUTTON1 & self.buttons
        
    def isRightButton(self):
        return VK_XBUTTON2 & self.buttons

class QKeyboardEvent(QPanelEvent):
    """ QKeyboardEvent: An event representing keyboard input """
    __slots__ = 'buttons', 'value', 'name', 'frame', 'widget'
    def __init__(self, type, stamp, buttons, id, value, frame, widget):
        QPanelEvent.__init__(self, type, stamp, id)
        self.buttons = buttons
        self.value = value
        self.frame = frame
        self.widget = widget

class QPointerEvent(QPanelEvent):
    """ QPointerEvent: An event representing pointer input """
    __slots__ = 'buttons', 'hitFrame', 'frame', 'point', 'normal'
    def __init__(self, type, stamp, buttons, id, frame, hitFrame, point, normal):
        QPanelEvent.__init__(self, type, stamp, id)
        self.buttons = buttons
        self.frame = frame
        self.hitFrame = hitFrame
        self.point = point
        self.normal = normal
        
    def isLeftButton(self):
        return VK_XBUTTON1 & self.buttons
        
    def isRightButton(self):
        return VK_XBUTTON2 & self.buttons
