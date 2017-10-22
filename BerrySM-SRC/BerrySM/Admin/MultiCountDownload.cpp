// MultiCountDownload.cpp : implementation file
//

#include "stdafx.h"
#include "Admin.h"
#include "MultiCountDownload.h"


// CMultiCountDownload dialog

IMPLEMENT_DYNAMIC(CMultiCountDownload, CDialog)

CMultiCountDownload::CMultiCountDownload(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiCountDownload::IDD, pParent)
	, m_StartDate(0)
	, m_LastDate(0)
{

}

CMultiCountDownload::~CMultiCountDownload()
{
}

void CMultiCountDownload::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_StartDate);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER2, m_LastDate);
}


BEGIN_MESSAGE_MAP(CMultiCountDownload, CDialog)
	ON_BN_CLICKED(IDOK, &CMultiCountDownload::OnBnClickedOk)
END_MESSAGE_MAP()


// CMultiCountDownload message handlers

void CMultiCountDownload::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	OnOK();
}

BOOL CMultiCountDownload::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	SYSTEMTIME systimeTemp;
	GetSystemTime(&systimeTemp);
	//마감날을 현재시간으로 설정한다.
	m_LastDate = CTime(systimeTemp);

	//시잘날자를 현재달의 1일로 설정
	systimeTemp.wDay = 1;
	m_StartDate = CTime(systimeTemp);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
