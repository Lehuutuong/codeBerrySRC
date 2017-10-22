// AdminDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Admin.h"
#include "AdminDlg.h"
#include "io.h"
#include "XLEzAutomation.h" 
#include "AdminUpdateDlg.h"
#include "AccountUpdateDlg.h"
#include "DlgLogin.h"
#include "../Common/Common.h"
#include "AutoControl.h"
#include "MultiCountDownload.h"
WCHAR	g_szAccount[100]= L"";

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAdminDlg dialog

ACCOUNTINFO* g_pAccountInfo = NULL;

CAdminDlg	*pAdminDlg;
CAdminDlg::CAdminDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdminDlg::IDD, pParent)	
	, m_nIndex(0)	
	, m_strAccount(_T(""))
	, m_strPassword(_T(""))
	, m_timeReg(0)
	, m_timeExpire(0)
	, m_strRegID(_T(""))
	, m_nReflashTick(0)
	, m_strSearchAccount(_T(""))
	, m_nMaxMulti(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pAdminDlg = this;
	m_bModify = FALSE;
	m_nAccountIndex = 0;
}

void CAdminDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listID);
	DDX_Control(pDX, IDC_ADDID, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVEID, m_btnDel);
	DDX_Control(pDX, IDC_MODIFYID, m_btnModify);
	DDX_Control(pDX, IDC_ADDFROMFILE, m_btnAddFromFile);	
	DDX_Control(pDX, IDC_BTN_CLEAR, m_btnClear);
	DDX_Text(pDX, IDC_ACCOUNT2, m_strAccount);
	DDX_Text(pDX, IDC_PASSWORD2, m_strPassword);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER4, m_timeReg);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER5, m_timeExpire);
	DDX_Text(pDX, IDC_ID2, m_strRegID);
	DDX_Control(pDX, IDC_UPLOAD, m_btnUploadT);
	DDX_Control(pDX, IDC_BTN_ACCOUNTSET_UPLOAD, m_btnAccountSetUpload);
	DDX_Text(pDX, IDC_EDT_REFLASHTICK, m_nReflashTick);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_QUERYSTART, m_timeQueryStart);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_QUERYEND, m_timeQueryEnd);
	DDX_Text(pDX, IDC_SEARCH_ACCOUNT, m_strSearchAccount);
	DDX_CBIndex(pDX, IDC_COMBO2, m_nMaxMulti);
}

BEGIN_MESSAGE_MAP(CAdminDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CAdminDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_ADDID, &CAdminDlg::OnBnClickedAddid)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CAdminDlg::OnLvnItemchangedList1)
	ON_BN_CLICKED(IDC_REMOVEID, &CAdminDlg::OnBnClickedRemoveid)
	ON_BN_CLICKED(IDC_MODIFYID, &CAdminDlg::OnBnClickedModifyid)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CAdminDlg::OnNMDblclkList1)	
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CAdminDlg::OnHdnItemclickList1)
	ON_BN_CLICKED(IDC_ADDFROMFILE, &CAdminDlg::OnBnClickedAddfromfile)	
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CAdminDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_REFRESH, &CAdminDlg::OnBnClickedBtnRefresh)
	ON_BN_CLICKED(IDC_UPLOAD, &CAdminDlg::OnBnClickedUploadT)
	ON_BN_CLICKED(IDC_BTN_ACCOUNTSET_UPLOAD, &CAdminDlg::OnBnClickedBtnAccountsetUpload)
	ON_EN_CHANGE(IDC_EDT_REFLASHTICK, &CAdminDlg::OnEnChangeEdtReflashtick)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_QUERYHISTORY, &CAdminDlg::OnBnClickedBtnQueryhistory)
	ON_BN_CLICKED(IDC_BTN_SEARCHACCOUNT, &CAdminDlg::OnBnClickedBtnSearchaccount)
	ON_BN_CLICKED(IDC_BTN_AUTOEND, &CAdminDlg::OnBnClickedBtnAutoend)
	ON_BN_CLICKED(IDC_BTN_AUTORERUN, &CAdminDlg::OnBnClickedBtnAutorerun)
	ON_BN_CLICKED(IDC_BTN_AUTOSTOP, &CAdminDlg::OnBnClickedBtnAutostop)
	ON_BN_CLICKED(IDC_BTN_AUTOSTART, &CAdminDlg::OnBnClickedBtnAutostart)
	ON_BN_CLICKED(IDC_BTN_AUTOGAMERUN, &CAdminDlg::OnBnClickedBtnAutogamerun)
	ON_BN_CLICKED(IDC_BTN_ACCOUNTREFRESH, &CAdminDlg::OnBnClickedBtnAccountrefresh)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CAdminDlg::OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BTN_STATSTICS, &CAdminDlg::OnBnClickedBtnStatstics)
END_MESSAGE_MAP()


// CAdminDlg message handlers
BOOL CAdminDlg::OnInitDialog()
{
	CDialog::OnInitDialog();	

	WSADATA wsData;
	if (WSAStartup(0x2,&wsData))
	{
		if (WSAStartup(0x101,&wsData))
			return FALSE;
	}
	
	CDlgLogin dlg;
	if (dlg.DoModal() != IDOK)
	{
		EndDialog(IDCANCEL);
		return FALSE;
	}

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_listID.m_pDlg = this;
	// TODO: Add extra initialization here
	m_listID.InsertColumn(1, L"", LVCFMT_CENTER, 60);
	m_listID.InsertColumn(2, L"계정", LVCFMT_CENTER, 85);
	m_listID.InsertColumn(3, L"비번", LVCFMT_CENTER, 60);
	m_listID.InsertColumn(4, L"등록키", LVCFMT_CENTER, 135);
	m_listID.InsertColumn(5, L"등록날짜", LVCFMT_CENTER, 90);
	m_listID.InsertColumn(6, L"마감날짜", LVCFMT_CENTER, 90);
	m_listID.InsertColumn(7, L"관리자", LVCFMT_CENTER, 70);
	//m_listID.InsertColumn(7, L"상태", LVCFMT_CENTER, 220);
	//m_listID.InsertColumn(8, L"잔여시간", LVCFMT_CENTER, 250);
	m_listID.InsertColumn(8, L"멀티갯수", LVCFMT_CENTER, 70);
	//m_listID.InsertColumn(10, L"아이피", LVCFMT_CENTER, 90);
	m_listID.SetExtendedStyle(m_listID.GetExtendedStyle()|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	m_listID.SetExtendedStyle( m_listID.GetExtendedStyle() | LVS_EX_CHECKBOXES);
	m_listID.Init();

	SYSTEMTIME	time;
	GetLocalTime(&time);

	CTime	timeTemp(time.wYear, time.wMonth, time.wDay, 1 ,1 ,1);		
	m_timeReg = timeTemp;

	CTime	timeTemp2(time.wYear, time.wMonth, time.wDay, 1 ,1 ,1);
	m_timeExpire = timeTemp2;

	SYSTEMTIME systime;
	GetLocalTime(&systime);
	m_timeQueryStart = CTime(systime);
	m_timeQueryEnd = CTime(systime);

	UpdateData(FALSE);

	m_nIndex = -1;
	memset(&g_AccountInfo, 0, sizeof(ACCOUNTINFO));
	
	if(g_bBossAdmin && wcscmp(g_strAdminId, VMProtectDecryptStringW(ADMIN_TOP_ID)) == 0)
	{
		m_btnUploadT.ShowWindow(SW_SHOW);
		m_btnUploadT.EnableWindow(TRUE);
	}

	OnBnClickedOk();

	// 소켓창조 및 연결
	//WSADATA wsaData;
	//WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_nReflashTick = 30;
	UpdateData(FALSE);
	SetTimer(1, m_nReflashTick, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAdminDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CAdminDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		//SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


HCURSOR CAdminDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
WCHAR	szAccountFile[MAX_PATH];
BOOL	PrintLog(WCHAR *szFormat, ...)
{
	FILE	*fp;
	char	*aszLog;
	WCHAR	szLog[1000];
	va_list	arg;

	va_start(arg, szFormat);
	vswprintf(szLog, szFormat, arg);
	va_end(arg);
	aszLog = new char[wcslen(szLog)*2+20];
	fp = _wfopen(L"C:\\CS.log", L"a+");
	if(!fp)
		return FALSE;
	WideCharToMultiByte(CP_ACP, 0, szLog, wcslen(szLog)+1, aszLog, wcslen(szLog)*2+2, 0, 0);
	strcat(aszLog, "\n");
	fprintf(fp, aszLog);
	fclose(fp);
	delete [] aszLog;
	return TRUE;
}

void DwordToIPWString(DWORD PackedIpAddress, LPWSTR IpAddressBuffer)
{
	BYTE IpNumbers[4];

	IpNumbers[0] = (BYTE)((PackedIpAddress >> 24) & 0xFF);
	IpNumbers[1] = (BYTE)((PackedIpAddress >> 16) & 0xFF);
	IpNumbers[2] = (BYTE)((PackedIpAddress >> 8) & 0xFF);
	IpNumbers[3] = (BYTE)((PackedIpAddress >> 0) & 0xFF);
	swprintf(IpAddressBuffer, L"%d.%d.%d.%d", IpNumbers[0], IpNumbers[1], IpNumbers[2], IpNumbers[3]);
}

void CAdminDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	VMProtectBegin("OnBnClickedOk");

	SOCKADDR_IN		saddr;

	SOCKET sock = INVALID_SOCKET;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"소켓창조가 실패하였습니다.");
		return;
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
			return;
		}
	}

	int				i, nLen, nLen2;
	BYTE			pbBuff[MAX_PATH];	
	BYTE			*pbBuff2 = NULL;
	WCHAR			szTemp[MAX_PATH];
	ACCOUNTINFO		*pAccountInfo;
	BYTE			*pbUserInfo = NULL;;

	nLen = MakePacket(pbBuff, "cdSS", OPCODE_ADMINCONN, 0, (LPCTSTR)g_strAdminId, (LPCTSTR)g_strAdminPwd);	
	SendData(sock, (char*)pbBuff, nLen, 0);

	VMProtectEnd();

	nLen = RecvData(sock, (char*)pbBuff, 1, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return;
	}

	if( pbBuff[0] != OPCODE_ADMINCONN && pbBuff[0] != OPCODE_BOSSCONN)
	{
		closesocket(sock);
		AfxMessageBox(L"자료수신중 오류가 발생하였습니다.");
		return;
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
		return;
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
		return;
	}

	closesocket(sock);

	DecryptPacket(pbBuff2, nLen2);

	if(g_bBossAdmin)
	{
		m_szAddress.Format(L"%x", *(DWORD*)(pbBuff2 + nLen2 - 4));
	}
	nLen = *(int*)pbBuff2;
	if(nLen == 0xFFFFFFFF)
	{
		AfxMessageBox(L"비번이 정확치 않습니다.");
		return;
	}
	if(nLen == 0xFFFFFFFE)
	{
		AfxMessageBox(L"다른 관리자가 현재 접속중입니다.");
		return;
	}

	if(g_bBossAdmin)
	{
		m_btnDel.EnableWindow(TRUE);
		m_btnDel.ShowWindow(SW_SHOW);
		m_btnAdd.EnableWindow(TRUE);
		m_btnAdd.ShowWindow(SW_SHOW);
		m_btnModify.EnableWindow(TRUE);
		m_btnModify.ShowWindow(SW_SHOW);
		m_btnClear.EnableWindow(TRUE);
		m_btnClear.ShowWindow(SW_SHOW);
		m_btnAccountSetUpload.EnableWindow(TRUE);
		m_btnAccountSetUpload.ShowWindow(SW_SHOW);
		//m_btnAddFromFile.EnableWindow(TRUE);
		//m_btnAddFromFile.ShowWindow(SW_SHOW);
	}	

	m_listID.DeleteAllItems();

	if(g_pAccountInfo)
		delete[] g_pAccountInfo;

	g_pAccountInfo = new ACCOUNTINFO[nLen];
	memcpy(g_pAccountInfo, pbBuff2+4, nLen * sizeof(ACCOUNTINFO));

	for(i = 0; i < nLen; i++)
	{
		pAccountInfo = (ACCOUNTINFO*)(pbBuff2+i*sizeof(ACCOUNTINFO)+4);
		swprintf(szTemp, L"%d", i+1);
		m_listID.InsertItem(i, szTemp);
		BytesToString(szTemp, pAccountInfo->pbAccount, 8);
		m_listID.SetItemText(i, 3, szTemp);
		swprintf(szTemp, L"%04d/%02d/%02d", *(short*)pAccountInfo->pbRegDate, pAccountInfo->pbRegDate[2], pAccountInfo->pbRegDate[3]);
		m_listID.SetItemText(i, 4, szTemp);
		swprintf(szTemp, L"%04d/%02d/%02d", *(short*)pAccountInfo->pbExpDate, pAccountInfo->pbExpDate[2], pAccountInfo->pbExpDate[3]);
		m_listID.SetItemText(i, 5, szTemp);
		swprintf(szTemp, L"%X", pAccountInfo->dwStatus);
		m_listID.SetItemText(i, 6, szTemp);
		swprintf(szTemp, L"%s", pAccountInfo->szAccount);
		m_listID.SetItemText(i, 1, szTemp);
		swprintf(szTemp, L"%s", pAccountInfo->szPassword);
		m_listID.SetItemText(i, 2, szTemp);
		swprintf(szTemp, L"%s", pAccountInfo->szAdminName);
		m_listID.SetItemText(i, 6, szTemp);
		swprintf(szTemp, L"%d", pAccountInfo->nMaxMulti);
		m_listID.SetItemText(i, 7, szTemp);
		//DwordToIPWString(pAccountInfo->dwLocalIP, szTemp);
		//m_listID.SetItemText(i, 9, szTemp);
	}
	

	delete pbBuff2;

	GetDlgItem(IDC_BTN_REFRESH)->ShowWindow(SW_SHOW);
	UpdateData(FALSE);

}

void CAdminDlg::AddID(int nInd)
{
	WCHAR	szTemp[MAX_PATH];

	swprintf(szTemp, L"%d", nInd+1);
	if(nInd >= m_listID.GetItemCount())
		m_listID.InsertItem(nInd, szTemp);
	else
		m_listID.SetItemText(nInd, 0, szTemp);

	BytesToString(szTemp, g_AccountInfo.pbAccount, 8);
	m_listID.SetItemText(nInd, 3, szTemp);
	swprintf(szTemp, L"%04d/%02d/%02d", *(short*)g_AccountInfo.pbRegDate, g_AccountInfo.pbRegDate[2], g_AccountInfo.pbRegDate[3]);
	m_listID.SetItemText(nInd, 4, szTemp);
	swprintf(szTemp, L"%04d/%02d/%02d", *(short*)g_AccountInfo.pbExpDate, g_AccountInfo.pbExpDate[2], g_AccountInfo.pbExpDate[3]);
	m_listID.SetItemText(nInd, 5, szTemp);
	swprintf(szTemp, L"%s",g_AccountInfo.szAccount);
	m_listID.SetItemText(nInd, 1, szTemp);
	swprintf(szTemp, L"%s",g_AccountInfo.szPassword);
	m_listID.SetItemText(nInd, 2, szTemp);
	swprintf(szTemp, L"%s", g_AccountInfo.szAdminName);
	m_listID.SetItemText(nInd, 6, szTemp);
	swprintf(szTemp, L"%d", g_AccountInfo.nMaxMulti);
	m_listID.SetItemText(nInd, 7, szTemp);
}

void CAdminDlg::UpdateIDList(int nCount, int *pSelectedList, ACCOUNTINFO * pAccountList)
{
	for(int k = 0; k < nCount; k ++)
	{
		int i = pSelectedList[k];
		WCHAR	szTemp[MAX_PATH];

		swprintf(szTemp, L"%d", i+1);
		if(i >= m_listID.GetItemCount())
			m_listID.InsertItem(i, szTemp);
		else
			m_listID.SetItemText(i, 0, szTemp);

		BytesToString(szTemp, pAccountList[k].pbAccount, 8);
		m_listID.SetItemText(i, 3, szTemp);
		swprintf(szTemp, L"%04d/%02d/%02d", *(short*)pAccountList[k].pbRegDate, pAccountList[k].pbRegDate[2], pAccountList[k].pbRegDate[3]);
		m_listID.SetItemText(i, 4, szTemp);
		swprintf(szTemp, L"%04d/%02d/%02d", *(short*)pAccountList[k].pbExpDate, pAccountList[k].pbExpDate[2], pAccountList[k].pbExpDate[3]);
		m_listID.SetItemText(i, 5, szTemp);
		swprintf(szTemp, L"%s",pAccountList[k].szAccount);
		m_listID.SetItemText(i, 1, szTemp);
		swprintf(szTemp, L"%s",pAccountList[k].szPassword);
		m_listID.SetItemText(i, 2, szTemp);
		swprintf(szTemp, L"%s", pAccountList[k] .szAdminName);
		m_listID.SetItemText(i, 6, szTemp);
		swprintf(szTemp, L"%d", pAccountList[k].nMaxMulti);
		m_listID.SetItemText(i, 7, szTemp);
	}	
}

void CAdminDlg::OnBnClickedAddid()
{
	UpdateData(TRUE);

	if (m_strAccount.GetLength() > 15 || m_strPassword.GetLength() > 15)
	{
		AfxMessageBox(L"오토계정은 영문과 숫자로 15자 이하여야 합니다.");
		return;
	}

	CString			strID,strAccount;
	int				i;
	for (i = 0; i < m_listID.GetItemCount(); i ++)
	{
		strAccount = m_listID.GetItemText(i,1);
		if (strAccount == m_strAccount)
			break;
	}
	if (i < pAdminDlg->m_listID.GetItemCount())
	{
		AfxMessageBox(L"이미 존재하는 계정입니다.");
		return;
	}
	
	if(m_strRegID.IsEmpty())
		m_strRegID = L"0000000000000000";

	if(m_strRegID.GetLength() != 16)
	{
		AfxMessageBox(L"등록키입력이 정확치 않습니다.");
		return;
	}
	if(m_timeReg > m_timeExpire)
	{
		AfxMessageBox(L"날짜가 정확한지 확인하십시오.");
	}

	StringToBytes(g_AccountInfo.pbAccount, (LPCTSTR)m_strRegID); 
	*(short*)g_AccountInfo.pbRegDate = (short)m_timeReg.GetYear();
	g_AccountInfo.pbRegDate[2] = (BYTE)m_timeReg.GetMonth();
	g_AccountInfo.pbRegDate[3] = (BYTE)m_timeReg.GetDay();
	*(short*)g_AccountInfo.pbExpDate = (short)m_timeExpire.GetYear();
	g_AccountInfo.pbExpDate[2] = (BYTE)m_timeExpire.GetMonth();
	g_AccountInfo.pbExpDate[3] = (BYTE)m_timeExpire.GetDay();

	wcscpy(g_AccountInfo.szAccount,m_strAccount.GetBuffer());
	wcscpy(g_AccountInfo.szPassword,m_strPassword.GetBuffer());
	wcscpy(g_AccountInfo.szAdminName, (LPCWSTR)g_strAdminId);
	
	g_AccountInfo.nMaxMulti = m_nMaxMulti;

	BYTE	*pbBuff = NULL;
	int		nLen;

	UpdateData(TRUE);

	VMProtectBegin("OnBnClickedAddid");

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"소켓창조가 실패하였습니다.");
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
			AfxMessageBox(L"서버접속이 실패하였습니다.");
			return;
		}
	}

	BYTE TbbySendBuffer[sizeof(ACCOUNTINFO) + 100];
	nLen = MakePacket(TbbySendBuffer, "cddSSb", OPCODE_ADMIN_ACCOUNTUPDATE, 0, USERINFO_ADD, g_strAdminId, g_strAdminPwd, &g_AccountInfo, sizeof(g_AccountInfo));
	SendData(sock, (char*)TbbySendBuffer, nLen, 0);

	BYTE	pbRecvBuff[100];

	if(5 != RecvData(sock, (char*)pbRecvBuff, 5, 0) || *(DWORD*)(pbRecvBuff+1) != 0)
	{
		closesocket(sock);
		AfxMessageBox(SOCKET_ERROR_TEXT2);
		UpdateData(FALSE);
		return;
	}

	closesocket(sock);
	VMProtectEnd();

	if(pbRecvBuff[0] == OPCODE_ADMIN_ACCOUNTUPDATE)
	{
		AfxMessageBox(L"계정추가가 완료되었습니다.");
		AddID(m_listID.GetItemCount());
		ResetIndex();
	}
	else if(pbRecvBuff[0] == OPCODE_FAILADD)
	{
		AfxMessageBox(L"계정추가에 실패하였습니다.");
	}
}
void CAdminDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW	pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	POSITION		pos;

	pos = m_listID.GetFirstSelectedItemPosition();
	if(!pos)
	{		
		//m_btnModify.EnableWindow(FALSE);
		m_nIndex = -1;
		return;
	}
	//m_btnModify.EnableWindow(TRUE);
	
	m_nIndex = m_listID.GetNextSelectedItem(pos);
	*pResult = 0;
}

void CAdminDlg::OnBnClickedRemoveid()
{
	int nCount = m_listID.GetItemCount();
	int nSelectedCount = 0;
	int* nCheckedList = new int[nCount];
	for(int nItem = 0; nItem < nCount; nItem++)
	{
		if ( !ListView_GetCheckState(m_listID.GetSafeHwnd(), nItem) )
		{
		}
		else
		{
			nCheckedList[nSelectedCount] = nItem;
			nSelectedCount ++;
		}
	}
	if(nSelectedCount == 0)
		return;

	CString szConfirmReport;
	szConfirmReport = L"선택된 계정을 삭제합니다.\n";
	if( MessageBox(szConfirmReport, L"확인", MB_YESNO) == IDYES)
	{
		WCHAR		szTemp[MAX_PATH];
		int			nTemp, nTemp2, nTemp3;

		ACCOUNTINFO * pAccountInfoBuffer = new ACCOUNTINFO[nSelectedCount];
		ZeroMemory(pAccountInfoBuffer, sizeof(ACCOUNTINFO) * nSelectedCount);
		for(int i = 0; i < nSelectedCount; i ++)
		{
			m_listID.GetItemText(nCheckedList[i], 4, szTemp, MAX_PATH);
			swscanf(szTemp, L"%04d/%02d/%02d", &nTemp, &nTemp2, &nTemp3);
			*(short*)pAccountInfoBuffer[i].pbRegDate = (short)nTemp;
			pAccountInfoBuffer[i].pbRegDate[2] = (BYTE)nTemp2;
			pAccountInfoBuffer[i].pbRegDate[3] = (BYTE)nTemp3;

			m_listID.GetItemText(nCheckedList[i], 5, szTemp, MAX_PATH);
			swscanf(szTemp, L"%04d/%02d/%02d", &nTemp, &nTemp2, &nTemp3);
			*(short*)pAccountInfoBuffer[i].pbExpDate = (short)nTemp;
			pAccountInfoBuffer[i].pbExpDate[2] = (BYTE)nTemp2;
			pAccountInfoBuffer[i].pbExpDate[3] = (BYTE)nTemp3;

			pAccountInfoBuffer[i].dwStatus = 0;

			m_listID.GetItemText(nCheckedList[i], 1, szTemp, MAX_PATH);
			wcscpy(pAccountInfoBuffer[i].szAccount,szTemp);

			m_listID.GetItemText(nCheckedList[i], 2, szTemp, MAX_PATH);
			wcscpy(pAccountInfoBuffer[i].szPassword,szTemp);

			m_listID.GetItemText(nCheckedList[i], 6, szTemp, MAX_PATH);
			wcscpy(pAccountInfoBuffer[i].szAdminName, szTemp);

			m_listID.GetItemText(nCheckedList[i], 7, szTemp, MAX_PATH);
			swscanf(szTemp, L"%d", &nTemp);
			pAccountInfoBuffer[i].nMaxMulti = nTemp;

		}

		VMProtectBegin("OnBnClickedRemoveid");

		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(sock == INVALID_SOCKET)
		{
			AfxMessageBox(L"소켓창조가 실패하였습니다.");
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
				AfxMessageBox(L"서버접속이 실패하였습니다.");
				return;
			}
		}

		int nLen = sizeof(ACCOUNTINFO)*nSelectedCount + 100;
		LPBYTE pbySendBuffer = new BYTE[nLen];
		nLen = MakePacket(pbySendBuffer, "cdddSSb", OPCODE_ADMIN_ACCOUNTLISTUPDATE, 0, USERINFO_DEL, nSelectedCount, g_strAdminId, g_strAdminPwd, pAccountInfoBuffer, sizeof(ACCOUNTINFO)*nSelectedCount);
		SendData(sock, (char*)pbySendBuffer, nLen, 0);
		delete pbySendBuffer;

		BYTE	pbRecvBuff[100];

		if(5 != RecvData(sock, (char*)pbRecvBuff, 5, 0) || *(DWORD*)(pbRecvBuff+1) != 0)
		{
			closesocket(sock);
			AfxMessageBox(SOCKET_ERROR_TEXT2);
			UpdateData(FALSE);
			return;
		}

		closesocket(sock);
		if(pbRecvBuff[0] == OPCODE_ADMIN_ACCOUNTUPDATE)
		{
			AfxMessageBox(L"계정이 삭제되었습니다.");
		}
		else if(pbRecvBuff[0] == OPCODE_FAILADD)
		{
			AfxMessageBox(L"업데이트가 실패하였습니다.");
		}

		VMProtectEnd();

		for(int i = 0 ; i < nSelectedCount ; i ++ )
		{		
			m_listID.DeleteItem(nCheckedList[i]);
			for(int j = i + 1; j < nSelectedCount; j ++)
				nCheckedList[j] --;
			ListView_SetCheckState(m_listID.GetSafeHwnd(), nCheckedList[i], 0);
		}
		UpdateData(FALSE);	
		delete pAccountInfoBuffer;

		m_nIndex = 0;
		ResetIndex();
	}
}

void CAdminDlg::OnBnClickedModifyid()
{
	CString szConfirmReport;
	int nCount = m_listID.GetItemCount();
	if(nCount == 0)
		return;

	int nSelectedCount = 0;
	int* nCheckedList = new int[nCount];
	for(int nItem = 0; nItem < nCount; nItem++)
	{
		if ( !ListView_GetCheckState(m_listID.GetSafeHwnd(), nItem) )
		{
		}
		else
		{
			nCheckedList[nSelectedCount] = nItem;
			nSelectedCount ++;
		}
	}
	if(nSelectedCount)
	{
		WCHAR		szTemp[MAX_PATH];
		int			nTemp, nTemp2, nTemp3;

		ACCOUNTINFO * pAccountInfoBuffer = new ACCOUNTINFO[nSelectedCount];
		ZeroMemory(pAccountInfoBuffer, sizeof(ACCOUNTINFO) * nSelectedCount);
		for(int i = 0; i < nSelectedCount; i ++)
		{
			m_listID.GetItemText(nCheckedList[i], 3, szTemp, MAX_PATH);
			StringToBytes(pAccountInfoBuffer[i].pbAccount, szTemp);

			m_listID.GetItemText(nCheckedList[i], 4, szTemp, MAX_PATH);
			swscanf(szTemp, L"%04d/%02d/%02d", &nTemp, &nTemp2, &nTemp3);
			*(short*)pAccountInfoBuffer[i].pbRegDate = (short)nTemp;
			pAccountInfoBuffer[i].pbRegDate[2] = (BYTE)nTemp2;
			pAccountInfoBuffer[i].pbRegDate[3] = (BYTE)nTemp3;

			m_listID.GetItemText(nCheckedList[i], 5, szTemp, MAX_PATH);
			swscanf(szTemp, L"%04d/%02d/%02d", &nTemp, &nTemp2, &nTemp3);
			*(short*)pAccountInfoBuffer[i].pbExpDate = (short)nTemp;
			pAccountInfoBuffer[i].pbExpDate[2] = (BYTE)nTemp2;
			pAccountInfoBuffer[i].pbExpDate[3] = (BYTE)nTemp3;

			pAccountInfoBuffer[i].dwStatus = 0;

			m_listID.GetItemText(nCheckedList[i], 1, szTemp, MAX_PATH);
			wcscpy(pAccountInfoBuffer[i].szAccount,szTemp);

			m_listID.GetItemText(nCheckedList[i], 2, szTemp, MAX_PATH);
			wcscpy(pAccountInfoBuffer[i].szPassword,szTemp);

			m_listID.GetItemText(nCheckedList[i], 6, szTemp, MAX_PATH);
			wcscpy(pAccountInfoBuffer[i].szAdminName, szTemp);

			m_listID.GetItemText(nCheckedList[i], 7, szTemp, MAX_PATH);
			swscanf(szTemp, L"%d", &nTemp);
			pAccountInfoBuffer[i].nMaxMulti = nTemp;
		}

		CAccountUpdateDlg dlg;
		dlg.m_nAccountCount = nSelectedCount;
		dlg.m_pAccountList = pAccountInfoBuffer;
		if(dlg.DoModal() == IDOK)
		{
			BYTE	*pbBuff = NULL;

			UpdateData(TRUE);

			VMProtectBegin("OnBnClickedModifyid");

			SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(sock == INVALID_SOCKET)
			{
				AfxMessageBox(L"소켓창조가 실패하였습니다.");
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
					AfxMessageBox(L"서버접속이 실패하였습니다.");
					return;
				}
			}

			int nLen = sizeof(ACCOUNTINFO)*nSelectedCount + 100;
			LPBYTE pbySendBuffer = new BYTE[nLen];
			nLen = MakePacket(pbySendBuffer, "cdddSSb", OPCODE_ADMIN_ACCOUNTLISTUPDATE, 0, USERINFO_UPDATE, nSelectedCount, g_strAdminId, g_strAdminPwd, pAccountInfoBuffer, sizeof(ACCOUNTINFO)*nSelectedCount);
			SendData(sock, (char*)pbySendBuffer, nLen, 0);
			delete pbySendBuffer;

			BYTE	pbRecvBuff[100];

			if(5 != RecvData(sock, (char*)pbRecvBuff, 5, 0) || *(DWORD*)(pbRecvBuff+1) != 0)
			{
				closesocket(sock);
				AfxMessageBox(SOCKET_ERROR_TEXT2);
				UpdateData(FALSE);
				return;
			}

			closesocket(sock);
			if(pbRecvBuff[0] == OPCODE_ADMIN_ACCOUNTUPDATE)
			{
				AfxMessageBox(L"계정정보가 갱신되었습니다.");
			}
			else if(pbRecvBuff[0] == OPCODE_FAILADD)
			{
				AfxMessageBox(L"업데이트가 실패하였습니다.");
			}

			for(int i = 0 ; i < nSelectedCount ; i++ )
			{		
				ListView_SetCheckState(m_listID.GetSafeHwnd(), nCheckedList[i], 0);
			}
			UpdateIDList(nSelectedCount, nCheckedList, pAccountInfoBuffer);

			VMProtectEnd();
			UpdateData(FALSE);	
		}
		delete pAccountInfoBuffer;
	}

	delete nCheckedList;
}

void CAdminDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = (NMITEMACTIVATE *)(pNMHDR);

	m_nIndex = pNMItemActivate->iItem;
	if(m_nIndex >= m_listID.GetItemCount())
		OnBnClickedAddid();
	else
		OnBnClickedModifyid();
	*pResult = 0;
}



void CAdminDlg::OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;

}

int CAdminDlg::SearchID(const WCHAR *szID,int nIndex)
{
	int			i = 0;
	CString		strTemp;

	UpdateData(TRUE);

	for(i = 0; i < m_listID.GetItemCount(); i++)
	{
		strTemp = m_listID.GetItemText(i, nIndex);
		if(strTemp == szID)
			break;
	}

	return i;
}

// Excel파일로부터 계정추가
void CAdminDlg::OnBnClickedAddfromfile()
{
	int				i, k, l;
	CFileDialog		dlg(TRUE, 0, 0, 4|2, L"Excel Files (*.xlsx)|*.xlsx|"); 

	UpdateData(TRUE);

	if(dlg.DoModal() != IDOK)
		return;

	TCHAR szPath[MAX_PATH];
	swprintf(szPath, L"%s", (LPCTSTR)dlg.GetPathName());


	CXLEzAutomation XL(FALSE);
	WCHAR	*szEnd;
	CString szData;
	int		nAddCnt = 0, nReCnt = 0;

	XL.OpenExcelFile(szPath);

	i = 2;

	int nMaxCntAddAccount = 10000;
	ACCOUNTINFO * pAccountInfo = new ACCOUNTINFO[nMaxCntAddAccount];
	ZeroMemory(pAccountInfo, sizeof(ACCOUNTINFO) * nMaxCntAddAccount);
	for(int j = 0; j < nMaxCntAddAccount; j ++)
	{
		// 아이디
		szData = XL.GetCellValue(1, i);		

		if(szData.IsEmpty())
			break;
		if (wcsstr(szData, L".0000"))
		{
			szData.Delete(szData.GetLength() - 7 , 7);
		}
		wcscpy(pAccountInfo[nAddCnt].szAccount, (LPCTSTR)szData);

		k = m_listID.GetItemCount();
		l = SearchID(pAccountInfo[nAddCnt].szAccount,5);
		if(l >= k) // 현재 리스트에 있는 계정과 중복되지 않는가?
		{
			// 비번
			szData = XL.GetCellValue(2, i);

			if(szData.IsEmpty())
				break;
			if (wcsstr(szData, L".0000"))
			{
				szData.Delete(szData.GetLength() - 7 , 7);
			}
			wcscpy(pAccountInfo[nAddCnt].szPassword, (LPCTSTR)szData);

			// 등록날자
			szData = XL.GetCellValue(3, i);
			if(szData.IsEmpty())
				break;
			*(short*)&(pAccountInfo[nAddCnt].pbRegDate[0]) = (short)wcstol(szData.Left(4), &szEnd, 10);
			pAccountInfo[nAddCnt].pbRegDate[2] = (BYTE)wcstol(szData.Mid(5, 2), &szEnd, 10);
			pAccountInfo[nAddCnt].pbRegDate[3] = (BYTE)wcstol(szData.Mid(8, 2), &szEnd, 10);

			// 마감날자
			szData = XL.GetCellValue(4, i);
			if(szData.IsEmpty())
				break;
			*(short*)&(pAccountInfo[nAddCnt].pbExpDate[0]) = (short)wcstol((LPCTSTR)szData.Left(4), &szEnd, 10);
			pAccountInfo[nAddCnt].pbExpDate[2] = (BYTE)wcstol((LPCTSTR)szData.Mid(5, 2), &szEnd, 10);
			pAccountInfo[nAddCnt].pbExpDate[3] = (BYTE)wcstol((LPCTSTR)szData.Mid(8, 2), &szEnd, 10);

			// 발급자
			wcscpy(pAccountInfo[nAddCnt].szAdminName, (LPCTSTR)g_strAdminId);

			nAddCnt ++;
		}
		else
		{
			nReCnt++;
		}
		i++;
	}
	XL.ReleaseExcel();

	CString strReport;
	strReport.Format(L"%d개 계정은 이미 추가되어 있습니다. %d개의 계정을 추가합니다.", nReCnt, nAddCnt);
	AfxMessageBox(strReport);

	if(nAddCnt <= 0)
	{
		return;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"소켓창조가 실패하였습니다.");
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
			AfxMessageBox(L"서버접속이 실패하였습니다.");
			return;
		}
	}

	int nLen = sizeof(ACCOUNTINFO) * nAddCnt + 100;
	LPBYTE pbySendBuffer = new BYTE[nLen];
	nLen = MakePacket(pbySendBuffer, "cdddSSb", OPCODE_ADMIN_ACCOUNTLISTUPDATE, 0, USERINFO_ADD, nAddCnt, g_strAdminId, g_strAdminPwd, pAccountInfo, sizeof(ACCOUNTINFO) * nAddCnt);
	SendData(sock, (char*)pbySendBuffer, nLen, 0);
	delete pbySendBuffer;

	BYTE	pbRecvBuff[100];

	if(5 != RecvData(sock, (char*)pbRecvBuff, 5, 0) || *(DWORD*)(pbRecvBuff+1) != 0)
	{
		closesocket(sock);
		AfxMessageBox(SOCKET_ERROR_TEXT2);
		UpdateData(FALSE);
		return;
	}

	closesocket(sock);
	if(pbRecvBuff[0] == OPCODE_ADMIN_ACCOUNTUPDATE)
	{
		AfxMessageBox(L"계정추가가 성공하였습니다.");
	}
	else if(pbRecvBuff[0] == OPCODE_FAILADD)
	{
		AfxMessageBox(L"업데이트가 실패하였습니다.");
	}

	OnBnClickedOk();
}

void CAdminDlg::OnCancel()
{
	CDialog::OnCancel();
}


void CAdminDlg::OnBnClickedBtnClear()
{
	CString szConfirmReport;
	int nCount = m_listID.GetItemCount();
	if(nCount == 0)
		return;

	int nSelectedCount = 0;
	int* nCheckedList = new int[nCount];
	for(int nItem = 0; nItem < nCount; nItem++)
	{
		if ( !ListView_GetCheckState(m_listID.GetSafeHwnd(), nItem) )
		{
		}
		else
		{
			nCheckedList[nSelectedCount] = nItem;
			nSelectedCount ++;
		}
	}
	if(nSelectedCount)
	{
		if(nSelectedCount == nCount)
		{
			szConfirmReport.Format(L"모든 등록번호를 초기화합니다.");

			if( MessageBox(szConfirmReport, L"확인", MB_YESNO) == IDYES)
			{
				BYTE	*pbBuff = NULL;
				int		nLen;

				pbBuff = new BYTE[100];
				ZeroMemory(pbBuff, 100);

				UpdateData(TRUE);

				VMProtectBegin("OnBnClickedBtnClear");

				SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if(sock == INVALID_SOCKET)
				{
					AfxMessageBox(L"소켓창조가 실패하였습니다.");
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
						AfxMessageBox(L"서버접속이 실패하였습니다.");
						return;
					}
				}

				nLen = MakePacket(pbBuff, "cdSS", OPCODE_INITMAC, 0, g_strAdminId, g_strAdminPwd);
				SendData(sock, (char*)pbBuff, nLen, 0);

				BYTE	pbRecvBuff[100];

				if(5 != RecvData(sock, (char*)pbRecvBuff, 5, 0) || *(DWORD*)(pbRecvBuff+1) != 0)
				{
					closesocket(sock);
					AfxMessageBox(SOCKET_ERROR_TEXT2);
					UpdateData(FALSE);
					return;
				}
				closesocket(sock);

				AfxMessageBox(L"등록번호초기화가 완료되었습니다.");

				for(int i = 0 ; i < m_listID.GetItemCount() ; i++ )
				{		
					m_listID.SetItemText(i, 3, L"0000000000000000");
				}

				VMProtectEnd();
				UpdateData(FALSE);	
			}
		}
		else
		{
			CString szConfirmReport;
			szConfirmReport = L"아래의 계정의 등록번호를 초기화합니다.\n";
			for(int i = 0; i < nSelectedCount; i ++)
			{
				CString szAccount;
				TCHAR *sztmp = szAccount.GetBuffer(MAX_PATH);
				m_listID.GetItemText(nCheckedList[i], 1, sztmp, MAX_PATH);
				szAccount.ReleaseBuffer();
				szConfirmReport.Append(L"\n");
				szConfirmReport.Append(szAccount);
			}
			if( MessageBox(szConfirmReport, L"확인", MB_YESNO) == IDYES)
			{
				WCHAR		szTemp[MAX_PATH];
				int			nTemp, nTemp2, nTemp3;

				m_bModify = TRUE;

				ACCOUNTINFO * pAccountInfoBuffer = new ACCOUNTINFO[nSelectedCount];
				ZeroMemory(pAccountInfoBuffer, sizeof(ACCOUNTINFO) * nSelectedCount);
				for(int i = 0; i < nSelectedCount; i ++)
				{
					m_listID.GetItemText(nCheckedList[i], 4, szTemp, MAX_PATH);
					swscanf(szTemp, L"%04d/%02d/%02d", &nTemp, &nTemp2, &nTemp3);
					*(short*)pAccountInfoBuffer[i].pbRegDate = (short)nTemp;
					pAccountInfoBuffer[i].pbRegDate[2] = (BYTE)nTemp2;
					pAccountInfoBuffer[i].pbRegDate[3] = (BYTE)nTemp3;

					m_listID.GetItemText(nCheckedList[i], 5, szTemp, MAX_PATH);
					swscanf(szTemp, L"%04d/%02d/%02d", &nTemp, &nTemp2, &nTemp3);
					*(short*)pAccountInfoBuffer[i].pbExpDate = (short)nTemp;
					pAccountInfoBuffer[i].pbExpDate[2] = (BYTE)nTemp2;
					pAccountInfoBuffer[i].pbExpDate[3] = (BYTE)nTemp3;

					m_listID.GetItemText(nCheckedList[i], 6, szTemp, MAX_PATH);
					if(!wcscmp(szTemp, L"접속중"))
						pAccountInfoBuffer[i].dwStatus = 1;
					else
						pAccountInfoBuffer[i].dwStatus = 0;


					m_listID.GetItemText(nCheckedList[i], 1, szTemp, MAX_PATH);
					wcscpy(pAccountInfoBuffer[i].szAccount,szTemp);

					m_listID.GetItemText(nCheckedList[i], 2, szTemp, MAX_PATH);
					wcscpy(pAccountInfoBuffer[i].szPassword,szTemp);

					m_listID.GetItemText(nCheckedList[i], 6, szTemp, MAX_PATH);
					wcscpy(pAccountInfoBuffer[i].szAdminName, szTemp);

					m_listID.GetItemText(nCheckedList[i], 7, szTemp, MAX_PATH);
					swscanf(szTemp, L"%d", &nTemp);
					pAccountInfoBuffer[i].nMaxMulti = nTemp;

				}

				UpdateData(TRUE);

				VMProtectBegin("OnBnClickedBtnClear1");

				SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if(sock == INVALID_SOCKET)
				{
					AfxMessageBox(L"소켓창조가 실패하였습니다.");
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
						AfxMessageBox(L"서버접속이 실패하였습니다.");
						return;
					}
				}

				int nLen = sizeof(ACCOUNTINFO)*nSelectedCount + 100;
				LPBYTE pbySendBuffer = new BYTE[nLen];
				nLen = MakePacket(pbySendBuffer, "cdddSSb", OPCODE_ADMIN_ACCOUNTLISTUPDATE, 0, USERINFO_UPDATE, nSelectedCount, g_strAdminId, g_strAdminPwd, pAccountInfoBuffer, sizeof(ACCOUNTINFO)*nSelectedCount);
				SendData(sock, (char*)pbySendBuffer, nLen, 0);
				delete pbySendBuffer;

				BYTE	pbRecvBuff[100];

				if(5 != RecvData(sock, (char*)pbRecvBuff, 5, 0) || *(DWORD*)(pbRecvBuff+1) != 0)
				{
					closesocket(sock);
					AfxMessageBox(SOCKET_ERROR_TEXT2);
					UpdateData(FALSE);
					return;
				}

				closesocket(sock);

				VMProtectEnd();

				AfxMessageBox(L"등록번호가 초기화되었습니다.");

				for(int i = 0 ; i < nSelectedCount ; i++ )
				{		
					m_listID.SetItemText(nCheckedList[i], 3, L"0000000000000000");
					ListView_SetCheckState(m_listID.GetSafeHwnd(), nCheckedList[i], 0);
				}
				UpdateData(FALSE);	

				delete pAccountInfoBuffer;
			}
		}
	}

	delete nCheckedList;

}

// 계정리스트의 번호항목을 다시 매긴다.
void CAdminDlg::ResetIndex(void)
{
	int i = 0;
	int	nCount = m_listID.GetItemCount();
	WCHAR	szTemp[MAX_PATH] = {0,};

	for(i = 0 ; i < nCount ; i++)
	{
		swprintf(szTemp, L"%d", i + 1);
		m_listID.SetItemText(i, 0, szTemp);
	}
}

void CAdminDlg::Show(BOOL bShow)
{
	if (bShow)
	{
		this->ShowWindow(SW_SHOW);
	}
	else
	{
		this->ShowWindow(SW_HIDE);
	}
}
void CAdminDlg::OnBnClickedBtnRefresh()
{
	SOCKADDR_IN		saddr;

	SOCKET sock = INVALID_SOCKET;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		return;
	}
	else
	{
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(g_szServerIP);
		saddr.sin_port = htons(SERVER_PORT_ADMINSECURE);

		if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)))
		{
			closesocket(sock);
			return;
		}
	}

	int				i, nLen, nLen2;
	BYTE			pbBuff[MAX_PATH];	
	BYTE			*pbBuff2 = NULL;
	WCHAR			szTemp[MAX_PATH];
	ACCOUNTINFO		*pAccountInfo;
	BYTE			*pbUserInfo = NULL;;

	nLen = MakePacket(pbBuff, "cdSS", OPCODE_QUERYANGLERSTATE, 0, (LPCTSTR)g_strAdminId, (LPCTSTR)g_strAdminPwd);	
	SendData(sock, (char*)pbBuff, nLen, 0);

	VMProtectEnd();

	nLen = RecvData(sock, (char*)pbBuff, 1, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		return;
	}

	if( pbBuff[0] != OPCODE_QUERYANGLERSTATE)
	{
		closesocket(sock);
		return;
	}

	nLen = RecvData(sock, (char*)pbBuff, 4, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		return;
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
		return;
	}

	closesocket(sock);

	DecryptPacket(pbBuff2, nLen2);

	int nStateNum = *(int*)pbBuff2;
	if(nStateNum == 0xFFFFFFFF)
	{
		return;
	}

	STATEINFO* pStateInfo = (STATEINFO*)(pbBuff2 + 4);

	int nAccountCnt = m_listID.GetItemCount();

	for(i = 0; i < nAccountCnt; i++)
	{
		WCHAR szAccount[20];
		m_listID.GetItemText(i, 1, szAccount, 20);

		int j = 0;
		for(j = 0; j < nStateNum; j ++)
		{
			if(wcscmp(szAccount, pStateInfo[j].szID) == 0)
			{
				WCHAR szState[100] = {0};
				if(wcslen(pStateInfo[j].szState) != 0)
					swprintf(szState, L"%s_%s", pStateInfo[j].szState, pStateInfo[j].szLastTime);
				//m_listID.SetItemText(i, 6, szState);
				//m_listID.SetItemText(i, 7, pStateInfo[j].szRestTime);
				break;
			}
		}
		if(j == nStateNum)
		{
			m_listID.SetItemText(i, 6, L"");
		}
	}

	OnBnClickedBtnSearchaccount();
	//OnBnClickedOk();
}


void CAdminDlg::OnBnClickedUploadT()
{
	UpdateData();

	
	if(m_szAddress == L"")
	{
		AfxMessageBox(L"설정을 확인해주십시오.");
		return;
	}

	LPTSTR endptr;
	DWORD dwAddress = _tcstol(m_szAddress.GetBuffer(),&endptr,16);

	WCHAR	szPath[MAX_PATH], szFileName[MAX_PATH];
	FILE	*fp;
	int		j, nLen;
	BYTE	*pbBuff;

	CA2W ca2wData(DATA_SET_NAME);
	CA2W ca2wDll(DLL_NAME);	
	CA2W ca2wExe(EXE_NAME);

	GetModuleFileName(NULL, szPath, MAX_PATH);
	wcsrchr(szPath, _T('\\'))[1] = 0;

	nLen = 0;

	swprintf(szFileName, L"%s%s", szPath, ca2wData.m_psz);
	fp = _wfopen(szFileName, L"rb");
	if(!fp)
	{
		AfxMessageBox(L"설정파일이 존재하지 않습니다.");
		return;
	}
	nLen += _filelength(_fileno(fp));
	fclose(fp);

	swprintf(szFileName, L"%s%s", szPath, ca2wDll.m_psz);
	fp = _wfopen(szFileName, L"rb");
	if(!fp)
		return;
	nLen += _filelength(_fileno(fp));
	fclose(fp);	

	swprintf(szFileName, L"%s%s", szPath, ca2wExe.m_psz);
	fp = _wfopen(szFileName, L"rb");
	if(!fp)
		return;
	nLen += _filelength(_fileno(fp));
	fclose(fp);

	nLen += 16;
	pbBuff = new BYTE[nLen];

	nLen = 0;
	swprintf(szFileName, L"%s%s", szPath, ca2wData.m_psz);
	fp = _wfopen(szFileName, L"rb");
	j = _filelength(_fileno(fp));
	*(int*)(pbBuff+nLen) = j;
	nLen += 4;
	fread(pbBuff+nLen, 1, j, fp);
	fclose(fp);

	nLen += j;
	swprintf(szFileName, L"%s%s", szPath, ca2wDll.m_psz);
	fp = _wfopen(szFileName, L"rb");
	j = _filelength(_fileno(fp));
	*(int*)(pbBuff+nLen) = j;
	nLen += 4;
	fread(pbBuff+nLen, 1, j, fp);
	fclose(fp);

	nLen += j;
	swprintf(szFileName, L"%s%s", szPath, ca2wExe.m_psz);
	fp = _wfopen(szFileName, L"rb");
	j = _filelength(_fileno(fp));
	*(int*)(pbBuff+nLen) = j;
	nLen += 4;
	fread(pbBuff+nLen, 1, j, fp);
	fclose(fp);

	nLen += j;

	memcpy(pbBuff+nLen, &dwAddress, 4);
	nLen += 4;

	GetModuleFileName(NULL, szPath, MAX_PATH);
	_tcsrchr(szPath, _T('\\'))[0] = 0;
	wcscat(szPath, L"\\Update.tmp");


	fp = _wfopen(szPath, L"wb");
	if(fp)
	{
		fwrite(pbBuff, nLen, 1, fp);
		fclose(fp);
	}

	LPBYTE pbySendBuffer = new BYTE[nLen + 100];
	VMProtectBegin("OnBnClickedUpload");
	if(IDCANCEL == AfxMessageBox(L"업데이트를 진행하시겠습니까?", MB_OKCANCEL))
	{
		nLen = MakePacket(pbySendBuffer, "cdSSd", OPCODE_ADMINFILE, 0, g_strAdminId, g_strAdminPwd, dwAddress);
	}
	else
	{
		nLen = MakePacket(pbySendBuffer, "cdSSbd", OPCODE_ADMINFILE, 0, g_strAdminId, g_strAdminPwd, pbBuff, nLen, dwAddress);
	}

	delete pbBuff;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"소켓창조가 실패하였습니다.");
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
			AfxMessageBox(L"서버접속이 실패하였습니다.");
			return;
		}
	}

	SendData(sock, (char*)pbySendBuffer, nLen, 0);

	delete pbySendBuffer;

	BYTE	pbRecvBuff[100];

	if(9 != RecvData(sock, (char*)pbRecvBuff, 9, 0) || pbRecvBuff[0] != OPCODE_ADMINFILE)
	{
		AfxMessageBox(SOCKET_ERROR_TEXT2);
		UpdateData(FALSE);
		return;
	}
	DecryptPacket(pbRecvBuff + 5, 4);
	if(*(DWORD*)(pbRecvBuff + 5) != 1)
	{
		AfxMessageBox(L"패치업로드가 실패하였습니다.");
		UpdateData(FALSE);
		return;
	}	
	AfxMessageBox(L"업로드에 성공하였습니다.");

	VMProtectEnd();
}

void CAdminDlg::OnBnClickedBtnAccountsetUpload()
{
	CXLEzAutomation XL(FALSE);	
	WCHAR szFileName[MAX_PATH];
	WCHAR szExcelPath[MAX_PATH];	
	WCHAR szPath[MAX_PATH], *p;
	GetModuleFileName(NULL, szPath, MAX_PATH);
	p = wcsrchr(szPath, '\\');
	if (p) p[0] = '\0';

	OutputDebugString(szPath);
	swprintf(szExcelPath, L"%s\\%s", szPath, L"Account.xlsx");
	swprintf(szFileName, L"%s\\%s", szPath, L"Account.ini");
	DeleteFile(szFileName);

	OutputDebugString(szExcelPath);
	OutputDebugString(szFileName);

	if (!XL.OpenExcelFile(szExcelPath))
	{
		AfxMessageBox(L"업로드파일이 없습니다.");
		return;
	}
	
	CString strText;

	int nCount = 0;

	int nRow = 1;
	BOOL bContinue = TRUE;
	while (TRUE)
	{
		nRow ++;
		CString strAccount;
		CString strGameId;
		CString strGamePwd;
		CString strGamePwd2;
		CString strCharCount;
		CString strGameServer;
		CString strPlayNC;
		CString strRoyalName;

		int nCol = 1;
		strAccount = XL.GetCellValue(nCol ++, nRow); 
		if (strAccount.IsEmpty())
			break;
		if (wcsstr(strAccount, L".0000"))
		{
			strAccount.Delete(strAccount.GetLength() - 7 , 7);
		}

		OutputDebugString(strAccount);

		int i;
		for (i = 0; i < m_listID.GetItemCount(); i ++)
		{
			OutputDebugString(m_listID.GetItemText(i, 1));
			if (strAccount == m_listID.GetItemText(i, 1) && g_strAdminId == m_listID.GetItemText(i, 8))
				break;
		}
		if (i >= m_listID.GetItemCount())
		{
			OutputDebugString(L"Step1");
			continue;
		}

		strGameId = XL.GetCellValue(nCol ++, nRow); 
		if (strGameId.IsEmpty())
			break;
		if (wcsstr(strGameId, L".0000"))
		{
			strGameId.Delete(strGameId.GetLength() - 7 , 7);
		}
		OutputDebugStringW(strGameId);

		strGamePwd = XL.GetCellValue(nCol ++, nRow); 
		if (strGamePwd.IsEmpty())
			break;
		if (wcsstr(strGamePwd, L".0000"))
		{
			strGamePwd.Delete(strGamePwd.GetLength() - 7 , 7);
		}
		OutputDebugStringW(strGamePwd);

		strGamePwd2 = XL.GetCellValue(nCol ++, nRow); 
		if (strGamePwd2.IsEmpty())
			break;
		if (wcsstr(strGamePwd2, L".0000"))
		{
			strGamePwd2.Delete(strGamePwd2.GetLength() - 7 , 7);
		}
		OutputDebugStringW(strGamePwd2);
		strCharCount = XL.GetCellValue(nCol ++, nRow);
		if (strCharCount.IsEmpty())
			break;
		if (wcsstr(strCharCount, L".0000"))
		{
			strCharCount.Delete(strCharCount.GetLength() - 7 , 7);
		}
		int nCharCount = _ttoi(strCharCount.GetBuffer());
		if (nCharCount > MAX_CHARNUM)
			nCharCount = MAX_CHARNUM;

		OutputDebugStringW(strCharCount);
		strGameServer = XL.GetCellValue(nCol ++, nRow);

		if (strGameServer.IsEmpty())
			break;

		OutputDebugStringW(strGameServer);
		/*OutputDebugStringW(strGameServer2);
		OutputDebugStringW(strGameServer3);
		OutputDebugStringW(strGameServer4);*/

		strPlayNC = XL.GetCellValue(nCol ++, nRow);
		if (strPlayNC.IsEmpty())
			break;
		BOOL bPlayNC = (strPlayNC == "Y");

		strRoyalName = XL.GetCellValue(nCol ++, nRow); 
		if (wcsstr(strRoyalName, L".0000"))
		{
			strRoyalName.Delete(strRoyalName.GetLength() - 7 , 7);
		}
		OutputDebugStringW(strRoyalName);

		int nAlreadyCount = GetPrivateProfileInt(strAccount.GetBuffer(), L"COUNT", 0, szFileName);
		//for (int j = 0; j < nCharCount; j ++)
		{
			strText.Format(L"ID%d", nAlreadyCount);
			WritePrivateProfileString(strAccount.GetBuffer(), strText, strGameId.GetBuffer(), szFileName);
			strText.Format(L"PASS%d", nAlreadyCount);
			WritePrivateProfileString(strAccount.GetBuffer(), strText, strGamePwd.GetBuffer(), szFileName);
			strText.Format(L"CHARPASS%d", nAlreadyCount);
			WritePrivateProfileString(strAccount.GetBuffer(), strText, strGamePwd2.GetBuffer(), szFileName);
			strText.Format(L"SERVER%d", nAlreadyCount);
			WritePrivateProfileString(strAccount.GetBuffer(), strText, strGameServer.GetBuffer(), szFileName);
			
			strText.Format(L"ROYAL%d", nAlreadyCount);
			WritePrivateProfileString(strAccount.GetBuffer(), strText, strRoyalName.GetBuffer(), szFileName);
			
			strText.Format(L"CHARNUM%d", nAlreadyCount);
			MyWritePrivateProfileIntW(strAccount.GetBuffer(), strText, nCharCount, szFileName);
			strText.Format(L"PLAYNC%d", nAlreadyCount);
			MyWritePrivateProfileIntW(strAccount.GetBuffer(), strText, bPlayNC, szFileName);
		}
		nAlreadyCount ++;
		MyWritePrivateProfileIntW(strAccount.GetBuffer(), L"COUNT", nAlreadyCount, szFileName);
			
		for (i = 0; i < nCount; i ++)
		{
			WCHAR szAccount[100] = {0,};
			strText.Format(L"ID%d", i); 
			GetPrivateProfileString(L"GENERAL", strText, L"", szAccount, 100, szFileName);
			if (wcscmp(strAccount.GetBuffer(), szAccount) == 0)
			{
				break;
			}
		}
		if (i >= nCount)
		{
			strText.Format(L"ID%d", nCount);
			WritePrivateProfileString(L"GENERAL", strText, strAccount.GetBuffer(), szFileName);
			nCount ++;
		}		
	}
	CString szCount;
	szCount.Format(L"%d", nCount);
	WritePrivateProfileString(L"GENERAL", L"COUNT", szCount, szFileName);
	XL.ReleaseExcel();

	if (nCount > 0)
	{
		for (int i = 0; i < nCount; i++)
		{
			WCHAR szAccount[20];
			WCHAR szAccountFile[MAX_PATH];

			strText.Format(L"ID%d", i);
			GetPrivateProfileString(L"GENERAL", strText, L"", szAccount, 20, szFileName);
			if (wcslen(szAccount) == 0)
				continue;

			swprintf(szAccountFile, L"%s\\%s", szPath, szAccount);
			DeleteFile(szAccountFile);

			int nAccountCount = GetPrivateProfileInt(szAccount, L"COUNT", 0, szFileName);
			for (int j = 0; j < nAccountCount; j ++)
			{
				WCHAR szGameId[40];
				WCHAR szGamePwd[40];
				WCHAR szGamePwd2[40];
				WCHAR szGameServer[40];
				WCHAR szRoyalName[40];
				int  nCharNum = 0;
				BOOL bPlayNC = 0;
				
				strText.Format(L"ID%d", j);
				GetPrivateProfileString(szAccount, strText, L"", szGameId, 40, szFileName);
				strText.Format(L"PASS%d", j);
				GetPrivateProfileString(szAccount, strText,L"", szGamePwd, 40, szFileName);
				strText.Format(L"CHARPASS%d", j);
				GetPrivateProfileString(szAccount, strText,L"", szGamePwd2, 40, szFileName);
				strText.Format(L"SERVER%d", j);
				GetPrivateProfileString(szAccount, strText,L"", szGameServer, 40, szFileName);
				strText.Format(L"ROYAL%d", j);
				GetPrivateProfileString(szAccount, strText,L"", szRoyalName, 40, szFileName);
				strText.Format(L"CHARNUM%d", j);
				nCharNum = GetPrivateProfileInt(szAccount, strText, 0, szFileName);
				strText.Format(L"PLAYNC%d", j);
				bPlayNC = GetPrivateProfileInt(szAccount, strText, 0, szFileName);
				
				strText.Format(L"ID%d", j);
				WritePrivateProfileString(L"ACCOUNT", strText, szGameId, szAccountFile);
				strText.Format(L"PASS%d", j);
				WritePrivateProfileString(L"ACCOUNT", strText, szGamePwd, szAccountFile);
				strText.Format(L"CHARPASS%d", j);
				WritePrivateProfileString(L"ACCOUNT", strText, szGamePwd2, szAccountFile);
				strText.Format(L"SERVER%d", j);
				WritePrivateProfileString(L"ACCOUNT", strText, szGameServer, szAccountFile);
				strText.Format(L"ROYAL%d", j);
				WritePrivateProfileString(L"ACCOUNT", strText, szRoyalName, szAccountFile);
				strText.Format(L"CHARNUM%d", j);
				MyWritePrivateProfileIntW(L"ACCOUNT", strText, nCharNum, szAccountFile);
				strText.Format(L"PLAYNC%d", j);
				MyWritePrivateProfileIntW(L"ACCOUNT", strText, bPlayNC, szAccountFile);
			}
			
			MyWritePrivateProfileIntW(L"ACCOUNT", L"COUNT", nAccountCount, szAccountFile);			

			FILE *fp;
			int	nLength;
			LPBYTE lpBuffer;
			
			fp = _wfopen(szAccountFile, L"rb");	
			if (!fp)
				continue;
			nLength = _filelength(_fileno(fp));
			if (nLength == 0)
			{
				fclose(fp);
				continue;
			}
			lpBuffer = new BYTE[nLength];
			if (lpBuffer == NULL)
			{
				fclose(fp);
				continue;
			}
			fread(lpBuffer, 1, nLength, fp);
			fclose(fp);
			
			VMProtectBegin("CAdminDlg::OnBnClickedBtnAccountsetUpload");

			LPBYTE pbySendBuffer = new BYTE[nLength + 100];
			nLength = MakePacket(pbySendBuffer, "cdSSSb", OPCODE_ACCOUNTSET_UPLOAD, 0, g_strAdminId, g_strAdminPwd, szAccount, lpBuffer, nLength);
			delete lpBuffer;

			SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(sock == INVALID_SOCKET)
			{
				AfxMessageBox(L"소켓창조가 실패하였습니다.");
				DeleteFile(szAccountFile);
				DeleteFile(szFileName);
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
					AfxMessageBox(L"서버접속이 실패하였습니다.");
					DeleteFile(szAccountFile);
					DeleteFile(szFileName);
					delete pbySendBuffer;
					return;
				}
			}

			SendData(sock, (char*)pbySendBuffer, nLength, 0);

			delete pbySendBuffer;

			BYTE	pbRecvBuff[100];

			if(9 != RecvData(sock, (char*)pbRecvBuff, 9, 0) || pbRecvBuff[0] != OPCODE_ACCOUNTSET_UPLOAD)
			{
				AfxMessageBox(SOCKET_ERROR_TEXT2);
				DeleteFile(szAccountFile);
				DeleteFile(szFileName);
				UpdateData(FALSE);
				return;
			}
			DecryptPacket(pbRecvBuff + 5, 4);
			if(*(DWORD*)(pbRecvBuff + 5) != 1)
			{
				AfxMessageBox(L"계정업로드가 실패하였습니다.");
				DeleteFile(szAccountFile);
				DeleteFile(szFileName);
				UpdateData(FALSE);
				return;
			}	
			DeleteFile(szAccountFile);
		}		
	}
	else
	{
		MessageBox(L"업로드할 계정이 없습니다.");
		return;
	}
	DeleteFile(szFileName);
	strText.Format(L"%d개의 오토계정 캐릭정보가 업로드되었습니다. ", nCount);
	MessageBox(strText, L"확인", MB_ICONEXCLAMATION);
}

BOOL CAdminDlg::MyWritePrivateProfileIntW(
						  __in_opt LPCWSTR lpAppName,
						  __in_opt LPCWSTR lpKeyName,
						  __in_opt INT nValue,
						  __in_opt LPCWSTR lpFileName
					   )
{
	WCHAR szValue[20];
	swprintf(szValue, L"%u", nValue);
	return WritePrivateProfileStringW(lpAppName, lpKeyName, szValue, lpFileName);
}

void CAdminDlg::OnEnChangeEdtReflashtick()
{
	UpdateData(TRUE);
	if(m_nReflashTick < 10)
	{
		m_nReflashTick = 10;
		UpdateData(FALSE);
	}
}

void CAdminDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	if(nIDEvent == 1)
	{
		KillTimer(nIDEvent);
		//OnBnClickedBtnRefresh();
		SetTimer(nIDEvent, m_nReflashTick * 1000, NULL);
	}
	if(nIDEvent == 2)
	{
		KillTimer(nIDEvent);
		(CButton*)(GetDlgItem(IDC_BTN_QUERYHISTORY))->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_QUERYHISTORY)->SetWindowText(L"상태조회");
	}
	CDialog::OnTimer(nIDEvent);
}


DWORD WINAPI QueryHistoryThread(LPVOID pParam)
{
	SOCKADDR_IN		saddr;

	SOCKET sock = INVALID_SOCKET;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		::MessageBox(NULL, L"소켓창조가 실패하였습니다.", NULL, MB_OK);
		((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
		return 0;
	}
	else
	{
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(g_szServerIP);
		saddr.sin_port = htons(SERVER_PORT_ADMINSECURE);

		if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)))
		{
			closesocket(sock);
			::MessageBox(NULL, L"서버접속이 실패하였습니다.", NULL, MB_OK);
			((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
			return 0;
		}
	}

	int				i, nLen, nLen2;
	BYTE			pbBuff[MAX_PATH];	
	BYTE			*pbBuff2 = NULL;
	WCHAR			szTemp[MAX_PATH];
	ACCOUNTINFO		*pAccountInfo;
	BYTE			*pbUserInfo = NULL;;

	SYSTEMTIME systimeStart;
	SYSTEMTIME systimeEnd;
	((CAdminDlg*)pParam)->m_timeQueryStart.GetAsSystemTime(systimeStart);
	((CAdminDlg*)pParam)->m_timeQueryEnd.GetAsSystemTime(systimeEnd);
	FILETIME filetimeStart;
	SystemTimeToFileTime(&systimeStart, &filetimeStart);
	FILETIME filetimeEnd;
	SystemTimeToFileTime(&systimeEnd, &filetimeEnd);

	BYTE pbBuffer[16] = {0,};
	memcpy(pbBuffer, &filetimeStart, sizeof(FILETIME));
	memcpy(pbBuffer+8, &filetimeEnd, sizeof(FILETIME));

	nLen = MakePacket(pbBuff, "cdSSb", OPCODE_QUERYANGLERHISTORY, 0, (LPCTSTR)g_strAdminId, (LPCTSTR)g_strAdminPwd, pbBuffer, 16);	
	SendData(sock, (char*)pbBuff, nLen, 0);

	VMProtectEnd();

	nLen = RecvData(sock, (char*)pbBuff, 1, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		::MessageBox(NULL, L"자료수신중 오류가 발생하였습니다.", NULL, MB_OK);
		((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
		return 0;
	}

	if( pbBuff[0] != OPCODE_QUERYANGLERHISTORY)
	{
		closesocket(sock);
		::MessageBox(NULL, L"자료수신중 오류가 발생하였습니다.", NULL, MB_OK);
		((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
		return 0;
	}

	nLen = RecvData(sock, (char*)pbBuff, 4, 0);
	if(nLen <= 0)
	{
		closesocket(sock);
		::MessageBox(NULL, L"자료수신중 오류가 발생하였습니다.", NULL, MB_OK);
		((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
		return 0;
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
		::MessageBox(NULL, L"자료수신중 오류가 발생하였습니다.", NULL, MB_OK);
		((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
		return 0;
	}

	closesocket(sock);

	DecryptPacket(pbBuff2, nLen2);

	int nCount = *(int*)pbBuff2;
	if(nCount == 0xFFFFFFFF)
	{
		::MessageBox(NULL, L"비번이 정확치 않습니다.", NULL, MB_OK);
		((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
		return 0;
	}

	HISTORYINFO* pStateInfo = (HISTORYINFO*)(pbBuff2 + 4);

	CString		sExcelFile = L"";                // Filename and path for the file to be created
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	WCHAR szPath[MAX_PATH], *p;
	GetModuleFileName(NULL, szPath, MAX_PATH);
	p = wcsrchr(szPath, '\\');
	if (p) p[0] = '\0';
	sExcelFile.Format( L"%s\\수금_%04d%02d%02d-%04d%02d%02d.xlsx", szPath,	
		systimeStart.wYear, systimeStart.wMonth, systimeStart.wDay,
		systimeEnd.wYear, systimeEnd.wMonth, systimeEnd.wDay);

	DeleteFile(sExcelFile);

	try{
		CXLEzAutomation XL(FALSE); // FALSE: 처리 과정을 화면에 보이지 않는다
		int nCol = 1, nRow = 1;
		XL.SetCellValue(nCol ++, nRow, L"오토계정");
		XL.SetCellValue(nCol ++, nRow, L"게임계정");
		XL.SetCellValue(nCol ++, nRow, L"서버");
		XL.SetCellValue(nCol ++, nRow, L"캐릭번호");	
		XL.SetCellValue(nCol ++, nRow, L"접속시간");
		XL.SetCellValue(nCol ++, nRow, L"찾은아덴");
		XL.SetCellValue(nCol ++, nRow, L"맡긴아덴");
		for (int i = 0; i < nCount; i++)
		{
			nRow ++;
			nCol = 1;
			CString strNum;
			strNum.Format(L"%d", pStateInfo[i].nCharNum);
			XL.SetCellValue(nCol ++, nRow, pStateInfo[i].szID);
			XL.SetCellValue(nCol ++, nRow, pStateInfo[i].szGameId);
			XL.SetCellValue(nCol ++, nRow, pStateInfo[i].szServer);
			XL.SetCellValue(nCol ++, nRow, strNum);
			XL.SetCellValue(nCol ++, nRow, pStateInfo[i].szConnectTime);
			strNum.Format(L"%d", pStateInfo[i].dwGetAden);		
			XL.SetCellValue(nCol ++, nRow, strNum);
			strNum.Format(L"%d", pStateInfo[i].dwDepositeAden);		
			XL.SetCellValue(nCol ++, nRow, strNum);
		}

		XL.SaveFileAs(sExcelFile);
		XL.ReleaseExcel();
	}
	catch(...)
	{
		::MessageBox(NULL, L"엑셀파일작성중 오류가 발생하였습니다.", NULL, MB_OK);
		((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
		return 0;
	}

	::MessageBox(NULL, L"상태조회가 완료되었습니다.", L"", MB_OK);

	((CAdminDlg*)pParam)->SetTimer(2, 1000, NULL);
	return 1;
}
void CAdminDlg::OnBnClickedBtnQueryhistory()
{
	(CButton*)(GetDlgItem(IDC_BTN_QUERYHISTORY))->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_QUERYHISTORY)->SetWindowText(L"상태조회중...");
	CreateThread(NULL, NULL, QueryHistoryThread, this, NULL, NULL);
	UpdateData(TRUE);	
}

void CAdminDlg::OnBnClickedBtnSearchaccount()
{
	int			i;
	POINT		ptItem;
	CSize		sizeItem;

	UpdateData(TRUE);

	for(i = 0; i < m_listID.GetItemCount(); i++)
	{
		if(m_listID.GetItemText(i, 1) == m_strSearchAccount)
			break;
	}

	if(i >= m_listID.GetItemCount())
	{
		return;
	}

	m_listID.SetItemState(i, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
	m_listID.GetItemPosition(i, &ptItem);
	sizeItem.cx = 0;
	sizeItem.cy = ptItem.y-15;
	m_listID.Scroll(sizeItem);
}

void CAdminDlg::AutoControlCmdSend(DWORD dwOpcode)
{
	CString szConfirmReport;
	int nCount = m_listID.GetItemCount();
	if(nCount == 0)
		return;

	int nSelectedCount = 0;
	int* nCheckedList = new int[nCount];
	for(int nItem = 0; nItem < nCount; nItem++)
	{
		if ( !ListView_GetCheckState(m_listID.GetSafeHwnd(), nItem) )
		{
		}
		else
		{
			nCheckedList[nSelectedCount] = nItem;
			nSelectedCount ++;
		}
	}
	if(nSelectedCount)
	{
		CString szConfirmReport;
		szConfirmReport = L"아래의 계정에 조종신호를 보냅니다.\n";
		for(int i = 0; i < min(10, nSelectedCount); i ++)
		{
			CString szAccount;
			TCHAR *sztmp = szAccount.GetBuffer(MAX_PATH);
			m_listID.GetItemText(nCheckedList[i], 1, sztmp, MAX_PATH);
			szAccount.ReleaseBuffer();
			szConfirmReport.Append(L"\n");
			szConfirmReport.Append(szAccount);
		}
		if(nSelectedCount > 10)
			szConfirmReport.Append(L"\n...");
		if( MessageBox(szConfirmReport, L"확인", MB_YESNO) == IDYES)
		{
			WCHAR		szTemp[MAX_PATH];
			int			nTemp, nTemp2, nTemp3;

			for(int i = 0; i < nSelectedCount; i ++)
			{
				if(g_pAccountInfo[nCheckedList[i]].dwLocalIP == 0 && g_pAccountInfo[nCheckedList[i]].dwConnectIP == 0)
					continue;

				AUTOCONTROLCMD * pAutoControlCmd = new AUTOCONTROLCMD;
				m_listID.GetItemText(nCheckedList[i], 1, szTemp, MAX_PATH);
				wcscpy(pAutoControlCmd->szAccount, szTemp);
				m_listID.GetItemText(nCheckedList[i], 2, szTemp, MAX_PATH);
				wcscpy(pAutoControlCmd->szPassword, szTemp);
				pAutoControlCmd->dwOpcode = dwOpcode;
				pAutoControlCmd->dwLocalIP = g_pAccountInfo[nCheckedList[i]].dwLocalIP;
				pAutoControlCmd->dwInternetIP = g_pAccountInfo[nCheckedList[i]].dwConnectIP;
				if(pAutoControlCmd->dwInternetIP == pAutoControlCmd->dwLocalIP)
					pAutoControlCmd->dwInternetIP = 0;
				CreateThread(NULL, NULL, AutoControlCmdSendThread, pAutoControlCmd, NULL, NULL);
			}
		}
	}
	delete nCheckedList;
	return;
}

void CAdminDlg::OnBnClickedBtnAutoend()
{
	AutoControlCmdSend(OPCODE_AUTOCTRLCMD_END);
}

void CAdminDlg::OnBnClickedBtnAutorerun()
{
	AutoControlCmdSend(OPCODE_AUTOCTRLCMD_RERUN);
}

void CAdminDlg::OnBnClickedBtnAutostop()
{
	AutoControlCmdSend(OPCODE_AUTOCTRLCMD_STOP);
}

void CAdminDlg::OnBnClickedBtnAutostart()
{
	AutoControlCmdSend(OPCODE_AUTOCTRLCMD_START);
}

void CAdminDlg::OnBnClickedBtnAutogamerun()
{
	AutoControlCmdSend(OPCODE_AUTOCTRLCMD_GAMERUN);
}

void CAdminDlg::OnBnClickedBtnAccountrefresh()
{
	OnBnClickedOk();
	//OnBnClickedBtnRefresh();
}

void CAdminDlg::OnCbnSelchangeCombo2()
{
	// TODO: Add your control notification handler code here
}

void CAdminDlg::OnBnClickedBtnStatstics()
{
	CMultiCountDownload dlgPopUp;

	if(dlgPopUp.DoModal() == IDOK)
	{
		//입력자료 체크
		if(dlgPopUp.m_StartDate >  dlgPopUp.m_LastDate)
		{
			AfxMessageBox(L"입력자료 오유입니다. 시작날자가 마감날자보다 큽니다.");
		}
		else
		{
			//Excel파일에 쓰기
			CXLEzAutomation XL(FALSE);
			CString sExcelFile;
			TCHAR szPath[MAX_PATH];
			GetModuleFileName(NULL, szPath, MAX_PATH);
			_tcsrchr(szPath, _T('\\'))[0] = 0;

			sExcelFile.Format( L"%s\\일별멀티개수조회_%04d%02d%02d_%04d%02d%02d.xlsx", szPath,dlgPopUp.m_StartDate.GetYear(), dlgPopUp.m_StartDate.GetMonth(), dlgPopUp.m_StartDate.GetDay(),
				dlgPopUp.m_LastDate.GetYear(), dlgPopUp.m_LastDate.GetMonth(), dlgPopUp.m_LastDate.GetDay());
			DeleteFile(sExcelFile);

			int nCol = 1, nRow = 1;
			CString szDate;
			XL.SetCellValue(nCol ++, nRow, L"오토계정");
			CTimeSpan plusTime(86400);
			std::vector<CString> AccountGroup;

			for(CTime iTime = dlgPopUp.m_StartDate; iTime <= dlgPopUp.m_LastDate; iTime = iTime + plusTime)
			{
				szDate.Format(L"%02d/%02d", iTime.GetMonth(), iTime.GetDay());
				XL.SetCellValue(nCol, 1, szDate);
				if(Download_MultiCountPerDate(iTime, &XL, nCol, AccountGroup) != 0)
				{
					szDate = L"자료수신중 오유가 발생하였습니다(오유 발생 날자: (" +  szDate;
					szDate += ")";
					AfxMessageBox(L"자료수신중 오유가 발생하였습니다.");
					return;
				}
				else
				{

				}
				nCol += 1;
			}
			AfxMessageBox(L"일별멀티개수조회에 성공하였습니다.");
			XL.SaveFileAs(sExcelFile);
			XL.ReleaseExcel();	
		}
	}
}


int CAdminDlg::Download_MultiCountPerDate(CTime pDate, CXLEzAutomation* XL, int nCol, std::vector<CString>& AccountGroup)
{

	VMProtectBegin("OnBnClickedButtonAcupload5");


	LPBYTE pbySendBuffer = new BYTE[6 + 100];
	int nLen = MakePacket(pbySendBuffer, "cddcc", OPCODE_MULTICOUNT_DATE, 0,  pDate.GetYear(), pDate.GetMonth(), pDate.GetDay());

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET)
	{
		AfxMessageBox(L"소켓창조가 실패하였습니다.");
		return 1;
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
			AfxMessageBox(L"서버접속이 실패하였습니다.");
			return 1;
		}
	}

	SendData(sock, (char*)pbySendBuffer, nLen, 0);

	delete pbySendBuffer;

	int nRecvLen = 0;
	BYTE nOPCode = -100;
	int	 nPacketLen = 0;
	nRecvLen = recv(sock, (char*)&nOPCode, 1, 0);
	if(nRecvLen <= 0)
	{
		closesocket(sock);
		return 1;
	}

	if(nOPCode != OPCODE_MULTICOUNT_DATE)
	{
		AfxMessageBox(SOCKET_ERROR_TEXT2);
		UpdateData(FALSE);
		return 1;
	}

	int nTemp = 0;
	while(nTemp < 4)
	{
		nRecvLen = recv(sock, ((char*)&nPacketLen)+nTemp, 4-nTemp, 0);
		if(nRecvLen <=0)
		{
			closesocket(sock);
			return 1;
		}
		nTemp += nRecvLen;
	}

	if(nPacketLen > 10000000 || nPacketLen < 0)
	{
		closesocket(sock);
		return 1;
	}

	LPBYTE	pbRecvBuff = new BYTE[nPacketLen];

	if(nPacketLen != RecvData(sock, (char*)pbRecvBuff, nPacketLen, 0))
	{
		AfxMessageBox(SOCKET_ERROR_TEXT2);
		UpdateData(FALSE);
		return 1;
	}
	DecryptPacket(pbRecvBuff, nPacketLen);

	int nWritenLen = 0;

	while(nWritenLen < nPacketLen)
	{
		CHAR szID[100];
		CString strValue;

		strcpy(szID, (CHAR*)pbRecvBuff);					pbRecvBuff += strlen(szID) + 1;			nWritenLen += strlen(szID) + 1;
		int nRow = 2;
		strValue = szID;
		//		MessageBoxA(NULL, szID, szID,MB_OKCANCEL);
		if(AccountGroup.size() == 0)
		{
			XL->SetCellValue(1, nRow, strValue);
			AccountGroup.push_back(strValue);
		}

		for(int i = 0; i <  AccountGroup.size(); i++)
		{
			if(AccountGroup[i].Compare(strValue) == 0)
			{
				nRow = i + 2;
				break;
			}
			else if(i == (AccountGroup.size() - 1))
			{
				strValue = szID;
				AccountGroup.push_back(strValue);
				nRow = i + 2 + 1;
				XL->SetCellValue(1, nRow, strValue);

			}
		}

		int nMultiCount = *(BYTE*)pbRecvBuff;			pbRecvBuff += 1;							nWritenLen += 1;	
		strValue.Format(L"%d", nMultiCount);
		XL->SetCellValue(nCol, nRow, strValue);
	}

	VMProtectEnd();
	return 0;

}
