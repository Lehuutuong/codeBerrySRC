// BaseInfoTable.cpp : implementation file
//Engine

#include "stdafx.h"
#include "DNFServer.h"
#include "UserInfoTable.h"
#include "ServerEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserInfoTable

IMPLEMENT_DYNAMIC(CUserInfoTable, CRecordset)

CUserInfoTable::CUserInfoTable(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CUserInfoTable)
	m_dwIndex = 0;
	m_szID = _T("");
	m_szPwd = _T("");
	m_szRegDate = _T("");
	m_szExpDate = _T("");
	m_szMAC = _T("");
	m_bEnable = TRUE;
	m_szIP = _T("");
	m_dwState = 0;
	m_szAdmin = _T("");
	m_nFields = 10;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CUserInfoTable::GetDefaultConnect()
{
	return _T("ODBC;DSN=DNFAuto");
}

CString CUserInfoTable::GetDefaultSQL()
{
	return _T("[UserInfo]");
}

void CUserInfoTable::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CUserInfoTable)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[INDEX]"), m_dwIndex);
	RFX_Text(pFX, _T("[ID]"), m_szID);
	RFX_Text(pFX, _T("[PWD]"), m_szPwd);
	RFX_Text(pFX, _T("[REGDATE]"), m_szRegDate);
	RFX_Text(pFX, _T("[EXPDATE]"), m_szExpDate);
	RFX_Text(pFX, _T("[MAC]"), m_szMAC);
	RFX_Bool(pFX, _T("[USE]"), m_bEnable);
	RFX_Text(pFX, _T("[IP]"), m_szIP);
	RFX_Int(pFX, _T("[STATE]"), m_dwState);
	RFX_Text(pFX, _T("[ADMIN]"), m_szAdmin);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CUserInfoTable diagnostics

#ifdef _DEBUG
void CUserInfoTable::AssertValid() const
{
	CRecordset::AssertValid();
}

void CUserInfoTable::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
