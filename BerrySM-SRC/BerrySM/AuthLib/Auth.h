#pragma once
class CAuth{
public:
	BOOL init();//���������ʱ�ȭ:�����߻��Ⱑ��
#ifdef AUTH_CLIENT
	BOOL GetBufferClientToServer(IN BYTE id[48],IN BYTE pass[48],IN BYTE deviceKey[16],IN int clientVersion,OUT BYTE buffer[132]);//exe���� server�� �۽��ϴ� ������Ŷ����
	BOOL GetInfoServerToClient(IN BYTE buffer[132],OUT BYTE opCode[96],OUT int *serverVersion);//server���� exe�� �۽��� �ڷ�κ��� �������
#endif

#ifdef AUTH_SERVER
	BOOL GetInfoClientToServer(IN BYTE buffer[132],OUT BYTE id[48],OUT BYTE pass[48],OUT BYTE deviceKey[16],OUT BYTE randBuf[16],OUT int *clientVersion);//exe���� server�� �۽��� �ڷ�κ��� �������
	BOOL GetBufferServerToClient(IN int clientVersion,IN BYTE randBuf[16],IN BYTE opCode[96],OUT BYTE buffer[132]);//server���� exe�� �۽��ϴ� ������Ŷ����
#endif

private:
	void Interleave(IN BYTE b0[16],IN BYTE b1[16],IN BYTE b2[16],IN BYTE b3[16],IN BYTE b4[16],IN BYTE b5[16],IN BYTE b6[16],IN BYTE b7[16],OUT BYTE interBuf[128]);//��Ʈ�� ���͸���
	void DeInterleave(IN BYTE interBuf[128],OUT BYTE b0[16],OUT BYTE b1[16],OUT BYTE b2[16],OUT BYTE b3[16],OUT BYTE b4[16],OUT BYTE b5[16],OUT BYTE b6[16],OUT BYTE b7[16]);//��Ʈ�� ���͸��� �ؼ�
	void GetRandBuffer(OUT BYTE randBuf[16]);//�����ľ��
	BYTE m_RandBuf[16];
};

typedef struct randseqgen_key
{      
	unsigned char state[256];       
	unsigned char x;        
	unsigned char y;
} randseqgen_key;