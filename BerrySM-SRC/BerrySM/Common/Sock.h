#pragma once

class CSock : public CObject
{
public:
	CSock();
	CSock(SOCKET sock);
	~CSock();

	BOOL RecvData(SOCKET sock, char *buf, int len, int flags = 0);
	BOOL SendData(SOCKET sock, char *buf, int len, int flags = 0);
protected:
	SOCKET	m_sock;
public:
	BOOL	RecvData(char *buf, int len, int flags = 0);
	BOOL	SendData(char *buf, int len, int flags = 0);
};
