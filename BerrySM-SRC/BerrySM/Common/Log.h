#pragma once

#ifdef _TEST
#define InitLog			InitLogA
#define WriteLog		WriteLogA
#define WriteBinaryLog	WriteBinaryLogA
#define WriteDebugLog	WriteDebugLogA
#else
#define InitLog
#define WriteLog
#define WriteBinaryLog
#define WriteDebugLog
#endif // _TEST

void InitLogA(LPSTR lpFileName);
void WriteLogA(LPSTR lpFormat, ...);
void WriteBinaryLogA(LPBYTE lpBuffer, DWORD dwBufferLength);
void WriteDebugLogA(LPSTR lpFormat, ...);
VOID RebuildPE(LPSTR lpModuleName, LPSTR lpFileName);
