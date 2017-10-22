#include "stdafx.h"
#include "RC4.h"
#include "SHA1.h"

int		g_nRecvKeyLen = 0;
BYTE	g_byRecvKey[20] = {0};

int		g_nSendKeyLen = 0;
BYTE	g_bySendKey[20] = {0};

rc4_key g_randgen;
int g_randgenrunMode; // 1: play(continue), 2: pause, 0: stop 

#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t

BOOL PrintRCDbgLog(WCHAR *szFormat, ...)
{
	char	*aszLog;
	WCHAR	szLog[1000];
	va_list	arg;

	va_start(arg, szFormat);
	vswprintf(szLog, szFormat, arg);
	va_end(arg);
	aszLog = new char[wcslen(szLog)*2+20];
	WideCharToMultiByte(CP_ACP, 0, szLog, wcslen(szLog)+1, aszLog, wcslen(szLog)*2+2, 0, 0);
	strcat(aszLog, "\n");
	OutputDebugStringA(aszLog);
	delete [] aszLog;
	return TRUE;
}

void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
{
	VMProtectBegin("Protect 0-01");
	unsigned char t;
	unsigned char index1;
	unsigned char index2;
	unsigned char* state;
	int counter;

	state = &key->state[0];
	for(counter = 0; counter < 256; counter++)
		state[counter] = counter;
	key->x = 0;
	key->y = 0;
	index1 = 0;
	index2 = 0;
	for(counter = 0; counter < 256; counter++)
	{
		index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
		swap_byte(&state[counter], &state[index2]);
		index1 = (index1 + 1) % key_data_len;
	}
	VMProtectEnd();
}

DWORD RandGenerator()
{
	VMProtectBegin("Protect 0-02");
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	DWORD dwTick = GetTickCount();
	CHAR seed[100];
	sprintf(seed, "%d-%d-%d %d:%d:%d:%d", systime.wYear, systime.wMonth, systime.wDay, 
		systime.wHour, systime.wMinute, systime.wSecond, dwTick);

	prepare_key((LPBYTE)seed, strlen(seed), &g_randgen);
	unsigned char t;
	unsigned char x;
	unsigned char y;
	unsigned char* state;
	unsigned char xorIndex;

	state = &g_randgen.state[0];
	g_randgenrunMode = 1;
	while(g_randgenrunMode == 1 || g_randgenrunMode == 2)
	{
		Sleep(100);
		x = g_randgen.x;
		y = g_randgen.y;
		if(g_randgenrunMode == 2)
		{
			continue;
		}
		x = (x + 1) % 256;
		y = (state[x] + y) % 256;
		swap_byte(&state[x], &state[y]);
		xorIndex = (state[x] + state[y]) % 256;
		g_randgen.x = x;
		g_randgen.y = y;
	}
	VMProtectEnd();
	return 1;
}

void GetRandSeq(LPBYTE pSeq, int nLen)
{
	VMProtectBegin("Protect 0-03");
	g_randgenrunMode = 2;
	Sleep(200);
	unsigned char t;
	unsigned char x;
	unsigned char y;
	unsigned char* state;
	unsigned char xorIndex;
	x = g_randgen.x;
	y = g_randgen.y;
	state = &g_randgen.state[0];
	for(int i = 0; i < nLen; i ++)
	{
		x = (x + 1) % 256;
		y = (state[x] + y) % 256;
		swap_byte(&state[x], &state[y]);
		xorIndex = (state[x] + state[y]) % 256;
		pSeq[i] ^= state[xorIndex];
	}
	g_randgen.x = x;
	g_randgen.y = y;
	g_randgenrunMode = 1;
	VMProtectEnd();
}
void StopRandGeneraor()
{
	g_randgenrunMode = 0;
}
void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key)
{
	VMProtectBegin("Protect 0-04");
	unsigned char t;
	unsigned char x;
	unsigned char y;
	unsigned char* state;
	unsigned char xorIndex;
	int counter;

	x = key->x;
	y = key->y;
	state = &key->state[0];
	for(counter = 0; counter < buffer_len; counter++)
	{
		x = (x + 1) % 256;
		y = (state[x] + y) % 256;
		swap_byte(&state[x], &state[y]);
		xorIndex = (state[x] + state[y]) % 256;
		buffer_ptr[counter] ^= state[xorIndex];
	}
	key->x = x;
	key->y = y;
	VMProtectEnd();
}

void Initrc4SendCipher()
{
	VMProtectBegin("Protect 0-05");
	if(g_nSendKeyLen == 0)
	{
		// 비번으로 부터 SHA1을 리용한 RC4키생성
		LPWSTR pszKey = VMProtectDecryptStringW(L"802F920671E29950");
		LPWSTR pszKeyStop;
		ULONGLONG kull = _wcstoui64(pszKey, &pszKeyStop, 16);
		CXYZ sha1;
		sha1.Update((LPBYTE)&kull, sizeof(kull));
		sha1.Final();
		sha1.GetHash(g_bySendKey);
		g_nSendKeyLen = 20;
	}
	VMProtectEnd();
}
void rc4Encrypt(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("Protect 0-06");
	if(g_nSendKeyLen == 0)
	{
		Initrc4SendCipher();
	}
	rc4_key key;
	prepare_key(g_bySendKey, g_nSendKeyLen, &key);
	rc4(pbybuf, nbuflen, &key);
	VMProtectEnd();
}
BOOL rc4EncryptStream(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("Protect 0-07");
	if(g_nSendKeyLen == 0)
	{
		Initrc4SendCipher();
	}
	if(nbuflen < 4)
	{
		return FALSE;
	}
	rc4_key key;
	prepare_key(g_bySendKey, g_nSendKeyLen, &key);

	//무결성추가
	DWORD dwCrc = 0;
	for(int i = 0; i < nbuflen - 4; i ++)
	{
		int j = i & 0x3;
		DWORD dw = pbybuf[i];
		dw <<= j * 8;
		dwCrc ^= dw;
		dwCrc *= 16777619;
	}
	*(DWORD*)(pbybuf + nbuflen - 4) = dwCrc;

	//란수씌우기
	for(int i = 8; i < nbuflen; i ++)
	{
		int j = i & 0x7;
		pbybuf[i] ^= pbybuf[j];
	}
	rc4(pbybuf, nbuflen, &key);

	VMProtectEnd();
	return TRUE;
}
void Initrc4RecvCipher()
{
	VMProtectBegin("Protect 0-08");
	if(g_nRecvKeyLen == 0)
	{
		// 비번으로 부터 SHA1을 리용한 RC4키생성
		LPWSTR pszKey = VMProtectDecryptStringW(L"E26EA83E7B2E445B");
		LPWSTR pszKeyStop;
		ULONGLONG kull = _wcstoui64(pszKey, &pszKeyStop, 16);
		CXYZ sha1;
		sha1.Update((LPBYTE)&kull, sizeof(kull));
		sha1.Final();
		sha1.GetHash(g_byRecvKey);
		g_nRecvKeyLen = 20;
	}
	VMProtectEnd();
}
void rc4Decrypt(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("Protect 0-09");
	if(g_nRecvKeyLen == 0)
	{
		Initrc4RecvCipher();
	}
	rc4_key key;
	prepare_key(g_byRecvKey, g_nRecvKeyLen, &key);
	rc4(pbybuf, nbuflen, &key);
	VMProtectEnd();
}

BOOL rc4DecryptStream(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("Protect 0-10");
	if(g_nRecvKeyLen == 0)
	{
		Initrc4RecvCipher();
	}

	if(nbuflen < 4)
	{
		return FALSE;
	}

	rc4_key key;
	prepare_key(g_byRecvKey, g_nRecvKeyLen, &key);
	rc4(pbybuf, nbuflen, &key);

	//란수벗기기
	for(int i = 8; i < nbuflen; i ++)
	{
		int j = i & 0x7;
		pbybuf[i] ^= pbybuf[j];
	}

	//무결성검사
	DWORD dwCrc = 0;
	for(int i = 0; i < nbuflen - 4; i ++)
	{
		int j = i & 0x3;
		DWORD dw = pbybuf[i];
		dw <<= j * 8;
		dwCrc ^= dw;
		dwCrc *= 16777619;
	}
	if(*(DWORD*)(pbybuf + nbuflen - 4) == dwCrc)
	{
		return TRUE;
	}

	VMProtectEnd();
	return FALSE;
}

void Initrc4Cipher()
{
	Initrc4RecvCipher();
	Initrc4SendCipher();
}

BOOL rc4EncryptDriver(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("Protect 0-11");
	rc4_key key;
	BYTE	initKey[20] = {
		0x21, 0xAE, 0xC9, 0xD0, 0xE4, 0x68, 0xD0, 0xB5, 0xE9, 0xB8,
		0x36, 0x43, 0x93, 0x50, 0xCB, 0x68, 0x34, 0xC3, 0x77, 0x3C
	};

	if(nbuflen < 4)
	{
		return FALSE;
	}

	prepare_key(initKey, 20, &key);

	//무결성추가
	DWORD dwCrc = 0;
	for(int i = 0; i < nbuflen - 4; i ++)
	{
		int j = i & 0x3;
		DWORD dw = pbybuf[i];
		dw <<= j * 8;
		dwCrc ^= dw;
		dwCrc *= 7;
	}
	*(DWORD*)(pbybuf + nbuflen - 4) = dwCrc;

	//PrintRCDbgLog(L"rc4EncryptDriver: crc = %x", dwCrc);

	//란수씌우기
	for(int i = 8; i < nbuflen; i ++)
	{
		int j = i & 0x7;
		pbybuf[i] ^= pbybuf[j];
	}

	rc4(pbybuf, nbuflen, &key);

	VMProtectEnd();
	return TRUE;
}

BOOL rc4DecryptDriver(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("Protect 0-12");

	rc4_key key;
	BYTE	initKey[20] = {
		0x89, 0xCF, 0x95, 0x30, 0xA6, 0x8E, 0x0A, 0x31, 0x32, 0x59, 
		0x62, 0x22, 0xD4, 0xD5, 0x13, 0x69, 0x0B, 0x45, 0x66, 0x26
	};

	if(nbuflen < 4)
	{
		return FALSE;
	}

	prepare_key(initKey, 20, &key);
	rc4(pbybuf, nbuflen, &key);

	//란수벗기기
	for(int i = 8; i < nbuflen; i ++)
	{
		int j = i & 0x7;
		pbybuf[i] ^= pbybuf[j];
	}

	//무결성검사
	DWORD dwCrc = 0;
	for(int i = 0; i < nbuflen - 4; i ++)
	{
		int j = i & 0x3;
		DWORD dw = pbybuf[i];
		dw <<= j * 8;
		dwCrc ^= dw;
		dwCrc *= 7;
	}
	//PrintRCDbgLog(L"rc4DecryptDriver: crc = %x, %x", dwCrc, *(DWORD*)(pbybuf + nbuflen - 4));
	if(*(DWORD*)(pbybuf + nbuflen - 4) == dwCrc)
	{
		return TRUE;
	}

	VMProtectEnd();
	return FALSE;
}