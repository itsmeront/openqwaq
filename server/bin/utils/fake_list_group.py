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
fake_list_group.py: A mock group listing script for testing the interface.
This script provides an outline for how to implement a proper group list
script. Usage:

  fake_list_group.py -s sample.server.com -u user@domain.com -p pass -g group

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
  opts.add_option("-s",
	help="enter authentication server to use",
	action="store",type="string",dest="server",default=None)
  opts.add_option("-g",
	help="enter group name",
	action="store",type="string",dest="group",default=None)

  (options, args) = opts.parse_args(sys.argv[1:])

  return options 

def checkOptions(options):
  """
  Verifies the necessary options. Raises ValueError if
  any of the required information is not available.
  """
  if options.user == None:
  	raise ValueError, "A user must be specified (-u)"
  if options.password == None:
  	raise ValueError, "A pass code must be specified (-p)"
  if options.server == None:
  	raise ValueError, "A server must be specified (-l)"
  if options.group == None:
  	raise ValueError, "A group must be specified (-g)"

def listing(options):
    """
    Lists all the existing users in the group. Returns a (made up)
    list of users for the mock script.
    """
    results = []
    for i in range(1, 10):
      name = "user" + str(i)
      user = UserData(name, name+"@domain.com")
      # This information is optional
      user.firstName = 'Joe' + str(i)
      user.lastName = 'User'
      user.displayName = 'Joe R. User'
      user.company = 'Sample Corp'
      results.append(user)
    return results

def main():
  try:
    options = parseOptions();
    checkOptions(options);
    users = listing(options);
    printListing(users);
  except:
    printError(str(sys.exc_value))

if __name__ == "__main__":
    main()
