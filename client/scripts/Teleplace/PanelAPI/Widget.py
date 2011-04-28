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

from Teleplace.Proxy import Proxy

class QWidget(object):
    """QWidget is the base class for all user interface widget objects"""

    def __init__(self, panel, tClass, name = None):
        """Initializes the canvas for a given panel.
        wClass is the Teleplace class name.
        name is a reference id, usually None, which means this is a new
        object and a name will be generated.
        """
        self.panel = panel
        self.tClass = tClass
        self.changeAction = None
        if(name is None):
            self.name = self.panel.generateName()
            Proxy.send(self.panel, "makeWidget", tClass, self.name)
        else:
            self.name = name
        panel.objectDictionary[self.name]=self
    
    def getTClass(self):
        """return the actual internal Teleplace class name"""
        return self.getValue("getClassName")

    def getName(self):
        """return the dynamically generated shared name"""
        return self.name

    def getClassName(self):
        """return the Python class name
        this is part of the persistence mechanism"""
        return type(self).__name__
    
    def getValue(self,valueName, args = None):
        """read arbitrary values directly from the object. Need to know the accessor name to use this."""
        return Proxy.send(self.panel, "getWidgetValue", self.name, valueName, args)

    def sendMessage(self, myMessage, arguments = None):
        """send a future message to a widget"""
        Proxy.send(self.panel, "widgetMessage", self.name, myMessage, arguments)
        
    """-------------------------------------------
    CPlayer messages
    """
    
    def setExtent(self, ext):
        self.sendMessage("extent:", ext)
        
    def getExtent(self):
        return getValue("extent")
    
    def setTopLeft(self, topLeft):
        self.sendMessage("topLeft:", (topLeft,))
        
    def getTopLeft(self):
        return self.getValue("topLeft")
    
    def setBottomRight(self, botRight):
        self.sendMessage("bottomRight:", (botRight,))
        
    def getBottomRight(self):
        return self.getValue("bottomRight")
    
    def setBounds(self, rect):
        self.sendMessage("bounds:", ((rect.left(), rect.top(), rect.width(), rect.height()),))
    
    def getBounds(self):
        return QRect(self.getLeft(), self.getTop(), self.getWidth(), self.getHeight())
        
    def getCenter(self):
        return self.getValue("center")
    
    def getWidth(self):
        return self.getValue("width")
    
    def setWidth(self, width):
        self.sendMessage("width:", width)
        
    def getHeight(self):
        return self.getValue("height")
    
    def setHeight(self, height):
        self.sendMessage("height:", height)
    
    def getTop(self):
        return self.getValue("top")
    
    def setTop(self, top):
        self.sendMessage("top:", top)
        
    def getLeft(self):
        return self.getValue("left")
    
    def setLeft(self, left):
        self.sendMessage("left:", left)
    
    def getBottom(self):
        return self.getValue("bottom")
    
    def setBottom(self, bottom):
        self.sendMessage("bottom:", bottom)
        
    def getRight(self):
        return self.getValue("right")
    
    def setRight(self, right):
        self.sendMessage("right:", right)
        
    def getVisible(self):
        return self.getValue("visible")
    
    def setVisible(self, bool):
        self.sendMessage("visible:", bool)
    """-------------------------------------------
    Change Events
    """

    def onWidgetChanged(self, key, userID):
        """This is sent to the widget when the widget has been changed. It
        is up to the widget itself to respond. Use the key to determine what
        the actual change was using getChangeValue(key)"""
        if self.changeAction is not None:
            self.changeAction(self, key, userID)

    def getChangeValue(self, key):
        """Read the actual change value from the widget. This might be somewhat cryptic, depending upon
        the widget and the key. See documentation for individual widgets."""
        return self.getValue("getKeyValue:", (key,))

    def setChangeAction(self, action):
        self.changeAction = action
    
class TextWidget(QWidget):
    """ A text entry widget. Can include scroll bars as required. Actions that are returned are:
    - selection - indicates that the text selection has changed to a new start, stop value.
    - replaceSelection - indicates that the current selection has been replaced. First value is the replacement text,
    second and third values are the selection indices of the text that will be replaced.
    - scrollOffset - the text is scrolled up or down. Two values indicating left-right and up-down scroll changes
    - bounds - the location of the moved text widget"""

    def __init__(self, panel, name = None):
        """Initializes the text widget for a given panel"""
        QWidget.__init__(self, panel,"QPythonText", name)
        
    def setHScrollable(self, bool):
        self.sendMessage("hScrollable:", bool)
        
    def getHScrollable(self):
        return self.getValue("hScrollable")
    
    def setVScrollable(self, bool):
        self.sendMessage("vScrollable:", bool)
        
    def getVScrollable(self):
        return self.getValue("vScrollable")
    
    def setText(self, text):
        self.sendMessage("setText:", text)
        
    def getText(self):
        return self.getValue("getText")

class PushButton(QWidget):
    """ A simple push button widget. Actions returned are:
    - fire. Indicates that the widget has been pressed by a user.
    - bounds. the location of the moved widget.
    """
    def __init__(self, panel, name = None):
        """ Initializes the push button widget"""
        QWidget.__init__(self, panel, "QPythonPushButton", name)

    def setLabel(self, label):
        self.sendMessage("label:", label)

    def getLabel(self):
        return self.getValue("label")


class RadioButton(QWidget):
    """ A single radio button. Will search for all other radio buttons in the app and interact with them.
    Actions returned are same as PushButton above
    """

    def __init__(self, panel, name = None):
        """ Initializes the radio button widget"""
        QWidget.__init__(self, panel, "QPythonRadioButton", name)

    def setLabel(self, label):
        self.sendMessage("label:", label)

    def getLabel(self, label):
        return self.getValue("label")

    def select(self):
        self.sendMessage("select")

    def deselect(self):
        self.sendMessage("deselect")

    def getSelected(self):
        return self.getValue("selected")
        

class CheckBox(QWidget):
    """ A single radio button. Will search for all other radio buttons in the app and interact with them.
    Actions returned are same as PushButton above
    """

    def __init__(self, panel, name = None):
        """ Initializes the radio button widget"""
        QWidget.__init__(self, panel, "QPythonCheckBox", name)

    def setLabel(self, label):
        self.sendMessage("label:", label)

    def getLabel(self, label):
        return self.getValue("label")

    def select(self):
        self.sendMessage("select")

    def deselect(self):
        self.sendMessage("deselect")

    def getSelected(self):
        return self.getValue("selected")

class ImageButton(QWidget):
    
    """ A simple push button widget. Actions returned are:
    - fire. Indicates that the widget has been pressed by a user.
    - bounds. the location of the moved widget.
    """
    def __init__(self, panel, name = None):
        """ Initializes the push button widget"""
        QWidget.__init__(self, panel, "QPythonImageButton", name)

    def setDisabledImage(self, image):
        self.sendMessage("setDisabledImage:", image.name)
    
    def setNormalImage(self, image):
        self.sendMessage("setNormalImage:", image.name)
    
    def setOverImage(self, image):
        self.sendMessage("setOverImage:", image.name)
    
    def setPressedImage(self, image):
        self.sendMessage("setPressedImage:", image.name)
    
    def setSelectedDisabledImage(self, image):
        self.sendMessage("setSelectedDisabledImage:", image.name)
    
    def setSelectedImage(self, image):
        self.sendMessage("setSelectedImage:", image.name)

    def setSelectedOverImage(self, image):
        self.sendMessage("setSelectedOverImage:", image.name)
    
    def setSelectedPressedImage(self, image):
        self.sendMessage("setSelectedPressedImage:", image.name)
    
    def select(self):
        self.sendMessage("select")

    def deselect(self):
        self.sendMessage("deselect")

    def getSelected(self):
        return self.getValue("selected")    

        
