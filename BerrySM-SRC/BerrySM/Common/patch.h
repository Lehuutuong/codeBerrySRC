#pragma once

#define PTR_MAX_HP									0xE2A3A0
#define PTR_MAX_MP									0xE2A39C
#define PTR_CUR_HP0									0xDD67F0
#define PTR_CUR_HP1									0xDD67F4
#define PTR_CUR_HP2									0xDD67F8
#define PTR_CUR_MP0									0xDD67FC
#define PTR_CUR_MP1									0xDD6800
#define PTR_CUR_MP2									0xDD6804
//#define PTR_CHARAC_CLS							0xE272F8
#define PTR_ATTACK_MON								0xE272F4
#define PTR_SKILL_STATE								0xCB8E78
#define PTR_SKILL_TIME								0xCB8C90
#define PTR_POINT_INFO								0xCB8C08
#define PTR_EXP										0xE2A3B4
#define PTR_VALID_SKILL								0xE2BB28
//#define PTR_INVENTORY								0xBA32D8
//#define PTR_OBJ_LIST								0xE27328
//#define PTR_OBJ_CNT								0xE27320
#define PTR_MAPID									0xB5EBD8
#define PTR_FORMAT_SEND_PACKET						0x57B640
#define PTR_FUNC_VALTAR								0x5AF660
#define PTR_FUNC_WALL								0x4ED470
#define PTR_FUNC_MOVE								0x4EE5B0
#define PTR_OWNER_ID								0xCB8C7C
#define PTR_REMOVE_MED								0x4E5EC0
#define PTR_SKILL_DISP_LIST							0xCBECB0
#define PTR_GET_MOMENT								0x5923A0
#define PTR_LOOPDISPINFO_INIT						0x50ED60
#define PTR_LOOPDISPINFO_VALID1						0x50ED90
#define PTR_LOOPDISPINFO_VALID2						0x50F0F0
#define PTR_LOOPDISPINFO_GETREAL					0x50EFF0
#define PTR_LOOPDISPINFO_GETNEXT					0x50F010
#define PTR_FUNC_DRAW2								0x557810
#define PTR_SEND_PACKET								0x57AEB0
#define PTR_CHAT_REPLACE_FUNC						0x438930
#define PTR_LIN_HWND								0xBA4DD0
//#define PTR_GET_OBJECT_TYPE						0x5B04D0
#define PTR_RECV_PACKET								0x53E3A0
#define PTR_ACTION_SOCIAL1							0x65DAA0
#define PTR_ACTION_SOCIAL2							0x65DBD0
#define PTR_ACTION_LOOT								0x65D7F0
#define PTR_NPC_ACTION								0x4112E0
#define PTR_WAREHOUSE_WINDOW						0xE27674
#define PTR_TRADE_WINDOW							0xE27278
#define PTR_GET_CHARAC_LIST							0x402660
#define PTR_DECRYPT_VALUE							0x47A040
#define PTR_CHARACTER_PASSWORD						0xB7ACF0
#define PTR_HACK_ATTACK								0x5AFE20
#define PTR_CHAR_WEIGHT								0xE2A39A
#define PTR_CHAR_SATIETY							0xE2A39B
#define PTR_ENTER_CHAT								0x43C400
#define PTR_CAST_SKILL								0x70B200
#define PTR_ENABLE_CAST_SKILL_TICK					0xE2BB18
#define PTR_OPENED_WINDOW							0xE296F8
#define PTR_ERROR_REPORT							0x45A5D4
#define PTR_SHOP_UI									0xE2B56C
#define PTR_USERPLAYSTATS_UI						0xE2B528
#define PTR_SHORT_ITEM								0xBA32F0
#define PTR_FUNC_SETSHORT							0x4AB840
#define OFFSET_CHAR_PROFILE							0x87C
#define OFFSET_CHAR_LEVEL							0x89C
#define PTR_PERSONAL_SHOP							0xE2A990
#define PTR_ENABLE_OPEN_SHOP						0x685210
#define PTR_CLEAR_SELLITEMLIST						0x682260
#define PTR_CLEAR_BUYITEMLIST						0x682770
#define PTR_REGISTER_SELLITEM						0x682130
#define PTR_REGISTER_BUYITEM						0x682640
#define PTR_STAND_XPOS								0xCB9130
#define PTR_STAND_YPOS								0xCB9134
#define OPCODE_SEND_INTRADEMOVE						0x00

#define OPCODE_RECV_SYSMSG							0x0B
#define OPCODE_RECV_CONFIRM							0x27
#define OPCODE_RECV_OBJCLEAR						0x9A
#define OPCODE_RECV_NPC_REPLY						0x67
#define OPCODE_RECV_VARIOUS_STATUS					0x55
#define OPCODE_RECV_ATTACK							0xFF
#define OPCODE_RECV_STAT_CHANGE						0x3F
#define OPCODE_RECV_WHISPER							0xFE
#define OPCODE_RECV_WHO								0x22
#define OPCODE_RECV_CHARNUM							0xEC
#define OPCODE_RECV_VOICE_CHAT						0xAD
#define OPCODE_RECV_NOTICE							0x0D
#define OPCODE_RECV_LOGIN_CHECK						0x09
#define OPCODE_RECV_ITEMADD							0xB5
#define OPCODE_RECV_ITEMMODIFY						0xD1
#define OPCODE_RECV_MEMPOSADD						0x70

#define OPCODE_SEND_USE_ITEM						0x5B
#define OPCODE_SEND_CAST_SKILL						0xED
#define OPCODE_SEND_CHAT							0x4F
#define OPCODE_SEND_MELEE_ATTACK					0x02
#define OPCODE_SEND_BOW_ATTACK						0x15
#define OPCODE_SEND_DROP_ITEM						0x7C
#define OPCODE_SEND_INVITE_PARTY					0x07
#define OPCODE_SEND_NPC_TALK						0x71
#define OPCODE_SEND_ATTACK_CONTINUE					0x40
#define OPCODE_SEND_SELECT_CHAR						0x43
#define OPCODE_SEND_BREAK_PARTY						0x3C
#define OPCODE_SEND_GLOBAL_CHAT						0x4F
#define OPCODE_SEND_DESTROY_ITEM					0x08
#define OPCODE_SEND_CREATE_CHAR						0xFB
#define OPCODE_SEND_NPC_ACTION						0x29
#define OPCODE_SEND_WHISPER							0xE4