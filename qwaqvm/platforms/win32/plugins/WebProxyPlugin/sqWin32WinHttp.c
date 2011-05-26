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

/* sqWin32WinHttp.c: Windows proxy configuration */
#include <malloc.h>
#include <windows.h>
#include <wininet.h>
#include "WebProxyPlugin.h"

/**************************************************************************/
/* Prototype declarations for lack of winhttp.h in mingw headers          */
/**************************************************************************/

/* #include <winhttp.h> */

#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY 0
#define WINHTTP_ACCESS_TYPE_NO_PROXY      1
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY   3

#define WINHTTP_AUTOPROXY_AUTO_DETECT         0x00000001
#define WINHTTP_AUTOPROXY_CONFIG_URL          0x00000002
#define WINHTTP_AUTOPROXY_RUN_INPROCESS       0x00010000
#define WINHTTP_AUTOPROXY_RUN_OUTPROCESS_ONLY 0x00020000

#define WINHTTP_AUTO_DETECT_TYPE_DHCP   0x00000001
#define WINHTTP_AUTO_DETECT_TYPE_DNS_A  0x00000002

typedef struct {
  DWORD dwFlags;
  DWORD dwAutoDetectFlags;
  LPCWSTR lpszAutoConfigUrl;
  LPVOID lpvReserved;
  DWORD dwReserved;
  BOOL fAutoLogonIfChallenged;
} WINHTTP_AUTOPROXY_OPTIONS;

typedef struct {
  DWORD dwAccessType;
  LPWSTR lpszProxy;
  LPWSTR lpszProxyBypass;
} WINHTTP_PROXY_INFO;

typedef struct {
  BOOL fAutoDetect;
  LPWSTR lpszAutoConfigUrl;
  LPWSTR lpszProxy;
  LPWSTR lpszProxyBypass;
} WINHTTP_CURRENT_USER_IE_PROXY_CONFIG;

typedef HINTERNET (CALLBACK *pfnWinHttpOpen)(
  LPCWSTR pwszUserAgent,
  DWORD dwAccessType,
  LPCWSTR pwszProxyName,
  LPCWSTR pwszProxyBypass,
  DWORD dwFlags
);

typedef BOOL (CALLBACK *pfnWinHttpGetProxyForUrl)(
  HINTERNET hSession,
  LPCWSTR lpcwszUrl,
  WINHTTP_AUTOPROXY_OPTIONS* pAutoProxyOptions,
  WINHTTP_PROXY_INFO* pProxyInfo
);

typedef BOOL (CALLBACK *pfnWinHttpGetIEProxyConfig)(
  WINHTTP_CURRENT_USER_IE_PROXY_CONFIG* pProxyConfig
);

typedef BOOL (CALLBACK *pfnWinHttpDetectAutoProxyConfigUrl)(
  DWORD dwAutoDetectFlags,
  LPWSTR* ppwszAutoConfigUrl
);

typedef BOOL (CALLBACK *pfnWinHttpGetDefaultProxyConfig)(
  WINHTTP_PROXY_INFO* pProxyInfo
);

static HMODULE   hWinhttpDLL = NULL;
static HINTERNET hSession    = NULL;

/* We look up all of these dynamically since we don't have the import libs */
static pfnWinHttpOpen             pWinHttpOpen;
static pfnWinHttpGetProxyForUrl   pWinHttpGetProxyForUrl;
static pfnWinHttpGetIEProxyConfig pWinHttpGetIEProxyConfig;
static pfnWinHttpDetectAutoProxyConfigUrl pWinHttpDetectAutoProxyConfigUrl;
static pfnWinHttpGetDefaultProxyConfig pWinHttpGetDefaultProxyConfig;

/* winHttpInit: Initialize winHttp support. */
int winHttpInit(void) {
  WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieConfig;

  /* Open WinHttp.dll */
  if(!hWinhttpDLL) {
    hWinhttpDLL = LoadLibrary( "winhttp.dll" );
    pWinHttpOpen = (pfnWinHttpOpen)
      GetProcAddress( hWinhttpDLL, "WinHttpOpen");
    pWinHttpGetProxyForUrl = (pfnWinHttpGetProxyForUrl)
     GetProcAddress( hWinhttpDLL, "WinHttpGetProxyForUrl");
    pWinHttpGetIEProxyConfig = (pfnWinHttpGetIEProxyConfig)
      GetProcAddress( hWinhttpDLL, "WinHttpGetIEProxyConfigForCurrentUser");
    pWinHttpDetectAutoProxyConfigUrl= (pfnWinHttpDetectAutoProxyConfigUrl)
      GetProcAddress( hWinhttpDLL, "WinHttpDetectAutoProxyConfigUrl");
    pWinHttpGetDefaultProxyConfig = (pfnWinHttpGetDefaultProxyConfig)
      GetProcAddress( hWinhttpDLL, "WinHttpGetDefaultProxyConfiguration" );

    if(!pWinHttpOpen || !pWinHttpGetProxyForUrl ||
       !pWinHttpGetIEProxyConfig || !pWinHttpDetectAutoProxyConfigUrl ||
       !pWinHttpGetDefaultProxyConfig) {
      printf("Could not locate Winhttp functions\n");
      return 0;
    }
  }

  /* Create a single hInternet Session which is reused for all queries */
  if(!hSession) {
    hSession = pWinHttpOpen( L"Squeak",
			     WINHTTP_ACCESS_TYPE_NO_PROXY,
			     0,0,0);
    if(!hSession) {
      printf("WinHttpOpen failed\n");
      return 0;
    }
  }
  return 1;
}

/**************************************************************************/
/* The actual functions using WinHttp                                     */
/**************************************************************************/

/* dynamicAutoProxyUrl: Detect the url for the autoproxy (PAC)
   configuration file. Returns NULL on error; a pointer to the PAC url
   otherwise. This can be REALLY slow (>10 secs) so call it with care. */
char *dynamicAutoProxyUrl(void) {
  static char configUrl[1024];
  WCHAR *wideConfigUrl;

  if(!winHttpInit()) return NULL;
  if(!pWinHttpDetectAutoProxyConfigUrl(WINHTTP_AUTO_DETECT_TYPE_DNS_A |
				       WINHTTP_AUTO_DETECT_TYPE_DHCP,
				       &wideConfigUrl)) {
    printLastError("WinHttpDetectAutoProxyConfigUrl failed\n");
    return NULL;
  }

  /* Convert URL to UTF-8 */
  configUrl[0] = 0;
  if(wideConfigUrl) {
    WideCharToMultiByte(CP_UTF8, 0, wideConfigUrl, -1, configUrl, 
			1024, NULL, NULL);
    GlobalFree(wideConfigUrl);
  }
  return configUrl;
}

/* staticAutoProxyUrl: Answer the configured url for the autoproxy (PAC)
   configuration file. Returns NULL on error or a pointer to the PAC url. */
char *staticAutoProxyUrl(void) {
  static char configUrl[1024];
  WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieConfig;

  if(!winHttpInit()) return NULL;

  ZeroMemory(&ieConfig, sizeof(ieConfig));
  if(!pWinHttpGetIEProxyConfig(&ieConfig)) {
    printLastError("WinHttpGetIEProxyConfig failed");
    return NULL;
  }

  configUrl[0] = 0;
  if(ieConfig.lpszAutoConfigUrl) {
    WideCharToMultiByte(CP_UTF8, 0, ieConfig.lpszAutoConfigUrl, 
			-1, configUrl, 2000, NULL, NULL);
  }

  /* Clean up */
  if(ieConfig.lpszProxy) GlobalFree(ieConfig.lpszProxy);
  if(ieConfig.lpszProxyBypass) GlobalFree(ieConfig.lpszProxyBypass);
  if(ieConfig.lpszAutoConfigUrl) GlobalFree(ieConfig.lpszAutoConfigUrl);
  return configUrl;
}

/* defaultProxyServer: Answer the default proxy server(s) for this box.*/
char *defaultProxyServer(void) {
  static char proxyInfo[2048];
  WINHTTP_PROXY_INFO info;
  WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieConfig;
  WCHAR *defProxy=NULL, *ieProxy=NULL;

  if(!winHttpInit()) return NULL;

  ZeroMemory(&info, sizeof(info));
  if(pWinHttpGetDefaultProxyConfig(&info)) {
    defProxy = info.lpszProxy;
  } else {
    printLastError("WinHttpGetDefaultProxyConfiguration failed\n");
  }

  ZeroMemory(&ieConfig, sizeof(ieConfig));
  if(pWinHttpGetIEProxyConfig(&ieConfig)) {
    ieProxy = ieConfig.lpszProxy;
  } else {
    printLastError("WinHttpGetIEProxyConfig failed");
  }

  proxyInfo[0] = 0;
  if(defProxy && *defProxy) {
    WideCharToMultiByte(CP_UTF8, 0, defProxy, -1, proxyInfo, 2000, NULL, NULL);
  }
  if(ieProxy && *ieProxy) {
    char *proxyStart;
    int proxyLen;
    if(*proxyInfo) strcat(proxyInfo,"; ");
    proxyStart = proxyInfo+strlen(proxyInfo);
    proxyLen = sizeof(proxyInfo) - strlen(proxyInfo);
    WideCharToMultiByte(CP_UTF8, 0,ieProxy, -1, proxyStart,proxyLen,NULL,NULL);
  }

  /* Clean up */
  if(info.lpszProxy) GlobalFree(info.lpszProxy);
  if(info.lpszProxyBypass) GlobalFree(info.lpszProxyBypass);
  if(ieConfig.lpszProxy) GlobalFree(ieConfig.lpszProxy);
  if(ieConfig.lpszProxyBypass) GlobalFree(ieConfig.lpszProxyBypass);
  if(ieConfig.lpszAutoConfigUrl) GlobalFree(ieConfig.lpszAutoConfigUrl);
  return proxyInfo;
}

/* proxyServerForUrl: Detect the proxy server for the given URL. */
char *proxyServerForUrl(char *url, char *pacLoc) {
  int i;
  WINHTTP_AUTOPROXY_OPTIONS  options;
  WINHTTP_PROXY_INFO info;
  DWORD szProxy = sizeof(info);
  WCHAR wurl[1024];
  WCHAR wpac[1024];
  static char proxy[1024];

  if(!winHttpInit()) return NULL;

  /* Convert UTF-8 to WCHAR */
  MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, 1024);
  MultiByteToWideChar(CP_UTF8, 0, pacLoc, -1, wpac, 1024);

  ZeroMemory(&options, sizeof(options));
  ZeroMemory(&info, sizeof(info));

  options.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
  options.lpszAutoConfigUrl = wpac;
  options.fAutoLogonIfChallenged = TRUE;

  /* printf("Looking for proxy url...\n"); */
  if(!pWinHttpGetProxyForUrl( hSession, wurl, &options, &info)){
      printLastError("WinHttpGetProxyForUrl failed");
      return NULL;
  }

  /* Copy proxy info */
  if(info.lpszProxy) {
    WideCharToMultiByte(CP_UTF8,0,info.lpszProxy, -1, proxy, 1024, NULL, NULL);
  } else *proxy = 0;
  if(info.lpszProxy) GlobalFree(info.lpszProxy);
  if(info.lpszProxyBypass) GlobalFree(info.lpszProxyBypass);

  return proxy;
}
