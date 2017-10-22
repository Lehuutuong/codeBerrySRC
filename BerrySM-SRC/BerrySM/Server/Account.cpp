#include "stdafx.h"
#include "Account.h"
#include "ServerEngine.h"
#include "io.h"
#include "AnglerState.h"

int					g_nAccountNum = 0;
ACCOUNTINFO			*g_pAccountInfo = NULL;
int					g_nAdminNum = 0;
ADMININFO			*g_pAdminInfo = NULL;
int					g_nAnglerStateNum = 0;
STATEINFO			*g_pAnglerStateInfo = NULL;

void	SaveAccountInfo()
{
	WCHAR	szPath[MAX_PATH];
	FILE	*fp;
	BYTE	*pbBuffer, pbRand;
	int		i, nLen;

	wcscpy(szPath, g_szServerDataFilePath);
	wcscat(szPath, L"Profile");
	wcscat(szPath, L"\\" SERVER_PROFILE);

	// 자료암호화
	nLen = g_nAccountNum*sizeof(ACCOUNTINFO)+4;
	pbBuffer = new BYTE[nLen];
	ZeroMemory(pbBuffer, nLen);
	*(int *)pbBuffer = g_nAccountNum;
	
	if(g_nAccountNum)
		memcpy(pbBuffer+4, g_pAccountInfo, sizeof(ACCOUNTINFO)*g_nAccountNum);

	pbRand = 0x17;
	for(i = 0; i < nLen; i++)
	{
		pbBuffer[i] ^= pbRand;
		pbRand = pbBuffer[i];
	}
	fp = _wfopen(szPath, L"wb");
	if(!fp)
	{
		PrintLog(L"데이터베이스보관실패");
		delete [] pbBuffer;
		return;
	}
	fwrite(pbBuffer, 1, nLen, fp);
	fclose(fp);
	delete [] pbBuffer;
}

void	SaveAdminInfo()
{
	WCHAR	szPath[MAX_PATH];
	FILE	*fp;
	BYTE	*pbBuffer, pbRand;
	int		i, nLen;

	wcscpy(szPath, g_szServerDataFilePath);
	wcscat(szPath, L"Profile");
	wcscat(szPath, L"\\" SERVER_PROFILE2);

	// 자료암호화
	nLen = g_nAdminNum*sizeof(ADMININFO)+4;
	pbBuffer = new BYTE[nLen];
	ZeroMemory(pbBuffer, nLen);
	*(int *)pbBuffer = g_nAdminNum;
	if(g_nAdminNum)
		memcpy(pbBuffer+4, g_pAdminInfo, sizeof(ADMININFO)*g_nAdminNum);

	pbRand = 0x17;
	for(i = 0; i < nLen; i++)
	{
		pbBuffer[i] ^= pbRand;
		pbRand = pbBuffer[i];
	}
	fp = _wfopen(szPath, L"wb");
	if(!fp)
	{
		PrintLog(L"데이터베이스보관실패");
		delete [] pbBuffer;
		return;
	}
	fwrite(pbBuffer, 1, nLen, fp);
	fclose(fp);
	delete [] pbBuffer;
}

void	LoadAccountInfo()
{
	WCHAR	szPath[MAX_PATH];
	FILE	*fp;
	BYTE	*pbBuffer, pbRand, pbTemp;
	int		i, nLen;

	wcscpy(szPath, g_szServerDataFilePath);
	wcscat(szPath, L"Profile");
	CreateDirectory(szPath, NULL);

	wcscat(szPath, L"\\" SERVER_PROFILE);
	
	fp = _wfopen(szPath, L"rb");
	if(!fp)
	{
		//PrintLog(L"사용자계정데이터베이스열기 - 실패");
		return;
	}
	
	nLen = _filelength(_fileno(fp));
	pbBuffer = new BYTE[nLen];
	fread(pbBuffer, 1, nLen, fp);
	fclose(fp);

	// 자료해문
	for(i = 0; i < nLen; i++)
	{
		pbTemp = pbBuffer[i];
		if(i)
			pbBuffer[i] ^= pbRand;
		else
			pbBuffer[i] ^= 0x17;
		pbRand = pbTemp;
	}
	
	g_nAccountNum = *(int *)pbBuffer;
	if(g_pAccountInfo)
	{
		delete [] g_pAccountInfo;
		g_pAccountInfo = NULL;
	}
	g_pAccountInfo = new ACCOUNTINFO[g_nAccountNum];
	memcpy(g_pAccountInfo, pbBuffer+4, sizeof(ACCOUNTINFO)*g_nAccountNum);
	
	delete [] pbBuffer;
}

void LoadStateInfo()
{
	if(g_nAnglerStateNum != g_nAccountNum)
	{
		g_nAnglerStateNum = g_nAccountNum;
		if(g_pAnglerStateInfo != NULL)
			delete [] g_pAnglerStateInfo;
		g_pAnglerStateInfo = new STATEINFO[g_nAnglerStateNum];
	}

	//CreateAnglerStateDBTable();
	for(int i = 0; i < g_nAnglerStateNum; i++)
	{
		wcscpy(g_pAnglerStateInfo[i].szID, g_pAccountInfo[i].szAccount);
		//GetAnglerState(&g_pAnglerStateInfo[i]);
	}
}

void	LoadAdminInfo()
{
	WCHAR	szPath[MAX_PATH];
	FILE	*fp;
	BYTE	*pbBuffer, pbRand, pbTemp;
	int		i, nLen;

	wcscpy(szPath, g_szServerDataFilePath);
	wcscat(szPath, L"Profile");
	wcscat(szPath, L"\\" SERVER_PROFILE2);

	fp = _wfopen(szPath, L"rb");
	if(!fp)
	{
		//PrintLog(L"관리자계정데이터베이스열기 - 실패");
		return;
	}
	nLen = _filelength(_fileno(fp));
	pbBuffer = new BYTE[nLen];
	fread(pbBuffer, 1, nLen, fp);
	fclose(fp);

	// 자료해문
	for(i = 0; i < nLen; i++)
	{
		pbTemp = pbBuffer[i];
		if(i)
			pbBuffer[i] ^= pbRand;
		else
			pbBuffer[i] ^= 0x17;
		pbRand = pbTemp;
	}

	g_nAdminNum = *(int *)pbBuffer;
	if(g_pAdminInfo)
	{
		delete [] g_pAdminInfo;
		g_pAdminInfo = NULL;
	}
	g_pAdminInfo = new ADMININFO[g_nAdminNum];
	memcpy(g_pAdminInfo, pbBuffer+4, sizeof(ADMININFO)*g_nAdminNum);
	
	delete [] pbBuffer;
}


int GetHuntUserIndex(LPWSTR szID, LPWSTR szPwd)
{
	int i;
	for(i = 0; i < g_nAccountNum; i++)
	{					
		if(wcscmp(g_pAccountInfo[i].szAccount,szID) == 0 && wcscmp(g_pAccountInfo[i].szPassword,szPwd) == 0)
			break;
	}
	if(i < g_nAccountNum)
		return i;
	return -1;
}

BOOL CheckHuntUserExpiration(int nIndex)
{
	// 기한검사.
	DWORD		dwCur, dwExp, dwReg;
	SYSTEMTIME	time;

	GetLocalTime(&time);
	dwExp = *(short *)g_pAccountInfo[nIndex].pbExpDate;
	dwExp <<= 16;
	dwExp |= g_pAccountInfo[nIndex].pbExpDate[2]<<8;
	dwExp |= g_pAccountInfo[nIndex].pbExpDate[3];
	dwReg = *(short *)g_pAccountInfo[nIndex].pbRegDate;
	dwReg <<= 16;
	dwReg |= g_pAccountInfo[nIndex].pbRegDate[2]<<8;
	dwReg |= g_pAccountInfo[nIndex].pbRegDate[3];
	dwCur = (time.wYear<<16)|(time.wMonth<<8)|time.wDay;
	if(dwCur <= dwExp && dwCur >= dwReg)
	{
		return TRUE;
	}
	return FALSE;
}

int CheckHuntUserInfo(LPWSTR szID, LPWSTR szPwd, LPBYTE byKey) // -1: 인증실패, -2: 사용기한 만료, 0이상: 성공
{
	int nIndex = GetHuntUserIndex(szID, szPwd);
	if(nIndex == -1)
		return -1;
	if(!CheckHuntUserExpiration(nIndex))
		return -2;
	if(byKey == NULL)
		return nIndex;

	BYTE byZero[8] = {0,};
	if( memcmp(g_pAccountInfo[nIndex].pbAccount, byZero, 8) == 0 )
	{
		//새로 장치키등록
		memcpy(g_pAccountInfo[nIndex].pbAccount, byKey, 8);
		SaveAccountInfo();
		return nIndex;
	}
	if(memcmp(g_pAccountInfo[nIndex].pbAccount, byKey, 8) == 0)
		return nIndex;

	return -1;	
}

BOOL GetAdminInfo(LPWSTR szAdminID, LPWSTR szAdminPwd, ADMININFO *pAdminInfo)
{
	if(!wcscmp(szAdminID, ADMIN_TOP_ID) && !wcscmp(szAdminPwd, ADMIN_TOP_PWD))
	{
		wcscpy(pAdminInfo->szID, szAdminID);
		pAdminInfo->dwLevel = ADMINLEVEL_TOP;
		return TRUE;
	}

	for(int i = 0 ; i < g_nAdminNum ; i++)
	{
		if(!wcscmp(g_pAdminInfo[i].szID, szAdminID) && !wcscmp(g_pAdminInfo[i].szPwd, szAdminPwd))
		{
			memcpy(pAdminInfo, &g_pAdminInfo[i], sizeof(ADMININFO));
			return TRUE;
		}
	}

	return FALSE;
}

void BackUpAccountFile()
{
	WCHAR	szSrcPath[MAX_PATH], szDstPath[MAX_PATH];
	WCHAR	szSrcFile[MAX_PATH], szDstFile[MAX_PATH];

	wcscpy(szSrcPath, g_szServerDataFilePath);
	wcscat(szSrcPath, L"Profile\\");
	
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	WCHAR szDate[20];
	wsprintf(szDate, L"%d-%d-%d_", systime.wYear, systime.wMonth, systime.wDay);
	wcscpy(szDstPath, szSrcPath);
	wcscat(szDstPath, szDate);

	wcscpy(szSrcFile, szSrcPath);
	wcscat(szSrcFile, SERVER_PROFILE);
	wcscpy(szDstFile, szDstPath);
	wcscat(szDstFile, SERVER_PROFILE);
	CopyFile(szSrcFile, szDstFile, FALSE);

	wcscpy(szSrcFile, szSrcPath);
	wcscat(szSrcFile, SERVER_PROFILE2);
	wcscpy(szDstFile, szDstPath);
	wcscat(szDstFile, SERVER_PROFILE2);
	CopyFile(szSrcFile, szDstFile, FALSE);
}