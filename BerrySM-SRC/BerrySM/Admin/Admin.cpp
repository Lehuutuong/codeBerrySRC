// Admin.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Admin.h"
#include "DlgLogin.h"
#include "AdminDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

char g_szServerIP[20] = "";

// CAdminApp

BEGIN_MESSAGE_MAP(CAdminApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAdminApp construction

CAdminApp::CAdminApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CAdminApp object

CAdminApp theApp;

CString				g_strAdminId;
CString				g_strAdminPwd;
ACCOUNTINFO			g_AccountInfo;
WCHAR				g_szNewPW[MAX_PATH];
BYTE*				g_pbAdminInfo = NULL;
int					g_nAdminNum = 0;
BOOL				g_bBossAdmin = FALSE;

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

void EncryptPacket(BYTE *pbData, int nLen)
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

void DecryptPacket(BYTE *pbData, int nLen)
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
	VMProtectBegin("MakePacket");
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
				nOffset += wcslen(szTemp)*2+2;
			}
			break;
		case 's':
			{
				char	*szTemp = va_arg(arg, char *);
				sprintf((char *)pbPacket+nOffset, "%s", szTemp);
				nOffset += 20/*strlen(szTemp)*/+1;
			}
			break;
		}
	}	
	va_end(arg);
	*(int *)(pbPacket+1) = nOffset-5;
	EncryptPacket(pbPacket+5, nOffset-5);
	VMProtectEnd();
	return nOffset;
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


int StringToBytes(BYTE *pbBuffer, const WCHAR *szBuffer)
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


// CAdminApp initialization

BOOL CAdminApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	
	CAdminDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
