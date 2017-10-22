extern SOCKET	g_sockServerAdminQuery;
extern SOCKET	g_sockServerForAdminSecure;

DWORD	ServerThreadAdminQuery(LPVOID pParam);
DWORD	ServerThreadForAdminSecure(LPVOID pParam);
DWORD   CommThreadAdminQuery(LPVOID pParam);
DWORD   CommThreadForAdminSecure(LPVOID pParam);
int		SaveAccountSetFile(LPBYTE lpBuffer, int nLength, LPWSTR szFileName);
