#!/usr/bin/python

#
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
#
#

"""
demon_auth.py: A mock authentication script for testing the interface.
This script provides an outline for how to implement a proper auth
script. Usage:

  demon_auth.py -u user@domain.com -p pass

"""

# authbase holds some utilities for delegated auth
from authbase import *

# optparse for argument parsing, sys for various system pieces
import optparse, sys

def parseOptions():
  """
  Parses the options for this script and returns an 'options'
  object holding all the values that have been parsed from the
  command line.
  """
  opts = optparse.OptionParser(version="%prog v0.9")

  opts.add_option("-u",
	help="enter username in user name principal format",
	action="store",type="string",dest="user",default=None)
  opts.add_option("-p",
	help="enter pass code",
	action="store",type="string",dest="password",default=None)

  (options, args) = opts.parse_args(sys.argv[1:])

  return options 

def checkOptions(options):
  """
  Verifies the necessary options. Raises ValueError if
  any of the required information is not available.
  """
  if options.user == None:
  	raise ValueError, "A user must be specified (-u)"

def authenticate(options):
    """
    Authenticates the user and returns the data for the authencated user.
    Here (the script being a mock) no authentication takes place.
    """

    if(options.user[:5] == 'extra' or
       options.user[:6] == 'belial'):

      user = UserData(options.user, options.user)

      # This information is optional
      user.firstName = None
      user.lastName = None
      user.displayName = None
      user.company = None

      # For testing, we list every user in two groups
      user.groups.append("demons")
      return user

    else:
      raise ValueError, "Not a demon"

def main():
  try:
    options = parseOptions();
    checkOptions(options);

    reply = authenticate(options);
    printReply(reply);
  except:
    printFailure(sys.exc_info())

if __name__ == "__main__":
    main()
