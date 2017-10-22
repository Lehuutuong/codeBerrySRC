#include "stdafx.h"

#ifdef _TEST
CHAR g_szLogFile[MAX_PATH];

CRITICAL_SECTION g_csLog;

void InitLogA(LPSTR lpFileName)
{
	sprintf_s(g_szLogFile, "%s\\%s", GetCurrentPath(), lpFileName);

	FILE *fp;
	fopen_s(&fp, g_szLogFile, "wb");
	if (fp)
	{
		fclose(fp);
	}

	InitializeCriticalSection(&g_csLog);
}

void WriteLogA(LPSTR lpFormat, ...)
{
	CHAR szLog[1011];
	va_list	arg;

	va_start(arg, lpFormat);
	_vsnprintf_s(szLog, 1000, lpFormat, arg);
	va_end(arg);
	strcat_s(szLog, "\r\n");

	EnterCriticalSection(&g_csLog);

	SYSTEMTIME stCurrent;
	GetLocalTime(&stCurrent);

	FILE *fp;
	fopen_s(&fp, g_szLogFile, "ab+");
	if (fp)
	{
		fprintf(fp, "%02d:%02d:%02d.%03d	[%d] %s", stCurrent.wHour, stCurrent.wMinute, stCurrent.wSecond, stCurrent.wMilliseconds, GetCurrentProcessId(), szLog);
		fclose(fp);
	}

	LeaveCriticalSection(&g_csLog);
}

void WriteBinaryLogA(LPBYTE lpBuffer, DWORD dwBufferLength)
{
	if (dwBufferLength == 0)
		return;

	CHAR *szLog = new CHAR[2 * dwBufferLength + 1];
	__try
	{
		for (DWORD i = 0; i < dwBufferLength; i ++)
		{
			wsprintfA(&szLog[2 * i], "%02X", lpBuffer[i]);
		}
		WriteLogA(szLog);
	}
	__except(1)
	{
	}
	delete szLog;
}

void WriteDebugLogA(LPSTR lpFormat, ...)
{
	CHAR szLog[1001];
	va_list	arg;

	va_start(arg, lpFormat);
	_vsnprintf_s(szLog, 1000, lpFormat, arg);
	va_end(arg);
	strcat_s(szLog, "\r\n");

	OutputDebugStringA(szLog);
}

VOID RebuildPE(LPSTR lpModuleName, LPSTR lpFileName)
{
	PIMAGE_NT_HEADERS pNtHeader;
	HMODULE hModule = GetModuleHandleA(lpModuleName);
	if(hModule)
	{
		CHAR szFileName[MAX_PATH];
		GetModuleFileNameA(hModule, szFileName, MAX_PATH - 1);

		pNtHeader = (PIMAGE_NT_HEADERS)((ULONG)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);
		DWORD ImageSize = pNtHeader->OptionalHeader.SizeOfImage;
		BYTE* Buffer = new BYTE [ImageSize];
		if(Buffer)
		{
			memcpy(Buffer, hModule, ImageSize);
			FILE *fp;
			fopen_s(&fp, lpFileName, "wb");
			if (fp)
			{
				pNtHeader = (PIMAGE_NT_HEADERS)((ULONG)Buffer + ((PIMAGE_DOS_HEADER)Buffer)->e_lfanew);
				pNtHeader->OptionalHeader.ImageBase = (DWORD)hModule;
				WORD i, NumberOfSections = pNtHeader->FileHeader.NumberOfSections;
				PIMAGE_SECTION_HEADER pSecHeader = IMAGE_FIRST_SECTION(pNtHeader);
				FILE *fp2;
				fopen_s(&fp2, szFileName, "rb");
				if (fp2)
				{
					int nFileSize = _filelength(_fileno(fp2));
					BYTE* Buffer2 = new BYTE [nFileSize];
					if (Buffer2)
					{
						fread(Buffer2, 1, nFileSize, fp2);
						PIMAGE_NT_HEADERS pNtHeader2 = (PIMAGE_NT_HEADERS)((ULONG)Buffer2 + ((PIMAGE_DOS_HEADER)Buffer2)->e_lfanew);
						pNtHeader->OptionalHeader.CheckSum = pNtHeader2->OptionalHeader.CheckSum;
						WriteLogA("FileName=%s ModuleName=%s BaseAddress=0x%x Checksum=0x%X", szFileName, lpModuleName, hModule, pNtHeader->OptionalHeader.CheckSum);

						PIMAGE_SECTION_HEADER pSecHeader2 = IMAGE_FIRST_SECTION(pNtHeader2);
						for(i = 0; i < NumberOfSections; i ++)
						{
							pSecHeader[i].VirtualAddress = pSecHeader2[i].VirtualAddress;
							pSecHeader[i].Misc.VirtualSize = pSecHeader2[i].Misc.VirtualSize;
						}
						delete Buffer2;
					}
					fclose(fp2);
				}
				for(i = 0; i < NumberOfSections; i ++)
				{
					pSecHeader[i].PointerToRawData = pSecHeader[i].VirtualAddress;
					pSecHeader[i].SizeOfRawData = pSecHeader[i].Misc.VirtualSize;
				}
				fwrite(Buffer, 1, ImageSize, fp);
				fclose(fp);
			}
			delete Buffer;
		}
	}
}
#endif