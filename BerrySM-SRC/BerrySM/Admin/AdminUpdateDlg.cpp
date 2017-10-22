// AdminDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Admin.h"
#include "AdminUpdateDlg.h"
#include "AdminDlg.h"

ADMININFO	g_AdminUpdateInfo;

IMPLEMENT_DYNAMIC(CAdminUpdateDlg, CDialog)

CAdminUpdateDlg::CAdminUpdateDlg(CWnd* pParent /*=NULL*/)
: CDialog(CAdminUpdateDlg::IDD, pParent)
, m_szID(_T(""))
, m_szPwd(_T(""))
, m_nIndex(0)
, m_nMaxUser(0)
, m_strKey(_T(""))
{

}

CAdminUpdateDlg::~CAdminUpdateDlg()
{
}

void CAdminUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LST_ADMIN, m_cAdminList);
	DDX_Text(pDX, IDC_EDT_ID, m_szID);
	DDX_Text(pDX, IDC_EDT_PWD, m_szPwd);
	DDX_Control(pDX, IDC_CMB_LEVEL, m_cLevel);
	DDX_Text(pDX, IDC_EDIT1, m_nMaxUser);
	DDX_Text(pDX, IDC_EDT_KEY, m_strKey);
	DDV_MaxChars(pDX, m_strKey, 16);
}


BEGIN_MESSAGE_MAP(CAdminUpdateDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_ADD, &CAdminUpdateDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_CHANGE, &CAdminUpdateDlg::OnBnClickedBtnChange)
	ON_BN_CLICKED(IDC_BTN_DEL, &CAdminUpdateDlg::OnBnClickedBtnDel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LST_ADMIN, &CAdminUpdateDlg::OnLvnItemchangedLstAdmin)
	ON_BN_CLICKED(IDCANCEL, &CAdminUpdateDlg::OnBnClickedCancel)
END_MESSAGE_MAP()



// �����߰�
void CAdminUpdateDlg::OnBnClickedBtnAdd()
{
	int			i = 0;
	CString		strTemp;

	if(g_nAdminNum >= MAX_ADMIN)
	{
		AfxMessageBox(L"��ϰ����� �����ڼ��� �ʰ��Ͽ����ϴ�.");
		return;
	}

	UpdateData(TRUE);

	if(m_szID.IsEmpty())
	{
		AfxMessageBox(L"���̵� �Է��Ͻʽÿ�.");
		return;
	}
	if(m_szID.GetLength() > 19)
	{
		AfxMessageBox(L"���̵�� 20�����Ϸ� �����ؾ� �մϴ�.");
		return;
	}
	if(m_szPwd.IsEmpty())
	{
		AfxMessageBox(L"����� �Է��Ͻʽÿ�.");
		return;
	}
	if(m_cLevel.GetCurSel() < 0)
	{
		AfxMessageBox(L"����� �����Ͻʽÿ�.");
		return;
	}
	if(m_szPwd.GetLength() < 5 || m_szPwd.GetLength() > 19)
	{
		AfxMessageBox(L"����� 5���̻�, 20�����Ϸ� �����Ͽ��� �մϴ�.");
		return;
	}

	if(m_strKey.IsEmpty())
	{
		m_strKey = L"0000000000000000";
		UpdateData(FALSE);
	}
	else if(m_strKey.GetLength() < 16)
	{
		AfxMessageBox(L"��Ϲ�ȣ�� 16�ڷ� �����Ͽ��� �մϴ�.");
		return;
	}

	for(i = 0 ; i < g_nAdminNum ; i++)
	{
		strTemp = m_cAdminList.GetItemText(i, 1);
		if(!wcscmp((LPCTSTR)strTemp, (LPCTSTR)m_szID))
			break;
	}

	if(i >= g_nAdminNum)
	{
		ZeroMemory(&g_AdminUpdateInfo, sizeof(ADMININFO));
		wcscpy(g_AdminUpdateInfo.szID, (LPCTSTR)m_szID);
		wcscpy(g_AdminUpdateInfo.szPwd, (LPCTSTR)m_szPwd);
		g_AdminUpdateInfo.dwLevel = m_cLevel.GetCurSel();
		g_AdminUpdateInfo.dwMaxUser = m_nMaxUser;
		StringToBytes(g_AdminUpdateInfo.pbAccount, m_strKey.GetBuffer(MAX_PATH));

		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(sock == INVALID_SOCKET)
		{
			AfxMessageBox(L"����â���� �����Ͽ����ϴ�.");
			return;
		}
		else
		{
			SOCKADDR_IN		saddr;
			saddr.sin_family = AF_INET;
			saddr.sin_addr.s_addr = inet_addr(g_szServerIP);
			saddr.sin_port = htons(SERVER_PORT_ADMINSECURE);

			if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)))
			{
				closesocket(sock);
				AfxMessageBox(L"���������� �����Ͽ����ϴ�.");
				return;
			}
		}

		BYTE	TbbySendBuffer[sizeof(ADMININFO) + 100];
		int nLen = MakePacket(TbbySendBuffer, "cddSSb", OPCODE_BOSS_ADMINUPDATE, 0, ADMININFO_ADD, g_strAdminId, g_strAdminPwd, &g_AdminUpdateInfo, sizeof(g_AdminUpdateInfo));

		if(nLen != SendData(sock, (char*)TbbySendBuffer, nLen, 0))
		{
			closesocket(sock);
			AfxMessageBox(SOCKET_ERROR_TEXT1);
			UpdateData(FALSE);
			return;
		}

		BYTE	pbRecvBuff[100];

		if(5 != RecvData(sock, (char*)pbRecvBuff, 5, 0) || *(DWORD*)(pbRecvBuff+1) != 0)
		{
			closesocket(sock);
			AfxMessageBox(SOCKET_ERROR_TEXT2);
			UpdateData(FALSE);
			return;
		}
		closesocket(sock);

		if(pbRecvBuff[0] == OPCODE_BOSS_ADMINUPDATE)
		{
			AfxMessageBox(L"�����ڰ����߰��� �Ϸ�Ǿ����ϴ�.");
			g_nAdminNum++;
			UpdateAdminInfo(g_nAdminNum-1);
			ResetIndex();
		}
		else if(pbRecvBuff[0] == OPCODE_FAILADD)
		{
			AfxMessageBox(L"�̹� �����ϴ� �����Դϴ�.");
		}
	}
	else
	{
		AfxMessageBox(L"�̹� �����ϴ� �����Դϴ�.");
	}
}

// ����
void CAdminUpdateDlg::OnBnClickedBtnChange()
{
	CString	strTemp;

	UpdateData(TRUE);

	if(m_szPwd.GetLength() < 5 || m_szPwd.GetLength() > 19)
	{
		AfxMessageBox(L"����� 5���̻�, 20�����Ϸ� �����Ͽ��� �մϴ�.");
		return;
	}
	if(m_strKey.IsEmpty())
	{
		m_strKey = L"0000000000000000";
	}
	else if(m_strKey.GetLength() < 16)
	{
		AfxMessageBox(L"��Ϲ�ȣ�� 16���̿��� �մϴ�.");
		return;
	}

	wcscpy(g_AdminUpdateInfo.szID, m_cAdminList.GetItemText(m_nIndex, 1).GetBuffer());
	wcscpy(g_AdminUpdateInfo.szPwd, (LPCTSTR)m_szPwd);
	g_AdminUpdateInfo.dwLevel = m_cLevel.GetCurSel();
	g_AdminUpdateInfo.dwMaxUser = m_nMaxUser;
	StringToBytes(g_AdminUpdateInfo.pbAccount, m_strKey.GetBuffer(MAX_PATH));

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"����â���� �����Ͽ����ϴ�.");
		return;
	}
	else
	{
		SOCKADDR_IN		saddr;
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(g_szServerIP);
		saddr.sin_port = htons(SERVER_PORT_ADMINSECURE);

		if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)))
		{
			closesocket(sock);
			AfxMessageBox(L"���������� �����Ͽ����ϴ�.");
			return;
		}
	}

	BYTE	TbbySendBuffer[sizeof(ADMININFO) + 100];
	int nLen = MakePacket(TbbySendBuffer, "cddSSb", OPCODE_BOSS_ADMINUPDATE, 0, ADMININFO_UPDATE, g_strAdminId, g_strAdminPwd, &g_AdminUpdateInfo, sizeof(g_AdminUpdateInfo));

	if(nLen != SendData(sock, (char*)TbbySendBuffer, nLen, 0))
	{
		closesocket(sock);
		AfxMessageBox(SOCKET_ERROR_TEXT1);
		UpdateData(FALSE);
		return;
	}

	BYTE	pbRecvBuff[100];

	if(5 != RecvData(sock, (char*)pbRecvBuff, 5, 0) || *(DWORD*)(pbRecvBuff+1) != 0)
	{
		closesocket(sock);
		AfxMessageBox(SOCKET_ERROR_TEXT2);
		UpdateData(FALSE);
		return;
	}

	closesocket(sock);

	if(pbRecvBuff[0] != OPCODE_BOSS_ADMINUPDATE)
	{
		AfxMessageBox(L"������Ʈ�� �����Ͽ����ϴ�.");
		UpdateData(FALSE);
		return;
	}

	AfxMessageBox(L"������������ ����Ǿ����ϴ�.");
	UpdateAdminInfo(m_nIndex);
	ResetIndex();
}

// ����
void CAdminUpdateDlg::OnBnClickedBtnDel()
{
	CString	strTemp;

	UpdateData(TRUE);

	if(!wcscmp(L"�ֻ���������", m_cAdminList.GetItemText(m_nIndex, 3)))
	{
		AfxMessageBox(L"�ֻ��������ڴ� ������ �� �����ϴ�.");
		return;
	}	

	ADMININFO AdminInfo;
	memset(&AdminInfo, 0, sizeof(AdminInfo));
	wcscpy(AdminInfo.szID, m_cAdminList.GetItemText(m_nIndex, 1).GetBuffer());

	LPBYTE	pbySendBuffer = NULL;
	int		nLen;

	//��������
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"����â���� �����Ͽ����ϴ�.");
		return;
	}
	else
	{
		SOCKADDR_IN		saddr;
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(g_szServerIP);
		saddr.sin_port = htons(SERVER_PORT_ADMINSECURE);

		if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)))
		{
			closesocket(sock);
			AfxMessageBox(L"���������� �����Ͽ����ϴ�.");
			return;
		}
	}

	BYTE TbbySendBuffer[sizeof(ADMININFO) + 100];
	nLen = MakePacket(TbbySendBuffer, "cddSSb", OPCODE_BOSS_ADMINUPDATE, 0, ADMININFO_DEL, g_strAdminId, g_strAdminPwd, &(AdminInfo), sizeof(AdminInfo));
	SendData(sock, (char*)TbbySendBuffer, nLen, 0);

	BYTE TbbyRecvBuffer[100];
	if(5 != RecvData(sock, (char*)TbbyRecvBuffer, 5, 0))
	{
		closesocket(sock);
		AfxMessageBox(SOCKET_ERROR_TEXT2);
		UpdateData(FALSE);
		return;
	}
	closesocket(sock);

	AfxMessageBox(L"�����ڰ� �����Ǿ����ϴ�.");

	m_cAdminList.DeleteItem(m_nIndex);
	m_nIndex = max(m_nIndex-1, 0);
	g_nAdminNum--;

	m_szID.Empty();
	m_szPwd.Empty();
	UpdateData(FALSE);

	ResetIndex();
}

BOOL CAdminUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	int i = 0;

	m_cAdminList.InsertColumn(1, L"��ȣ", LVCFMT_CENTER, 50);
	m_cAdminList.InsertColumn(2, L"���̵�", LVCFMT_CENTER, 100);
	m_cAdminList.InsertColumn(3, L"���", LVCFMT_CENTER, 100);
	m_cAdminList.InsertColumn(4, L"���", LVCFMT_CENTER, 100);
	m_cAdminList.InsertColumn(5, L"��ȿ������", LVCFMT_CENTER, 100);
	m_cAdminList.InsertColumn(6, L"��Ϲ�ȣ", LVCFMT_CENTER, 150);
	m_cAdminList.SetExtendedStyle(m_cAdminList.GetExtendedStyle()|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	m_cLevel.AddString(L"�ֻ���");
	m_cLevel.AddString(L"����");

	ADMININFO	*pAdminInfo; 
	WCHAR		szTemp[MAX_PATH] = {0,};

	if(g_nAdminNum)
	{
		for(i = 0 ; i < g_nAdminNum ; i++)
		{
			pAdminInfo = (ADMININFO*)(g_pbAdminInfo+i*sizeof(ADMININFO));

			swprintf(szTemp, L"%d", i+1);
			m_cAdminList.InsertItem(i, szTemp);

			swprintf(szTemp, L"%s", pAdminInfo->szID);
			m_cAdminList.SetItemText(i, 1, szTemp);

			swprintf(szTemp, L"%s", pAdminInfo->szPwd);
			m_cAdminList.SetItemText(i, 2, szTemp);

			if(pAdminInfo->dwLevel == 0)
				m_cAdminList.SetItemText(i, 3, L"�ֻ���������");
			else
				m_cAdminList.SetItemText(i, 3, L"����������");

			swprintf(szTemp, L"%d", pAdminInfo->dwMaxUser);
			m_cAdminList.SetItemText(i, 4, szTemp);

			BytesToString(szTemp, pAdminInfo->pbAccount, 8);
			m_cAdminList.SetItemText(i, 5, szTemp);

			m_cAdminList.SetItemData(i, (DWORD_PTR)i);
		}
	}

	(CWnd*)GetDlgItem(IDC_BTN_DEL)->EnableWindow(FALSE);
	(CWnd*)GetDlgItem(IDC_BTN_CHANGE)->EnableWindow(FALSE);

	return TRUE;
}

void CAdminUpdateDlg::UpdateAdminInfo(int nInd)
{
	WCHAR	szTemp[MAX_PATH];
	UpdateData(TRUE);

	swprintf(szTemp, L"%d", nInd+1);
	if(nInd >= m_cAdminList.GetItemCount())
		m_cAdminList.InsertItem(nInd, szTemp);
	else
		m_cAdminList.SetItemText(nInd, 0, szTemp);

	swprintf(szTemp, L"%s", g_AdminUpdateInfo.szID);
	m_cAdminList.SetItemText(nInd, 1, szTemp);
	swprintf(szTemp, L"%s", g_AdminUpdateInfo.szPwd);
	m_cAdminList.SetItemText(nInd, 2, szTemp);
	if(g_AdminUpdateInfo.dwLevel == 0)
		m_cAdminList.SetItemText(nInd, 3, L"�ֻ���������");
	else
		m_cAdminList.SetItemText(nInd, 3, L"����������");

	swprintf(szTemp, L"%d", g_AdminUpdateInfo.dwMaxUser);
	m_cAdminList.SetItemText(nInd, 4, szTemp);

	BytesToString(szTemp, g_AdminUpdateInfo.pbAccount, 8);
	m_cAdminList.SetItemText(nInd, 5, szTemp);

	m_cAdminList.SetItemData(nInd, nInd);
}

void CAdminUpdateDlg::OnLvnItemchangedLstAdmin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW	pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	POSITION		pos;

	pos = m_cAdminList.GetFirstSelectedItemPosition();
	if(!pos)
	{
		(CWnd*)GetDlgItem(IDC_BTN_DEL)->EnableWindow(FALSE);
		(CWnd*)GetDlgItem(IDC_BTN_CHANGE)->EnableWindow(FALSE);
		m_nIndex = -1;
		return;
	}

	(CWnd*)GetDlgItem(IDC_BTN_DEL)->EnableWindow(TRUE);
	(CWnd*)GetDlgItem(IDC_BTN_CHANGE)->EnableWindow(TRUE);

	m_nIndex = m_cAdminList.GetNextSelectedItem(pos);

	UpdateData(TRUE);
	m_szID = m_cAdminList.GetItemText(m_nIndex, 1);
	m_szPwd = m_cAdminList.GetItemText(m_nIndex, 2);
	CString strTemp = m_cAdminList.GetItemText(m_nIndex, 3);

	if(!wcscmp((LPCTSTR)strTemp, L"�ֻ���������"))
	{
		m_cLevel.SetCurSel(0);
	}
	else
		m_cLevel.SetCurSel(1);

	CString szTemp;
	szTemp = m_cAdminList.GetItemText(m_nIndex, 4);

	m_nMaxUser = _wtoi(szTemp.GetBuffer(100));
	m_strKey = m_cAdminList.GetItemText(m_nIndex, 5);

	UpdateData(FALSE);
	*pResult = 0;
}

void CAdminUpdateDlg::OnCancel()
{
	int i = 0;
	ADMININFO *pbAdminInfo;
	CString strTemp;

	if(g_pbAdminInfo)
	{
		delete [] g_pbAdminInfo;
		g_pbAdminInfo = NULL;
	}

	g_pbAdminInfo = new BYTE[g_nAdminNum*sizeof(ADMININFO)];

	for(i = 0 ; i < g_nAdminNum ; i++)
	{
		pbAdminInfo = (ADMININFO*)(g_pbAdminInfo + i*sizeof(ADMININFO));
		strTemp = m_cAdminList.GetItemText(i, 1);
		wcscpy(pbAdminInfo->szID, (LPCTSTR)strTemp);
		strTemp = m_cAdminList.GetItemText(i, 2);
		wcscpy(pbAdminInfo->szPwd, (LPCTSTR)strTemp);
		strTemp = m_cAdminList.GetItemText(i, 3);
		if(!wcscmp((LPCTSTR)strTemp, L"�ֻ���������"))
		{
			pbAdminInfo->dwLevel = 0;
		}
		else
			pbAdminInfo->dwLevel = 1;

		strTemp = m_cAdminList.GetItemText(i, 4);
		pbAdminInfo->dwMaxUser = _wtoi(strTemp.GetBuffer(100));
		strTemp = m_cAdminList.GetItemText(i, 5);
		StringToBytes(pbAdminInfo->pbAccount, strTemp.GetBuffer(MAX_PATH));
	}

	CDialog::OnCancel();
}

void CAdminUpdateDlg::OnBnClickedCancel()
{	
	OnCancel();
}

void CAdminUpdateDlg::ResetIndex(void)
{
	int i = 0;
	int	nCount = m_cAdminList.GetItemCount();
	WCHAR	szTemp[MAX_PATH] = {0,};

	for(i = 0 ; i < nCount ; i++)
	{
		swprintf(szTemp, L"%d", i + 1);
		m_cAdminList.SetItemText(i, 0, szTemp);
	}
}
