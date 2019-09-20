#ifndef __FOSERV_H__
#define __FOSERV_H__

/********************************************************************
	created:	18:08:2004   23:48; updated: begin of 2007

	author:		Oleg Mareskin
	add/edit:	Anton Tsvetinsky
	
	purpose:	
*********************************************************************/
#include "stdafx.h"

#include "main.h"

#include "math.h"

#include "zlib\\zlib.h"
#include "netproto.h"
#include "BufMngr.h"

//!Cvet +++++++++++++++++++++++++++++++++++++++++
#include "sql.h"

#include "MapMngr.h"


#define PATH_OBJECTS ".\\objects\\"
#define PATH_NPC ".\\npc\\"

//макросы для работы с параметрами
#define CHANGE_STAT(client,stat,oper,count) {client##->info.st[(stat)]##oper##(count);\
if(client##->info.st[(stat)]>9999) client##->info.st[(stat)]=9999;\
if(FLAG(client##->info.flags,FCRIT_PLAYER)&&!FLAG(client##->info.flags,FCRIT_DISCONNECT))\
Send_Param(client,TYPE_STAT,(stat));}
#define CHANGE_SKILL(client,skill,oper,count) {client##->info.sk[(skill)]##oper##(count);\
if(client##->info.sk[(skill)]>999) client##->info.sk[(skill)]=999;\
if(FLAG(client##->info.flags,FCRIT_PLAYER)&&!FLAG(client##->info.flags,FCRIT_DISCONNECT))\
Send_Param(client,TYPE_SKILL,(skill));}
#define CHANGE_PERK(client,perk,oper,count) {client##->info.pe[(perk)]##oper##(count);\
if(client##->info.pe[(perk)]>9) client##->info.pe[(perk)]=9;\
if(FLAG(client##->info.flags,FCRIT_PLAYER)&&!FLAG(client##->info.flags,FCRIT_DISCONNECT))\
Send_Param(client,TYPE_PERK,(perk));}

//дистанция
#define DISTANCE(cl1,cl2) (sqrt(((cl1##->info.x-cl2##->info.x)*(cl1##->info.x-cl2##->info.x))+\
((cl1##->info.y-cl2##->info.y)*(cl1##->info.y-cl2##->info.y))))


#define TALK_MAX_TIME 60000

const BYTE NOT_ANSWER_CLOSE_DIALOG	=0;
const BYTE NOT_ANSWER_BEGIN_BATTLE	=1;

//Условие
const DEMAND_NONE		=0;
const DEMAND_STAT		=1;
const DEMAND_SKILL		=2;
const DEMAND_PERK		=3;
const DEMAND_QUEST		=4;
const DEMAND_ITEM		=5;
const DEMAND_VAR		=6;

struct demand
{
	demand():type(DEMAND_NONE),param(0),oper('>'),count(0){};

	BYTE type;

	BYTE param;
	string var_name;
	char oper;
	int count;
};

typedef vector<demand*> demand_list;

//Результат
const RESULT_NONE		=0;
const RESULT_STAT		=1;
const RESULT_SKILL		=2;
const RESULT_PERK		=3;
const RESULT_QUEST		=4;
const RESULT_ITEM		=5;
const RESULT_VAR		=6;
const RESULT_LOCK		=7;

struct result
{
	result():type(RESULT_NONE),param(0),oper('+'),count(0){};

	BYTE type;

	BYTE param;
	string var_name;
	char oper;
	int count;
};

typedef vector<result*> result_list;

struct answer
{
	answer():link(0),id_text(0){};
//	~answer(){SAFEDELA(demands);SAFEDELA(results);};

	DWORD link;
	DWORD id_text;

	demand_list demands;

	result_list results;
};

typedef vector<answer*> answers_list;

struct npc_dialog
{
	npc_dialog():id(0),id_text(0),time_break(TALK_MAX_TIME),not_answer(NOT_ANSWER_CLOSE_DIALOG){};
//	~npc_dialog(){SAFEDELA(answers);};

	DWORD id;

	DWORD id_text;
	answers_list answers;

	UINT time_break; //время на прочтение диалога игроком
	BYTE not_answer; //что делать если нет ответа
};

typedef map<DWORD, npc_dialog*, less<DWORD> > dialogs_map;

struct npc_info
{
	npc_info():talking(0){};
	//~npc_info(){SAFEDELA(dialogs);};

//ид игрока с которым он сейчас разговаривает
	CrID talking;
//диалоги
	dialogs_map dialogs;
//текущий скампонованный диалог
	npc_dialog compiled_dialog;
};

//!Cvet -------------------------------------------

struct CLIENT
{
	SOCKET s; // Socket id
	SOCKADDR_IN from;

	CBufMngr bin; // буфер входящий
	CBufMngr bout; // буфер исходящий

	WORD state; // статус
	z_stream zstrm; // поток
	crit_info info; // Инфа криттера
	npc_info* i_npc; //!Cvet инфа НПЦ

	CLIENT() {s=-1;} // constructor initial
	~CLIENT() {SAFEDEL(i_npc);} // destructor
};

typedef map<DWORD, CLIENT*, less<DWORD> > cl_map;

class CServer
{
	bool Active; // включен сервер

	cl_map cl; //!Cvet Карта клиентов
	cl_map pc; //!Cvet Карта НПЦ
	cl_map cr; //!Cvet Карта всех криттеров
	cl_map map_cr[MAX_MAPS]; //!Cvet Карта криттеров по картам

	SOCKET s; // socket
	fd_set read_set,write_set,exc_set;

	void ClearClients(); //Del all clients
//	void RemoveClient(CrID id); //Del client by id

	void RemoveCritter(CrID id);
	void DisconnectClient(CrID idchannel); //!Cvet Disconnect client by id
	int ConnectClient(SOCKET serv); //add new client
	
	int Input(CLIENT* acl);
	int Output(CLIENT* acl);
	void Process(CLIENT* acl);

//	void Process_GetName(CLIENT* acl); !Cvet убрал
	void Process_CreateClient(CLIENT* acl); //!Cvet
	void Process_GetLogIn(CLIENT* acl); //!Cvet
	void Process_GetText(CLIENT* acl);
	void Process_Rotate(CLIENT* acl);
	void Process_Move(CLIENT* acl);

	void Process_ChangeObject(CLIENT* acl);
	void Process_UseObject(CLIENT* acl); //!Cvet заявка на использование объекта
	void Process_UseSkill(CLIENT* acl); //!Cvet заявка на использование скилла

	void Process_Talk_NPC(CLIENT* acl); //!Cvet разговор с НПЦ

	void Process_GetTime(CLIENT* acl); //!Cvet

	void Process_MapLoaded(CLIENT* acl);

	void Send_AddCritter(CLIENT* acl,crit_info* pinfo); // посылка Критера
//!Cvet +++++++++++++++++++++++++++++++++++++++++++++
	void Send_LoginMsg(CLIENT* acl, BYTE LogMsg);
	void Send_RemoveCritter(CLIENT* acl,CrID id);

	void Send_XY(CLIENT* acl); //посылка точки которую должен принять игрок

	void Send_Move(CLIENT* acl, BYTE move_params);
	void Send_Action(CLIENT* acl, BYTE num_action, BYTE rate_object);

	void Send_AddObjOnMap(CLIENT* acl, dyn_obj* o);
	void Send_RemObjFromMap(CLIENT* acl, dyn_obj* o);
	void Send_AddObject(CLIENT* acl, dyn_obj* send_obj);
	void Send_RemObject(CLIENT* acl, dyn_obj* send_obj);
	void Send_WearObject(CLIENT* acl, dyn_obj* send_obj);

	void Send_AllParams(CLIENT* acl, BYTE type_param);
	void Send_Param(CLIENT* acl, BYTE type_param, BYTE num_param);

	void Send_Talk(CLIENT* acl, npc_dialog* send_dialog);

	void Send_LoadMap(CLIENT* acl);
	void Send_Map(CLIENT* acl, WORD map_num);

	void SetVisibleCl(CLIENT* acl);
	void SetVisibleObj(CLIENT* acl);

	inline int AddClIntoListId(CLIENT* acl, CrID add_id);
	inline int DelClFromListId(CLIENT* acl, CrID del_id);
	void DelClFromAllListId(CrID del_id, WORD num_map);
	inline int AddObjIntoListInd(CLIENT* acl, DWORD add_ind);
	inline int DelObjFromListInd(CLIENT* acl, DWORD del_ind);

	void GenParam(CLIENT* acl); //генерация параметров игрока s[7]..s[38]
	void GenLook(CLIENT* acl); //установим область видимости
	void GenWear(dyn_obj* wear_obj, bool use_obj); //установим область видимости

	CMapMngr mm;

	stat_map all_s_obj; //вся статика в игре
	dyn_map all_obj; //вся динамика объектов в игре

	list_id::iterator it_li; //итератор для списков list_id

	int LoadAllStaticObjects(); //загрузка статичкских объектов

//Объекты динамика
	int LoadAllObj(); //загрузка динамических объектов из mySQL
	void SaveAllObj(); //запись динамических объектов в mySQL

	void DeleteObj(DWORD id_obj); //удаление объекта
	void CreateObjToPl(CrID c_pl_idchannel, WORD num_st_obj); //создание объекта на тайле
	void CreateObjToTile(MapTYPE c_map, HexTYPE c_x, HexTYPE c_y, WORD num_st_obj); //создание объекта у игрока
	//обмен объектами
	void TransferDynObjPlPl(CLIENT* from_acl, CLIENT* to_acl, DWORD id_obj);
	void TransferDynObjTlPl(tile_info* from_tile, CLIENT* to_acl, DWORD id_obj);
	void TransferDynObjPlTl(CLIENT* from_acl, tile_info* to_tile, DWORD id_obj);

//Игроки
	int LoadAllPlayers(); //загрузка всех клиентов (при запуске сервера)
	void SaveAllDataPlayers(); //сохранение всех данных всех игроков

//Скрипты
//	void Scr_SkillUD(BYTE act, CLIENT* acl, CrID target_id);
	int Act_Attack(CLIENT* acl, BYTE rate_object, CrID target_id);
//	void Scr_Reload(BYTE act, CLIENT* acl);

//НПЦ
	int NPC_LoadAll();
	void NPC_Remove(CLIENT* npc);
	void NPC_Process(CLIENT* npc);

	void NPC_Dialog_Close(CLIENT* npc, CLIENT* acl, BYTE onhead_say);
	int  NPC_Dialog_Compile(CLIENT* npc, CLIENT* acl, npc_dialog* new_dlg);
	int NPC_Check_demand(CLIENT* npc, CLIENT* acl, answer* check_answ);
	void NPC_Use_result(CLIENT* npc, CLIENT* acl, answer* use_answ);

//Слежение за читерством
	void SetCheat(CLIENT* acl, char* cheat_message);

//Игровое время
	SYSTEMTIME sys_time;
	WORD Game_Time;
	BYTE Game_Day;
	BYTE Game_Month;
	WORD Game_Year;

	void CreateParamsMaps();
	params_map stats_map;
	params_map skills_map;
	params_map perks_map;
	params_map object_map;
//!Cvet ---------------------------------------------

	int GetCmdNum(char* cmd);

	char* GetParam(char* cmdstr,char** next);
	char* my_strlwr(char* str);
	char* my_strupr(char* str);
	int PartialRight(char* str,char* et);
	char* MakeName(char* str,char* str2);

	void ProcessSocial(CLIENT* sender,WORD socid,char* aparam);
	CLIENT* LocateByPartName(char* name);

	char inBUF[2048];
	char *outBUF;
	DWORD outLEN;

//	DWORD cur_obj_id; //!Cvet
	// Для того чтобы исключить повторный сигнал о подключение уже
	// присутствующего игрока на сервере, ид посл-го подключенного.
	CrID last_id;

	SQL sql; //!Cvet

	CFileMngr fm; //!Cvet

public:

//!Cvet запись времени выполнения +++
	int loop_time;
	int loop_cycles;
	int loop_min;
	int loop_max;
	
	int lt_FDsel;
	int lt_conn;
	int lt_input;
	int lt_proc_cl;
	int lt_proc_pc;
	int lt_output;
	int lt_discon;

	int lt_FDsel_max;
	int lt_conn_max;
	int lt_input_max;
	int lt_proc_cl_max;
	int lt_proc_pc_max;
	int lt_output_max;
	int lt_discon_max;

	int lags_count;
//!Cvet ---

	int Init();
	void Finish();

	void RunGameLoop(); // серверная петля

	 CServer();
	 ~CServer();
};


#endif //__FOSERV_H__