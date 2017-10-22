// DlgLogin.cpp : implementation file
//

#include "stdafx.h"
#include "Admin.h"
#include "DlgLogin.h"
#include "AdminUpdateDlg.h"


// DlgLogin dialog

IMPLEMENT_DYNAMIC(CDlgLogin, CDialog)

CDlgLogin::CDlgLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgLogin::IDD, pParent)
	, m_szID(_T(""))
	, m_szPWD(_T(""))
{

}

CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ID, m_szID);
	DDX_Text(pDX, IDC_EDIT_PW, m_szPWD);
}


BEGIN_MESSAGE_MAP(CDlgLogin, CDialog)
	ON_BN_CLICKED(IDOK2, &CDlgLogin::OnBnClickedLogin)
	ON_BN_CLICKED(IDCANCEL, &CDlgLogin::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_MANAGE_ADMIN, &CDlgLogin::OnBnClickedManageAdmin)
END_MESSAGE_MAP()


// DlgLogin message handlers

void CDlgLogin::OnBnClickedLogin()
{
	if (CheckAdmin())
	{
		OnOK();
	}	
}

void GetLocalIP(DWORD* pIP)
{
	char	szName[128];
	DWORD	dwLen = sizeof(szName);
	*pIP = 0;
	if (gethostname(szName, dwLen) == SOCKET_ERROR)
	{
		if (!GetComputerNameA(szName, &dwLen))
			return;
	}
	hostent* pHostent = gethostbyname(szName);
	if (!pHostent)
		return;
	DWORD dwIP = 0;
	int i = 0;
	while (pHostent->h_addr_list[i] != 0)
	{
		memcpy(&dwIP, pHostent->h_addr_list[i], sizeof(int));
		dwIP = ntohl(dwIP);
		i ++;
	}
	*pIP = dwIP;
}

void MakeServerIP(char *szServerIP)
{
#ifdef _SVC_190
	DWORD dwIP;
	GetLocalIP(&dwIP);
	BYTE dwIP1, dwIP2, dwIP3, dwIP4;
	dwIP1 = (dwIP & 0xFF000000) >> 24;
	dwIP2 = (dwIP & 0x00FF0000) >> 16;
	dwIP3 = (dwIP & 0x0000FF00) >> 8;
	dwIP4 = (dwIP & 0x000000FF);
	if(dwIP3 == 1)
		sprintf(szServerIP, SERVER_IP);
	else
		sprintf(szServerIP, "%d.%d.%d.250", dwIP1, dwIP2, dwIP3);
#else
	sprintf(szServerIP, "%s", SERVER_IP);
#endif

}

BOOL CDlgLogin::CheckAdmin()
{
	UpdateData(TRUE);

	SOCKADDR_IN		saddr;

	if(!m_szID.GetLength())
	{
		AfxMessageBox(L"아이디를 입력하여주십시오.");
		return FALSE;
	}
	if(!m_szPWD.GetLength())
	{
		AfxMessageBox(L"비번을 입력하여주십시오.");
		return FALSE;
	}

	MakeServerIP(g_szServerIP);

	g_strAdminId = m_szID;
	g_strAdminPwd = m_szPWD;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"소켓창조가 실패하였습니다.");
		return FALSE;
	}
	else
	{
		saddr.sin_family = AF_INET;

		saddr.sin_addr.s_addr = inet_addr(g_szServerIP);
		saddr.sin_port = htons(SERVER_PORT_ADMINSECURE);

		if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)))
		{
			closesocket(sock);
			AfxMessageBox(L"서버접속이 실패하였습니다.");
			return FALSE;
		}
	}

	int				i, nLen, nLen2;
	BYTE			pbBuff[MAX_PATH];	
	BYTE			*pbBuff2 = NULL;
	WCHAR			szTemp[MAX_PATH];
	ACCOUNTINFO		*pAccountInfo;
	BYTE			*pbUserInfo = NULL;;

	nLen = MakePacket(pbBuff, "cdSS", OPCODE_ADMINCONN, 0, (LPCTSTR)m_szID, (LPCTSTR)m_szPWD);	
	SendData(sock, (char*)pbBuff, nLen, 0);

	nLen = RecvData(sock, (char*)pbBuff, 1, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return FALSE;
	}

	if( pbBuff[0] != OPCODE_ADMINCONN && pbBuff[0] != OPCODE_BOSSCONN)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return FALSE;
	}

	if(pbBuff[0] == OPCODE_BOSSCONN)
		g_bBossAdmin = TRUE;
	else
		g_bBossAdmin = FALSE;


	nLen = RecvData(sock, (char*)pbBuff, 4, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return FALSE;
	}

	nLen = *(int*)pbBuff;
	pbBuff2 = new BYTE[nLen];

	nLen2 = 0;
	while(nLen2 < nLen)
	{
		int nTemp;		
		nTemp = RecvData(sock, (char*)pbBuff2+nLen2, nLen-nLen2, 0);
		if(nTemp <= 0)
			break;
		nLen2 += nTemp;
	}

	if(nLen2 <= 0 || nLen2 != nLen)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return FALSE;
	}

	closesocket(sock);
	DecryptPacket(pbBuff2, nLen2);

	if(g_bBossAdmin)
	{
		//m_szAddress.Format(L"%x", *(DWORD*)(pbBuff2 + nLen2 - 4));
	}
	nLen = *(int*)pbBuff2;
	if(nLen == 0xFFFFFFFF)
	{
		AfxMessageBox(L"비번이 정확치 않습니다.");
		return FALSE;
	}
	if(nLen == 0xFFFFFFFE)
	{
		AfxMessageBox(L"다른 관리자가 현재 접속중입니다.");
		return FALSE;
	}

	delete pbBuff2;
	return TRUE;		
}

BOOL CDlgLogin::AdminListDownload()
{
	UpdateData(TRUE);

	SOCKADDR_IN		saddr;

	if(!m_szID.GetLength())
	{
		AfxMessageBox(L"아이디를 입력하여주십시오.");
		return FALSE;
	}
	if(!m_szPWD.GetLength())
	{
		AfxMessageBox(L"비번을 입력하여주십시오.");
		return FALSE;
	}

	MakeServerIP(g_szServerIP);

	g_strAdminId = m_szID;
	g_strAdminPwd = m_szPWD;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"소켓창조가 실패하였습니다.");
		return FALSE;
	}
	else
	{
		saddr.sin_family = AF_INET;



		saddr.sin_addr.s_addr = inet_addr(g_szServerIP);
		saddr.sin_port = htons(SERVER_PORT_ADMINSECURE);

		if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)))
		{
			closesocket(sock);
			AfxMessageBox(L"서버접속이 실패하였습니다.");
			return FALSE;
		}
	}

	int				i, nLen, nLen2;
	BYTE			pbBuff[MAX_PATH];	
	BYTE			*pbBuff2 = NULL;
	WCHAR			szTemp[MAX_PATH];
	ACCOUNTINFO		*pAccountInfo;
	BYTE			*pbUserInfo = NULL;;

	nLen = MakePacket(pbBuff, "cdSS", OPCODE_BOSSCONN, 0, (LPCTSTR)m_szID, (LPCTSTR)m_szPWD);	
	SendData(sock, (char*)pbBuff, nLen, 0);

	nLen = RecvData(sock, (char*)pbBuff, 1, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return FALSE;
	}

	if( pbBuff[0] != OPCODE_BOSSCONN)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return FALSE;
	}


	nLen = RecvData(sock, (char*)pbBuff, 4, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return FALSE;
	}

	nLen = *(int*)pbBuff;
	pbBuff2 = new BYTE[nLen];

	nLen2 = 0;
	while(nLen2 < nLen)
	{
		int nTemp;		
		nTemp = RecvData(sock, (char*)pbBuff2+nLen2, nLen-nLen2, 0);
		if(nTemp <= 0)
			break;
		nLen2 += nTemp;
	}

	if(nLen2 <= 0 || nLen2 != nLen)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return FALSE;
	}

	closesocket(sock);
	DecryptPacket(pbBuff2, nLen2);

	nLen = *(int*)pbBuff2;
	if(nLen == 0xFFFFFFFF)
	{
		AfxMessageBox(L"비번이 정확치 않습니다.");
		return FALSE;
	}
	if(nLen == 0xFFFFFFFE)
	{
		AfxMessageBox(L"다른 관리자가 현재 접속중입니다.");
		return FALSE;
	}

	g_nAdminNum = nLen;
	g_pbAdminInfo = new BYTE[sizeof(ADMININFO)*g_nAdminNum];
	ZeroMemory(g_pbAdminInfo, sizeof(ADMININFO)*g_nAdminNum);
	memcpy(g_pbAdminInfo, pbBuff2 + 4, sizeof(ADMININFO)*g_nAdminNum);

	delete pbBuff2;
	return TRUE;		
}

void CDlgLogin::OnBnClickedCancel()
{	
	OnCancel();
}

void CDlgLogin::OnBnClickedManageAdmin()
{
	if (AdminListDownload())
	{
		CAdminUpdateDlg	dlg;
		dlg.DoModal();
	}
}




