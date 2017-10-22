
extern SOCKET	g_sockServerDes;
extern SOCKET	g_sockServerSecure;
extern SOCKET	g_sockServerState;
DWORD   ServerThreadDes(LPVOID pParam);
DWORD	CommThreadDes(LPVOID pParam);
DWORD	ServerThreadClientState(LPVOID pParam);
DWORD	CommThreadSecure(LPVOID pParam);
DWORD	ServerThreadSecure(LPVOID pParam);
LPBYTE	GetAccountFile(LPWSTR szUserId, int& nLength);
DWORD	ServerThreadClientState(LPVOID pParam);

