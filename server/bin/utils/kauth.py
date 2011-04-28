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
kauth.py: Active Directory authentication using Kerberos.

  kauth.py -s ldap_server -b base_dn -u user@domain.com -p pass

"""

# authbase holds some utilities for delegated auth
from authbase import *

# optparse for argument parsing, sys for various system pieces
import optparse, sys
import re

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
  opts.add_option("-l",
        help="enter the client security certificate to use for secure \
        ldap queries e.g. TLS/SSL.  This implies that normal insecure \
        binds will not take place so this augments the -l option",
        action="store",type="string",dest="ldaps_cert",default=None)
  opts.add_option("-k",
        help="enter the absolute path name of the client TGT+TGS cert \
        file to use e.g. the location and name of where the TGT file \
        will be placed",action="store",type="string",dest="cert",default=None)
  opts.add_option("-f",
        help="request a forwardable ticket",
        action="store_true",dest="forward",default=None)
  opts.add_option("-b",
        help="specify the base DN for the ldap search",
        action="store",type="string",dest="base_dn",default=None)
  opts.add_option("-d",
        help="turn on debugging",
        action="store_true",dest="debug",default=None)

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
  	raise ValueError, "An ldap server must be specified (-s)"
  if options.base_dn == None:
  	raise ValueError, "A base dn must be specified (-b)"

def authenticate(options):
    """
    Authenticate then return user information for the authencated user.
    This version does kerberos authentication.
    """

    kinit(options)
    user = userLdapInfo(options)
    kdestroy(options)

    return user

def authenticate2(options):
    """
    Authenticate then return user information for the authencated user.
    This version does serialized simple bind operations
    """
    lastErr = lastFail = None
    for server in options.server.split():
      try:
        try:
          return simpleLdapInfo(options, server)
        except authFail:
          lastFail = sys.exc_info()
          continue
      except:
        lastErr = sys.exc_info()
        continue

    # If we caught an error, resignal the error instead of
    # authFailure. Errors imply more severe problems.
    if(lastErr is not None):
      raise lastErr[0], str(lastErr[1])
    else:
      raise lastFail[0], str(lastFail[1])

def authenticate3(options):
    """
    Authenticate then return user information for the authencated user.
    This version does concurrent simple bind operations
    """
    return simpleLdapInfo2(options, options.server.split())

def main():
  try:
    try:
      options = parseOptions();
      checkOptions(options);
      
      reply = authenticate3(options);
      printReply(reply);
    except authFail:
      printFailure(sys.exc_info())
  except:
    printError(sys.exc_info())

if __name__ == "__main__":
    main()
