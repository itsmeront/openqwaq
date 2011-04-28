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

# HelloWorld.py: A very simple Qwaq Python application

# Import the Panel API
from Qwaq.PanelAPI import *

class HelloWorld(QPanelApp):
    """A simple panel application """

    def onCreate(self):
        """Initializes the hello world application"""
        self.myColor = QColor.Cyan
        self.helloText = "Hello, World"
        self.panel["hello"] = self.helloText

    def onNewUser(self, userID):
        """A complete redraw when a new user arrives - including the first one"""
        self.setExtent(400, 100) #set the extent of the panel in pixels
        self.draw()
        
    def draw(self):
        """Draws the application on the given canvas"""
        # Clear its background to cyan
        self.canvas.clearColor(self.myColor)
        font = self.canvas.selectFont("BitstreamVeraSans", 64,0)
        sz = self.canvas.measureString(self.helloText,0,len(self.helloText),font)
        px = (self.extent[0] - sz[0]) // 2
        py = (self.extent[1]- sz[1]) // 2.
        self.canvas.drawString(self.helloText,px, py, font, QColor.Black)
        self.canvas.flush()
 
