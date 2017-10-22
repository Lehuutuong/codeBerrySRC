#include "stdafx.h"
#include "Resource.h"
#include "ServerEngine.h"
#include "io.h"
#include "stdio.h"
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include "RC4.h"
#include "../AuthLib/Auth.h"
#include "CrackCheckEngine.h"
#include "ClientServer.h"
#include "AdminServer.h"
#include "Account.h"
#include "Patch.h"
#include "inifile.h"
#include "tstdlibs.h"
#include "AnglerState.h"

#pragma comment(lib, "iphlpapi.lib")

CCrackCheckEngine* m_pCrackCheckEngine = NULL;

SQLHENV g_sql_hEnv = 0;
SQLHDBC g_sql_hDBC = 0;

WCHAR g_szftpID[20] = _T("skRtlrnsdmlgkfn");//낚시군의하루
WCHAR g_szftpPwd[20] = _T("qjfdldjEo?");//벌이어때?
WCHAR g_szftpServer[20];
WCHAR g_szftpFileName[] = _T("/AnglerPatch.pak");
WCHAR g_szPatchFile[] = _T("FTP\\AnglerPatch.pak");
WCHAR g_szAccountPath[MAX_PATH];
WCHAR g_szServerDataFilePath[MAX_PATH];
WCHAR g_szOrimStatePath[MAX_PATH];

CRITICAL_SECTION	g_CR;
CRITICAL_SECTION	g_PrintLog_CR;
CRITICAL_SECTION	g_OrimState_CR;

DWORD				g_dwPatchAddress = 0;

DWORD				g_dwVersion;
BOOL				g_bVisibleLog;

BOOL				g_bStopServer = 0;


BOOL ERRORCHECK(SQLHANDLE hSql, SQLSMALLINT sqltype)
{
	BOOL bError=FALSE;
	SQLSMALLINT msgLen=0,i=1;
	SQLINTEGER error=0;
	SQLTCHAR state[128]=_T("");
	SQLTCHAR szlastErr[SQL_MAX_MESSAGE_LENGTH];
	if(!hSql) 
		return 0;
	while(SQLGetDiagRec(sqltype, hSql, i++, state,	&error, szlastErr, SQL_MAX_MESSAGE_LENGTH, &msgLen)!=SQL_NO_DATA)
	{
		OutputDebugStringW(szlastErr);
		if(wcsstr(szlastErr, L"has gone away") || wcsstr(szlastErr, L"오류"))
		{
			ConnectToDBTables();
			bError=TRUE;
		}
	}
	return bError;
}

int RecvData(SOCKET sock, char *buf, int len, int flags)
{
	int nReadBytes = 0;
	while (nReadBytes < len)
	{
		int nRecv = recv(sock, buf + nReadBytes, len - nReadBytes, flags);
		if (nRecv <= 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				break;
		}
		else
		{
			nReadBytes += nRecv;
		}
		Sleep(1);
	}
	return nReadBytes;
}

int SendData(SOCKET sock, char *buf, int len, int flags)
{
	int nWriteBytes = 0;
	while (nWriteBytes < len)
	{
		int nSend = send(sock, buf + nWriteBytes, len - nWriteBytes, flags);
		if (nSend <= 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				break;
		}
		else
		{
			nWriteBytes += nSend;
		}
		Sleep(1);
	}
	return nWriteBytes;
}

BOOL	PrintLog(WCHAR *szFormat, ...)
{
	va_list		arg;
	FILE		*fp;
	WCHAR		szLog[1000], szPath[MAX_PATH], szTemp[MAX_PATH];
	SYSTEMTIME	time;
	int			i;
	
	// 시간얻기
	GetLocalTime(&time);
	EnterCriticalSection(&g_PrintLog_CR);
	// 문자렬얻기 및 변환
	va_start(arg, szFormat);
	vswprintf(szLog, szFormat, arg);
	va_end(arg);

	wcscat(szLog, L"\r\n");
	// 리스트에 추가
	if(g_bVisibleLog)
	{
		i = g_pListLog->GetItemCount();
		if(i >= 5000)
		{
			g_pListLog->DeleteItem(4999);
		}
		swprintf(szTemp, L"%04d/%02d/%02d %02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
		g_pListLog->InsertItem(0, szTemp);
		g_pListLog->SetItemText(0, 1, szLog);
		g_pListLog->SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	}

	// 파일에 추가
	wcscpy(szPath, g_szServerDataFilePath);
	swprintf(szTemp, L"Trace\\%04d-%02d-%02d.log", time.wYear, time.wMonth, time.wDay);
	wcscat(szPath, szTemp);

	fp = _wfopen(szPath, L"ab+");
	if(!fp)
	{
		fp = _wfopen(szPath, L"wb");
		if(!fp)
		{
			LeaveCriticalSection(&g_PrintLog_CR);
			return FALSE;
		}
	}

	fseek(fp, 0L, SEEK_END);
	if(ftell(fp) == 0)
	{
		WORD wd = 0xFEFF;
		fwrite(&wd, 1, 2, fp);
	}

	WCHAR wsztime[50] = {0,};
	swprintf(wsztime, L"%2d:%02d:%02d - ", time.wHour, time.wMinute, time.wSecond);
	fwrite(wsztime, 1, 2*(wcslen(wsztime)), fp);

	fwrite(szLog, 1, 2*(wcslen(szLog)), fp);
	fclose(fp);

	LeaveCriticalSection(&g_PrintLog_CR);
	return TRUE;
}


UINT GetMyPrivateProfileInt(LPTSTR lpSecName, LPTSTR lpKeyName, int nDefault, CIniFile* pInifile)
{

	CIniSection* pSection = pInifile->GetSection(lpSecName);
	if( pSection )
	{
		CIniKey* pKey = pSection->GetKey(lpKeyName);
		if( pKey )
		{
#ifdef _UNICODE
			std::wstring str = pKey->GetValue();
#else
			std::string str = pKey->GetValue();
#endif
			return _ttoi(str.c_str());
		}
	}
	return nDefault;
}

DWORD GetMyPrivateProfileString(LPTSTR lpSecName, LPTSTR lpKeyName, LPTSTR lpDefault, LPTSTR lpReturn, DWORD dwSize, CIniFile* pInifile)
{
	CIniSection* pSection = pInifile->GetSection(lpSecName);
	if( pSection )
	{
		CIniKey* pKey = pSection->GetKey(lpKeyName);
		if( pKey )
		{
#ifdef _UNICODE
			std::wstring str = pKey->GetValue();
#else
			std::string str = pKey->GetValue();
#endif
			if(dwSize>str.length())
			{
				_tcscpy(lpReturn, str.c_str());
				return 0;
			}
			else
				return str.length();

		}
	}
	_tcscpy_s(lpReturn, dwSize, lpDefault);
	return -1;
}

BOOL WriteMyPrivateProfileString(LPCTSTR lpSecName, LPCTSTR lpKeyName, LPCTSTR lpString, CIniFile* pInifile)
{
	pInifile->AddSection(lpSecName)->AddKey(lpKeyName)->SetValue(lpString);
	return TRUE;
}

void EncryptAutoSettingBuffer(BYTE *pbData, int nLen)
{
	int i;

	for(i = 0; i < nLen; i++)
	{
		pbData[i] ^= (BYTE)(0x9C+(i&0xFF));
	}
}

void GetDateUpdate(LPBYTE pbyDate, int nUpdateDays)
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	systime.wYear = *(short*)pbyDate;
	systime.wMonth = pbyDate[2];
	systime.wDay = pbyDate[3];
	FILETIME filetimeStart;
	SystemTimeToFileTime(&systime, &filetimeStart);
	ULONGLONG ullOneDay = 864000000000;
	if(nUpdateDays >= 0)
	{
		*(ULONGLONG*)(&filetimeStart) += ullOneDay * nUpdateDays;
	}
	else
	{
		*(ULONGLONG*)(&filetimeStart) -= ullOneDay * abs(nUpdateDays);
	}
	FileTimeToSystemTime(&filetimeStart, &systime);
	*(short*)pbyDate = systime.wYear;
	pbyDate[2] = (BYTE)systime.wMonth;
	pbyDate[3] = (BYTE)systime.wDay;
}

int GetDateDiff(LPBYTE pbyDate1, LPBYTE pbyDate2)
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	systime.wYear = *(short*)pbyDate1;
	systime.wMonth = pbyDate1[2];
	systime.wDay = pbyDate1[3];
	ULONGLONG filetimeStart1;
	SystemTimeToFileTime(&systime, (LPFILETIME)&filetimeStart1);

	systime.wYear = *(short*)pbyDate2;
	systime.wMonth = pbyDate2[2];
	systime.wDay = pbyDate2[3];
	ULONGLONG filetimeStart2;
	SystemTimeToFileTime(&systime, (LPFILETIME)&filetimeStart2);
	ULONGLONG ullOneDay = 864000000000;
	if(filetimeStart2 >= filetimeStart1)
	{
		ULONGLONG ullDiffDays = (filetimeStart2 - filetimeStart1) / ullOneDay;
		return (int)ullDiffDays;
	}
	else
	{
		ULONGLONG ullDiffDays = (filetimeStart1 - filetimeStart2) / ullOneDay;
		return 0 - (int)ullDiffDays;
	}
}

BOOL ExtractFileFromResource(UINT nResourceID, LPWSTR lpszFileName)
{
	VMProtectBegin("ExtractFileFromResource");

	HGLOBAL		hGlobal = NULL;
	HRSRC		hSource = NULL;
	LPVOID		lpVoid  = NULL;
	int			nSize   = 0;
	BOOL		bRet = FALSE;
	HINSTANCE	hInstance = GetModuleHandle(NULL);

	hSource = FindResource(hInstance, MAKEINTRESOURCE(nResourceID), VMProtectDecryptStringW(L"BIN"));

	if(hSource == NULL)
	{
		return(FALSE);
	}

	hGlobal = LoadResource(hInstance, hSource);
	if(hGlobal == NULL)
	{
		return(FALSE);
	}

	lpVoid = LockResource(hGlobal);
	if(lpVoid == NULL)
	{
		return(FALSE);
	}

	nSize = (UINT)SizeofResource(hInstance, hSource);

	FILE* fp = _wfopen(lpszFileName, L"wb");
	if(fp)
	{
		bRet = (fwrite(lpVoid, 1, nSize, fp) == (size_t)nSize);
		fclose(fp);
	}

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	VMProtectEnd();

	return bRet;
}

void InitServer()
{
	VMProtectBegin("InitServer");

	WSADATA		wsaData;
	WSAStartup(0x202,&wsaData);

	InitializeCriticalSection(&g_CR);
	InitializeCriticalSection(&g_PrintLog_CR);
	InitializeCriticalSection(&g_OrimState_CR);


	wcscpy(g_szServerDataFilePath, L"C:\\AnglerServer\\");
	CreateDirectory(g_szServerDataFilePath, NULL);

	wcscpy(g_szAccountPath, g_szServerDataFilePath);
	wcscat(g_szAccountPath, L"AccountSet");
	CreateDirectory(g_szAccountPath, NULL);	

	TCHAR szPath[MAX_PATH];
	wcscpy(szPath, g_szServerDataFilePath);
	wcscat(szPath, L"Trace");
	CreateDirectory(szPath, NULL);	

	LoadAccountInfo();
	LoadAdminInfo();
	LoadAutoPatchFile();

	InitDBConnection();

	LoadStateInfo();
	Initrc4Cipher();

	PrintLog(L"Start Server");
	
	CA2W ca2w(SERVER_FTP_IP);
	wcscpy(g_szftpServer, ca2w.m_psz);

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ServerThreadSecure, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ServerThreadAdminQuery, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ServerThreadForAdminSecure, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ServerThreadDes, NULL, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ServerThreadClientState, NULL, NULL, NULL);
	
	CreateThread(NULL, 10 * 1024, (LPTHREAD_START_ROUTINE)ServerCheckThread, NULL, NULL, NULL);


	m_pCrackCheckEngine = new CCrackCheckEngine();
	if (m_pCrackCheckEngine)
		m_pCrackCheckEngine->Init();
	m_pCrackCheckEngine->WriteStatusLog("Start Server");

	VMProtectEnd();
}

void WriteAccountInfo()
{
	char szBuffer[500] = {0};
	char szFileName[MAX_PATH] = {0};

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	sprintf_s(szFileName, MAX_PATH, "%s\\Account_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);
	FILE *fp;
	fopen_s(&fp, szFileName, "wb");
	if (fp)
	{
		EnterCriticalSection(&g_CR);
		for (int i = 0; i < g_nAccountNum; i ++)
		{
			if(CheckHuntUserExpiration(i) == TRUE)
			{
				sprintf_s(szBuffer, 500, "ID=%ws Pass=%ws Multi=%d Start=%04d-%02d-%02d End=%04d-%02d-%02d IP=%ws Admin=%ws", 
					g_pAccountInfo[i].szAccount, g_pAccountInfo[i].szPassword, 
					MULTI_COUNT, 
					*(short *)g_pAccountInfo[i].pbRegDate, g_pAccountInfo[i].pbRegDate[2], g_pAccountInfo[i].pbRegDate[3], 
					*(short *)g_pAccountInfo[i].pbExpDate, g_pAccountInfo[i].pbExpDate[2], g_pAccountInfo[i].pbExpDate[3], 
					"", g_pAccountInfo[i].szAdminName);
				fprintf(fp, "%s\n", szBuffer);
			}
		}
		LeaveCriticalSection(&g_CR);

		fclose(fp);
	}
}

DWORD	ServerCheckThread(LPVOID pParam)
{
	Sleep(10000);
	FILETIME   filetime;
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	SystemTimeToFileTime(&systime, &filetime);

	ULONG64 ul64EndTick = 10000000; // 1초
	ul64EndTick *= 60 * 60 * 24; // 하루
	//ul64EndTick *= 60 ; // 1분
	ul64EndTick += *(ULONG64*)(&filetime);

	for(;;)
	{
		// 유효계정수 로그기록
		EnterCriticalSection(&g_CR);
		int nID = GetUseTodayLog(L"accountmanager");
		if(nID == -1)
		{
			InsertTodayLog(L"accountmanager", 4, NULL, NULL, GetValidateCountAtNow());
			WriteAccountInfo();
		}
		BackUpAccountFile();
		LeaveCriticalSection(&g_CR);

		GetLocalTime(&systime);
		SystemTimeToFileTime(&systime, &filetime);

		ULONG64 ul64CurrentTick;
		ul64CurrentTick = *(ULONG64*)(&filetime);

		if(ul64CurrentTick > ul64EndTick) // 종료
		{
			TCHAR szPath[MAX_PATH] = {0,};
			GetModuleFileName(NULL, szPath, MAX_PATH);
			ShellExecute(NULL, L"open", szPath, NULL, NULL, SW_SHOW);

			Sleep(5000); // 로드되는 시간을 잡는다.
			g_bStopServer = 1;
			closesocket(g_sockServerAdminQuery);
			closesocket(g_sockServerForAdminSecure);
			closesocket(g_sockServerDes);
			closesocket(g_sockServerSecure);
			closesocket(g_sockServerState);

			Sleep(3000); // 현재 접속된 련결을 위해서
			FreeDBConnection();
			ExitProcess(1);

		}
		Sleep(1000 * 60 * 30); // 30분 // MySQL DB Wait_TimeOut 이 표준으로 8시간임을 고려하여 주기적으로 접속
	}
}
void BytesToString(WCHAR *szBuffer, BYTE *pbBuffer, int nLen)
{
	int			i;

	for(i = 0; i < nLen; i++)
	{
		swprintf(szBuffer+i*2, L"%02X", pbBuffer[i]);
	}
	szBuffer[i*2] = 0;
}

void PrintLogWorker(const char *format,...)
{	
	char logData1[1024];	

	va_list arglist;

	va_start(arglist, format);
	_vsnprintf(logData1, 1024, format, arglist);
	va_end(arglist);

	OutputDebugStringA( logData1 );
}

void WaitForRecvClosePacketAndCloseSocket(SOCKET* pSockComm)
{
	DWORD dwTimeout = RECV_TIMEOUT;
	setsockopt(*pSockComm, SOL_SOCKET, SO_RCVTIMEO, (char*)&dwTimeout, 4);

	PrintLogWorker("WaitClose Start (%d)", *pSockComm);
	DWORD dwTick = GetTickCount();
	char byTmp[1];
	int nRecv = recv(*pSockComm, byTmp, 1, 0);
	if (nRecv <= 0)
	{
		if (GetTickCount() - dwTick < RECV_TIMEOUT) // Close 패킷이 수신된 경우(우아한 Close)
		{
			PrintLogWorker("WaitClose Success (%d) Tick: %d", *pSockComm, GetTickCount() - dwTick);
			closesocket(*pSockComm);
			return;
		}
	}
	PrintLogWorker("WaitClose Fail (%d) LastError: %d, nRecv: %d", *pSockComm, WSAGetLastError(), nRecv);
	struct linger optval;
	optval.l_onoff = 1;
	optval.l_linger = 0;
	setsockopt(*pSockComm, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));
	closesocket(*pSockComm);
}

void DwordToIPString(DWORD PackedIpAddress, LPTSTR IpAddressBuffer)
{
	BYTE IpNumbers[4];

	IpNumbers[0] = (BYTE)((PackedIpAddress >> 24) & 0xFF);
	IpNumbers[1] = (BYTE)((PackedIpAddress >> 16) & 0xFF);
	IpNumbers[2] = (BYTE)((PackedIpAddress >> 8) & 0xFF);
	IpNumbers[3] = (BYTE)((PackedIpAddress >> 0) & 0xFF);
	_stprintf(IpAddressBuffer, _T("%d.%d.%d.%d"), IpNumbers[0], IpNumbers[1], IpNumbers[2], IpNumbers[3]);
}

void EncryptPacket(BYTE *pbData, int nLen)
{
	int			i;

	for(i = 0; i < nLen; i++)
	{
		pbData[i] ^= (BYTE)(0x17+(i&0xFF));
	}
}


void EncryptPacketAdmin(BYTE *pbData, int nLen)
{
	int i;

	BYTE byTmp1 = 0, byTmp2 = 0;
	for(i = 0; i < nLen; i++)
	{
		byTmp2 = pbData[i];
		pbData[i] ^= (byTmp1 * 3);
		pbData[i] ^= (BYTE)(0x51+(i&0xFF));
		byTmp1 = byTmp2;
	}
}

void DecryptPacketAdmin(BYTE *pbData, int nLen)
{
	int i;

	BYTE byTmp = 0;
	for(i = 0; i < nLen; i++)
	{
		pbData[i] ^= (byTmp * 3);
		pbData[i] ^= (BYTE)(0x51+(i&0xFF));
		byTmp = pbData[i];
	}
}

int MakePacket(BYTE *pbPacket, char *szFormat, ...)
{
	va_list	arg;
	int		i, nLen, nOffset = 0;

	nLen = strlen(szFormat);
	va_start(arg, szFormat);
	for(i = 0; i < nLen; i++)
	{
		switch(szFormat[i])
		{
		case 'c':
			*(BYTE *)(pbPacket+nOffset) = va_arg(arg, char);
			nOffset += 1;
			break;
		case 'h':
			*(WORD *)(pbPacket+nOffset) = va_arg(arg, WORD);
			nOffset += 2;
			break;
		case 'd':
			*(DWORD *)(pbPacket+nOffset) = va_arg(arg, DWORD);
			nOffset += 4;
			break;
		case 'b':
			{
				BYTE	*pbTemp = va_arg(arg, BYTE *);
				int		n = va_arg(arg, int);
				if(n)
				{
					memcpy(pbPacket+nOffset, pbTemp, n);
					nOffset += n;
				}
			}
			break;
		case 'r':
			{
				int		j, n = va_arg(arg, int);
				if(n)
				{
					srand((UINT)time(NULL));
					for(j = 0; j < n; j++)
					{
						pbPacket[nOffset++] = (BYTE)(rand()&0xFF);
					}
				}
			}
			break;
		case 'z':
			{
				int		n = va_arg(arg, int);
				memset(pbPacket+nOffset, 0, n);
				nOffset += n;
			}
			break;
		case 'S':
			{
				WCHAR	*szTemp = va_arg(arg, WCHAR *);
				swprintf((WCHAR *)(pbPacket+nOffset), L"%s", szTemp);
				nOffset += (wcslen(szTemp)+1) * sizeof(WCHAR);
			}
			break;
		case 's':
			{
				char	*szTemp = va_arg(arg, char *);
				sprintf((char *)pbPacket+nOffset, "%s", szTemp);
				nOffset += strlen(szTemp)+1;
			}
			break;
		}
	}
	*(int *)(pbPacket+1) = nOffset-5;
	va_end(arg);
	EncryptPacket(pbPacket+5, nOffset-5);
	return nOffset;
}


int MakePacketSecure(BYTE *pbPacket, char *szFormat, ...)
{
	va_list	arg;
	int		i, nLen, nOffset = 0;

	nLen = strlen(szFormat);
	va_start(arg, szFormat);
	for(i = 0; i < nLen; i++)
	{
		switch(szFormat[i])
		{
		case 'c':
			*(BYTE *)(pbPacket+nOffset) = va_arg(arg, char);
			nOffset += 1;
			break;
		case 'h':
			*(WORD *)(pbPacket+nOffset) = va_arg(arg, WORD);
			nOffset += 2;
			break;
		case 'd':
			*(DWORD *)(pbPacket+nOffset) = va_arg(arg, DWORD);
			nOffset += 4;
			break;
		case 'b':
			{
				BYTE	*pbTemp = va_arg(arg, BYTE *);
				int		n = va_arg(arg, int);
				if(n)
				{
					memcpy(pbPacket+nOffset, pbTemp, n);
					nOffset += n;
				}
			}
			break;
		case 'r':
			{
				int		j, n = va_arg(arg, int);
				if(n)
				{
					srand((UINT)time(NULL));
					for(j = 0; j < n; j++)
					{
						pbPacket[nOffset++] = (BYTE)(rand()&0xFF);
					}
				}
			}
			break;
		case 'z':
			{
				int		n = va_arg(arg, int);
				memset(pbPacket+nOffset, 0, n);
				nOffset += n;
			}
			break;
		case 'S':
			{
				WCHAR	*szTemp = va_arg(arg, WCHAR *);
				swprintf((WCHAR *)(pbPacket+nOffset), L"%s", szTemp);
				nOffset += (wcslen(szTemp)+1) * sizeof(WCHAR);
			}
			break;
		case 's':
			{
				char	*szTemp = va_arg(arg, char *);
				sprintf((char *)pbPacket+nOffset, "%s", szTemp);
				nOffset += strlen(szTemp)+1;
			}
			break;
		}
	}
	*(int *)(pbPacket+1) = nOffset-5;
	va_end(arg);
	rc4EncryptStream(pbPacket+5, nOffset-5);
	return nOffset;
}

int MakePacketAdmin(BYTE *pbPacket, char *szFormat, ...)
{
	va_list	arg;
	int		i, nLen, nOffset = 0;

	nLen = strlen(szFormat);
	va_start(arg, szFormat);
	for(i = 0; i < nLen; i++)
	{
		switch(szFormat[i])
		{
		case 'c':
			*(BYTE *)(pbPacket+nOffset) = va_arg(arg, char);
			nOffset += 1;
			break;
		case 'h':
			*(WORD *)(pbPacket+nOffset) = va_arg(arg, WORD);
			nOffset += 2;
			break;
		case 'd':
			*(DWORD *)(pbPacket+nOffset) = va_arg(arg, DWORD);
			nOffset += 4;
			break;
		case 'b':
			{
				BYTE	*pbTemp = va_arg(arg, BYTE *);
				int		n = va_arg(arg, int);
				if(n)
				{
					memcpy(pbPacket+nOffset, pbTemp, n);
					nOffset += n;
				}
			}
			break;
		case 'r':
			{
				int		j, n = va_arg(arg, int);
				if(n)
				{
					srand((UINT)time(NULL));
					for(j = 0; j < n; j++)
					{
						pbPacket[nOffset++] = (BYTE)(rand()&0xFF);
					}
				}
			}
			break;
		case 'z':
			{
				int		n = va_arg(arg, int);
				memset(pbPacket+nOffset, 0, n);
				nOffset += n;
			}
			break;
		case 'S':
			{
				WCHAR	*szTemp = va_arg(arg, WCHAR *);
				swprintf((WCHAR *)(pbPacket+nOffset), L"%s", szTemp);
				nOffset += (wcslen(szTemp)+1) * sizeof(WCHAR);
			}
			break;
		case 's':
			{
				char	*szTemp = va_arg(arg, char *);
				sprintf((char *)pbPacket+nOffset, "%s", szTemp);
				nOffset += strlen(szTemp)+1;
			}
			break;
		}
	}
	*(int *)(pbPacket+1) = nOffset-5;
	va_end(arg);
	EncryptPacketAdmin(pbPacket+5, nOffset-5);
	return nOffset;
}

int StringToBytes(BYTE *pbBuffer, WCHAR *szBuffer)
{
	int		i, nLen;
	WCHAR	szTemp[3], *szEnd;

	nLen = wcslen(szBuffer);
	nLen /= 2;
	for(i = 0; i < nLen; i++)
	{
		wcsncpy(szTemp, szBuffer+i*2, 2);
		szTemp[2] = 0;
		pbBuffer[i] = (BYTE)wcstol(szTemp, &szEnd, 16);
	}
	return nLen;
}

int InsertCharacState(LPWSTR szAutoAccount, LPWSTR szIP, LPWSTR szCharacState)
{
	if(wcslen(szCharacState) >= 1000)
		return 0;

	CString szRegistTime;
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	szRegistTime.Format(L"%d-%d-%d %d:%d:%d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	int nRet = 0;

	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	int AccountStateId = -1;
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );

		SQLTCHAR SqlCharacStateInsert[2000] = {0,};
		SQLTCHAR SqlCharacStateInsertPtn[] = _T("INSERT INTO `characstate` SET `regist_time` = '%s', `account` = '%s' , `ip` = '%s' , %s");
		_stprintf(SqlCharacStateInsert, SqlCharacStateInsertPtn, szRegistTime, szAutoAccount, szIP, szCharacState);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlCharacStateInsert,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
			nRet = 1;
		}
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	return nRet;
}

int InsertPostGoldState(LPWSTR szAccount, ULONGLONG ullPostGold, LPWSTR szAdmin, SYSTEMTIME systime)
{
	WCHAR szRegistTime[100];
	swprintf(szRegistTime, L"%d-%d-%d %d:%d:%d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	int nRet = 0;
	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	int AccountStateId = -1;
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );

		SQLTCHAR SqlInsertPostGoldPtn[] = _T("INSERT INTO `postgoldstate` SET `regist_time` = '%s', `account` = '%s', \
											 `gold` = '%I64d', `admin` = '%s'");
		SQLTCHAR SqlInsertPostGold[500] = {0,};
		swprintf(SqlInsertPostGold, SqlInsertPostGoldPtn, szRegistTime, szAccount, ullPostGold, szAdmin);

		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlInsertPostGold,
			SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );
	}
	return nRet;
}

int InsertAccountState(LPCTSTR szAutoAccount, LPCTSTR szGameId, LPCTSTR szGamePwd, LPCTSTR szAccountState, LPCTSTR szAutoIP, int nIndex)
{
	CString szRegistTime;
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	szRegistTime.Format(L"%d-%d-%d %d:%d:%d", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
	CString szRegistDate;
	szRegistDate.Format(L"%d-%d-%d", systime.wYear, systime.wMonth, systime.wDay);

	int nRet = 0;

	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	int AccountStateId = -1;
	WCHAR szPrevAccountState[100] = {0};
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetAccountStatePtn[] = _T("SELECT `id`, `account_state` FROM accountstate WHERE `game_id` = '%s' AND `regist_time` >= '%s' ORDER BY `regist_time` DESC LIMIT 1");
		SQLTCHAR SqlGetAccountState[200] = {0,};
		_stprintf(SqlGetAccountState, SqlGetAccountStatePtn, szGameId, szRegistDate);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetAccountState,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(AccountStateId),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && AccountStateId != -1)
			{
			}

			sqlRet = SQLGetData( sql_hStmt,  // 
				2,
				SQL_C_TCHAR,
				szPrevAccountState,
				sizeof(szPrevAccountState),
				NULL );
			if(SQL_SUCCEEDED(sqlRet))
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	

		BOOL bInsert = FALSE;

		if(AccountStateId != -1) //갱신
		{
			if(wcscmp(szAccountState, szPrevAccountState) != 0) // 상태가 변경되는 경우
			{
				AccountStateId = -1; // 입력처리를 진행
				bInsert = TRUE;
			}
			else
			{
				sqlRet =
					SQLAllocHandle( SQL_HANDLE_STMT,
					g_sql_hDBC,
					&sql_hStmt );
				SQLTCHAR SqlAccountStateUpdate[1000] = {0,};
				SQLTCHAR SqlAccountStateUpdatePtn[] = _T("UPDATE `accountstate` SET `auto_account` = '%s' , `game_id` = '%s' , `account_state` = '%s' , `regist_time` = '%s', `game_pwd` = '%s', `auto_ip` = '%s' WHERE `id` = '%d'");
				_stprintf(SqlAccountStateUpdate, SqlAccountStateUpdatePtn, szAutoAccount, szGameId, szAccountState, szRegistTime, szGamePwd, szAutoIP, AccountStateId);
				sqlRet =
					SQLExecDirect( sql_hStmt,
					(SQLTCHAR*)SqlAccountStateUpdate,
					SQL_NTS );
				if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
				{
					nRet = 1;
				}
				SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
			}
		}

		if(AccountStateId == -1) //새로 입력
		{
			if(wcscmp(szAccountState, L"정상") == 0)
			{
				bInsert = TRUE;
			}
			else
			{
				sqlRet =
					SQLAllocHandle( SQL_HANDLE_STMT,
					g_sql_hDBC,
					&sql_hStmt );
				SQLTCHAR SqlGetSameAccountStatePtn[] = _T("SELECT `id` FROM accountstate WHERE `game_id` = '%s' AND `account_state` = '%s'");
				SQLTCHAR SqlGetSameAccountState[200] = {0,};
				_stprintf(SqlGetSameAccountState, SqlGetSameAccountStatePtn, szGameId, szAccountState);
				sqlRet =
					SQLExecDirect( sql_hStmt,
					(SQLTCHAR*)SqlGetSameAccountState,
					SQL_NTS );
				if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
				{

					sqlRet = SQLGetData( sql_hStmt,  // 
						1,
						SQL_INTEGER,
						&(AccountStateId),
						4,
						NULL );
					if(SQL_SUCCEEDED(sqlRet) && AccountStateId != -1)
					{
					}
				}
				if(AccountStateId == -1)
				{
					bInsert = TRUE;
					m_pCrackCheckEngine->AddBlockW(systime, g_pAccountInfo[nIndex].szAccount, 1);
				}
				SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
			}

			if(bInsert)
			{
				sqlRet =
					SQLAllocHandle( SQL_HANDLE_STMT,
					g_sql_hDBC,
					&sql_hStmt );
				SQLTCHAR SqlAccountStateInsert[1000] = {0,};
				SQLTCHAR SqlAccountStateInsertPtn[] = _T("INSERT INTO `accountstate` SET `auto_account` = '%s' , `game_id` = '%s' , `account_state` = '%s' , `regist_time` = '%s', `game_pwd` = '%s', `auto_ip` = '%s'");
				_stprintf(SqlAccountStateInsert, SqlAccountStateInsertPtn, szAutoAccount, szGameId, szAccountState, szRegistTime, szGamePwd, szAutoIP);
				sqlRet =
					SQLExecDirect( sql_hStmt,
					(SQLTCHAR*)SqlAccountStateInsert,
					SQL_NTS );
				if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
				{
					nRet = 1;
				}
				SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
			}
		}
	}
	return nRet;
}

BOOL ConnectToDBTables()
{
	OutputDebugString(_T("ConnectToDBTables"));

	FreeDBConnection();

	SQLHSTMT sql_hStmt = 0;
	SQLTCHAR szDNS[1024] ={0};
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet =
		SQLAllocHandle( SQL_HANDLE_ENV,
		SQL_NULL_HANDLE,
		&g_sql_hEnv );
	sqlRet =
		SQLSetEnvAttr( g_sql_hEnv,
		SQL_ATTR_ODBC_VERSION,
		(void*) SQL_OV_ODBC3,
		0 );
	sqlRet =
		SQLAllocHandle( SQL_HANDLE_DBC,
		g_sql_hEnv,
		&g_sql_hDBC );

	SQLTCHAR ConnectionString[MAX_PATH];
	_tcscpy(ConnectionString, _T("DSN=") SERVER_DSN _T(";"));
	sqlRet =
		SQLDriverConnect( g_sql_hDBC,
		0,
		(SQLTCHAR*)ConnectionString,
		SQL_NTS,
		szDNS,
		1024,
		&nSize,
		SQL_DRIVER_COMPLETE );
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		return TRUE;
	}
	ExitProcess(0);
	return FALSE;
}

int InitDBConnection()
{
	ConnectToDBTables();

	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = 0;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlCreateManageTbl[] = _T("CREATE TABLE `accountstate` ( \
`id` int(11) NOT NULL AUTO_INCREMENT, \
`regist_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`auto_account` varchar(100) NOT NULL DEFAULT '', \
`game_id` varchar(100) NOT NULL DEFAULT '', \
`game_pwd` varchar(100) NOT NULL DEFAULT '', \
`account_state` varchar(100)  NOT NULL DEFAULT '', \
`auto_ip` VARCHAR(50) NULL, \
PRIMARY KEY (`id`) \
) ENGINE=MyISAM  DEFAULT CHARSET=euckr COLLATE=euckr_korean_ci AUTO_INCREMENT=1 ; \
");
		sqlRet = SQLExecDirect( sql_hStmt, (SQLTCHAR*)SqlCreateManageTbl,	SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	

		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlCreateLogTbl[] = _T("\
CREATE TABLE `log` (\
`id` int(11) NOT NULL AUTO_INCREMENT,\
`auto_account` varchar(100) NOT NULL DEFAULT '',\
`log_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',\
`log_kind` int(10) NOT NULL DEFAULT '0',\
`log_count` INT(10) NOT NULL DEFAULT '0',\
`log_date1` DATE NULL,\
`log_date2` DATE NULL,\
`log_info` INT(3) NOT NULL DEFAULT '0',\
PRIMARY KEY (`id`)\
) ENGINE=MyISAM  DEFAULT CHARSET=euckr COLLATE=euckr_korean_ci AUTO_INCREMENT=1 ;\
");
		sqlRet = SQLExecDirect( sql_hStmt, (SQLTCHAR*)SqlCreateLogTbl,	SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	

		// 오토패치주소테이블
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlPatchAddressTbl[] = _T("\
CREATE TABLE `patch` (\
`id` int(11) NOT NULL AUTO_INCREMENT,\
`regist_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`address1` varchar(10) NOT NULL DEFAULT '0',\
PRIMARY KEY (`id`)\
) ENGINE=MyISAM  DEFAULT CHARSET=euckr COLLATE=euckr_korean_ci AUTO_INCREMENT=1 ;\
");
		sqlRet = SQLExecDirect( sql_hStmt, (SQLTCHAR*)SqlPatchAddressTbl,	SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	

		//중복방지 테이블
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlIPDupCheckTbl[] = _T("CREATE TABLE `ipdupcheck` ( \
`id` int(11) NOT NULL AUTO_INCREMENT, \
`regist_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`account` varchar(100) NOT NULL DEFAULT '', \
`ip` varchar(20) NOT NULL DEFAULT '', \
PRIMARY KEY (`id`) \
) ENGINE=MyISAM  DEFAULT CHARSET=euckr COLLATE=euckr_korean_ci AUTO_INCREMENT=1 ; \
");
		sqlRet = SQLExecDirect( sql_hStmt, (SQLTCHAR*)SqlIPDupCheckTbl,	SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );

		//수금골드상태통계 테이블
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlPostGoldStateTbl[] = _T("CREATE TABLE `postgoldstate` ( \
`id` int(11) NOT NULL AUTO_INCREMENT, \
`regist_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`account` varchar(100) NOT NULL DEFAULT '', \
`gold` varchar(20) NOT NULL DEFAULT '', \
`admin` varchar(100) NOT NULL DEFAULT '', \
PRIMARY KEY (`id`) \
) ENGINE=MyISAM  DEFAULT CHARSET=euckr COLLATE=euckr_korean_ci AUTO_INCREMENT=1 ; \
");
		sqlRet = SQLExecDirect( sql_hStmt, (SQLTCHAR*)SqlPostGoldStateTbl,	SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );

		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlAccountStatisticsTbl[] = _T("CREATE TABLE `accountstatistics` ( \
`id` int(11) NOT NULL AUTO_INCREMENT, \
`regist_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`account` varchar(100) NOT NULL DEFAULT '', \
`type` INT(3) NOT NULL DEFAULT '0',\
`admin` varchar(100) NOT NULL DEFAULT '', \
`maxmulti` INT(3) NOT NULL DEFAULT '0',\
`end_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
PRIMARY KEY (`id`) \
) ENGINE=MyISAM  DEFAULT CHARSET=euckr COLLATE=euckr_korean_ci AUTO_INCREMENT=1 ; \
");
		sqlRet = SQLExecDirect( sql_hStmt, (SQLTCHAR*)SqlAccountStatisticsTbl,	SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );

		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)_T("set names euckr"),
			SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	else
	{
		PrintLog(L"DB연결 실패");
		FreeDBConnection();
		ExitProcess(-1);
	}

	g_dwPatchAddress = GetPatchAddress();

	return TRUE;

}

void FreeDBConnection()
{
	if(g_sql_hDBC)
	{
		SQLDisconnect( g_sql_hDBC );
		SQLFreeHandle( SQL_HANDLE_DBC, g_sql_hDBC );
		SQLFreeHandle( SQL_HANDLE_ENV, g_sql_hEnv );
		g_sql_hDBC = NULL;
	}
}

int GetUseCountAtNow()
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullStep = 6000000000; //10분 (100 nano초 단위)
	ullStep *= 2; 
	ULONGLONG ullStart = *(ULONGLONG*)&filetime - ullStep;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szTime[100];
	_stprintf(szTime, _T("%04d-%02d-%02d %02d:%02d:%02d"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLTCHAR szDNS[1024] ={0};
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetUseCountAtNowPtn[] = _T("SELECT count(*) AS total from `log` WHERE log_time >= '%s' AND (log_kind = '10' OR log_kind = '11') AND log_count != 0");
		SQLTCHAR SqlGetUseCountAtNow[1000] = {0,};
		_stprintf(SqlGetUseCountAtNow, SqlGetUseCountAtNowPtn, szTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetUseCountAtNow,
			_tcslen(SqlGetUseCountAtNow) );

		if(!SQL_SUCCEEDED( sqlRet ))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 8: %d, %s", sqlRet, SqlGetUseCountAtNow);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	if(nRet < 0)
		nRet = 0;
	return nRet;

}

int GetCreateCountAtDay(SYSTEMTIME systime) {

	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullStart = *(ULONGLONG*)&filetime + ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLTCHAR szDNS[1024] ={0};
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetCreateCountAtDayPtn[] = _T("SELECT count(*) AS total from `log` WHERE log_time >= '%s' AND log_time < '%s' AND log_kind = '1'");
		SQLTCHAR SqlGetCreateCountAtDay[1000] = {0,};
		_stprintf(SqlGetCreateCountAtDay, SqlGetCreateCountAtDayPtn, szStartTime, szEndTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetCreateCountAtDay,
			_tcslen(SqlGetCreateCountAtDay) );

		if(!SQL_SUCCEEDED( sqlRet ))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 5: %d, %s", sqlRet, SqlGetCreateCountAtDay);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	if(nRet < 0)
		nRet = 0;
	return nRet;
}
int GetDeleteCountAtDay(SYSTEMTIME systime) {

	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullStart = *(ULONGLONG*)&filetime + ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetCreateCountAtDayPtn[] = _T("SELECT count(*) AS total from `log` WHERE log_time >= '%s' AND log_time < '%s' AND log_kind = '2'");
		SQLTCHAR SqlGetCreateCountAtDay[1000] = {0,};
		_stprintf(SqlGetCreateCountAtDay, SqlGetCreateCountAtDayPtn, szStartTime, szEndTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetCreateCountAtDay,
			_tcslen(SqlGetCreateCountAtDay) );

		if(!SQL_SUCCEEDED( sqlRet ))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 7: %d, %s", sqlRet, SqlGetCreateCountAtDay);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	if(nRet < 0)
		nRet = 0;
	return nRet;
}
int GetUseCount1AtDay(SYSTEMTIME systime) {

	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullStart = *(ULONGLONG*)&filetime + ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetCreateCountAtDayPtn[] = _T("SELECT count(*) AS total from `log` WHERE log_time >= '%s' AND log_time < '%s' AND (log_kind = '10' OR log_kind = '11') AND log_count != 0");
		SQLTCHAR SqlGetCreateCountAtDay[1000] = {0,};
		_stprintf(SqlGetCreateCountAtDay, SqlGetCreateCountAtDayPtn, szStartTime, szEndTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetCreateCountAtDay,
			_tcslen(SqlGetCreateCountAtDay) );

		if(!SQL_SUCCEEDED( sqlRet ))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 4: %d, %s", sqlRet, SqlGetCreateCountAtDay);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	if(nRet < 0)
		nRet = 0;
	return nRet;
}

int GetUseCount2AtDay(SYSTEMTIME systime) {

	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullStart = *(ULONGLONG*)&filetime + ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLTCHAR szDNS[1024] ={0};
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetCreateCountAtDayPtn[] = _T("SELECT count(*) AS total from `log` WHERE log_time >= '%s' AND log_time < '%s' AND log_kind = '11' AND log_count != 0");
		SQLTCHAR SqlGetCreateCountAtDay[1000] = {0,};
		_stprintf(SqlGetCreateCountAtDay, SqlGetCreateCountAtDayPtn, szStartTime, szEndTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetCreateCountAtDay,
			_tcslen(SqlGetCreateCountAtDay) );

		if(!SQL_SUCCEEDED( sqlRet ))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 6: %d, %s", sqlRet, SqlGetCreateCountAtDay);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	if(nRet < 0)
		nRet = 0;
	return nRet;
}

int GetUseCountAtDay(SYSTEMTIME systime) {

	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullStart = *(ULONGLONG*)&filetime + ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetCreateCountAtDayPtn[] = _T("SELECT count(*) AS total from `log` WHERE log_time >= '%s' AND log_time < '%s' AND (log_kind = '10' OR log_kind = '11') AND log_count != 0");
		SQLTCHAR SqlGetCreateCountAtDay[1000] = {0,};
		_stprintf(SqlGetCreateCountAtDay, SqlGetCreateCountAtDayPtn, szStartTime, szEndTime);
		_tcscat(SqlGetCreateCountAtDay, _T(" AND auto_account LIKE '%_1'"));
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetCreateCountAtDay,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	if(nRet < 0)
		nRet = 0;
	return nRet;
}

int GetUseTodayLog(LPTSTR lpAutoAccount, SYSTEMTIME* psystime, int *pnAuthCount)
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullStart = *(ULONGLONG*)&filetime + ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;
	WCHAR szLogTime[30] = {0};
	int nAuthCount = 0;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetUseTodayLogPtn[] = _T("SELECT id, log_time, log_count from `log` WHERE auto_account = '%s' AND log_time >= '%s' AND log_time < '%s' AND ( log_kind = '10' OR log_kind = '11' OR log_kind = '4') ORDER BY log_time DESC LIMIT 1");
		SQLTCHAR SqlGetUseTodayLog[1000] = {0,};
		_stprintf(SqlGetUseTodayLog, SqlGetUseTodayLogPtn, lpAutoAccount, szStartTime, szEndTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetUseTodayLog,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}

			sqlRet = SQLGetData( sql_hStmt,  // 
				2,
				SQL_C_TCHAR,
				szLogTime,
				sizeof(szLogTime),
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}

			sqlRet = SQLGetData( sql_hStmt,  // 
				3,
				SQL_INTEGER,
				&(nAuthCount),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	if(psystime && (wcslen(szLogTime) == 19))
	{
		GetLocalTime(psystime);
		WCHAR szDateTmp[10];
		memset(szDateTmp, 0, sizeof(szDateTmp));
		wcsncpy(szDateTmp, szLogTime, 4);
		psystime->wYear = _wtoi(szDateTmp);
		memset(szDateTmp, 0, sizeof(szDateTmp));
		wcsncpy(szDateTmp, szLogTime + 5, 2);
		psystime->wMonth = _wtoi(szDateTmp);
		memset(szDateTmp, 0, sizeof(szDateTmp));
		wcsncpy(szDateTmp, szLogTime + 8, 2);
		psystime->wDay = _wtoi(szDateTmp);
		memset(szDateTmp, 0, sizeof(szDateTmp));
		wcsncpy(szDateTmp, szLogTime + 11, 2);
		psystime->wHour = _wtoi(szDateTmp);
		memset(szDateTmp, 0, sizeof(szDateTmp));
		wcsncpy(szDateTmp, szLogTime + 14, 2);
		psystime->wMinute = _wtoi(szDateTmp);
		memset(szDateTmp, 0, sizeof(szDateTmp));
		wcsncpy(szDateTmp, szLogTime + 17, 2);
		psystime->wSecond = _wtoi(szDateTmp);
	}
	if(pnAuthCount)
		*(pnAuthCount) = nAuthCount;
	return nRet;
}

int GetUseYesterdayLog(LPTSTR lpAutoAccount) 
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullStart = *(ULONGLONG*)&filetime - ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetUseYesterdayLogPtn[] = _T("SELECT id AS total from `log` WHERE auto_account = '%s' AND log_time >= '%s' AND log_time < '%s' AND ( log_kind = '10' OR log_kind = '11') ORDER BY log_time DESC LIMIT 1");
		SQLTCHAR SqlGetUseYesterdayLog[1000] = {0,};
		_stprintf(SqlGetUseYesterdayLog, SqlGetUseYesterdayLogPtn, lpAutoAccount, szStartTime, szEndTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetUseYesterdayLog,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	return nRet;
}

void InsertTodayLog(LPTSTR lpAutoAccount, int nLogKind, LPBYTE byDate1, LPBYTE byDate2, int nDayCount, BYTE nMaxMulti)
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	TCHAR szTime[100];
	_stprintf(szTime, _T("%04d-%02d-%02d %02d:%02d:%02d"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		if(byDate1 != 0 && byDate2 != 0)
		{

			WCHAR szDate1[100];
			swprintf(szDate1, L"%04d-%02d-%02d", 
				*(short*)byDate1, byDate1[2], byDate1[3]);
			WCHAR szDate2[100];
			swprintf(szDate2, L"%04d-%02d-%02d", 
				*(short*)byDate2, byDate2[2], byDate2[3]);
			SQLTCHAR SqlInsertTodayLogPtn[] = _T("INSERT INTO `log` SET auto_account = '%s' , log_time = '%s', log_kind = '%d', log_count = '%d', log_date1 = '%s', log_date2 = '%s', log_info = '%d'");
			SQLTCHAR SqlInsertTodayLog[1000] = {0,};
			_stprintf(SqlInsertTodayLog, SqlInsertTodayLogPtn, lpAutoAccount, szTime, nLogKind, nDayCount, szDate1, szDate2, nMaxMulti);
			sqlRet =
				SQLExecDirect( sql_hStmt,
				(SQLTCHAR*)SqlInsertTodayLog,
				SQL_NTS );
		}
		else
		{
			SQLTCHAR SqlInsertTodayLogPtn[] = _T("INSERT INTO `log` SET auto_account = '%s' , log_time = '%s', log_kind = '%d', log_count = '%d'");
			SQLTCHAR SqlInsertTodayLog[1000] = {0,};
			_stprintf(SqlInsertTodayLog, SqlInsertTodayLogPtn, lpAutoAccount, szTime, nLogKind, nDayCount);
			sqlRet =
				SQLExecDirect( sql_hStmt,
				(SQLTCHAR*)SqlInsertTodayLog,
				SQL_NTS );
		}
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	return;
}

void InsertStatistics(LPTSTR lpAutoAccount, int nType, LPTSTR lpAdminName, BYTE nMaxMulti, LPBYTE byDate)
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	TCHAR szTime[100];
	_stprintf(szTime, _T("%04d-%02d-%02d %02d:%02d:%02d"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		
		SQLTCHAR SqlInsertTodayLogPtn[] = _T("INSERT INTO `accountstatistics` SET regist_time = '%s' , account = '%s', type = '%d', admin = '%s', maxmulti = '%d', end_time = '%s'");
		SQLTCHAR SqlInsertTodayLog[1000] = {0,};
		
		WCHAR szDate[100];
		swprintf(szDate, L"%04d-%02d-%02d", 
			*(short*)byDate, byDate[2], byDate[3]);

		_stprintf(SqlInsertTodayLog, SqlInsertTodayLogPtn, szTime, lpAutoAccount, nType, lpAdminName, nMaxMulti, szDate);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlInsertTodayLog,
			SQL_NTS );
		
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	return;
}

void UpdateTodayLog(int nID, SYSTEMTIME systime)
{
	TCHAR szTime[100];
	_stprintf(szTime, _T("%04d-%02d-%02d %02d:%02d:%02d"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		int nLogCount = 0;
		SQLTCHAR SqlGetLogCountPtn[] = _T("SELECT log_count from `log` WHERE id = '%d'");
		SQLTCHAR SqlGetLogCount[1000] = {0,};
		_stprintf(SqlGetLogCount, SqlGetLogCountPtn, nID);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetLogCount,
			/*SQL_NTS*/ wcslen(SqlGetLogCount));
		
		if(!SQL_SUCCEEDED(sqlRet))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 2: %d, %s", wcslen(SqlGetLogCount), SqlGetLogCount);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nLogCount),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nLogCount != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	

		nLogCount ++;
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlUpdateTodayLogPtn[] = _T("UPDATE `log` SET log_time = '%s', log_count = '%d' WHERE id = '%d'");
		SQLTCHAR SqlUpdateTodayLog[1000] = {0,};
		_stprintf(SqlUpdateTodayLog, SqlUpdateTodayLogPtn, szTime, nLogCount, nID);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlUpdateTodayLog,
			/*SQL_NTS*/ wcslen(SqlUpdateTodayLog));

		if(!SQL_SUCCEEDED(sqlRet))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 3: %d, %s", wcslen(SqlUpdateTodayLog), SqlUpdateTodayLog);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	return;
}

int InsertUseTodayLog(LPTSTR lpAutoAccont, int nIndex)
{
	SYSTEMTIME systime1;
	GetLocalTime(&systime1);
	SYSTEMTIME systime2;
	systime2 = systime1;

	int nAuthCount = 0;
	int nID = GetUseTodayLog(lpAutoAccont, &systime1, &nAuthCount);
	m_pCrackCheckEngine->AddUseTimeW(systime1, g_pAccountInfo[nIndex].szAccount, MULTI_COUNT, 15);

	if(nAuthCount >= 24 * 60 / 15 + 10) // 24시간에 15분간격으로 인증, 여유 10
	{
		m_pCrackCheckEngine->AbnormalUseTimeW(g_pAccountInfo[nIndex].szAccount, nAuthCount * 15);
	}

	ULONGLONG filetimeStart1;
	SystemTimeToFileTime(&systime1, (LPFILETIME)&filetimeStart1);
	ULONGLONG filetimeStart2;
	SystemTimeToFileTime(&systime2, (LPFILETIME)&filetimeStart2);

	if(nID == -1)
	{
		/*nID = GetUseYesterdayLog(lpAutoAccont);
		if(nID == -1) // 어제 로그 없으면
		{
			InsertTodayLog(lpAutoAccont, 10);	
		}
		else*/
		{
			InsertTodayLog(lpAutoAccont, 11);	
		}	
	}
	else
	{	
		UpdateTodayLog(nID, systime2);

		// 2분 40초 - 160초
		ULONGLONG ullAuthStepTick = 1600000000;
		if((filetimeStart2 - filetimeStart1) < ullAuthStepTick)
		{
			return 0; // 중복실행
		}
	}
	return 1;
}

int GetValidateCountAtNow()
{
	int nRet = 0;
	// 사용가능상태인가 본다.
	DWORD		dwCur, dwExp, dwReg;
	SYSTEMTIME	time;
	GetLocalTime(&time);
	EnterCriticalSection(&g_CR);
	for(int i = 0; i < g_nAccountNum; i ++)
	{
		dwExp = *(short *)g_pAccountInfo[i].pbExpDate;
		dwExp <<= 16;
		dwExp |= g_pAccountInfo[i].pbExpDate[2]<<8;
		dwExp |= g_pAccountInfo[i].pbExpDate[3];
		dwReg = *(short *)g_pAccountInfo[i].pbRegDate;
		dwReg <<= 16;
		dwReg |= g_pAccountInfo[i].pbRegDate[2]<<8;
		dwReg |= g_pAccountInfo[i].pbRegDate[3];
		dwCur = (time.wYear<<16)|(time.wMonth<<8)|time.wDay;
		if(dwCur <= dwExp && dwCur >= dwReg)
		{
			nRet += MULTI_COUNT;
		}
	}
	LeaveCriticalSection(&g_CR);
	return nRet;
}

int RegistAutoIP(LPWSTR szAutoAccount, LPWSTR szIP)
{
	int nRet = 0;

	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		WCHAR szLastRegistIme[20] = {0};
		WCHAR szLastIP[20] = {0};
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetAutoIPRegistTimePtn[] = _T("SELECT `regist_time`, `ip` from `ipdupcheck` WHERE `account` = '%s' LIMIT 1");
		SQLTCHAR SqlGetAutoIPRegistTime[1000] = {0,};
		_stprintf(SqlGetAutoIPRegistTime, SqlGetAutoIPRegistTimePtn, szAutoAccount);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetAutoIPRegistTime,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{			
			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_C_TCHAR,
				szLastRegistIme,
				sizeof(szLastRegistIme),
				NULL );
			sqlRet = SQLGetData( sql_hStmt,  // 
				2,
				SQL_C_TCHAR,
				szLastIP,
				sizeof(szLastIP),
				NULL );
		}
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	

		CString szRegistTime;
		SYSTEMTIME systimeRegist;
		GetLocalTime(&systimeRegist);
		szRegistTime.Format(L"%d-%d-%d %d:%d:%d", systimeRegist.wYear, systimeRegist.wMonth, systimeRegist.wDay, systimeRegist.wHour, systimeRegist.wMinute, systimeRegist.wSecond);
		ULONGLONG filetimeRegist;
		SystemTimeToFileTime(&systimeRegist, (LPFILETIME)&filetimeRegist);

		int nRegistMode = 0;
		if(wcslen(szLastRegistIme) != 19)
		{
			nRegistMode = 1; // 추가
		}
		else
		{
			if(wcscmp(szLastIP, szIP) == 0)
			{
				nRegistMode = 2; // 갱신
			}
			else
			{
				SYSTEMTIME systimeLastRegist;
				GetLocalTime(&systimeLastRegist);
				WCHAR szDateTmp[10];
				memset(szDateTmp, 0, sizeof(szDateTmp));
				wcsncpy(szDateTmp, szLastRegistIme, 4);
				systimeLastRegist.wYear = _wtoi(szDateTmp);
				memset(szDateTmp, 0, sizeof(szDateTmp));
				wcsncpy(szDateTmp, szLastRegistIme + 5, 2);
				systimeLastRegist.wMonth = _wtoi(szDateTmp);
				memset(szDateTmp, 0, sizeof(szDateTmp));
				wcsncpy(szDateTmp, szLastRegistIme + 8, 2);
				systimeLastRegist.wDay = _wtoi(szDateTmp);
				memset(szDateTmp, 0, sizeof(szDateTmp));
				wcsncpy(szDateTmp, szLastRegistIme + 11, 2);
				systimeLastRegist.wHour = _wtoi(szDateTmp);
				memset(szDateTmp, 0, sizeof(szDateTmp));
				wcsncpy(szDateTmp, szLastRegistIme + 14, 2);
				systimeLastRegist.wMinute = _wtoi(szDateTmp);
				memset(szDateTmp, 0, sizeof(szDateTmp));
				wcsncpy(szDateTmp, szLastRegistIme + 17, 2);
				systimeLastRegist.wSecond= _wtoi(szDateTmp);

				ULONGLONG filetimeLastRegist;
				SystemTimeToFileTime(&systimeLastRegist, (LPFILETIME)&filetimeLastRegist);

				// 3분 - 180초
				ULONGLONG ullAuthStepTick = 1800000000;
				if((filetimeRegist - filetimeLastRegist) > ullAuthStepTick)
				{
					nRegistMode = 2; // 갱신
				}	
			}		
		}

		if(nRegistMode == 1) // 추가
		{
			sqlRet =
				SQLAllocHandle( SQL_HANDLE_STMT,
				g_sql_hDBC,
				&sql_hStmt );

			SQLTCHAR SqlAutoIPRegist[2000] = {0,};
			SQLTCHAR SqlAutoIPRegistPtn[] = _T("INSERT INTO `ipdupcheck` SET `regist_time` = '%s', `account` = '%s' , `ip` = '%s'");
			_stprintf(SqlAutoIPRegist, SqlAutoIPRegistPtn, szRegistTime, szAutoAccount, szIP);
			sqlRet =
				SQLExecDirect( sql_hStmt,
				(SQLTCHAR*)SqlAutoIPRegist,
				SQL_NTS );
			if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
			{
				nRet = 1;
			}
			SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
		}
		else if(nRegistMode == 2) // 갱신
		{
			sqlRet =
				SQLAllocHandle( SQL_HANDLE_STMT,
				g_sql_hDBC,
				&sql_hStmt );

			SQLTCHAR SqlAutoIPRegist[2000] = {0,};
			SQLTCHAR SqlAutoIPRegistPtn[] = _T("UPDATE `ipdupcheck` SET `regist_time` = '%s', `ip` = '%s' WHERE `account` = '%s'");
			_stprintf(SqlAutoIPRegist, SqlAutoIPRegistPtn, szRegistTime, szIP, szAutoAccount);
			sqlRet =
				SQLExecDirect( sql_hStmt,
				(SQLTCHAR*)SqlAutoIPRegist,
				SQL_NTS );
			if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
			{
				nRet = 1;
			}
			SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
		}
	}
	return nRet;
}

BOOL CheckAutoIP(LPWSTR szAutoAccount, LPWSTR szIP) // TRUE: 정상, FALSE: 중복
{
	BOOL bRet = TRUE;

	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		WCHAR szLastIP[20] = {0};
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetAutoIPRegistTimePtn[] = _T("SELECT `ip` from `ipdupcheck` WHERE `account` = '%s' LIMIT 1");
		SQLTCHAR SqlGetAutoIPRegistTime[1000] = {0,};
		_stprintf(SqlGetAutoIPRegistTime, SqlGetAutoIPRegistTimePtn, szAutoAccount);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetAutoIPRegistTime,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{			
			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_C_TCHAR,
				szLastIP,
				sizeof(szLastIP),
				NULL );
		}
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	

		if(wcslen(szLastIP) == 0)
		{
			RegistAutoIP(szAutoAccount, szIP);
		}
		else
		{
			if(wcscmp(szIP, szLastIP) != 0)
			{
				bRet = FALSE;
			}
		}
	}
	return bRet;
}

int GetAccountCountAtDay(SYSTEMTIME systime, LPCTSTR szAccountState)
{
	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullStart = *(ULONGLONG*)&filetime + ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetAccountCountAtDayPtn[] = _T("SELECT count(*) AS total from `accountstate` WHERE regist_time >= '%s' AND regist_time < '%s' AND account_state = '%s'");
		SQLTCHAR SqlGetAccountCountAtDay[1000] = {0,};
		_stprintf(SqlGetAccountCountAtDay, SqlGetAccountCountAtDayPtn, szStartTime, szEndTime, szAccountState);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetAccountCountAtDay,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	if(nRet < 0)
		nRet = 0;
	return nRet;
}

void InsertPatchAddress(DWORD dwAddress)
{
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	TCHAR szTime[100];
	_stprintf(szTime, _T("%04d-%02d-%02d %02d:%02d:%02d"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlEmptyTable[] = _T("DELETE FROM `patch`");
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlEmptyTable,
			SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	

		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlInsertPatchAddressPtn[] = _T("INSERT INTO `patch` SET Address1 = '%x', regist_time = '%s'");
		SQLTCHAR SqlInsertPatchAddress[1000] = {0,};
		_stprintf(SqlInsertPatchAddress, SqlInsertPatchAddressPtn, dwAddress, szTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlInsertPatchAddress,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	return;
}

DWORD GetPatchAddress() 
{
	WCHAR szAddress[10] = L"0";
	int nRet = -1;
	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetPatchAddress[] = _T("SELECT address1 from `patch` ORDER BY id DESC LIMIT 1");
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetPatchAddress,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_C_TCHAR,
				szAddress,
				20,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	LPTSTR endptr;
	DWORD dwAddress = _tcstol(szAddress, &endptr, 16);
	return dwAddress;
}

int GetMonthUserCount(SYSTEMTIME systime, int nDays)
{
	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, 1);
	if(systime.wMonth == 12)
	{
		systime.wMonth = 1;
		systime.wYear += 1;
	}
	else
	{
		systime.wMonth += 1;
	}
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, 1);
	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlMonthUserCountPtn[] = _T("SELECT count(*) FROM (SELECT auto_account, count(*) as cnt FROM log WHERE log_time >= '%s' && log_time < '%s' && (log_kind = '10' || log_kind = '11') GROUP BY auto_account) AS tmplog WHERE cnt >= '%d'");
		SQLTCHAR SqlMonthUserCount[1000] = {0,};
		_stprintf(SqlMonthUserCount, SqlMonthUserCountPtn, szStartTime, szEndTime, nDays);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlMonthUserCount,
			_tcslen(SqlMonthUserCount) );

		if(!SQL_SUCCEEDED( sqlRet ))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 9: %d, %s", sqlRet, SqlMonthUserCount);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	if(nRet < 0)
		nRet = 0;
	return nRet;
}

int GetMonthUserCreateCount(SYSTEMTIME systime)
{
	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, 1);
	if(systime.wMonth == 12)
	{
		systime.wMonth = 1;
		systime.wYear += 1;
	}
	else
	{
		systime.wMonth += 1;
	}
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, 1);
	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlMonthUserCreateCountPtn[] = _T("SELECT count(*) as cnt FROM log WHERE log_time >= '%s' && log_time < '%s' && log_kind = 1");
		SQLTCHAR SqlMonthUserCreateCount[1000] = {0,};
		_stprintf(SqlMonthUserCreateCount, SqlMonthUserCreateCountPtn, szStartTime, szEndTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlMonthUserCreateCount,
			_tcslen(SqlMonthUserCreateCount) );

		if(!SQL_SUCCEEDED( sqlRet ))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 10: %d, %s", sqlRet, SqlMonthUserCreateCount);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	if(nRet < 0)
		nRet = 0;
	return nRet;
}
int GetMonthUserDeleteCount(SYSTEMTIME systime)
{
	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, 1);
	if(systime.wMonth == 12)
	{
		systime.wMonth = 1;
		systime.wYear += 1;
	}
	else
	{
		systime.wMonth += 1;
	}
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime.wYear, systime.wMonth, 1);
	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlMonthUserDeleteCountPtn[] = _T("SELECT count(*) as cnt FROM `log` WHERE log_time >= '%s' && log_time < '%s' && log_kind = '2'");
		SQLTCHAR SqlMonthUserDeleteCount[1000] = {0,};
		_stprintf(SqlMonthUserDeleteCount, SqlMonthUserDeleteCountPtn, szStartTime, szEndTime);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlMonthUserDeleteCount,
			_tcslen(SqlMonthUserDeleteCount) );

		if(!SQL_SUCCEEDED( sqlRet ))
		{
			WCHAR szReport[100];
			swprintf(szReport, L"오류발생 11: %d, %s", sqlRet, SqlMonthUserDeleteCount);
			OutputDebugStringW(szReport);
			ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT);
		}

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	if(nRet < 0)
		nRet = 0;
	return nRet;
}

int GetAccountAverageConnectTime(SYSTEMTIME systime) 
{
	int nAvgTime = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLTCHAR szDNS[1024] ={0};
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		TCHAR szDate[100];
		_stprintf(szDate, _T("%04d-%02d-%02d"), systime.wYear, systime.wMonth, systime.wDay);

		SQLTCHAR GetAccountUpdateHistoryPtn[] = _T("SELECT AVG(log_count) * 5 FROM log WHERE DATE(log_time) = '%s' AND log_kind >= '10' AND log_count != '0'");
		SQLTCHAR GetAccountUpdateHistory[1000] = {0,};
		_stprintf(GetAccountUpdateHistory, GetAccountUpdateHistoryPtn, szDate);

		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)GetAccountUpdateHistory,
			SQL_NTS );

		int nCounts;
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
			sqlRet = SQLGetData( sql_hStmt,
				1,
				SQL_INTEGER,
				&(nCounts),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet))
			{
				nAvgTime = nCounts;
			}
			else
			{
			}
		}
		else
		{
		}
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	else
	{
	}

	return nAvgTime;
}

int GetUserLoginTryCount(LPTSTR lpAutoAccount, SYSTEMTIME systime, int *pnTryCount) 
{
	TCHAR szDate[100];
	_stprintf(szDate, _T("%04d-%02d-%02d"), systime.wYear, systime.wMonth, systime.wDay);
	int nRet = -1;
	int nTryCount = 0;
	*pnTryCount = 0;

	SQLHSTMT sql_hStmt = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlUserLoginTryCountPtn[] = _T("SELECT id, log_count from `log` WHERE auto_account = '%s' AND DATE(log_time) = '%s' AND log_kind = '5' LIMIT 1");
		SQLTCHAR SqlUserLoginTryCount[1000] = {0,};
		_stprintf(SqlUserLoginTryCount, SqlUserLoginTryCountPtn, lpAutoAccount, szDate);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlUserLoginTryCount,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}

			sqlRet = SQLGetData( sql_hStmt,  // 
				2,
				SQL_INTEGER,
				&(nTryCount),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	*pnTryCount = nTryCount;
	return nRet;
}

int GetValidateUserCountAtDay(SYSTEMTIME systime)
{
	TCHAR szDate[100];
	_stprintf(szDate, _T("%04d-%02d-%02d"), systime.wYear, systime.wMonth, systime.wDay);

	int nRet = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetValidateAccountCountAtDayPtn[] = _T("SELECT log_count FROM log WHERE DATE(log_time) = '%s' AND log_kind = '4' LIMIT 1");
		SQLTCHAR SqlGetValidateAccountCountAtDay[1000] = {0,};
		_stprintf(SqlGetValidateAccountCountAtDay, SqlGetValidateAccountCountAtDayPtn, szDate);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetValidateAccountCountAtDay,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nRet),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nRet != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	if(nRet < 0)
		nRet = 0;
	return nRet / MULTI_COUNT;
}



LPBYTE GetMultiCount_DaybyDay(SYSTEMTIME systime, int* pLen) 
{
	int nAvgTime = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLTCHAR szDNS[1024] ={0};
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;
	int nLen = 0;

	LPBYTE pBuffer = new BYTE[0x20000];
	LPBYTE pTemp = pBuffer;
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		TCHAR szDate[100];
		_stprintf(szDate, _T("%04d-%02d-%02d 23:59"), systime.wYear, systime.wMonth, systime.wDay);

		SQLTCHAR szsqlFormat[] = _T("SELECT account,maxmulti FROM (SELECT * FROM accountstatistics WHERE regist_time <= '%s' AND end_time >= '%s' AND type <> 2 ORDER BY regist_time DESC) AS tmp_table GROUP BY account");
		SQLTCHAR szsql[1000] = {0,};
		_stprintf(szsql, szsqlFormat, szDate, szDate);

		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)szsql,
			SQL_NTS );

		SQLRETURN ret = SQLFetch(sql_hStmt);
		while( SQL_SUCCEEDED(ret) || ret == SQL_SUCCESS_WITH_INFO)
		{
			SQLLEN nTemp;
			CHAR szID[100];
			sqlRet = SQLGetData( sql_hStmt,
				1,
				SQL_C_CHAR,
				szID,
				100,
				&nTemp );

			int nMulti;
			sqlRet = SQLGetData( sql_hStmt,
				2,
				SQL_INTEGER,
				&nMulti,
				4,
				NULL );

			strcpy((char*)pTemp, (char*)szID);								pTemp += strlen(szID) + 1;
			*pTemp = (BYTE)nMulti;									pTemp += 1;				
			nLen += strlen(szID) + 1 + 1;
			ret = SQLFetch(sql_hStmt); 
		}
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}
	else
	{
	}
	if(pLen)
		*pLen = nLen;
	return pBuffer;
}
