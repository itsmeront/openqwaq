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

/* WebAuthPlugin header file */

/* sqAuthInitialize: Initialize the auth plugin.
   Arguments: None.
   Returns: Non-zero if successful.
*/
int sqAuthIntialize(void);

/* sqAuthShutdown: Shut down the auth plugin.
   Arguments: None.
   Returns: Non-zero if successful.
*/
int sqAuthShutdown(void);

/* sqAuthInitiate: Initiate the authentication session.
   Arguments:
     method - the authentication method to use (NTLM, Negotiate, etc)
     username - the user name to use (if NULL, use default)
     password - the password token to use (if NULL, use default)
     domain - the domain to use for auth (if NULL, use default)
	 principal - the Kerberos service principal name (required for Kerberos)
   Returns:
     An authentication session handle, or zero on error.
*/
int sqAuthInitiate(char *method, char *username, char *password, char *domain, char *principal);

/* sqAuthToken: Retrieve the next token to be sent to the remote.
   Arguments:
     handle - the authentication session handle.
     tokensize - a pointer obtaining the size of the token
   Returns:
     A pointer to the authentication token.
*/
char* sqAuthToken(int handle, int *tokensize);

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
int sqAuthContinue(int handle, char *token, int tokensize);

/* sqAuthDestroy: Destroys the authentication session.
   Arguments:
     handle - the authentication handle
   Returns: Zero if successful, an error code otherwise.
*/
int sqAuthDestroy(int handle);
