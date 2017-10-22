// DBManager.h: interface for the CDBManager class.
// Engine
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBMANAGER_H__7C5946AC_2C3C_4C71_9C53_E7489ED99AC4__INCLUDED_)
#define AFX_DBMANAGER_H__7C5946AC_2C3C_4C71_9C53_E7489ED99AC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UserInfoTable.h"
#include "AdminInfoTable.h"
#include "ServerEngine.h"

class CDBManager  
{
public:
	CDBManager();
	virtual ~CDBManager();

public:
	
	void Close();
	BOOL Open();

	BOOL GetUserInfo( DWORD nIndex, ACCOUNTINFO *pAccountInfo, TCHAR *pUserID = NULL, TCHAR *pAdminID = NULL );
	BOOL UpdateUserInfoTable(int nFlag, ACCOUNTINFO *pAccountInfo);

	BOOL GetAdminInfo( DWORD nIndex, ADMININFO *pAdminInfo, TCHAR *pAdminID = NULL );
	BOOL UpdateAdminInfoTable(int nFlag, ADMININFO *pAdminInfo);

	CDatabase				m_BaseInfoDB;
	CUserInfoTable			m_UserInfoTable;
	CAdminInfoTable			m_AdminInfoTable;
};

extern CDBManager g_dbmanager;

#endif // !defined(AFX_DBMANAGER_H__7C5946AC_2C3C_4C71_9C53_E7489ED99AC4__INCLUDED_)
