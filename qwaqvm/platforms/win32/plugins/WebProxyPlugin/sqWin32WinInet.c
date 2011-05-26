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

#if 0
/* sqWin32WinInet.c: Windows proxy configuration */
#include <malloc.h>
#include <windows.h>
#include <wininet.h>
#include "WebProxyPlugin.h"

/**************************************************************************/
/* Additional prototypes for lack of accurate mingw headers               */
/**************************************************************************/
#include <wininet.h> /* at least we got a header file */

#define PROXY_AUTO_DETECT_TYPE_DHCP 1
#define PROXY_AUTO_DETECT_TYPE_DNS_A 2

#define INTERNET_PER_CONN_FLAGS 1
#define INTERNET_PER_CONN_PROXY_SERVER 2
#define INTERNET_PER_CONN_PROXY_BYPASS 3
#define INTERNET_PER_CONN_AUTOCONFIG_URL 4
#define INTERNET_PER_CONN_AUTODISCOVERY_FLAGS 5

#define INTERNET_OPTION_PER_CONNECTION_OPTION 75

#define PROXY_TYPE_DIRECT 0x00000001
#define PROXY_TYPE_PROXY 0x00000002
#define PROXY_TYPE_AUTO_PROXY_URL 0x00000004
#define PROXY_TYPE_AUTO_DETECT 0x00000008

typedef struct {
  DWORD dwStructSize;
  LPSTR lpszScriptBuffer;
  DWORD dwScriptBufferSize;
} AUTO_PROXY_SCRIPT_BUFFER, *LPAUTO_PROXY_SCRIPT_BUFFER;

typedef struct {
  DWORD dwOption;
  union {
    DWORD dwValue;
    LPTSTR pszValue;
    FILETIME ftValue;
  } Value;
} INTERNET_PER_CONN_OPTION, *LPINTERNET_PER_CONN_OPTION;

typedef struct {
  DWORD dwSize;
  LPTSTR pszConnection;
  DWORD dwOptionCount;
  DWORD dwOptionError;
  LPINTERNET_PER_CONN_OPTION pOptions;
} INTERNET_PER_CONN_OPTION_LIST, *LPINTERNET_PER_CONN_OPTION_LIST;

typedef struct {
  BOOL  (* IsResolvable) (LPSTR lpszHost);
  DWORD (* GetIPAddress) (LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize);
  DWORD (* ResolveHostName) (LPSTR lpszHostName, LPSTR lpszIPAddress,
			     LPDWORD lpdwIPAddressSize);
  BOOL  (* IsInNet) (LPSTR lpszIPAddress, LPSTR lpszDest, LPSTR lpszMask);
  BOOL  (* IsResolvableEx) (LPSTR lpszHost);
  DWORD (* GetIPAddressEx) (LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize);
  DWORD (* ResolveHostNameEx) (LPSTR lpszHostName, LPSTR lpszIPAddress,
			       LPDWORD lpdwIPAddressSize);
  BOOL  (* IsInNetEx)  (LPSTR lpszIPAddress, LPSTR lpszIPPrefix);
  DWORD (* SortIpList) (LPSTR lpszIPAddressList, LPSTR lpszIPSortedList,
			LPDWORD lpszdwIPSortedListSize);
} AutoProxyHelperVtbl;

typedef struct {
  const struct AutoProxyHelperVtbl * lpVtbl;
} AutoProxyHelperFunctions;

typedef BOOL (CALLBACK *pfnInternetInitializeAutoProxyDll)(
    DWORD dwVersion,
    LPSTR lpszDownloadedTempFile,
    LPSTR lpszMime,
    AutoProxyHelperFunctions* lpAutoProxyCallbacks,
    LPAUTO_PROXY_SCRIPT_BUFFER lpAutoProxyScriptBuffer
);

typedef BOOL (CALLBACK *pfnInternetGetProxyInfo)(
  LPCSTR lpszUrl,
  DWORD dwUrlLength,
  LPSTR lpszUrlHostName,
  DWORD dwUrlHostNameLength,
  LPSTR* lplpszProxyHostName,
  LPDWORD lpdwProxyHostNameLength
);

typedef BOOL (CALLBACK *pfnInternetDeInitializeAutoProxyDll)(
  LPSTR lpszMime,
  DWORD dwReserved
);

typedef BOOL(CALLBACK *pfnDetectAutoProxyUrl)(
  LPSTR lpszAutoProxyUrl,
  DWORD dwAutoProxyUrlLength,
  DWORD dwDetectFlags
);

typedef HRESULT (CALLBACK *pfnURLDownloadToFile)(
    LPVOID pCaller,
    LPCTSTR szURL,
    LPCTSTR szFileName,
    DWORD dwReserved,
    LPVOID lpfnCB
);

typedef BOOL (CALLBACK *pfnInternetQueryOption)(
  HINTERNET hInternet,
  DWORD dwOption,
  LPVOID lpBuffer,
  LPDWORD lpdwBufferLength
);


/* We look up all of these dynamically since we don't have the import libs */
HMODULE   hJSProxyDLL = NULL;
HMODULE   hWininetDLL = NULL;
HMODULE   hUrlmonDLL  = NULL;
HINTERNET hInternet   = NULL;

pfnInternetInitializeAutoProxyDll   pInternetInitAutoProxyDll;
pfnInternetDeInitializeAutoProxyDll pInternetDeInitAutoProxyDll;
pfnInternetGetProxyInfo             pInternetGetProxyInfo;
pfnDetectAutoProxyUrl               pDetectAutoProxyUrl;
pfnURLDownloadToFile                pURLDownloadToFile;

/* IE proxy settings */
DWORD ieProxyFlags = 0;
char *ieAutoproxyUrl = NULL;
char *ieProxyServer = NULL;
char *ieProxyBypass = NULL;

/* winInetInit: Initialize everything we need from WinInet */
int winInetInit(void) {

  if(!hJSProxyDLL) {
    hJSProxyDLL = LoadLibrary( "jsproxy.dll" );
    hWininetDLL = LoadLibrary( "wininet.dll" );
    hUrlmonDLL  = LoadLibrary( "urlmon.dll" );

    pInternetInitAutoProxyDll =(pfnInternetInitializeAutoProxyDll)
      GetProcAddress( hJSProxyDLL, "InternetInitializeAutoProxyDll" );
    pInternetDeInitAutoProxyDll = (pfnInternetDeInitializeAutoProxyDll)
      GetProcAddress(hJSProxyDLL, "InternetDeInitializeAutoProxyDll" );
    pInternetGetProxyInfo = (pfnInternetGetProxyInfo)
      GetProcAddress( hJSProxyDLL, "InternetGetProxyInfo" );

    pDetectAutoProxyUrl = (pfnDetectAutoProxyUrl)
      GetProcAddress( hWininetDLL, "DetectAutoProxyUrl" );
    pURLDownloadToFile = (pfnURLDownloadToFile)
      GetProcAddress( hUrlmonDLL, "URLDownloadToFileA" );

    if(!pInternetInitAutoProxyDll || !pInternetDeInitAutoProxyDll ||
       !pInternetGetProxyInfo || !pDetectAutoProxyUrl || !pURLDownloadToFile){
      printf("Failed to lookup required functions\n");
      return 0;
    }
  }

  if(!hInternet) {
    hInternet = InternetOpen("Qwaq Forums", INTERNET_OPEN_TYPE_PRECONFIG,
			     NULL, NULL, 0);
    if(!hInternet) {
      printLastError("InternetOpen failed");
      return 0;
    }
  }

  if(!ieProxyFlags) {
    /* Detect IEs proxy options */
    INTERNET_PER_CONN_OPTION_LIST List;
    INTERNET_PER_CONN_OPTION Option[5];
    unsigned long nSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);

    Option[0].dwOption = INTERNET_PER_CONN_FLAGS; 
    Option[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    Option[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    Option[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
    Option[4].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;

    List.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
    List.pszConnection = NULL;
    List.dwOptionCount = 5;
    List.dwOptionError = 0;
    List.pOptions = Option;

    if(InternetQueryOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, 
			    &List, &nSize)) {
      ieProxyFlags   = Option[0].Value.dwValue;
      ieProxyServer  = Option[1].Value.pszValue;
      ieProxyBypass  = Option[2].Value.pszValue;
      ieAutoproxyUrl = Option[3].Value.pszValue;
      printf("WinInet IE configuration:\n");
      printf("IE proxy server: %s\n", ieProxyServer);
      printf("IE proxy bypass: %s\n", ieProxyBypass);
      printf("IE autoproxy url: %s\n", ieAutoproxyUrl);
      printf("IE proxy flags: %d (", ieProxyFlags);
      if(ieProxyFlags & PROXY_TYPE_DIRECT) printf("no proxy");
      if(ieProxyFlags & PROXY_TYPE_PROXY) printf("named proxy");
      if(ieProxyFlags & PROXY_TYPE_AUTO_PROXY_URL) printf("auto proxy url");
      if(ieProxyFlags & PROXY_TYPE_AUTO_DETECT) printf("auto proxy detect");
      printf(")\n");
    }
  }
  return 1;
}

/**************************************************************************/
/* WinInet versions of the proxy functions                                */
/**************************************************************************/

/* winInetDetectIEAutoProxyUrl: Detect IEs autoproxy config url. */
char *winInetDetectAutoProxyUrl(void) {
  static char pacLoc[1024];

  if(!winInetInit()) return NULL;

  if(pDetectAutoProxyUrl(pacLoc, sizeof(pacLoc), 
			 PROXY_AUTO_DETECT_TYPE_DHCP | 
			 PROXY_AUTO_DETECT_TYPE_DNS_A ) == FALSE) {
    printLastError("DetectAutoProxyUrl failed");
    return NULL;
  }
  return pacLoc;
}

char *winInetProxyServerForUrl(char *url, char* pacLoc) {
  static char proxyBuf[1024];
  char *proxyInfo = proxyBuf;
  DWORD proxyLength = sizeof(proxyBuf);
  char pacLoc[1024];
  char tmpPath[MAX_PATH], tmpFile[MAX_PATH];
  char hostname[1024];
  char *src, *dst;
  HRESULT hRes;

  if(!winInetInit()) return NULL;

  /* extract host name from url */
  src = url; dst = hostname;
  while(*src && *src++ != ':');
  if(!*src) {
    printf("Couldn't extract hostname from %s (no colon found)\n", url);
    return NULL; /* couldn't extract hostname */
  }
  while(*src && *src == '/') src++;
  if(!*src) {
    printf("Couldn't extract hostname from %s (no slash found)\n", url);
    return NULL; /* couldn't extract hostname */
  }
  while(*src && *src != '/') *dst++ = *src++;
  *dst = 0;

  printf("Proxy server:\nURL: %s\nHOST: %s\n", url, hostname);

  /* Detect the autoproxy-url by looking at DHCP and DNS-A records */
  printf("Looking for PAC file...\n");
  if(pDetectAutoProxyUrl(pacLoc, sizeof(pacLoc), 
			 PROXY_AUTO_DETECT_TYPE_DHCP | 
			 PROXY_AUTO_DETECT_TYPE_DNS_A ) == FALSE) {
    printLastError("DetectAutoProxyUrl failed");
    return NULL;
  }

  printf("PAC file: %s\n", pacLoc);

  /* download the PAC file from url */
  GetTempPath(sizeof(tmpPath), tmpPath);
  printf("TMP path: %s\n", tmpPath);
  GetTempFileName(tmpPath, NULL, 0, tmpFile);
  printf("TMP file: %s\n", tmpFile);
  if(FAILED(hRes = pURLDownloadToFile( NULL, pacLoc, tmpFile, 0, NULL ))) {
    printf("URLDownloadToFile failed (code: %d\n)", hRes);
    return NULL;
  }

  /* Initialize autoproxy functions */
  if(!pInternetInitAutoProxyDll(0, tmpFile, NULL, NULL, NULL)) {
    printLastError("InternetInitializeAutoProxyDll failed");
    return NULL;
  }

  /* get proxy information */
  *proxyInfo = 0;
  if(!pInternetGetProxyInfo(url,  strlen(url), 
			    hostname, strlen(hostname),
			    &proxyInfo, &proxyLength)) {
    printLastError("InternetInitializeAutoProxyDll failed");
    return NULL;
  }

  /* close autoproxy */
  if(!pInternetDeInitAutoProxyDll(NULL, 0)) {
    printLastError("InternetInitializeAutoProxyDll failed");
  }

  return proxyInfo;
}

#endif /* 0 */

