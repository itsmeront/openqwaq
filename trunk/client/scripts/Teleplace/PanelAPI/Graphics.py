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

# Graphics.py: Basic graphics support

from Teleplace.Proxy import Proxy
from Widget import *

class QRect(object):
    """A simple Rectangle representation"""
    __slots__ = 'x', 'y', 'w', 'h'
    def __init__(self, x, y, w, h):
        self.x = x
        self.y = y
        self.w = w
        self.h = h

    def __repr__(self):
        return "QRect(" + repr(self.x) + ", " + repr(self.y) + \
                   ", " + repr(self.w) + ", " + repr(self.h) + ")"

    def expandBy(self, pt):
        """Expand the rectangle by a given delta"""
        return QRect(self.x-pt[0], self.y-pt[1], self.w+(2*pt[0]), self.h+(2*pt[1]))

    def containsPoint(self, pt):
        """Answers whether the given point is inside the rectangle"""
        dx = pt[0] - self.x
        dy = pt[1] - self.y
        return (dx >= 0) and (dy >= 0) and (dx < self.w) and (dy < self.h)
    
    def left(self):
        return self.x
    
    def right(self):
        return self.x + self.w
    
    def top(self):
        return self.y
    
    def bottom(self):
        return self.y + self.h
    
    def topLeft(self):
        return (self.x, self.y)
    
    def bottomRight(self):
        return (self.x+self.w, self.y+self.h)
    
    def width(self):
        return self.w
    
    def height(self):
        return self.h

    @staticmethod
    def encompassing(pts):
        """Creates a rectangle spanned by the given list of points"""
        minX = pts[0][0]
        minY = pts[0][0]
        maxX = minX
        maxY = minY
        for pt in pts:
            if(pt[0] < minX): minX = pt[0]
            if(pt[1] < minY): minY = pt[1]
            if(pt[0] > maxX): maxX = pt[0]
            if(pt[1] > maxY): maxY = pt[1]
        return QRect(minX, minY, maxX-minX, maxY-minY)

class QColor(object):
    """A simple 32bit RGBA color representation."""
    __slots__ = 'rgba'

    @staticmethod
    def fromRGBA(rgba):
        cc = QColor(0, 0, 0, 0)
        cc.rgba = rgba
        return cc

    @staticmethod
    def fromTupleFloat(tcolor):
        if(tcolor is None):
            return QColor(0,0,0,0)
        else:
            cc = QColor(int(tcolor[0]*255),
                        int(tcolor[1]*255),
                        int(tcolor[2]*255),
                        int(tcolor[3]*255))
        return cc

    @staticmethod
    def fromFloat(f0,f1,f2,f3=1):
        return QColor(int(f0*255), int(f1*255), int(f2*255), int(f3*255))
    
    def __init__(self, r, g, b, a = 255):
        """Initializes the color object using r,g,b, and a"""
        self.rgba = r +  (g << 8) + (b << 16) + (a << 24)

    def __repr__(self):
        """Answers a readable representation of QColor"""
        return "QColor(" + repr(self.red() ) + ", " + repr(self.green()) + \
                    ", " + repr(self.blue()) + ", " + repr(self.alpha()) + ")"

    def __cmp__(self, other):
        """Compares two colors by their values"""
        return long(self.rgba).__cmp__(long(other.rgba))

    def red(self):
        """Returns the red component of the color (0..255)"""
        return int(self.rgba & 255)

    def blue(self):
        """Returns the blue component of the color (0..255)"""
        return int(self.rgba >> 16) & 255

    def green(self):
        """Returns the green component of the color (0..255)"""
        return int(self.rgba >> 8) & 255

    def alpha(self):
        """Returns the alpha component of the color (0..255)"""
        return int(self.rgba >> 24) & 255

#Color constants: QColor.Transparent, QColor.White etc.
QColor.Transparent = QColor(  0,   0,   0,   0)
QColor.White       = QColor(255, 255, 255, 255)
QColor.Black       = QColor(  0,   0,   0, 255)
QColor.Red         = QColor(255,   0,   0, 255)
QColor.Pink        = QColor(255, 180, 180, 255)
QColor.Green       = QColor(  0, 255,   0, 255)
QColor.LtGreen     = QColor(180, 255, 180, 255)
QColor.Blue        = QColor(  0,   0, 255, 255)
QColor.LtBlue      = QColor(180, 180, 255, 255)
QColor.Cyan        = QColor(  0, 255, 255, 255)
QColor.Magenta     = QColor(255,   0, 255, 255)
QColor.Yellow      = QColor(255, 255,   0, 255)
QColor.Gray        = QColor(128, 128, 128, 255)
QColor.DarkGray    = QColor( 64,  64,  64, 255)
QColor.DarkerGray  = QColor( 32,  32,  32, 255)
QColor.DarkestGray = QColor( 16,  16,  16, 255)
QColor.LightGray   = QColor(192, 192, 192, 255)
QColor.LighterGray = QColor(224, 224, 224, 255)
QColor.LightestGray= QColor(244, 244, 244, 255)

#Blending Rules
QColor.Over  = 3
QColor.Reverse = 6
QColor.Blend = 24

class QImage(object):
    __slots__ = 'name', 'width', 'height', 'depth'

    # Rule constants:deprecated
    Over  = 3
    Reverse = 6
    Blend = 24

    def __init__(self, name, w, h, d):
        self.name = name
        self.width = w
        self.height = h
        self.depth = d

    def __repr__(self):
        return ("QImage(" + repr(self.name) + "," + repr(self.width) +
                "," + repr(self.height) + "," + repr(self.depth) + ")")

    def bounds(self):
        """Returns the bounding box for the receiver"""
        return QRect(0, 0, self.width, self.height)

    def getName(self):
        return self.name

    def getClassName(self):
        # this is part of the persistence mechanism
        return type(self).__name__


class QCanvas(QWidget):
    """QCanvas is a canvas for a QPanel, implementing various graphics operations"""

    def __init__(self, panel, name = None):
        """Initializes the canvas for a given panel"""
        QWidget.__init__(self, panel,"QPythonDraw", name)

    # --- Invalidations ---
    def flush(self):
        self.sendMessage("flushDisplay")

    # --- clearing rectangles ---
    def clearColor(self, color):
        self.sendMessage("clearColor:", color.rgba)

    def clipRect(self, x, y, w, h):
        self.sendMessage("clipRectX:y:w:h:", (x, y, w, h))

    def clearRect(self, x, y, w, h, color):
        self.sendMessage("clearRectX:y:w:h:color:", (x, y, w, h, color.rgba))

    # --- drawing ovals ---
    def drawOval(self, x, y, w, h, c, bw, bc):
        self.sendMessage("drawOvalX:y:w:h:color:bw:bc:",(x, y, w, h, c.rgba, bw, bc.rgba))

    def fillOval(self, x, y, w, h, color):
        self.drawOval(x, y, w, h, color, 0, QColor.Transparent)

    def frameOval(self, x, y, w, h, bw, bc):
        self.drawOval(x, y, w, h, QColor.Transparent, bw, bc)

    # --- drawing rectangles ---
    def drawRect(self, x, y, w, h, c, bw, bc):
        self.sendMessage("drawRectX:y:w:h:color:bw:bc:", (x, y, w, h, c.rgba, bw, bc.rgba))

    def fillRect(self, x, y, w, h, color):
        self.drawRect(x, y, w, h, color, 0, QColor.Transparent)

    def frameRect(self, x, y, w, h, bw, bc):
        self.drawRect(x, y, w, h, QColor.Transparent, bw, bc)

    # --- drawing round rects ---
    def drawRoundRect(self, x, y, w, h, r, c, bw, bc):
        self.sendMessage("drawRoundRectX:y:w:h:r:color:bw:bc:", (x, y, w, h, r, c.rgba, bw, bc.rgba))

    def fillRoundRect(self, x, y, w, h, r, color):
        self.drawRoundRect(x, y, w, h, r, color, 0, QColor.Transparent)

    def frameRoundRect(self, x, y, w, h, r, bw, bc):
        self.drawRoundRect(x, y, w, h, r, QColor.Transparent, bw, bc)

    # --- drawing lines ---
    def drawLine(self, x0, y0, x1, y1, w, c):
        self.sendMessage("drawLineX1:y1:x2:y2:width:color:", (x0, y0, x1, y1, w, c.rgba))

    # --- drawing polygons ---
    def drawPoly(self, pts, c, bw, bc):
        self.sendMessage( "drawPoly:color:bw:bc:", (pts, c.rgba, bw, bc.rgba))

    def fillPoly(self, pts, color):
        self.drawPoly(pts, color, 0, QColor.Transparent)

    def framePoly(self, pts, bw, bc):
        self.drawPoly(pts, QColor.Transparent, bw, bc)

    # --- text support ---
    def selectFont(self, fontName, fontSize, fontEmphasis = 0):
         return Proxy.send(self.panel, "selectFont", fontName, fontSize, fontEmphasis)

    def measureString(self, string, start, stop, font):
        return Proxy.send(self.panel, "measureString", string, start, stop, font)

    def drawString(self, string, x, y, font, fontColor):
        self.sendMessage( "drawString:x:y:font:color:", (string, x, y, font, fontColor.rgba))

    # -- image support ---

    def loadImage(self, imageName, data=None):
        """ construct a remote image either by loading it via the imageName or directly via the data"""
        return Proxy.send(self.panel, "loadImage", imageName, data)
        
    def getImageData(self, image):
        """return the original data used to construct the image"""
        return Proxy.send(self.panel, "getImage", image.name)
    
    def clearImage(self, image):
        """remove the specified image from the forum"""
        Proxy.send(self.panel, "clearImage", image.name)

    def drawImage(self, image, rule, dstX, dstY, srcRect,scale = None):
        self.sendMessage("drawImage:rule:dst:src:ext:scale:", (image.name, rule, (dstX, dstY),
                   (srcRect.x, srcRect.y), (srcRect.w, srcRect.h), scale))

    # -- combination rule ---

    def reverseOn(self):
        self.sendMessage("reverseOn")
        #Proxy.send(self.panel, "reverseOn")

    def reverseOff(self):
        self.sendMessage("reverseOff")
        #Proxy.send(self.panel, "reverseOff")

    # -- miscellaneous---

    def deferUpdates(self, aBool):
        self.sendMessage("deferUpdates:", aBool)

QCanvas.Plain = 0
QCanvas.Bold = 1
QCanvas.Italic = 2
QCanvas.Underlined = 4
QCanvas.Narrow = 8
QCanvas.StrikeOut = 16

class QDisplayList(object):
    __slots__ = 'panel','name','qClass'

    def __init__(self, panel, name = None):
        """Initializes a display list"""
        self.panel = panel
        self.qClass = "PDisplayList"
        if(name is None):
             self.name = Proxy.send(self.panel, "makeNew", self.qClass)
        else:
            self.name = name
        panel.objectDictionary[self.name]=self

    def begin(self):
        """ begin() sets drawing into a capture mode, where all drawing - irrespective of target canvas
        is added to the current display list"""
        Proxy.send(self.panel, "beginDisplayList", self.name)

    def end(self):
        """ end() closes the drawing capture mode. All succeeding drawing calls are no longer added
        to the display list"""
        Proxy.send(self.panel, "endDisplayList", self.name)

    def draw(self, toCanvas = None):
        """ draw(toCanvas) draws the display list to the specified canvas. If the canvas is undefined, it
        defaults to the main canvas."""
        if toCanvas is not None:
            targetName = toCanvas.name
        else:
            targetName = "__mainCanvas__"
        Proxy.send(self.panel, "drawDisplayListOS", self.name, 0, 0, 1, 1, targetName)
        #self.drawOffsetScaled(0, 0, 1, 1, toCanvas)

    def drawOffsetScaled(self, xloc, yloc, xscale, yscale, toCanvas = None):
        """drawOffsetScaled draws the display list at the specified offset and scale locations. If the
        canvas is undefined, it defaults to the main canvas"""
        if toCanvas is not None:
            targetName = toCanvas.name
        else:
            targetName = "__mainCanvas__"
        Proxy.send(self.panel, "drawDisplayListOS", self.name, xloc, yloc, xscale, yscale, targetName)

    def destroy(self):
        """destroy() will remove the display list from the system, which frees up memory to be used for
        other things"""
        Proxy.send(self.panel, "destroyDisplayList", self.name)
        del self.panel.objectDictionary[self.name]


         
    
