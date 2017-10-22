#pragma once
#include <sqltypes.h>

extern char g_szServerLogPath[MAX_PATH]; 

class CCrackCheckEngine
{
public:
	CCrackCheckEngine(void);
	~CCrackCheckEngine(void);

	CRITICAL_SECTION m_csMainLog; // ���ռ���
	CRITICAL_SECTION m_csStatusLog; // ������
	CRITICAL_SECTION m_csBlockLog; // ����
	CRITICAL_SECTION m_csUseTimeLog; // ���ð�
	CRITICAL_SECTION m_csGoldLog; // ���ӸӴ�
	CRITICAL_SECTION m_csIssueLog; // �߱޸���
	CRITICAL_SECTION m_csAbnormalLog; // ũ��üũ
	CRITICAL_SECTION m_csCrackLog;
	CRITICAL_SECTION m_csLog; // ���·α�

private:
	
	int GetDays(CTime start, CTime end);
	int GetDays(TIMESTAMP_STRUCT start, TIMESTAMP_STRUCT end);


	void AddAccountWriteFile(char* szName, int nDays, int nAccountInfo);
	void DeleteAccountWriteFile(char* szName, int nDays, int nAccountInfo);;
	void ExtendAccountWriteFile(char* szName, int nDays, int nAccountInfo);

public:
	// �ʱ�ȭ�Լ�
	BOOL Init(void);

	void WriteMainLogA(char* szIP, char* szName, BOOL bNormal);
	void WriteMainLogW(WCHAR* szIP, WCHAR* szName, BOOL bNormal);

	// ���Ӱ������� ��� �Լ�
	void ConnectAccountNumber(SYSTEMTIME LocalTime, int number, int number6, int number10);

	// ��ȿ�������� ��� �Լ�
	void ValidAccountNumber(SYSTEMTIME LocalTime, int number, int number6, int number10);

	// ��ϰ������� ��� �Լ�
	void BlockAccountNumber(SYSTEMTIME LocalTime, int number);

	// ������ӽð��� ��� �Լ� (�ð��� ������ ��)
	void AverageConnectTime(SYSTEMTIME LocalTime, int nMinute);

	// ���ݰ�差�� ��� �Լ�
	void PostGoldAmount(SYSTEMTIME LocalTime, ULONGLONG ullPostGold);

	void WriteBlockA(SYSTEMTIME LocalTime, char* szName, int nCount);
	void WriteBlockW(SYSTEMTIME LocalTime, WCHAR* szName, int nCount);
	void AddBlockA(SYSTEMTIME LocalTime, CHAR* szName, int nCount);
	void AddBlockW(SYSTEMTIME LocalTime, WCHAR* szName, int nCount);

	// ���ð�
	void WriteUseTimeA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount);
	void WriteUseTimeW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount);

	void AddUseTimeA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount);
	void AddUseTimeW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount);

	// ���ӸӴ�
	void WriteGoldA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount);
	void WriteGoldW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount);

	void AddGoldA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount);
	void AddGoldW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount);

	// �߰������α׸� ��� �Լ�
	void AddAccountA(char* szName, CTime startDate, CTime endDate, int nAccountInfo = 0);
	void AddAccountW(WCHAR* szName, CTime startDate, CTime endDate, int nAccountInfo = 0);

	// ���������α׸� ��� �Լ�
	void DeleteAccountA(char* szName, CTime startDate, CTime endDate, int nAccountInfo = 0);
	void DeleteAccountW(WCHAR* szName, CTime startDate, CTime endDate, int nAccountInfo = 0);

	// ��������α׸� ��� �Լ�
	void ExtendAccountA(char* szName, CTime prevEndDate, CTime endDate, int nAccountInfo = 0);
	void ExtendAccountW(WCHAR* szName, CTime prevEndDate, CTime endDate, int nAccountInfo = 0);

	// ũ��üũ
	void __cdecl WriteCrackLog(int nKeyType, PCHAR FormatString,...); // �α���� �Լ�

	// ��Ŷ���ÿ���(��Ŷ����)�� ���Ͽ� ������ �α׸� ��´�.
	void WriteLogForPacketErrorA(char* szIp); //
	void WriteLogForPacketErrorW(WCHAR* szIp);

	// ���̵� Ʋ�� �α����
	void WriteLogForIdErrorA(char* szAccount, char* szIp);
	void WriteLogForIdErrorW(WCHAR* szAccount, WCHAR* szIp);

	// ��� Ʋ�� �α����
	void WriteLogForPwdErrorA(char* szAccount, char* szPwd, char* szIp);
	void WriteLogForPwdErrorW(WCHAR* szAccount, WCHAR* szPwd, WCHAR* szIp);

	// �α���ó�������� DB������ �����α����
	void WriteLogForLoginDBErrorA(char*szIp);
	void WriteLogForLoginDBErrorW(WCHAR*szIp);

	// ��ġŰ�� ���� �������ӷα����
	void WriteLogForMultiDeviceKeyA(char* szAccout, char* szIp, BOOL bLogin);
	void WriteLogForMultiDeviceKeyW(WCHAR* szAccout, WCHAR* szIp, BOOL bLogin);

	// �����ǿ� ���� �������� �α����
	void WriteLogForMultiIPA(char* szAccout, char* szIp, BOOL bLogin);
	void WriteLogForMultiIPW(WCHAR* szAccout, WCHAR* szIp, BOOL bLogin);

	// �Ϸ� �α���ȸ���� �ʰ��� �����α׸� ��� �Լ�
	void AbnormalLoginTryCountA(char* szName, int nCount);
	void AbnormalLoginTryCountW(WCHAR* szName, int nCount);

	// �Ϸ� 24�ð��̻� ����� ����� �ִ� �����α׸� ��� �Լ�
	void AbnormalUseTimeA(char* szName, int nCount);
	void AbnormalUseTimeW(WCHAR* szName, int nCount);

	// ���·α�
	void __cdecl CCrackCheckEngine:: WriteStatusLog(PCHAR FormatString,...);
};

