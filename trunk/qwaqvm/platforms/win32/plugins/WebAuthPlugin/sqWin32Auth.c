/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-2011, Teleplace, Inc., All Rights Reserved
 *
 * Redistributions in source code form must reproduce the above
 * copyright and this condition.
 *
 * The contents of this file are subject to the GNU General Public
 * License, Version 2 (the "License"); you may not use this file
 * except in compliance with the License. A copy of the License is
 * available at http://www.opensource.org/licenses/gpl-2.0.php.
 *
 */

#include <windows.h>
#include <errno.h> /* For cygwin gcc 3.4 */
#include "sq.h"
#include "WebAuthPlugin.h"

#define SECURITY_WIN32
#include <security.h>

const int debug = 0;

typedef struct {
  char *principal;
  TimeStamp expiry;
  CredHandle cred;
  CtxtHandle ctxt;
  SecBufferDesc sbdIn;
  SecBuffer inbuf[4];
  SecBufferDesc sbdOut;
  SecBuffer outbuf[4];
} sqAuthData;

/* sqAuthInitialize: Initialize the auth plugin.
   Arguments: None.
   Returns: Non-zero if successful.
*/
int sqAuthInitialize(void) {
  return 1;
}

/* sqAuthShutdown: Shut down the auth plugin.
   Arguments: None.
   Returns: Non-zero if successful.
*/
int sqAuthShutdown(void) {
  return 1;
}

/* sqAuthInitiate: Initiate the authentication session.
   Arguments:
     method - the authentication method to use (NTLM, Negotiate, etc)
     username - the user name to use (if NULL, use default)
     password - the password token to use (if NULL, use default)
     domain - the domain to use for auth (if NULL, use default)
	 principal - the Kerberos service principal name
   Returns:
     An authentication session handle, or zero on error.
*/

#define DEFAULT_AUTH_FLAGS ISC_REQ_ALLOCATE_MEMORY

int sqAuthInitiate(char *method, char *username, char *password, char *domain, char *principal){
  SEC_WINNT_AUTH_IDENTITY credentials;
  SECURITY_STATUS ret;
  ULONG flags = DEFAULT_AUTH_FLAGS;
  ULONG attrs = 0;
  sqAuthData *auth;


  if(debug) {
    printf("sqAuthInitiate: method=%s, user=%s, pass=%s, domain=%s\n",
	   method, username, password, domain);
  }

  auth = calloc(1, sizeof(sqAuthData));
  if(principal) auth->principal = strdup(principal);
  auth->sbdIn.ulVersion = SECBUFFER_VERSION;
  auth->sbdIn.pBuffers = auth->inbuf;
  auth->sbdOut.ulVersion = SECBUFFER_VERSION;
  auth->sbdOut.pBuffers = auth->outbuf;

  /* initialize additional credentials */
  memset(&credentials,0,sizeof(SEC_WINNT_AUTH_IDENTITY));

  // XXXX: fixme. use unicode and utf8 conversion
  credentials.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
  if(username) {
    credentials.User = username;
    credentials.UserLength = strlen(username);
  }
  if(password) {
    credentials.Password = password;
    credentials.PasswordLength = strlen(password);
  }
  if(domain) {
    credentials.Domain = domain;
    credentials.DomainLength = strlen(domain);
  }

  ret = AcquireCredentialsHandle(NULL, method, SECPKG_CRED_OUTBOUND, NULL,
								 username ? &credentials : NULL,
                                 NULL, NULL, &auth->cred, &auth->expiry);

  if (ret != SEC_E_OK) {
    if(debug) printf("AquireCredentialsHandle error: %x\n", ret);
    return 0;
  }

  auth->outbuf[0].BufferType = SECBUFFER_TOKEN;
  auth->outbuf[0].cbBuffer = 0;
  auth->outbuf[0].pvBuffer = NULL;
  auth->outbuf[1].BufferType = SECBUFFER_EMPTY;
  auth->outbuf[1].cbBuffer = 0;
  auth->outbuf[1].pvBuffer = NULL;
  auth->sbdOut.cBuffers = 2;

  ret = InitializeSecurityContext(&auth->cred, 
				  NULL, 
				  auth->principal,
				  flags, 
				  0, 
				  SECURITY_NETWORK_DREP,
				  NULL,
				  0,
				  &auth->ctxt,
				  &auth->sbdOut,
				  &attrs,
				  NULL);

  /* The only expected return value here is SEC_I_CONTINUE_NEEDED
     since we're just trying to establish the auth session */
  if(ret != SEC_I_CONTINUE_NEEDED) {
    if(debug) printf("InitializeSecurityContext returned: %x\n", ret);
    return 0;
  }
  /* XXXX: fixme. return a real handle */
  return (int)auth;

}

/* sqAuthContinue: Submit a token to the authentication session.
   Arguments:
     handle - the authentication session handle
     token - the pointer to the token data
     tokensize - the length of the token data.
   Returns:
     An integer status code.
     <0 - Error. The error code is platform specific.
      0 - OK. The session is authenticated.
      1 - Another server-roundtrip is needed.
*/
int sqAuthContinue(int handle, char *token, int tokensize) {
  sqAuthData *auth = (sqAuthData*)handle;
  SECURITY_STATUS ret;
  ULONG flags = DEFAULT_AUTH_FLAGS;
  ULONG attrs = 0;

  if(auth == NULL) return -1;

  auth->inbuf[0].BufferType = SECBUFFER_TOKEN;
  auth->inbuf[0].cbBuffer = tokensize;
  auth->inbuf[0].pvBuffer = token;
  auth->inbuf[1].BufferType = SECBUFFER_EMPTY;
  auth->inbuf[1].cbBuffer = 0;
  auth->inbuf[1].pvBuffer = NULL;
  auth->sbdIn.cBuffers = 2;

  ret = InitializeSecurityContext(&auth->cred,
				  &auth->ctxt,
				  auth->principal,
				  flags, 
				  0,
				  SECURITY_NETWORK_DREP, 
				  &auth->sbdIn, 
				  0,
				  NULL,
				  &auth->sbdOut,
				  &attrs, 
				  NULL);


  if (FAILED(ret)) {
    if(debug) printf("InitializeSecurityContext failed: %x\n", ret);
    return -(ret & 0x7FFFFFFF);
  }

  if(debug) printf("InitializeSecurityContext returned: %x\n", ret);

  /* Complete token if needed */
  if(ret == SEC_I_COMPLETE_NEEDED || ret == SEC_I_COMPLETE_AND_CONTINUE)  {
    SECURITY_STATUS ss;
    ss = CompleteAuthToken (&auth->ctxt, &auth->sbdOut);
    if(FAILED(ss)) {
      if(debug) printf("CompleteAuthToken failed: %x\n", ss);
      return ss;
    }
   }

  if(ret == SEC_E_OK || ret == SEC_I_COMPLETE_NEEDED) {
    /* success; session is established */
    return 0;
  }

  if(ret == SEC_I_CONTINUE_NEEDED || ret == SEC_I_COMPLETE_AND_CONTINUE) {
    /* need another round trip */
    return 1;
  }
  return -ret;
}

/* sqAuthToken: Retrieve the next token to be sent to the remote.
   Arguments:
     handle - the authentication session handle.
     tokensize - a pointer obtaining the size of the token
   Returns:
     A pointer to the authentication token.
*/
char* sqAuthToken(int handle, int *sizeptr) {
  sqAuthData *auth = (sqAuthData*)handle;
  static char *token = NULL;
  static int maxtoken = 0;
  SecBufferDesc *sbdOut = &auth->sbdOut;
  int bufsize, i;

  if(auth == NULL) return -1;

  /* Find and return the token to be sent to the server */
  bufsize = 0;
  for(i=0; i<sbdOut->cBuffers; i++) {
    SecBuffer buffer = sbdOut->pBuffers[i];
    if(debug) printf("Buffer: %d  (length: %d)\n", buffer.BufferType, buffer.cbBuffer);
    if(buffer.BufferType == SECBUFFER_TOKEN) {
      int newsize = bufsize + buffer.cbBuffer;
      if(newsize > maxtoken) {
	if(token) token = realloc(token, newsize);
	else token = malloc(newsize);
	maxtoken = newsize;
      }
      memcpy(token+bufsize, buffer.pvBuffer, buffer.cbBuffer);
      bufsize = newsize;
    }
  }
  *sizeptr = bufsize;
  return token;
}

/* sqAuthDestroy: Destroys the authentication session.
   Arguments:
     handle - the authentication handle
   Returns: Zero if successful, an error code otherwise.
*/
int sqAuthDestroy(int handle) {
  sqAuthData *auth = (sqAuthData*)handle;

  if(auth == NULL) return -1;

  DeleteSecurityContext(&auth->ctxt);
  FreeCredentialsHandle(&auth->cred);
  free(auth);
  return 0;
}

