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

# Q3D.py: Basic 3D support
""" This needs to be smart enough to cache all commands locally until a flush
is issued
"""

from Teleplace.Proxy import Proxy
from Graphics import *

# QBasic is the base construction class for all shared objects. It's __init__
# construction pattern is used by any object that is saved via a panel.objectDictionary.
   
class QBasic(object):
    """QBasic is the base class of all 3D shared objects in the Teleplace Python API"""
    __slots__ = 'panel','name','tClass'
    def __init__(self, panel, tclass, name):
        """ initialization of QBasic and most of its sub-classes
        panel is the main shared panel object used for communication
        tclass is the internal class name of the object
        name is typically a dynamically generated shared name between Teleplace and Python
        """
        self.panel = panel
        self.tClass = tclass
        if(name is None):
             self.name = Proxy.send(self.panel, "makeNew", self.tClass)
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

    def duplicate(self):
        """creates a copy of the current object"""
        newName = Proxy.send(self.panel, "duplicate", self.name)
        newObject = eval(self.getClassName())(self.panel, newName)
        return newObject
    
    def destroy(self):
        """completely destroys the target frame"""
        Proxy.send(self.panel,"destroy", self.name)
        del self.panel.objectDictionary[self.name]
        
    def castTo(self, toClass):
        """force a shared object to act like another independent of its true class
        type inside of Teleplace"""
        return toClass(self.panel, self.name)

    def getValue(self,valueName):
        """read arbitrary values directly from the object. Need to know the accessor name to use this."""
        return Proxy.send(self.panel, "get", self.name, valueName)
        
    def sendMessage(self, myMessage, arguments = None):
        """send a future message to a TFrame"""
        Proxy.send(self.panel, "sendWithArgs", self.name, myMessage, arguments)
        
    def sendPyMessage(self, myMessage, arguments = None):
        """Send a python message to a python panel application object"""
        Proxy.send(self.panel, "sendPyMessage", self.name, myMessage, arguments)      
   
class QReadTransform(QBasic):
    """QReadTransform is a minimal interface for 3D objects.
    It provides read only access for 3D transforms and queries."""
    # --- transforms ---

    def getLocalTransform(self):
        return self.getValue("getLocalTransform")

    def getGlobalTransform(self):
        return self.getValue("getGlobalTransform")
    
    def getTranslation(self):
        return self.getValue("translation")

    def getGlobalPosition(self):
        return self.getValue("globalPosition")

    def getQuaternion(self):
        return self.getValue("getQuaternion")
    
    def getRotationXYZ(self):
        return self.getValue("getRotationXYZ")

    def getGlobalQuaternion(self):
        return self.getValue("getGlobalQuaternion")
    
    def isGroup(self):
        return self.getValue("isGroup")
    
    def getChildren(self):
        idList = self.getValue("getChildren")
        if idList is not None:
            return self.convertIDList(idList, QAnonymous)
        else: return None

    def getParent(self):
        parent = self.getValue("getParent")
        if parent is None: return None
        return self.resolve(parent, QAnonymous)
    
    def getAllFrames(self):
        idList = self.getValue("getAllFrames")
        return self.convertIDList(idList, QAnonymous)

    def hasAncestor(self, ancestor):
        return Proxy.send(self.panel, "hasAncestor", self.name, ancestor.name)
    
    def hasAncestorOrMe(self, ancestor):
        return Proxy.send(self.panel, "hasAncestorOrMe", self.name, ancestor.name)

    def getObjectName(self):
        return self.getValue("getObjectName")
    
    def isAnonymous(self):
        return False
    
    def getC3X(self):
        """return an xml version of the object in the form of a byte array"""
        return Proxy.send(self.panel, "getC3X", self.name)
    
    def getLookAt(self):
        return self.getValue("getLookAt")
    
    def getLookUp(self):
        return self.getValue("getLookUp")
    
    def getLookSide(self):
        return self.getValue("getLookSide")
    
    def getLookAtLocal(self):
        return self.getValue("getLookAtLocal")
    
    def getLookUpLocal(self):
        return self.getValue("getLookUpLocal")
    
    def getLookSideLocal(self):
        return self.getValue("getLookSideLocal")

    def getScale(self):
        return self.getValue("getScale")
    
    def isSerialNumbered(self):
        return self.getValue("isSerialNumbered")
    
    def getSerialNumber(self):
        return self.getValue("serialNumber")
    
    def convertIDList(self, idList, defaultClass):
        rval = []
        if idList is None: return rval
        for id in idList:
            rval.append(self.resolve(id, defaultClass))
        return rval
    
    # if we already have a reference to this object return it, otherwise return QAnonymous
    def resolve(self, id, defaultClass):
        if(self.panel.objectDictionary.has_key(id)):
            return self.panel.objectDictionary[id]
        else:
            return defaultClass(self.panel, id)
    
    def findNamed(self, objectName, caseSensitive = False, exact = False):
        idList = Proxy.send(self.panel, "findNamed", self.name, objectName, caseSensitive, exact)
        return self.convertIDList(idList, QAnonymous)
    
    def findClass(self, className, descendants = False):
        idList = Proxy.send(self.panel, "findClass", self.name, className, descendants)
        return self.convertIDList(idList, QAnonymous)
        
    def findPythonApps(self):
        idList = Proxy.send(self.panel, "findClass", self.name, "PyPanelApp", False)
        return self.convertIDList(idList, QPythonApp)
    
    def findDisplayPanels(self):
        idList = Proxy.send(self.panel, "findClass", self.name, "QDisplayPanel", True)
        return self.convertIDList(idList, QDisplayPanel)

    def findAvatars(self):
        idList = Proxy.send(self.panel, "findClass", self.name, "TAvatarReplica", True)
        return self.convertIDList(idList, QAvatar)

        

class QTransformable(QReadTransform):
    """QTransformable allows the object to be moved around in the space. These
    transforms are relative to its parent frame."""
    # --- transforms ---
    
    def setIdentityTransform(self):
        Proxy.send(self.panel, "setIdentityTransform", self.name)
    
    def setRotation(self, rots):
        Proxy.send(self.panel, "setRotation", self.name)
        
    def setRotationsXYZ(self, x, y, z):
        Proxy.send(self.panel, "setRotationXYZ", x, y, z)

    def setLocalTransform(self, trans):
        Proxy.send(self.panel, "setLocalTransform", self.name, trans)
        
    def setTranslationXYZ(self, x, y, z):
        Proxy.send(self.panel, "setTranslationXYZ", self.name, x, y, z)

    def setTranslation(self, trans):
        Proxy.send(self.panel, "setTranslation", self.name, trans)

    def addRotationAroundX(self, angle):
        Proxy.send(self.panel, "addRotationAroundX", self.name, angle)

    def addRotationAroundY(self, angle):
        Proxy.send(self.panel, "addRotationAroundY", self.name, angle)

    def addRotationAroundZ(self, angle):
        Proxy.send(self.panel, "addRotationAroundZ", self.name, angle)

    def setRotationAroundX(self, angle):
        Proxy.send(self.panel, "setRotationAroundX", self.name, angle)

    def setRotationAroundY(self, angle):
        Proxy.send(self.panel, "setRotationAroundY", self.name, angle)

    def setRotationAroundZ(self, angle):
        Proxy.send(self.panel, "setRotationAroundZ", self.name, angle)

    def setQuaternion(self, quaternion):
        Proxy.send(self.panel, "setQuaternion", self.name, quaternion)

    def setScale(self, scale):
        Proxy.send(self.panel, "setScale", self.name, scale)
        
    def getRelativeTransformedPoint(self, frame, point):
        return Proxy.send(self.panel, "getRelativeTransformedPoint", self.name, frame.name, point)

    # --- material-ish ---
    
    def setTransparency(self, trans):
        Proxy.send(self.panel, "setTransparency", self.name, trans)

    def setVisible(self, vis):
        Proxy.send(self.panel, "setVisible", self.name, vis)

    def getVisible(self):
        return self.getValue("getVisible")
    
    def setPickable(self, pick):
        Proxy.send(self.panel, "setPickable", self.name, pick)

    def getPickable(self):
        return self.getValue("getPickable")

    def setHilite(self, color):
        Proxy.send(self.panel, "hilite", self.name, color.rgba)

    def unhilite(self):
        Proxy.send(self.panel, "unhilite", self.name)
        
    def getAllowWalk(self):
        return self.getValue("getAllowWalk")
    
    def setAllowWalk(self, allow):
        Proxy.send(self.panel, "setAllowWalk", self.name, allow)
        
    def setSerialNumber(self, snum):
        Proxy.send(self.panel, "sendWithArgs", self.name, "serialNumber:", snum)
        
    def generateSerialNumber(self):
        Proxy.send(self.panel, "sendWithArgs", self.name, "generateSerialNumber", None)

    
    # --- hierarchy ---

    def removeSelf(self):
        Proxy.send(self.panel, "removeSelf", self.name)
        
    def getBoundingBox(self):
        return self.getValue("getBoundingBox")
    
    def setObjectName(self, objectName):
        Proxy.send(self.panel, "setObjectName", self.name, objectName)

    # --- actions ---

    def setPointerAction(self, actionClassName, args):
        Proxy.send(self.panel, "setPointerAction", self.name, actionClassName, args)
        
    def registerPointerEvents(self):
        Proxy.send(self.panel, "registerPointerEvents", self.name)
        
    def registerFloorEvents(self):
        Proxy.send(self.panel, "registerFloorEvents", self.name)
        
    def goto(self, transform, count, deltaTime, callback):
        """this is for older apps that use this - only one that I know of"""
        return self.objectGoTo(transform, count, deltaTime, callback)

    def objectGoTo(self, transform, count = 5, deltaTime = 20, callback = None):
        return Proxy.send(self.panel, "objectGoto", self.name, transform, count, deltaTime, callback)


class QAnonymous(QTransformable):
    """QAnonymous is a 'found' frame. We don't necessarily know what it's class is, but we should be able to
    manipulate it to some degree. We should definitely NOT add children to this. Note that name MUST be defined."""
    
    def __init__(self,panel,name):
        QBasic.__init__(self, panel,"Anon",name)
        
    def isAnonymous(self):
        return True
    

class QPythonApp(QTransformable):
    """QPythonApp is a 'found' Python app frame. We know it is a Python application, and can do some simple
    queries on it as well as send it messages."""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel,"PyPanelApp", name)

    def getKeyValue(self):
        return Proxy.send(self.panel, "getRemoteKeyValue", self.name)
    

class QDisplayPanel(QTransformable):
    """QDisplayPanel is a resizable movable display object."""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QDisplayPanel", name)

    def setFileCard(self, qCard):
        Proxy.send(self.panel, "setFileCard", self.name, qCard.name)
        

class QAvatar(QTransformable):
    """QAvatar is the actual avatar object"""
    def __init__(self, panel, name = None):
        if(name is not None): 
            uid = Proxy.send(panel, "getUserAvatarID", name)
            if(uid is None): uid = name
        else: uid = None
        QBasic.__init__(self, panel, "QAvatar", uid)

    def getUserID(self):
        return self.getValue("getUserID")
        
    def getUserName(self):
        uid = self.getUserID()
        return self.panel.getUserName(uid)
    
    def avatarGoTo(self, transform, count= 5, sticky = False, label = None):
        Proxy.send(self.panel, "avatarGoTo", self.name, transform, count, sticky, label)
        
    def avatarGoToObject(self, object, count= 5, sticky = False, label = None):
        trans = object.getGlobalTransform()
        self.avatarGoTo(trans, count, sticky, label)
               
    def playAnimation(self, animationName, playType="action"):
        """playType = "action" or "cycle" """
        Proxy.send(self.panel, "playAnimation", self.name, animationName, playType)
        



        
class QFileCard(QTransformable):
    """QFileCard is a minimal data reference for objects that can be displayed in a QScreen."""
    
    @staticmethod
    def construct(panel, fileName):
        id = Proxy.send(panel, "makeCard", fileName)
        if id is not None:
            return QFileCard(panel, id)
        else: return None
    
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QFileCard", name)
        
    def setURL(self, url):
        Proxy.send(self.panel, "setCardURL", self.name, url)
        
    def setText(self, text):
        Proxy.send(self.panel, "setCardText", self.name, text)
        
    def setPythonApp(self, pyApp):
        Proxy.send(self.panel, "setCardPythonApp", self.name, pyApp)
        
    def setTexture(self, texture):
        Proxy.send(self.panel, "setCardTexture", self.name, texture.name)

    def getContentSummary(self):
        return self.getValue("contentSummary")

        
class QGroup(QTransformable):
    """QGroups can hold child frames and are the basis of a 3D hierarchy."""
 
    def __init__(self, panel, name = None):
        QBasic.__init__(self,panel,"TGroup", name)

    # --- hierarchy ---

    def addChild(self, child):
        Proxy.send(self.panel, "addChild", self.name, child.name)

    def removeChild(self, child):
        Proxy.send(self.panel, "removeChild", self.name, child.name)
    

class QImportGroup(QGroup):
    """A QImportGroup represents an imported 3D object."""
    def __init__(self, panel, name = None):
        self.panel = panel
        self.tClass = "QImportGroup"
        if(name is not None):
            self.name = name
            self.panel.objectDictionary[self.name]=self
        
    def loadFile(self, fileName):
        rval = Proxy.send(self.panel, "loadFile", fileName)
        if(rval is not None):
            self.name = rval
            self.panel.objectDictionary[self.name]=self
            
    def setScaleTo(self, scale):
        """ only QImportGroups have a setScaleTo """
        Proxy.send(self.panel, "setScaleTo", self.name, scale)

class QBaseGroup(QGroup):
    """A QBaseGroup is automatically generated - don't use this class directly.
    This is the interface object to the base group inside of Teleplace.
    QBaseGroup is automatically generated on both the Python and Squeak sides.
    It is the root object for the Python 3D objects"""
    __slots__ = 'panelGroup',
    def __init__(self, panel, name = "___baseObject___"):
        QBasic.__init__(self, panel, "QBaseGroup", name)

    def setAttached(self, bool):
        Proxy.send(self.panel, "setAttached", self.name, bool)

    def getAttached(self):
        return self.getValue("getAttached")
    

class QRootGroup(QGroup):
    """A QRootGroup is automatically generated - don't use this class directly.
    This is the interface object to the root of the entire space inside of Teleplace.
    It will not be generated if it does not have appropriate permissions
    It is the root object for all 3D objects in a particular Forum"""
    __slots__ = 'panelGroup',

    def __init__(self, panel, name = "___rootObject___"):
        QBasic.__init__(self, panel, "QRootGroup", name)

    def duplicate(self):
        # do NOT duplicate the root. Really bad idea.
        return self


class QPanelGroup(QReadTransform):
    """A QPanelGroup is automatically generated - don't use this class directly.
    This is the interface object to the 2D panel inside of Teleplace."""
    def __init__(self, panel, name = "___panelObject___"):
        QBasic.__init__(self, panel, "QPanelGroup", name)
    

class QPrimitive(QTransformable):
    """QPrimitive is a virtual class - don't use it either."""
    
    # --- modify materials ---

    def setColor(self, color):
        Proxy.send(self.panel, "setColor", self.name, color.rgba)

    def getColor(self):
        return QColor.fromTupleFloat(self.getValue("getColor"))
    
    def setAmbient(self, color):
        Proxy.send(self.panel, "setAmbient", self.name, color.rgba)

    def getAmbient(self):
        return QColor.fromTupleFloat(self.getValue("getAmbient"))

    def setDiffuse(self, color):
        Proxy.send(self.panel, "setDiffuse", self.name, color.rgba)

    def getDiffuse(self):
        return QColor.fromTupleFloat(self.getValue("getDiffuse"))

    def setSpecular(self, color):
        Proxy.send(self.panel, "setSpecular", self.name, color.rgba)

    def getSpecular(self):
        return QColor.fromTupleFloat(self.getValue("getSpecular"))

    def setShininess(self, shininess):
        Proxy.send(self.panel, "setShininess", self.name, shininess)

    def getShininess(self):
        return self.getValue("getShininess")

    def getTransparency(self):
        return self.getValue("getTransparency")

    def setTexture(self,texture):
        Proxy.send(self.panel, "setTexture", self.name, texture.name)

    def getTexture(self):
        return self.getValue("getTexture")
    
    def getMaterials(self):
        idList = self.getValue("getMaterials")
        if idList is not None:
            return self.convertIDList(idList, QMaterial)
        else: return None
        
    def setMaterial(self, material, index = 0):
        Proxy.send(self.panel, "setMaterial", self.name, material.name, index)    
        
class QStickyNote(QPrimitive):
    """Sticky note objects that are movable and editable by users"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QStickyNote", name)
        
    def setContents(self, aString):
        """set the string contents of the card"""
        self.sendMessage("contents:", aString)
        
    def getContents(self):
        """read the string contents of the card"""
        return self.getValue("contents")
    
    def getLog(self):
        """read the contents in XML form"""
        return self.getValue("getLog")
    
    
class QTopicCard(QPrimitive):
    """Sticky note objects that are movable and editable by users"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QTopicCard", name)    
        
    def setContents(self, aString):
        """set the string contents of the card"""
        self.sendMessage("contents:", aString)
        
    def getContents(self):
        """read the string contents of the card"""
        return self.getValue("contents")
    
    def getLog(self):
        """read the contents in XML form"""
        return self.getValue("getChatLog")
    
    def setCreator(self, creatorString):
        """set the creator string, usually a login email address"""
        self.sendMessage("creatorLogin:", creatorString)
        
    def getCreator(self):
        """read the creator string, usually a login email address"""
        return self.getValue("creatorLogin")
    
    def addMessage(self, mssgString, user, time = None):
        """add a new message to the QTopicCard"""
        self.sendMessage("addMessage:fromUser:atTime:", (mssgString, user, time))

class QRectangle(QPrimitive):
    """--- standard Rectangle primitive --- """
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TRectangle", name)

    def setExtent2(self, width, height):
        Proxy.send(self.panel, "setExtent2", self.name, width, height)

    def getExtent2(self):
        return self.getValue("getExtent2")

        
class QLabel(QRectangle):
    """--- a flat label that can be added to any object - size and orientation is defined by its transform ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QLabel", name)
        
    def setText(self, text):
        Proxy.send(self.panel, "setText", self.name, text)
        
    def getText(self):
        return self.getValue("getText")
        
    def setTextColor(self, color):
        Proxy.send(self.panel, "setTextColor", self.name,  color.rgba)
        
    def getTextColor(self):
        return QColor.fromTupleFloat(self.getValue("getTextColor"))

    def setBackgroundColor(self, color):
        Proxy.send(self.panel, "setBackgroundColor", self.name, color.rgba)
        
    def getBackgroundColor(self):
        return QColor.fromTupleFloat(self.getValue("getBackgroundColor"))
       

        
class QLabel2D(QTransformable):
    """ --- a flat label that can be added to any object - size and orientation are flat to user ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QLabel2D", name)
        
    def setText(self, text):
        Proxy.send(self.panel, "setText", self.name, text)
        
    def getText(self):
        return self.getValue("getText")
        
    def setTextColor(self, color):
        Proxy.send(self.panel, "setTextColor", self.name,  color.rgba)
        
    def getTextColor(self):
        return QColor.fromTupleFloat(self.getValue("getTextColor"))

    def setBackgroundColor(self, color):
        Proxy.send(self.panel, "setBackgroundColor", self.name, color.rgba)
        
    def getBackgroundColor(self):
        return QColor.fromTupleFloat(self.getValue("getBackgroundColor"))
       


class QCube(QPrimitive):
    """--- standard 3D cuboid primitive ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TCube", name)

    def setExtent3(self,x, y, z):
        Proxy.send(self.panel, "setExtent3", self.name, x, y, z)

    def getExtent3(self):
        return self.getValue("getExtent3")


class QSphere(QPrimitive):
    """--- standard sphere primitive ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TSphere", name)

    def setRadius(self, radius):
        Proxy.send(self.panel, "setRadius", self.name, radius)

    def getRadius(self):
        return self.getValue("getRadius")


class QConic(QPrimitive):
    """--- standard right conic section primitive ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TConic", name)

    def setHeight(self,height):
        Proxy.send(self.panel, "setHeight", self.name, height)

    def getHeight(self):
        return self.getValue("getHeight")

    def setTopRadius(self, topRad):
        Proxy.send(self.panel, "setTopRadius", self.name, topRad)

    def getTopRadius(self):
        return self.getValue("getTopRadius")

    def setBaseRadius(self, botRad):
        Proxy.send(self.panel, "setBaseRadius", self.name, botRad)

    def getBaseRadius(self):
        return self.getValue("getBaseRadius")


class QCylinder(QPrimitive):
    """--- standard right cylinder primitive ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TCylinder", name)

    def setHeight(self,height):
        Proxy.send(self.panel, "setHeight", self.name, height)

    def getHeight(self):
        return self.getValue("getHeight")

    def setRadius(self, radius):
        Proxy.send(self.panel, "setRadius", self.name, radius)

    def getRadius(self):
        return self.getValue("getRadius")
    

class QLozenge(QPrimitive):
    """--- lozenge, a rounded edged panel"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QLozenge", name)

    def setHeight(self,height):
        Proxy.send(self.panel, "setHeight", self.name, height)

    def getHeight(self):
        return self.getValue("getHeight")

    def setRadius(self, radius):
        Proxy.send(self.panel, "setRadius", self.name, radius)

    def getRadius(self):
        return self.getValue("getRadius")
 
    def setExtent2(self, width, height):
        Proxy.send(self.panel, "setExtent2", self.name, width, height)

    def getExtent2(self):
        return self.getValue("getExtent2")
   
    

class QPointCloud(QPrimitive):
    """--- point cloud object ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TPointCloud", name)

    def setVertices(self, points):
        Proxy.send(self.panel, "setVertices", self.name, points)

    def getVertices(self):
        return self.getValue("getVertices")

    def setColors(self, colors):
        """ need to convert array of QColors into array of rgba"""
        rgbColor = []
        for c in colors:
            rgbColor.append(c.rgba)
        Proxy.send(self.panel, "setColors", self.name, tuple(rgbColor))

    def getColors(self):
        return self.getValue("getColors")

    def setPointSize(self, size):
        Proxy.send(self.panel, "setPointSize", self.name, size)

    def getPointSize(self):
        return self.getValue("getPointSize")


class QTeapot(QPrimitive):
    """--- standard Utah teapot primitive ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TTeapot", name)



class QTracer(QTransformable):
    """--- a 3D line object ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QTracer", name)

    def appendPath(self, point):
        Proxy.send(self.panel, "appendPath", self.name, point)

    def appendPathXYZ(self, x, y, z):
        Proxy.send(self.panel, "appendPathXYZ", self.name, x, y, z)

    def resetPath(self):
        Proxy.send(self.panel, "resetPath", self.name)

    def setColor(self, color):
        Proxy.send(self.panel, "setColor", self.name, color.rgba)

    def setWidth(self, width):
        Proxy.send(self.panel, "setWidth", self.name, width)

    def getWidth(self, width):
        return self.getValue("getWidth")

    def setMaxSize(self, size):
        Proxy.send(self.panel, "setMaxSize", self.name, size)


class QRibbon(QTracer):
    """--- a 3D ribbon object ---"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "QRibbonTracer", name)


class QTexture(object):
    """--- a texture object ---"""
    __slots__ = 'panel','name','tClass'
    def __init__(self, panel, name = None):
        self.panel = panel
        self.tClass = "TTexture"
        if(name is None):
            self.name = panel.generateName()
        else:
            self.name = name
        panel.objectDictionary[self.name]=self

    def getTClass(self):
        return self.tClass

    def construct(self, fileName, extension = None):
        Proxy.send(self.panel, "makeTexture", self.name, fileName, extension)
  
    def getExtent(self):
        return self.getValue("extent")
    
    


class QRolodex(QGroup):
    """--- a container of QFileCards   """
    def __init__(self, panel, name = None):
        QBasic.__init__(self,panel,"QRolodex", name)

    # --- accessing ---

    def addFileCard(self, card):
        Proxy.send(self.panel, "addFileCard", self.name, card.name)

    def removeFileCard(self, card):
        Proxy.send(self.panel, "removeFileCard", self.name, card.name)
        
    def getFileCards(self):
        idList = self.getValue("getFileCards")
        return self.convertIDList(idList, QFileCard)
    

class QStandRolodex(QRolodex):
    """--- a container of QFileCards"""
    def __init__(self, panel, name = None):
        QBasic.__init__(self,panel,"QStandRolodex", name)


class QGeometry(QPrimitive):
    """--- generic geometry class"""
    
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TGeometry", name)
    
    def setVertices(self, vertices):
        Proxy.send(self.panel, "setVertices", self.name, vertices)
        
    def getVertices(self):
        return self.getValue("getVertices")
        
    def setNormals(self, normals):
        Proxy.send(self.panel, "setNormals", self.name, normals)
        
    def getNormals(self):
        return self.getValue("getNormals")
    
    def setTextureCoords(self, txtrCoords):
        Proxy.send(self.panel, "setTextureCoords", self.name, txtrCoords)
        
    def getTextureCoords(self):
        return self.getValue("getTxtrCoords")
        
    def setColors(self, colors):
        Proxy.send(self.panel, "setColors", self.name, colors)
        
    def getColors(self):
        return self.getValue("getColors")
        
    def addStrip(self,faceStrip, material, mode = "Triangles"):
        Proxy.send(self.panel, "addStripWithMode", self.name, faceStrip, material.name, mode)
    
# --- rendering mode constants

QGeometry.Points = "Points"
QGeometry.Lines = "Lines"
QGeometry.LineStrip = "LineStrip"
QGeometry.LineLoop = "LineLoop"
QGeometry.Triangles = "Triangles"
QGeometry.TriangleStrips = "TriangleStrips"
QGeometry.TriangleFan = "TriangleFan"
QGeometry.Quads = "Quads"
QGeometry.QuadStrip = "QuadStrip"
QGeometry.Polygon = "Polygon"


class QMaterial(QBasic):
    """--- Standard material model"""
    
    def __init__(self, panel, name = None):
        QBasic.__init__(self, panel, "TMaterial", name)

    def setTexture(self,texture):
        Proxy.send(self.panel, "setTexture", self.name, texture.name)

    def getTexture(self):
        return self.getValue("getTexture")    
        
    def setColor(self, color):
        Proxy.send(self.panel, "setColor", self.name, color.rgba)

    def setAmbient(self, color):
        Proxy.send(self.panel, "setAmbient", self.name, color.rgba)

    def getAmbient(self):
        return QColor.fromTupleFloat(self.getValue("getAmbient"))

    def setDiffuse(self, color):
        Proxy.send(self.panel, "setDiffuse", self.name, color.rgba)

    def getDiffuse(self):
        return QColor.fromTupleFloat(self.getValue("getDiffuse"))

    def setSpecular(self, color):
        Proxy.send(self.panel, "setSpecular", self.name, color.rgba)

    def getSpecular(self):
        return QColor.fromTupleFloat(self.getValue("getSpecular"))

    def setShininess(self, shininess):
        Proxy.send(self.panel, "setShininess", self.name, shininess)

    def getShininess(self):
        return self.getValue("getShininess")
    
    def setTransparency(self, trans):
        Proxy.send(self.panel, "setTransparency", self.name, trans)

    def getTransparency(self):
        return self.getValue("getTransparency")

    def setOpacity(self, trans):
        Proxy.send(self.panel, "setOpacity", self.name, trans)

    def getOpacity(self):
        return self.getValue("getOpacity")
    
    def setUVScale(self, scale):
        Proxy.send(self.panel, "setUVScale", self.name, scale)
        
    def getUVScale(self):
        return self.getValue("getUVScale")
    
    def setUVOffset(self, offset):
        Proxy.send(self.panel, "setUVOffset", self.name, offset)
        
    def getUVOffset(self):
        return self.getValue("getUVOffset")
    
    def setUVAngle(self, angle):
        Proxy.send(self.panel, "setUVAngle", self.name, angle)
        
    def getUVAngle(self):
        return self.getValue("getUVAngle")


   

