#include "stdafx.h"
#include "AdminServer.h"
#include "ServerEngine.h"
#include "CrackCheckEngine.h"
#include "Account.h"
#include "Patch.h"
#include "AnglerState.h"
#include <io.h>

SOCKET	g_sockServerAdminQuery;
SOCKET	g_sockServerForAdminSecure;

extern	CCrackCheckEngine* m_pCrackCheckEngine;


DWORD	ServerThreadAdminQuery(LPVOID pParam)
{
	Sleep(5000);
	sockaddr_in		addrServer;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(SERVER_PORT_ADMINQUERY);
	g_sockServerAdminQuery = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_sockServerAdminQuery == INVALID_SOCKET)
		return 0;
	for(;;)
	{
		if(bind(g_sockServerAdminQuery, (sockaddr*)&addrServer, sizeof(addrServer)))
		{
			Sleep(500);
			continue;
		}
		break;
	}
	if(listen(g_sockServerAdminQuery, SOMAXCONN))
		return 0;

	sockaddr_in		addrFrom;
	int				nFromLen;

	nFromLen = sizeof(addrFrom);
	while(1)
	{
		SOCKET		*pSocketComm = new SOCKET;

		if(g_bStopServer == 1)
			break;

		*pSocketComm = accept(g_sockServerAdminQuery, (struct sockaddr*)&addrFrom, &nFromLen);
		if(*pSocketComm == INVALID_SOCKET)
		{
			PrintLog(L"망오류: 련결요청을 받아 들일수 없습니다.");
			FreeDBConnection();
			ExitProcess(0);
			return 0;
		}
		PrintLog(L"[Query] IP %d.%d.%d.%d: - Request", addrFrom.sin_addr.S_un.S_un_b.s_b1, 
			addrFrom.sin_addr.S_un.S_un_b.s_b2,
			addrFrom.sin_addr.S_un.S_un_b.s_b3,
			addrFrom.sin_addr.S_un.S_un_b.s_b4
			);

		HANDLE han = CreateThread(NULL, 10 * 1024, (LPTHREAD_START_ROUTINE)CommThreadAdminQuery, (LPVOID)pSocketComm, NULL, NULL);
		if(han)
		{
			CloseHandle(han);
		}
		else
		{
			PrintLog(L"망오류: 스레드를 생성할수 없습니다.");
			FreeDBConnection();
			ExitProcess(0);
			return 0;
		}
	}

	return 0;
}


DWORD	ServerThreadForAdminSecure(LPVOID pParam)
{
	Sleep(5000);
	sockaddr_in		addrServer;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(SERVER_PORT_ADMINSECURE);
	g_sockServerForAdminSecure = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_sockServerForAdminSecure == INVALID_SOCKET)
		return 0;
	for(;;)
	{
		if(bind(g_sockServerForAdminSecure, (sockaddr*)&addrServer, sizeof(addrServer)))
		{
			Sleep(500);
			continue;
		}
		break;
	}
	if(listen(g_sockServerForAdminSecure, SOMAXCONN))
		return 0;

	sockaddr_in		addrFrom;
	int				nFromLen;

	nFromLen = sizeof(addrFrom);
	while(1)
	{
		SOCKET		*pSocketComm = new SOCKET;

		if(g_bStopServer == 1)
			break;

		*pSocketComm = accept(g_sockServerForAdminSecure, (struct sockaddr*)&addrFrom, &nFromLen);
		if(*pSocketComm == INVALID_SOCKET)
		{
			PrintLog(L"망오류: 련결요청을 받아 들일수 없습니다.");
			FreeDBConnection();
			ExitProcess(0);
			return 0;
		}
		PrintLog(L"[Admin] IP %d.%d.%d.%d: - Request", addrFrom.sin_addr.S_un.S_un_b.s_b1, 
			addrFrom.sin_addr.S_un.S_un_b.s_b2,
			addrFrom.sin_addr.S_un.S_un_b.s_b3,
			addrFrom.sin_addr.S_un.S_un_b.s_b4
			);

		HANDLE han = CreateThread(NULL, 10 * 1024, (LPTHREAD_START_ROUTINE)CommThreadForAdminSecure, (LPVOID)pSocketComm, NULL, NULL);
		if(han)
		{
			CloseHandle(han);
		}
		else
		{
			PrintLog(L"망오류: 스레드를 생성할수 없습니다.");
			FreeDBConnection();
			ExitProcess(0);
			return 0;
		}
	}

	return 0;
}

DWORD  CommThreadAdminQuery(LPVOID pParam)
{
	SOCKET	*pSockComm = (SOCKET *)pParam;
	BYTE	bOpCode;
	BYTE	*pbBufferRecv = NULL, *pbBufferSend = NULL;
	int		nRecvLen, nSendLen, nPacketLen, nTemp;

	nRecvLen = recv(*pSockComm, (char *)&bOpCode, 1, 0);
	if(nRecvLen <= 0)
	{
		PrintLog(L"망오류: Recv Error0: %x", *pSockComm);
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pSockComm;
		return 0;
	}
	nTemp = 0;
	while(nTemp < 4)
	{
		nRecvLen = recv(*pSockComm, ((char *)&nPacketLen)+nTemp, 4-nTemp, 0);
		if(nRecvLen <= 0)
		{
			PrintLog(L"망오류: Recv Error1: %x", *pSockComm);
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pSockComm;
			return 0;
		}
		nTemp += nRecvLen;
	}
	if(nPacketLen > 10000000 || nPacketLen < 0)
	{
		PrintLog(L"망오류: Packet Length - Unknown");
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pSockComm;
		return 0;
	}

	pbBufferRecv = NULL;
	if(nPacketLen)
	{
		pbBufferRecv = new BYTE[nPacketLen];
	}
	nTemp = 0;
	while(nTemp < nPacketLen)
	{
		nRecvLen = recv(*pSockComm, (char *)pbBufferRecv+nTemp, nPacketLen-nTemp, 0);
		if(nRecvLen <= 0)
		{
			PrintLog(L"망오류: Recv Error2: %x", *pSockComm);
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pSockComm;
			if(pbBufferRecv)
				delete [] pbBufferRecv;
			return 0;
		}
		nTemp += nRecvLen;
	}

	PrintLog(L"망접속 성공: %x", *pSockComm);

	nRecvLen = nPacketLen;
	EncryptPacket(pbBufferRecv, nRecvLen);
	switch(bOpCode)
	{		
	case OPCODE_USERQUERY:
		{
			if(nRecvLen == 16)
			{
				ULONGLONG ullStart;
				ULONGLONG ullEnd;
				memcpy(&ullStart, pbBufferRecv, 8);
				memcpy(&ullEnd, pbBufferRecv+8, 8);
				ULONGLONG ullOneDay = 864000000000;
				ULONGLONG ullDayCount = (ullEnd - ullStart)/ullOneDay + 1;
				int nDayCount = (int)ullDayCount;

				LPBYTE lpSendBuff = new BYTE[sizeof(int) * ( nDayCount * 4 + 6) + 100];
				int* pTbnUserCount = new int[nDayCount * 4 + 6];
				SYSTEMTIME systime;

				EnterCriticalSection(&g_CR);

				for(int i = 0; i < nDayCount; i++)
				{
					FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
					ullStart += 864000000000;

					if(systime.wYear*366 + systime.wMonth*31 + systime.wDay <= 2011*366+8*31+3) // 2011년 8월 3일 이전 조회
					{
						TCHAR szTmp[MAX_PATH], szPath[MAX_PATH];
						GetSystemDirectory(szTmp, MAX_PATH);
						_stprintf(szPath, _T("%s\\user32.log"), szTmp);
						TCHAR szKeyName[100];
						_stprintf(szKeyName, _T("%04d:%02d:%02d"), systime.wYear, systime.wMonth, systime.wDay);
						pTbnUserCount[4*i] = GetPrivateProfileInt(L"user", szKeyName, 0, szPath);
						pTbnUserCount[4*i+1] = 0;
						pTbnUserCount[4*i+2] = 0;
						pTbnUserCount[4*i+3] = 0;
					}
					else
					{
						pTbnUserCount[4*i] = GetUseCount1AtDay(systime);
						pTbnUserCount[4*i+1] = GetCreateCountAtDay(systime) * MULTI_COUNT;
						pTbnUserCount[4*i+2] = GetUseCount2AtDay(systime);
						pTbnUserCount[4*i+3] = GetDeleteCountAtDay(systime) * MULTI_COUNT;
					}
				}

				pTbnUserCount[nDayCount*4] = GetValidateCountAtNow();
				pTbnUserCount[nDayCount*4+1] = GetUseCountAtNow();
				pTbnUserCount[nDayCount*4+2] = GetMonthUserCount(systime, 1) * MULTI_COUNT;
				pTbnUserCount[nDayCount*4+3] = GetMonthUserCount(systime, 15) * MULTI_COUNT;
				pTbnUserCount[nDayCount*4+4] = GetMonthUserCreateCount(systime) * MULTI_COUNT;
				pTbnUserCount[nDayCount*4+5] = GetMonthUserDeleteCount(systime) * MULTI_COUNT;

				LeaveCriticalSection(&g_CR);

				nSendLen = MakePacket(lpSendBuff, "cddb", OPCODE_USERQUERY, 0, nDayCount, pTbnUserCount, (nDayCount * 4 + 6)*sizeof(int));
				send(*pSockComm, (char *)lpSendBuff, nSendLen, 0);


				delete lpSendBuff;
				delete pTbnUserCount;

			}
			break;
		}
	case OPCODE_ACCOUNTQUERY:
		{
			if(nRecvLen == 16)
			{
				ULONGLONG ullStart;
				ULONGLONG ullEnd;
				memcpy(&ullStart, pbBufferRecv, 8);
				memcpy(&ullEnd, pbBufferRecv+8, 8);
				ULONGLONG ullOneDay = 864000000000;
				ULONGLONG ullDayCount = (ullEnd - ullStart)/ullOneDay + 1;
				int nDayCount = (int)ullDayCount;

				LPBYTE lpSendBuff = new BYTE[sizeof(int) * ( nDayCount * 6 ) + 100];
				int* pTbnUserCount = new int[nDayCount*6];

				EnterCriticalSection(&g_CR);

				for(int i = 0; i < nDayCount; i++)
				{
					SYSTEMTIME systime;
					FileTimeToSystemTime((LPFILETIME)&ullStart, &systime);
					ullStart += 864000000000;
					pTbnUserCount[6*i] = GetAccountCountAtDay(systime, L"정상");
					pTbnUserCount[6*i+1] = GetAccountCountAtDay(systime, L"영구정지1");
					pTbnUserCount[6*i+2] = GetAccountCountAtDay(systime, L"본인인증");
					pTbnUserCount[6*i+3] = GetAccountCountAtDay(systime, L"보호정지");
					pTbnUserCount[6*i+4] = GetAccountCountAtDay(systime, L"거래제한");
					pTbnUserCount[6*i+5] = GetAccountCountAtDay(systime, L"기동실패");
				}
				LeaveCriticalSection(&g_CR);

				nSendLen = MakePacket(lpSendBuff, "cddb", OPCODE_ACCOUNTQUERY, 0, nDayCount, pTbnUserCount, (nDayCount*6)*4);
				send(*pSockComm, (char *)lpSendBuff, nSendLen, 0);


				delete lpSendBuff;
				delete pTbnUserCount;

			}
			break;
		}
	}

	if(pbBufferRecv)
		delete [] pbBufferRecv;

	WaitForRecvClosePacketAndCloseSocket(pSockComm);
	return 1;
}

int InitAccountFile()
{
	WCHAR	szFileName[MAX_PATH];	
	wsprintf(szFileName, L"%s\\*.ini",g_szAccountPath);

	WIN32_FIND_DATA FindData;
	HANDLE hFindFile = FindFirstFile(szFileName, &FindData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do 
		{
			if (wcscmp(FindData.cFileName, L".") != 0
				&& wcscmp(FindData.cFileName, L"..") != 0)
			{
				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
				}
				else
				{
					wsprintf(szFileName, L"%s\\%s", g_szAccountPath, FindData.cFileName);
					SetFileAttributes(szFileName, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szFileName);
				}
			}
		} while (FindNextFile(hFindFile, &FindData));
		FindClose(hFindFile);
	}
	return 0;
}

int SendPacket(SOCKET sock, char *buf, int len, int flags)
{
	int nWriteBytes = 0;
	while (nWriteBytes < len)
	{
		int nSend = send(sock, buf + nWriteBytes, min(5000, len - nWriteBytes), flags);
		if (nSend <= 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				break;
		}
		else
		{
			nWriteBytes += nSend;
		}
		Sleep(100);
	}
	return nWriteBytes;
}

DWORD  CommThreadForAdminSecure(LPVOID pParam)
{
	SOCKET	*pSockComm = (SOCKET *)pParam;
	BYTE	bOpCode;
	BYTE	*pbBufferRecv = NULL, *pbBufferSend = NULL;
	BOOL	bLogin = FALSE;
	int		nRecvLen, nSendLen, nPacketLen, nTemp;
	WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};
	ADMININFO	AdminInfo;
	ZeroMemory(&AdminInfo, sizeof(ADMININFO));

	nRecvLen = recv(*pSockComm, (char *)&bOpCode, 1, 0);
	if(nRecvLen <= 0)
	{
		PrintLog(L"망오류: Recv Error0: %x", *pSockComm);
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pSockComm;
		return 0;
	}
	nTemp = 0;
	while(nTemp < 4)
	{
		nRecvLen = recv(*pSockComm, ((char *)&nPacketLen)+nTemp, 4-nTemp, 0);
		if(nRecvLen <= 0)
		{
			PrintLog(L"망오류: Recv Error1: %x", *pSockComm);
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pSockComm;
			return 0;
		}
		nTemp += nRecvLen;
	}
	if(nPacketLen > 10000000 || nPacketLen < 0)
	{
		PrintLog(L"망오류: Packet Length - Unknown");
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pSockComm;
		return 0;
	}

	pbBufferRecv = NULL;
	if(nPacketLen)
	{
		pbBufferRecv = new BYTE[nPacketLen];
	}
	nTemp = 0;
	while(nTemp < nPacketLen)
	{
		nRecvLen = recv(*pSockComm, (char *)pbBufferRecv+nTemp, nPacketLen-nTemp, 0);
		if(nRecvLen <= 0)
		{
			PrintLog(L"망오류: Recv Error2: %x", *pSockComm);
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pSockComm;
			if(pbBufferRecv)
				delete [] pbBufferRecv;
			return 0;
		}
		nTemp += nRecvLen;
	}

	PrintLog(L"망접속 성공: %x", *pSockComm);

	nRecvLen = nPacketLen;
	DecryptPacketAdmin(pbBufferRecv, nRecvLen);
	switch(bOpCode)
	{
	case OPCODE_BOSSCONN:
		{
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};
			BYTE TbbySendBuff[(MAX_NOTICE_LEGNTH + 100) * 2 + 100] = {0,};

			int nPacketReadPos = 0;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if( bAdminSuccess) // 자료전송
			{
				if(AdminInfo.dwLevel == ADMINLEVEL_TOP)	// 최고관리자인 경우
				{
					pbBufferSend = new BYTE[sizeof(ADMININFO)*g_nAdminNum+1000];
					nSendLen = MakePacketAdmin(pbBufferSend, "cddb", OPCODE_BOSSCONN, 0, g_nAdminNum, g_pAdminInfo, sizeof(ADMININFO)*g_nAdminNum);
					LeaveCriticalSection(&g_CR);

					send(*pSockComm, (char *)pbBufferSend, nSendLen, 0);

					delete [] pbBufferSend;
					PrintLog(L"%s: OPCODE_BOSSCONN Ok", szAdminID);
				}
				else
				{						
					LeaveCriticalSection(&g_CR);
					PrintLog(L"%s: Abnormal OPCODE_BOSSCONN Try", szAdminID);
				}
			}
			else
			{
				LeaveCriticalSection(&g_CR);
				//관리자 접속실패
				PrintLog(L"OPCODE_BOSSCONN Failed");
				BYTE TbbySendBuff[100] = {0,};
				nSendLen = MakePacketAdmin(TbbySendBuff, "cdd", OPCODE_ADMINCONN, 0, 0xFFFFFFFF);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
			}
		}
		break;
	case OPCODE_ADMINCONN:	// AdminId || AdminPwd
		{
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};
			BYTE TbbySendBuff[(MAX_NOTICE_LEGNTH + 100) * 2 + 100] = {0,};

			int nPacketReadPos = 0;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if( bAdminSuccess) // 자료전송
			{
				DWORD dwOpcode = OPCODE_ADMINCONN;
				if(AdminInfo.dwLevel == ADMINLEVEL_TOP)	// 최고관리자인 경우
				{
					dwOpcode = OPCODE_BOSSCONN;
				}
				pbBufferSend = new BYTE[sizeof(ACCOUNTINFO)*g_nAccountNum+1000];

				if(g_nAccountNum)
					nSendLen = MakePacketAdmin(pbBufferSend, "cddbd", dwOpcode, 0, g_nAccountNum, g_pAccountInfo, sizeof(ACCOUNTINFO)*g_nAccountNum, g_dwPatchAddress);
				else
					nSendLen = MakePacketAdmin(pbBufferSend, "cddd", dwOpcode, 0, g_nAccountNum, g_dwPatchAddress);

				LeaveCriticalSection(&g_CR);

				send(*pSockComm, (char *)pbBufferSend, nSendLen, 0);

				delete [] pbBufferSend;
				PrintLog(L"%s: OPCODE_ADMINCONN Ok", szAdminID);
			}
			else
			{
				LeaveCriticalSection(&g_CR);
				//관리자 접속실패
				PrintLog(L"OPCODE_ADMINCONN Failed");
				BYTE TbbySendBuff[100] = {0,};
				nSendLen = MakePacketAdmin(TbbySendBuff, "cdd", OPCODE_ADMINCONN, 0, 0xFFFFFFFF);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
			}
		}
		break;
	case OPCODE_QUERYANGLERSTATE:	// AdminId || AdminPwd
		{
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};
			BYTE TbbySendBuff[(MAX_NOTICE_LEGNTH + 100) * 2 + 100] = {0,};

			int nPacketReadPos = 0;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if( bAdminSuccess) // 자료전송
			{
				pbBufferSend = new BYTE[sizeof(STATEINFO)*g_nAnglerStateNum+1000];

				nSendLen = MakePacketAdmin(pbBufferSend, "cddb", OPCODE_QUERYANGLERSTATE, 0, g_nAnglerStateNum, g_pAnglerStateInfo, sizeof(STATEINFO)*g_nAnglerStateNum);
				LeaveCriticalSection(&g_CR);

				send(*pSockComm, (char *)pbBufferSend, nSendLen, 0);

				delete [] pbBufferSend;
				PrintLog(L"%s: OPCODE_QUERYANGLERSTATE Ok", szAdminID);
			}
			else
			{
				LeaveCriticalSection(&g_CR);
				//관리자 접속실패
				PrintLog(L"OPCODE_QUERYANGLERSTATE Failed");
				BYTE TbbySendBuff[100] = {0,};
				nSendLen = MakePacketAdmin(TbbySendBuff, "cdd", OPCODE_QUERYANGLERSTATE, 0, 0xFFFFFFFF);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
			}
		}
		break;
	case OPCODE_QUERYANGLERHISTORY:	// AdminId || AdminPwd
		{
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};
			BYTE TbbySendBuff[(MAX_NOTICE_LEGNTH + 100) * 2 + 100] = {0,};

			int nPacketReadPos = 0;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminPwd) + 1);

			ULONGLONG ullStart;
			ULONGLONG ullEnd;
			memcpy(&ullStart, pbBufferRecv+nPacketReadPos, 8);
			memcpy(&ullEnd, pbBufferRecv+nPacketReadPos+8, 8);
			SYSTEMTIME systime1;
			FileTimeToSystemTime((LPFILETIME)&ullStart, &systime1);
			SYSTEMTIME systime2;
			FileTimeToSystemTime((LPFILETIME)&ullEnd, &systime2);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			LeaveCriticalSection(&g_CR);
			if( bAdminSuccess) // 자료전송
			{
				int nCount = 0;
				HISTORYINFO* pHistoryInfo = GetAnglerHistory(systime1, systime2, &nCount);
				pbBufferSend = new BYTE[sizeof(HISTORYINFO)*nCount+1000];
				nSendLen = MakePacketAdmin(pbBufferSend, "cddb", OPCODE_QUERYANGLERHISTORY, 0, nCount, pHistoryInfo, sizeof(HISTORYINFO)*nCount);
				SendPacket(*pSockComm, (char *)pbBufferSend, nSendLen, 0);
				
				delete [] pbBufferSend;
				PrintLog(L"%s: OPCODE_QUERYANGLERHISTORY Ok", szAdminID);
			}
			else
			{
				//관리자 접속실패
				PrintLog(L"OPCODE_QUERYANGLERHISTORY Failed");
				BYTE TbbySendBuff[100] = {0,};
				nSendLen = MakePacketAdmin(TbbySendBuff, "cdd", OPCODE_QUERYANGLERHISTORY, 0, 0xFFFFFFFF);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
			}
		}
		break;
	case OPCODE_BOSS_ADMINUPDATE:	// UpdateType || AdminId || AdminPwd || AdminInfo
		{
			int			nUpdateType = 0;
			ADMININFO	*pUpdateAdminInfo = NULL;
			BOOL		bFailed = FALSE;
			int			i = 0;
			ADMININFO	*pAdminTemp = NULL;
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};

			int nPacketReadPos = 0;
			nUpdateType = *(int *)pbBufferRecv;
			nPacketReadPos += 4;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminPwd) + 1);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if(bAdminSuccess == FALSE || AdminInfo.dwLevel != ADMINLEVEL_TOP)
			{
				PrintLog(L"%s: Abnormal OPCODE_BOSS_ADMINUPDATE Try", szAdminID);
				LeaveCriticalSection(&g_CR);
				break;
			}

			if(sizeof(ADMININFO) != nRecvLen - nPacketReadPos)
			{
				PrintLog(L"관리자 %s 로부터 송신된 관리자추가 패킷오류", szAdminID);
				LeaveCriticalSection(&g_CR);
				break;
			}

			pUpdateAdminInfo = (ADMININFO *)(pbBufferRecv + nPacketReadPos);

			switch(nUpdateType)
			{
			case ADMININFO_ADD:
				for(i = 0 ; i < g_nAdminNum ; i++)
				{
					if(!wcscmp(g_pAdminInfo[i].szID, pUpdateAdminInfo->szID))
						break;
				}

				if(i < g_nAdminNum)
					bFailed = TRUE;
				else
				{
					pAdminTemp = new ADMININFO[g_nAdminNum + 1];
					ZeroMemory(pAdminTemp, sizeof(ADMININFO) * (g_nAdminNum+1));
					memcpy(pAdminTemp, g_pAdminInfo, sizeof(ADMININFO) * g_nAdminNum);
					memcpy(pAdminTemp+g_nAdminNum, pUpdateAdminInfo, sizeof(ADMININFO));

					g_nAdminNum++;
					if(g_pAdminInfo)
					{
						delete [] g_pAdminInfo;
						g_pAdminInfo = NULL;
					}

					g_pAdminInfo = new ADMININFO[g_nAdminNum];
					memcpy(g_pAdminInfo, pAdminTemp, sizeof(ADMININFO)*g_nAdminNum);

					SaveAdminInfo();
					bFailed = FALSE;
					delete [] pAdminTemp;
				}

				break;
			case ADMININFO_UPDATE:
				for(i = 0 ; i < g_nAdminNum ; i++)
				{
					if(!wcscmp(g_pAdminInfo[i].szID, pUpdateAdminInfo->szID))
						break;
				}
				if(i< g_nAdminNum)
				{
					memcpy(&g_pAdminInfo[i], pUpdateAdminInfo, sizeof(ADMININFO));

					SaveAdminInfo();
					bFailed = FALSE;
				}
				else
					bFailed = TRUE;
				break;
			case ADMININFO_DEL:
				for(i = 0 ; i < g_nAdminNum ; i++)
				{
					if(!wcscmp(g_pAdminInfo[i].szID, pUpdateAdminInfo->szID))
						break;
				}
				if(i < g_nAdminNum)
				{
					pAdminTemp = new ADMININFO[g_nAdminNum - 1];
					ZeroMemory(pAdminTemp, sizeof(ADMININFO)*(g_nAdminNum-1));
					memcpy(pAdminTemp, g_pAdminInfo, sizeof(ADMININFO)*i);
					memcpy(pAdminTemp + i, &g_pAdminInfo[i + 1], sizeof(ADMININFO) * (g_nAdminNum - i - 1));

					if(g_pAdminInfo)
					{
						delete [] g_pAdminInfo;
						g_pAdminInfo = NULL;
					}

					g_pAdminInfo = new ADMININFO[g_nAdminNum - 1];
					g_nAdminNum--;
					ZeroMemory(g_pAdminInfo, sizeof(ADMININFO)*g_nAdminNum);
					memcpy(g_pAdminInfo, pAdminTemp, sizeof(ADMININFO) * g_nAdminNum);

					SaveAdminInfo();

					bFailed = FALSE;	
					delete [] pAdminTemp;
				}
				else
					bFailed = TRUE;
				break;

			default:
				break;
			}

			LeaveCriticalSection(&g_CR);

			if(!bFailed)
			{
				BYTE TbbySendBuff[100];
				ZeroMemory(TbbySendBuff, 100);
				nSendLen = MakePacketAdmin(TbbySendBuff, "cd", OPCODE_BOSS_ADMINUPDATE, 0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);

				PrintLog(L"관리자 %s 관리자계정조작 성공 %d", AdminInfo.szID, nUpdateType);
			}
			else
			{
				BYTE TbbySendBuff[100];
				ZeroMemory(TbbySendBuff, 100);
				nSendLen = MakePacketAdmin(TbbySendBuff, "cd", OPCODE_FAILADD, 0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);

				PrintLog(L"관리자 %s 관리자계정조작 실패 %d", AdminInfo.szID, nUpdateType);
			}
		}

		break;

	case OPCODE_ADMIN_ACCOUNTUPDATE:	// UpdateType || AdminID || AdminPwd || AccountInfo
		{
			ACCOUNTINFO		*pUpdateAccountInfo = NULL;
			int				i = 0;
			ACCOUNTINFO		*pAccountTemp = NULL;
			BOOL			bFailed = TRUE;
			int				nUpdateType = 0;
			int				nUserNum = 0;

			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};

			int nPacketReadPos = 0;
			nUpdateType = *(int *)pbBufferRecv;
			nPacketReadPos += 4;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminPwd) + 1);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if(bAdminSuccess == FALSE || AdminInfo.dwLevel != ADMINLEVEL_TOP)
			{
				PrintLog(L"%s: Abnormal OPCODE_ADMIN_ACCOUNTUPDATE Try", szAdminID);
				LeaveCriticalSection(&g_CR);
				break;
			}

			pUpdateAccountInfo = (ACCOUNTINFO*)(pbBufferRecv + nPacketReadPos);

			switch(nUpdateType)
			{
			case USERINFO_ADD:

				for(i = 0 ; i < g_nAccountNum ; i++)
				{
					if(!wcscmp(g_pAccountInfo[i].szAccount, pUpdateAccountInfo->szAccount))
						break;

					if(!wcscmp(g_pAccountInfo[i].szAdminName, AdminInfo.szID))
					{
						nUserNum++;
					}
				}
				if(i < g_nAccountNum)	// 아이디중복
				{
				}
				else if( nUserNum >= AdminInfo.dwMaxUser )
				{
				}
				else
				{
					int nDayCount = GetDateDiff(pUpdateAccountInfo->pbRegDate, pUpdateAccountInfo->pbExpDate);
					if(nDayCount >= 0)
					{
						nDayCount ++;

						SYSTEMTIME systime;
						GetLocalTime((&systime));
						BYTE byRegDate[4];
						*(short*)byRegDate = systime.wYear;
						byRegDate[2] = (BYTE)systime.wMonth;
						byRegDate[3] = (BYTE)systime.wDay;
						int nDayCount1 = GetDateDiff(byRegDate, pUpdateAccountInfo->pbExpDate);
						if(nDayCount1 >= 0)
						{
							nDayCount1 ++;
							if(nDayCount1 < nDayCount) // 시작일이 현재보다 앞선 경우
							{
								nDayCount = nDayCount1;
								memcpy(pUpdateAccountInfo->pbRegDate, byRegDate, 4);
							}

							CTime startDate, endDate;
							startDate = CTime( *(short*)pUpdateAccountInfo->pbRegDate, pUpdateAccountInfo->pbRegDate[2], pUpdateAccountInfo->pbRegDate[3], 0, 0, 0);
							endDate = CTime( *(short*)pUpdateAccountInfo->pbExpDate, pUpdateAccountInfo->pbExpDate[2], pUpdateAccountInfo->pbExpDate[3], 0, 0, 0);
							m_pCrackCheckEngine->AddAccountW(pUpdateAccountInfo->szAccount, startDate, endDate);

							pAccountTemp = new ACCOUNTINFO[g_nAccountNum + 1];
							ZeroMemory(pAccountTemp, sizeof(ACCOUNTINFO)*(g_nAccountNum+1));
							memcpy(pAccountTemp, g_pAccountInfo, sizeof(ACCOUNTINFO)*g_nAccountNum);
							memcpy(pAccountTemp + g_nAccountNum, pUpdateAccountInfo, sizeof(ACCOUNTINFO));

							if(g_pAccountInfo)
							{
								delete [] g_pAccountInfo;
								g_pAccountInfo = NULL;
							}

							g_pAccountInfo = new ACCOUNTINFO[g_nAccountNum + 1];
							g_nAccountNum++;
							ZeroMemory(g_pAccountInfo, sizeof(ACCOUNTINFO)*g_nAccountNum);
							memcpy(g_pAccountInfo, pAccountTemp, sizeof(ACCOUNTINFO) * g_nAccountNum);
							SaveAccountInfo();

							InsertTodayLog(pUpdateAccountInfo->szAccount, 1, pUpdateAccountInfo->pbRegDate, pUpdateAccountInfo->pbExpDate, nDayCount, pUpdateAccountInfo->nMaxMulti);
							InsertStatistics(pUpdateAccountInfo->szAccount, 1, pUpdateAccountInfo->szAdminName, pUpdateAccountInfo->nMaxMulti, pUpdateAccountInfo->pbExpDate);

							bFailed = FALSE;
							delete [] pAccountTemp;
						}
					}
				}
				break;

			case USERINFO_DEL:
				{
					for(i = 0 ; i < g_nAccountNum ; i++)
					{
						if(!wcscmp(g_pAccountInfo[i].szAccount, pUpdateAccountInfo->szAccount))
						{
							memcpy((LPVOID)pUpdateAccountInfo, (LPVOID)&g_pAccountInfo[i], sizeof(ACCOUNTINFO));
							break;
						}
					}
					if(i < g_nAccountNum)
					{
						pAccountTemp = new ACCOUNTINFO[g_nAccountNum - 1];
						ZeroMemory(pAccountTemp, sizeof(ACCOUNTINFO)*(g_nAccountNum-1));
						memcpy(pAccountTemp, g_pAccountInfo, sizeof(ACCOUNTINFO)*i);
						memcpy(pAccountTemp + i, &g_pAccountInfo[i + 1], sizeof(ACCOUNTINFO) * (g_nAccountNum - i - 1));

						if(g_pAccountInfo)
						{
							delete [] g_pAccountInfo;
							g_pAccountInfo = NULL;
						}

						g_pAccountInfo = new ACCOUNTINFO[g_nAccountNum - 1];
						g_nAccountNum--;
						ZeroMemory(g_pAccountInfo, sizeof(ACCOUNTINFO)*g_nAccountNum);
						memcpy(g_pAccountInfo, pAccountTemp, sizeof(ACCOUNTINFO) * g_nAccountNum);

						SaveAccountInfo();

						SYSTEMTIME systime;
						GetLocalTime((&systime));
						BYTE byRegDate[4];
						*(short*)byRegDate = systime.wYear;
						byRegDate[2] = (BYTE)systime.wMonth;
						byRegDate[3] = (BYTE)systime.wDay;

						int nDayCount1 = GetDateDiff(byRegDate, pUpdateAccountInfo->pbExpDate);
						if(nDayCount1 < 0) // 만료일 이후에 삭제하는 경우
							nDayCount1 = 0;
						else
						{
							nDayCount1 ++;
						}
						int nDayCount2 = GetDateDiff(pUpdateAccountInfo->pbRegDate, pUpdateAccountInfo->pbExpDate);
						if(nDayCount2 < 0) // 만료일 이후에 삭제하는 경우
							nDayCount2 = 0;
						else
						{
							nDayCount2 ++;
						}

						CTime prevStartDate;
						prevStartDate = CTime( *(short*)pUpdateAccountInfo->pbRegDate, pUpdateAccountInfo->pbRegDate[2], pUpdateAccountInfo->pbRegDate[3], 0, 0, 0);
						CTime prevEndDate;
						prevEndDate = CTime( *(short*)pUpdateAccountInfo->pbExpDate, pUpdateAccountInfo->pbExpDate[2], pUpdateAccountInfo->pbExpDate[3], 0, 0, 0);
						m_pCrackCheckEngine->DeleteAccountW(pUpdateAccountInfo->szAccount, prevStartDate, prevEndDate);

						InsertTodayLog(pUpdateAccountInfo->szAccount, 2, pUpdateAccountInfo->pbRegDate, pUpdateAccountInfo->pbExpDate, min(nDayCount1, nDayCount2), 0);
						InsertStatistics(pUpdateAccountInfo->szAccount, 2, pUpdateAccountInfo->szAdminName, pUpdateAccountInfo->nMaxMulti, pUpdateAccountInfo->pbExpDate);

						bFailed = FALSE;
						delete [] pAccountTemp;
					}
				}

				break;

			case USERINFO_UPDATE:
				{
					for(i = 0 ; i < g_nAccountNum ; i++)
					{
						if(!wcscmp(g_pAccountInfo[i].szAccount, pUpdateAccountInfo->szAccount))
							break;
					}
					if(i < g_nAccountNum)
					{
						if(	memcmp(pUpdateAccountInfo->pbRegDate, g_pAccountInfo[i].pbRegDate, 4) == 0 ) // 계정속성은 변경시킬수 없다
						{
							int nDayCount = GetDateDiff(pUpdateAccountInfo->pbRegDate, pUpdateAccountInfo->pbExpDate);
							if(nDayCount >= 0)
							{
								if(	memcmp(g_pAccountInfo[i].pbExpDate, pUpdateAccountInfo->pbExpDate, 4) != 0)
								{
									// 기한변경
									BYTE byRegDate[4];
									memcpy(byRegDate, g_pAccountInfo[i].pbExpDate, 4);
									nDayCount = GetDateDiff(byRegDate, pUpdateAccountInfo->pbExpDate);
									CTime prevEndDate;
									prevEndDate = CTime( *(short*)g_pAccountInfo[i].pbExpDate, g_pAccountInfo[i].pbExpDate[2], g_pAccountInfo[i].pbExpDate[3], 0, 0, 0);
									CTime endDate;
									endDate = CTime( *(short*)pUpdateAccountInfo->pbExpDate, pUpdateAccountInfo->pbExpDate[2], pUpdateAccountInfo->pbExpDate[3], 0, 0, 0);
									m_pCrackCheckEngine->ExtendAccountW(pUpdateAccountInfo->szAccount, prevEndDate, endDate);
									InsertTodayLog(pUpdateAccountInfo->szAccount, 3, byRegDate, pUpdateAccountInfo->pbExpDate, nDayCount, pUpdateAccountInfo->nMaxMulti);
									InsertStatistics(pUpdateAccountInfo->szAccount, 3, pUpdateAccountInfo->szAdminName, pUpdateAccountInfo->nMaxMulti, pUpdateAccountInfo->pbExpDate);
								}
								memcpy(&g_pAccountInfo[i], pUpdateAccountInfo, sizeof(ACCOUNTINFO));
								SaveAccountInfo();

								bFailed = FALSE;
							}
						}
					}
				}
				break;

			default:
				break;
			}

			LeaveCriticalSection(&g_CR);

			if(bFailed)	// 데이터베이스 변경실패
			{
				BYTE TbbySendBuff[100];
				ZeroMemory(TbbySendBuff, 100);
				nSendLen = MakePacketAdmin(TbbySendBuff, "cd", OPCODE_FAILADD, 0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);

				PrintLog(L"%s: 오토계정조작 실패", szAdminID);
			}
			else
			{
				BYTE TbbySendBuff[100];
				ZeroMemory(TbbySendBuff, 100);
				nSendLen = MakePacketAdmin(TbbySendBuff, "cd", OPCODE_ADMIN_ACCOUNTUPDATE, 0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);

				PrintLog(L"%s: 오토계정조작 실패", szAdminID);
			}

		}
		break;

	case OPCODE_ADMIN_ACCOUNTLISTUPDATE:	// UpdateType || Count || AdminID || AdminPwd || AccountInfo (Count)
		{
			int				nUpdateType = 0;
			int				nCount = 0;
			ACCOUNTINFO		*pAccountInfo = NULL;
			int				i = 0;
			ACCOUNTINFO		*pAccountTemp = NULL;
			BOOL			bFailed = TRUE;
			int				nUserNum = 0;

			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};

			int nPacketReadPos = 0;
			nUpdateType = *(int *)pbBufferRecv;
			nPacketReadPos += 4;
			nCount = *(int *)(pbBufferRecv + nPacketReadPos);
			nPacketReadPos += 4;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminPwd) + 1);
			pAccountInfo = (ACCOUNTINFO*)(pbBufferRecv + nPacketReadPos);


			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if(bAdminSuccess == FALSE || AdminInfo.dwLevel != ADMINLEVEL_TOP)
			{
				PrintLog(L"관리자 %s 가 비정상적인 OPCODE_ADMIN_ACCOUNTLISTUPDATE 시도", szAdminID);
				LeaveCriticalSection(&g_CR);
				break;
			}

			switch(nUpdateType)
			{
			case USERINFO_ADD:
				{
					for(int j = 0; j < nCount; j ++)
					{
						nUserNum = 0;
						for(i = 0 ; i < g_nAccountNum ; i++)
						{
							if(!wcscmp(g_pAccountInfo[i].szAccount, pAccountInfo[j].szAccount))
								break;

							if(!wcscmp(g_pAccountInfo[i].szAdminName, AdminInfo.szID))
							{
								nUserNum++;
							}
						}
						if(i < g_nAccountNum)	// 아이디중복
						{
						}
						else if(nUserNum >= AdminInfo.dwMaxUser )
						{
						}
						else
						{
							int nDayCount = GetDateDiff(pAccountInfo[j].pbRegDate, pAccountInfo[j].pbExpDate);
							if(nDayCount >= 0)
							{
								nDayCount ++;

								SYSTEMTIME systime;
								GetLocalTime((&systime));
								BYTE byRegDate[4];
								*(short*)byRegDate = systime.wYear;
								byRegDate[2] = (BYTE)systime.wMonth;
								byRegDate[3] = (BYTE)systime.wDay;
								int nDayCount1 = GetDateDiff(byRegDate, pAccountInfo[j].pbExpDate);
								if(nDayCount1 >= 0)
								{
									nDayCount1 ++;
									if(nDayCount1 < nDayCount) // 시작일이 현재보다 앞선 경우
									{
										nDayCount = nDayCount1;
										memcpy(pAccountInfo[j].pbRegDate, byRegDate, 4);
									}

									CTime startDate, endDate;
									startDate = CTime( *(short*)pAccountInfo[j].pbRegDate, pAccountInfo[j].pbRegDate[2], pAccountInfo[j].pbRegDate[3], 0, 0, 0);
									endDate = CTime( *(short*)pAccountInfo[j].pbExpDate, pAccountInfo[j].pbExpDate[2], pAccountInfo[j].pbExpDate[3], 0, 0, 0);
									m_pCrackCheckEngine->AddAccountW(pAccountInfo[j].szAccount, startDate, endDate);

									pAccountTemp = new ACCOUNTINFO[g_nAccountNum + 1];
									ZeroMemory(pAccountTemp, sizeof(ACCOUNTINFO)*(g_nAccountNum+1));
									memcpy(pAccountTemp, g_pAccountInfo, sizeof(ACCOUNTINFO)*g_nAccountNum);
									memcpy(pAccountTemp + g_nAccountNum, &(pAccountInfo[j]), sizeof(ACCOUNTINFO));

									if(g_pAccountInfo)
									{
										delete [] g_pAccountInfo;
										g_pAccountInfo = NULL;
									}

									g_pAccountInfo = new ACCOUNTINFO[g_nAccountNum + 1];
									g_nAccountNum++;
									ZeroMemory(g_pAccountInfo, sizeof(ACCOUNTINFO)*g_nAccountNum);
									memcpy(g_pAccountInfo, pAccountTemp, sizeof(ACCOUNTINFO) * g_nAccountNum);
									SaveAccountInfo();

									InsertTodayLog(pAccountInfo[j].szAccount, 1, pAccountInfo[j].pbRegDate, pAccountInfo[j].pbExpDate, nDayCount, pAccountInfo[j].nMaxMulti);
									InsertStatistics(pAccountInfo[j].szAccount, 1, pAccountInfo[j].szAdminName, pAccountInfo[j].nMaxMulti, pAccountInfo[j].pbExpDate);
									bFailed = FALSE;
									delete [] pAccountTemp;
								}
							}
						}
					}
				}
				break;
			case USERINFO_UPDATE:
				{
					for(int j = 0; j < nCount; j ++)
					{
						for(i = 0 ; i < g_nAccountNum ; i++)
						{
							if(!wcscmp(g_pAccountInfo[i].szAccount, pAccountInfo[j].szAccount))
							{
								break;
							}
						}
						if(i < g_nAccountNum)
						{
							if(	memcmp(pAccountInfo[j].pbRegDate, g_pAccountInfo[i].pbRegDate, 4) == 0 ) // 시작일은 변경시킬수 없다
							{
								int nDayCount = GetDateDiff(pAccountInfo[j].pbRegDate, pAccountInfo[j].pbExpDate);
								if(nDayCount >= 0)
								{
									if(	memcmp(g_pAccountInfo[i].pbExpDate, pAccountInfo[j].pbExpDate, 4) != 0 || g_pAccountInfo[i].nMaxMulti != pAccountInfo[j].nMaxMulti)
									{
										// 기한변경
										BYTE byRegDate[4];
										memcpy(byRegDate, g_pAccountInfo[i].pbExpDate, 4);
										nDayCount = GetDateDiff(byRegDate, pAccountInfo[j].pbExpDate);
										CTime prevEndDate;
										prevEndDate = CTime( *(short*)g_pAccountInfo[i].pbExpDate, g_pAccountInfo[i].pbExpDate[2], g_pAccountInfo[i].pbExpDate[3], 0, 0, 0);
										CTime endDate;
										endDate = CTime( *(short*)pAccountInfo[j].pbExpDate, pAccountInfo[j].pbExpDate[2], pAccountInfo[j].pbExpDate[3], 0, 0, 0);
										m_pCrackCheckEngine->ExtendAccountW(pAccountInfo[j].szAccount, prevEndDate, endDate);
										InsertTodayLog(pAccountInfo[j].szAccount, 3, byRegDate, pAccountInfo[j].pbExpDate, nDayCount, pAccountInfo[j].nMaxMulti);
										InsertStatistics(pAccountInfo[j].szAccount, 3, pAccountInfo[j].szAdminName, pAccountInfo[j].nMaxMulti, pAccountInfo[j].pbExpDate);
									}
									
									memcpy(&g_pAccountInfo[i], &pAccountInfo[j], sizeof(ACCOUNTINFO));
									SaveAccountInfo();
									bFailed = FALSE;
								}
							}
						}
					}
				}
				break;
			case USERINFO_DEL:
				{
					for(int j = 0; j < nCount; j ++)
					{
						for(i = 0 ; i < g_nAccountNum ; i++)
						{
							if(!wcscmp(g_pAccountInfo[i].szAccount, pAccountInfo[j].szAccount))
							{
								memcpy((LPVOID)&pAccountInfo[j], (LPVOID)&g_pAccountInfo[i], sizeof(ACCOUNTINFO));
								break;
							}
						}
						if(i < g_nAccountNum)
						{
							pAccountTemp = new ACCOUNTINFO[g_nAccountNum - 1];
							ZeroMemory(pAccountTemp, sizeof(ACCOUNTINFO)*(g_nAccountNum-1));
							memcpy(pAccountTemp, g_pAccountInfo, sizeof(ACCOUNTINFO)*i);
							memcpy(pAccountTemp + i, &g_pAccountInfo[i + 1], sizeof(ACCOUNTINFO) * (g_nAccountNum - i - 1));

							if(g_pAccountInfo)
							{
								delete [] g_pAccountInfo;
								g_pAccountInfo = NULL;
							}

							g_pAccountInfo = new ACCOUNTINFO[g_nAccountNum - 1];
							g_nAccountNum--;
							ZeroMemory(g_pAccountInfo, sizeof(ACCOUNTINFO)*g_nAccountNum);
							memcpy(g_pAccountInfo, pAccountTemp, sizeof(ACCOUNTINFO) * g_nAccountNum);

							SaveAccountInfo();

							SYSTEMTIME systime;
							GetLocalTime((&systime));
							BYTE byRegDate[4];
							*(short*)byRegDate = systime.wYear;
							byRegDate[2] = (BYTE)systime.wMonth;
							byRegDate[3] = (BYTE)systime.wDay;

							int nDayCount1 = GetDateDiff(byRegDate, pAccountInfo[j].pbExpDate);
							if(nDayCount1 < 0) // 만료일 이후에 삭제하는 경우
								nDayCount1 = 0;
							else
							{
								nDayCount1 ++;
							}
							int nDayCount2 = GetDateDiff(pAccountInfo[j].pbRegDate, pAccountInfo[j].pbExpDate);
							if(nDayCount2 < 0) // 만료일 이후에 삭제하는 경우
								nDayCount2 = 0;
							else
							{
								nDayCount2 ++;
							}


							CTime prevStartDate;
							prevStartDate = CTime( *(short*)pAccountInfo[j].pbRegDate,pAccountInfo[j].pbRegDate[2], pAccountInfo[j].pbRegDate[3], 0, 0, 0);
							CTime prevEndDate;
							prevEndDate = CTime( *(short*)pAccountInfo[j].pbExpDate, pAccountInfo[j].pbExpDate[2], pAccountInfo[j].pbExpDate[3], 0, 0, 0);
							m_pCrackCheckEngine->DeleteAccountW(pAccountInfo[j].szAccount, prevStartDate, prevEndDate);

							InsertTodayLog(pAccountInfo[j].szAccount, 2, pAccountInfo[j].pbRegDate, pAccountInfo[j].pbExpDate, min(nDayCount1, nDayCount2), 0);
							InsertStatistics(pAccountInfo[j].szAccount, 2, pAccountInfo[j].szAdminName, pAccountInfo[j].nMaxMulti, pAccountInfo[j].pbExpDate);
							bFailed = FALSE;
							delete [] pAccountTemp;
						}
					}
				}
				break;
			default:
				break;
			}

			LeaveCriticalSection(&g_CR);
			if(bFailed)	// 데이터베이스 변경실패
			{
				BYTE TbbySendBuff[100];
				ZeroMemory(TbbySendBuff, 100);
				nSendLen = MakePacketAdmin(TbbySendBuff, "cd", OPCODE_FAILADD, 0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);

				PrintLog(L"%s: 오토계정목록조작 실패", szAdminID);
			}
			else
			{
				BYTE TbbySendBuff[100];
				ZeroMemory(TbbySendBuff, 100);
				nSendLen = MakePacketAdmin(TbbySendBuff, "cd", OPCODE_ADMIN_ACCOUNTUPDATE, 0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);

				PrintLog(L"%s: 오토계정목록조작 성공", szAdminID);
			}

		}
		break;


	case OPCODE_INITMAC:	// AdminID || AdminPwd
		{
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};

			int nPacketReadPos = 0;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminPwd) + 1);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if(bAdminSuccess == FALSE || AdminInfo.dwLevel != ADMINLEVEL_TOP)
			{
				PrintLog(L"관리자 %s 가 비정상적인 오토장치키초기화를 시도", szAdminID);
				LeaveCriticalSection(&g_CR);
				break;
			}

			for(int i = 0 ; i < g_nAccountNum ; i++)
			{
				if(AdminInfo.dwLevel == ADMINLEVEL_TOP)
				{
					ZeroMemory(g_pAccountInfo[i].pbAccount, 8);
				}
				else if(!wcscmp(g_pAccountInfo[i].szAdminName, AdminInfo.szID))
				{
					ZeroMemory(g_pAccountInfo[i].pbAccount, 8);
				}
			}

			SaveAccountInfo();
			LeaveCriticalSection(&g_CR);

			BYTE TbbySendBuff[100];
			ZeroMemory(TbbySendBuff, 100);
			nSendLen = MakePacketAdmin(TbbySendBuff, "cd", OPCODE_INITMAC, 0);
			send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);

			PrintLog(L"Initialize MAC Address");

		}
		break;

	case OPCODE_ADMINEXIT:	// AdminID || AdminPwd
		{
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};

			int nPacketReadPos = 0;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminPwd) + 1);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			LeaveCriticalSection(&g_CR);

			if(bAdminSuccess == FALSE)
			{
				PrintLog(L"관리자 %s 가 비정상적인 OPCODE_ADMINEXIT 시도", szAdminID);			
				break;
			}

			PrintLog(L"관리자 %s 가 접속을 종료", szAdminID);
		}
		break;
	case OPCODE_ADMINFILE: // AdminID || AdminPwd || 업로드
		{
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};

			int nPacketReadPos = 0;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminPwd) + 1);

			EnterCriticalSection(&g_CR);

			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if(bAdminSuccess == FALSE || wcscmp(AdminInfo.szID, ADMIN_TOP_ID) != 0)
			{
				PrintLog(L"관리자 %s 가 비정상적인 OPCODE_ADMINFILE 시도", szAdminID);
				LeaveCriticalSection(&g_CR);
				break;
			}			
			LeaveCriticalSection(&g_CR);

			BOOL bRet = TRUE;
			if(nRecvLen - nPacketReadPos != 4)
			{
				bRet = UpdateAutoPatchFile(pbBufferRecv + nPacketReadPos, nRecvLen - nPacketReadPos - 4);
			}
			
			DWORD dwPatchAddress = *(DWORD*)(pbBufferRecv + nRecvLen - 4);
			InsertPatchAddress(dwPatchAddress);
			g_dwPatchAddress = dwPatchAddress;

			PrintLog(L"Manager Update");

			// 응답패킷
			BYTE TbbySendBuff[100];
			ZeroMemory(TbbySendBuff, 100);
			nSendLen = MakePacketAdmin(TbbySendBuff, "cdd", OPCODE_ADMINFILE, 0, bRet);
			send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
		}
		break;
	case OPCODE_ACCOUNTSET_UPLOAD:// AdminId || AdminPwd || UseID || 업로드
		{
			WCHAR	szAdminID[20] = {0,}, szAdminPwd[20] = {0,};
			WCHAR szUserID[20];

			int nPacketReadPos = 0;
			wcsncpy_s(szAdminID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminID) + 1);
			wcsncpy_s(szAdminPwd, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szAdminPwd) + 1);
			wcsncpy_s(szUserID, 20, (WCHAR*)(pbBufferRecv + nPacketReadPos), 19);
			nPacketReadPos += 2 * (wcslen(szUserID) + 1);

			EnterCriticalSection(&g_CR);
			
			BOOL bAdminSuccess = GetAdminInfo(szAdminID, szAdminPwd, &AdminInfo);

			if(bAdminSuccess == FALSE || AdminInfo.dwLevel != ADMINLEVEL_TOP)
			{
				PrintLog(L"관리자 %s 가 비정상적인 OPCODE_ACCOUNTSET_UPLOAD 시도", szAdminID);
				LeaveCriticalSection(&g_CR);
				break;
			}	
			LeaveCriticalSection(&g_CR);

			BOOL bRet = TRUE;
			bRet = SaveAccountSetFile(pbBufferRecv + nPacketReadPos, nRecvLen - nPacketReadPos, szUserID);

			// 응답패킷
			BYTE TbbySendBuff[100];
			ZeroMemory(TbbySendBuff, 100);
			nSendLen = MakePacketAdmin(TbbySendBuff, "cdd", OPCODE_ACCOUNTSET_UPLOAD, 0, bRet);
			send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
		}
		break;
	case OPCODE_MULTICOUNT_DATE:
		{
			SYSTEMTIME tDate;

			BYTE* buff = pbBufferRecv;
			tDate.wYear	= *(DWORD*)buff;								buff+=4;
			tDate.wMonth	= *buff;									buff++;
			tDate.wDay		= *buff;									buff++;

			int nLen = 0;
			LPBYTE pSendBuff = GetMultiCount_DaybyDay(tDate, &nLen);

			if(nLen != 0)
			{
				LPBYTE pbySendBuffer = new BYTE[nLen + 100];
				nSendLen = MakePacketAdmin(pbySendBuffer, "cdb", OPCODE_MULTICOUNT_DATE, 0, pSendBuff, nLen);
				send(*pSockComm, (char *)pbySendBuffer, nSendLen, 0);
				delete pbySendBuffer;
				delete pSendBuff;
			}
			else
			{
				LPBYTE pbySendBuffer = new BYTE[nLen + 100];
				nSendLen = MakePacketAdmin(pbySendBuffer, "cd", OPCODE_MULTICOUNT_DATE, 0);
				send(*pSockComm, (char *)pbySendBuffer, nSendLen, 0);
				delete pbySendBuffer;
				delete pSendBuff;
			}
		}
		break;
	}

	if(pbBufferRecv)
		delete [] pbBufferRecv;

	WaitForRecvClosePacketAndCloseSocket(pSockComm);
	return 1;
}

int SaveAccountSetFile(LPBYTE lpBuffer, int nLength, LPWSTR szFileName)
{
	WCHAR szPath[MAX_PATH];
	swprintf(szPath, L"%s\\%s.ini", g_szAccountPath, szFileName);
	DeleteFile(szPath);

	FILE *fp;
	fp = _wfopen(szPath, L"wb");
	if(!fp)
	{
		return 0;
	}
	fwrite(lpBuffer, 1, nLength, fp);
	fclose(fp);
	return 1;
}
