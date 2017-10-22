#pragma once
#include <sqltypes.h>

extern char g_szServerLogPath[MAX_PATH]; 

class CCrackCheckEngine
{
public:
	CCrackCheckEngine(void);
	~CCrackCheckEngine(void);

	CRITICAL_SECTION m_csMainLog; // 통합서버
	CRITICAL_SECTION m_csStatusLog; // 사용상태
	CRITICAL_SECTION m_csBlockLog; // 정지
	CRITICAL_SECTION m_csUseTimeLog; // 사용시간
	CRITICAL_SECTION m_csGoldLog; // 게임머니
	CRITICAL_SECTION m_csIssueLog; // 발급리력
	CRITICAL_SECTION m_csAbnormalLog; // 크랙체크
	CRITICAL_SECTION m_csCrackLog;
	CRITICAL_SECTION m_csLog; // 상태로그

private:
	
	int GetDays(CTime start, CTime end);
	int GetDays(TIMESTAMP_STRUCT start, TIMESTAMP_STRUCT end);


	void AddAccountWriteFile(char* szName, int nDays, int nAccountInfo);
	void DeleteAccountWriteFile(char* szName, int nDays, int nAccountInfo);;
	void ExtendAccountWriteFile(char* szName, int nDays, int nAccountInfo);

public:
	// 초기화함수
	BOOL Init(void);

	void WriteMainLogA(char* szIP, char* szName, BOOL bNormal);
	void WriteMainLogW(WCHAR* szIP, WCHAR* szName, BOOL bNormal);

	// 접속계정수를 찍는 함수
	void ConnectAccountNumber(SYSTEMTIME LocalTime, int number, int number6, int number10);

	// 유효계정수를 찍는 함수
	void ValidAccountNumber(SYSTEMTIME LocalTime, int number, int number6, int number10);

	// 블록계정수를 찍는 함수
	void BlockAccountNumber(SYSTEMTIME LocalTime, int number);

	// 평균접속시간을 찍는 함수 (시간의 단위는 분)
	void AverageConnectTime(SYSTEMTIME LocalTime, int nMinute);

	// 수금골드량을 찍는 함수
	void PostGoldAmount(SYSTEMTIME LocalTime, ULONGLONG ullPostGold);

	void WriteBlockA(SYSTEMTIME LocalTime, char* szName, int nCount);
	void WriteBlockW(SYSTEMTIME LocalTime, WCHAR* szName, int nCount);
	void AddBlockA(SYSTEMTIME LocalTime, CHAR* szName, int nCount);
	void AddBlockW(SYSTEMTIME LocalTime, WCHAR* szName, int nCount);

	// 사용시간
	void WriteUseTimeA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount);
	void WriteUseTimeW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount);

	void AddUseTimeA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount);
	void AddUseTimeW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount);

	// 게임머니
	void WriteGoldA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount);
	void WriteGoldW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount);

	void AddGoldA(SYSTEMTIME LocalTime, char* szName, int nMulti, int nCount);
	void AddGoldW(SYSTEMTIME LocalTime, WCHAR* szName, int nMulti, int nCount);

	// 추가계정로그를 찍는 함수
	void AddAccountA(char* szName, CTime startDate, CTime endDate, int nAccountInfo = 0);
	void AddAccountW(WCHAR* szName, CTime startDate, CTime endDate, int nAccountInfo = 0);

	// 삭제계정로그를 찍는 함수
	void DeleteAccountA(char* szName, CTime startDate, CTime endDate, int nAccountInfo = 0);
	void DeleteAccountW(WCHAR* szName, CTime startDate, CTime endDate, int nAccountInfo = 0);

	// 연장계정로그를 찍는 함수
	void ExtendAccountA(char* szName, CTime prevEndDate, CTime endDate, int nAccountInfo = 0);
	void ExtendAccountW(WCHAR* szName, CTime prevEndDate, CTime endDate, int nAccountInfo = 0);

	// 크랙체크
	void __cdecl WriteCrackLog(int nKeyType, PCHAR FormatString,...); // 로그찍는 함수

	// 패킷관련오유(패킷변조)에 대하여 아이피 로그를 찍는다.
	void WriteLogForPacketErrorA(char* szIp); //
	void WriteLogForPacketErrorW(WCHAR* szIp);

	// 아이디 틀림 로그찍기
	void WriteLogForIdErrorA(char* szAccount, char* szIp);
	void WriteLogForIdErrorW(WCHAR* szAccount, WCHAR* szIp);

	// 비번 틀림 로그찍기
	void WriteLogForPwdErrorA(char* szAccount, char* szPwd, char* szIp);
	void WriteLogForPwdErrorW(WCHAR* szAccount, WCHAR* szPwd, WCHAR* szIp);

	// 로그인처리과정에 DB에서의 오류로그찍기
	void WriteLogForLoginDBErrorA(char*szIp);
	void WriteLogForLoginDBErrorW(WCHAR*szIp);

	// 장치키에 의한 다중접속로그찍기
	void WriteLogForMultiDeviceKeyA(char* szAccout, char* szIp, BOOL bLogin);
	void WriteLogForMultiDeviceKeyW(WCHAR* szAccout, WCHAR* szIp, BOOL bLogin);

	// 아이피에 의한 다중접속 로그찍기
	void WriteLogForMultiIPA(char* szAccout, char* szIp, BOOL bLogin);
	void WriteLogForMultiIPW(WCHAR* szAccout, WCHAR* szIp, BOOL bLogin);

	// 하루 로그인회수를 초과한 계정로그를 찍는 함수
	void AbnormalLoginTryCountA(char* szName, int nCount);
	void AbnormalLoginTryCountW(WCHAR* szName, int nCount);

	// 하루 24시간이상 사용한 기록이 있는 계정로그를 찍는 함수
	void AbnormalUseTimeA(char* szName, int nCount);
	void AbnormalUseTimeW(WCHAR* szName, int nCount);

	// 상태로그
	void __cdecl CCrackCheckEngine:: WriteStatusLog(PCHAR FormatString,...);
};

