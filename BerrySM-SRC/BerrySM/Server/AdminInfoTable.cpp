// BaseInfoTable.cpp : implementation file
//Engine

#include "stdafx.h"
#include "DNFServer.h"
#include "AdminInfoTable.h"
#include "ServerEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserInfoTable

IMPLEMENT_DYNAMIC(CAdminInfoTable, CRecordset)

CAdminInfoTable::CAdminInfoTable(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CUserInfoTable)
	m_dwIndex = 0;
	m_szID = _T("");
	m_szPwd = _T("");
	m_dwState = 0;
	m_dwLevel = 0;
	m_nFields = 5;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CAdminInfoTable::GetDefaultConnect()
{
	return _T("ODBC;DSN=DNFAuto");
}

CString CAdminInfoTable::GetDefaultSQL()
{
	return _T("[AdminInfo]");
}

void CAdminInfoTable::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CUserInfoTable)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Int(pFX, _T("[INDEX]"), m_dwIndex);
	RFX_Text(pFX, _T("[ID]"), m_szID);
	RFX_Text(pFX, _T("[PWD]"), m_szPwd);
	RFX_Int(pFX, _T("[STATE]"), m_dwState);
	RFX_Int(pFX, _T("[LEVEL]"), m_dwLevel);
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
