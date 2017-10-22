#pragma once


// CMultiCountDownload dialog

class CMultiCountDownload : public CDialog
{
	DECLARE_DYNAMIC(CMultiCountDownload)

public:
	CMultiCountDownload(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMultiCountDownload();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CTime m_StartDate;
	CTime m_LastDate;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
