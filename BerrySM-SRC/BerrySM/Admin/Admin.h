// Admin.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

extern char g_szServerIP[20];

// CAdminApp:
// See Admin.cpp for the implementation of this class
//

class CAdminApp : public CWinApp
{
public:
	CAdminApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

#define		MAX_ADMIN			100

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

struct AUTOCONTROLCMD
{
	WCHAR	szAccount[20];
	WCHAR	szPassword[20];
	DWORD   dwOpcode;
	DWORD   dwLocalIP;
	DWORD   dwInternetIP;
};

struct COLLECTIONINFO
{
	DWORD		dwAutoNum; //�����ȣ
	WCHAR		wszAccountName[50];//�������
	WCHAR		wszCharName[50];//ĳ����
	DWORD		CharNum;//ĳ����ȣ
	DWORD		CharLv;//ĳ������
	WCHAR		wszTime[50];//���Žð�
	DWORD		dwCollectCnt; //����ȸ��
	DWORD		dwCollectGold;//������
	WCHAR		wszCollectCharName[50];//����ĳ���̸�
};
extern ACCOUNTINFO			g_AccountInfo;
extern CAdminApp			theApp;
extern WCHAR				g_szNewPW[MAX_PATH];
extern COLLECTIONINFO		*g_CollectInfo;
extern BYTE*				g_pbAdminInfo;
extern int					g_nAdminNum;
extern BOOL					g_bBossAdmin;
extern CString				g_strAdminId;
extern CString				g_strAdminPwd;

void	BytesToString(WCHAR *szBuffer, BYTE *pbBuffer, int nLen);
int		StringToBytes(BYTE *pbBuffer, const WCHAR *szBuffer);
int		RecvData(SOCKET sock, char *buf, int len, int flags);
int		SendData(SOCKET sock, char *buf, int len, int flags);

extern int		MakePacket(BYTE *pbPacket, char *szFormat, ...);
extern	void EncryptPacket(BYTE *pbData, int nLen);
extern	void DecryptPacket(BYTE *pbData, int nLen);
