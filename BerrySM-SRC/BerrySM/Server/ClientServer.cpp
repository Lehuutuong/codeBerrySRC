#include "stdafx.h"
#include "ClientServer.h"
#include "ServerEngine.h"
#include "RC4.h"
#include "../AuthLib/Auth.h"
#include "CrackCheckEngine.h"
#include "Account.h"
#include "AnglerState.h"
#include <io.h>

SOCKET	g_sockServerDes;
SOCKET	g_sockServerSecure;
SOCKET	g_sockServerState;

extern void PrintLogWorker(const char *format,...);
extern	CCrackCheckEngine* m_pCrackCheckEngine;
DWORD ServerThreadDes(LPVOID pParam)
{
	sockaddr_in		addrServer;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(SERVER_PORT_DES);
	g_sockServerDes = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_sockServerDes == INVALID_SOCKET)
		return 0;

	for(;;)
	{
		if(bind(g_sockServerDes, (sockaddr*)&addrServer, sizeof(addrServer)))
		{
			Sleep(500);
			continue;
		}
		break;
	}
	if(listen(g_sockServerDes, SOMAXCONN))
		return 0;

	sockaddr_in		addrFrom;
	int				nFromLen;

	nFromLen = sizeof(addrFrom);
	while(1)
	{
		ACCOUNTSOCKET *pSocketComm = new ACCOUNTSOCKET;

		if(g_bStopServer == 1)
		{
			break;
		}

		pSocketComm->Sock = accept(g_sockServerDes, (struct sockaddr*)&addrFrom, &nFromLen);
		if(pSocketComm->Sock == INVALID_SOCKET)
		{
			PrintLog(L"망오류: 련결요청을 받아 들일수 없습니다.");
			FreeDBConnection();
			ExitProcess(0);
			return 0;
		}
		PrintLog(L"[Des] IP %d.%d.%d.%d: - Request2: %x", addrFrom.sin_addr.S_un.S_un_b.s_b1, 
			addrFrom.sin_addr.S_un.S_un_b.s_b2,
			addrFrom.sin_addr.S_un.S_un_b.s_b3,
			addrFrom.sin_addr.S_un.S_un_b.s_b4,
			pSocketComm->Sock);

		int nIPPiece[4];
		nIPPiece[0] = addrFrom.sin_addr.S_un.S_un_b.s_b1;
		nIPPiece[1] = addrFrom.sin_addr.S_un.S_un_b.s_b2;
		nIPPiece[2] = addrFrom.sin_addr.S_un.S_un_b.s_b3;
		nIPPiece[3] = addrFrom.sin_addr.S_un.S_un_b.s_b4;
		pSocketComm->dwIP = nIPPiece[0] << 24;
		pSocketComm->dwIP |= nIPPiece[1] << 16;
		pSocketComm->dwIP |= nIPPiece[2] << 8;
		pSocketComm->dwIP |= nIPPiece[3];

		HANDLE han = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CommThreadDes, (LPVOID)pSocketComm, NULL, NULL);
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

DWORD	ServerThreadSecure(LPVOID pParam)
{
	sockaddr_in		addrServer;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(SERVER_PORT_SECURE);
	g_sockServerSecure = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_sockServerSecure == INVALID_SOCKET)
		return 0;

	for(;;)
	{
		if(bind(g_sockServerSecure, (sockaddr*)&addrServer, sizeof(addrServer)))
		{
			Sleep(500);
			continue;
		}
		break;
	}
	if(listen(g_sockServerSecure, SOMAXCONN))
		return 0;

	sockaddr_in		addrFrom;
	int				nFromLen;

	nFromLen = sizeof(addrFrom);
	while(1)
	{
		ACCOUNTSOCKET *pSocketComm = new ACCOUNTSOCKET;

		if(g_bStopServer == 1)
		{
			break;
		}

		pSocketComm->Sock = accept(g_sockServerSecure, (struct sockaddr*)&addrFrom, &nFromLen);
		if(pSocketComm->Sock == INVALID_SOCKET)
		{
			PrintLog(L"망오류: 련결요청을 받아 들일수 없습니다.");
			FreeDBConnection();
			ExitProcess(0);
			return 0;
		}
		PrintLog(L"[Secure] IP %d.%d.%d.%d: - Request: %x", addrFrom.sin_addr.S_un.S_un_b.s_b1, 
			addrFrom.sin_addr.S_un.S_un_b.s_b2,
			addrFrom.sin_addr.S_un.S_un_b.s_b3,
			addrFrom.sin_addr.S_un.S_un_b.s_b4,
			pSocketComm->Sock);

		int nIPPiece[4];
		nIPPiece[0] = addrFrom.sin_addr.S_un.S_un_b.s_b1;
		nIPPiece[1] = addrFrom.sin_addr.S_un.S_un_b.s_b2;
		nIPPiece[2] = addrFrom.sin_addr.S_un.S_un_b.s_b3;
		nIPPiece[3] = addrFrom.sin_addr.S_un.S_un_b.s_b4;
		pSocketComm->dwIP = nIPPiece[0] << 24;
		pSocketComm->dwIP |= nIPPiece[1] << 16;
		pSocketComm->dwIP |= nIPPiece[2] << 8;
		pSocketComm->dwIP |= nIPPiece[3];

		HANDLE han = CreateThread(NULL, 10 * 1024, (LPTHREAD_START_ROUTINE)CommThreadSecure, (LPVOID)pSocketComm, NULL, NULL);
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




DWORD WINAPI CommThreadDes(LPVOID pParam)
{
	ACCOUNTSOCKET *pAccountSockComm = (ACCOUNTSOCKET*)pParam;
	SOCKET	*pSockComm = &pAccountSockComm->Sock;
	BYTE	bOpCode;
	BYTE	*pbBufferRecv = NULL, *pbBufferSend = NULL;
	int		nRecvLen, nPacketLen, nTemp;

	int  nTime = RECV_TIMEOUT;
	setsockopt(*pSockComm, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTime,4);
	nRecvLen = recv(*pSockComm, (char *)&bOpCode, 1, 0);
	if(nRecvLen <= 0)
	{
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pAccountSockComm;
		return 0;
	}

	nTemp = 0;
	while(nTemp < 4)
	{
		nRecvLen = recv(*pSockComm, ((char *)&nPacketLen)+nTemp, 4-nTemp, 0);
		if(nRecvLen <= 0)
		{
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pAccountSockComm;
			return 0;
		}
		nTemp += nRecvLen;
	}
	if(nPacketLen > 10000000 || nPacketLen < 0)
	{
		PrintLog(L"망오류: Packet Length - Unknown");
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pAccountSockComm;
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
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pAccountSockComm;
			if(pbBufferRecv)
			{
				delete [] pbBufferRecv;
				pbBufferRecv = NULL;
			}
			return 0;
		}
		nTemp += nRecvLen;
	}

	int nCloseSocket = 0;
	nRecvLen = nPacketLen;
	WCHAR szAutoIP[20];
	DwordToIPString(pAccountSockComm->dwIP, szAutoIP);

	switch(bOpCode)
	{
	case OPCODE_LOGIN_DES:
		{
			BYTE idd[48],passs[48],deviceKeyy[16];
			int clientVersionn;
			BYTE randBuf[16];
			BYTE opCode[96] = {0};
			CAuth s;

			if(s.GetInfoClientToServer(pbBufferRecv, idd, passs, deviceKeyy, randBuf, &clientVersionn) == TRUE)
			{
				WCHAR	szID[20] = {0,},szPwd[20]= {0,};
				BYTE pbyDevKey[8];
				memcpy(pbyDevKey, deviceKeyy, 8);
				wcsncpy_s(szID, 20, (WCHAR*)(idd), 19);
				wcsncpy_s(szPwd, 20, (WCHAR*)(passs), 19);

				EnterCriticalSection(&g_CR);

				*(DWORD*)opCode = GetTickCount();
				// 등록된 유저인가 검사.
				int nIndex = -1;
				nIndex = CheckHuntUserInfo(szID, szPwd, pbyDevKey);

				int nLogAdd = 0;
				if(nIndex == -1)
				{
					*(DWORD*)opCode = 0xFFFFFFFF;
					PrintLog(L"Hunt User Fail ID: %s, Request2: %s", szID, szAutoIP);
				}
				else if(nIndex == -2)
				{
					*(DWORD*)opCode = 0xFFFFFFFE;
				}
				else
				{
					nLogAdd = 1;
					//if(CheckAutoIP(szID, szAutoIP) == TRUE) // 중복IP체크
					{
						*(DWORD*)opCode = 1;
						*(DWORD*)(opCode + 4) = g_dwPatchAddress;
						*(DWORD*)(opCode + 8) = 1; // *(DWORD*)(opCode + 8) = nMultiIndex;
					}
					/*else
					{
						PrintLog(L"Hunt User Double IP ID %s, Request2: %s", szID, szAutoIP);
						m_pCrackCheckEngine->WriteLogForMultiIPW(szID, szAutoIP, TRUE);
					}*/
				}

				if(nLogAdd == 1)
				{
					if(InsertUseTodayLog(szID, nIndex) == 0) // 중복실행체크
					{
						PrintLog(L"Hunt User Double Key ID %s, Request2: %s", szID, szAutoIP);
					}
					else
					{
						PrintLog(L"Hunt User Key ID %s, Request2: %s", szID, szAutoIP);
					}
				}

				LeaveCriticalSection(&g_CR);

				BYTE buffer[137]; // Opcode(1) | Length(4) | Data(132)
				buffer[0] = OPCODE_LOGIN_DES;
				*(DWORD*)(buffer + 1) = 132;
				int nLen = 137;
				if(s.GetBufferServerToClient(clientVersionn, randBuf, opCode, buffer + 5)==TRUE)
				{
					send(*pSockComm, (char *)buffer, nLen, 0);
				}

			}
		}
		break;
	}
	WaitForRecvClosePacketAndCloseSocket(pSockComm);
	delete pAccountSockComm;
	if(pbBufferRecv)
	{
		delete [] pbBufferRecv;
		pbBufferRecv = NULL;
	}
	return 1;
}

DWORD  CommThreadSecure(LPVOID pParam)
{
	ACCOUNTSOCKET *pAccountSockComm = (ACCOUNTSOCKET*)pParam;
	SOCKET	*pSockComm = &pAccountSockComm->Sock;

	BYTE	bOpCode;
	BYTE	*pbBufferRecv = NULL, *pbBufferSend = NULL;
	int		nRecvLen, nSendLen, nPacketLen, nTemp;
	int		nRet = 0;
	ADMININFO	AdminInfo;
	ZeroMemory(&AdminInfo, sizeof(ADMININFO));

	int  nTime = RECV_TIMEOUT;
	setsockopt(*pSockComm, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTime,4);
	nRecvLen = recv(*pSockComm, (char *)&bOpCode, 1, 0);
	if(nRecvLen <= 0)
	{
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pAccountSockComm;
		return 0;
	}
	nTemp = 0;
	while(nTemp < 4)
	{
		nRecvLen = recv(*pSockComm, ((char *)&nPacketLen)+nTemp, 4-nTemp, 0);
		if(nRecvLen <= 0)
		{
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pAccountSockComm;
			return 0;
		}
		nTemp += nRecvLen;
	}
	if(nPacketLen > 10000000 || nPacketLen < 0)
	{
		PrintLog(L"망오류: Packet Length - Unknown");
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pAccountSockComm;
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
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pAccountSockComm;
			if(pbBufferRecv)
				delete [] pbBufferRecv;
			return 0;
		}
		nTemp += nRecvLen;
	}

	WCHAR szAutoIP[20];
	DwordToIPString(pAccountSockComm->dwIP, szAutoIP);

	nRecvLen = nPacketLen;
	if(nRecvLen != 0 && rc4DecryptStream(pbBufferRecv, nRecvLen) == FALSE)
	{
		WCHAR szAutoIP[20];
		DwordToIPString(pAccountSockComm->dwIP, szAutoIP);
		m_pCrackCheckEngine->WriteLogForPacketErrorW(szAutoIP);
		bOpCode = 0;
	}
	switch(bOpCode)
	{
	case OPCODE_EXIT: // | Rand(8) | DevKey(8) | ID | PWD |
		{
			ACCOUNTINFO	*pAccountInfo = NULL;
			LPBYTE byKey = pbBufferRecv + 8;
			WCHAR	szID[20],szPwd[20];
			wcsncpy_s(szID, 20, (WCHAR*)(pbBufferRecv + 16), 19);
			wcsncpy_s(szPwd, 20, (WCHAR*)(pbBufferRecv + 16 + (2*(wcslen(szID)+1))), 19);

			EnterCriticalSection(&g_CR);

			int nIndex = CheckHuntUserInfo(szID, szPwd, byKey);
			if(nIndex == -1)
			{
			}
			else if(nIndex == -2)
			{
			}
			else
			{
				// 사용가능상태인가 본다.
				PrintLog(L"Account %s ID: %s - Exit", szAutoIP, szID);
			}

			LeaveCriticalSection(&g_CR);
		}
		break;

	case OPCODE_LOGIN:  // | Rand(8) | DeviceKey(8) | Version(4) | ID | PWD |
		{
			BYTE TbbyRand[8];
			memcpy(TbbyRand, pbBufferRecv, 8);
			LPBYTE byKey = pbBufferRecv + 8;

			WCHAR	szID[20] = {0,},szPwd[20] = {0,};
			wcsncpy_s(szID, 20, (WCHAR*)(pbBufferRecv + 20), 19);
			wcsncpy_s(szPwd, 20, (WCHAR*)(pbBufferRecv + 20 + (2*(wcslen(szID)+1))), 19);

			EnterCriticalSection(&g_CR);

			int nIndex = CheckHuntUserInfo(szID, szPwd, byKey);

			DWORD dwResult = 0;
			if(nIndex == -1)
			{
				m_pCrackCheckEngine->WriteLogForPwdErrorW(szID, szPwd, szAutoIP);
				dwResult = 0xFFFFFFFF;
			}
			else if(nIndex == -2)
			{
				dwResult = 0xFFFFFFFE;
				PrintLog(L"Account %s ID: %s - Expiration", szAutoIP, szID);
			}
			else
			{
				if(bOpCode == OPCODE_LOGIN)
				{
					SYSTEMTIME systime;
					int nTryCount = 0;
					GetLocalTime(&systime);

					int nDBIndex = GetUserLoginTryCount(szID, systime, &nTryCount);
					if(nTryCount >= LOGIN_COUNT_ATDAY) // 하루 로그인회수 초과
					{
						m_pCrackCheckEngine->AbnormalLoginTryCountW(g_pAccountInfo[nIndex].szAccount, nTryCount);
					}
					if (nDBIndex == -1)
						InsertTodayLog(szID, 5);	
					else
						UpdateTodayLog(nDBIndex, systime);
				}
				if(g_dwVersion == *(DWORD *)(pbBufferRecv+16))
				{
					RegistAutoIP(szID, szAutoIP);
					dwResult = 0;
					PrintLog(L"Account %s ID: %s - Version OK", szAutoIP, szID);
				}
				else
				{
					//업데이트시 인증키 초기화
					//memset(g_pAccountInfo[nIndex].pbAccount, 0, 8);
					//SaveAccountInfo();
					// 업데이트된 파일을 보낸다.
					dwResult = 0xCDCDCDCD;
					PrintLog(L"Account %s ID: %s - Update", szAutoIP, szID);
				}
			}
			LeaveCriticalSection(&g_CR);

			BYTE TbbySendRand[8];
			*(DWORD*)TbbySendRand = GetTickCount();
			*(DWORD*)(TbbySendRand+4) = GetTickCount() * rand();

			if(dwResult == 0xCDCDCDCD) // 업데이트
			{
				BYTE TbbySendBuff[200] = {0,};

				LPWSTR ftpID = g_szftpID;
				LPWSTR ftpPwd = g_szftpPwd;
				LPWSTR ftpServer = g_szftpServer;
				LPWSTR ftpFileName = g_szftpFileName;

				nSendLen = MakePacketSecure(TbbySendBuff, "cdbbdSSSSd", bOpCode, 0, TbbySendRand, 8, TbbyRand, 8, 0xCDCDCDCD, ftpID, ftpPwd, ftpServer, ftpFileName, 0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
			}
			else if (dwResult == 0)
			{
				BYTE TbbySendBuff[sizeof(ACCOUNT_INFO) + 200] = {0,};
				nSendLen = MakePacketSecure(TbbySendBuff, "cdbbddd", bOpCode, 0, TbbySendRand, 8, TbbyRand, 8, dwResult, g_pAccountInfo[nIndex].nMaxMulti, 0);
				PrintLog(L"멀티갯수 %d", g_pAccountInfo->nMaxMulti);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
			}
			else
			{
				BYTE TbbySendBuff[100] = {0,};
				nSendLen = MakePacketSecure(TbbySendBuff, "cdbbdd", bOpCode, 0, TbbySendRand, 8, TbbyRand, 8, dwResult,  0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
			}

		}

		break;

	case OPCODE_SECONDLOGIN:  // | Rand(8) | DeviceKey(8) | Version(4) | LocalIP(4) | ID | PWD |
		{
			BYTE TbbyRand[8];
			memcpy(TbbyRand, pbBufferRecv, 8);
			LPBYTE byKey = pbBufferRecv + 8;
			DWORD dwLocalIP = *(DWORD*)(pbBufferRecv + 20);
			WCHAR	szID[20] = {0,},szPwd[20] = {0,};
			wcsncpy_s(szID, 20, (WCHAR*)(pbBufferRecv + 24), 19);
			wcsncpy_s(szPwd, 20, (WCHAR*)(pbBufferRecv + 24 + (2*(wcslen(szID)+1))), 19);

			EnterCriticalSection(&g_CR);

			int nIndex = CheckHuntUserInfo(szID, szPwd, byKey);
			DWORD dwResult = 0;
			if(nIndex == -1)
			{
				m_pCrackCheckEngine->WriteLogForPwdErrorW(szID, szPwd, szAutoIP);
				dwResult = 0xFFFFFFFF;
			}
			else if(nIndex == -2)
			{
				dwResult = 0xFFFFFFFE;
				PrintLog(L"Account %s ID: %s - Expiration", szAutoIP, szID);
			}
			else
			{
				if(g_pAccountInfo[nIndex].dwLocalIP != dwLocalIP)
				{
					g_pAccountInfo[nIndex].dwLocalIP = dwLocalIP;
					SaveAccountInfo();
				}
				if(g_pAccountInfo[nIndex].dwConnectIP != pAccountSockComm->dwIP)
				{
					g_pAccountInfo[nIndex].dwConnectIP = pAccountSockComm->dwIP;
					SaveAccountInfo();
				}

				/*if(CheckAutoIP(szID, szAutoIP) == TRUE) // 중복IP체크
				{
				}
				else
				{
					PrintLog(L"Hunt User Double IP ID %s, Request2: %s", szID, szAutoIP);
					m_pCrackCheckEngine->WriteLogForMultiIPW(szID, szAutoIP, TRUE);
					dwResult = 0xFFFFFFFD;
				}*/

				if(InsertUseTodayLog(szID, nIndex) == 0) // 중복실행체크
				{
					PrintLog(L"Hunt User Double Key ID %s, Request2: %s", szID, szAutoIP);
				}
				else
				{
					PrintLog(L"Hunt User Key ID %s, Request2: %s", szID, szAutoIP);
				}
			}
			LeaveCriticalSection(&g_CR);

			BYTE TbbySendRand[8];
			*(DWORD*)TbbySendRand = GetTickCount();
			*(DWORD*)(TbbySendRand+4) = GetTickCount() * rand();

			BYTE TbbySendBuff[100] = {0,};
			nSendLen = MakePacketSecure(TbbySendBuff, "cdbbdd", bOpCode, 0, TbbySendRand, 8, TbbyRand, 8, dwResult, 0);
			send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);

		}

		break;

	case OPCODE_ACCOUNTSET_DOWNLOAD: // | Rand(8) | DeviceKey(8) | Version(4) | ID | PWD |
		{
			BYTE TbbyRand[8];
			memcpy(TbbyRand, pbBufferRecv, 8);
			LPBYTE byKey = pbBufferRecv + 8;

			WCHAR	szID[20] = {0,},szPwd[20] = {0,};
			wcsncpy_s(szID, 20, (WCHAR*)(pbBufferRecv + 20), 19);
			wcsncpy_s(szPwd, 20, (WCHAR*)(pbBufferRecv + 20 + (2*(wcslen(szID)+1))), 19);

			EnterCriticalSection(&g_CR);

			int nIndex = CheckHuntUserInfo(szID, szPwd, byKey);
			DWORD dwResult = 0;
			if(nIndex == -1)
			{
				m_pCrackCheckEngine->WriteLogForPwdErrorW(szID, szPwd, szAutoIP);
				dwResult = 0xFFFFFFFF;
			}
			else if(nIndex == -2)
			{
				dwResult = 0xFFFFFFFE;
				PrintLog(L"Account %s ID: %s - Expiration", szAutoIP, szID);
			}
			else
			{
				dwResult = 0;	
			}

			LeaveCriticalSection(&g_CR);

			BYTE TbbySendRand[8];
			*(DWORD*)TbbySendRand = GetTickCount();
			*(DWORD*)(TbbySendRand+4) = GetTickCount() * rand();

			if(dwResult == 0) // 게임계정다운
			{
				int	nLength = 0;
				LPBYTE lpBuffer = GetAccountFile(szID, nLength);
				LPBYTE lpbySendBuff = new BYTE [100 + nLength];
				nSendLen = MakePacketSecure(lpbySendBuff, "cdbbdbd", bOpCode, 0, TbbySendRand, 8, TbbyRand, 8, dwResult, lpBuffer, nLength, 0);
				send(*pSockComm, (char *)lpbySendBuff, nSendLen, 0);
			}
			else
			{
				BYTE TbbySendBuff[100] = {0,};
				nSendLen = MakePacketSecure(TbbySendBuff, "cdbbdd", bOpCode, 0, TbbySendRand, 8, TbbyRand, 8, dwResult, 0);
				send(*pSockComm, (char *)TbbySendBuff, nSendLen, 0);
			}

		}
		break;

	case OPCODE_ACCOUNT_STATE: // | Rand(8) | DeviceKey | ID | PWD | STATE_REPORT
		{
			int			nAccountNum = 0;
			ACCOUNTINFO	*pAccountInfo = NULL;

			LPBYTE byKey = pbBufferRecv + 8;
			WCHAR	szID[20] = {0,},szPwd[20] = {0,};
			wcsncpy_s(szID, 20, (WCHAR*)(pbBufferRecv + 16), 19);
			wcsncpy_s(szPwd, 20, (WCHAR*)(pbBufferRecv + 16 + (2*(wcslen(szID)+1))), 19);

			LPWSTR szStateReport = (LPWSTR) ( pbBufferRecv + 16 + (2*(wcslen(szID)+1))+(2*(wcslen(szPwd)+1)) );
			CString szGame_ID;
			CString szGame_Pwd;
			CString szAccount_State;

			AfxExtractSubString(szGame_ID, szStateReport, 0, L'\t');
			AfxExtractSubString(szGame_Pwd, szStateReport, 1, L'\t');
			AfxExtractSubString(szAccount_State, szStateReport, 2, L'\t');

			EnterCriticalSection(&g_CR);

			int nIndex = CheckHuntUserInfo(szID, szPwd, byKey);
			if(nIndex == -1)
			{
			}
			else if(nIndex == -2)
			{
			}
			else
			{
				InsertAccountState(szID, szGame_ID, szGame_Pwd, szAccount_State, szAutoIP, nIndex);
			}

			LeaveCriticalSection(&g_CR);
		}
		break;

	case OPCODE_CHARAC_STATE: // | Rand(8) I 0xCDCDCDCD(4) | Key(8) | ID | PWD | StateReport |
		{
			if(*(DWORD*)(pbBufferRecv + 8) == 0xCDCDCDCD)
			{
				LPBYTE		byAccountInfo = pbBufferRecv + 20;
				LPBYTE		byKey = pbBufferRecv + 12;

				WCHAR	szID[20] = {0,},szPwd[20]= {0,};
				wcsncpy_s(szID, 20, (WCHAR*)(byAccountInfo), 19);
				wcsncpy_s(szPwd, 20, (WCHAR*)(byAccountInfo+(2*(wcslen(szID)+1))), 19);

				LPWSTR szStateReport = (LPWSTR) (pbBufferRecv+20+(2*(wcslen(szID)+1))+(2*(wcslen(szPwd)+1)));

				EnterCriticalSection(&g_CR);

				// 등록된 유저인가 검사.
				int nIndex = -1;
				nIndex = CheckHuntUserInfo(szID, szPwd, byKey);

				BYTE TbbySendBuff[100] = {0,};
				if(nIndex == -1)
				{
				}
				else if(nIndex == -2)
				{
				}
				else
				{
					InsertCharacState(szID, szAutoIP, szStateReport);
					LPWSTR szGoldPost = wcsstr(szStateReport, L"state = '수금:");
					if(szGoldPost)
					{
						szGoldPost = wcsstr(szGoldPost, L">@<");
						if(szGoldPost)
						{
							szGoldPost += 3;
							LPWSTR szGoldPostEnd = wcsstr(szGoldPost, L">'");
							if(szGoldPostEnd)
							{
								szGoldPostEnd[0] = 0;
								ULONGLONG ullGoldPost = 0;
								ullGoldPost = _wtoi64(szGoldPost);
								if(ullGoldPost)
								{
									SYSTEMTIME systime;
									GetLocalTime(&systime);
									InsertPostGoldState(szID, ullGoldPost, g_pAccountInfo[nIndex].szAdminName, systime);
									m_pCrackCheckEngine->AddGoldW(systime, g_pAccountInfo[nIndex].szAccount, MULTI_COUNT, (DWORD)ullGoldPost);
								}
							}
						}							
					}
				}
				LeaveCriticalSection(&g_CR);
			}
		}
		break;
	}

	WaitForRecvClosePacketAndCloseSocket(pSockComm);
	delete pAccountSockComm;
	if(pbBufferRecv)
		delete [] pbBufferRecv;
	return 1;
}

LPBYTE GetAccountFile(LPWSTR szUserId, int& nLength)
{
	LPBYTE lpBuffer = NULL;

	WCHAR szFileName[MAX_PATH];
	wsprintf(szFileName, L"%s\\%s.ini", g_szAccountPath, szUserId);

	FILE *fp;	
	fp = _wfopen(szFileName, L"rb");
	if (fp)
	{
		nLength = _filelength(_fileno(fp));
		if (nLength)
		{
			lpBuffer = new BYTE[nLength];
			if (lpBuffer)
			{
				fread(lpBuffer, 1, nLength, fp);
			}
		}
		fclose(fp);
	}
	if (lpBuffer == NULL)
		nLength = 0;
	return lpBuffer;
}


DWORD  CommThreadState(LPVOID pParam)
{
	ACCOUNTSOCKET *pAccountSockComm = (ACCOUNTSOCKET*)pParam;
	SOCKET	*pSockComm = &pAccountSockComm->Sock;

	BYTE	bOpCode;
	BYTE	*pbBufferRecv = NULL, *pbBufferSend = NULL;
	int		nRecvLen, nSendLen, nPacketLen, nTemp;
	int		nRet = 0;
	ADMININFO	AdminInfo;
	ZeroMemory(&AdminInfo, sizeof(ADMININFO));

	int  nTime = RECV_TIMEOUT;
	setsockopt(*pSockComm, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTime,4);
	nRecvLen = recv(*pSockComm, (char *)&bOpCode, 1, 0);
	if(nRecvLen <= 0)
	{
		PrintLogWorker("CommThreadState Error 1 %x", pAccountSockComm->Sock);
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pAccountSockComm;
		return 0;
	}
	nTemp = 0;
	while(nTemp < 4)
	{
		nRecvLen = recv(*pSockComm, ((char *)&nPacketLen)+nTemp, 4-nTemp, 0);
		if(nRecvLen <= 0)
		{
			PrintLogWorker("CommThreadState Error 2 %x", pAccountSockComm->Sock);
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pAccountSockComm;
			return 0;
		}
		nTemp += nRecvLen;
	}
	if(nPacketLen > 10000000 || nPacketLen < 0)
	{
		PrintLog(L"망오류: Packet Length - Unknown");
		WaitForRecvClosePacketAndCloseSocket(pSockComm);
		delete pAccountSockComm;
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
			WaitForRecvClosePacketAndCloseSocket(pSockComm);
			delete pAccountSockComm;
			if(pbBufferRecv)
				delete [] pbBufferRecv;
			return 0;
		}
		nTemp += nRecvLen;
	}

	BOOL bSocketClosed = FALSE;
	PANGLERSTATE pOrimState;
	switch(bOpCode)
	{
	case OPCODE_ANGLER_STATE: //
		{
			EnterCriticalSection(&g_CR);
			int j = -1;
			pOrimState = (PANGLERSTATE)pbBufferRecv;
			for (int i = 0; i < g_nAccountNum; i ++)
			{
				if(wcscmp(g_pAccountInfo[i].szAccount, pOrimState->wszID) == 0)
				{
					j = i;
					break;
				}
			}
			if(j == -1)
			{
				LeaveCriticalSection(&g_CR);
				break;
			}
			LeaveCriticalSection(&g_CR);

			TCHAR wszGameId[200] = {0,};
			MultiToWide(pOrimState->szGameID, wszGameId);
			PrintLog(L"[STATE] (%s)(%s - %d번캐릭)(%s)", pOrimState->wszID, wszGameId, pOrimState->byCharNum, g_wszState[pOrimState->byType]);

			WaitForRecvClosePacketAndCloseSocket(pSockComm); 
			bSocketClosed = TRUE;

			EnterCriticalSection(&g_OrimState_CR);
			UpdateAnglerHistory(pOrimState);
			LeaveCriticalSection(&g_OrimState_CR);
		}
		break;
	}
	if(bSocketClosed == FALSE)
	{
		WaitForRecvClosePacketAndCloseSocket(pSockComm); 
	}

	delete pAccountSockComm;
	if(pbBufferRecv)
		delete [] pbBufferRecv;
	return 1;
}

DWORD	ServerThreadClientState(LPVOID pParam)
{
	sockaddr_in		addrServer;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(SERVER_PORT_STATE);
	g_sockServerState = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_sockServerState == INVALID_SOCKET)
		return 0;

	for(;;)
	{
		if(bind(g_sockServerState, (sockaddr*)&addrServer, sizeof(addrServer)))
		{
			Sleep(500);
			continue;
		}
		break;
	}
	if(listen(g_sockServerState, SOMAXCONN))
		return 0;

	sockaddr_in		addrFrom;
	int				nFromLen;

	nFromLen = sizeof(addrFrom);
	while(1)
	{
		ACCOUNTSOCKET *pSocketComm = new ACCOUNTSOCKET;

		if(g_bStopServer == 1)
		{
			break;
		}

		pSocketComm->Sock = accept(g_sockServerState, (struct sockaddr*)&addrFrom, &nFromLen);
		if(pSocketComm->Sock == INVALID_SOCKET)
		{
			PrintLog(L"망오류: 련결요청을 받아 들일수 없습니다.");
			FreeDBConnection();
			ExitProcess(0);
			return 0;
		}
		PrintLogWorker("[STATE] IP %d.%d.%d.%d: - Request: %x", addrFrom.sin_addr.S_un.S_un_b.s_b1, 
			addrFrom.sin_addr.S_un.S_un_b.s_b2,
			addrFrom.sin_addr.S_un.S_un_b.s_b3,
			addrFrom.sin_addr.S_un.S_un_b.s_b4,
			pSocketComm->Sock);

		int nIPPiece[4];
		nIPPiece[0] = addrFrom.sin_addr.S_un.S_un_b.s_b1;
		nIPPiece[1] = addrFrom.sin_addr.S_un.S_un_b.s_b2;
		nIPPiece[2] = addrFrom.sin_addr.S_un.S_un_b.s_b3;
		nIPPiece[3] = addrFrom.sin_addr.S_un.S_un_b.s_b4;
		pSocketComm->dwIP = nIPPiece[0] << 24;
		pSocketComm->dwIP |= nIPPiece[1] << 16;
		pSocketComm->dwIP |= nIPPiece[2] << 8;
		pSocketComm->dwIP |= nIPPiece[3];

		HANDLE han = CreateThread(NULL, 10 * 1024, (LPTHREAD_START_ROUTINE)CommThreadState, (LPVOID)pSocketComm, NULL, NULL);
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