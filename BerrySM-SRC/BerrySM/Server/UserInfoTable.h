#if !defined(AFX_USERINFOTABLE_H__4843177E_8BF4_4563_A9F2_882A7EEF6D3B__INCLUDED_)
#define AFX_USERINFOTABLE_H__4843177E_8BF4_4563_A9F2_882A7EEF6D3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BaseInfoTable.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserInfoTable recordset
#include <afxdb.h>

class CUserInfoTable : public CRecordset
{
public:
	CUserInfoTable(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CUserInfoTable)

// Field/Param Data
	//{{AFX_FIELD(CUserInfoTable, CRecordset)
	int		m_dwIndex;
	CString	m_szID;
	CString m_szPwd;
	CString	m_szRegDate;
	CString m_szExpDate;
	CString m_szMAC;
	BOOL	m_bEnable;
	CString m_szIP;
	int		m_dwState;
	CString	m_szAdmin;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserInfoTable)
	public:
	virtual CString GetDefaultConnect();    // Default connection string
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASEINFOTABLE_H__4843177E_8BF4_4563_A9F2_882A7EEF6D3B__INCLUDED_)
