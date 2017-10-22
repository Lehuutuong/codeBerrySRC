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
	L"��������", 
	L"�˶����", 
	L"������", 
	L"�̽Ƿ���", 
	L"����", 
	L"�ϵ�", 
	L"�ɷ��Ͻ�", 
	L"����", 
	L"ũ������", 
	L"����", 
	L"���帮��", 
	L"����", 
	L"�ƽ��׾�", 
	L"��ũ����", 
	L"�߼�", 
	L"���", 
	L"ĳ����", 
	L"���ٽ�î", 
	L"����", 
	L"�ľƱ׸���", 
	L"����", 
	L"������", 
	L"������", 
	L"����", 
	L"���", 
	L"�̷ν�", 
	L"�õ����", 
	L"�ε�����", 
	L"��Ÿ��", 
	L"��������", 
	L"�߶�ī��", 
	L"��������", 
	L"��Ʈ", 
	L"�۷���", 
	L"���ٿ��", 
	L"���", 
	L"����", 
	L"�ǵ���", 
	L"�׶�ī��", 
	L"��Ÿ�ٵ�", 
	L"��������Ʈ", 
	L"�Ƶ�", 
	L"�ƿ��Ű��", 
	L"���Ľ�", 
	L"��ũ", 
	L"������Ʈ", 
	L"�����ϻ��", 
	L"�ο��׸�", 
	L"���̳�",
	L"Ŀ��"
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
	"���ž�� ü�� ȸ����", 
	"ü�� ȸ����", 
	"��� ü�� ȸ����", 
	"���� ü�� ȸ����", 
	"���� ü�� ȸ����", 
	"���� ��� ü�� ȸ����", 
	"���� ���� ü�� ȸ����", 
	"�ż� ü�� ȸ����", 
	"�ż� ��� ü�� ȸ����", 
	"�ż� ���� ü�� ȸ����", 
	"�ٳ��� �ֽ�", 
	"������ �ֽ�", 
	"��� �ֽ�", 
};

#define MAX_MPORTION	2
static LPSTR g_lpMPortionName[MAX_MPORTION] = 
{
	"���ž�� �Ķ� ����", 
};

#define MAX_RSCROLL		5
static LPSTR g_lpRScrollName[MAX_RSCROLL] = 
{
	"������ ��� ���� ��ȯ ����", 
	"���ž�� ��ȯ �ֹ���", 
	"���ž�� ���� ��ȯ �ֹ���", 
	"��ȯ �ֹ���", 
	"���� ��ȯ �ֹ���", 
};

static ITEM_INFO g_infoWareHouseItem[] = {
	{"�Ƶ���", 10000}, 
	{"", 0}
};

static ITEM_INFO g_infoShopItem[] = {
	{"���ž�� Ȯ�� �ֹ���", 10}, 
	{"", 0}
};

enum CharClass {
	CharClassRoyal,			// ����
	CharClassKnight,		// ���
	CharClassElf,			// ����
	CharClassWizard,		// ������
	CharClassDarkelf,		// ��ũ����
	CharClassDragonKnight,	// ����
	CharClassIllusionist,	// ȯ����
	CharClassEnd
};

enum VILLAGES {
	VillageTalkingIsland,		// ���ϴ� ��
	VillageGludio,				// �۷���
	VillageKent,				// ��Ʈ
	VillageWindawood,			// ���ٿ��
	VillageSilverKnightTown,	// ����� ����
	VillageOrcForest,			// ��ũ ��
	VillageGiran,				// ���
	VillageHeine,				// ���̳�
	VillageLairOfValakas,		// �߶�ī�� ����
	VillageOren,				// ����
	VillageAden,				// �Ƶ�
	VillageHiddenValley,		// ������ ���
	VillageBehemoth,			// ������
	TradeZoneMarket,			// ����
	VillageEnd
};

enum REGIONS {
	RegionTalkingTraining,		// ������(�ʺ���)
	RegionTalkingWForest,		// ���� ���(�ʺ���)
	RegionGuide0_1_1,			// ���� ��/���Ǯ ����
	RegionGuide0_1_2,			// ����� ��
	RegionGuide0_1_8,			// �縷 �������
	RegionGuide0_1_4,			// ���� ���� 1��
	RegionGuide0_1_5,			// ���� ���� 2��
	RegionEnd
};

enum NPC_TYPE {
	NpcTypeNone, 
	NpcTypeWareHouse,	// â������
	NpcTypeGrocery,		// ��ȭ��
	NpcTypeTeleporter,	// �ڷ�����
	NpcTypeDungeon, 	// ����
	NpcTypeQuest, 		// ����Ʈ
	NpcTypeTradeTeleporter, // ���� �ڷ�����
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
// 	{VillageTalkingIsland,			"����",		NpcTypeWareHouse,	0,    32575, 32946},
// 	{VillageGludio,					"ī��",		NpcTypeWareHouse,	4,    32616, 32800},
// 	{VillageKent,	    			"����",		NpcTypeWareHouse,	4,    33072, 32798},
// 	{VillageWindawood,    			"Ÿ��Ų",	NpcTypeWareHouse,	4,    32610, 33184},
	{VillageSilverKnightTown,		"���",		NpcTypeWareHouse,	4,    33079, 33395},
// 	{VillageOrcForest,				"����ƾ",	NpcTypeWareHouse,	4,    32746, 32444},
// 	{VillageGiran,  				"����",	NpcTypeWareHouse,	4,    33422, 32812},
// 	{VillageGiran,   				"���",		NpcTypeWareHouse,	4,    33431, 32816},	
// 	{VillageHeine,   				"��Ŵ",		NpcTypeWareHouse,	4,    33601, 33235},
// 	{VillageLairOfValakas,			"������",	NpcTypeWareHouse,	4,    33724, 32491},
// 	{VillageOren,					"����",		NpcTypeWareHouse,	4,    34054, 32287},
// 	{VillageAden,					"ī��",		NpcTypeWareHouse,	4,    33962, 33243},
// 	{VillageAden, 					"����Ų",	NpcTypeWareHouse,	4,    34086, 33144},
// 	{VillageAden,				 	"ī���",	NpcTypeWareHouse,	4,    33922, 33345},
// 	{VillageAden,					"����",		NpcTypeWareHouse,	4,    34001, 33351},
	{VillageHiddenValley,			"�丮",		NpcTypeWareHouse,	2005, 32675, 32852},

// 	{VillageTalkingIsland,			"�ǵ���",	NpcTypeGrocery,		0,    {32641, 32950},	32640, 32951},
// 	{VillageGludio,					"��",		NpcTypeGrocery,		4,    {32596, 32743},	32596, 32741},
// 	{VillageKent,					"�̼Ҹ���",	NpcTypeGrocery,		4,    {33064, 32733},	33067, 32733},
// 	{VillageWindawood,				"���̳�",	NpcTypeGrocery,		4,    {32632, 33194},	32632, 33192},
	{VillageSilverKnightTown,		"�Ḱ",		NpcTypeGrocery,		4,    {33097, 33385},	33096, 33383}, 
// 	{VillageOrcForest,				"�轼",		NpcTypeGrocery,		4,    {32752, 32432},	32751, 32433},
// 	{VillageGiran,   				"���̾�",	NpcTypeGrocery,		4,    {33455, 32820},	33457, 32819},
// 	{VillageHeine,   				"�긮Ʈ",	NpcTypeGrocery,		4,    {33641, 33286},	33641, 33283},
// 	{VillageLairOfValakas,			"����",		NpcTypeGrocery,		4,    {33739, 32492},	33740, 32490},
// 	{VillageOren,					"��콺",	NpcTypeGrocery,		4,    {34065, 32289},	34065, 32287},
// 	{VillageAden,   				"���",		NpcTypeGrocery,		4,    {34152, 33124},	34152, 33121},	
	{VillageHiddenValley,			"�����",	NpcTypeGrocery,		2005, {32675, 32841}},

	{VillageTalkingIsland,			"��ī��",	NpcTypeTeleporter,	0,    32583, 32922},
	{VillageGludio,					"�ƽ���",  	NpcTypeTeleporter,	4,    32611, 32732},
	{VillageKent,					"���ٸ�",  	NpcTypeTeleporter,	4,    33050, 32783},
	{VillageWindawood,				"Ʈ����",  	NpcTypeTeleporter,	4,    32615, 33170},
	{VillageSilverKnightTown,		"��Ʈ",  	NpcTypeTeleporter,	4,    33080, 33385},
	{VillageOrcForest,				"���",		NpcTypeTeleporter,	4,    32721, 32443},
	{VillageAden,					"��������",	NpcTypeTeleporter,	4,    33934, 33351},
	{VillageGiran,					"����",		NpcTypeTeleporter,	4,    33437, 32795},
	{VillageHeine,					"����",		NpcTypeTeleporter,	4,    33613, 33257},
	{VillageLairOfValakas,			"������",	NpcTypeTeleporter,	4,    33709, 32499},
	{VillageOren,					"Ű���콺",	NpcTypeTeleporter,	4,    34063, 32278},
	{VillageAden,					"�ø��콺",	NpcTypeTeleporter,	4,    33964, 33252},
	{VillageHiddenValley,			"������",	NpcTypeTeleporter,	2005, 32679, 32860},
	{TradeZoneMarket,				"����",		NpcTypeTeleporter,	800,  32805, 32930},

	{VillageTalkingIsland,			"������",	NpcTypeDungeon,		0,    32578, 32922},

	{VillageHiddenValley,			"ī�ÿ����",	NpcTypeQuest,	2005, 32689, 32850},
	{VillageSilverKnightTown,		"������",		NpcTypeQuest,	4,	  33068, 33390},
	{VillageSilverKnightTown,		"���� �Ʒ� ����",NpcTypeQuest,	4,	  33071, 33393},

	{VillageGludio,					"�޵�",  		NpcTypeTradeTeleporter,	4,   32629, 32721},
	{VillageGludio,					"�޵�",  		NpcTypeTradeTeleporter,	4,   32633, 32795},
	{VillageGiran,					"����",  		NpcTypeTradeTeleporter,	4,   33438, 32807},
	{VillageOren,					"����",  		NpcTypeTradeTeleporter,	4,   34063, 32272},
	{VillageOren,					"����",  		NpcTypeTradeTeleporter,	4,	 34061, 32305},
	{VillageSilverKnightTown,		"����",  		NpcTypeTradeTeleporter,	4,	 33083, 33406},
	{VillageSilverKnightTown,		"����",  		NpcTypeTradeTeleporter,	4,	 33101, 33371},

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
	{"�Ƶ���",			 1, "�Ƶ���"}, 
	{"���� ����",		 1, "���� ����"},
	{"���� ����",		 1, "���� ����"},
	{"���� ���� �ֹ���", 0, "����"}, // �ູ����
	{"���� ���� �ֹ���", 0, "�൥��"}, // �ູ����
	{"���� ���� �ֹ���", 1, "��"}, 
	{"���� ���� �ֹ���", 1, "����"}, 
	{"������ �ϱ���",	 1, "������ �ϱ���"},
	{"��ο� �ϵ��� �ϱ���", 1, "��ο� �ϵ��� �ϱ���"},
	{"������ �ϱ� ",		 1, "������ �ϱ� "}
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
	"������ �ϱ� 6/14",
	"������ �ϱ� 6/16",
	"������ �ϱ� 6/18",
	"������ �ϱ� 6/21",
	"������ �ϱ� 6/22",
	"������ �ϱ� 6/25",
	"������ �ϱ� 7/05",
	"������ �ϱ� 7/17",
	"������ �ϱ� 7/18",
	"������ �ϱ� 8/05",
	"������ �ϱ� 8/08",
	"������ �ϱ� 8/09",
	"������ �ϱ� 8/10",
	"������ �ϱ� 8/11",
	"������ �ϱ� 8/12",
	"������ �ϱ� 8/13",
	"������ �ϱ� 8/14",
	"������ �ϱ� 8/15"
};

enum {
	DIARY_NONE = 0,
	DIARY_RETRIEVE,
	DIARY_DEPOSITE,
	DIARY_CHECK
} DiaryState;

typedef enum {
	TradeResultSuccess = 0, 
	TradeResultPledgeEmpty = 1,		// ���Ϳ� �������� ����
	TradeResultSellerEmpty = 2,		// ����ĳ�� �̸��� ����
	TradeResultTeleportFailed = 3,	// �������� �̵��� ����
	TradeResultSellerNotFound = 4,  // ����ĳ���� ã���� ����
	TradeResultSellerDisconnected = 5,  // ����ĳ���� ���� ������
} TradeResult;

static LPSTR g_lpTradeResult[] = {
	"���� �Ϸ�", 
	"���Ϳ� �������� ����", 
	"����ĳ�� �̸��� ����", 
	"�������� �̵��� ����", 
	"����ĳ���� ã���� ����", 
	"����ĳ���� ���� ������", 
};

typedef enum {
	ObjectTypeDied = 3,			// ���� ������Ʈ
	ObjectTypeSmallBoard = 4,	// ���� �Խ���
	ObjectTypeCharacter = 5,	// ĳ����
	ObjectTypeNpc = 6,			// ���ǽ�
	ObjectTypeItem = 9,			// ������
	ObjectTypeMonster = 10,		// ����
	ObjectTypeNpc1 = 12,		// ���ǽ�
	ObjectTypeBoard = 14		// �Խ���
} ObjectType;



LPSTR GetLineagePath(LPSTR lpPath, DWORD cbData);
int GetServerIndex(LPCSTR lpGameServerName);
