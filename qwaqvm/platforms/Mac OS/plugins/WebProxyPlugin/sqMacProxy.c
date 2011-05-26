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

/*
 * sqMacProxy.c: Mac proxy functions.
 */
#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>
#include <SystemConfiguration/SCSchemaDefinitions.h>
#include "WebProxyPlugin.h"
#include "sqMemoryAccess.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h> 

/* staticAutoProxyUrl: Answer the static URL for the PAC file to use.
   Arguments: None.
   Returns: Url for PAC file if any.
   Notes: If configured, the platform should answer the URL for proxy
   auto configuration (PAC) files. This method is assumed to be fast
   and not to DHCP and/or DNS discovery.
*/
char* staticAutoProxyUrl(void) {
	static char pacUrl[1024];
	CFDictionaryRef proxyDict;
	CFStringRef pacRef;

	proxyDict = SCDynamicStoreCopyProxies(NULL);
	if(!proxyDict) return NULL;  /* something went wrong */
	/* Get the configured PAC file url */
	pacRef = (CFStringRef) CFDictionaryGetValue(proxyDict, kSCPropNetProxiesProxyAutoConfigURLString);
	/* Extract the C-string in UTF-8 */
	if(!pacRef || !CFStringGetCString(pacRef, pacUrl, sizeof(pacUrl), kCFStringEncodingUTF8)) {
		pacUrl[0] = 0;
	}
	CFRelease(proxyDict);
	return pacUrl;
}

/* dynamicAutoProxyUrl: Answer the dynamic URL for the PAC file to use.
   Arguments: None.
   Returns: Url for PAC file if any.
   Notes: If supported, this method performs DHCP/DNS discovery for
   finding the url for proxy auto configuration (PAC) files. It will
   only be called as a last resort and may take some time (on Windows
   it can take upwards of five seconds to complete).
*/
char* dynamicAutoProxyUrl(void) {
	/* there is no support by MacOS to perform discovery so we simply fail */
	return NULL;
}

/* defaultProxyServer: Answer the default proxy server(s) for this box.
   Arguments: None.
   Returns: The default proxy server(s).
   Notes: If configured, this method answers the proxy server(s) that
   have been statically assigned to this box.
*/
char* defaultProxyServer(void) {
	static char result[1024];
	char proxyHost[1024];
	int  port;

	CFDictionaryRef proxyDict;
	CFStringRef hostRef;
	CFNumberRef portRef;
	
	/* The list of entries that we concat together when listing proxies */
	const void **nextKey;

	/* This is the set of keys that we use to report proxies back */
	const void *proxyKeys[] = {
		kSCPropNetProxiesHTTPProxy,		kSCPropNetProxiesHTTPPort,
		kSCPropNetProxiesHTTPSProxy,	kSCPropNetProxiesHTTPSPort,
		NULL
	};

	proxyDict = SCDynamicStoreCopyProxies(NULL);
	if(!proxyDict) return NULL;  /* something went wrong */

	/* Enumerate over all key pairs to construct the proxy string */
	result[0] = 0;
	nextKey = proxyKeys;
	while(*nextKey) {
		/* First key of a pair holds proxy host */
		hostRef = (CFStringRef) CFDictionaryGetValue(proxyDict, *nextKey);
		/* Get the C-string leaving space at the end for :port if needed */
		if(!hostRef || !CFStringGetCString(hostRef, proxyHost, sizeof(proxyHost)-10, kCFStringEncodingUTF8)) {
			proxyHost[0] = 0;
		}
		/* Second value of a pair holds port key */
		nextKey++;
		if(proxyHost[0]) { /* only extract port if we had a host */
			portRef = (CFNumberRef) CFDictionaryGetValue(proxyDict, *nextKey);
			if(portRef && CFNumberGetValue(portRef, kCFNumberIntType, &port)) {
				/* we left room for the port above so simply concat it */
				char *strEnd = proxyHost + strlen(proxyHost);
				sprintf(strEnd, ":%u", port);
			}
		}
		/* append proxy string to the result if we have enough space left */
		if(proxyHost[0] && strlen(result) + strlen(proxyHost) + 2 < sizeof(result)) {
			if(result[0]) strcat(result, ";");
			strcat(result, proxyHost);
		}
		/* advance past port key */
		nextKey++;
	}

	CFRelease(proxyDict);
	return result;
}

/* webProxyForUrl: Answer proxy information for the given url.
   Arguments:
     urlString: The URL for which the proxy server is requested.
     pacLoc: The URL for the PAC to use (if any).
   Return value: List of proxy servers (if any).
*/
char *proxyServerForUrl(char *urlString, char *pacLoc) {
	/* the Mac doesn't have an easy way to do that right now */
	return NULL;
}
