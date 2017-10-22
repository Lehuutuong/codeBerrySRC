#include "stdafx.h"
#include "AutoControl.h"
#include "Admin.h"

void EncryptPacketForAutoControl(BYTE *pbData, int nLen)
{
	int i;

	BYTE byTmp1 = 0, byTmp2 = 0;
	for(i = 0; i < nLen; i++)
	{
		byTmp2 = pbData[i];
		pbData[i] ^= (byTmp1 * 3);
		pbData[i] ^= (BYTE)(0x1C+(i&0xFF));
		byTmp1 = byTmp2;
	}
}

int MakePacketForAutoControl(BYTE *pbPacket, char *szFormat, ...)
{
	VMProtectBegin("MakePacketForAutoControl");
	va_list	arg;
	int		i, nLen, nOffset = 0;

	nLen = strlen(szFormat);
	va_start(arg, szFormat);
	for(i = 0; i < nLen; i++)
	{
		switch(szFormat[i])
		{
		case 'c':
			*(BYTE *)(pbPacket+nOffset) = va_arg(arg, char);
			nOffset += 1;
			break;
		case 'h':
			*(WORD *)(pbPacket+nOffset) = va_arg(arg, WORD);
			nOffset += 2;
			break;
		case 'd':
			*(DWORD *)(pbPacket+nOffset) = va_arg(arg, DWORD);
			nOffset += 4;
			break;
		case 'b':
			{
				BYTE	*pbTemp = va_arg(arg, BYTE *);
				int		n = va_arg(arg, int);
				if(n)
				{
					memcpy(pbPacket+nOffset, pbTemp, n);
					nOffset += n;
				}
			}
			break;
		case 'z':
			{
				int		n = va_arg(arg, int);
				memset(pbPacket+nOffset, 0, n);
				nOffset += n;
			}
			break;
		case 'S':
			{
				WCHAR	*szTemp = va_arg(arg, WCHAR *);
				swprintf((WCHAR *)(pbPacket+nOffset), L"%s", szTemp);
				nOffset += wcslen(szTemp)*2+2;
			}
			break;
		case 's':
			{
				char	*szTemp = va_arg(arg, char *);
				sprintf((char *)pbPacket+nOffset, "%s", szTemp);
				nOffset += 20/*strlen(szTemp)*/+1;
			}
			break;
		}
	}	
	va_end(arg);
	*(int *)(pbPacket+1) = nOffset-5;
	EncryptPacketForAutoControl(pbPacket+5, nOffset-5);
	VMProtectEnd();
	return nOffset;
}

void DwordToIPString(DWORD PackedIpAddress, LPSTR IpAddressBuffer)
{
	BYTE IpNumbers[4];

	IpNumbers[0] = (BYTE)((PackedIpAddress >> 24) & 0xFF);
	IpNumbers[1] = (BYTE)((PackedIpAddress >> 16) & 0xFF);
	IpNumbers[2] = (BYTE)((PackedIpAddress >> 8) & 0xFF);
	IpNumbers[3] = (BYTE)((PackedIpAddress >> 0) & 0xFF);
	sprintf(IpAddressBuffer, "%d.%d.%d.%d", IpNumbers[0], IpNumbers[1], IpNumbers[2], IpNumbers[3]);
}

DWORD WINAPI AutoControlCmdSendThread(LPVOID pParam)
{
	AUTOCONTROLCMD * pAutoControlCmd = (AUTOCONTROLCMD*)(pParam);
	BYTE TbbySendBuffer[200];
	int nLength = MakePacketForAutoControl(TbbySendBuffer, "cdSS", pAutoControlCmd->dwOpcode, 0, pAutoControlCmd->szAccount, pAutoControlCmd->szPassword);
	if(pAutoControlCmd->dwLocalIP)
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(sock == INVALID_SOCKET)
		{
		}
		else
		{
			SOCKADDR_IN		saddr;
			saddr.sin_family = AF_INET;
			char szIP[20];
			DwordToIPString(pAutoControlCmd->dwLocalIP, szIP);
			saddr.sin_addr.s_addr = inet_addr(szIP);
			saddr.sin_port = htons(SERVER_PORT_AUTOCONTROL);

			int		nTime = 3000;
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(char *)&nTime,4);

			if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)) == 0)
			{
				SendData(sock, (char*)TbbySendBuffer, nLength, 0);
				closesocket(sock);
				delete pAutoControlCmd;
				return 1;
			}
			closesocket(sock);
		}
	}
	if(pAutoControlCmd->dwInternetIP)
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(sock == INVALID_SOCKET)
		{
		}
		else
		{
			SOCKADDR_IN		saddr;
			saddr.sin_family = AF_INET;
			char szIP[20];
			DwordToIPString(pAutoControlCmd->dwInternetIP, szIP);
			saddr.sin_addr.s_addr = inet_addr(szIP);
			saddr.sin_port = htons(SERVER_PORT_AUTOCONTROL);

			int		nTime = 10000;
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(char *)&nTime,4);

			if(connect(sock, (SOCKADDR*)&saddr, sizeof(saddr)) == 0)
			{
				SendData(sock, (char*)TbbySendBuffer, nLength, 0);
				closesocket(sock);
				delete pAutoControlCmd;
				return 1;
			}
			closesocket(sock);
		}
	}
	delete pAutoControlCmd;
	return 0;
}