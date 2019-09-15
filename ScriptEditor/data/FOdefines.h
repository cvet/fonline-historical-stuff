#ifndef __FODEFINES__H__
#define __FODEFINES__H__

/********************************************************************
	created:	17:07:2007   13:15

	author:		Anton Tsveatinsky, Oleg Mareskin
	
	purpose:	
********************************************************************/

//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

//On test definitions

//#define TEST_NO_ENCAUNTERS
//#define TEST_FAST_ENCAUNTER
//#define TEST_NO_ONATTACK
//#define TEST_FAST_RESPAWN
//#define TEST_NO_TIMEOUT


//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

#pragma warning (disable : 4786)

#include <windows.h>

#include <map>
#include <vector>
#include <deque>
#include <set>
#include <string>
using namespace std;

#include "ITEMPID.H"

//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

//Types
//typedef DWORD		CrID;
//typedef BYTE		CrTYPE;
//typedef BYTE		MSGTYPE;
#define	CrID		DWORD
#define	CrTYPE		BYTE
#define	MSGTYPE		BYTE
#define PrID		WORD	// !Deniska. ������������� ���������.

//����� ����������
typedef map<string, BYTE, less<string> > StrByteMap;
typedef map<string, BYTE, less<string> >::iterator StrByteMapIt;
typedef map<string, BYTE, less<string> >::value_type StrByteMapVal;
typedef map<BYTE, string, less<BYTE> > ByteStrMap;
typedef map<BYTE, string, less<BYTE> >::iterator ByteStrMapIt;
typedef map<BYTE, string, less<BYTE> >::value_type ByteStrMapVal;

typedef map<DWORD, string, less<DWORD> > StringMap;
typedef map<DWORD, string, less<DWORD> >::iterator StringMapIt;
typedef map<DWORD, string, less<DWORD> >::value_type StringMapVal;
typedef multimap<DWORD, string, less<DWORD> > StringMulMap;
typedef multimap<DWORD, string, less<DWORD> >::iterator StringMulMapIt;
typedef multimap<DWORD, string, less<DWORD> >::value_type StringMulMapVal;
typedef set<string> StringSet;
typedef map<string, WORD, less<string> > StrWordMap;
typedef map<string, WORD, less<string> >::iterator StrWordMapIt;
typedef map<string, WORD, less<string> >::value_type StrWordMapVal;
typedef map<string, DWORD, less<string> > StrDWordMap;
typedef map<string, DWORD, less<string> >::iterator StrDWordMapIt;
typedef map<string, DWORD, less<string> >::value_type StrDWordMapVal;
typedef map<WORD, string, less<WORD> > WordStrMap;

typedef vector<int> IntVec;
typedef vector<BYTE> ByteVec;
typedef vector<short> ShortVec;
typedef vector<WORD> WordVec;
typedef vector<WORD>::iterator WordVecIt;
typedef vector<DWORD> DwordVec;
typedef vector<char> CharVec;
typedef vector<string> StrVec;

typedef set<WORD> WordSet;
typedef set<DWORD> DwordSet;
typedef set<CrID> CridSet;
typedef set<CrID>::iterator CridSetIt;

typedef deque<DWORD> DwordDeq;

typedef pair<WORD,WORD> WordPair;
typedef pair<char,char> CharPair;

//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

//Error codes
#define FO_OK						1

#define FO_ERROR					-1

#define FO_ERROR_NULL_PTR			-2

#define FO_ERROR_HEX_LIMITS			-10

#define FO_ERROR_PROTOMAP_LIMITS	-20

#define FO_ERROR_MAP_LIMITS			-30
#define FO_ERROR_MAP_NOT_FIND		-31
#define FO_ERROR_MAP_CANT_ADD_OBJ	-32

#define FO_ERROR_CRIT_LIMITS		-40
#define FO_ERROR_CRIT_NOT_FIND		-41

#define FO_ERROR_OBJ_NOT_CREATED	-50

#define FO_ERROR_ARG0				-100
#define FO_ERROR_ARG1				-101
#define FO_ERROR_ARG2				-102
#define FO_ERROR_ARG3				-103
#define FO_ERROR_ARG4				-104

//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

//��� ������ � ��������� �����������
#define BIN__N(x) (x) | x>>3 | x>>6 | x>>9
#define BIN__B(x) (x) & 0xf | (x)>>12 & 0xf0
#define BIN8(v) (BIN__B(BIN__N(0x##v)))

#define BIN16(bin16,bin8)	((BIN8(bin16)<<8)|(BIN8(bin8)))
#define BIN32(bin32,bin24,bin16,bin8)	((BIN8(bin32)<<24)|(BIN8(bin24)<<16)|(BIN8(bin16)<<8)|(BIN8(bin8)))

//��� ������ � ������ (�� ���� - �������)
#define BITS(x,y) ((x)&(y))
#define FLAG BITS
#define FLAGS(x,y) (((x)&(y))==y)

#define SET_BITS(x,y) ((x)=(x)|(y))
#define SETFLAG SET_BITS

//#define UNSET_BITS(x,y) {if((x)&(y)) (x)=(x)^(y);}
#define UNSET_BITS(x,y) ((x)=((x)|(y))^(y))
#define UNSETFLAG UNSET_BITS

//�������� ������
#define ABS(x) ((x)>=0?(x):-(x))

//��� ��������
#define BREAK_BEGIN do{
#define BREAK_END }while(0)

#define MAKEDWORD(a,b,c,d)								\
((DWORD)(BYTE)(d) | ((DWORD)(BYTE)(c) << 8) |			\
((DWORD)(BYTE)(b) << 16) | ((DWORD)(BYTE)(a) << 24 ))

#define CONVERT_GRAMM(x) ((UINT)((float)(x)*0.4535924f*1000))
//#define CONVERT_GRAMM(x) ((x)*453)

/************************************************************************/
/* Special objects pids                                                 */
/************************************************************************/

#define SP_SCEN_LIGHT				(F2PROTO_OFFSET_SCENERY+141) //Light Source
#define SP_SCEN_BLOCK				(F2PROTO_OFFSET_SCENERY+67) //Secret Blocking Hex
#define SP_SCEN_IBLOCK				(F2PROTO_OFFSET_SCENERY+344) //Block Hex Auto Inviso
#define SP_SCEN_TRIGGER				(3852)

#define SP_WALL_BLOCK				(F2PROTO_OFFSET_WALL+622) //Wall s.t.

#define SP_GRID_EXITGRID			(F2PROTO_OFFSET_SCENERY+49) //Exit Grid Map Marker
#define SP_GRID_ENTIRE				(3853)

#define SP_MISC_SCRBLOCK			(F2PROTO_OFFSET_MISC+12) //Scroll block

#define SP_MISC_GRID_MAP_BEG		(F2PROTO_OFFSET_MISC+16) //Grid to LocalMap begin
#define SP_MISC_GRID_MAP_END		(F2PROTO_OFFSET_MISC+23) //Grid to LocalMap end
#define SP_MISC_GRID_MAP(pid)		((pid)>=SP_MISC_GRID_MAP_BEG && (pid)<=SP_MISC_GRID_MAP_END)

#define SP_MISC_GRID_GM_BEG			(F2PROTO_OFFSET_MISC+31) //Grid to GlobalMap begin
#define SP_MISC_GRID_GM_END			(F2PROTO_OFFSET_MISC+46) //Grid to GlobalMap end
#define SP_MISC_GRID_GM(pid)		((pid)>=SP_MISC_GRID_GM_BEG && (pid)<=SP_MISC_GRID_GM_END)

//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

#define EFFECT_0_TIME_PROC		20
#define EFFECT_0_SPEED_MUL		20.0f

#define TRACE_MAX_ROUNDS		(9)
#define TRACE_BULLET_ANGLE		(Random(5,7))

#define GENDER_MALE				0
#define GENDER_FEMALE			1

#define AGE_MAX					60
#define AGE_MIN					14

//Sound
#define AMBIENT_SOUND_TIME		60000 // Random(X/2,X);

//Ai
#define PATH_AI					".\\ai\\AI.TXT"
#define PATH_AI_BODY			".\\ai\\AIBDYMSG.TXT"
#define PATH_AI_GENERAL			".\\ai\\AIGENMSG.TXT"

#define MAX_TEAM_NUMS			8

#define MOVE_DMG_LEGS			(AP_MS*2)

#define WORLD_START_TIME		"07:00 30:10:2146 x32"

#define RATE_MASK_USE			0x0F
#define RATE_MASK_AIM			0xF0

#define WHISP_DIST				2

//Objects
#define OBJECT_DEF_MAIN			1010
#define OBJECT_DEF_EXT			1010
#define OBJECT_DEF_ARM			1210

const DWORD FADING_PERIOD		=1000;

//Maps
#define F1_MAP_VERSION			0x13000000
#define F2_MAP_VERSION			0x14000000
#define FO_MAP_VERSION_OLD		0xF0000000
#define FO_MAP_VERSION			0xFA000000

#define TRANSFER_TIME			10000

//#define MAX_CITIES			50
//#define MAX_ENCAUNTERS		100
//#define MAX_MAPS				200

#define LOOT_DIST				1

//Proto Maps
#define VERSION_PROTOTYPE_MAP	1
#define MAX_PROTO_MAPS			250
#define MAP_PROTO_EXT			".map"

//Type entires
#define ENTIRE_DEFAULT			(0x00)
#define ENTIRE_REPLICATION		(0xF2)
#define ENTIRE_CAR				(0xF3)
#define ENTIRE_CAR_CRITS		(0xF4)

//Crits
#define CRIT_MIN_TIME_ON_MAP	30000

#define PROTOMAPOBJ_CRITTER		1
#define PROTOMAPOBJ_OBJECT		2

#define PMAPOBJ_ONHEX			1
#define PMAPOBJ_INCONT			2
#define PMAPOBJ_ONCRIT_INV		3
#define PMAPOBJ_ONCRIT_HAND1	4
#define PMAPOBJ_ONCRIT_HAND2	5
#define PMAPOBJ_ONCRIT_ARMOR	6

//Locations
#define VERSION_PROTOTYPE_LOCATION	1
#define MAX_PROTO_LOCATIONS		200
#define LOCATION_PROTO_EXT		".loc"

#define MAX_MAPS_IN_LOCATION	20

#define MAX_KNOWN_LOCATION		60

//type
#define LOCATION_TYPE_NONE		0
#define LOCATION_TYPE_CITY		1
#define LOCATION_TYPE_ENCAUNTER	2
#define LOCATION_TYPE_SPECIAL	3
#define LOCATION_TYPE_QUEST		4

//Global map
const WORD GM_MAXX				=1400;
const WORD GM_MAXY				=1500;
const WORD GM_ZONE_LEN			=50; //������ ���� ������ GM_MAXX � GM_MAXY
const WORD GM_MAXZONEX			=GM_MAXX/GM_ZONE_LEN;
const WORD GM_MAXZONEY			=GM_MAXY/GM_ZONE_LEN;
#define GM_MAX_GROUP_COUNT		10
const DWORD GM_MOVE_1KM_TIME	=10000; //30000
const DWORD GM_ZONE_TIME_PROC	=500;

const BYTE GM_SPEED_SLOW		=0;
const BYTE GM_SPEED_NORM		=1;
const BYTE GM_SPEED_FAST		=2;

#define GM_ZONE(x) ((x)/GM_ZONE_LEN)

#define FOLLOW_DIST				(6)

//Flags Hex
//Static
const BYTE FH_BLOCK					=  BIN8(00000001); //������������
const BYTE FH_NOTRAKE				=  BIN8(00000010); //�����������������
const BYTE FH_WALL					=  BIN8(00000100); //�����
const BYTE FH_SCEN					=  BIN8(00001000); //��������
const BYTE FH_SCEN_GRID				=  BIN8(00010000); //������� ����-�������

//Dynamic
const BYTE FH_CRITTER		= BIN8(00000001); //�����, ���
const BYTE FH_DEAD_CRITTER	= BIN8(00000010); //�����, ���
const BYTE FH_ITEM			= BIN8(00000100); //������������ �������
const BYTE FH_DOOR			= BIN8(00001000); //������������ �����
const BYTE FH_BLOCK_ITEM	= BIN8(00010000); //������������ ������������ �������
const BYTE FH_NRAKE_ITEM	= BIN8(00100000); //������������ ����������������� �������

const WORD FH_NOWAY			=BIN16(00010001,00000001); //����� ��� ������������
const WORD FH_NOSHOOT		=BIN16(00100000,00000010); //����� ��� �����������������

//Client map
#define CLIENT_MAP_EXT			".fomap"
#define SERVER_MAP_EXT			".map"
#define CLIENT_MAP_FORMAT_VER	(0x1)
#define CLIENT_MAP_MAXNAME		(32)

//Coordinates
#define MAXHEXX					(400)
#define MAXHEXY					(400)
#define MAXTILEX				(MAXHEXX/2)
#define MAXTILEY				(MAXHEXY/2)

//defines
#define MAX_NAME				(12)
#define MIN_NAME				(4)
#define MAX_LOGIN				(10)
#define MIN_LOGIN				(4)
#define MAX_NET_TEXT			(100)

#define MAX_SCENERY				(5000)

#define MAX_DIALOG_TEXT			(MAX_FOTEXT)
#define MAX_DLG_LEN_IN_BYTES	(64 * 1024)

#define MAX_BUF_LEN				(4096)

//Crits
#define MAX_CRIT_TYPES			(103)

#define MAX_NPC					(99898)
#define NPC_MIN_ID				(101)
#define NPC_MAX_ID				(NPC_MIN_ID+MAX_NPC)
#define USERS_START_ID			(NPC_MAX_ID+2)

#define MAX_ANSWERS				100
#define TALK_MAX_TIME			60000
#define DIALOG_BARTER_TIME		90000
#define DIALOGS_LST_NAME		"dialogs.lst"
#define DIALOGS_PATH			".\\dialogs\\"

#define VARS_PATH				".\\vars\\"

#define MAX_SCRIPT_NAME			(64)

#define MAX_START_SPECIAL		(40)

const BYTE TALK_NPC_DISTANCE	=3;

#ifdef TEST_FAST_RESPAWN
const DWORD RESPOWN_TIME_PLAYER	=10000;
#else
const DWORD RESPOWN_TIME_PLAYER	=10*60000;
#endif
const DWORD RESPOWN_TIME_NPC	=120*60000;
const DWORD RESPOWN_TIME_INFINITY=4*24*60*60000;

//��������� ����� ��� ��������
#define AP_MS					(1200)

const DWORD ACTTIME_DROP_OBJ	=AP_MS;
const DWORD ACTTIME_USE_OBJ		=AP_MS*2;
const DWORD ACTTIME_USE_DOOR	=AP_MS*3;
const DWORD ACTTIME_USE_CONT	=AP_MS*3;
const DWORD ACTTIME_PICK_OBJ	=AP_MS*2;
const DWORD ACTTIME_RELOAD		=AP_MS*4;
const DWORD ACTTIME_REPAIR_OBJ	=AP_MS*10;
const DWORD ACTTIME_ACT_OBJ		=AP_MS*2;
const DWORD ACTTIME_UNACT_OBJ	=AP_MS*1;
const DWORD ACTTIME_SHOW_OBJ	=AP_MS*2;
const DWORD ACTTIME_HIDE_OBJ	=AP_MS*2;
const DWORD ACTTIME_USE_SKILL	=AP_MS*4;

//��������
const BYTE MOVE_ERROR		=0;
const BYTE MOVE_WALK		=1;
const BYTE MOVE_RUN			=2;

//move_params
const WORD MF_NEXTDIR_1		=BIN16(00000000,00000111);
const WORD MF_NEXTDIR_2		=BIN16(00000000,00111000);
const WORD MF_NEXTDIR_3		=BIN16(00000001,11000000);
const WORD MF_NEXTDIR_4		=BIN16(00001110,00000000);
const WORD MF_NEXTDIR_5		=BIN16(01110000,00000000);
const WORD MF_RUN			=BIN16(10000000,00000000);

//����� ������ ��� �������
const BYTE NPC_SAY_NONE		=0;
const BYTE NPC_SAY_ERROR	=1;
const BYTE NPC_SAY_IMBYSY	=2;
const BYTE NPC_SAY_HELLO	=3;
const BYTE NPC_SAY_IMHAPPY	=4;
const BYTE NPC_SAY_ILOVEYOU	=5;
const BYTE NPC_SAY_LEAVEME	=6;
const BYTE NPC_SAY_FUCKOFF	=7;
const BYTE NPC_SAY_IHATEYOU	=8;

//��������� ���������
const BYTE COND_LIFE				=1;//��������� �����
	const BYTE COND_LIFE_NONE			=1;//���������
	const BYTE COND_LIFE_ACTWEAP		=2;//���������
	const BYTE COND_LIFE_USEOBJ			=3;//���������
const BYTE COND_KNOCK_OUT			=2;//��������� �������
	const BYTE COND_KO_UP				=1;//��������� ������� ����� �����
	const BYTE COND_KO_DOWN				=2;//��������� ������� ���� �����
const BYTE COND_DEAD				=3;//��������� ������
	const BYTE COND_DEAD_UP				=1;//��������� ������
	const BYTE COND_DEAD_DOWN			=2;//��������� ������
	const BYTE COND_DEAD_MESS			=3;//��������� ������
	const BYTE COND_DEAD_BURN			=4;//��������� ������
	const BYTE COND_DEAD_HEAD			=5;//��������� ������
	const BYTE COND_DEAD_BRUST			=6;//��������� ������
	const BYTE COND_DEAD_PULSE			=7;//��������� ������
	const BYTE COND_DEAD_LASER			=8;//��������� ������
	const BYTE COND_DEAD_BURN2			=9;//��������� ������
	const BYTE COND_DEAD_PULSE2			=10;//��������� ������
	const BYTE COND_DEAD_EXPLODE		=11;//��������� ������
	const BYTE COND_DEAD_FUSED			=12;//��������� ������
	const BYTE COND_DEAD_BURN_RUN		=13;//��������� ������
const BYTE COND_NOT_IN_GAME			=4;//��������� ��� ����

//other
#define MIN_VISIBLE_CRIT		(6)
#define MIN_HIT					(5)
#define MAX_HIT					(95)

//����� ���������
#define FCRIT_PLAYER			BIN8(00000001)
#define FCRIT_NPC				BIN8(00000010)
#define FCRIT_MOB				BIN8(00000100)
#define FCRIT_DISCONNECT		BIN8(00001000)
#define FCRIT_CHOSEN			BIN8(00010000)
#define FCRIT_RULEGROUP			BIN8(00100000)

//�����
const BYTE SLOT_INV				=0;
const BYTE SLOT_HAND1			=1;
const BYTE SLOT_HAND2			=2;
const BYTE SLOT_ARMOR			=3;

//Body Types
//Slot protos offsets:
#define SLOT_MAIN_PROTO_OFFSET	(1000)
#define SLOT_EXT_PROTO_OFFSET	(1030)
#define SLOT_ARMOR_PROTO_OFFSET	(1060)
//1000 - 1100 protos reserved

enum EBodyType
{
	BT_Men			=0,
	BT_Women		=1,
	BT_Children		=2,
	BT_SuperMutants	=3,
	BT_Ghouls		=4,
	BT_Brahmin		=5,
	BT_Radscorpions	=6,
	BT_Rats			=7,
	BT_Floaters		=8,
	BT_Centaurs		=9,
	BT_Robots		=10,
	BT_Dogs			=11,
	BT_Manti		=12,
	BT_DeathClaws	=13,
	BT_Plants		=14,
	BT_Geckos		=15,
	BT_Aliens		=16,
	BT_GiantAnts	=17,
	BT_BigBadBoss	=18,
};

//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

//Params types
const BYTE TYPE_STAT				=1;
const BYTE TYPE_SKILL				=2;
const BYTE TYPE_PERK				=3;
const BYTE TYPE_VAR					=4; //For Npc, registers
const BYTE TYPE_KILL				=5;
const BYTE TYPE_LOYALITY			=6;
const BYTE TYPE_OTHER				=7; //Not allocated, only for net transfer

//Maximums
#define MAX_STATS					(63)
#define MAX_SKILLS					(18)
#define MAX_PERKS					(255)
#define MAX_SPECIAL					(7)
#define MAX_TRAITS					(16)
#define MAX_VARS					(20)
#define MAX_KILLS					(MAX_BODY_TYPES)
#define MAX_LOYALITY				(63)

/************************************************************************/
/* STATS                                                                */
/************************************************************************/
//Primary, special
#define ST_STRENGHT					0 //{����}
#define ST_PERCEPTION				1 //{����������}
#define ST_ENDURANCE				2 //{������������}
#define ST_CHARISMA					3 //{�������}
#define ST_INTELLECT				4 //{��}
#define ST_AGILITY					5 //{�����������}
#define ST_LUCK						6 //{�����}
//Secondary
#define ST_MAX_LIFE					7 //{����. ����� �����}
#define ST_MAX_COND					8 //{������������ ��������}
#define ST_ARMOR_CLASS				9 //{����� �����}
#define ST_MELEE_DAMAGE				10 //{���� �������������}
#define ST_WEAPON_DAMAGE			11 //{���� �������}
#define ST_CARRY_WEIGHT				12 //{����. ����}
#define ST_SEQUENCE					13 //{�������}
#define ST_HEALING_RATE				14 //{�������}
#define ST_CRITICAL_CHANCE			15 //{���� �����������.}
#define ST_MAX_CRITICAL				16 //{������������ �����������}
#define ST_INGURE_ABSORB			17 //{����� �������}
#define ST_LASER_ABSORB				18 //{����� ����������� �������}
#define ST_FIRE_ABSORB				19 //{����� ����������� �����}
#define ST_PLASMA_ABSORB			20 //{����� ����������� �������}
#define ST_ELECTRO_ABSORB			21 //{����� ����������� ��������������}
#define ST_EMP_ABSORB				22 //{����� ����������� EMP}
#define ST_BLAST_ABSORB				23 //{����� ����������� ��� ������}
#define ST_INGURE_RESIST			24 //{���������������� �������}
#define ST_LASER_RESIST				25 //{���������������� ������� �������}
#define ST_FIRE_RESIST				26 //{���������������� ������� �����}
#define ST_PLASMA_RESIST			27 //{���������������� ������� �������}
#define ST_ELECTRO_RESIST			28 //{���������������� ������� ��������������}
#define ST_EMP_RESIST				29 //{���������������� ������� EMP}
#define ST_BLAST_RESIST				30 //{���������������� ������� ��� ������}
#define ST_RADIATION_RESISTANCE		31 //{���������������� ��������}
#define ST_POISON_RESISTANCE		32 //{���������������� ����}
#define ST_AGE						33 //{�������}
#define ST_GENDER					34 //{���}
#define ST_CURRENT_HP				35 //{������� ����� �����}
#define ST_POISONING_LEVEL			36 //{������� ������� ����}
#define ST_RADIATION_LEVEL			37 //{������� ������� ��������}
#define ST_CURRENT_STANDART			38 //{������� ��������}
#define ST_EXPERIENCE				39 //{�����}
#define ST_LEVEL					40 //{�������}
#define ST_UNSPENT_SKILL_POINTS		41 //{�������� ����� �������}
#define ST_UNSPENT_PERKS			42 //{�������� �� ������ ������}
#define ST_KARMA					43 //{�����}

#define ST_RESERVED2				44 //{reserved}
#define ST_RESERVED3				45 //{reserved}
#define ST_RESERVED4				46 //{reserved}
#define ST_RESERVED5				47 //{reserved}
#define ST_RESERVED6				48 //{reserved}
#define ST_RESERVED7				49 //{reserved}
#define ST_RESERVED8				50 //{reserved}
#define ST_RESERVED9				51 //{reserved}
#define ST_RESERVED10				52 //{reserved}

#define ST_SCRIPT_VAR0				53 //{Script var0}
#define ST_SCRIPT_VAR1				54 //{Script var1}
#define ST_SCRIPT_VAR2				55 //{Script var2}
#define ST_SCRIPT_VAR3				56 //{Script var3}
#define ST_SCRIPT_VAR4				57 //{Script var4}
#define ST_SCRIPT_VAR5				58 //{Script var5}
#define ST_SCRIPT_VAR6				59 //{Script var6}
#define ST_SCRIPT_VAR7				60 //{Script var7}
#define ST_SCRIPT_VAR8				61 //{Script var8}
#define ST_SCRIPT_VAR9				62 //{Script var9}

const int ShowStats[]={9,12,10,24,32,31,14,15,37,36};
const int ShowStatsCnt=sizeof(ShowStats)/sizeof(ShowStats[0]);
// 9  ac
// 12 max w
// 10 melee
// 24 dmg norm
// 32 dmg poison
// 31 dmg rad
// 14 hp lvl
// 15 critical
// 37 poison level
// 36 rad level

/************************************************************************/
/* SKILLS                                                               */
/************************************************************************/
#define SK_SMALL_GUNS				0 //{������ ������}
#define SK_BIG_GUNS					1 //{������� ������}
#define SK_ENERGY_WEAPONS			2 //{�������������� ������}
#define SK_UNARMED					3 //{�� ��������}
#define SK_MELEE_WEAPONS			4 //{�������� ������}
#define SK_THROWING					5 //{������}
#define SK_FIRST_AID				6 //{������}
#define SK_DOCTOR					7 //{����}
#define SK_SNEAK					8 //{����������}
#define SK_LOCKPICK					9 //{��������}
#define SK_STEAL					10 //{������}
#define SK_TRAPS					11 //{�������}
#define SK_SCIENCE					12 //{�����}
#define SK_REPAIR					13 //{������}
#define SK_SPEECH					14 //{����}
#define SK_BARTER					15 //{�����} ��������� �� ���
#define SK_GAMBLING					16 //{������} ��������� �� ���
#define SK_OUTDOORSMAN				17 //{������ �������� ����}


#define MAX_SKILL_VAL		300

#define FIRST_AID_TIME_OUT(tmul)	(24*60*60*1000/tmul/3)
#define DOCTOR_TIME_OUT(tmul)		(24*60*60*1000/tmul/1)
#define REPAIR_TIME_OUT(tmul)		(24*60*60*1000/tmul/4)
#define SCIENCE_TIME_OUT(tmul)		(24*60*60*1000/tmul/6)
#define LOCKPICK_TIME_OUT(tmul)		(   15*60*1000/tmul)
//#define STEAL_TIME_OUT(tmul)		(24*60*60*1000/tmul/6)
//#define TRAPS_TIME_OUT(tmul)		(24*60*60*1000/tmul/6)

/************************************************************************/
/* PERKS                                                                */
/************************************************************************/
//Traits
//***************************************************** Fast Metabolism *** DONE
	// Radiation Resist = 0 on start game
	// Poison Resist = 0 on start game
	// Healing rate +2
//***************************************************** Bruiser *********** TODO
//***************************************************** Small Frame ******* DONE
	// - 10*Strength on carry weight
	// + 1 Agility
//***************************************************** One Hander ******** DONE
	// Twohand weapon hit -40%
	// Onehand weapon hit +20%
	// Hint: unarmed is not weapon
//***************************************************** Finesse *********** DONE
	// Critical chance +10
	// Damage -30%
//***************************************************** Kamikaze ********** TODO
//***************************************************** Heavy Handed ****** DONE
//***************************************************** Fast Shot ********* TODO
//***************************************************** Bloody Mess ******* DONE
	// Every dead is like critical
//***************************************************** Jinxed ************ TODO
//***************************************************** Good Natured ****** DONE
	// - 60% on attack skills
	// + 60% on non-attack skills
//***************************************************** Chem Reliant ****** TODO
//***************************************************** Chem Resistant **** TODO
//***************************************************** Sex Appeal ******** DONE
	// In dialogs
//***************************************************** Skilled *********** DONE
	// +5 skill points per level
	// Perk per 4 level
//***************************************************** Gifted ************ DONE
	// -5 skill points per level
	// +1 for all special
	// -10 for all skills
//******************************************************************************
	//Left
#define PE_FAST_METABOLISM			0 //{����� �������}
#define PE_BRUISER					1 //{�����}
#define PE_SMALL_FRAME				2 //{���������}
#define PE_ONE_HANDER				3 //{���������}
#define PE_FINESSE					4 //{��������}
#define PE_KAMIKAZE					5 //{���������}
#define PE_HEAVY_HANDED				6 //{������� ����}
#define PE_FAST_SHOT				7 //{������� �������}
	//Right
#define PE_BLOODY_MESS				8 //{�������� ������}
#define PE_JINXED					9 //{������������}
#define PE_GOOD_NATURED				10 //{�����������}
#define PE_CHEM_RELIANT				11 //{����������}
#define PE_CHEM_RESISTANT			12 //{����������}
#define PE_NIGHT_PERSON				13 //{��������������}  # was: Night Person
#define PE_SKILLED					14 //{������}
#define PE_GIFTED					15 //{���������}
//Perks
#define PE_AWARENESS				16 //{���������������}
#define PE_A_MELEE_ATT				17 //{���. ���������� �����}
#define PE_A_MELEE_DAM				18 //{���. ���� � ����������}
#define PE_A_MOVE					19 //{���. ��������}
#define PE_A_DAM					20 //{���. �����������}
#define PE_A_SPEED					21 //{���. �������� ���������}
#define PE_PASS_FRONT				22 //{���������� ������}
#define PE_RAPID_HEAL				23 //{������� �������}
#define PE_MORE_CRIT_DAM			24 //{������ ����������� ���������}
#define PE_NIGHT_SIGHT				25 //{������ �������}
#define PE_PRESENCE					26 //{�����������}
#define PE_RES_NUKLEAR				27 //{������������� ��������}
#define PE_ENDURENCE				28 //{������������}
#define PE_STR_BACK					29 //{������� �����}
#define PE_MARKSMAN					30 //{������ �������}
#define PE_STEALHING				31 //{��������� ���}
#define PE_LIFEFULL					32 //{�������}
#define PE_MERCHANT					33 //{������ ��������}
#define PE_FORMED					34 //{������������}
#define PE_HEALER					35 //{������}
#define PE_TR_DIGGER				36 //{�������������}
#define PE_BEST_HITS				37 //{������ �����}
#define PE_COMPASION				38 //{����������}
#define PE_KILLER					39 //{������}
#define PE_SNIPER					40 //{�������}
#define PE_SILENT_DEATH				41 //{���������� ������}
#define PE_C_FIGHTER				42 //{��������� ����}
#define PE_MIND_BLOCK				43 //{���������� ��������}
#define PE_PROLONGATION_LIFE		44 //{��������� �����}
#define PE_RECOURCEFULNESS			45 //{��������������}
#define PE_SNAKE_EATER				46 //{���������� ����}
#define PE_REPEARER					47 //{���������}
#define PE_MEDIC					48 //{�����}
#define PE_SKILLED_THIEF			49 //{������ ���}
#define PE_SPEAKER					50 //{������}
#define PE_GUTCHER					51 //{�����!}
#define PE_FRIENDLY_FOE				52 //{}
#define PE_PICK_POCKER				53 //{���������}
#define PE_GHOST					54 //{�������}
#define PE_CHAR_CULT				55 //{����� ��������}
#define PE_THIFER					56 //{�������}
#define PE_DISCOVER					57 //{�������������}
#define PE_THE_PURETY				58 //{���� �������}
#define PE_OVERROAD					59 //{���������}
#define PE_ANIMAL_FRIENDSHIP		60 //{���� ��������}
#define PE_SCOUT					61 //{�����}
#define PE_MIST_CHAR				62 //{������������ ����������}
#define PE_RANGER					63 //{��������}
#define PE_PICK_POCKET_2			64 //{���������}
#define PE_INTERLOCUTER				65 //{����������}
#define PE_NOVICE					66 //{��������� ������}
#define PE_PRIME_SKILL				67 //{�������� ������!}
#define PE_MUTATION					68 //{�������!}
#define PE_NARC_NUKACOLA			69 //{����������� � NukaCola}			DRUG EFFECT
#define PE_NARC_BUFFOUT				70 //{����������� � �������}			DRUG EFFECT
#define PE_NARC_MENTAT				71 //{����������� � �������}			DRUG EFFECT
#define PE_NARC_PSYHO				72 //{����������� � �����}				DRUG EFFECT
#define PE_NARC_RADAWAY				73 //{����������� �������}				DRUG EFFECT
#define PE_DISTANT_WEAP				74 //{������������ ������}
#define PE_ACCURARY_WEAP			75 //{������ ������}
#define PE_PENETRATION_WEAP			76 //{����������� ������}
#define PE_KILLER_WEAP				77 //{������� ������}
#define PE_ENERGY_ARMOR				78 //{�������������� �����}
#define PE_BATTLE_ARMOR				79 //{������ �����}
#define PE_WEAP_RANGE				80 //{��������������}
#define PE_RAPID_RELOAD				81 //{������� �����������}
#define PE_NIGHT_SPYGLASS			82 //{������ � ������ ��������}
#define PE_FLAMER					83 //{�������}
#define PE_APA_I					84 //{���������� ����� I}
#define PE_APA_II					85 //{���������� ����� II}
#define PE_FORCEAGE					86 //{������ ��������}
#define PE_DEADLY_NARC				87 //{����������� �����������}			DRUG EFFECT
#define PE_CHARMOLEANCE				88 //{������� }
#define PE_GEKK_SKINER				89 //{������� �����}
#define PE_SKIN_ARMOR				90 //{����-�����}
#define PE_A_SKIN_ARMOR				91 //{��������� ����- �����.}
#define PE_SUPER_ARMOR				92 //{����� �����}
#define PE_A_SUPER_ARMOR			93 //{��������� ����� �����}
#define PE_VAULT_INOCUL				94 //{�������� �����}
#define PE_ADRENALIN_RUSH			95 //{������ ����������}
#define PE_CAREFULL					96 //{������������}
#define PE_INTELEGENCE				97 //{���������}
#define PE_PYROKASY					98 //{���������}
#define PE_DUDE						99 //{�����}
//==========
#define PE_A_STR					100 //{������ ����}
#define PE_A_PER					101 //{������ ����������}
#define PE_A_END					102 //{������ ������������}
#define PE_A_CHA					103 //{������ �������}
#define PE_A_INT					104 //{������ ���}
#define PE_A_AGL					105 //{������ ��������}
#define PE_A_LUC					106 //{������ �����}
//===========
#define PE_PURERER					107 //{���� ����������}
#define PE_IMAG						108 //{����������}
#define PE_EVASION					109 //{���������}
#define PE_DROSHKADRAT				110 //{���������� �� ����- �����}
#define PE_KARMA_GLOW				111 //{�������� �����}
#define PE_SILENT_STEPS				112 //{������ ����}
#define PE_ANATOMY					113 //{��������}
#define PE_CHAMER					114 //{�������������� ��������}
#define PE_ORATOR					115 //{������}
#define PE_PACKER					116 //{���������}
#define PE_EDD_GAYAN_MANIAC			117 //{������- ������}
#define PE_FAST_REGENERATION		118 //{������� �������������}
#define PE_VENDOR					119 //{��������}
#define PE_STONE_WALL				120 //{�������� �����}
#define PE_THIEF_AGAIN				121 //{���}
#define PE_WEAPON_SKILL				122 //{�������� �������}
#define PE_MAKE_VAULT				123 //{���������� �����}
#define PE_ALC_BUFF_1				124 //{�������� �������� ����� �����}
#define PE_ALC_BUFF_2				125 //{�������� �������� ����� �����}
#define PE_ALC_RAIS_1				126 //{�������� �������� ����� �����}
#define PE_ALC_RAIS_2				127 //{�������� �������� ����� ����� II}
#define PE_AUTODOC_BUFF_1			128 //{������� ������� ����� �����}
#define PE_AUTODOC_BUFF_2			129 //{������� ������� ����� ����� II}
#define PE_AUTODOC_RAIS_1			130 //{������� ������� ����� �����}
#define PE_AUTODOC_RAIS_2			131 //{������� ������� ����� ����� II}
#define PE_EXCREMENT_EXPEDITOR		132 //{��������� ��������}
#define PE_WEAPON_ENHANCED			133 //{������ ��������� ������� ����}
#define PE_UNLUCK					134 //{������������}
#define PE_BOOKWORM					135 //{�������� (������� �����)}

//Damages
#define PE_DAMAGE_POISONED			200 //{��������}
#define PE_DAMAGE_RADIATED			201 //{�������}
#define PE_DAMAGE_EYE				202 //{���������� �����}
#define PE_DAMAGE_RIGHT_ARM			203 //{���������� ������ ����}
#define PE_DAMAGE_LEFT_ARM			204 //{���������� ����� ����}
#define PE_DAMAGE_RIGHT_LEG			205 //{���������� ������ ����}
#define PE_DAMAGE_LEFT_LEG			206 //{���������� ����� ����}
#define PE_DMGRESERVED1				207 //{damage reserved}
#define PE_DMGRESERVED2				208 //{damage reserved}
#define PE_DMGRESERVED3				209 //{damage reserved}

//Modes
#define PE_TAG_SKILL1				210 //{������������� ����������� 1}
#define PE_TAG_SKILL2				211 //{������������� ����������� 2}
#define PE_TAG_SKILL3				212 //{������������� ����������� 3}
#define PE_HIDE_MODE				215 //{��������� ���������� (����/���)}

const int TRAIT_BEGIN				=0;
const int TRAIT_END					=15;
const int DAMAGE_BEGIN				=200;
const int DAMAGE_END				=206;
const int PERK_BEGIN				=16;
const int PERK_END					=135;
const int PERK_COUNT				=PERK_END-PERK_BEGIN;
//const int NARC_BEGIN				=;
//const int NARC_END					=;

/************************************************************************/
/* BODY TYPES && KILLS                                                  */
/************************************************************************/
#define BT_MEN						(0)
#define BT_WOMEN					(1)
#define BT_CHILDREN					(2)
#define BT_SUPER_MUTANT				(3)
#define BT_GHOUL					(4)
#define BT_BRAHMIN					(5)
#define BT_RADSCORPION				(6)
#define BT_RAT						(7)
#define BT_FLOATER					(8)
#define BT_CENTAUR					(9)
#define BT_ROBOT					(10)
#define BT_DOG						(11)
#define BT_MANTI					(12)
#define BT_DEADCLAW					(13)
#define BT_PLANT					(14)
#define BT_GECKO					(15)
#define BT_ALIEN					(16)
#define BT_GIANT_ANT				(17)
#define BT_BIG_BAD_BOSS				(18)


#define MAX_BODY_TYPES				(19)

/************************************************************************/
/* LOYALITY                                                             */
/************************************************************************/
#define LOYALITY_VIS_2_33			0
#define LOYALITY_VIS_34_62			1
#define LOYALITY_DEN				2
#define LOYALITY_KLAMATH			3
#define LOYALITY_MODOC				4
#define LOYALITY_VAULT_CITY			5
#define LOYALITY_GECKO				6
#define LOYALITY_NEW_RENO			7
#define LOYALITY_NCR				8
#define LOYALITY_REDDING			9
#define LOYALITY_SF					10
#define LOYALITY_NAVARRO			11

/************************************************************************/
/* OTHER                                                                */
/************************************************************************/
//#define OTHER_MAP_TIMEOUT			0


//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

const BYTE ACT_NULL				=1;
const BYTE ACT_SHOW_OBJ			=2;
const BYTE ACT_HIDE_OBJ			=3;
const BYTE ACT_ACTIVATE_OBJ		=4;
const BYTE ACT_DACTIVATE_OBJ	=5;
const BYTE ACT_USE_OBJ			=6;
const BYTE ACT_DEFEAT			=8;
	const BYTE ACT_DEFEAT_MISS		=1;
	const BYTE ACT_DEFEAT_FRONT		=2;
	const BYTE ACT_DEFEAT_REAR		=3;
	const BYTE ACT_DEFEAT_KO_FRONT	=4;
	const BYTE ACT_DEFEAT_KO_REAR	=5;
const BYTE ACT_REFRESH			=9;
const BYTE ACT_DEAD				=10;
const BYTE ACT_DISCONNECT		=12;
const BYTE ACT_DROP_OBJ			=13;
const BYTE ACT_PICK_UP			=14;
const BYTE ACT_PICK_DOWN		=15;
const BYTE ACT_FUN				=16;
const BYTE ACT_RESPAWN			=17;
const BYTE ACT_USE_SKILL		=19;

//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

struct ContainerObj 
{
	DWORD id;
	WORD pid;
	DWORD count;

	bool operator==(const DWORD& _id){return (this->id==_id);}
//	bool operator==(const WORD& _pid){return (this->pid==_pid);}

	ContainerObj():id(0),pid(0),count(0){}
	ContainerObj(DWORD _id, WORD _pid, DWORD _count):id(_id),pid(_pid),count(_count){}
};

typedef vector<ContainerObj> ContObjVec;
typedef vector<ContainerObj>::iterator ContObjVecIt;

//******************************************************************************************
//******************************************************************************************
//******************************************************************************************

#endif //__FODEFINES__H__