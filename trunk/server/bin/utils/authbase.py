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
authbase.py: Basic utilities for delegated auth scripts.
"""
import xml.dom.ext
import xml.dom.minidom
import subprocess
import pexpect, re
import ldap, ldap.sasl
from sys import exit
import time

class UserData:
  """
  Class UserData holds the relevant information about a user:
    * loginName: The canoncical login name (required)
    * email: The user's email address (required)
    * displayName: The user's display name (optional)
    * firstName: Given name (optional)
    * lastName: Surname (optional)
    * company: Company name (optional)
  Plus a list of groups the user is a member of.
  """
  def __init__(self, loginName, email):
    self.loginName = loginName
    self.email = email
    self.displayName = None
    self.firstName = None
    self.lastName = None
    self.company = None
    self.groups = []

  def asXmlNode(self, doc, nodeName):
    # Canonical login name (required)
    top = doc.createElement(nodeName)
    elem = doc.createElement("loginName")
    elem.appendChild(doc.createTextNode(self.loginName))
    top.appendChild(elem)
    # Email address
    elem = doc.createElement("email")
    elem.appendChild(doc.createTextNode(self.email))
    top.appendChild(elem)
    # Display name for user (optional)
    if(self.displayName != None):
      elem = doc.createElement("displayName")
      elem.appendChild(doc.createTextNode(self.displayName))
      top.appendChild(elem)
    # First name for user (optional)
    if(self.firstName != None):
      elem = doc.createElement("firstName")
      elem.appendChild(doc.createTextNode(self.firstName))
      top.appendChild(elem)
    # Last name for user (optional)
    if(self.lastName != None):
      elem = doc.createElement("lastName")
      elem.appendChild(doc.createTextNode(self.lastName))
      top.appendChild(elem)
    # Company for user (optional)
    if(self.company != None):
      elem = doc.createElement("company")
      elem.appendChild(doc.createTextNode(self.company))
      top.appendChild(elem)
    # Groups(s) (required)
    for group in self.groups:
      elem = doc.createElement("group")
      elem.appendChild(doc.createTextNode(group))
      top.appendChild(elem)
    return top

class authFail(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

class krbServError(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

class ldapServError(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

class connectFail(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

class resolveFail(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

def ldaphex(blob):
  """
  Convert binary blob to ldap hex \\xx\\yy\\zz notation
  Obscure conversion based on code found at
  http://code.activestate.com/recipes/496969/
  """
  return '\\'+'\\'.join([hex(ord(c))[2:].zfill(2) for c in blob])

def upnFromEmail(email):
  """
  Converts an email address like user.name@domain.com into
  UPN format like user.name@DOMAIN.COM
  """
  (username, domain) = email.split("@")
  return username.lower() + "@" + domain.upper()

def processLdapInfo(user, con, baseDN, userDN, values):
   # Fill in the base user info:
   # attribute		   map
   #====================================
   # 'displayName'	=> user name
   # 'givenName'	=> first name
   # 'sn'		=> last name
   # 'mail'		=> email address
   # 'title'		=> job title
   # 'Company'		=> company
   # 'memberOf'	=> group(s) user is a member of

   try:
     user.email = values['mail'][0]
   except KeyError:
     pass

   try:
     user.firstName = values['givenName'][0]
   except KeyError:
     pass

   try:
     user.displayName = values['displayName'][0]
   except KeyError:
     pass

   # Now that we have the DN, bind as that user and ask for tokenGroups
   # print "searching for tokenGroups"
   results = con.search_s( userDN, ldap.SCOPE_BASE, '(objectClass=*)',
                           ['tokenGroups'])
   # print "result: " + str(results)

   if(len(results) == 0):
     raise authFail, "tokenGroups attribute not found"

   if(results[0][0] is None):
     groupIDs = []
   else:
     groupIDs = results[0][1]['tokenGroups'];

   # print "searching for sids"
   query = "".join(['(objectSid=' + ldaphex(id) + ')' for id in groupIDs])
   results = con.search_s(baseDN, ldap.SCOPE_SUBTREE,
                          '(|' + query + ')', ['objectSid', 'cn']);
   # print "result: " + str(results)
   if(len(results) == 0):
     raise authFail, "tokenGroups attribute not found"

   for group in results:
     if(group[0] is not None):
       user.groups.append(group[1]['cn'][0])
   # print "unbinding"
   con.unbind()
   
   return user


def processBindLdapInfo(server, user, userBaseDN, userPasswd, authUser, baseDN, userDN, values, bindPassword, bindUser):
   # Slight alteration on the processLdapInfo function: user
   # a 'bind user' to retrieve user info
   # Fill in the base user info:
   # attribute		   map
   #====================================
   # 'displayName'	=> user name
   # 'givenName'	=> first name
   # 'sn'		=> last name
   # 'mail'		=> email address
   # 'title'		=> job title
   # 'Company'		=> company
   # 'memberOf'	=> group(s) user is a member of

   try:
     user.email = values['mail'][0]
   except KeyError:
     pass

   try:
     user.firstName = values['givenName'][0]
   except KeyError:
     pass

   try:
     user.displayName = values['displayName'][0]
   except KeyError:
     pass

   # Try a connection to the deinfed server
   ldapServer = ("ldaps://%s:3269" % str(server))
   printDebug("Query LDAP for user information.  LDAP server to use: " + str(ldapServer))

   try:
	con = ldap.initialize(ldapServer)
   except ldap.CONNECT_ERROR:
	raise connectFailure, "Connection to server %s failed in bind operation." + server

   # bind user
   try:
	con.simple_bind_s(authUser, userPasswd)
   except ldap.INVALID_CREDENTIALS:
	raise authFail, "Invalid Credentials for user: " + authUser

   printDebug("Connected to the LDAP server with username: "+authUser)
   
   # Now that we have the DN, bind as that user and ask for tokenGroups
   # print "searching for tokenGroups"
   printDebug("Quering LDAP server with base DN: "+userBaseDN)
   #results = con.search_s( '', ldap.SCOPE_BASE, '(objectClass=*)',
   results = con.search_s( userDN, ldap.SCOPE_BASE, '(objectClass=*)',
                           ['tokenGroups'])
   printDebug("TokenGroup query result: "+str(results))
   if(len(results) == 0):
     raise authFail, "tokenGroups attribute not found"

   if(results[0][0] is None):
     groupIDs = []
   else:
     groupIDs = results[0][1]['tokenGroups'];
   
   # Intentially close
   con.unbind()
   printDebug("End tokenGroups lookup phase")

   # connect with the bind user to resolve group SIDs
   printDebug("Phase SID Lookup: Start.")

   # Is it required to be on the GC to resolve SIDs?
   #ldapServer = "ldaps://"+server+":3269"
   printDebug("Phase SID: using server "+ldapServer+" to query")
   try:
	con = ldap.initialize(ldapServer)
   except ldap.CONNECT_ERROR:
	raise connectFailure, "Connection to server %s failed in bind operation." + server
   printDebug("Phase SID lookup: Successfully initialized LDAP connection to "+ldapServer)

   #
   try:
	if BINDUSER:
	  con.simple_bind_s(bindUser, bindPassword)
	else:
	  con.simple_bind_s(authUser, userPasswd)
   except ldap.INVALID_CREDENTIALS:
	raise authFail, "Invalid Credentials for user: " + bindUser
   printDebug("Phase SID lookup: Connected to the LDAP server with username: "+authUser)

   printDebug("Doing group SID -> name lookup")
   printDebug("Phase SID: using userBaseDn "+str(userBaseDN)+".")
   results = []
   if DEBUG == True:
     for id in groupIDs:
        query = '(objectSid=' + ldaphex(id) + ')'
        printDebug("Phase SID lookup: group SID converted query is "+str(query))
        res = (con.search_s(userBaseDN, ldap.SCOPE_SUBTREE, 
		query, ['objectSid', 'cn']))
        if len(res) != 0:
          results.append(res)
          printDebug("Phase SID lookup: SID name is " + str(res))
        else:
          printDebug("Phase SID lookup: SID "+id+" is not resolvable!")
   else:
     query = "".join(['(objectSid=' + ldaphex(id) + ')' for id in groupIDs])
     query = '(|'+query+')'
     printDebug("Phase SID lookup: group SID converted query is "+str(query))
     results = con.search_s(userBaseDN, ldap.SCOPE_SUBTREE, query, ['objectSid', 'cn']);

   printDebug("Group SID to name results: "+ str(results))
   
   if(len(results) == 0):
     raise authFail, "tokenGroups attribute not found"

   if DEBUG == True:
     for group in results:
       if(group[0] is not None):
         user.groups.append(str(group[0][1]['cn']))
   else:
     for group in results:
       if(group[0] is not None):
         user.groups.append(group[1]['cn'][0])

   printDebug("Phase SID lookup: unbinding LDAP connection")
   con.unbind_s()
   
   return user
   
def userLdapInfo(options):
   """
   Retrieve LDAP information for given user
   """

   user = UserData(options.user, options.user)

   # Make the server option compiliant
   ldapServer = ("ldap://%s:389" % options.server)

   # First query: Use UPN to resolve DN
   query = ("(userPrincipalName=%s)" % upnFromEmail(options.user))

   auth = ldap.sasl.gssapi("")
   con = ldap.initialize(ldapServer)
   con.sasl_interactive_bind_s( "", auth )
   # Ask for the user data that we want to provide back
   results = con.search_s( options.base_dn, ldap.SCOPE_SUBTREE, query,
                           ['displayName', 'mail', 'givenName', 'sn'])

   if(len(results) == 0):
     raise authFail, "User not in Active Directory: " + options.user
   userDN = results[0][0];
   values = results[0][1];

   # Now that we have the DN, bind as that user and ask for tokenGroups
   con.sasl_interactive_bind_s( userDN, auth )
   processLdapInfo(user, con, options.base_dn, userDN, values)
   return user


def simpleLdapInfo(options, server):
   """
   Retrieve LDAP information for given user
   simpleLdapInfo assumes that it does a simple bind (no gssapi)
   and takes a single server as argument (no async operation)
   """
   user = UserData(options.user, options.user)
   # print "connecting to " + server
   try:
     con = ldap.initialize("ldap://%s:389" % server)
     con.simple_bind_s(options.user, options.password)
   except ldap.LDAPError, err:
     # print str(err[0])
     raise authFail, "unable to authenticate user: " + (err[0]["desc"])

   # First query: Use UPN to resolve DN
   query = ("(userPrincipalName=%s)" % upnFromEmail(options.user))
   results = con.search_s( options.base_dn, ldap.SCOPE_SUBTREE, query,
                           ['displayName', 'mail', 'givenName', 'sn'])
   if(len(results) == 0):
       raise authFail, "User not in Active Directory: " + options.user
   userDN = results[0][0];
   values = results[0][1];
   # Process ldap info for user
   processLdapInfo(user, con, options.base_dn, userDN, values)
   return user

def waitForAll(reqList, conList, timeout):
  respList = {}
  start = time.time()
  # print reqList
  for server in reqList:
    # allow for timeout to specify negative (no timeout)
    # as well as 'real' timeouts
    if(timeout >= 0):
      remain = timeout - (time.time() - start)
      if(remain <= 0.0): remain = 0
    else: remain = -1
    con = conList[server]
    resp = con.result3(reqList[server], 1, remain)
    # print "Response from " + server + ": " + str(resp)
    respList[server] = resp

  return respList

def simpleLdapInfo2(options, serverList):
   """
   Retrieve LDAP information for given user
   simpleLdapInfo2 assumes that it does a simple bind (no gssapi)
   and takes a server list as argument (concurrent async operation)
   """
   user = UserData(options.user, options.user)
   conList = {}
   reqList = {}
   timeout = -1 # wait indefinitely for now

   # print "connecting to " + server
   for server in serverList:
     try:
       con = ldap.initialize("ldap://%s:389" % server)
       req = con.simple_bind(options.user, options.password)
       conList[server] = con
       reqList[server] = req
     except:
       continue

   try:
     waitForAll(reqList, conList, timeout)
   except ldap.INVALID_CREDENTIALS:
     raise authFail, "Invalid Credentials: " + options.user

   # First query: Use UPN to resolve DN
   query = ("(userPrincipalName=%s)" % upnFromEmail(options.user))
   reqList = {}
   for server in conList:
     con = conList[server]
     req = con.search( options.base_dn, ldap.SCOPE_SUBTREE, query,
                       ['displayName', 'mail', 'givenName', 'sn'])
     reqList[server] = req

   respList = waitForAll(reqList, conList, timeout)
   for server in respList:
     results = respList[server][1]
     if(results is not None and len(results) > 0):
       con = conList[server]
       break

   if(results is None or len(results) == 0):
     raise authFail, "User not in Active Directory: " + options.user
   userDN = results[0][0];
   values = results[0][1];
   # Process ldap info for user
   processLdapInfo(user, con, options.base_dn, userDN, values)
   return user

def resolveUserDC(userDN):
  """
  Return host servicing DC from usersDN
  """
  # Prefix for service record for gc
  dcSrvRecord = '_ldap._tcp'
  # User's base DN as derived from their full DN
  userBaseDN = ''
  # Regular expression to match/subsititue from DN
  expression = re.compile('DC=')
  # Regular expression for return of host command worked
  hasDnsRecord = re.compile('(has SRV)')

  dnString = userDN.split(',')
  for ele in dnString:
    if expression.match(str(ele)):
      dcSrvRecord += "."
      dcSrvRecord += expression.sub('',str(ele))
      userBaseDN += ele + ','
  
  userBaseDN = userBaseDN.rstrip(',')
  printDebug("Derived user's base DN: " + userBaseDN)
  printDebug("Derived domain to query: " + dcSrvRecord)

  resolveDCHost = 'UNKNOWN'
  
  hostLookupFH = subprocess.Popen('host -t SRV ' + dcSrvRecord,stdout=subprocess.PIPE, shell=True)
  resolveDCHost = hostLookupFH.stdout.read()
  
  if hasDnsRecord.search(resolveDCHost):
    dcHost = resolveDCHost.split(' ')
    dcHost = dcHost[-1].rstrip('\n').rstrip('.')
  else:
    raise resolveFail, "DNS reolve failure:  Could not find SRV record for " + resolveDCHost
  
  printDebug("Derived domain controller: " + dcHost)
    
  return dcHost,userBaseDN
  
# end resolveUsersDC

def userLdapsInfo(options):
   """
   Retrieve user ldap info using SSL/TLS using a
   'bind user'.  This assumes that the user for
   which info is being retrieved has already been 
   authenticated.
   """

   user = UserData(options.user, options.user)

   # Try a connection to the deinfed server
   ldapServer = ("ldaps://%s" % options.server)

   try:
	con = ldap.initialize(ldapServer)
   except ldap.CONNECT_ERROR:
	raise connectFailure, "Connection to server %s failed" + options.server

   printDebug("Doing initial authentication.  Connected to server " + ldapServer)

   # First query: Use UPN to resolve DN
   try:
     con.simple_bind_s(options.user,options.password)
   except ldap.INVALID_CREDENTIALS:
	raise authFail, "Unable to authenticate user.  Wrong username or password." + options.user

   filter = ("(userPrincipalName=%s)" % options.user)
   results = con.search_s( options.base_dn, ldap.SCOPE_SUBTREE, filter,
	['displayName', 'mail', 'givenName', 'sn', 'dn'])

   if(len(results) == 0):
       raise authFail, "User authenticated, however, could not retrieve user information from the AD server: " + options.server

   userDN = str(results[0][0])
   values = results[0][1]

   printDebug("LDAP returned user's DN: " + userDN)

   # This should be closed now
   con.unbind_s()

   # Get the users correct domain controller to query for tokengroups
   userDCServer, userBaseDN = resolveUserDC(userDN)

   # Process ldap info for user
   processBindLdapInfo(userDCServer, user, userBaseDN, options.password, options.user, options.base_dn, userDN, values, options.bindUser, options.bindPassword)

   return user

# end userLdapsInfo

def waitForAll(reqList, conList, timeout):
  respList = {}
  start = time.time()
  # print reqList
  for server in reqList:
    # allow for timeout to specify negative (no timeout)
    # as well as 'real' timeouts
    if(timeout >= 0):
      remain = timeout - (time.time() - start)
      if(remain <= 0.0): remain = 0
    else: remain = -1
    con = conList[server]
    resp = con.result3(reqList[server], 1, remain)
    # print "Response from " + server + ": " + str(resp)
    respList[server] = resp
# end simpleSecureLdapInfo

   
def authSimpleSSL(options):
   """
   Authenticate the user with a simple secure bind
   """
   # Force SSL/TLS
   ldap.set_option(ldap.OPT_X_TLS_DEMAND,1)

   # Try a connection to the deinfed server
   ldapServer = ("ldaps://%s" % options.server)
   try:
	con = ldap.initialize(ldapServer)
   except ldap.CONNECT_ERROR:
	raise connectFailure, "Connection to server %s failed" + options.server

   # Try doing a simple bind as a form of "authentication"
   try:
	con.simple_bind_s(options.user, options.userPasswd)
   except ldap.INVALID_CREDENTIALS:
	raise authFail, "Invalid Credentials: " + options.user

   # This should be closed
   con.unbind_s()
# end authSimpleSSL
   
def kdestroy(options):
  """
  Destroy user kerberos credentials
  """

  if(options.cert != None):
    kdestroyCmd = ( '/usr/kerberos/bin/kdestroy -c %s' % options.cert )
  else:
    kdestroyCmd = ('/usr/kerberos/bin/kdestroy')

  res = subprocess.call(kdestroyCmd)

def kinit(options):
  """
  Initialize user kerberos ticket
  """
  upn = upnFromEmail(options.user)
  if(options.cert != None):
    kinit_cmd = ('/usr/kerberos/bin/kinit -c %s %s' % (options.cert,upn))
  else:
    kinit_cmd = ('/usr/kerberos/bin/kinit %s' % upn)

  servErr = ''
  try:
    servCon = pexpect.spawn(kinit_cmd)
    servCon.timeout=1
    resp = servCon.expect(['Password for ', pexpect.EOF])
    if(resp == 0):
      servCon.sendline(options.password)
      servCon.expect(pexpect.EOF)
  except:
    servErr = servCon.before
    servCon.close()
    raise krbServError, servErr

  servErr = servCon.before
  servCon.close()

  #print("servErr: %s" % servErr)
  if(servErr != ''):
    if(re.search('Preauthentication failed', servErr)):
      raise authFail, servErr
    # We get the following error if the user is not in AD
    if(re.search('Client not found in Kerberos database', servErr)):
      raise authFail, servErr
    # We get this error when the domain is not in AD
    if(re.search('Cannot find KDC for requested realm', servErr)):
      raise authFail, servErr
    if(re.search('Resource temporarily unavailable',servErr)):
      servErr = "kerberos server unavailable."
      raise krbServError, servErr
    if(not re.search(upn, servErr)):
      raise krbServError, servErr
     
def printListing(userlist):
  """
  Prints a list of users from the given data set.
  """
  doc = xml.dom.minidom.Document()
  top = doc.createElement("ok")
  doc.appendChild(top)
  for user in userlist:
    top.appendChild(user.asXmlNode(doc, "user"))
  xml.dom.ext.PrettyPrint(doc)

def printReply(user):
  """
  Prints the reply from the user data set after
  successful authentication has taken place.
  """
  doc = xml.dom.minidom.Document()
  doc.appendChild(user.asXmlNode(doc, "ok"))
  xml.dom.ext.PrettyPrint(doc)

def printError(sysInfo):
  """
  Prints the error response if anything went wrong in the script.
  """
  errType = str(sysInfo[0])
  errString = str(sysInfo[1])
  doc = xml.dom.minidom.Document()
  top = doc.createElement("error")
  top.appendChild(doc.createTextNode(str(sysInfo[1])))
  doc.appendChild(top)
  xml.dom.ext.PrettyPrint(doc)

def printDebug(debugInfo):
  """
  Print debugging information
  """
  if DEBUG == True:
    doc = xml.dom.minidom.Document()
    top = doc.createElement("debug")
    top.appendChild(doc.createTextNode(str(debugInfo)))
    doc.appendChild(top)
    xml.dom.ext.PrettyPrint(doc)

def printFailure(sysInfo):
  """
  Prints the error response if anything went wrong in the script.
  """
  errType = str(sysInfo[0])
  errString = str(sysInfo[1])
  doc = xml.dom.minidom.Document()
  top = doc.createElement("failed")
  top.appendChild(doc.createTextNode(str(sysInfo[1])))
  doc.appendChild(top)
  xml.dom.ext.PrettyPrint(doc)
