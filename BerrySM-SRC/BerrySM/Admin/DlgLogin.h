#pragma once


// DlgLogin dialog

class CDlgLogin : public CDialog
{
	DECLARE_DYNAMIC(CDlgLogin)

public:
	CDlgLogin(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgLogin();

// Dialog Data
	enum { IDD = IDD_DLG_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedLogin();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedManageAdmin();
	CString m_szID;
	CString m_szPWD;
	BOOL CheckAdmin();
	BOOL AdminListDownload();
};

