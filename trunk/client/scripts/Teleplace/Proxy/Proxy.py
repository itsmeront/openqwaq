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

""" Proxy.py: Teleplace proxy
This module exists for the sole purpose of loading it prior to everything else
when running from inside Teleplace Forums. In this case, the send() function will
be replaced with the actual proxy send() function which ends up in Teleplace.
You might replace this function for debugging purposes with whatever you'd
like (you could for example replace it with your own local proxy that feeds
back into your local services)."""

def send(*args):
    "The send function is the bridge into Teleplace"
    raise NotImplementedError, "Teleplace is not present"
