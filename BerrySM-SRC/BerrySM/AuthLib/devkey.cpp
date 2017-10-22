#include "stdafx.h"
#include <Wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "iphlpapi.lib")


ULONGLONG GetBaseBoardId()
{
	VMProtectBegin("GetBaseBoardId");
	VirtualizerStart();
	HRESULT hres;

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,             
		0, 
		CLSCTX_INPROC_SERVER, 
		IID_IWbemLocator, (LPVOID *) &pLoc);

	if (FAILED(hres))
	{
		return 0;                 // Program has failed.
	}


	IWbemServices *pSvc = NULL;

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), 
		NULL,                    
		NULL,                    
		0,                       
		NULL,                    
		0,                       
		0,                       
		&pSvc                    
		);

	if (FAILED(hres))
	{
		pLoc->Release();     
		return 0;                // Program has failed.
	}


	hres = CoSetProxyBlanket(
		pSvc,                        
		RPC_C_AUTHN_WINNT,           
		RPC_C_AUTHZ_NONE,            
		NULL,                        
		RPC_C_AUTHN_LEVEL_CALL,      
		RPC_C_IMP_LEVEL_IMPERSONATE, 
		NULL,                        
		EOAC_NONE                    
		);

	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();     
		return 0;               // Program has failed.
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"), 
		bstr_t("Select * from Win32_BaseBoard"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		return 0;               // Program has failed.
	}


	IWbemClassObject *pclsObj;
	ULONG uReturn = 0;

	BOOL bPS2 = FALSE;

	BYTE byId[16] = {0,};
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
			&pclsObj, &uReturn);

		if(0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		// Get the value of the Name property
		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);

		if(vtProp.vt == VT_BSTR && wcslen(vtProp.bstrVal))
		{
			int nLen = wcslen(vtProp.bstrVal);
			for(int i = 0; i < min(nLen, 16); i++)
			{
				byId[i] = (BYTE)vtProp.bstrVal[i];
			}
			VariantClear(&vtProp);
			break;
		}

		VariantClear(&vtProp);
	}

	ULONGLONG ullBaseBoardId;
	for(int i = 0; i < 16; i++)
	{	
		byId[i] ^= byId[15-i];
	}
	ullBaseBoardId = *(ULONGLONG*)byId;


	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	pclsObj->Release();

	VirtualizerEnd();
	VMProtectEnd();
	return ullBaseBoardId;
}

ULONGLONG GetCpuId()
{
	VMProtectBegin("GetCpuId");
	VirtualizerStart();

	HRESULT hres;

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,             
		0, 
		CLSCTX_INPROC_SERVER, 
		IID_IWbemLocator, (LPVOID *) &pLoc);

	if (FAILED(hres))
	{
		return 0;                 // Program has failed.
	}


	IWbemServices *pSvc = NULL;

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), 
		NULL,                    
		NULL,                    
		0,                       
		NULL,                    
		0,                       
		0,                       
		&pSvc                    
		);

	if (FAILED(hres))
	{
		pLoc->Release();     
		return 0;                // Program has failed.
	}


	hres = CoSetProxyBlanket(
		pSvc,                        
		RPC_C_AUTHN_WINNT,           
		RPC_C_AUTHZ_NONE,            
		NULL,                        
		RPC_C_AUTHN_LEVEL_CALL,      
		RPC_C_IMP_LEVEL_IMPERSONATE, 
		NULL,                        
		EOAC_NONE                    
		);

	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();     
		return 0;               // Program has failed.
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"), 
		bstr_t("Select * from Win32_Processor"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		return 0;               // Program has failed.
	}


	IWbemClassObject *pclsObj;
	ULONG uReturn = 0;

	BOOL bPS2 = FALSE;

	BYTE byId[16] = {0,};
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
			&pclsObj, &uReturn);

		if(0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		// Get the value of the Name property
		hr = pclsObj->Get(L"ProcessorId", 0, &vtProp, 0, 0);

		if(vtProp.vt == VT_BSTR && wcslen(vtProp.bstrVal))
		{
			int nLen = wcslen(vtProp.bstrVal);
			for(int i = 0; i < min(nLen, 16); i++)
			{
				byId[i] = (BYTE)vtProp.bstrVal[i];
			}
			VariantClear(&vtProp);
			break;
		}

		VariantClear(&vtProp);
	}

	ULONGLONG ullCpuId = 0;
	for(int i = 0; i < 16; i++)
	{	
		byId[i] ^= byId[15-i];
	}
	ullCpuId = *(ULONGLONG*)byId;


	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	pclsObj->Release();

	VirtualizerEnd();
	VMProtectEnd();
	return ullCpuId;
}

ULONGLONG GetBiosId()
{
	VMProtectBegin("GetBiosId");
	VirtualizerStart();

	HRESULT hres;

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,             
		0, 
		CLSCTX_INPROC_SERVER, 
		IID_IWbemLocator, (LPVOID *) &pLoc);

	if (FAILED(hres))
	{
		return 0;                 // Program has failed.
	}


	IWbemServices *pSvc = NULL;

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), 
		NULL,                    
		NULL,                    
		0,                       
		NULL,                    
		0,                       
		0,                       
		&pSvc                    
		);

	if (FAILED(hres))
	{
		pLoc->Release();     
		return 0;                // Program has failed.
	}


	hres = CoSetProxyBlanket(
		pSvc,                        
		RPC_C_AUTHN_WINNT,           
		RPC_C_AUTHZ_NONE,            
		NULL,                        
		RPC_C_AUTHN_LEVEL_CALL,      
		RPC_C_IMP_LEVEL_IMPERSONATE, 
		NULL,                        
		EOAC_NONE                    
		);

	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();     
		return 0;               // Program has failed.
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"), 
		bstr_t("Select * from Win32_Bios"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		return 0;               // Program has failed.
	}


	IWbemClassObject *pclsObj;
	ULONG uReturn = 0;

	BOOL bPS2 = FALSE;

	BYTE byId[16] = {0,};
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
			&pclsObj, &uReturn);

		if(0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		// Get the value of the Name property
		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);

		if(vtProp.vt == VT_BSTR && wcslen(vtProp.bstrVal))
		{
			int nLen = wcslen(vtProp.bstrVal);
			for(int i = 0; i < min(nLen, 16); i++)
			{
				byId[i] = (BYTE)vtProp.bstrVal[i];
			}
			VariantClear(&vtProp);
			break;
		}

		VariantClear(&vtProp);
	}

	ULONGLONG ullBiosId = 0;
	for(int i = 0; i < 16; i++)
	{	
		byId[i] ^= byId[15-i];
	}
	ullBiosId = *(ULONGLONG*)byId;


	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	pclsObj->Release();

	VirtualizerEnd();
	VMProtectEnd();
	return ullBiosId;
}

ULONGLONG GetMacAddr()
{
	VMProtectBegin("GetMacAddr");
	VirtualizerStart();

	PIP_ADAPTER_INFO		pInfo, pInfoTemp;
	ULONG					uSize;
	BYTE					pbPrimaryMac[6];
	int						nMin;

	ULONGLONG ullMacAddr = 0;
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	GetAdaptersInfo(NULL, &uSize);
	pInfo = (PIP_ADAPTER_INFO)malloc(uSize);
	if (GetAdaptersInfo(pInfo, &uSize))
		return ullMacAddr;

	nMin = INT_MAX;
	pInfoTemp = pInfo;
	while(pInfoTemp)
	{		
		if (nMin > (int)pInfoTemp->Index)
		{
			nMin = pInfoTemp->Index;
			memcpy(pbPrimaryMac, pInfoTemp->Address, 6);
		}
		pInfoTemp = pInfoTemp->Next;
	}

	LPBYTE pbKey = (LPBYTE)&ullMacAddr;
	pbKey[0] = pbPrimaryMac[3];
	pbKey[1] = pbPrimaryMac[2];
	pbKey[2] = pbPrimaryMac[2]^pbPrimaryMac[3];
	pbKey[3] = pbPrimaryMac[1];
	pbKey[4] = pbPrimaryMac[5]^pbPrimaryMac[4];
	pbKey[5] = pbPrimaryMac[5];
	pbKey[6] = pbPrimaryMac[4];
	pbKey[7] = pbPrimaryMac[0];

	*(DWORD*)pbKey ^= 0x67A5966C; 
	*(DWORD*)(pbKey+4) ^= 0xA54BDF76;

	CHAR Volume[MAX_PATH];
	GetSystemDirectoryA(Volume, MAX_PATH);
	strchr(Volume, '\\')[1] = 0;
	char VolumeName[MAX_PATH+1], FileSystemName[MAX_PATH+1];
	DWORD VolumeSerialNo = 0, MaxComponentLength, FileSystemFlags;
	GetVolumeInformationA(Volume, VolumeName, MAX_PATH, &VolumeSerialNo,
		&MaxComponentLength, &FileSystemFlags,
		FileSystemName, MAX_PATH);

	*(DWORD*)pbKey ^= VolumeSerialNo; 
	*(DWORD*)(pbKey+4) ^= VolumeSerialNo;

	VirtualizerEnd();
	VMProtectEnd();
	return ullMacAddr;
}

BOOL PrintKeyLog(WCHAR *szFormat, ...)
{
	FILE	*fp;
	char	*aszLog;
	WCHAR	szLog[1000];
	va_list	arg;

	va_start(arg, szFormat);
	vswprintf(szLog, szFormat, arg);
	va_end(arg);
	aszLog = new char[wcslen(szLog)*2+20];
	fp = _wfopen(L"C:\\keylog.txt", L"a+");
	if (!fp)
		return FALSE;
	WideCharToMultiByte(CP_ACP, 0, szLog, wcslen(szLog)+1, aszLog, wcslen(szLog)*2+2, 0, 0);
	strcat(aszLog, "\n");
	fprintf(fp, aszLog);
	fclose(fp);
	delete [] aszLog;
	return TRUE;
}

ULONGLONG GetDeviceKey()
{
	VMProtectBegin("GetDeviceKey");

	ULONGLONG ullCpuId = GetCpuId();
	ULONGLONG ullBaseBoardId = GetBaseBoardId();
	ULONGLONG ullBiosId = GetBiosId();
	ULONGLONG ullMacAddr = GetMacAddr();

	ULONGLONG ullDeviceToken = 0;
	ullDeviceToken ^= ullCpuId;
	ullDeviceToken ^= ullBaseBoardId;
	ullDeviceToken ^= ullBiosId;
	ullDeviceToken ^= ullMacAddr;

	WCHAR szKey1High[20] = {0};
	WCHAR szKey1Low[20] = {0};
	WCHAR szKey2High[20] = {0};
	WCHAR szKey2Low[20] = {0};
	WCHAR szKey3High[20] = {0};
	WCHAR szKey3Low[20] = {0};
	WCHAR szKey4High[20] = {0};
	WCHAR szKey4Low[20] = {0};
	WCHAR szKeyHigh[20] = {0};
	WCHAR szKeyLow[20] = {0};

	swprintf(szKey1High, L"%x", (DWORD)(ullCpuId >> 32));
	swprintf(szKey1Low, L"%x", (DWORD)(ullCpuId & 0xFFFFFFFF));
	swprintf(szKey2High, L"%x", (DWORD)(ullBaseBoardId >> 32));
	swprintf(szKey2Low, L"%x", (DWORD)(ullBaseBoardId & 0xFFFFFFFF));
	swprintf(szKey3High, L"%x", (DWORD)(ullBiosId >> 32));
	swprintf(szKey3Low, L"%x", (DWORD)(ullBiosId & 0xFFFFFFFF));
	swprintf(szKey4High, L"%x", (DWORD)(ullMacAddr >> 32));
	swprintf(szKey4Low, L"%x", (DWORD)(ullMacAddr & 0xFFFFFFFF));
	swprintf(szKeyHigh, L"%x", (DWORD)(ullMacAddr >> 32));
	swprintf(szKeyLow, L"%x", (DWORD)(ullMacAddr & 0xFFFFFFFF));

	PrintKeyLog(L"Key1: %s, %s", szKey1High, szKey1Low);
	PrintKeyLog(L"Key2: %s, %s", szKey2High, szKey2Low);
	PrintKeyLog(L"Key3: %s, %s", szKey3High, szKey3Low);
	PrintKeyLog(L"Key4: %s, %s", szKey4High, szKey4Low);
	PrintKeyLog(L"Key5: %s, %s", szKeyHigh, szKeyLow);

	VMProtectEnd();
	return ullDeviceToken;
}
