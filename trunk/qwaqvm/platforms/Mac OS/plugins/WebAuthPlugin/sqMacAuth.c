/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-20011, Teleplace, Inc., All Rights Reserved
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

#include "sqVirtualMachine.h"
#include "sqConfig.h"
#include "sqPlatformSpecific.h"

#include "WebAuthPlugin.h"

/* These are  undocumented (and possibly unsupported) APIs. But they work. */
#include "NtlmGenerator.h"
#include "spnegoBlob.h"

const int debug = 1;

typedef struct {
	NtlmGeneratorRef ntlm;
	CFDataRef token;
	CFStringRef username;
	CFStringRef password;
	CFStringRef domain;
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

int sqAuthInitiate(char *method, char *username, char *password, char *domain, char *principal){
	sqAuthData *auth;
	OSStatus result;

	if(debug) {
		printf("sqAuthInitiate: method=%s, user=%s, pass=%s, domain=%s\n",
				method, username, password, domain);
	}

	auth = calloc(1, sizeof(sqAuthData));
	/* Fixme: Retrieve defaults from key-chain */
	if(username == NULL) username = "";
	if(password == NULL) password = "";
	if(domain == NULL) domain = "";
	if(principal == NULL) principal = "";

	auth->username = CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*)username, strlen(username), kCFStringEncodingUTF8, 0);
	auth->password = CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*)password, strlen(password), kCFStringEncodingUTF8, 0);
	auth->domain = CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*)domain, strlen(domain), kCFStringEncodingUTF8, 0);
	if(strcasecmp(method, "NTLM") == 0) {
		/* Create the NTLM generator */
		result = NtlmGeneratorCreate(NW_Any, &auth->ntlm);
		if(result != noErr) goto failed;
		/* and the client request */
		result = NtlmCreateClientRequest(auth->ntlm, &auth->token);
		if(result != noErr) goto failed;
	} else if(strcasecmp(method, "Negotiate") == 0) {
		/* This is currently a no-go. Apple's CFNetwork.framework
		   does not expose the spnego functions publicly, so we
		   cannot link aginst the undocumented API. */
		result = noErr;
		goto failed;
#if 0
		char *blob;
		uint len;
		/* this is a hack. break the principal apart 
		   so that the api can reassemble it */
		char *svcType = principal;
		char *svcName = strchr(svcType, '/');
		if(svcName) {
			*svcName = 0;
			svcName++;
			result = spnegoTokenInitFromPrincipal(svcName, svcType, &blob, &len);
			if(result != 0) goto failed;
			auth->token = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)blob, len, kCFAllocatorNull);
		}
#endif
	} else {
		printf("sqAuthInitiate: Unsupported authentication method \"%s\"", method);
		result = noErr;
		goto failed;
	}

	/* XXXX: fixme. return a real handle */
	return (int)auth;

failed:
	printf("sqAuthInitiate failed: %d\n", (int)result);
	if(auth) {
		if(auth->ntlm) NtlmGeneratorRelease(auth->ntlm);
		if(auth->username) CFRelease(auth->username);
		if(auth->password) CFRelease(auth->password);
		if(auth->domain) CFRelease(auth->domain);
		if(auth->token) CFRelease(auth->token);
		free(auth);
	}
	return 0;
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
	CFDataRef dataRef;
	OSStatus result;

	if(auth == NULL) return -1;

	if(debug) {
		printf("sqAuthContinue: handle = %x, tokensize = %d\n", handle, tokensize);
	}

	/* Release the previous token */
	CFRelease(auth->token);

	dataRef = CFDataCreate(kCFAllocatorDefault, (UInt8*)token, tokensize);
	result = NtlmCreateClientResponse(auth->ntlm, dataRef, auth->domain, auth->username, auth->password, &auth->token);
	CFRelease(dataRef);

	if(result == noErr) return 1;
	return result > 0 ? -result : result;
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

	if(auth == NULL) return NULL;
	*sizeptr = CFDataGetLength(auth->token);
	return (char*)CFDataGetBytePtr(auth->token);
}

/* sqAuthDestroy: Destroys the authentication session.
   Arguments:
     handle - the authentication handle
   Returns: Zero if successful, an error code otherwise.
*/
int sqAuthDestroy(int handle) {
	sqAuthData *auth = (sqAuthData*)handle;

	if(auth == NULL) return -1;
	if(debug) {
		printf("sqAuthDestroy: handle = %x\n", handle);
	}
	if(auth->ntlm) {
		NtlmGeneratorRelease(auth->ntlm);
	}
	CFRelease(auth->username);
	CFRelease(auth->password);
	CFRelease(auth->domain);
	CFRelease(auth->token);
	free(auth);
	return 0;
}
