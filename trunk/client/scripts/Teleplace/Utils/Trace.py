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

# Trace.py: Tracing information for OpenQwaq

# A lot of more information can be retrieved from the trace,
# but we are happy with this for the moment...
import sys

lineNumber = 0
value = ""
exception = None
traceLimit = 99
trace = []

def tracer(frame, event, arg):
    global lineNumber
    global value
    global exception
    global trace

    # This is ugly. It appears that when an exception occurs the tracer
    # is being called for each return frame in the stack. We're only
    # interested in the first one (since it contains the whole stack)
    # so we need to guard against the others. Unfortunately, this means
    # we do need an explicit resetTrace below.
    if event == "exception" and exception is None:
        lineNumber = frame.f_lineno
        value = arg[1]
        exception = arg[0]
        n = 0
        trace = []
        while((frame is not None) and (n < traceLimit)):
            lineno = frame.f_lineno
            code = frame.f_code
            fileName = code.co_filename
            name = code.co_name
            trace.append("File %s, line %d, in %s\r" %(fileName, lineno, name))
            frame = frame.f_back
    return tracer

def printStack():
    """Returns a string with the last stack trace"""
    return "".join(trace)

def resetTrace():
    """Resets the trace to catch the next exception"""
    global exception
    exception = None

sys.settrace(tracer)
