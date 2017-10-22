#include "stdafx.h"
#include <io.h>
#include <Shlwapi.h>
#include "CrackCheckEngine.h"
#include "ServerEngine.h"

#define IMAGE_FOLDER_NAME			"Image"
#define LOG_FOLDER_NAME				"Log"

#define COUNT_KEYNAME				"Count"
#define COUNT6_KEYNAME				"Count6"
#define COUNT10_KEYNAME				"Count10"

#define ADDACCOUNT_SECTION			"#########   Add"
#define DELACCOUNT_SECTION			"#########   Delete"
#define EXTENDACCOUNT_SECTION		"#########   Extend"

#define CONNECTACCOUNT_SECTION		"#########   Connect   #########"
#define VALIDACCOUNT_SECTION		"#########   Valid     #########"
#define BLOCKACCOUNT_SECTION		"#########   Block     #########"
#define AVGCONNECTTIME_SECTION		"#########   AvgTime   #########"
#define POSTGOLD_SECTION			"#########   Gold      #########"
#define LOGINTRYCOUNT_SECTION		"#########   LoginTry  #########"
#define USETIME_SECTION				"#########   UseTime  #########"
#define MULTI_SECTION				"#########   Multi      #########"
#define ISSUE_SECTION				"#########   Issue   #########"

#define SECTION_SUFFIX				"   #########"

#define ACCOUNT_SERVER_PORT			14321
#define CRACKINFO_PORT				14323

#define MAX_LOG_KEY				6

#define LOG_KEY_MULTIIP			"MultiIP"
#define LOG_KEY_MULTIDEVKEY		"MultiDevKey"
#define LOG_KEY_DBERROR			"DBError"
#define LOG_KEY_PACKETERROR		"PacketError"
#define LOG_KEY_IDFAILED		"IdFail"
#define LOG_KEY_PWDFAILED		"PwdFail"

typedef struct _CRACK_ORDER_PACKET{
	DWORD nErrorType;
	DWORD dwType;
	WORD nYear;
	BYTE nMonth;
	BYTE nDay;
} CRACK_ORDER_PACKET, *PCRACK_ORDER_PACKET;

typedef struct _CRACK_DATA_PACKET{
	DWORD nLen;//자료길이
	DWORD dwType; // 자료형태
	BYTE* pData;
}CRACK_DATA_PACKET,*PCRACK_DATA_PACKET;


#define	_key_type_multiip				0
#define	_key_type_multidevkey			1
#define	_key_type_dberror				2
#define	_key_type_packeterror			3
#define	_key_type_idfail				4
#define	_key_type_pwdfail				5


char g_szLogKey[MAX_LOG_KEY][50] = {
	LOG_KEY_MULTIIP,			
	LOG_KEY_MULTIDEVKEY,		
	LOG_KEY_DBERROR,			
	LOG_KEY_PACKETERROR,			
	LOG_KEY_IDFAILED,		
	LOG_KEY_PWDFAILED,
};

typedef enum _ORDER_TYPE {
	OrderTypeMain = 0, // 통합서버
	OrderTypeStatus = 1, // 사용상태
	OrderTypeAccount = 2, // 계정
	OrderTypeUseTime = 3, // 사용시간
	OrderTypeMoney = 4, // 게임머니
	OrderTypeIssue = 5, // 발급리력
	OrderTypeCrack = 6, // 크랙체크
	OrderTypeDownloadImage = 7, // 이미지폴더 다운로드
	OrderTypeDeleteImage = 8, // 이미지파일 전부삭제
	OrderTypeLog = 9, // 상태로그
} ORDER_TYPE, *PORDER_TYPE;

char g_szServerLogPath[MAX_PATH] = {0}; 
char g_szImagePath[MAX_PATH] = {0};

extern CCrackCheckEngine* m_pCrackCheckEngine;
extern void WriteAccountInfo();

#define RC4_KEYLEN1	20
#define swap_byte1(x,y) t = *(x); *(x) = *(y); *(y) = t

typedef struct rc4_key1
{     
	unsigned char x;        
	unsigned char y;
	unsigned char state[256];       
} rc4_key1;


void prepare_key1(unsigned char *key_data_ptr, int key_data_len, rc4_key1 *key)
{
	unsigned char t;
	unsigned char index1;
	unsigned char index2;
	unsigned char* state;
	short counter;

	state = &key->state[0];
	for(counter = 0; counter < 256; counter++)
		state[counter] = (BYTE)counter;
	key->x = 0;
	key->y = 0;
	index1 = 0;
	index2 = 0;
	for(counter = 0; counter < 256; counter++)
	{
		index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
		swap_byte1(&state[counter], &state[index2]);
		index1 = (index1 + 1) % key_data_len;
	}
}

void rc41(unsigned char *buffer_ptr, int buffer_len, rc4_key1 *key)
{
	unsigned char t;
	unsigned char x;
	unsigned char y;
	unsigned char* state;
	unsigned char xorIndex;
	int counter;

	x = key->x;
	y = key->y;
	state = &key->state[0];
	for(counter = 0; counter < buffer_len; counter++)
	{
		x = (x + 1) % 256;
		y = (state[x] + y) % 256;
		swap_byte1(&state[x], &state[y]);
		xorIndex = (state[x] + state[y]) % 256;
		buffer_ptr[counter] ^= state[xorIndex];
	}
	key->x = x;
	key->y = y;
}

void rc41Convert(LPBYTE pbybuf, int nbuflen)
{
	rc4_key1 key;
	BYTE	initKey[RC4_KEYLEN1] = {
		0xCD, 0x38, 0xE6, 0x0C, 0x3D, 0x21, 0x5F, 0x1A, 0x20, 0xAA, 
		0x11, 0x73, 0x6C, 0xFF, 0x7A, 0xC6, 0x37, 0x66, 0x1F, 0xD9, 
	};

	prepare_key1(initKey, RC4_KEYLEN1, &key);
	rc41(pbybuf, nbuflen, &key);

}

void CrackCloseSocket(SOCKET* pSockComm)
{
	if( shutdown( *pSockComm, SD_BOTH ) != SOCKET_ERROR )
	{  
		fd_set  readfds;
		fd_set  errorfds;
		timeval timeout;

		FD_ZERO( &readfds );
		FD_ZERO( &errorfds );
		FD_SET( *pSockComm, &readfds );
		FD_SET( *pSockComm, &errorfds );

		timeout.tv_sec  = 10;//10s대기
		timeout.tv_usec = 0;

		select( 1, &readfds, NULL, &errorfds, &timeout );
	}
	closesocket(*pSockComm);

}

BOOL SendData(SOCKET* pSock, PCRACK_DATA_PACKET pDataPacket)
{
	int nLen, nnLen;

	if (send(*pSock, (char *)(&pDataPacket->nLen), 8, 0) != 8) // 길이정보와 형태정보를 보내기
	{
		CrackCloseSocket(pSock);
		delete pSock;
		return FALSE;
	}

	nLen=0;
	if (pDataPacket->nLen > 0)
	{
		while(nLen<(int)pDataPacket->nLen)
		{
			nnLen=send(*pSock, ((char *)(pDataPacket->pData))+nLen, min(pDataPacket->nLen-nLen,500), 0);
			if(nnLen<=0)
			{
				CrackCloseSocket(pSock);
				delete pSock;
				return FALSE;
			}
			nLen+=nnLen;
		}
	}
	return TRUE;
}

BOOL SendLog(SOCKET* pSock, CRACK_ORDER_PACKET orderPacket, LPSTR lpLogName)
{
	char szFile[MAX_PATH] = {0};
	FILE* fp;
	BYTE* pBuffer = NULL;
	CRACK_DATA_PACKET dataPacket;
	DWORD fileLen = 0;

	sprintf_s(szFile, MAX_PATH, "%s\\%s_%04d_%02d_%02d.log", g_szServerLogPath, lpLogName, orderPacket.nYear, orderPacket.nMonth, orderPacket.nDay);
	fopen_s(&fp, szFile, "rb");
	if (fp)
	{
		fileLen = _filelength(_fileno(fp));
		pBuffer = new BYTE [fileLen];
		fread(pBuffer, 1, fileLen, fp);
		fclose(fp);
	}

	dataPacket.dwType = orderPacket.dwType;
	dataPacket.nLen = fileLen;
	if (fileLen > 0) dataPacket.pData = pBuffer;
	if (SendData(pSock, &dataPacket) == FALSE)
	{
		if (pBuffer) delete pBuffer;
		return FALSE;
	}
	if (pBuffer) 
	{
		delete pBuffer;
		pBuffer = NULL;
	}

	return TRUE;
}

BOOL SendImage(SOCKET* pSock)
{
	CRACK_DATA_PACKET dataPacket;

	TCHAR szPath[MAX_PATH];
	wcscpy(szPath, g_szServerDataFilePath);
	_tcscat(szPath, _T("Image\\*.*"));
	CFileFind cfindfile;
	BOOL bFileExist = false;
	bFileExist = cfindfile.FindFile(szPath);
	while(bFileExist)
	{
		bFileExist = cfindfile.FindNextFile();
		if(cfindfile.IsDots())continue;
		if(!(cfindfile.IsDirectory()))
		{
			CFile cf;
			if(cf.Open(cfindfile.GetFilePath(), CFile::modeRead|CFile::typeBinary, NULL))
			{
				int nLen = (int)cf.GetLength();
				LPBYTE pBuf = new BYTE[200 + nLen];
				memset(pBuf, 0, 200 + nLen);
				CString strFIleName = cfindfile.GetFileName();
				CT2A ct2a(strFIleName.GetBuffer());
				memcpy(pBuf, ct2a.m_psz, min(200, 2 * (strlen(ct2a.m_psz) + 1)));
				cf.Read(pBuf + 200, nLen);
				cf.Close();
				dataPacket.dwType = OrderTypeDownloadImage;
				dataPacket.nLen = nLen + 200;
				dataPacket.pData = pBuf;
				SendData(pSock, &dataPacket);
				delete pBuf;
			}
		}
	}
	// 마감패킷송신
	dataPacket.dwType = OrderTypeDownloadImage;
	dataPacket.nLen = 0;
	dataPacket.pData = 0;
	SendData(pSock, &dataPacket);

	return TRUE;
}

BOOL DeleteImage()
{
	TCHAR szPath[MAX_PATH];
	wcscpy(szPath, g_szServerDataFilePath);
	_tcscat(szPath, _T("Image\\*.*"));
	CFileFind cfindfile;
	BOOL bFileExist = false;
	bFileExist = cfindfile.FindFile(szPath);
	while(bFileExist)
	{
		bFileExist = cfindfile.FindNextFile();
		if(cfindfile.IsDots())continue;
		if(!(cfindfile.IsDirectory()))
		{
			DeleteFile(cfindfile.GetFilePath());
		}
	}
	return TRUE;
}

DWORD  AccountCommThread(LPVOID pParam)
{
	SOCKET	*pSock = (SOCKET *)pParam;
	int  nTime = 30000;
	int nLen,nnLen;

	//접속아이피구하기
	SOCKADDR_IN sa;
	int len = sizeof(sa);
	getpeername(*pSock, (sockaddr *)&sa, &len);
	CHAR *lpAddress = inet_ntoa(sa.sin_addr);
	char szAddress[20] = {0};
	strcpy_s(szAddress, 20, lpAddress);

	setsockopt(*pSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTime,4);

	//지령수신
	nLen = 0;
	CRACK_ORDER_PACKET orderPacket;
	while(nLen < sizeof(CRACK_ORDER_PACKET))
	{
		nnLen = recv(*pSock, ((char *)&orderPacket)+nLen, sizeof(CRACK_ORDER_PACKET)-nLen, 0);
		if(nnLen <=0)
		{
			CrackCloseSocket(pSock);
			delete pSock;
			return 0;
		}
		nLen += nnLen;
	}

	switch (orderPacket.dwType)
	{
	case OrderTypeMain: // 통합서버
		SendLog(pSock, orderPacket, "Main");
		break;
	case OrderTypeStatus: // 사용상태
		WriteAccountInfo();
		SendLog(pSock, orderPacket, "Account");
		SendLog(pSock, orderPacket, "UseTime");
		SendLog(pSock, orderPacket, "Block");
		SendLog(pSock, orderPacket, "Gold");
		SendLog(pSock, orderPacket, "Issue");
		break;
	case OrderTypeAccount: // 계정
		WriteAccountInfo();
		SendLog(pSock, orderPacket, "Account");
		break;
	case OrderTypeUseTime: // 사용시간
		SendLog(pSock, orderPacket, "UseTime");
		break;
	case OrderTypeMoney: // 게임머니
		SendLog(pSock, orderPacket, "Gold");
		break;
	case OrderTypeIssue: // 발급리력
		SendLog(pSock, orderPacket, "Issue");
		break;
	case OrderTypeCrack: // 크랙체크
		SendLog(pSock, orderPacket, "Abnormal");
		SendLog(pSock, orderPacket, "Crack");
		break;
	case OrderTypeDownloadImage: // 이미지폴더 다운로드
		SendImage(pSock);
		break;
	case OrderTypeDeleteImage: // 이미지파일 전부삭제
		DeleteImage();
		break;
	case OrderTypeLog: // 상태체크
		SendLog(pSock, orderPacket, "Log");
		break;
	}

	CrackCloseSocket(pSock);
	return 1;
}


DWORD WINAPI AccountServerThread(LPVOID Context)
{
	sockaddr_in		addrServer;
	SOCKET			sockServer;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(ACCOUNT_SERVER_PORT);
	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockServer == INVALID_SOCKET)
	{
		return 0;
	}

	while(1)
	{
		if(bind(sockServer, (sockaddr*)&addrServer, sizeof(addrServer)))
		{
			Sleep(500);
			continue;
		}
		break;
	}


	if(listen(sockServer, SOMAXCONN))
	{
		return 0;
	}

	sockaddr_in		addrFrom;
	int				nFromLen;

	nFromLen = sizeof(addrFrom);
	while(1)
	{
		SOCKET		*pSocketComm = new SOCKET;

		*pSocketComm = accept(sockServer, (struct sockaddr*)&addrFrom, &nFromLen);
		if(*pSocketComm == INVALID_SOCKET)
		{
			return 0;
		}

		HANDLE han = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AccountCommThread, (LPVOID)pSocketComm, NULL, NULL);
		if(han)
		{
			CloseHandle(han);
		}
		else
		{
			return 0;
		}
	}

	return 0;
}

DWORD  InfoCommThread(LPVOID pParam)
{
	SOCKET	*pSock = (SOCKET *)pParam;
	int  nTime = 30000;

	//접속아이피구하기
	SOCKADDR_IN sa;
	int len = sizeof(sa);
	getpeername(*pSock, (sockaddr *)&sa, &len);
	CHAR *lpAddress = inet_ntoa(sa.sin_addr);
	char szIp[20] = {0};
	strcpy_s(szIp, 20, lpAddress);

	setsockopt(*pSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTime,4);

	int nLens[10] = {0};
	int nnLen = 0, nLen = 0;
	char szFileName[MAX_PATH] = {0};
	FILE* fp = NULL;
	SYSTEMTIME localTime;
	int err;

	int nCount = 0;
	nLen=0;
	while(nLen<8)
	{
		nnLen = recv(*pSock, (char*)nLens, 8-nLen, 0);
		if (nnLen <= 0)
		{
			err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK && nCount<15)
			{
				Sleep(1000);
				nCount++;
				continue;
			}

			CrackCloseSocket(pSock);
			return 0;
		}
		nLen+=nnLen;
	}


	int nRecvLen = 0;

	nRecvLen = nLens[1];


	char* szRecvBuffer =new char[nRecvLen];
	if (szRecvBuffer)
	{
		nLen=0;
		nCount = 0;
		while(nLen<nRecvLen)
		{
			nnLen = recv(*pSock, szRecvBuffer+nLen, min(nRecvLen-nLen, 1000), 0);
			if (nnLen <= 0)
			{
				err = WSAGetLastError();
				if (err == WSAEWOULDBLOCK && nCount<15)
				{
					Sleep(1000);
					nCount++;
					continue;
				}


				delete szRecvBuffer;
				CrackCloseSocket(pSock);
				return 0;
			}
			nLen+=nnLen;
		}

		rc41Convert((BYTE*)szRecvBuffer, nRecvLen);

		if (nLens[0] == 0)
		{
			// 프로쎄서 목록 얻기
			GetLocalTime(&localTime);
			sprintf_s(szFileName, MAX_PATH, "%s\\%s_%02d_%02d_%02d_%02d_%02d.txt", g_szImagePath, szIp, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond);
			fopen_s(&fp, szFileName, "wb");
			if (fp)
			{
				fwrite(szRecvBuffer, nLens[1], 1, fp);
				fclose(fp);
			}
		}
		else if (nLens[0] == 1)
		{
			// 캡쳐화면 얻기
			GetLocalTime(&localTime);
			sprintf_s(szFileName, MAX_PATH, "%s\\%s_%02d_%02d_%02d_%02d_%02d.jpg", g_szImagePath, szIp, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond);
			fopen_s(&fp, szFileName, "wb");
			if (fp)
			{
				fwrite(szRecvBuffer, nLens[1], 1, fp);
				fclose(fp);
			}

		}
		else if (nLens[0] == 2)
		{
			// 프로쎄서 목록 얻기
			GetLocalTime(&localTime);
			sprintf_s(szFileName, MAX_PATH, "%s\\tree_%s_%02d_%02d_%02d_%02d_%02d.txt", g_szImagePath, szIp, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond);
			fopen_s(&fp, szFileName, "wb");
			if (fp)
			{
				fwrite(szRecvBuffer, nLens[1], 1, fp);
				fclose(fp);
			}
		}

		delete szRecvBuffer;
	}

	CrackCloseSocket(pSock);
	return 1;
}


DWORD WINAPI InfoServerThread(LPVOID Context)
{
	sockaddr_in		addrServer;
	SOCKET			sockServer;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = INADDR_ANY;
	addrServer.sin_port = htons(CRACKINFO_PORT);
	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockServer == INVALID_SOCKET)
	{
		return 0;
	}

	while(1)
	{
		if(bind(sockServer, (sockaddr*)&addrServer, sizeof(addrServer)))
		{
			Sleep(500);
			continue;
		}
		break;
	}


	if(listen(sockServer, SOMAXCONN))
	{
		return 0;
	}

	sockaddr_in		addrFrom;
	int				nFromLen;

	nFromLen = sizeof(addrFrom);
	while(1)
	{
		SOCKET		*pSocketComm = new SOCKET;

		*pSocketComm = accept(sockServer, (struct sockaddr*)&addrFrom, &nFromLen);
		if(*pSocketComm == INVALID_SOCKET)
		{
			return 0;
		}

		HANDLE han = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InfoCommThread, (LPVOID)pSocketComm, NULL, NULL);
		if(han)
		{
			CloseHandle(han);
		}
		else
		{
			return 0;
		}
	}

	return 0;
}

BOOL WritePrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, int nValue, LPCSTR lpFileName)
{
	CHAR szValue[50];
	sprintf_s(szValue, 50, "%d", nValue);

	return WritePrivateProfileStringA(lpAppName, lpKeyName, szValue, lpFileName);
}

CCrackCheckEngine::CCrackCheckEngine(void)
{
}

CCrackCheckEngine::~CCrackCheckEngine(void)
{
}

// 초기화함수
BOOL CCrackCheckEngine::Init(void)
{
	CHAR szPath[MAX_PATH];
	CW2A cw2a(g_szServerDataFilePath);
	strcpy(szPath, cw2a.m_psz);
	CHAR *p = strrchr(szPath, '\\');
	if (p) p[0] = 0;
	sprintf_s(g_szImagePath, "%s\\%s", szPath, IMAGE_FOLDER_NAME);
	sprintf_s(g_szServerLogPath, "%s\\%s", szPath, LOG_FOLDER_NAME);

	// 폴더를 만든다.
	if (CreateDirectoryA(g_szServerLogPath, NULL) == FALSE)
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			return FALSE;
		}
	}

	if (CreateDirectoryA(g_szImagePath, NULL) == FALSE)
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			return FALSE;
		}
	}

	InitializeCriticalSection(&m_csMainLog);
	InitializeCriticalSection(&m_csStatusLog);
	InitializeCriticalSection(&m_csBlockLog);
	InitializeCriticalSection(&m_csUseTimeLog);
	InitializeCriticalSection(&m_csGoldLog);
	InitializeCriticalSection(&m_csIssueLog);
	InitializeCriticalSection(&m_csCrackLog);
	InitializeCriticalSection(&m_csAbnormalLog);
	InitializeCriticalSection(&m_csLog);

	HANDLE hAccountThread = CreateThread(NULL, 0, AccountServerThread, NULL, 0, NULL);
	if (hAccountThread) CloseHandle(hAccountThread);

	HANDLE hInfoThread = CreateThread(NULL, 0, InfoServerThread, NULL, 0, NULL);
	if (hInfoThread) CloseHandle(hInfoThread);

	return TRUE;
}


#pragma region 통합서버

void CCrackCheckEngine:: WriteMainLogA(char* szIP, char* szName, BOOL bNormal)
{
	char szBuffer[500] = {0};

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	sprintf_s(szBuffer, 500, "Time=%02d:%02d:%02d IP=%s Name=%s Normal=%s", LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, szIP, szName, bNormal ? "정상" : "비정상");

	EnterCriticalSection(&m_csMainLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Main_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);
	FILE* fp = NULL;
	fopen_s(&fp, szFileName, "a+");
	if (fp)
	{
		fprintf(fp, "%s\n", szBuffer);
		fclose(fp);
	}
	LeaveCriticalSection(&m_csMainLog);
}

void CCrackCheckEngine:: WriteMainLogW(WCHAR* szIP, WCHAR* szName, BOOL bNormal)
{
	char aszIP[100] = {0}, aszName[100] = {0};
	sprintf_s(aszIP, 100, "%ws", szIP);
	sprintf_s(aszName, 100, "%ws", szName);
	WriteMainLogA(aszIP, aszName, bNormal);
}

#pragma endregion

#pragma region 사용상태

void CCrackCheckEngine::ConnectAccountNumber(SYSTEMTIME LocalTime, int number, int number6, int number10)
{
	EnterCriticalSection(&m_csStatusLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Status_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	WritePrivateProfileIntA(CONNECTACCOUNT_SECTION, COUNT_KEYNAME, number, szFileName);
	WritePrivateProfileIntA(CONNECTACCOUNT_SECTION, COUNT6_KEYNAME, number6, szFileName);
	WritePrivateProfileIntA(CONNECTACCOUNT_SECTION, COUNT10_KEYNAME, number10, szFileName);

	LeaveCriticalSection(&m_csStatusLog);
}


void CCrackCheckEngine::ValidAccountNumber(SYSTEMTIME LocalTime, int number, int number6, int number10)
{
	EnterCriticalSection(&m_csStatusLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Status_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	WritePrivateProfileIntA(VALIDACCOUNT_SECTION, COUNT_KEYNAME, number, szFileName);
	WritePrivateProfileIntA(VALIDACCOUNT_SECTION, COUNT6_KEYNAME, number6, szFileName);
	WritePrivateProfileIntA(VALIDACCOUNT_SECTION, COUNT10_KEYNAME, number10, szFileName);

	LeaveCriticalSection(&m_csStatusLog);
}


void CCrackCheckEngine::BlockAccountNumber(SYSTEMTIME LocalTime, int number)
{
	EnterCriticalSection(&m_csStatusLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Status_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	WritePrivateProfileIntA(BLOCKACCOUNT_SECTION, COUNT_KEYNAME, number, szFileName);

	LeaveCriticalSection(&m_csStatusLog);
}


void CCrackCheckEngine::AverageConnectTime(SYSTEMTIME LocalTime, int nMinute)
{
	EnterCriticalSection(&m_csStatusLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Status_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	WritePrivateProfileIntA(AVGCONNECTTIME_SECTION, COUNT_KEYNAME, nMinute, szFileName);

	LeaveCriticalSection(&m_csStatusLog);
}

void CCrackCheckEngine::PostGoldAmount(SYSTEMTIME LocalTime, ULONGLONG ullPostGold)
{
	EnterCriticalSection(&m_csStatusLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Status_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	char  szValue[100] = {0};
	sprintf_s(szValue, 100, "%I64d", ullPostGold);
	WritePrivateProfileStringA(POSTGOLD_SECTION, COUNT_KEYNAME, szValue, szFileName);

	LeaveCriticalSection(&m_csStatusLog);
}

void CCrackCheckEngine::WriteBlockA(SYSTEMTIME LocalTime, char* szName, int nCount)
{
	EnterCriticalSection(&m_csBlockLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Block_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);
	WritePrivateProfileIntA(BLOCKACCOUNT_SECTION, szName, nCount, szFileName);
	LeaveCriticalSection(&m_csBlockLog);
}

void CCrackCheckEngine::WriteBlockW(SYSTEMTIME LocalTime, WCHAR* szName, int nCount)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	WriteBlockA(LocalTime, szAccount, nCount);
}

void CCrackCheckEngine::AddBlockA(SYSTEMTIME LocalTime, char* szName, int nCount)
{
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Block_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	EnterCriticalSection(&m_csBlockLog);

	nCount += GetPrivateProfileIntA(BLOCKACCOUNT_SECTION, szName, 0, szFileName);

	LeaveCriticalSection(&m_csBlockLog);

	WriteBlockA(LocalTime, szName, nCount);
}

void CCrackCheckEngine::AddBlockW(SYSTEMTIME LocalTime, WCHAR* szName, int nCount)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	AddBlockA(LocalTime, szAccount, nCount);
}

#pragma endregion

int GetBufferValue(LPSTR lpBuffer, LPSTR lpValue)
{
	int nValue = 0;

	CHAR *p = StrStrIA(lpBuffer, lpValue);
	if (p)
	{
		nValue = atoi(p + strlen(lpValue));
	}

	return nValue;
}

#pragma region 사용시간

void CCrackCheckEngine::WriteUseTimeA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount)
{
	char szBuffer[500] = {0};

	sprintf_s(szBuffer, 500, "Multi=%d UseTime=%d", nMulti, nCount);

	EnterCriticalSection(&m_csUseTimeLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\UseTime_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);
	WritePrivateProfileStringA(USETIME_SECTION, szName, szBuffer, szFileName);
	LeaveCriticalSection(&m_csUseTimeLog);
}

void CCrackCheckEngine::WriteUseTimeW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	WriteUseTimeA(LocalTime, szAccount, nMulti, nCount);
}

void CCrackCheckEngine::AddUseTimeA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount)
{
	char szBuffer[500] = {0};

	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\UseTime_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	EnterCriticalSection(&m_csUseTimeLog);

	GetPrivateProfileStringA(USETIME_SECTION, szName, "", szBuffer, 500, szFileName);

	nCount += GetBufferValue(szBuffer, "UseTime=");

	LeaveCriticalSection(&m_csUseTimeLog);

	WriteUseTimeA(LocalTime, szName, nMulti, nCount);
}

void CCrackCheckEngine::AddUseTimeW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	AddUseTimeA(LocalTime, szAccount, nMulti, nCount);
}

#pragma endregion

#pragma region 게임머니

void CCrackCheckEngine::WriteGoldA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount)
{
	char szBuffer[500] = {0};

	sprintf_s(szBuffer, 500, "Multi=%d Gold=%d", nMulti, nCount);

	EnterCriticalSection(&m_csGoldLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Gold_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);
	WritePrivateProfileStringA(POSTGOLD_SECTION, szName, szBuffer, szFileName);
	LeaveCriticalSection(&m_csGoldLog);
}

void CCrackCheckEngine::WriteGoldW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	WriteGoldA(LocalTime, szAccount, nMulti, nCount);
}

void CCrackCheckEngine::AddGoldA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount)
{
	char szBuffer[500] = {0};

	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Gold_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	EnterCriticalSection(&m_csGoldLog);

	GetPrivateProfileStringA(POSTGOLD_SECTION, szName, "", szBuffer, 500, szFileName);

	nCount += GetBufferValue(szBuffer, "Gold=");

	LeaveCriticalSection(&m_csGoldLog);

	WriteGoldA(LocalTime, szName, nMulti, nCount);
}

void CCrackCheckEngine::AddGoldW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	AddGoldA(LocalTime, szAccount, nMulti, nCount);
}

#pragma endregion

#pragma region 발급리력

void CCrackCheckEngine::AddAccountA(char* szName, CTime startDate, CTime endDate, int nAccountInfo)
{
	CTime c=CTime::GetCurrentTime();
	CTime cDate(c.GetYear(),c.GetMonth(),c.GetDay(),0,0,0);
	CTime sDate(startDate.GetYear(),startDate.GetMonth(),startDate.GetDay(),0,0,0);
	CTime eDate(endDate.GetYear(),endDate.GetMonth(),endDate.GetDay(),0,0,0);

	if(sDate<cDate)sDate=cDate;
	if(eDate<cDate)eDate=cDate;
	if(eDate<sDate)eDate=sDate;
	CTimeSpan dd=eDate-sDate;

	int nDays = (int)dd.GetDays()+1;

	AddAccountWriteFile(szName, nDays, nAccountInfo);
}

void CCrackCheckEngine::AddAccountW(WCHAR* szName, CTime startDate, CTime endDate, int nAccountInfo)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	AddAccountA(szAccount, startDate, endDate, nAccountInfo);
}

void CCrackCheckEngine::AddAccountWriteFile(char* szName, int nDays, int nAccountInfo)
{
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	EnterCriticalSection(&m_csIssueLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Issue_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	char szSectionName[MAX_PATH] = {0};
	UINT nCount = 0;
	if (nDays >= 0) sprintf_s(szSectionName, MAX_PATH, "%s+%d", ADDACCOUNT_SECTION, nDays);
	else sprintf_s(szSectionName, MAX_PATH, "%s-%d", ADDACCOUNT_SECTION, (-1)*nDays);

	if (nAccountInfo)
	{
		char szAccountInfo[10];
		sprintf(szAccountInfo, "(%d)", nAccountInfo);
		strcat_s(szSectionName, MAX_PATH, szAccountInfo);
	}
	strcat_s(szSectionName, MAX_PATH, SECTION_SUFFIX);

	nCount = GetPrivateProfileIntA(szSectionName, COUNT_KEYNAME, 0, szFileName);
	nCount ++;
	WritePrivateProfileIntA(szSectionName, COUNT_KEYNAME, nCount, szFileName);
	WritePrivateProfileIntA(szSectionName, szName, nDays, szFileName);

	LeaveCriticalSection(&m_csIssueLog);
}

void CCrackCheckEngine::DeleteAccountA(char* szName, CTime startDate, CTime endDate, int nAccountInfo)
{
	CTime c=CTime::GetCurrentTime();
	CTime cDate(c.GetYear(),c.GetMonth(),c.GetDay(),0,0,0);
	CTime sDate(startDate.GetYear(),startDate.GetMonth(),startDate.GetDay(),0,0,0);
	CTime eDate(endDate.GetYear(),endDate.GetMonth(),endDate.GetDay(),0,0,0);

	if(sDate<cDate)sDate=cDate;
	if(eDate<cDate)eDate=cDate;
	if(eDate<sDate)eDate=sDate;
	CTimeSpan dd=eDate-sDate;

	int nDays = (int)dd.GetDays();
	if(sDate>cDate)nDays++;

	DeleteAccountWriteFile(szName, nDays, nAccountInfo);
}

void CCrackCheckEngine::DeleteAccountW(WCHAR* szName, CTime startDate, CTime endDate, int nAccountInfo)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	DeleteAccountA(szAccount, startDate, endDate, nAccountInfo);
}

void CCrackCheckEngine::DeleteAccountWriteFile(char* szName, int nDays, int nAccountInfo)
{
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	EnterCriticalSection(&m_csIssueLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Issue_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	char szSectionName[MAX_PATH] = {0};
	UINT nCount = 0;
	if (nDays < 0) nDays = 0;
	sprintf_s(szSectionName, MAX_PATH, "%s_%d", DELACCOUNT_SECTION, nDays);

	if (nAccountInfo)
	{
		char szAccountInfo[10];
		sprintf(szAccountInfo, "(%d)", nAccountInfo);
		strcat_s(szSectionName, MAX_PATH, szAccountInfo);
	}
	strcat_s(szSectionName, MAX_PATH, SECTION_SUFFIX);

	nCount = GetPrivateProfileIntA(szSectionName, COUNT_KEYNAME, 0, szFileName);
	nCount ++;
	WritePrivateProfileIntA(szSectionName, COUNT_KEYNAME, nCount, szFileName);
	WritePrivateProfileIntA(szSectionName, szName, nDays, szFileName);

	LeaveCriticalSection(&m_csIssueLog);
}

void CCrackCheckEngine::ExtendAccountA(char* szName, CTime prevEndDate, CTime endDate, int nAccountInfo)
{
	CTime c=CTime::GetCurrentTime();
	CTime cDate(c.GetYear(),c.GetMonth(),c.GetDay(),0,0,0);
	CTime sDate(prevEndDate.GetYear(),prevEndDate.GetMonth(),prevEndDate.GetDay(),0,0,0);
	CTime eDate(endDate.GetYear(),endDate.GetMonth(),endDate.GetDay(),0,0,0);

	if(sDate<cDate){sDate=cDate;sDate-=CTimeSpan(1,0,0,0);}
	if(eDate<cDate){eDate=cDate;eDate-=CTimeSpan(1,0,0,0);}

	CTimeSpan dd=eDate-sDate;

	int nDays = (int)dd.GetDays();

	ExtendAccountWriteFile(szName, nDays, nAccountInfo);
}

void CCrackCheckEngine::ExtendAccountW(WCHAR* szName, CTime prevEndDate, CTime endDate, int nAccountInfo)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	ExtendAccountA(szAccount, prevEndDate, endDate, nAccountInfo);
}

void CCrackCheckEngine::ExtendAccountWriteFile(char* szName, int nDays, int nAccountInfo)
{
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	EnterCriticalSection(&m_csIssueLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Issue_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	char szSectionName[MAX_PATH] = {0};
	UINT nCount = 0;
	if (nDays >= 0) sprintf_s(szSectionName, MAX_PATH, "%s+%d", EXTENDACCOUNT_SECTION, nDays);
	else sprintf_s(szSectionName, MAX_PATH, "%s-%d", EXTENDACCOUNT_SECTION, (-1)*nDays);

	if (nAccountInfo)
	{
		char szAccountInfo[10];
		sprintf(szAccountInfo, "(%d)", nAccountInfo);
		strcat_s(szSectionName, MAX_PATH, szAccountInfo);
	}
	strcat_s(szSectionName, MAX_PATH, SECTION_SUFFIX);

	nCount = GetPrivateProfileIntA(szSectionName, COUNT_KEYNAME, 0, szFileName);
	nCount ++;
	WritePrivateProfileIntA(szSectionName, COUNT_KEYNAME, nCount, szFileName);
	WritePrivateProfileIntA(szSectionName, szName, nDays, szFileName);

	LeaveCriticalSection(&m_csIssueLog);
}

#pragma endregion

#pragma region 크랙체크

void __cdecl CCrackCheckEngine:: WriteCrackLog(int nKeyType, PCHAR FormatString,...)
{
	char szBuffer[500] = {0};
	char szLog[500] = {0};
	va_list ap;

	va_start(ap, FormatString);
	vsnprintf_s(szBuffer, 500, 500, FormatString, ap);
	va_end(ap);

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	EnterCriticalSection(&m_csCrackLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Crack_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);
	FILE* fp = NULL;
	fopen_s(&fp, szFileName, "a+");
	if (fp)
	{
		sprintf_s(szLog, 500, "[%s] %02d:%02d:%02d %s", g_szLogKey[nKeyType], LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, szBuffer);
		fprintf(fp, "%s\n", szLog);
		fclose(fp);
	}
	LeaveCriticalSection(&m_csCrackLog);
}


void CCrackCheckEngine::WriteLogForPacketErrorA(char* szIp)
{
	WriteCrackLog(_key_type_packeterror, "ip=%s", szIp);
}

void CCrackCheckEngine::WriteLogForPacketErrorW(WCHAR* szIp)
{
	WriteCrackLog(_key_type_packeterror, "ip=%ws", szIp);
}

void CCrackCheckEngine::WriteLogForIdErrorA(char* szAccount, char* szIp)
{
	WriteCrackLog(_key_type_idfail, "id=%s ip=%s", szAccount, szIp);
}

void CCrackCheckEngine::WriteLogForIdErrorW(WCHAR* szAccount, WCHAR* szIp)
{
	WriteCrackLog(_key_type_idfail, "id=%ws ip=%ws", szAccount, szIp);
}

void CCrackCheckEngine::WriteLogForPwdErrorA(char* szAccount, char* szPwd, char* szIp)
{
	WriteCrackLog(_key_type_pwdfail, "id=%s pwd=%s ip=%s", szAccount, szPwd, szIp);
}

void CCrackCheckEngine::WriteLogForPwdErrorW(WCHAR* szAccount, WCHAR* szPwd, WCHAR* szIp)
{
	WriteCrackLog(_key_type_pwdfail, "id=%ws pwd=%ws ip=%ws", szAccount, szPwd, szIp);
}

void CCrackCheckEngine::WriteLogForLoginDBErrorA(char*szIp)
{
	WriteCrackLog(_key_type_dberror, "ip=%s", szIp);
}

void CCrackCheckEngine::WriteLogForLoginDBErrorW(WCHAR*szIp)
{
	WriteCrackLog(_key_type_dberror, "ip=%ws", szIp);
}

// bLogin: 첫 로그인이면 TRUE, 15분마다 정기적인 인증이면 FALSE
void CCrackCheckEngine::WriteLogForMultiDeviceKeyA(char* szAccout, char* szIp, BOOL bLogin)
{
	if (bLogin) WriteCrackLog(_key_type_multidevkey, "id=%s ip=%s", szAccout, szIp);
	else WriteCrackLog(_key_type_multidevkey, "id=%s ip=%s period", szAccout, szIp);
}

// bLogin: 첫 로그인이면 TRUE, 15분마다 정기적인 인증이면 FALSE
void CCrackCheckEngine::WriteLogForMultiDeviceKeyW(WCHAR* szAccout, WCHAR* szIp, BOOL bLogin)
{
	if (bLogin) WriteCrackLog(_key_type_multidevkey, "id=%ws ip=%ws", szAccout, szIp);
	else WriteCrackLog(_key_type_multidevkey, "id=%ws ip=%ws period", szAccout, szIp);
}

// bLogin: 첫 로그인이면 TRUE, 15분마다 정기적인 인증이면 FALSE
void CCrackCheckEngine::WriteLogForMultiIPA(char* szAccout, char* szIp, BOOL bLogin)
{
	if (bLogin) WriteCrackLog(_key_type_multiip, "id=%s ip=%s", szAccout, szIp);
	else WriteCrackLog(_key_type_multiip, "id=%s ip=%s period", szAccout, szIp);
}

// bLogin: 첫 로그인이면 TRUE, 15분마다 정기적인 인증이면 FALSE
void CCrackCheckEngine::WriteLogForMultiIPW(WCHAR* szAccout, WCHAR* szIp, BOOL bLogin)
{
	if (bLogin) WriteCrackLog(_key_type_multiip, "id=%ws ip=%ws", szAccout, szIp);
	else WriteCrackLog(_key_type_multiip, "id=%ws ip=%ws period", szAccout, szIp);
}

void CCrackCheckEngine::AbnormalLoginTryCountW(WCHAR* szName, int nCount)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	AbnormalLoginTryCountA(szAccount, nCount);
}

void CCrackCheckEngine::AbnormalLoginTryCountA(CHAR* szName, int nCount)
{
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	EnterCriticalSection(&m_csAbnormalLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Abnormal_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	WritePrivateProfileIntA(LOGINTRYCOUNT_SECTION, szName, nCount, szFileName);

	LeaveCriticalSection(&m_csAbnormalLog);
}

void CCrackCheckEngine::AbnormalUseTimeW(WCHAR* szName, int nCount)
{
	char szAccount[100] = {0};
	sprintf_s(szAccount, 100, "%ws", szName);
	AbnormalUseTimeA(szAccount, nCount);
}

void CCrackCheckEngine::AbnormalUseTimeA(CHAR* szName, int nCount)
{
	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	EnterCriticalSection(&m_csAbnormalLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Abnormal_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);

	WritePrivateProfileIntA(USETIME_SECTION, szName, nCount, szFileName);

	LeaveCriticalSection(&m_csAbnormalLog);
}

#pragma endregion

#pragma region 상태로그

void __cdecl CCrackCheckEngine:: WriteStatusLog(PCHAR FormatString,...)
{
	char szBuffer[500] = {0};
	char szLog[500] = {0};
	va_list ap;

	va_start(ap, FormatString);
	vsnprintf_s(szBuffer, 500, 500, FormatString, ap);
	va_end(ap);

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);

	EnterCriticalSection(&m_csLog);
	char szFileName[MAX_PATH];
	sprintf_s(szFileName, MAX_PATH, "%s\\Log_%04d_%02d_%02d.log", g_szServerLogPath, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay);
	FILE* fp = NULL;
	fopen_s(&fp, szFileName, "a+");
	if (fp)
	{
		sprintf_s(szLog, 500, "%02d:%02d:%02d %s", LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, szBuffer);
		fprintf(fp, "%s\n", szLog);
		fclose(fp);
	}
	LeaveCriticalSection(&m_csLog);
}

#pragma endregion
