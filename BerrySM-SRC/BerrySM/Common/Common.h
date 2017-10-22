#pragma once

#ifdef _TEST

// #define SERVER_IP		"192.168.1.10" 
// #define	SERVER_FTP_IP	"192.168.1.10"
#define SERVER_IP		"192.168.1.198"
#define	SERVER_FTP_IP	"192.168.1.198"
#define VMProtectBegin(x)
#define VMProtectEnd()
#define VMProtectDecryptStringW(x) x
#define VMProtectDecryptStringA(x) x

#else
#define SERVER_IP		"218.10.18.46"
#define	SERVER_FTP_IP	"218.10.18.46"
#include "../common/VMProtectSDK.h"

#endif


#if 0
#include "virtualizersdk.h"
#pragma comment(lib, "VirtualizerSDK32.lib")
#else
#define VirtualizerStart()
#define VirtualizerEnd()
#endif

#define		SERVER_PORT_ADMINQUERY		6850
#define		SERVER_PORT_ADMINSECURE		6851
#define		SERVER_PORT_DES				5882
#define		SERVER_PORT_SECURE			5893
#define		SERVER_PORT_STATE			5900
#define		SERVER_PORT_AUTOCONTROL		5700

#define		SERVER_DSN			_T("berrydb")


#define		MSG_TITLE			"Berry"

#define		EXE_NAME			"Berry.exe"
#define		DLL_NAME			"BerryEngine.dll"
#define		DATA_SET_NAME		"Version.ini"

#define		UPDATE_NAME			"Update.exe"

#define		EXE_MUTEX			"Global\\Angler_Mutex"
#define		SERVER_MUTEX		L"SERVER"

#define		ADMIN_TOP_ID		L"berrytop"
#define		ADMIN_TOP_PWD		L"topadmin_"

#define     MAX_NOTICE_LEGNTH		1000

#define		OPCODE_ADMINCONN				0x10
#define		OPCODE_ADMIN_ACCOUNTUPDATE		0x11
#define		OPCODE_ADMINFILE				0x13
#define		OPCODE_BOSSCONN					0x14
#define		OPCODE_BOSS_ADMINUPDATE			0x15
#define		OPCODE_ADMIN_ACCOUNTLISTUPDATE	0x16
#define		OPCODE_INITMAC					0x17
#define		OPCODE_ADMINEXIT				0x18
#define		OPCODE_FAILADD					0x19
#define		OPCODE_QUERYANGLERSTATE			0x22
#define		OPCODE_QUERYANGLERHISTORY		0x21


#define     OPCODE_ACCOUNTSET_UPLOAD	0x40
#define     OPCODE_ACCOUNTSET_DOWNLOAD	0x41

#define		OPCODE_LOGIN			0x20
#define		OPCODE_SECONDLOGIN		0x21
#define		OPCODE_EXIT				0x22
#define		OPCODE_LOGIN_DES		0x23

#define		OPCODE_MULTICOUNT_DATE			0x25

#define     OPCODE_NOTICE_QUERY		0x30

#define		OPCODE_ACCOUNT_STATE	0x70
#define		OPCODE_CHARAC_STATE		0x71

#define     OPCODE_USERQUERY		0x80
#define     OPCODE_ACCOUNTQUERY     0x81

#define     OPCODE_MEMBER_INFO		0xA0

#define     OPCODE_ANGLER_STATE		0x61

#define		OPCODE_AUTOCTRLCMD_END		0x10
#define		OPCODE_AUTOCTRLCMD_RERUN	0x11
#define		OPCODE_AUTOCTRLCMD_STOP		0x12
#define		OPCODE_AUTOCTRLCMD_START	0x13
#define		OPCODE_AUTOCTRLCMD_GAMERUN	0x14

#define		USERINFO_UPDATE		0
#define		USERINFO_ADD		1
#define		USERINFO_DEL		2

#define		ADMININFO_UPDATE	0
#define		ADMININFO_ADD		1
#define		ADMININFO_DEL		2

#define     RECV_TIMEOUT        30000

#define		MULTI_COUNT			1

//////////////////////////////////////////////////////////////////////////

enum
{
	ANGLER_CONNECT = 0,
	ANGLER_REST_USETIME,
	ANGLER_GETADEN,
	ANGLER_ERROR_BUYBAIT,
	ANGLER_BUYBAIT_CNT,
	ANGLER_ENTERFISHINGHOLE,
	ANGLER_ERROR_MOVETOFISHINGPLACE,
	ANGLER_STARTFISHING,
	ANGLER_FISHING_BAITNUM,
	ANGLER_ENDFISHING,
	ANGLER_DEPOSITADEN,
	ANGLER_DISCONNECT,
	ANGLER_NONE,
};

struct HISTORYINFO
{
	WCHAR	szID[20];
	WCHAR	szGameId[40];
	WCHAR	szServer[20];
	int		nCharNum;
	WCHAR   szConnectTime[20];
	DWORD   dwGetAden;
	DWORD   dwDepositeAden;
};

typedef struct {
	WORD	wOpCode;
	WCHAR	wszID[24];
	char	szGameID[40];
	BYTE    byServer;
	BYTE	byCharNum;
	BYTE	byType;
	DWORD	dwValue;
}XANGLERSTATE, *PANGLERSTATE;

struct STATEINFO
{
	WCHAR	szID[20];
	WCHAR   szState[20];
	WCHAR   szLastTime[20];
	WCHAR   szRestTime[40];
};

#define MAX_GAME_SERVER		50

#define MAX_MACRO			10

#define MAX_CHARNUM			6

static LPWSTR g_lpGameServerName[MAX_GAME_SERVER] = 
{
	L"데포로쥬", 
	L"켄라우헬", 
	L"질리언", 
	L"이실로테", 
	L"조우", 
	L"하딘", 
	L"케레니스", 
	L"오웬", 
	L"크리스터", 
	L"아툰", 
	L"가드리아", 
	L"군터", 
	L"아스테어", 
	L"듀크데필", 
	L"발센", 
	L"어레인", 
	L"캐스톨", 
	L"세바스챤", 
	L"데컨", 
	L"파아그리오", 
	L"에바", 
	L"사이하", 
	L"마프르", 
	L"린델", 
	L"쥬드", 
	L"켈로스", 
	L"시드랏슈", 
	L"로데마이", 
	L"안타라스", 
	L"파프리온", 
	L"발라카스", 
	L"린드비오르", 
	L"켄트", 
	L"글루디오", 
	L"윈다우드", 
	L"기란", 
	L"오렌", 
	L"판도라", 
	L"그랑카인", 
	L"라스타바드", 
	L"데스나이트", 
	L"아덴", 
	L"아우라키아", 
	L"할파스", 
	L"오크", 
	L"바포메트", 
	L"아인하사드", 
	L"로엔그린", 
	L"하이네",
	L"커츠"
};
typedef enum {
	F5 = 0,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12
}SHORT_KEY;

typedef enum {
	TYPE_NONE = 0,
	TYPE_ITEM,
	TYPE_UNKNOWN,
	TYPE_SKILL
}SHORT_TYPE;

typedef enum {
	ITEM_FALSE = 0,
	ITEM_TRUE
}SHORT_ITEM;

typedef struct _XSHORTINFO{
	WORD wSlotIndex;
	CHAR szItemName[40];
	WORD wType;
	WORD wIsItem;
}XSHORTINFO,*PXSHORTINFO;

typedef struct _ACCOUNT_INFO {
	CHAR	szGameId[40];
	CHAR	szGamePass[20];
	CHAR	szGamePass2[10];
	CHAR	szGameServer[40];
	BOOL	bPlayNC;
	BOOL	bDiaryWork;
	CHAR	szGameCharName[20];
} ACCOUNT_INFO, *PACCOUNT_INFO;

typedef struct _PROTECT_INFO {
	BOOL	bUseHP1;
	int		dwUseHP1;
	CHAR	szUseHP1[40];
	BOOL	bUseHP2;
	DWORD	dwUseHP2;
	CHAR	szUseHP2[40];
	BOOL	bUseMP;
	DWORD	dwUseMP;
	CHAR	szUseMP[40];
	BOOL	bRestHP;
	DWORD	dwRestHP1;
	DWORD	dwRestHP2;
	BOOL	bRestMP;
	DWORD	dwRestMP1;
	DWORD	dwRestMP2;
	BOOL	bReturnHP;
	DWORD	dwReturnHP;
	BOOL	bReturnMP;
	DWORD	dwReturnMP;
	BOOL	bReturnHPortion;
	DWORD	dwReturnHPortion;
	BOOL	bReturnMPortion;
	DWORD	dwReturnMPortion;
	BOOL	bReturnWeight;
	DWORD	dwReturnWeight;
	BOOL	bReturnHaste;
	CHAR	szReturnScroll[40];
} PROTECT_INFO, *PPROTECT_INFO;

typedef struct _ITEM_INFO {
	CHAR	szName[40];
	DWORD	dwCount;
} ITEM_INFO, *PITEM_INFO;

#define MAX_ITEM		20
typedef struct _WAREHOUSE_INFO {
	CHAR	szWareHouseNpc[40];
	int		nWareHouseType;
	int		nWareHouseItemCount;
	ITEM_INFO infoWareHouseItem[MAX_ITEM];
} WAREHOUSE_INFO, *PWAREHOUSE_INFO;

typedef struct _SHOP_INFO {
	CHAR	szShopNpc[40];
	int		nShopItemCount;
	ITEM_INFO infoShopItem[MAX_ITEM];
} SHOP_INFO, *PSHOP_INFO;

#define MAX_HPORTION	13
static LPSTR g_lpHPortionName[MAX_HPORTION] = 
{
	"상아탑의 체력 회복제", 
	"체력 회복제", 
	"고급 체력 회복제", 
	"강력 체력 회복제", 
	"농축 체력 회복제", 
	"농축 고급 체력 회복제", 
	"농축 강력 체력 회복제", 
	"신속 체력 회복제", 
	"신속 고급 체력 회복제", 
	"신속 강력 체력 회복제", 
	"바나나 주스", 
	"오렌지 주스", 
	"사과 주스", 
};

#define MAX_MPORTION	2
static LPSTR g_lpMPortionName[MAX_MPORTION] = 
{
	"상아탑의 파란 물약", 
};

#define MAX_RSCROLL		5
static LPSTR g_lpRScrollName[MAX_RSCROLL] = 
{
	"숨겨진 계곡 마을 귀환 부적", 
	"상아탑의 귀환 주문서", 
	"상아탑의 혈맹 귀환 주문서", 
	"귀환 주문서", 
	"혈맹 귀환 주문서", 
};

static ITEM_INFO g_infoWareHouseItem[] = {
	{"아데나", 10000}, 
	{"", 0}
};

static ITEM_INFO g_infoShopItem[] = {
	{"상아탑의 확인 주문서", 10}, 
	{"", 0}
};

enum CharClass {
	CharClassRoyal,			// 군주
	CharClassKnight,		// 기사
	CharClassElf,			// 요정
	CharClassWizard,		// 마법사
	CharClassDarkelf,		// 다크엘프
	CharClassDragonKnight,	// 용기사
	CharClassIllusionist,	// 환술사
	CharClassEnd
};

enum VILLAGES {
	VillageTalkingIsland,		// 말하는 섬
	VillageGludio,				// 글루디오
	VillageKent,				// 켄트
	VillageWindawood,			// 윈다우드
	VillageSilverKnightTown,	// 은기사 마을
	VillageOrcForest,			// 오크 숲
	VillageGiran,				// 기란
	VillageHeine,				// 하이네
	VillageLairOfValakas,		// 발라카스 둥지
	VillageOren,				// 오렌
	VillageAden,				// 아덴
	VillageHiddenValley,		// 숨겨진 계곡
	VillageBehemoth,			// 베히모스
	TradeZoneMarket,			// 시장
	VillageEnd
};

enum REGIONS {
	RegionTalkingTraining,		// 수련장(초보존)
	RegionTalkingWForest,		// 동쪽 계곡(초보존)
	RegionGuide0_1_1,			// 동쪽 숲/라우풀 신전
	RegionGuide0_1_2,			// 고요한 숲
	RegionGuide0_1_8,			// 사막 경계지대
	RegionGuide0_1_4,			// 수련 던전 1층
	RegionGuide0_1_5,			// 수련 던전 2층
	RegionEnd
};

enum NPC_TYPE {
	NpcTypeNone, 
	NpcTypeWareHouse,	// 창고지기
	NpcTypeGrocery,		// 잡화상
	NpcTypeTeleporter,	// 텔레포터
	NpcTypeDungeon, 	// 던전
	NpcTypeQuest, 		// 퀘스트
	NpcTypeTradeTeleporter, // 시장 텔레포터
};

typedef struct _POS {
	WORD	x;
	WORD	y;
	DWORD	dwMapID;
} POS, *PPOS;

typedef struct _NPC_INFO {
	VILLAGES	nVillage;
	CHAR		szNpcName[40];
	NPC_TYPE	nNpcType;
	WORD		wMapNumber;
	POS			pos;
	POS			posFront;
} NPC_INFO, *PNPC_INFO;

static NPC_INFO g_infoNpc[] = {
// 	{VillageTalkingIsland,			"도린",		NpcTypeWareHouse,	0,    32575, 32946},
// 	{VillageGludio,					"카림",		NpcTypeWareHouse,	4,    32616, 32800},
// 	{VillageKent,	    			"스람",		NpcTypeWareHouse,	4,    33072, 32798},
// 	{VillageWindawood,    			"타르킨",	NpcTypeWareHouse,	4,    32610, 33184},
	{VillageSilverKnightTown,		"고담",		NpcTypeWareHouse,	4,    33079, 33395},
// 	{VillageOrcForest,				"쿠하틴",	NpcTypeWareHouse,	4,    32746, 32444},
// 	{VillageGiran,  				"사우람",	NpcTypeWareHouse,	4,    33422, 32812},
// 	{VillageGiran,   				"노딤",		NpcTypeWareHouse,	4,    33431, 32816},	
// 	{VillageHeine,   				"하킴",		NpcTypeWareHouse,	4,    33601, 33235},
// 	{VillageLairOfValakas,			"엑셀론",	NpcTypeWareHouse,	4,    33724, 32491},
// 	{VillageOren,					"히림",		NpcTypeWareHouse,	4,    34054, 32287},
// 	{VillageAden,					"카무",		NpcTypeWareHouse,	4,    33962, 33243},
// 	{VillageAden, 					"팀프킨",	NpcTypeWareHouse,	4,    34086, 33144},
// 	{VillageAden,				 	"카루딤",	NpcTypeWareHouse,	4,    33922, 33345},
// 	{VillageAden,					"쥬케",		NpcTypeWareHouse,	4,    34001, 33351},
	{VillageHiddenValley,			"토리",		NpcTypeWareHouse,	2005, 32675, 32852},

// 	{VillageTalkingIsland,			"판도라",	NpcTypeGrocery,		0,    {32641, 32950},	32640, 32951},
// 	{VillageGludio,					"룻",		NpcTypeGrocery,		4,    {32596, 32743},	32596, 32741},
// 	{VillageKent,					"이소리야",	NpcTypeGrocery,		4,    {33064, 32733},	33067, 32733},
// 	{VillageWindawood,				"엘미나",	NpcTypeGrocery,		4,    {32632, 33194},	32632, 33192},
	{VillageSilverKnightTown,		"멜린",		NpcTypeGrocery,		4,    {33097, 33385},	33096, 33383}, 
// 	{VillageOrcForest,				"잭슨",		NpcTypeGrocery,		4,    {32752, 32432},	32751, 32433},
// 	{VillageGiran,   				"메이어",	NpcTypeGrocery,		4,    {33455, 32820},	33457, 32819},
// 	{VillageHeine,   				"브리트",	NpcTypeGrocery,		4,    {33641, 33286},	33641, 33283},
// 	{VillageLairOfValakas,			"베리",		NpcTypeGrocery,		4,    {33739, 32492},	33740, 32490},
// 	{VillageOren,					"비우스",	NpcTypeGrocery,		4,    {34065, 32289},	34065, 32287},
// 	{VillageAden,   				"라온",		NpcTypeGrocery,		4,    {34152, 33124},	34152, 33121},	
	{VillageHiddenValley,			"프라운",	NpcTypeGrocery,		2005, {32675, 32841}},

	{VillageTalkingIsland,			"루카스",	NpcTypeTeleporter,	0,    32583, 32922},
	{VillageGludio,					"아스터",  	NpcTypeTeleporter,	4,    32611, 32732},
	{VillageKent,					"스텐리",  	NpcTypeTeleporter,	4,    33050, 32783},
	{VillageWindawood,				"트레이",  	NpcTypeTeleporter,	4,    32615, 33170},
	{VillageSilverKnightTown,		"메트",  	NpcTypeTeleporter,	4,    33080, 33385},
	{VillageOrcForest,				"듀발",		NpcTypeTeleporter,	4,    32721, 32443},
	{VillageAden,					"엘레리스",	NpcTypeTeleporter,	4,    33934, 33351},
	{VillageGiran,					"윌마",		NpcTypeTeleporter,	4,    33437, 32795},
	{VillageHeine,					"리올",		NpcTypeTeleporter,	4,    33613, 33257},
	{VillageLairOfValakas,			"레슬리",	NpcTypeTeleporter,	4,    33709, 32499},
	{VillageOren,					"키리우스",	NpcTypeTeleporter,	4,    34063, 32278},
	{VillageAden,					"시리우스",	NpcTypeTeleporter,	4,    33964, 33252},
	{VillageHiddenValley,			"도리아",	NpcTypeTeleporter,	2005, 32679, 32860},
	{TradeZoneMarket,				"린지",		NpcTypeTeleporter,	800,  32805, 32930},

	{VillageTalkingIsland,			"유리에",	NpcTypeDungeon,		0,    32578, 32922},

	{VillageHiddenValley,			"카시오페아",	NpcTypeQuest,	2005, 32689, 32850},
	{VillageSilverKnightTown,		"토벌대원",		NpcTypeQuest,	4,	  33068, 33390},
	{VillageSilverKnightTown,		"기초 훈련 교관",NpcTypeQuest,	4,	  33071, 33393},

	{VillageGludio,					"메데",  		NpcTypeTradeTeleporter,	4,   32629, 32721},
	{VillageGludio,					"메데",  		NpcTypeTradeTeleporter,	4,   32633, 32795},
	{VillageGiran,					"엘바",  		NpcTypeTradeTeleporter,	4,   33438, 32807},
	{VillageOren,					"엘버",  		NpcTypeTradeTeleporter,	4,   34063, 32272},
	{VillageOren,					"엘버",  		NpcTypeTradeTeleporter,	4,	 34061, 32305},
	{VillageSilverKnightTown,		"벨거",  		NpcTypeTradeTeleporter,	4,	 33083, 33406},
	{VillageSilverKnightTown,		"벨거",  		NpcTypeTradeTeleporter,	4,	 33101, 33371},

	{VillageEnd,}
};

#define MAX_TRADE_ITEM	10
typedef struct _TRADE_ITEM_INFO 
{
	CHAR	szName[40];
	BYTE	byBless;
	CHAR	szAliasName[40];
} TRADE_ITEM_INFO, *PTRADE_ITEM_INFO;

static TRADE_ITEM_INFO g_infoTradeItem[MAX_TRADE_ITEM] = {
	{"아데나",			 1, "아데나"}, 
	{"보물 상자",		 1, "보물 상자"},
	{"보상 상자",		 1, "보상 상자"},
	{"갑옷 마법 주문서", 0, "축젤"}, // 축복받은
	{"무기 마법 주문서", 0, "축데이"}, // 축복받은
	{"갑옷 마법 주문서", 1, "젤"}, 
	{"무기 마법 주문서", 1, "데이"}, 
	{"오림의 일기장",	 1, "오림의 일기장"},
	{"어두운 하딘의 일기장", 1, "어두운 하딘의 일기장"},
	{"오림의 일기 ",		 1, "오림의 일기 "}
};

typedef enum {
	STAGE_BOX = 0,
	STAGE_NODIARY,
	STAGE_DIARY
}STAGESTATE;

typedef enum {
	STATE_ADENA = 0,
	STATE_EVENTBOX,
	STATE_GOLDBOX,
	STATE_BLESSEDARMOR,
	STATE_BLESSEDWEAPON,
	STATE_ARMOR,
	STATE_WEAPON,
	STATE_ORIMDIARY,
	STATE_HARDINDIARY,
	STATE_DIARYPIECE
}RESULTSTATE;

static char g_szDiary[18][50] = {
	"오림의 일기 6/14",
	"오림의 일기 6/16",
	"오림의 일기 6/18",
	"오림의 일기 6/21",
	"오림의 일기 6/22",
	"오림의 일기 6/25",
	"오림의 일기 7/05",
	"오림의 일기 7/17",
	"오림의 일기 7/18",
	"오림의 일기 8/05",
	"오림의 일기 8/08",
	"오림의 일기 8/09",
	"오림의 일기 8/10",
	"오림의 일기 8/11",
	"오림의 일기 8/12",
	"오림의 일기 8/13",
	"오림의 일기 8/14",
	"오림의 일기 8/15"
};

enum {
	DIARY_NONE = 0,
	DIARY_RETRIEVE,
	DIARY_DEPOSITE,
	DIARY_CHECK
} DiaryState;

typedef enum {
	TradeResultSuccess = 0, 
	TradeResultPledgeEmpty = 1,		// 혈맹에 가입하지 못함
	TradeResultSellerEmpty = 2,		// 수거캐릭 이름이 없음
	TradeResultTeleportFailed = 3,	// 시장으로 이동이 실패
	TradeResultSellerNotFound = 4,  // 수거캐릭을 찾을수 없음
	TradeResultSellerDisconnected = 5,  // 수거캐릭이 연결 끊어짐
} TradeResult;

static LPSTR g_lpTradeResult[] = {
	"수거 완료", 
	"혈맹에 가입하지 못함", 
	"수거캐릭 이름이 없음", 
	"시장으로 이동이 실패", 
	"수거캐릭을 찾을수 없음", 
	"수거캐릭이 연결 끊어짐", 
};

typedef enum {
	ObjectTypeDied = 3,			// 죽은 오브젝트
	ObjectTypeSmallBoard = 4,	// 작은 게시판
	ObjectTypeCharacter = 5,	// 캐릭터
	ObjectTypeNpc = 6,			// 엔피시
	ObjectTypeItem = 9,			// 아이템
	ObjectTypeMonster = 10,		// 몬스터
	ObjectTypeNpc1 = 12,		// 엔피시
	ObjectTypeBoard = 14		// 게시판
} ObjectType;



LPSTR GetLineagePath(LPSTR lpPath, DWORD cbData);
int GetServerIndex(LPCSTR lpGameServerName);
