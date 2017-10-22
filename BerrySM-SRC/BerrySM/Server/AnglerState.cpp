#include "stdafx.h"
#include "AnglerState.h"
#include "ServerEngine.h"
#include "Account.h"
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
extern	SQLHDBC g_sql_hDBC;
SYSTEMTIME	g_SysTime;
WCHAR		g_szAnglerStateTableName[100];

extern BOOL ERRORCHECK(SQLHANDLE hSql, SQLSMALLINT sqltype);

WCHAR	g_wszState[ANGLER_NONE + 1][100] = {
	L"접속중",
	L"잔여사용시간",
	L"아덴(%d)찾기완료",
	L"미끼구입오류",
	L"미끼구입완료",
	L"낚시터입장",
	L"낚시위치차지오류",
	L"낚시시작",
	L"낚시중(미끼 %d개)",
	L"낚시완료",
	L"아덴(%d)맡기기완료",
	L"접속해제",
};

WCHAR g_wszRestTimePattern[] = L"잔여:%d분,예약:%d개_%s";
void WideToMulti(WCHAR *wszBuf, char *szBuf)
{
	for (int i = 0; i < wcslen(wszBuf); i ++)
	{
		WCHAR wszTemp = wszBuf[i];
		szBuf[i] = (char)(wszTemp);
	}
}

void MultiToWide(char *szBuf, WCHAR *wszBuf)
{
	for (int i = 0; i < strlen(szBuf); i ++)
	{
		WCHAR wszTemp = 0;
		wszTemp = szBuf[i];
		wszBuf[i] = wszTemp;
	}
}

int GetAnglerHistoryIndex(PANGLERSTATE pOrimState)
{
	if(pOrimState->byType == ANGLER_CONNECT)
		return -1;

	CreateAnglerStateDBTable();

	int nRet = 0;
	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	int nOrimStateId = -1;
	int nFinished = -1;
	WCHAR szPrevAccountState[100] = {0};
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetOrimStatePtn[] = _T("SELECT `id` FROM `%s` WHERE `account` = '%s' AND `gameid` = '%s' AND `server` = '%s' AND `charnum` = '%d'  ORDER BY `id` DESC LIMIT 1");
		SQLTCHAR SqlGetOrimState[600] = {0,};

		TCHAR wszGameId[40] = {0,};	
		int nServerIndex = (pOrimState->byServer)%(MAX_GAME_SERVER + 1);
		MultiToWide(pOrimState->szGameID, wszGameId);
		_stprintf(SqlGetOrimState, SqlGetOrimStatePtn, g_szAnglerStateTableName, pOrimState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pOrimState->byCharNum);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetOrimState,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nOrimStateId),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nOrimStateId != -1)
			{
			}
		}
		else
		{
			if(ERRORCHECK(sql_hStmt, SQL_HANDLE_STMT))
			{
				SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );
				return GetAnglerHistoryIndex(pOrimState);
			}
		}
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );

	}

	if (nOrimStateId == -1)
	{
		nRet = -1;
	}
	else
	{
		nRet = nOrimStateId;
	}
	return nRet;
}

int InsertAnglerHistory(PANGLERSTATE pAnglerState)
{
	SYSTEMTIME SysTime;
	GetLocalTime(&SysTime);
	WCHAR szRegistTime[100];
	swprintf(szRegistTime, L"%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
	
	int nRet = 0;
	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	TCHAR wszGameId[200] = {0,};
	MultiToWide(pAnglerState->szGameID, wszGameId);
	int nServerIndex = (pAnglerState->byServer)%(MAX_GAME_SERVER + 1);
	int AccountStateId = -1;
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		SQLTCHAR SqlInsertOrimState[1000] = {0,};
		switch(pAnglerState->byType)
		{
		case ANGLER_CONNECT:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `connect_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType]);
			}
			break;
		case ANGLER_REST_USETIME:
			{
				WCHAR szRestUseTime[200];
				swprintf(szRestUseTime, g_wszRestTimePattern, pAnglerState->dwValue / 10, pAnglerState->dwValue % 10, szRegistTime);
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `restusetime` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRestUseTime);
			}
			break;
		case ANGLER_GETADEN:
			{
				WCHAR szState[200];
				swprintf(szState, g_wszState[pAnglerState->byType], pAnglerState->dwValue);
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `getaden_time` = '%s', `getaden_cnt` = '%d', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, pAnglerState->dwValue, szRegistTime, szState);
			}
			break;
		case ANGLER_ERROR_BUYBAIT:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, g_wszState[pAnglerState->byType]);
			}
			break;
		case ANGLER_BUYBAIT_CNT:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `bait_cnt` = '%d', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					pAnglerState->dwValue, szRegistTime, g_wszState[pAnglerState->byType]);
			}
			break;
		case ANGLER_ENTERFISHINGHOLE:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `enter_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType]);
			}
			break;
		case ANGLER_ERROR_MOVETOFISHINGPLACE:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, g_wszState[pAnglerState->byType]);
			}
			break;
		case ANGLER_STARTFISHING:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `start_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType]);
			}
			break;
		case ANGLER_FISHING_BAITNUM:
			{
				WCHAR szState[200];
				swprintf(szState, g_wszState[pAnglerState->byType], pAnglerState->dwValue);
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, szState);
			}
			break;
		case ANGLER_ENDFISHING:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `end_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType]);
			}
			break;
		case ANGLER_DEPOSITADEN:
			{
				WCHAR szState[200];
				swprintf(szState, g_wszState[pAnglerState->byType], pAnglerState->dwValue);
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `depositaden_time` = '%s', `depositaden_cnt` = '%d', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, pAnglerState->dwValue, szRegistTime, szState);
			}
			break;
		case ANGLER_DISCONNECT:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("INSERT INTO `%s` SET `account` = '%s', \
`gameid` = '%s', `server` = '%s', `charnum` = '%d', `disconnect_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, pAnglerState->wszID, wszGameId, g_lpGameServerName[nServerIndex], pAnglerState->byCharNum, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType]);
			}
			break;
		default:
			break;
		}

		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlInsertOrimState,
			SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );

	}
	return nRet;
}

int UpdateAnglerHistory(PANGLERSTATE pAnglerState, int nIndex)
{
	SYSTEMTIME SysTime;
	GetLocalTime(&SysTime);
	WCHAR szRegistTime[100];
	swprintf(szRegistTime, L"%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
	
	int nRet = 0;
	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	TCHAR wszGameId[200] = {0,};
	MultiToWide(pAnglerState->szGameID, wszGameId);
	int nServerIndex = (pAnglerState->byServer)%(MAX_GAME_SERVER + 1);
	int AccountStateId = -1;
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		SQLTCHAR SqlInsertOrimState[1000] = {0,};
		switch(pAnglerState->byType)
		{
		case ANGLER_CONNECT:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`connect_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType], nIndex);
			}
			break;
		case ANGLER_REST_USETIME:
			{
				WCHAR szRestUseTime[200];
				swprintf(szRestUseTime, g_wszRestTimePattern, pAnglerState->dwValue / 10, pAnglerState->dwValue % 10, szRegistTime);
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`restusetime` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, szRestUseTime, nIndex); 
			}
			break;
		case ANGLER_GETADEN:
			{
				WCHAR szState[200];
				swprintf(szState, g_wszState[pAnglerState->byType], pAnglerState->dwValue);
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`getaden_time` = '%s', `getaden_cnt` = '%d', `laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, pAnglerState->dwValue, szRegistTime, szState, nIndex);
			}
			break;
		case ANGLER_ERROR_BUYBAIT:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, g_wszState[pAnglerState->byType], nIndex);
			}
			break;
		case ANGLER_BUYBAIT_CNT:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`bait_cnt` = '%d', `laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					pAnglerState->dwValue, szRegistTime, g_wszState[pAnglerState->byType], nIndex);
			}
			break;
		case ANGLER_ENTERFISHINGHOLE:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`enter_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType], nIndex);
			}
			break;
		case ANGLER_ERROR_MOVETOFISHINGPLACE:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, g_wszState[pAnglerState->byType], nIndex);
			}
			break;
		case ANGLER_STARTFISHING:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`start_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType], nIndex);
			}
			break;
		case ANGLER_FISHING_BAITNUM:
			{
				WCHAR szState[200];
				swprintf(szState, g_wszState[pAnglerState->byType], pAnglerState->dwValue);
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					 szRegistTime, szState, nIndex);
			}
			break;
		case ANGLER_ENDFISHING:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`end_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType], nIndex);
			}
			break;
		case ANGLER_DEPOSITADEN:
			{
				WCHAR szState[200];
				swprintf(szState, g_wszState[pAnglerState->byType], pAnglerState->dwValue);
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`depositaden_time` = '%s', `depositaden_cnt` = '%d', `laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, pAnglerState->dwValue, szRegistTime, szState, nIndex);
			}
			break;
		case ANGLER_DISCONNECT:
			{
				SQLTCHAR SqlInsertOrimStatePtn[] = _T("UPDATE `%s` SET \
`disconnect_time` = '%s', `laststate_time` = '%s', `laststate_text` = '%s' WHERE `id` = '%d'");
				swprintf(SqlInsertOrimState, SqlInsertOrimStatePtn, g_szAnglerStateTableName, 
					szRegistTime, szRegistTime, g_wszState[pAnglerState->byType], nIndex);
			}
			break;
		default:
			break;
		}

		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlInsertOrimState,
			SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );

	}
	return nRet;

}

void UpdateAnglerHistory(PANGLERSTATE pAnglerState)
{
	DWORD dwTick = GetTickCount();
	TCHAR wszGameId[200] = {0,};
	MultiToWide(pAnglerState->szGameID, wszGameId);

	pAnglerState->byCharNum += 1;

	if(pAnglerState->byType != ANGLER_FISHING_BAITNUM)
	{
		int nIndex = GetAnglerHistoryIndex(pAnglerState);

		if (nIndex == -1)
		{
			InsertAnglerHistory(pAnglerState);
		}
		else
		{
			UpdateAnglerHistory(pAnglerState, nIndex);
		}
	}

	SetAnglerStateInfo(pAnglerState);

	dwTick = GetTickCount() - dwTick;
	PrintLog(L"[STATE] (%s)(%s)(%s) Tick: %d", pAnglerState->wszID, wszGameId, g_wszState[pAnglerState->byType], dwTick);
}

HISTORYINFO* GetAnglerHistory(SYSTEMTIME systime1, SYSTEMTIME systime2, int* pnCount)
{
	TCHAR szStartTime[100];
	_stprintf(szStartTime, _T("%04d-%02d-%02d 00:00:00"), systime1.wYear, systime1.wMonth, systime1.wDay);
	FILETIME filetime;
	SystemTimeToFileTime(&systime2, &filetime);
	ULONGLONG ullOneDay = 864000000000;
	ULONGLONG ullEnd = *(ULONGLONG*)&filetime + ullOneDay;
	FileTimeToSystemTime((LPFILETIME)&ullEnd, &systime2);
	TCHAR szEndTime[100];
	_stprintf(szEndTime, _T("%04d-%02d-%02d 00:00:00"), systime2.wYear, systime2.wMonth, systime2.wDay);

	int nCount = -1;

	SQLHSTMT sql_hStmt = 0;
	SQLTCHAR szDNS[1024] ={0};
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetAccountCount[1000] = {0,};
		SQLTCHAR SqlGetAccountCountPtn[] = _T("SELECT count(*) AS total from `%s` WHERE `connect_time` >= '%s' AND `connect_time` < '%s'");
		_stprintf(SqlGetAccountCount, SqlGetAccountCountPtn, g_szAnglerStateTableName, szStartTime, szEndTime);

		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetAccountCount,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{

			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_INTEGER,
				&(nCount),
				4,
				NULL );
			if(SQL_SUCCEEDED(sqlRet) && nCount != -1)
			{
			}
		}

		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	}

	*pnCount = 0;
	if(nCount <= 0)
	{
		return NULL;
	}
	HISTORYINFO* pHistoryInfo = new HISTORYINFO[nCount];

	SQLTCHAR SqlGetAccountState[1000] = {0,};
	SQLTCHAR SqlGetAccountStatePtn[] = _T("SELECT `account`, `gameid`, `server`, `charnum`, `connect_time`, `getaden_cnt`, `depositaden_cnt` \
FROM `%s` WHERE `connect_time` >= '%s' AND `connect_time` < '%s' ORDER BY `account`, `gameid`");
	_stprintf(SqlGetAccountState, SqlGetAccountStatePtn, g_szAnglerStateTableName, szStartTime, szEndTime);

	sqlRet =
		SQLAllocHandle( SQL_HANDLE_STMT,
		g_sql_hDBC,
		&sql_hStmt );
	sqlRet =
		SQLExecDirect( sql_hStmt,
		(SQLTCHAR*)SqlGetAccountState,
		SQL_NTS );


	for(int k = 0; k < nCount; k ++)
	{
		WCHAR	szID[20];
		WCHAR	szGameId[40];
		WCHAR	szServer[20];
		int		nCharNum;
		WCHAR   szConnectTime[20];
		DWORD   dwGetAden;
		DWORD   dwDepositeAden;

		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
			sqlRet = SQLGetData( sql_hStmt,
				1,
				SQL_C_TCHAR,
				szID,
				sizeof(szID),
				NULL );
			sqlRet = SQLGetData( sql_hStmt,
				2,
				SQL_C_TCHAR,
				szGameId,
				sizeof(szGameId),
				NULL );
			sqlRet = SQLGetData( sql_hStmt,
				3,
				SQL_C_TCHAR,
				szServer,
				sizeof(szServer),
				NULL );
			sqlRet = SQLGetData( sql_hStmt,
				4,
				SQL_INTEGER,
				&nCharNum,
				4,
				NULL );
			sqlRet = SQLGetData( sql_hStmt,
				5,
				SQL_C_TCHAR,
				szConnectTime,
				sizeof(szConnectTime),
				NULL );
			sqlRet = SQLGetData( sql_hStmt,
				6,
				SQL_INTEGER,
				&dwGetAden,
				4,
				NULL );
			sqlRet = SQLGetData( sql_hStmt,
				7,
				SQL_INTEGER,
				&dwDepositeAden,
				4,
				NULL );

			wcscpy(pHistoryInfo[*pnCount].szID, szID);
			wcscpy(pHistoryInfo[*pnCount].szGameId, szGameId);
			wcscpy(pHistoryInfo[*pnCount].szServer, szServer);
			wcscpy(pHistoryInfo[*pnCount].szConnectTime, szConnectTime);
			pHistoryInfo[*pnCount].nCharNum = nCharNum;
			pHistoryInfo[*pnCount].dwGetAden = dwGetAden;
			pHistoryInfo[*pnCount].dwDepositeAden = dwDepositeAden;
			(*pnCount) ++;
		}
		else
		{
			break;
		}
	}

	SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );	
	return pHistoryInfo;
}


void CreateAnglerStateDBTable()
{
	GetLocalTime(&g_SysTime);
	swprintf(g_szAnglerStateTableName, L"anglerstate_%04d-%02d-%02d", g_SysTime.wYear, g_SysTime.wMonth, g_SysTime.wDay);

	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;
			sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );

	SQLTCHAR SqlAnglerStateTblPtn[] = _T("CREATE TABLE `%s` ( \
`id` int(11) NOT NULL AUTO_INCREMENT, \
`account` varchar(100) NOT NULL DEFAULT '', \
`gameid` varchar(100) NOT NULL DEFAULT '', \
`server` varchar(100) NOT NULL DEFAULT '', \
`charnum` INT(10) NOT NULL DEFAULT '0',\
`connect_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`getaden_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`getaden_cnt` INT(10) NOT NULL DEFAULT '0',\
`bait_cnt` INT(10) NOT NULL DEFAULT '0',\
`enter_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`start_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`end_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`depositaden_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`depositaden_cnt` INT(10) NOT NULL DEFAULT '0',\
`disconnect_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`laststate_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00', \
`laststate_text` varchar(100) NOT NULL DEFAULT '', \
`restusetime` varchar(100) NOT NULL DEFAULT '', \
PRIMARY KEY (`id`) \
) ENGINE=MyISAM  DEFAULT CHARSET=euckr COLLATE=euckr_korean_ci AUTO_INCREMENT=1 ; \
");
		SQLTCHAR SqlAnglerStateTbl[3000];
		_stprintf(SqlAnglerStateTbl, SqlAnglerStateTblPtn, g_szAnglerStateTableName);

		sqlRet = SQLExecDirect( sql_hStmt, (SQLTCHAR*)SqlAnglerStateTbl,	SQL_NTS );
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );
}

void GetAnglerState(STATEINFO* pStateInfo)
{
	pStateInfo->szLastTime[0] = 0;
	pStateInfo->szState[0] = 0;
	pStateInfo->szRestTime[0] = 0;

	int nRet = 0;
	SQLHSTMT sql_hStmt = 0;
	SQLSMALLINT nSize = 0;
	SQLRETURN sqlRet = SQL_SUCCESS;

	WCHAR szPrevAccountState[100] = {0};
	if( SQL_SUCCEEDED( sqlRet ) )
	{
		sqlRet =
			SQLAllocHandle( SQL_HANDLE_STMT,
			g_sql_hDBC,
			&sql_hStmt );
		SQLTCHAR SqlGetOrimStatePtn[] = _T("SELECT `laststate_time`, `laststate_text`, `restusetime` FROM `%s` WHERE `account` = '%s' ORDER BY `id` DESC LIMIT 1");
		SQLTCHAR SqlGetOrimState[600] = {0,};

		_stprintf(SqlGetOrimState, SqlGetOrimStatePtn, g_szAnglerStateTableName, pStateInfo->szID);
		sqlRet =
			SQLExecDirect( sql_hStmt,
			(SQLTCHAR*)SqlGetOrimState,
			SQL_NTS );
		if( SQL_SUCCEEDED( sqlRet = SQLFetch( sql_hStmt ) ) )
		{
			sqlRet = SQLGetData( sql_hStmt,  // 
				1,
				SQL_C_TCHAR,
				pStateInfo->szLastTime,
				40,
				NULL );
			if(SQL_SUCCEEDED(sqlRet))
			{
			}
			sqlRet = SQLGetData( sql_hStmt,  // 
				2,
				SQL_C_TCHAR,
				pStateInfo->szState,
				40,
				NULL );
			if(SQL_SUCCEEDED(sqlRet))
			{
			}
			sqlRet = SQLGetData( sql_hStmt,  // 
				3,
				SQL_C_TCHAR,
				pStateInfo->szRestTime,
				80,
				NULL );
			if(SQL_SUCCEEDED(sqlRet))
			{
			}
		}
		SQLFreeHandle( SQL_HANDLE_STMT, sql_hStmt );
	}

	return;
}

void SetAnglerStateInfo(PANGLERSTATE pAnglerState)
{
	for(int i = 0; i < g_nAnglerStateNum; i++)
	{
		if(wcscmp(g_pAnglerStateInfo[i].szID, pAnglerState->wszID) == 0)
		{
			if(pAnglerState->byType == ANGLER_REST_USETIME) // 잔여시간
			{
				SYSTEMTIME SysTime;
				GetLocalTime(&SysTime);
				WCHAR szRegistTime[100];
				swprintf(szRegistTime, L"%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);

				WCHAR szRestUseTime[40];
				swprintf(szRestUseTime, g_wszRestTimePattern, pAnglerState->dwValue / 10, pAnglerState->dwValue % 10, szRegistTime);
				wcscpy(g_pAnglerStateInfo[i].szRestTime, szRestUseTime);
			}
			else
			{
				swprintf(g_pAnglerStateInfo[i].szState, g_wszState[pAnglerState->byType], pAnglerState->dwValue);
				SYSTEMTIME SysTime;
				GetLocalTime(&SysTime);
				swprintf(g_pAnglerStateInfo[i].szLastTime, L"%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
			}
			break;
		}
	}
}