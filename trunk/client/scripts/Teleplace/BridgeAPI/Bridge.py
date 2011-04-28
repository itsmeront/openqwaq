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

# Bridge.py: Teleplace Bridge Interface

from Teleplace.Proxy import Proxy
import sys

class QBridge(object):
    """QBridge represents a facade to the actual Teleplace bridge"""

    def __init__(self):
        self.objectDictionary = {}

    # ---------------------------------------------------------#
    # Test functions:
    #

    def test0(self):
        """Simple test callback"""
        return Proxy.send(self, "test0")

    def test1(self, arg1):
        """Simple test callback"""
        return Proxy.send(self, "test1", arg1)

    def test2(self, arg1, arg2):
        """Simple test callback"""
        return Proxy.send(self, "test2", arg1, arg2)

    def test3(self, arg1, arg2, arg3):
        """Simple test callback"""
        return Proxy.send(self, "test3", arg1, arg2, arg3)

    def getUserID(self):
        """Retrieves the ID for the local user"""
        return Proxy.send(self, "getUserID")
    
    def println(self, obj, replicated = False):
        """Prints obj into the Teleplace transcript"""
        Proxy.send(self, "println", str(obj),replicated)
        
    def apiVersion(self):
        """Retrieves the Panel API version which is the version string of the Teleplace Client. """
        return Proxy.send(self, "apiVersion")
