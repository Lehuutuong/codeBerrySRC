#include "stdafx.h"
#include "Patch.h"
#include "ServerEngine.h"
#include "inifile.h"
#include "tstdlibs.h"
#include "io.h"

extern UINT GetMyPrivateProfileInt(LPTSTR lpSecName, LPTSTR lpKeyName, int nDefault, CIniFile* pInifile);

BOOL UpdateAutoPatchFile(BYTE *pbBuffer, int nLen)
{
	WCHAR	szPath[MAX_PATH];
	FILE	*fp;

	wcscpy(szPath, g_szServerDataFilePath);
	wcscat(szPath, g_szPatchFile);

	fp = _wfopen(szPath, L"wb");
	if(!fp)
	{
		return FALSE;
	}
	if(nLen != fwrite(pbBuffer, 1, nLen, fp))
	{
		fclose(fp);
		return FALSE;
	}
	fclose(fp);

	LoadAutoPatchFile();

	return TRUE;
}

void	LoadAutoPatchFile()
{
	WCHAR	szPath[MAX_PATH];
	FILE	*fp;

	wcscpy(szPath, g_szServerDataFilePath);
	wcscat(szPath, g_szPatchFile);

	fp = _wfopen(szPath, L"rb");
	if(!fp)
		return;

	int nLen = _filelength(fileno(fp));
	LPBYTE pbBuffer = new BYTE[nLen];
	fread(pbBuffer, nLen, 1, fp);
	fclose(fp);

	nLen = *(DWORD*)pbBuffer;

	//EncryptAutoSettingBuffer(pbBuffer + 4, nLen);

	char* pBuff = (CHAR*)(pbBuffer + 4);
	CA2W ca2w(pBuff);

	tstd::tstringstream ss1;
	ss1 << ca2w.m_psz;

	delete pbBuffer;

	CIniFile inifile;
	inifile.Load(ss1);
	g_dwVersion = (DWORD)GetMyPrivateProfileInt(L"Patch", L"Version", 0, &inifile);
}

