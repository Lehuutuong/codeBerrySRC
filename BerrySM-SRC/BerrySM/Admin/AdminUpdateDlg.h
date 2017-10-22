#pragma once
#include "afxcmn.h"
#include "afxwin.h"

class CAdminUpdateDlg : public CDialog
{
	DECLARE_DYNAMIC(CAdminUpdateDlg)

public:
	CAdminUpdateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAdminUpdateDlg();

	// Dialog Data
	enum { IDD = IDD_DLG_ADMINUPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_cAdminList;
	CString m_szID;
	CString m_szPwd;
	CComboBox m_cLevel;
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnChange();
	afx_msg void OnBnClickedBtnDel();
	virtual BOOL OnInitDialog();
	void UpdateAdminInfo(int nInd);
	afx_msg void OnLvnItemchangedLstAdmin(NMHDR *pNMHDR, LRESULT *pResult);
	int m_nIndex;
protected:
	virtual void OnCancel();
public:
	afx_msg void OnBnClickedCancel();
	void ResetIndex(void);
	int m_nMaxUser;
	CString m_strKey;
};
