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

# Croquet.py: Access to Croquet object types

from Teleplace.Proxy import Proxy

class TEvent(object):
	def __init__(self, buttons):
		self.buttons = buttons

	def anyButtonPressed(self):
		return self.buttons <> 0

class TObject(object):
	"TObject implements various bits of interaction between Croquet and Python"
	def __setattr__(self, item, value):
		"Inform Croquet about changing the value"
		Proxy.send(self, "pySet:to:", item, value)
		return object.__setattr__(self, item, value)

	def __getattr__(self, item):
		"Query Croquet about this value"
		value = Proxy.send(self, "pyGet:", item)
		if(value <> None): object.__setattr__(self, item, value)
		return value

class TFrame(TObject):
	"TFrame is the base class for all 3D Croquet objects"
	distance = 0.0
	speed = 0.0

	def setSpeed(self, speed):
		self.speed = speed

	def setDistance(self, distance):
		self.distance = distance

	def addRotationAroundX(self, angle):
		"Add to the rotation around the x-axis"
		Proxy.send(self, "addRotationAroundX:", angle)

	def addRotationAroundY(self, angle):
		"Add to the rotation around the y-axis"
		Proxy.send(self, "addRotationAroundY:", angle)

	def addRotationAroundZ(self, angle):
		"Add to the rotation around the y-axis"
		Proxy.send(self, "addRotationAroundZ:", angle)

	def translateBy(self, deltaX, deltaY, deltaZ):
		"Translate this frame by delta X, Y, and Z"
		Proxy.send(self, "translateByX:y:z:", deltaX, deltaY, deltaZ)

	def colorize(self, r, g, b):
		"Translate this frame by delta X, Y, and Z"
		Proxy.send(self, "colorizeR:g:b:", r, g, b)

	def playSound(self, soundName):
		"Play the sound with the given name"
		Proxy.send(self, "playSound:", soundName)

	def startStepping(self, function):
		"Start stepping the given function"
		Proxy.send(self, "startStepping:", function)

	def stopStepping(self, function):
		"Stop stepping the given function"
		Proxy.send(self, "stopStepping:", function)

	def random(self, maxValue):
		"Answer a random number between 0.0 and maxValue"
		return Proxy.send(self, "random:", maxValue)

	def moveTowards(self, target, amount, distance):
		"Move towards the given target by the specified amount"
		Proxy.send(self, "moveTowards:amount:distance:", target, amount, distance)

	def removeAllScripts(self):
		"Remove all scripts"
		Proxy.send(self, "removeAllScripts")


	def future(self, msecs):
		return TMessageMaker(self, msecs)

class TFutureMessage:

	def __init__(self, msecs, frame, function):
		self.msecs = msecs
		self.function = function

	def __call__(self, *args):
		self.args = args
		self.schedule()

	def schedule(self):
		Proxy.send(target, "future:invoke:", msecs, self)

	def invoke(self):
		function.__call__(*args)

class TMessageMaker:
	def __init__(self, frame, msecs):
		self.frame = frame
		self.msecs = msecs

	def __getattr__(self, attrname):
		return TFutureMessage(self.msecs, frame, frame.__getattr__(attrname))
