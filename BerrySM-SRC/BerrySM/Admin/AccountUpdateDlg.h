#pragma once

#include "Admin.h"
#include "afxwin.h"
#include "afxdtctl.h"

// CAccountUpdateDlg dialog

class CAccountUpdateDlg : public CDialog
{
	DECLARE_DYNAMIC(CAccountUpdateDlg)

public:
	CAccountUpdateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAccountUpdateDlg();

	int			m_nAccountCount;
	ACCOUNTINFO * m_pAccountList;
	
// Dialog Data
	enum { IDD = IDD_DLG_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_ctrlIDList;
	CString m_szUpdatePwd;
	CTime m_ctUpdateExpTime;
	afx_msg void OnBnClickedOk();
	BOOL m_bUpdateKeyInit;
	int m_nMultiNum;
};
