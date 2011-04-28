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

# Scribbler.py: The main file for a Python Panel App

# Import the Panel API
from Qwaq.PanelAPI import *
from Button import *
import StringIO # needed for load/save

class Scribbler(QPanelApp):
    """A simple panel application """

    # --------------------------------------------------------------- #
    # Initialization
    # --------------------------------------------------------------- #

    def onCreate(self):
        """Initializes the paint application"""

        # Debug covers various amounts of logging info. The use of println()
        # is pretty slow so some of the more extensive logs are only shown
        # if debug is enabled.
        self.debug = True
        if(self.debug):
            self.println("[Scribbler]: Local User ID is " + str(self.userID))
            self.println("[Scribbler]: Base User ID is " + str(self.panel.getBaseUserID()))           
            self.println("[Scribbler]: I am controlling:" + str(self.userID))
    
            # Print some status info in the Qwaq Forums transcript
            self.println("[Scribbler]: Creating new app")
            self.println("[Scribbler]: API version is " + str(self.apiVersion))
            self.println("[Scribbler]: Document name is " + str(self.panel.getDocumentName()))

            # Some extra info: Print all the user names currently logged in
            userNames = ", ".join([self.panel.getUserName(uid) for uid in self.panel.getUsers()])
            self.println("[Scribbler]: Users in this space: " + userNames)
        # userFilter can be set to constrain the app to only respond to a single user.
        # its default is None as here.
        self.userFilter = None

        # Set the defaults for our application
        self.lastPos = (0, 0)
        self.activeRadius = 5
        self.activeColor = QColor.Red.rgba
        self.strokes = self.panel["strokes"]
        self.activePoints = ()
        self.activeStroke = None
        self.drawing = False
        self.dlStrokes = ()
        
        # Set up the buttons
        self.buttons = []
        self.setExtent(800, 600)
        self.setupColorButtons()
        self.setupToolButtons()

        # If strokes exists, then the document has already been loaded.
        if(self.strokes is None):
            if(self.panel.getDocumentName() is None):
                # No document name given - initialize empty document
                self.strokes = ()
                # Save the strokes
                self.panel["strokes"] = self.strokes
                self.ready = True
            else:
                # The document hasn't been loaded yet. 
                # We will receive an event when it is.
                self.ready = False
        else:
            # The document has been loaded already
            for stroke in self.strokes:
                self.addDLStroke(stroke)
            self.ready = True        
        # Construct a display list for the application frame
        self.makeAppFrame() 

    # --------------------------------------------------------------- #

    def onDestroy(self):
        """Cleans up after the paint application"""
        self.dlAppFrame.destroy()
        for dlStroke in self.dlStrokes:
            dlStroke.destroy()
        if(self.debug): self.println("[Scribbler]: Going away, bye, bye")

    # --------------------------------------------------------------- #

    def onNewUser(self, userID):
        """A complete redraw when a new user arrives"""
        if(self.debug): self.println("[Scribbler]: Add new user")
        self.setExtent(800, 600)
        self.draw()
        
    # --------------------------------------------------------------- #

    def onLoadSuccessEvent(self, success):
        if success:
            if(self.debug): self.println("[Scribbler]: onLoadSuccessEvent data is loaded")
            self.strokes = self.panel["strokes"]
            for stroke in self.strokes:
                self.addDLStroke(stroke)
        else:
            # No document name given - initialize empty document
            if(self.debug): self.println("[Scribbler]: onLoadSuccessEvent failure")
            self.strokes = ()
            # Save the strokes
            self.panel["strokes"] = self.strokes

        self.ready = True
        self.draw() # redraw strokes

    # --------------------------------------------------------------- #

    def setupColorButtons(self):
        """Initializes all the color buttons"""
        allColors = (QColor.White, QColor.Black, QColor.LightGray, QColor.Red, QColor.Yellow,
                     QColor.Green, QColor.Cyan, QColor.Blue, QColor.Magenta)
        px = self.extent[0] - 36
        py = (self.extent[1] - len(allColors) * 32) // 2
        for color in allColors:
            box = QRect(px, py, 32, 32)
            button = Button(box, None, color, False)
            button.action = self.activateColor
            button.argument = color.rgba
            self.buttons.append(button)
            py = py + 31

    # --------------------------------------------------------------- #

    def setupToolButtons(self):
        """Initializes all the tool buttons"""
        toolSpec = (
            ("ClearButton.png", self.clearStrokes, None, QColor.White),
            (),
            # ("SelectTool.png", self.activateSelect, None, QColor.White),
            # ("TextTool.png", self.activateText, None, QColor.White),
            ("SmallBrush.png", self.activateBrush, 2, QColor.White),
            ("MidBrush.png", self.activateBrush, 5, QColor.White),
            ("LargeBrush.png", self.activateBrush, 10, QColor.White),
            (),
            ("DebugButton.png", self.toggleDebug, None, QColor.White),
        )
        px = 4
        py = (self.extent[1] - len(toolSpec) * 32) // 2

        for spec in toolSpec:
            if(len(spec) > 0):
                pic = spec[0]
                image = self.canvas.loadImage(pic)
                if(image is None): self.println("[Scribbler]: Failed to load " + pic)
                box = QRect(px, py, 32, 32)
                button = Button(box, image, spec[3], False)
                button.action = spec[1]
                button.argument = spec[2]
                self.buttons.append(button)
            py = py + 31

    # --------------------------------------------------------------- #

    def activateBrush(self, button, radius):
        """Activates a particular brush size for drawing"""
        self.activeRadius = radius

    # --------------------------------------------------------------- #

    def activateColor(self, button, rgba):
        """Activates a particular drawing color"""
        self.activeColor = rgba

    # --------------------------------------------------------------- #

    def clearStrokes(self, button, ignored):
        """Toggle the current drawing tool"""
        self.strokes = ()
        for dls in self.dlStrokes:
            dls.destroy()
        self.dlStrokes = ()
        self.panel["strokes"] = ()
        self.activePoints = ()
        self.draw()

    # --------------------------------------------------------------- #

    def toggleDebug(self, button, ignored):
        """Toggles the amount of debug output"""
        # The debug property is purely local and has no effect on the replicated
        # environment. If we wanted to use it replicated we'd need to use the
        # same steps as for color or brush sizes.
        self.debug = not(self.debug)
        if(self.debug): self.println("[Scribbler]: Debug is now ON")
        else: self.println("[Scribbler]: Debug is now OFF")

    # --------------------------------------------------------------- #

    def onKeyStroke(self, event):
        self.println("onKeyStroke.event.value, buttons: " + str((event.type,event.value, event.buttons)))
    
    # --------------------------------------------------------------- #

    def onMouseDown(self, event):
        """Handles the mouseDown event"""
        if(self.debug): self.println("[Scribbler]: onMouseDown")
        if(not(self.ready)): return # we aren't ready yet

        if(event.buttons & 4):
            # See if we pressed inside a button
            for button in self.buttons:
                if(button.bounds.containsPoint(event.position)):
                    return button.action(button, button.argument)
            # Otherwise start a new stroke
            self.startStroke(event)

    # --------------------------------------------------------------- #

    def onMouseMove(self, event):
        """Handles the mouseMove event"""
        if(self.drawing):
            self.followStroke(event)

    # --------------------------------------------------------------- #

    def onMouseUp(self, event):
        """Handles the mouseUp event"""
        if(self.drawing):
            self.followStroke(event)
            self.endStroke(event)


    # --------------------------------------------------------------- #

    def startStroke(self, event):
        self.activePoints = (event.position,)
        # And draw immediately
        self.lastPos = event.position
        self.drawTo(event.position)
        self.drawing = True

    # --------------------------------------------------------------- #

    def followStroke(self, event):
        """Takes a mouseMove event and continues to draw the current stroke"""
        if(self.lastPos == event.position): return
        self.activePoints = self.activePoints+(event.position,)
        self.drawTo(event.position)

    # --------------------------------------------------------------- #

    def endStroke(self, event):
        """Takes a mouseUp event and finalizes the current stroke"""
        stroke = (self.activeColor,
                  self.activeRadius,
                  self.activePoints)
        self.strokes = self.strokes + (stroke,)  # Append local (cached)
        self.panel.append("strokes", stroke)     # Append replicated
        self.drawing = False
        self.addDLStroke(stroke).drawOffsetScaled(0,0,1,1)

    # --------------------------------------------------------------- #

    def addDLStroke(self, stroke):
        newStroke = QDisplayList(self.panel)
        newStroke.begin()
        self.drawStroke(stroke[0], stroke[1], stroke[2])
        newStroke.end()
        self.dlStrokes = self.dlStrokes + (newStroke,)
        return newStroke

    # --------------------------------------------------------------- #

    def drawTo(self, newPos):
        """Draws a line from to the next position. Updates lastPos."""
        self.canvas.drawLine(self.lastPos[0], self.lastPos[1],
                             newPos[0], newPos[1],
                             self.activeRadius*2,
                             QColor.fromRGBA(self.activeColor))
        rect = QRect.encompassing((self.lastPos, newPos))
        rect = rect.expandBy((self.activeRadius, self.activeRadius))
        self.canvas.flush()
        self.lastPos = newPos

    # --------------------------------------------------------------- #

    def drawStroke(self, rgba, radius, pts):
        """Draws a stroke on the given canvas"""
        lastPt = None
        if(len(pts)==1):lastPt = pts[0]
        for nextPt in pts:
            if(lastPt is not None):
                self.canvas.drawLine(lastPt[0], lastPt[1],
                            nextPt[0], nextPt[1],
                            radius*2, QColor.fromRGBA(rgba))
            lastPt = nextPt

    # --------------------------------------------------------------- #

    def makeAppFrame(self):
        """Draws the application's frame and title"""
        # Obviously, this is just an example for how to draw text since
        # we have the application title already set initially.
        title = "Scribbler"
        font = self.canvas.selectFont("Default", 24, 0)
        if(self.debug): self.println("[Scribbler]: Selected font is " + str(font))
        sz = self.canvas.measureString(title, 0, len(title), font)
        px = (self.extent[0] - sz[0]) // 2
        py = sz[1] // 2
        self.dlAppFrame = QDisplayList(self.panel)
        self.dlAppFrame.begin()
        self.canvas.frameRect(20, 10 + py, self.extent[0] - 40, self.extent[1] - py - 20,
                         1, QColor.Black)
        self.canvas.fillRect(px-10, py, sz[0]+20,sz[1], QColor.White)
        self.canvas.drawString(title, px, 10, font, QColor.Black)
        self.dlAppFrame.end()

    # --------------------------------------------------------------- #

    def draw(self):
        """Draws the application on the given canvas"""
        if(self.debug): self.println("draw")
        # Clear its background to white
        self.canvas.clearColor(QColor.White)
        # Draw the existing strokes
        if(self.ready):
            for dls in self.dlStrokes:
               dls.draw()
            # And the currently active stroke using the old method...
            self.drawStroke(self.activeColor,
                            self.activeRadius,
                            self.activePoints)
        
        # Draw the app frame (on top of the strokes)
        self.dlAppFrame.draw()
        # Draw the buttons
        for button in self.buttons:
            button.draw(self.canvas)
        # And force an update
        self.canvas.flush()
