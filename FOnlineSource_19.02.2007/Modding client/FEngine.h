#ifndef _FENGINE_H_
#define _FENGINE_H_

#include "CSpriteManager.h"
#include "SoundMngr.h"
#include "CHexField.h"
#include "Critter.h"
#include "CFont.h"
#include "netproto.h"

#include "BufMngr.h"

#define DI_BUF_SIZE 64

#define DI_ONDOWN(a,b) if((didod[ i ].dwOfs==a) && (didod[ i ].dwData & 0x80)) {b;}
#define DI_ONUP(a,b) if((didod[ i ].dwOfs==a) && !(didod[ i ].dwData & 0x80)) {b;}
#define DI_ONMOUSE(a,b) if(didod[ i ].dwOfs==a) {b;}

#define EDIT_LEN 94

#define MAX_MESS 500
#define MAX_MESS_IN_MESSBOX 15
#define MAX_MESSBOX EDIT_LEN*MAX_MESS_IN_MESSBOX

#define MAX_DIALOG_TEXT 4000

//!Cvet режимы экрана
const BYTE SCREEN_LOGIN			=0;
const BYTE SCREEN_REGISTRATION	=1;
const BYTE SCREEN_MAIN			=2;
const BYTE SCREEN_INVENTORY		=3;
const BYTE SCREEN_LOCAL_MAP		=4;
const BYTE SCREEN_GLOBAL_MAP	=5;
const BYTE SCREEN_DIALOG_NPC	=6;
const BYTE SCREEN_PIP_BOY		=7;

//!Cvet режимы курсора
const BYTE CUR_DEFAULT			=0;
const BYTE CUR_MOVE				=1;
const BYTE CUR_USE_OBJECT		=2;
const BYTE CUR_USE_SKILL		=3;
const BYTE CUR_WAIT				=4;

//!Cvet LMenu
const DWORD LMENU_SHOW_TIME		=300;
//режимы
const BYTE LMENU_OFF			=0;
const BYTE LMENU_PLAYER			=1;
const BYTE LMENU_NPC			=2;
const BYTE LMENU_ITEM			=3;
const BYTE LMENU_SCENERY		=4;
//ячейки для lmenu
typedef vector<BYTE> LMenu_list;
const BYTE LMENU_NODE_LOOK		=0;
const BYTE LMENU_NODE_TALK		=1;
const BYTE LMENU_NODE_BREAK		=2;

//!Cvet действия чезена
const BYTE ACTION_NONE					=0;
const BYTE ACTION_MOVE					=1;
const BYTE ACTION_SHOW_OBJECT			=2;
const BYTE ACTION_HIDE_OBJECT			=3;
const BYTE ACTION_ACTIVATE_OBJECT		=4;
const BYTE ACTION_DACTIVATE_OBJECT		=5;
const BYTE ACTION_USE_OBJ_ON_CRITTER	=6;
const BYTE ACTION_USE_OBJ_ON_ITEM		=7;
const BYTE ACTION_USE_SKL_ON_CRITTER	=8;
const BYTE ACTION_USE_SKL_ON_ITEM		=9;
const BYTE ACTION_TALK_NPC				=10;

// Анимация обычная набор спрайтов
struct AnyAnimData 
{
		AnyFrames* eng; // Анимация берем оружие в руки
		AnyAnimData():eng(NULL){};
		~AnyAnimData(){SAFEDEL(eng)};
};

class CFEngine
{
public:
	int Init(HWND _hWnd);
	void Clear();
	void ClearCritters(); //!Cvet
	void RemoveCritter(CrID remid); //!Cvet
	void Restore();
	void RestoreDI();
    
	int Render();

	int Worldmap();// глобальная карта игрока 0 уровень
	int Console();// консоль игрока

	void NetDiscon();
	WORD state;

	CFEngine();
private:
	bool crtd;
	bool islost;
	bool dilost;

	bool CtrlDwn;
	bool AltDwn; //!Cvet
	bool ShiftDwn;
	bool edit_mode;

	WORD cur_hold;
//!Cvet +++++++++++++++++++++++++++++++
	BYTE screen_mode;
	void SetScreen(BYTE new_screen);
	void SetScreenCastling(BYTE mode1, BYTE mode2){ if		(screen_mode==mode1) screen_mode=mode2; 
													else  if(screen_mode==mode2) screen_mode=mode1; }
	BOOL IsScreen(BYTE check_screen){ if(check_screen==screen_mode) return 1; return 0; };

	BYTE cur_mode;
	void SetCur(BYTE new_cur);
	void SetCurCastling(BYTE cur1, BYTE cur2){	if	   (cur_mode==cur1) cur_mode=cur2; 
												else if(cur_mode==cur2) cur_mode=cur1; }
	BOOL IsCur(BYTE check_cur){ if(check_cur==cur_mode) return 1; return 0; };
//!Cvet -------------------------------
	BYTE lang;
	char ed_str[EDIT_LEN+1];
	int cur_edit; //!Cvet

	HWND hWnd;

	LPDIRECT3D8 lpD3D;
	LPDIRECT3DDEVICE8 lpDevice;

	LPDIRECTINPUT8 lpDInput;
	LPDIRECTINPUTDEVICE8 lpKeyboard;
	LPDIRECTINPUTDEVICE8 lpMouse;

	CSpriteManager sm;
	CHexField hf;
	CFOFont fnt;
	CSoundMngr sdm;

	WORD cur_def,cur_move,cur_move_block,cur_hand,cur_use_o,cur_use_s,cur_wait; //!Cvet указатели

	WORD splash,cur,cur_right,cur_left,cur_up,cur_down,cur_ru,cur_lu,cur_rd,cur_ld;
	int cur_x,cur_y;
	WORD alet;
    // дополнитиельно загрузим пару картинок для инвентаря и т.д.
	WORD actdn,actup,ifacen,worldmap1,worldmap2,worldmap3,worldmap4,Arroyo,
		Junk,Bos,Vault13,Redding,Staff,Statue2,Flagncr,Pole,Enclave,Flagencl,Navarro1,Navarro2,
		Klamath,Gecko,Vtcity,Epa,Modoc,Den,Reno1,Reno2,Reno3,Reno4,Reno5,Reno6,Mbase1,Mbase2,
		Sfchina,Vault15,Gathedral,Hub,Glow,Bones13,Hills,Cart,Miniplayer;
	//Инвентарь
//	BYTE invbtn; //0-ничего не зажато 1-зажата scr up 2-scr down 3-OK


    WORD end_id[4]; // глюк связанный с загрузкой последующих ресурсов!!!
	AnyFrames* pany;

	int InitDInput();

//	char* InpBuf;
//	DWORD inlen;
//	DWORD inpos;
//	char* OutBuf;
//	DWORD outlen;
//	DWORD outpos;
	char* ComBuf;
	DWORD comlen;
	DWORD compos;

	CBufMngr bin;
	CBufMngr bout;
	z_stream zstrm;
	bool zstrmok;

	DWORD stat_com,stat_decom;

	SOCKADDR_IN remote;
	SOCKET sock;
	fd_set read_set,write_set,exc_set;
	int InitNet();
	int NetCon();
	void ParseSocket(WORD wait=0);
	int NetInput();
	int NetOutput();
	void NetProcess();
//!Cvet +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	void Net_SendLogIn(char* login, char* pass);
	void Net_SendCreatePlayer(crit_info* newcr);

	void Net_SendUseObject(BYTE crit_obj_type, DWORD targ_id, BYTE crit_ori, BYTE crit_num_action, BYTE crit_rate_object);
	void Net_SendChangeObject(DWORD idobj, BYTE num_slot);
	void Net_SendTalk(CrID id_to_talk, BYTE answer);
	void Net_SendGetTime();
//!Cvet -------------------------------------------------------------
	void Net_SendText(char* str);
	void Net_SendRot(BYTE rot); //[0=CW,1=CCW]
    void Net_SendMove(BYTE dir); //!Cvet переделал

	void Net_SendGiveMeMap(WORD map_num); //!Cvet
	void Net_SendLoadMapOK(); //!Cvet

	void Net_OnAddCritter();
	void Net_OnAddObjOnMap();
	void Net_OnRemObjFromMap();
	void Net_OnRemoveCritter();
	void Net_OnCritterText();
	void Net_OnCritterRot(BYTE rot); //[0=CW,1=CCW]
//!Cvet +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    void Net_OnCritterMove(); //движение
	void Net_OnCritterAction(); //действие

	void Net_OnChosenXY(); //помещение чузена в новую клетку
	void Net_OnChosenParams(); //передача параметров игроку
	void Net_OnChosenParam(); //передача параметра
	void Net_OnChosenLogin(); //сетевые сообщения
	void Net_OnChosenAddObject(); //добавление объекта
	void Net_OnChosenTalk();

	void Net_OnGameTime();

	void Net_OnLoadMap();
	void Net_OnMap();

	HexTYPE TargetX;
	HexTYPE TargetY;
	int GetMouseTile(int cursor_x, int cursor_y);

	int GetMouseCritter(int cursor_x, int cursor_y);
	int GetMouseScenery(int cursor_x, int cursor_y);
	int GetMouseItem(int cursor_x, int cursor_y);

	TICK Ping;

	char opt_login[MAX_LOGIN+1];
	char opt_pass[MAX_LOGIN+1];

//Параметры
	void CreateParamsMaps();
	params_map stats_map;
	params_map skills_map;
	params_map perks_map;
	params_map object_map;
	void CreateStringsParamsMaps();
	params_str_map stats_str_map;
	params_str_map skills_str_map;
	params_str_map perks_str_map;
	//params_str_map object_map;

//Объекты в игре
	stat_map all_s_obj; //вся статика в игре
	//имена объектов
	char name_obj[100][MAX_OBJECT_NAME];
	//инфо объектов
	char info_obj[100][MAX_OBJECT_INFO];
	//картинки в инвентаре
	WORD inv_pic_b[367]; //большие
	WORD inv_pic_s[367]; //малые
	//картинки использования
	WORD pic_use[100];

//Управление чузеном
	BYTE chosen_action;

	HexTYPE PathMoveX;
	HexTYPE PathMoveY;
	DWORD TargetID;

	void SetChosenAction(BYTE set_action){ chosen_action=set_action; };
	int IsChosenAction(BYTE check_action){ if(chosen_action==check_action) return 1; return 0; };

	void ChosenProcess();

//анимация всего интерфейса
	int Init_Iface();

	int numIface; //номер анимации интерфейса

//Инвентарь
	WORD invbox,invokon,invokoff,invscrupin,invscrupout,invscrupoff,invscrdwin,invscrdwout,invscrdwoff; //анимация

	BYTE InvHold; //номер зажатой кнопки - нмера внизу //0 - кнопка не зажата
	int scroll_items;

	int InvX; //х,у основного меню окна, все остальное будет отталкиваться именно от этих координат
	int InvY; 

	INTRECT InvMain,InvObl,InvChosen;
	int HeightItem;// = 30; //высота картинки в инвенторе, всего там 10
	//слот1,2, армор
	INTRECT InvSlot1,InvSlot2,InvArmor;
	//кнопки
	INTRECT InvBtnUp,InvBtnDown,InvBtnOk;
	//точки вывода инфы
	INTRECT txtObject;
	//это нужно для перетаскивания окна инвентари
	int invvectx;
	int invvecty;

	void InvDrawGraph(); 
	void InvDrawText();
	void InvMouseMove();
	int InvMouseDown();
	int InvMouseUp();

//Основной интерфейс
	WORD iface,panel,intscrupon,intscrupoff,intscrdownon,intscrdownoff,intchangesloton,intchangeslotoff,
	intinvon,intinvoff,intmenuon,intmenuoff,intskillon,intskilloff,intmapon,intmapoff,intinfoon,intinfooff,
	intpipon,inpipoff;
	WORD diodeG,diodeY,diodeR;

	BYTE IntHold; //номер зажатой кнопки - нмера внизу //0 - кнопка не зажата
	int IntX; //х,у основного меню окна, все остальное будет отталкиваться именно от этих координат
	int IntY; 

	INTRECT IntMain; 
	//слоты
	INTRECT IntObject; //слот объекта
	//кнопки
	INTRECT IntBScrUp,IntBScrDown,IntBChangeSlot,IntBInv,IntBMenu,IntBSkill,IntBMap,IntBInfo,IntBPip;
	//точки вывода инфы
	INTRECT IntTXT;
	//action points //15 зеленых(200мс) 3 желтых(1000мс) 2 красных(10000мс)
	INTRECT IntAP,IntHP,IntAC;

	int IntAPstepX,IntAPstepY;

	void IntDrawGraph(); 
	void IntDrawText();
	int IntMouseDown();
	int IntMouseUp();

//Меню левой кнопки (LMenu)
	WORD lm_talk_off,lm_talk_on,lm_look_off,lm_look_on,lm_break_off,lm_break_on;

	bool LMenu_try_activated;
	DWORD LMenu_start_time;
	int LMenu_cur_x;
	int LMenu_cur_y;
	int LMenu_x;
	int LMenu_y;
	int LMenu_node_height;

	LMenu_list* LMenu_nodes;
	int LMenu_cur_node;

	LMenu_list LMenu_player_nodes;
	LMenu_list LMenu_npc_nodes;
	LMenu_list LMenu_scenery_nodes;
	LMenu_list LMenu_item_nodes;

	BYTE LMenu_mode;

	void SetLMenu(BYTE set_lmenu) { LMenu_mode=set_lmenu; };
	BOOL IsLMenu() { if(LMenu_mode!=LMENU_OFF) return 1; return 0; };
	void LMenuTryCreate();
	void LMenuDraw();
	void LMenuMouseMove();
	void LMenuMouseUp();

//Логин/пасс
	WORD loginpic;

	BYTE LogFocus; //позиция курсора 0-нигде 1-login 2-pass
	BYTE LogMsg; //номер сообщения

	int LogX;
	int LogY;

	INTRECT LogMain,LogWLogin,LogWPass,LogBOk,LogBReg;

	void ShowLogIn();
	void LogInput();
	
//Регистрация
	crit_info New_cr;

	WORD registpic;

	BYTE RegFocus;

	int RegX;
	int RegY;

	INTRECT RegMain,RegWS,RegWP,RegWE,RegWC,RegWI,RegWA,RegWL,RegWLogin,RegWPass,RegWName,
		RegWCases0,RegWCases1,RegWCases2,RegWCases3,RegWCases4,RegWBType,RegWGender,RegWAge,RegBReg,RegBBack;
	
	int CheckRegData(crit_info* newcr);

	void ShowRegistration();
	void RegInput();

//Диалог
	WORD dialog_begin,dialog_answ,dialog_end;

	BYTE DlgHold;

	int DlgCurAnsw;

	BYTE all_answers;

	int dlgvectx;
	int dlgvecty;

	char text_dialog[MAX_DIALOG_TEXT+1];
	char text_answer[MAX_ANSWERS][MAX_DIALOG_TEXT+1];

	int DlgX;
	int DlgY;

	INTRECT DlgMain,DlgBegin,DlgEnd,DlgText,DlgAnsw;

	int DlgNextAnswX;
	int DlgNextAnswY;

	void DlgDrawGraph();
	void DlgDrawText();

	void DlgMouseMove();
	void DlgMouseDown();
	void DlgMouseUp();

	int LoadDialogFromFile(CrID id_npc, DWORD id_dialog, char* dialog);

//Игровое время
	WORD Game_Hours,Game_Mins,Game_Time,Game_Year;
	BYTE Game_Day,Game_Month;

	DWORD Game_LastTime;
	DWORD Game_CurTime;

	void SetWeather();

	BYTE dayR,dayG,dayB;
//!Cvet ----------------------------------------------------------------------

	void ParseInput();

	void SetColor(BYTE r,BYTE g,BYTE b);
	void SetColor(DWORD color); //!Cvet

	crit_map critters;
	CCritter* AddCritter(crit_info* pinfo);

	CCritter* lpChosen;
	// пробные НПСы
	CCritter* lpNPC0;
	CCritter* lpNPC1;
	// для отладки
	CrID LstMoveId;
    CrID LstAddCritId;
	CrID LstDelCritId;
	CrID LstSayCritId;
	char newbie[31];

//!Cvet ++++++++++++++++++++++++++++++++++++++++++++++++++++++
char mess_str[EDIT_LEN+1]; // строка для окна сообщений расширяемая
char all_mess[300][MAX_TEXT+128];
BYTE max_mess,scr_mess;

	void AddMess(DWORD text_color,char* message_text, ...);

//	char mess_str[EDIT_LEN+1]; // строка для окна сообщений расширяемая
//!Cvet ------------------------------------------------------
};


#endif
