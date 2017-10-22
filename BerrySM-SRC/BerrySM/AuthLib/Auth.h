#pragma once
class CAuth{
public:
	BOOL init();//인증엔진초기화:란수발생기가동
#ifdef AUTH_CLIENT
	BOOL GetBufferClientToServer(IN BYTE id[48],IN BYTE pass[48],IN BYTE deviceKey[16],IN int clientVersion,OUT BYTE buffer[132]);//exe에서 server로 송신하는 인증패킷구성
	BOOL GetInfoServerToClient(IN BYTE buffer[132],OUT BYTE opCode[96],OUT int *serverVersion);//server에서 exe로 송신한 자료로부터 정보얻기
#endif

#ifdef AUTH_SERVER
	BOOL GetInfoClientToServer(IN BYTE buffer[132],OUT BYTE id[48],OUT BYTE pass[48],OUT BYTE deviceKey[16],OUT BYTE randBuf[16],OUT int *clientVersion);//exe에서 server로 송신한 자료로부터 정보얻기
	BOOL GetBufferServerToClient(IN int clientVersion,IN BYTE randBuf[16],IN BYTE opCode[96],OUT BYTE buffer[132]);//server에서 exe로 송신하는 인증패킷구성
#endif

private:
	void Interleave(IN BYTE b0[16],IN BYTE b1[16],IN BYTE b2[16],IN BYTE b3[16],IN BYTE b4[16],IN BYTE b5[16],IN BYTE b6[16],IN BYTE b7[16],OUT BYTE interBuf[128]);//비트렬 인터리브
	void DeInterleave(IN BYTE interBuf[128],OUT BYTE b0[16],OUT BYTE b1[16],OUT BYTE b2[16],OUT BYTE b3[16],OUT BYTE b4[16],OUT BYTE b5[16],OUT BYTE b6[16],OUT BYTE b7[16]);//비트렬 인터리브 해석
	void GetRandBuffer(OUT BYTE randBuf[16]);//란수렬얻기
	BYTE m_RandBuf[16];
};

typedef struct randseqgen_key
{      
	unsigned char state[256];       
	unsigned char x;        
	unsigned char y;
} randseqgen_key;