// AdminDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "CheckListCtrl.h"
#include "afxdtctl.h"
#include "XLEzAutomation.h"
#include <vector>
#define SOCKET_ERROR_TEXT1 L"자료송신중 오류가 발생하였습니다."
#define SOCKET_ERROR_TEXT2 L"자료수신중 오류가 발생하였습니다."

// CAdminDlg dialog
class CAdminDlg : public CDialog
{
// Construction
public:
	CAdminDlg(CWnd* pParent = NULL);	// standard constructor
	BOOL		m_bModify;
	int			m_nAccountIndex;

// Dialog Data
	enum { IDD = IDD_ADMIN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void AddID(int nInd);
	void UpdateIDList(int nCount, int *pSelectedList, ACCOUNTINFO * pAccountList);

	int SearchID(const WCHAR *szID,int nIndex);

	

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CCheckListCtrl m_listID;
	CString m_strAdminPW;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedAddid();
	CButton m_btnAdd;
	CButton m_btnDel;
	CButton m_btnModify;
	CButton m_btnConnect;
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	int m_nIndex;
	afx_msg void OnBnClickedRemoveid();
	afx_msg void OnBnClickedModifyid();
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_btnAddFromFile;
	afx_msg void OnBnClickedAddfromfile();
protected:
	virtual void OnCancel();
public:
	afx_msg void OnBnClickedBtnId();
	afx_msg void OnEnChangeEdit4();
	CButton m_btnClear;
	afx_msg void OnBnClickedBtnClear();
	CString m_strAdminID;
	void ResetIndex(void);
	CString m_szAddress;
	BOOL CreateCustom(UINT nID, CWnd* pParentWnd);
	void Show(BOOL bShow);
	CString m_strAccount;
	CString m_strPassword;
	CTime m_timeReg;
	CTime m_timeExpire;
	CString m_strRegID;
	afx_msg void OnBnClickedBtnRefresh();
	CButton m_btnUploadT;
	afx_msg void OnBnClickedUploadT();
	afx_msg void OnBnClickedBtnAccountsetUpload();
	BOOL MyWritePrivateProfileIntW(__in_opt LPCWSTR lpAppName,__in_opt LPCWSTR lpKeyName,__in_opt INT nValue,__in_opt LPCWSTR lpFileName);
	CButton m_btnAccountSetUpload;
	int m_nReflashTick;
	afx_msg void OnEnChangeEdtReflashtick();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void AutoControlCmdSend(DWORD dwOpcode);

	CTime m_timeQueryStart;
	CTime m_timeQueryEnd;
	afx_msg void OnBnClickedBtnQueryhistory();
	CString m_strSearchAccount;
	afx_msg void OnBnClickedBtnSearchaccount();
	afx_msg void OnBnClickedBtnAutoend();
	afx_msg void OnBnClickedBtnAutorerun();
	afx_msg void OnBnClickedBtnAutostop();
	afx_msg void OnBnClickedBtnAutostart();
	afx_msg void OnBnClickedBtnAutogamerun();
	afx_msg void OnBnClickedBtnAccountrefresh();
	int m_nMaxMulti;
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnBnClickedBtnStatstics();
	int Download_MultiCountPerDate(CTime pDate, CXLEzAutomation* XL, int nCol, std::vector<CString>& AccountGroup);
};
extern CAdminDlg	*pAdminDlg;