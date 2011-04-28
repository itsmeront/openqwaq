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

# Button.py: A simple button

from Qwaq.PanelAPI import *

class Button(object):
    """A simple button object for the Scribble application"""

    def __init__(self, bounds, image, color, selected):
        """Initialize with the given set of attributes"""
        self.image = image
        self.color = color
        self.bounds = bounds
        self.selected = selected

    def draw(self, canvas):
        """Draws the button on the given canvas"""
        px = self.bounds.x
        py = self.bounds.y
        pw = self.bounds.w
        ph = self.bounds.h
        # Draw selection if button is selected
        if(self.selected):
            canvas.drawRect(px, py, pw, ph, QColor.White, 2, QColor.Black)
        else:
            canvas.drawRect(px, py, pw, ph, QColor.White, 1, QColor.Black)
        # Draw image or color (depending on image presence)
        if(self.image is None):
            canvas.fillRect(px+2, py+2, pw-4, pw-4, self.color)
        else:
            canvas.drawImage(self.image, QImage.Over, px, py, self.image.bounds())
