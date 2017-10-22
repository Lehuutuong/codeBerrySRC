typedef struct rc4_key
{      
	unsigned char state[256];       
	unsigned char x;        
	unsigned char y;
} rc4_key;

DWORD RandGenerator();
void GetRandSeq(LPBYTE pSeq, int nLen);
void StopRandGeneraor();

void Initrc4SendCipher();
void Initrc4RecvCipher();
void Initrc4Cipher();

void rc4Encrypt(LPBYTE pbybuf, int nbuflen);
void rc4Decrypt(LPBYTE pbybuf, int nbuflen);
BOOL rc4EncryptStream(LPBYTE pbybuf, int nbuflen);
BOOL rc4DecryptStream(LPBYTE pbybuf, int nbuflen);
BOOL rc4EncryptDriver(LPBYTE pbybuf, int nbuflen);
BOOL rc4DecryptDriver(LPBYTE pbybuf, int nbuflen);

