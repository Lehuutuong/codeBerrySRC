#include "stdafx.h"
#include "Auth.h"
#include "des.h"

#ifdef AUTH_SERVER
#define SERVER_VERSION 1

ULONGLONG m_clientKeys[SERVER_VERSION]={
	0x0C646261D2B8499D
};
ULONGLONG m_serverKeys[SERVER_VERSION]={
	0x5B5F54EAF817F831
};
#endif

#ifdef AUTH_CLIENT
ULONGLONG m_clientKey=0x0C646261D2B8499D;
ULONGLONG m_serverKey=0x5B5F54EAF817F831;
#endif

#ifdef AUTH_CLIENT

//exe에서 server로 송신하는 인증패킷구성
BOOL CAuth::GetBufferClientToServer(IN BYTE id[48],IN BYTE pass[48],IN BYTE deviceKey[16],IN int clientVersion,OUT BYTE buffer[132])
{
	VMProtectBegin("GetBufferClientToServer");
	VirtualizerStart();
	BYTE data[8];
	GetRandBuffer(m_RandBuf);
	Interleave(id,id+16,id+32,pass,pass+16,pass+32,deviceKey,m_RandBuf,buffer);
	for(int i=0;i<1;i++)
	{
		des_encrytion(*((BIT64*)&m_clientKey),*((BIT64*)(buffer+i*8)),(BIT64*)data);
		memcpy(buffer+i*8,data,8);
		//des_encrytion(*((BIT64*)&m_clientKey),*((BIT64*)(buffer+i*8)),(BIT64*)data);
		//memcpy(buffer+i*8,data,8);
		//des_encrytion(*((BIT64*)&m_clientKey),*((BIT64*)(buffer+i*8)),(BIT64*)data);
		//memcpy(buffer+i*8,data,8);
	}
	*((int*)(buffer+128))=clientVersion;
	VirtualizerEnd();
	VMProtectEnd();
	return TRUE;
}

//server에서 exe로 송신한 자료로부터 정보얻기
BOOL CAuth::GetInfoServerToClient(IN BYTE buffer[132],OUT BYTE opCode[96],OUT int *serverVersion)
{
	//VMProtectBegin("GetInfoServerToClient");
	VirtualizerStart();
	*serverVersion=*((int*)(buffer+128));
	BOOL rtn=TRUE;
	if(*serverVersion<=0)rtn=FALSE;
	if(rtn==TRUE){
		BYTE data[8];
		for(int i=0;i<16;i++)
		{
			des_decrytion(*((BIT64*)&m_serverKey),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			memcpy(buffer+i*8,data,8);
			//des_decrytion(*((BIT64*)&m_serverKey),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			//memcpy(buffer+i*8,data,8);
			//des_decrytion(*((BIT64*)&m_serverKey),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			//memcpy(buffer+i*8,data,8);
		}
		BYTE randBuf[16],randBuf1[16];
		DeInterleave(buffer,opCode,opCode+16,opCode+32,opCode+48,opCode+64,opCode+80,randBuf,randBuf1);
		if(memcmp(m_RandBuf,randBuf,16)!=0)
		{
			rtn=FALSE;
			memset(opCode,0,96);
		}
	}
	VirtualizerEnd();
	//VMProtectEnd();
	return rtn;

}
#endif

#ifdef AUTH_SERVER//exe에서 server로 송신한 자료로부터 정보얻기
BOOL CAuth::GetInfoClientToServer(IN BYTE buffer[132],OUT BYTE id[48],OUT BYTE pass[48],OUT BYTE deviceKey[16],OUT BYTE randBuf[16],OUT int *clientVersion)
{
	VMProtectBegin("GetInfoClientToServer");
	//	VirtualizerStart();
	int ver;
	*clientVersion=*((int*)(buffer+128));
	ver=(*clientVersion);
	BOOL rtn=TRUE;
	//	if(ver>SERVER_VERSION)rtn=FALSE;
	//	if(ver<=0)rtn=FALSE;
	if(rtn==TRUE){
		//		ver--;
		BYTE data[8];
		for(int i=0;i<16;i++)
		{
			des_decrytion(*((BIT64*)(&m_clientKeys[0])),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			memcpy(buffer+i*8,data,8);
			// 			des_decrytion(*((BIT64*)(&m_clientKeys[0])),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			// 			memcpy(buffer+i*8,data,8);
			//des_decrytion(*((BIT64*)(&m_clientKeys[ver])),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			//memcpy(buffer+i*8,data,8);
		}
		DeInterleave(buffer,id,id+16,id+32,pass,pass+16,pass+32,deviceKey,randBuf);
	}
	//	VirtualizerEnd();
	VMProtectEnd();
	return rtn;
}

//server에서 exe로 송신하는 인증패킷구성
BOOL CAuth::GetBufferServerToClient(IN int clientVersion,IN BYTE randBuf[16],IN BYTE opCode[96],OUT BYTE buffer[132])
{
	VMProtectBegin("GetBufferServerToClient");
	//	VirtualizerStart();
	BOOL rtn=TRUE;
	//	if(clientVersion>SERVER_VERSION)rtn=FALSE;
	//	if(clientVersion<=0)rtn=FALSE;
	if(rtn==TRUE){
		// 		if(clientVersion!=SERVER_VERSION)
		// 		{
		// 			memset(opCode,0,96);
		// 		}
		BYTE data[8];
		GetRandBuffer(m_RandBuf);
		Interleave(opCode,opCode+16,opCode+32,opCode+48,opCode+64,opCode+80,randBuf,m_RandBuf,buffer);
		for(int i=0;i<16;i++)
		{
			des_encrytion(*((BIT64*)(&m_serverKeys[0])),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			memcpy(buffer+i*8,data,8);
			// 			des_encrytion(*((BIT64*)(&m_serverKeys[clientVersion-1])),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			// 			memcpy(buffer+i*8,data,8);
			//des_encrytion(*((BIT64*)(&m_serverKeys[clientVersion-1])),*((BIT64*)(buffer+i*8)),(BIT64*)data);
			//memcpy(buffer+i*8,data,8);
		}
		*((int*)(buffer+128))=SERVER_VERSION;
	}
	//	VirtualizerEnd();
	VMProtectEnd();
	return rtn;

}
#endif

//인증엔진초기화:란수발생기가동
BOOL CAuth::init()
{
	return TRUE;
}

//비트렬 인터리브
void CAuth::Interleave(IN BYTE b0[16],IN BYTE b1[16],IN BYTE b2[16],IN BYTE b3[16],IN BYTE b4[16],IN BYTE b5[16],IN BYTE b6[16],IN BYTE b7[16],OUT BYTE interBuf[128])
{
	VMProtectBegin("Interleave");
	VirtualizerStart();
	for(int i=0;i<16;i++)
	{
		interBuf[i*8]=b0[i];
		interBuf[i*8+1]=b1[i];
		interBuf[i*8+2]=b2[i];
		interBuf[i*8+3]=b3[i];
		interBuf[i*8+4]=b4[i];
		interBuf[i*8+5]=b5[i];
		interBuf[i*8+6]=b6[i];
		interBuf[i*8+7]=b7[i];
	}
	VirtualizerEnd();
	VMProtectEnd();
}

//비트렬 인터리브 해석
void CAuth::DeInterleave(IN BYTE interBuf[128],OUT BYTE b0[16],OUT BYTE b1[16],OUT BYTE b2[16],OUT BYTE b3[16],OUT BYTE b4[16],OUT BYTE b5[16],OUT BYTE b6[16],OUT BYTE b7[16])
{
	VMProtectBegin("DeInterleave");
	VirtualizerStart();
	for(int i=0;i<16;i++)
	{
		b0[i]=interBuf[i*8];
		b1[i]=interBuf[i*8+1];
		b2[i]=interBuf[i*8+2];
		b3[i]=interBuf[i*8+3];
		b4[i]=interBuf[i*8+4];
		b5[i]=interBuf[i*8+5];
		b6[i]=interBuf[i*8+6];
		b7[i]=interBuf[i*8+7];
	}
	VirtualizerEnd();
	VMProtectEnd();
}

//란수렬얻기
void CAuth::GetRandBuffer(OUT BYTE randBuf[16])
{
#ifdef AUTH_SERVER
	srand(GetTickCount());
	*((int*)(randBuf))=rand();
	*((int*)(randBuf+4))=rand();
	*((int*)(randBuf+8))=rand();
	*((int*)(randBuf+12))=rand();
#endif

#ifdef AUTH_CLIENT
	GetRandSeq(randBuf, 16);
#endif

}

