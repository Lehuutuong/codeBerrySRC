// AccountUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Admin.h"
#include "AccountUpdateDlg.h"


// CAccountUpdateDlg dialog

IMPLEMENT_DYNAMIC(CAccountUpdateDlg, CDialog)

CAccountUpdateDlg::CAccountUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAccountUpdateDlg::IDD, pParent)
	, m_szUpdatePwd(_T(""))
	, m_ctUpdateExpTime(0)
	, m_bUpdateKeyInit(FALSE)
	, m_nMultiNum(0)
{
	m_nAccountCount = 0;
	m_pAccountList = NULL;
}

CAccountUpdateDlg::~CAccountUpdateDlg()
{
}

void CAccountUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ID, m_ctrlIDList);
	DDX_Text(pDX, IDC_PASSWORD, m_szUpdatePwd);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_UPDATEEXP, m_ctUpdateExpTime);
	DDX_Check(pDX, IDC_CHK_INIT, m_bUpdateKeyInit);
	DDX_CBIndex(pDX, IDC_COMBO2, m_nMultiNum);
}

BEGIN_MESSAGE_MAP(CAccountUpdateDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CAccountUpdateDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CAccountUpdateDlg message handlers

BOOL CAccountUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	if(m_nAccountCount)
	{
		for(int i = 0; i < m_nAccountCount; i ++)
		{
			m_ctrlIDList.AddString(m_pAccountList[i].szAccount);
		}
	}

	m_szUpdatePwd = m_pAccountList[0].szPassword;
	CTime	timeTemp(*(short*)m_pAccountList[0].pbExpDate, m_pAccountList[0].pbExpDate[2], m_pAccountList[0].pbExpDate[3], 1 ,1 ,1);
	m_ctUpdateExpTime = timeTemp;

	UpdateData(FALSE);
	return TRUE;
}

void CAccountUpdateDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	if(m_szUpdatePwd.GetLength() > 15)
	{
		AfxMessageBox(L"비번은 영문과 숫자로 15자 이하여야 합니다.");
		return;
	}
	for(int i = 0; i < m_nAccountCount; i ++)
	{
		SYSTEMTIME systime;
		GetLocalTime(&systime);
		systime.wYear = *(short*)m_pAccountList[i].pbRegDate;
		systime.wMonth = m_pAccountList[i].pbRegDate[2];
		systime.wDay = m_pAccountList[i].pbRegDate[3];
		ULONGLONG filetimeStart1;
		SystemTimeToFileTime(&systime, (LPFILETIME)&filetimeStart1);

		*(short*)m_pAccountList[i].pbExpDate = (short)m_ctUpdateExpTime.GetYear();
		m_pAccountList[i].pbExpDate[2] = (BYTE)m_ctUpdateExpTime.GetMonth();
		m_pAccountList[i].pbExpDate[3] = (BYTE)m_ctUpdateExpTime.GetDay();
		systime.wYear = *(short*)m_pAccountList[i].pbExpDate;
		systime.wMonth = m_pAccountList[i].pbExpDate[2];
		systime.wDay = m_pAccountList[i].pbExpDate[3];
		ULONGLONG filetimeStart2;
		SystemTimeToFileTime(&systime, (LPFILETIME)&filetimeStart2);
		if(filetimeStart1 > filetimeStart2)
		{
			AfxMessageBox(L"날짜가 정확하지 않은 계정이 존재합니다.");
			return;
		}
		wcscpy(m_pAccountList[i].szPassword,m_szUpdatePwd.GetBuffer());
		m_pAccountList[i].nMaxMulti = m_nMultiNum;
		if(m_bUpdateKeyInit)
			memset(m_pAccountList[i].pbAccount, 0, sizeof(m_pAccountList[i].pbAccount));

	}
	OnOK();
}
