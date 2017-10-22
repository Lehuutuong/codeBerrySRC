#ifndef _SERVER_ENGINE_H
#define _SERVER_ENGINE_H

#define		MAX_ADMIN			100
#define		SERVER_PROFILE		L"AnglerProfile.dat"
#define		SERVER_PROFILE2		L"AnglerProfile2.dat"
#define		LOGIN_COUNT_ATDAY	5

#define		ADMINLEVEL_TOP		0
#define		ADMINLEVEL_LOW		1

struct ACCOUNTINFO
{
	BYTE	pbAccount[8];
	BYTE	pbRegDate[4];
	BYTE	pbExpDate[4];
	BYTE	dwStatus;
	WCHAR	szAccount[20];
	WCHAR	szPassword[20];
	WCHAR	szAdminName[20];
	DWORD   dwLocalIP;
	DWORD   dwConnectIP;
	BYTE	nMaxMulti;
	BYTE	dwReserve[31];
};

struct ADMININFO
{
	WCHAR	szID[20];
	WCHAR	szPwd[20];
	int		dwMaxUser;
	int		dwLevel;
	int		dwState;
	BYTE	pbAccount[8];
};

struct ACCOUNTSOCKET
{
	DWORD	dwIP;
	SOCKET	Sock;
};

extern	ACCOUNTINFO			*g_pAccountInfo;
extern	int					g_nAccountNum;
extern	ADMININFO			*g_pAdminInfo;
extern	int					g_nAdminNum;
extern	STATEINFO			*g_pAnglerStateInfo;
extern  int					g_nAnglerStateNum;
extern	CRITICAL_SECTION	g_CR;
extern	CRITICAL_SECTION	g_OrimState_CR;
extern	CListCtrl			*g_pListLog;
extern  BOOL				g_bVisibleLog;
extern  DWORD				g_dwPatchAddress;
extern  DWORD				g_dwVersion;
extern  BOOL				g_bStopServer;

extern  WCHAR g_szftpID[20];
extern  WCHAR g_szftpPwd[20];
extern  WCHAR g_szftpServer[20];
extern  WCHAR g_szftpFileName[];
extern  WCHAR g_szPatchFile[];
extern  WCHAR g_szAccountPath[MAX_PATH];
extern  WCHAR g_szServerDataFilePath[MAX_PATH];
extern  WCHAR g_szOrimStatePath[MAX_PATH];


void	InitServer();
DWORD	ServerCheckThread(LPVOID pParam);

void	WaitForRecvClosePacketAndCloseSocket(SOCKET* pSockComm);
int		MakePacket(BYTE *pbPacket, char *szFormat, ...);
int		MakePacketAdmin(BYTE *pbPacket, char *szFormat, ...);
int		MakePacketSecure(BYTE *pbPacket, char *szFormat, ...);
void	EncryptPacket(BYTE *pbData, int nLen);
void	EncryptPacketAdmin(BYTE *pbData, int nLen);
void	DecryptPacketAdmin(BYTE *pbData, int nLen);
void	EncryptAutoSettingBuffer(BYTE *pbData, int nLen);
BOOL	ConnectToDBTables();

BOOL	PrintLog(WCHAR *szFormat, ...);

void	DwordToIPString(DWORD PackedIpAddress, LPTSTR IpAddressBuffer);
int		GetDateDiff(LPBYTE pbyDate1, LPBYTE pbyDate2);

BOOL	GetAdminInfo(LPWSTR szAdminID, LPWSTR szAdminPwd, ADMININFO *pAdminInfo);

int InitDBConnection();
void FreeDBConnection();

int GetUseCountAtNow();
int GetCreateCountAtDay(SYSTEMTIME systime);
int GetDeleteCountAtDay(SYSTEMTIME systime);
int GetUseCount1AtDay(SYSTEMTIME systime);
int GetUseCount2AtDay(SYSTEMTIME systime);
int GetUseCountAtDay(SYSTEMTIME systime);
int GetUseTodayLog(LPTSTR lpAutoAccount, SYSTEMTIME* psystime = NULL, int *pnAuthCount = 0);
int GetUseYesterdayLog(LPTSTR lpAutoAccount);
int GetValidateCountAtNow();
void InsertTodayLog(LPTSTR lpAutoAccount, int nLogKind, LPBYTE byDate1 = NULL, LPBYTE byDate2 = NULL, int nDayCount = 0, BYTE nMaxMulti = 0);
void InsertStatistics(LPTSTR lpAutoAccount, int nType, LPTSTR lpAdminName, BYTE nMaxMulti, LPBYTE byDate);
void UpdateTodayLog(int nId, SYSTEMTIME systime);
int InsertUseTodayLog(LPTSTR lpAutoAccont, int nIndex);
int GetMonthUserCount(SYSTEMTIME systime, int nDays);
int GetMonthUserCreateCount(SYSTEMTIME systime);
int GetMonthUserDeleteCount(SYSTEMTIME systime);
int GetValidateUserCountAtDay(SYSTEMTIME systime);
int GetAccountAverageConnectTime(SYSTEMTIME systime);

int GetAccountCountAtDay(SYSTEMTIME systime, LPCTSTR szAccountState);
int InsertAccountState(LPCTSTR szAutoAccount, LPCTSTR szGameId, LPCTSTR szGamePwd, LPCTSTR szAccountState, LPCTSTR szAutoIP, int nIndex);
int InsertCharacState(LPWSTR szAutoAccount, LPWSTR szIP, LPWSTR szCharacState);

int RegistAutoIP(LPWSTR szAutoAccount, LPWSTR szIP);
BOOL CheckAutoIP(LPWSTR szAutoAccount, LPWSTR szIP);

int UpdatePostGold(ULONGLONG ullPostGold, SYSTEMTIME systime);
ULONGLONG GetPostGoldAmount(SYSTEMTIME systime);
int InsertPostGoldState(LPWSTR szAccount, ULONGLONG ullPostGold, LPWSTR szAdmin, SYSTEMTIME systime);

int GetUserLoginTryCount(LPTSTR lpAutoAccount, SYSTEMTIME systime, int *pnTryCount);

void InsertPatchAddress(DWORD dwAddress);
DWORD GetPatchAddress();

LPBYTE GetMultiCount_DaybyDay(SYSTEMTIME systime, int* pLen) ;

#endif _SERVER_ENGINE_H