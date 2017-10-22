#include "StdAfx.h"
#include "Sock.h"

CSock::CSock()
{
	m_sock = INVALID_SOCKET;
}

CSock::CSock(SOCKET sock)
{
	m_sock = sock;
}

CSock::~CSock()
{
	if (m_sock != INVALID_SOCKET)
	{
		if (shutdown(m_sock, SD_BOTH) != SOCKET_ERROR)
		{  
			fd_set  readfds;
			fd_set  errorfds;
			timeval timeout;

			FD_ZERO(&readfds);
			FD_ZERO(&errorfds);
			FD_SET(m_sock, &readfds);
			FD_SET(m_sock, &errorfds);

			timeout.tv_sec  = 10;//10s´ë±â
			timeout.tv_usec = 0;

			select(1, &readfds, NULL, &errorfds, &timeout);
		}
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
}

BOOL CSock::RecvData(SOCKET sock, char *buf, int len, int flags/* = 0*/)
{
	DWORD dwTick = GetTickCount();
	int nReadBytes = 0;
	while (nReadBytes < len)
	{
		int nRecv = recv(sock, buf + nReadBytes, len - nReadBytes, flags);
		if (nRecv <= 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK || GetTickCount() - dwTick > 30000)
				break;
		}
		else
		{
			nReadBytes += nRecv;
		}
		Sleep(1);
	}
	return (nReadBytes == len);
}

BOOL CSock::SendData(SOCKET sock, char *buf, int len, int flags/* = 0*/)
{
	DWORD dwTick = GetTickCount();
	int nWriteBytes = 0;
	while (nWriteBytes < len)
	{
		int nSend = send(sock, buf + nWriteBytes, len - nWriteBytes, flags);
		if (nSend <= 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK || GetTickCount() - dwTick > 30000)
				break;
		}
		else
		{
			nWriteBytes += nSend;
		}
		Sleep(1);
	}
	return (nWriteBytes == len);
}

BOOL CSock::RecvData(char *buf, int len, int flags/* = 0*/)
{
	return RecvData(m_sock, buf, len, flags);
}

BOOL CSock::SendData(char *buf, int len, int flags/* = 0*/)
{
	return SendData(m_sock, buf, len, flags);
}
