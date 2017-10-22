// DBManager.cpp: implementation of the CDBManager class.
//Engine
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBManager.h"
#include "DNFServer.h"
#include "ServerEngine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDBManager g_dbmanager;

CDBManager::CDBManager()
{

}

CDBManager::~CDBManager()
{

}

BOOL CDBManager::Open()
{
	CString szConnect;
	BOOL bResult;

	WCHAR szPath[MAX_PATH] = {0,};
	int i = 0;
	
	GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);

	i = wcslen(szPath) - 1;
	while(szPath[i] != L'\\')
		i--;
	szPath[i+1] = 0;
	wcscat(szPath, L"Account.mdb");	

	szConnect.Format(_T("DRIVER={Microsoft Access Driver (*.mdb)};DBQ=%s;PWD=QhQhgownfrp2;"), szPath);

	try
	{		
		bResult = m_BaseInfoDB.OpenEx( szConnect );

		if ( bResult )
		{
			m_UserInfoTable.m_pDatabase = &m_BaseInfoDB;
			m_AdminInfoTable.m_pDatabase = &m_BaseInfoDB;
		}

	}
	catch (CDBException* e)
	{
		e->Delete();

		if ( m_BaseInfoDB.IsOpen() )
			m_BaseInfoDB.Close();
		return FALSE;
	}

	return bResult;
}	

void CDBManager::Close()
{
	try
	{	
		if ( m_BaseInfoDB.IsOpen() )
			m_BaseInfoDB.Close();
	}
	catch(CDBException *e)
	{
		e->Delete();
	}
}

BOOL CDBManager::UpdateUserInfoTable(int nFlag, ACCOUNTINFO *pAccountInfo)
{
	try
	{
		m_UserInfoTable.m_strFilter.Empty();
		m_UserInfoTable.m_strSort.Empty();

		if (m_UserInfoTable.IsOpen())
			m_UserInfoTable.Close();
		
		if( nFlag != 1 )
			m_UserInfoTable.m_strFilter.Format( L"ID='%s'", pAccountInfo->szAccount );
		
		m_UserInfoTable.m_strSort = L"INDEX ASC";
		
		m_UserInfoTable.Open();
		switch (nFlag)
		{
		case 0: //Edit
			m_UserInfoTable.Edit();
			m_UserInfoTable.m_szPwd.Format(L"%s", pAccountInfo->szPassword);
			m_UserInfoTable.m_szRegDate.Format(L"%04d-%02d-%02d", *(short*)&pAccountInfo->pbRegDate[0], pAccountInfo->pbRegDate[2], pAccountInfo->pbRegDate[3]);
			m_UserInfoTable.m_szExpDate.Format(L"%04d-%02d-%02d", *(short*)&pAccountInfo->pbExpDate[0], pAccountInfo->pbExpDate[2], pAccountInfo->pbExpDate[3]);
			m_UserInfoTable.m_szMAC.Format(L"%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", pAccountInfo->pbAccount[0], pAccountInfo->pbAccount[1], pAccountInfo->pbAccount[2],
				pAccountInfo->pbAccount[3], pAccountInfo->pbAccount[4], pAccountInfo->pbAccount[5], pAccountInfo->pbAccount[6], pAccountInfo->pbAccount[7]) ;
			m_UserInfoTable.m_szAdmin.Format(L"%s", pAccountInfo->szAdminName);
			m_UserInfoTable.m_dwState = pAccountInfo->dwStatus;
			m_UserInfoTable.Update();
			break;
		case 1: //Add
			m_UserInfoTable.AddNew();
			m_UserInfoTable.m_szID.Format(L"%s", pAccountInfo->szAccount);
			m_UserInfoTable.m_szPwd.Format(L"%s", pAccountInfo->szPassword);
			m_UserInfoTable.m_szRegDate.Format(L"%04d-%02d-%02d", *(short*)&pAccountInfo->pbRegDate[0], pAccountInfo->pbRegDate[2], pAccountInfo->pbRegDate[3]);
			m_UserInfoTable.m_szExpDate.Format(L"%04d-%02d-%02d", *(short*)&pAccountInfo->pbExpDate[0], pAccountInfo->pbExpDate[2], pAccountInfo->pbExpDate[3]);
			m_UserInfoTable.m_szMAC.Format(L"%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", pAccountInfo->pbAccount[0], pAccountInfo->pbAccount[1], pAccountInfo->pbAccount[2],
				pAccountInfo->pbAccount[3], pAccountInfo->pbAccount[4], pAccountInfo->pbAccount[5], pAccountInfo->pbAccount[6], pAccountInfo->pbAccount[7]) ;
			m_UserInfoTable.m_szAdmin.Format(L"%s", pAccountInfo->szAdminName);
			m_UserInfoTable.m_dwState = pAccountInfo->dwStatus;
			m_UserInfoTable.Update();
			break;
		case 2: //Del
			m_UserInfoTable.Delete();
			break;
		}
		m_UserInfoTable.Close();		
	}
	catch (CDBException* e)
	{
		e->Delete();
		if (m_UserInfoTable.IsOpen())
			m_UserInfoTable.Close();
		return FALSE;
	}
	return TRUE;
}

BOOL CDBManager::GetUserInfo( DWORD nIndex, ACCOUNTINFO* pAccountInfo, TCHAR *pUserID,  TCHAR *pAdminID )
{
	int i = 0;
	try
	{
		ZeroMemory ( pAccountInfo, sizeof( ACCOUNTINFO ) );
		m_UserInfoTable.m_strFilter.Empty();
		m_UserInfoTable.m_strSort.Empty();
		
		if( m_UserInfoTable.IsOpen() ) 
			m_UserInfoTable.Close();
		
		if ( pUserID )
		{
			m_UserInfoTable.m_strFilter.Format( L"ID='%s'", pUserID ); 
		}
		else if ( pAdminID )
		{
			m_UserInfoTable.m_strFilter.Format( L"ADMIN='%s'", pAdminID );
		}
		
		
		m_UserInfoTable.m_strSort = L"INDEX ASC" ;		
		m_UserInfoTable.Open();
	
		if( m_UserInfoTable.GetRecordCount() > 0 ) 
		{
			if ( !m_UserInfoTable.IsBOF() ) 
				m_UserInfoTable.MoveFirst();
		}
		else 
		{
			m_UserInfoTable.Close();
			return FALSE;
		}
		
		m_UserInfoTable.Move( nIndex );
		
		if( m_UserInfoTable.IsEOF() )
		{
			m_UserInfoTable.Close();
			return FALSE;
		}
		
		//계정, 패스워드, 관리자
		wcscpy(pAccountInfo->szAccount, m_UserInfoTable.m_szID.GetBuffer(20));
		wcscpy(pAccountInfo->szPassword, m_UserInfoTable.m_szPwd.GetBuffer(20));
		wcscpy(pAccountInfo->szAdminName, m_UserInfoTable.m_szAdmin.GetBuffer(20));

		//접속상태
		pAccountInfo->dwStatus = m_UserInfoTable.m_dwState;

		CString	szTemp;
		WCHAR	*szEnd;

		//맥어드레스
		for(i = 0 ; i < 8 ; i++)
		{
			szTemp = m_UserInfoTable.m_szMAC.Mid(i * 3, 2);
			pAccountInfo->pbAccount[i] = wcstol((LPCWSTR)szTemp, &szEnd, 16);
		}

		//등록날자
		szTemp = m_UserInfoTable.m_szRegDate.Left(4);
		*(short*)&pAccountInfo->pbRegDate[0] = wcstol(szTemp.GetBuffer(20), &szEnd, 10);
		szTemp = m_UserInfoTable.m_szRegDate.Mid(5, 2);
		pAccountInfo->pbRegDate[2] = wcstol(szTemp.GetBuffer(20), &szEnd, 10);
		szTemp = m_UserInfoTable.m_szRegDate.Mid(8,2);
		pAccountInfo->pbRegDate[3] = wcstol(szTemp.GetBuffer(20), &szEnd, 10);

		//마감날자
		szTemp = m_UserInfoTable.m_szExpDate.Left(4);
		*(short*)&pAccountInfo->pbExpDate[0] = wcstol(szTemp.GetBuffer(20), &szEnd, 10);
		szTemp = m_UserInfoTable.m_szExpDate.Mid(5, 2);
		pAccountInfo->pbExpDate[2] = wcstol(szTemp.GetBuffer(20), &szEnd, 10);
		szTemp = m_UserInfoTable.m_szExpDate.Mid(8,2);
		pAccountInfo->pbExpDate[3] = wcstol(szTemp.GetBuffer(20), &szEnd, 10);
		
		m_UserInfoTable.Close();
	}
	catch (CDBException* e)
	{
		e->Delete();
		if (m_UserInfoTable.IsOpen())
			m_UserInfoTable.Close();
		return FALSE;
	}	
	
	return TRUE;
}

BOOL CDBManager::GetAdminInfo( DWORD nIndex, ADMININFO *pAdminInfo, TCHAR *pAdminID )
{
	int i = 0;
	try
	{
		ZeroMemory ( pAdminInfo, sizeof( ADMININFO ) );
		m_AdminInfoTable.m_strFilter.Empty();
		m_AdminInfoTable.m_strSort.Empty();

		if( m_AdminInfoTable.IsOpen() ) 
			m_AdminInfoTable.Close();

		if ( pAdminID )
		{
			m_AdminInfoTable.m_strFilter.Format( L"ID='%s'", pAdminID ); 
		}
		
		m_AdminInfoTable.m_strSort = L"INDEX ASC" ;		
		m_AdminInfoTable.Open();

		if( m_AdminInfoTable.GetRecordCount() > 0 ) 
		{
			if ( !m_AdminInfoTable.IsBOF() ) 
				m_AdminInfoTable.MoveFirst();
		}
		else 
		{
			m_AdminInfoTable.Close();
			return FALSE;
		}

		m_AdminInfoTable.Move( nIndex );

		if( m_AdminInfoTable.IsEOF() )
		{
			m_AdminInfoTable.Close();
			return FALSE;
		}

		wcscpy(pAdminInfo->szID, m_AdminInfoTable.m_szID.GetBuffer(20));
		wcscpy(pAdminInfo->szPwd, m_AdminInfoTable.m_szPwd.GetBuffer(20));
		pAdminInfo->dwLevel = m_AdminInfoTable.m_dwLevel;
		pAdminInfo->dwState = m_AdminInfoTable.m_dwState;
		
		m_AdminInfoTable.Close();
	}
	catch (CDBException* e)
	{
		e->Delete();
		if (m_AdminInfoTable.IsOpen())
			m_AdminInfoTable.Close();
		return FALSE;
	}	

	return TRUE;
}

BOOL CDBManager::UpdateAdminInfoTable(int nFlag, ADMININFO *pAdminInfo)
{
	try
	{
		m_AdminInfoTable.m_strFilter.Empty();
		m_AdminInfoTable.m_strSort.Empty();

		if (m_AdminInfoTable.IsOpen())
			m_AdminInfoTable.Close();

		if( nFlag != 1 )
			m_AdminInfoTable.m_strFilter.Format( L"ID='%s'", pAdminInfo->szID );

		m_AdminInfoTable.m_strSort = L"INDEX ASC";

		m_AdminInfoTable.Open();
		switch (nFlag)
		{
		case 0: //Edit
			m_AdminInfoTable.Edit();
			m_AdminInfoTable.m_szPwd.Format(L"%s", pAdminInfo->szPwd);
			m_AdminInfoTable.m_dwState = pAdminInfo->dwState;
			m_AdminInfoTable.m_dwLevel = pAdminInfo->dwLevel;
			m_AdminInfoTable.Update();
			break;
		case 1: //Add
			m_AdminInfoTable.AddNew();
			m_AdminInfoTable.m_szID.Format(L"%s", pAdminInfo->szID);
			m_AdminInfoTable.m_szPwd.Format(L"%s", pAdminInfo->szPwd);
			m_AdminInfoTable.m_dwState = pAdminInfo->dwState;
			m_AdminInfoTable.m_dwLevel = pAdminInfo->dwLevel;
			m_AdminInfoTable.Update();
			break;
		case 2: //Del
			m_AdminInfoTable.Delete();
			break;
		}
		m_AdminInfoTable.Close();		
	}
	catch (CDBException* e)
	{
		e->Delete();
		if (m_AdminInfoTable.IsOpen())
			m_AdminInfoTable.Close();
		return FALSE;
	}
	return TRUE;
}
