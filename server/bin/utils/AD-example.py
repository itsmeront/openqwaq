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
from authbase import *
# optparse for argument parsing, sys for various system pieces
import optparse, sys
import re

import authbase
# Default debugging is false
authbase.DEBUG = False
# Default bind user is off/false
authbase.BINDUSER = False

# Need all these SSL/TLS/LDAP options:
# Force SSL/TLS
ldap.set_option(ldap.OPT_X_TLS_DEMAND,1)
# Force use of user+server client certs, if -l is chosen
# this should be set to 1 and the cert passed on the 
# next line
ldap.set_option(ldap.OPT_X_TLS_REQUIRE_CERT,0)
#ldap.set_option(ldap.OPT_TLS_CERTFILE,"your_cert_file")
# Force LDAPv3
ldap.set_option(ldap.OPT_PROTOCOL_VERSION,ldap.VERSION3)
# Timeout to LDAP/AD server in 2 seconds
ldap.set_option(ldap.OPT_NETWORK_TIMEOUT,5)
# On our AD servers, referals come back correctly, however,
# the referals are bogus/blank, turn off referal chasing for now
ldap.set_option(ldap.OPT_REFERRALS, 1)

#############################
#  Funcs/classes
#############################

def parseOptions():
  """
  Parses the options for this script and returns an 'options'
  object holding all the values that have been parsed from the
  command line.
  """
  opts = optparse.OptionParser(version="%prog v0.9")

  opts.add_option("-u",
        help="enter authenticated username in user name principal format",
        action="store",type="string",dest="user",default=None)
  opts.add_option("-p",
        help="enter authenticated user pass code",
        action="store",type="string",dest="password",default=None)
  opts.add_option("-U",
        help="enter bind user's AD UPN name",
        action="store",type="string",dest="bindUser",default="")
  opts.add_option("-P",
        help="enter bind user's AD pass code",
        action="store",type="string",dest="bindPassword",default="")
  opts.add_option("-s",
        help="enter authentication server to use",
        action="store",type="string",dest="server",default=None)
  opts.add_option("-S",
        help="forces SSL/TLS",
        action="store_true",dest="ssl_tls",default=None)
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
  # Must have both bind user and password set if using
  if options.bindUser == "":
    BINDUSER = True
    if options.bindPassword != "":
        raise ValueError, "If using a bind user, both username, -U, and password, -P, must be specified" 
  if options.bindUser != "":
    BINDUSER = True
    if options.bindPassword == "":
        raise ValueError, "If using a bind user, both username, -U, and password, -P, must be specified" 
  if options.debug == True:
	authbase.DEBUG = True

def authenticate4(options):
    """
    Authenticate user using simple bind
    SSL security
    """
    
    user = userLdapsInfo(options)

    return(user)

def main():
  try:
    try:
      options = parseOptions();
      checkOptions(options);

      reply = authenticate4(options);
      printReply(reply);
    except authFail:
      printFailure(sys.exc_info())
  except:
    printError(sys.exc_info())


#####################
#
#####################
if __name__ == "__main__":
    main()
