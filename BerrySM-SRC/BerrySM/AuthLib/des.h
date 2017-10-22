#pragma once
/*
typedef unsigned long   DWORD;
typedef unsigned short  WORD;
typedef unsigned char   BYTE;
*/
typedef	union	{
        DWORD   dw[2];
        WORD    w[4];
        BYTE	b[8];
} BIT64;
void des_encrytion(BIT64 key, BIT64 plain, BIT64 *out);
void des_decrytion(BIT64 key, BIT64 plain, BIT64 *out);
