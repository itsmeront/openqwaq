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

/* WebProxyPlugin.h: Header file for web proxy support. */

/* staticAutoProxyUrl: Answer the static URL for the PAC file to use.
   Arguments: None.
   Returns: Url for PAC file if any.
   Notes: If configured, the platform should answer the URL for proxy
   auto configuration (PAC) files. This method is assumed to be fast
   and not to DHCP and/or DNS discovery.
*/
char* staticAutoProxyUrl(void);

/* dynamicAutoProxyUrl: Answer the dynamic URL for the PAC file to use.
   Arguments: None.
   Returns: Url for PAC file if any.
   Notes: If supported, this method performs DHCP/DNS discovery for
   finding the url for proxy auto configuration (PAC) files. It will
   only be called as a last resort and may take some time (on Windows
   it can take upwards of five seconds to complete).
*/
char* dynamicAutoProxyUrl(void);

/* defaultProxyServer: Answer the default proxy server(s) for this box.
   Arguments: None.
   Returns: The default proxy server(s).
   Notes: If configured, this method answers the proxy server(s) that
   have been statically assigned to this box.
*/
char* defaultProxyServer(void);

/* webProxyForUrl: Answer proxy information for the given url.
   Arguments:
     urlString: The URL for which the proxy server is requested.
     pacLoc: The URL for the PAC to use (if any).
   Return value: List of proxy servers (if any).
*/
char *proxyServerForUrl(char *urlString, char *pacLoc);

