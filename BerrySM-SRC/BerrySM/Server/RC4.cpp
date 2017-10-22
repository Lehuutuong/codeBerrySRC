#include "stdafx.h"
#include "RC4.h"
#include "SHA1.h"

int		g_nRecvKeyLen = 0;
ULONGLONG	g_ullRecvPass = 0xA249FAB23B8E9173; // 16진표기 16자
BYTE	g_byRecvKey[20] = {0};

int		g_nSendKeyLen = 0;
ULONGLONG	g_ullSendPass = 0xBACBE544A11FC169; // 16진표기 16자
BYTE	g_bySendKey[20] = {0};

rc4_key g_randgen;
int g_randgenrunMode; // 1: play(continue), 2: pause, 0: stop 

#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t

void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
{
	unsigned char t;
	unsigned char index1;
	unsigned char index2;
	unsigned char* state;
	short counter;

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
}

DWORD WINAPI RandGenerator(LPVOID pParam)
{
	DWORD dwTick = GetTickCount();
	CHAR seed[100];
	sprintf(seed, "%d", dwTick);
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
	return 1;
}

void GetRandSeq(LPBYTE pSeq, int nLen)
{
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
}
void StopRandGeneraor()
{
	g_randgenrunMode = 0;
}
void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key)
{
	unsigned char t;
	unsigned char x;
	unsigned char y;
	unsigned char* state;
	unsigned char xorIndex;
	short counter;

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
}

void Initrc4SendCipher()
{
	if(g_nSendKeyLen == 0)
	{
		// 비번으로 부터 SHA1을 리용한 RC4키생성
		CXYZ sha1;
		sha1.Update((LPBYTE)&g_ullSendPass, sizeof(g_ullSendPass));
		sha1.Final();
		sha1.GetHash(g_bySendKey);
		g_nSendKeyLen = 20;
	}
}
void rc4Encrypt(LPBYTE pbybuf, int nbuflen)
{
	if(g_nSendKeyLen == 0)
	{
		Initrc4SendCipher();
	}
	rc4_key key;
	prepare_key(g_bySendKey, g_nSendKeyLen, &key);
	rc4(pbybuf, nbuflen, &key);
}

BOOL rc4EncryptStream(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("rc4Encrypt");
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
	if(g_nRecvKeyLen == 0)
	{
		// 비번으로 부터 SHA1을 리용한 RC4키생성
		CXYZ sha1;
		sha1.Update((LPBYTE)&g_ullRecvPass, sizeof(g_ullRecvPass));
		sha1.Final();
		sha1.GetHash(g_byRecvKey);
		g_nRecvKeyLen = 20;
	}
}
void rc4Decrypt(LPBYTE pbybuf, int nbuflen)
{
	if(g_nRecvKeyLen == 0)
	{
		Initrc4RecvCipher();
	}
	rc4_key key;
	prepare_key(g_byRecvKey, g_nRecvKeyLen, &key);
	rc4(pbybuf, nbuflen, &key);
}

BOOL rc4DecryptStream(LPBYTE pbybuf, int nbuflen)
{
	VMProtectBegin("rc4DecryptStream");
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

	return FALSE;

	VMProtectEnd();
}

void Initrc4Cipher()
{
	Initrc4RecvCipher();
	Initrc4SendCipher();
}