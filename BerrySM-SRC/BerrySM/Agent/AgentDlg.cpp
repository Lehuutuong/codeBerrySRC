// AgentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Agent.h"
#include "AgentDlg.h"
#include "ntdll.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAgentDlg dialog



DWORD WINAPI AgentRoutine();

CAgentDlg::CAgentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAgentDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAgentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAgentDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CAgentDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CAgentDlg message handlers

BOOL CAgentDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	// TODO: Add extra initialization here

	::CreateThread(NULL, 1024, (LPTHREAD_START_ROUTINE)AgentRoutine, NULL, 0, NULL);
	MinimizeToTray();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAgentDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAgentDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAgentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI AgentRoutine()
{
	WCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	wcscpy(wcsrchr(szPath, L'\\')+1, L"Server.exe");
	HANDLE Mutex = NULL;
	while(TRUE)
	{
		Mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"SERVER");
		if(Mutex == NULL)
		{
			Sleep(2000);
			ShellExecute(NULL, L"open", szPath, NULL, NULL, SW_SHOW);
			Sleep(10000);
		}
		else
		{
			::CloseHandle(Mutex);
			Sleep(10 * 1000);
			continue;
		}
	}
	
	return 1;
}


void CAgentDlg::MinimizeToTray(void)
{
	NOTIFYICONDATA ni;

	HICON hIcon = (HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDR_MAINFRAME), RT_GROUP_ICON), MAKEINTRESOURCE(IDR_MAINFRAME), 1, 0x10, 0x10, 0);
	ni.hIcon = hIcon;
	ni.cbSize = sizeof(ni);
	ni.hWnd = GetSafeHwnd();
	ni.uID = IDR_MAINFRAME;
	ni.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	ni.uCallbackMessage = WM_USER + 0x14D;
	wcscpy(ni.szTip, L"Agent");
	Shell_NotifyIcon(NIM_ADD, &ni);
	ShowWindow(SW_HIDE);
	if(hIcon) 
		DestroyIcon(hIcon);
}

LRESULT CAgentDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

	switch (message)
	{
	case WM_USER + 0x14D:
		if(lParam == WM_LBUTTONDBLCLK)
		{
			if(IsWindowVisible())
				ShowWindow(SW_HIDE);
			else
				ShowWindow(SW_SHOW);
		}
		break; 
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CAgentDlg::OnBnClickedCancel()
{
	NOTIFYICONDATA ni;
	RtlZeroMemory(&ni, sizeof(ni));
	ni.cbSize = sizeof(ni);
	ni.hWnd= m_hWnd;
	ni.uID = IDR_MAINFRAME;
	Shell_NotifyIcon(NIM_DELETE, &ni);

	OnCancel();
}
