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

# Importer.py: An importer which allows us to revert to a previously
# used set of modules. Useful for development.
#

import sys, __builtin__

originalModules = {}
updatedModules = {}
builtinImport = None
installed = 0

def customImport(modname, globals=None, locals=None, fromlist=[]):
    global updatedModules
    result = apply(builtinImport, (modname, globals, locals, fromlist))
    updatedModules[modname] = 1
    return result

def forceReload():
    global originalModules, updatedModules
    for modname in updatedModules.keys():
        if not originalModules.has_key(modname):
            try:
                del(sys.modules[modname])
            except:
                pass

def installModule():
    global originalModules, updatedModules, builtinImport, installed
    if installed == 0:
        originalModules = sys.modules.copy()
        builtinImport = __builtin__.__import__
        __builtin__.__import__ = customImport
        updatedModules = {}
        installed = installed + 1

def uninstallModule():
    if installed > 0:
        __builtin__.__import__ = builtinImport
        builtinImport = None
        installed = installed - 1
