#include "stdafx.h"



LPSTR GetLineagePath(LPSTR lpPath, DWORD cbData)
{
	RtlZeroMemory(lpPath, cbData);

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\NC Soft\\Lineage", REG_OPTION_NON_VOLATILE, KEY_READ, &hKey) == ERROR_SUCCESS) 
	{
		if (RegQueryValueEx(hKey, "ExecutePath", NULL, NULL, (PBYTE)lpPath, &cbData) != ERROR_SUCCESS)
			RtlZeroMemory(lpPath, cbData);
		RegCloseKey(hKey);
	}
	return lpPath;
}

int GetServerIndex(LPCSTR lpGameServerName)
{
	for (int i = 0; i < MAX_GAME_SERVER; i ++)
	{
		if (strcmp(g_lpGameServerName[i], lpGameServerName) == 0)
			return i;
	}
	return -1;
}
