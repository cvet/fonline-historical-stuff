//FEngine_Iface.cpp by Cvet

#include "stdafx.h"

#include "FEngine.h"
#include "common.h"
#include "keyb.h"

const BYTE DLG_HOLD_NONE	=0;
const BYTE DLG_HOLD_MAIN	=1;
const BYTE DLG_HOLD_ANSW	=2;

char* LoginMess[50]=
{
	{""}, //0
	{"ОШИБКА!!! Логин не может быть короче четырех символов."}, //1
	{"ОШИБКА!!! Пароль не может быть короче четырех символов."}, //2
	{"ОШИБКА ПРИ РЕГИСТРАЦИИ!!! Аккаунт с таким именем уже есть - измените логин."}, //3
	{"ОШИБКА ПРИ АУНТЕФИКАЦИИ!!! Игрок под этим логином уже в игре - обязательно обратитесь к администратору сервера."}, //4
	{"ОШИБКА ПРИ РЕГИСТРАЦИИ!!! Неверные данные SPECIAL."}, //5
	{"РЕГИСТРАЦИЯ ПРОЙДЕНА УСПЕШНО. ЖДИТЕ ПОДТВЕЖДЕНИЯ НА ВАШ E-MAIL."}, //6
	{"ИДЕТ СОЕДИНЕНИЕ С СЕРВЕРОМ.... ЖДИТЕ...."}, //7
	{"СБОЙ СОЕДИНЕНИЯ!!! При повторениях сбоя - обратитесь к админестратору сервера."}, //8
	{"ОШИБКА ПРИ АУНТЕФИКАЦИИ!!! Неверный логин или пароль."}, //9
	{"СОЕДИНЕНИЕ УСТАНОВЛЕНО.... АУНТЕФИКАЦИЯ.... ЖДИТЕ...."}, //10
	{"ОШИБКА!!! Клетка занята... попробуйте немного позже"}, //11
	{"ОТКЛЮЧЕНИЕ ПО КОМАНДЕ ИГРОКА"}, //12
	{"ОШИБКА ПРИ РЕГИСТРАЦИИ!!! Длинное или короткое имя."}, //13
	{"ОШИБКА ПРИ РЕГИСТРАЦИИ!!! Длинное или короткое одно(несколько) из склонений."}, //14
	{"ОШИБКА ПРИ РЕГИСТРАЦИИ!!! Невеный пол."}, //15
	{"ОШИБКА ПРИ РЕГИСТРАЦИИ!!! Неверный возраст."}, //16
};

int CFEngine::Init_Iface()
{
	WriteLog("Инициализация интерфейса...\n");

	CreateStringsParamsMaps();

	GetPrivateProfileString("LOGIN","login","",opt_login,MAX_LOGIN,CFG_FILE);
	GetPrivateProfileString("LOGIN","pass","",opt_pass,MAX_LOGIN,CFG_FILE);

	char key1[255];
	char key2[255];
	int IB;

//берем настройки для инвентаря
	numIface=GetPrivateProfileInt("IFACE","Iface",0,CFG_FILE);
	InvX=GetPrivateProfileInt("IFACE","InvX",100,CFG_FILE);
	InvY=GetPrivateProfileInt("IFACE","InvY",100,CFG_FILE);
	scroll_items=0;

	sprintf(key1,"Iface%d",numIface);

	for(IB=0;IB<=3;IB++) { sprintf(key2,"InvMain%d",IB); InvMain[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"InvObl%d",IB); InvObl[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	HeightItem=GetPrivateProfileInt(key1,"HeightItem",100,CFG_INT_FILE);
	for(IB=0;IB<=1;IB++) { sprintf(key2,"InvChosen%d",IB); InvChosen[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}

	for(IB=0;IB<=3;IB++) { sprintf(key2,"InvSlot1%d",IB); InvSlot1[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"InvSlot2%d",IB); InvSlot2[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"InvArmor%d",IB); InvArmor[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	
	for(IB=0;IB<=3;IB++) { sprintf(key2,"InvBtnUp%d",IB); InvBtnUp[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"InvBtnDown%d",IB); InvBtnDown[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"InvBtnOk%d",IB); InvBtnOk[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}

	for(IB=0;IB<=3;IB++) { sprintf(key2,"txtObject%d",IB); txtObject[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}

//берем настройки для интерфейса
	IntX=GetPrivateProfileInt("IFACE","IntX",0,CFG_FILE); //это только для знающих лидей, малоли надо будет Х поменять, в кфг его нет
	IntY=MODE_HEIGHT-GetPrivateProfileInt(key1,"IntMain3",300,CFG_INT_FILE);

//	IntX=0;
//	IntY=0;

	IntAP[0]=GetPrivateProfileInt(key1,"IntAP0",1,CFG_INT_FILE); IntAP.l+=IntX;
	IntAP[1]=GetPrivateProfileInt(key1,"IntAP1",1,CFG_INT_FILE); IntAP.t+=IntY;
	IntAPstepX=GetPrivateProfileInt(key1,"IntAPstepX",1,CFG_INT_FILE);
	IntAPstepY=GetPrivateProfileInt(key1,"IntAPstepY",1,CFG_INT_FILE);

	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntMain%d",IB); IntMain[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntMain[IB]+=IntX; else IntMain[IB]+=IntY;}

	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntObject%d",IB); IntObject[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntObject[IB]+=IntX; else IntObject[IB]+=IntY;}

	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBScrUp%d",IB); IntBScrUp[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBScrUp[IB]+=IntX; else IntBScrUp[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBScrDown%d",IB); IntBScrDown[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBScrDown[IB]+=IntX; else IntBScrDown[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBChangeSlot%d",IB); IntBChangeSlot[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBChangeSlot[IB]+=IntX; else IntBChangeSlot[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBInv%d",IB); IntBInv[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBInv[IB]+=IntX; else IntBInv[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBMenu%d",IB); IntBMenu[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBMenu[IB]+=IntX; else IntBMenu[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBSkill%d",IB); IntBSkill[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBSkill[IB]+=IntX; else IntBSkill[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBMap%d",IB); IntBMap[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBMap[IB]+=IntX; else IntBMap[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBInfo%d",IB); IntBInfo[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBInfo[IB]+=IntX; else IntBInfo[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntBPip%d",IB); IntBPip[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntBPip[IB]+=IntX; else IntBPip[IB]+=IntY;}

	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntTXT%d",IB); IntTXT[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntTXT[IB]+=IntX; else IntTXT[IB]+=IntY;}

	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntHP%d",IB); IntHP[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntHP[IB]+=IntX; else IntHP[IB]+=IntY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"IntAC%d",IB); IntAC[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) IntAC[IB]+=IntX; else IntAC[IB]+=IntY;}

//берем настройки для меню логин/пасс
	LogX=(MODE_WIDTH-GetPrivateProfileInt(key1,"LogMain2",0,CFG_INT_FILE))/2;
	LogY=(MODE_HEIGHT-GetPrivateProfileInt(key1,"LogMain3",0,CFG_INT_FILE))/2;
	LogFocus=0;

	for(IB=0;IB<=3;IB++) { sprintf(key2,"LogMain%d",IB); LogMain[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) LogMain[IB]+=LogX; else LogMain[IB]+=LogY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"LogWLogin%d",IB); LogWLogin[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) LogWLogin[IB]+=LogX; else LogWLogin[IB]+=LogY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"LogWPass%d",IB); LogWPass[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) LogWPass[IB]+=LogX; else LogWPass[IB]+=LogY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"LogBOk%d",IB); LogBOk[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) LogBOk[IB]+=LogX; else LogBOk[IB]+=LogY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"LogBReg%d",IB); LogBReg[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) LogBReg[IB]+=LogX; else LogBReg[IB]+=LogY;}

//Регистрация
	RegX=(MODE_WIDTH-GetPrivateProfileInt(key1,"RegMain2",0,CFG_INT_FILE))/2;
	RegY=(MODE_HEIGHT-GetPrivateProfileInt(key1,"RegMain3",0,CFG_INT_FILE))/2;
	RegFocus=0;

	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWS%d",IB); RegWS[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWS[IB]+=RegX; else RegWS[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWP%d",IB); RegWP[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWP[IB]+=RegX; else RegWP[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWE%d",IB); RegWE[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWE[IB]+=RegX; else RegWE[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWC%d",IB); RegWC[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWC[IB]+=RegX; else RegWC[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWI%d",IB); RegWI[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWI[IB]+=RegX; else RegWI[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWA%d",IB); RegWA[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWA[IB]+=RegX; else RegWA[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWL%d",IB); RegWL[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWL[IB]+=RegX; else RegWL[IB]+=RegY;}

	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWLogin%d",IB); RegWLogin[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWLogin[IB]+=RegX; else RegWLogin[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWPass%d",IB); RegWPass[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWPass[IB]+=RegX; else RegWPass[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWName%d",IB); RegWName[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWName[IB]+=RegX; else RegWName[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWCases0%d",IB); RegWCases0[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWCases0[IB]+=RegX; else RegWCases0[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWCases1%d",IB); RegWCases1[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWCases1[IB]+=RegX; else RegWCases1[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWCases2%d",IB); RegWCases2[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWCases2[IB]+=RegX; else RegWCases2[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWCases3%d",IB); RegWCases3[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWCases3[IB]+=RegX; else RegWCases3[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWCases4%d",IB); RegWCases4[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWCases4[IB]+=RegX; else RegWCases4[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWBType%d",IB); RegWBType[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWBType[IB]+=RegX; else RegWBType[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWGender%d",IB); RegWGender[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWGender[IB]+=RegX; else RegWGender[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegWAge%d",IB); RegWAge[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegWAge[IB]+=RegX; else RegWAge[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegBReg%d",IB); RegBReg[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegBReg[IB]+=RegX; else RegBReg[IB]+=RegY;}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"RegBBack%d",IB); RegBBack[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);
	if(IB%2==0) RegBBack[IB]+=RegX; else RegBBack[IB]+=RegY;}

	New_cr.st[ST_STRENGHT	]=5;
	New_cr.st[ST_PERCEPTION	]=5;
	New_cr.st[ST_ENDURANCE	]=5;
	New_cr.st[ST_CHARISMA	]=5;
	New_cr.st[ST_INTELLECT	]=5;
	New_cr.st[ST_AGILITY	]=5;
	New_cr.st[ST_LUCK		]=5;
	strcpy(New_cr.name,"noname");
	strcpy(New_cr.cases[0],"кого");
	strcpy(New_cr.cases[1],"кому");
	strcpy(New_cr.cases[2],"кого");
	strcpy(New_cr.cases[3],"с кем");
	strcpy(New_cr.cases[4],"о ком");
	strcpy(New_cr.login,"login");
	strcpy(New_cr.pass,"password");
	New_cr.base_type=0;
	New_cr.st[ST_AGE]=22;
	New_cr.st[ST_GENDER]=0;

//Диалог
	DlgX=100;
	DlgY=100;

	DlgHold=DLG_HOLD_NONE;
	DlgCurAnsw=-1;
	all_answers=0;
	dlgvectx=0;
	dlgvecty=0;

	for(IB=0;IB<=3;IB++) { sprintf(key2,"DlgBegin%d",IB); DlgBegin[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"DlgEnd%d",IB); DlgEnd[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"DlgText%d",IB); DlgText[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	for(IB=0;IB<=3;IB++) { sprintf(key2,"DlgAnsw%d",IB); DlgAnsw[IB]=GetPrivateProfileInt(key1,key2,1,CFG_INT_FILE);}
	DlgNextAnswX=GetPrivateProfileInt(key1,"DlgNextAnswX",1,CFG_INT_FILE);
	DlgNextAnswY=GetPrivateProfileInt(key1,"DlgNextAnswY",1,CFG_INT_FILE);

//инициализация интерфейсбокса
	max_mess=0; scr_mess=0;  
//	AddMess("Загрузка ПипБоя...");
//	AddMess("Загрузка голодисков...");
//	AddMess("Загрузка предметов...");
//	AddMess("Загрузка статистики...");
//	AddMess("Загрузка $$$$$$...");
//	AddMess("Загрузка завершена...");
//	AddMess("Загрузка ПипБоя...");
//	AddMess("Загрузка голодисков...");
//	AddMess("Загрузка предметов...");
//	AddMess("Загрузка статистики...");
//	AddMess("Загрузка $$$$$$...");
//	AddMess("Загрузка завершена...");

//LMenu
	LMenu_cur_node=0;
	LMenu_try_activated=false;
	LMenu_start_time=0;
	SetLMenu(LMENU_OFF);

	LMenu_node_height=GetPrivateProfileInt(key1,"LMenu_node_height",40,CFG_INT_FILE);

	//ноды игрока
	LMenu_player_nodes.push_back(LMENU_NODE_LOOK);
	LMenu_player_nodes.push_back(LMENU_NODE_BREAK);
	//ноды непися
	LMenu_npc_nodes.push_back(LMENU_NODE_LOOK);
	LMenu_npc_nodes.push_back(LMENU_NODE_TALK);
	LMenu_npc_nodes.push_back(LMENU_NODE_BREAK);
	//ноды сценери
	LMenu_scenery_nodes.push_back(LMENU_NODE_BREAK);
	//ноды объектов
	LMenu_item_nodes.push_back(LMENU_NODE_BREAK);


//загрузка графики

//Интерфейс
	sprintf(key1,"ifacen%d.frm",numIface); if(!(ifacen=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"panel%d.frm",numIface); if(!(panel=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;

	sprintf(key1,"hlgrn%d.frm",numIface); if(!(diodeG=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"hlyel%d.frm",numIface); if(!(diodeY=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"hlred%d.frm",numIface); if(!(diodeR=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;

//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intscrupon=sm.LoadSprite("intscrupoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intscrupoff=sm.LoadSprite("intscrupoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intscrdownon=sm.LoadSprite("intscrdownoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intscrdownoff=sm.LoadSprite("intscrdownoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intchangesloton=sm.LoadSprite("intchangeslotoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intchangeslotoff=sm.LoadSprite("intchangeslotoff.frm",PT_ART_IFACE))) return 0;
	sprintf(key1,"ifacen%d.frm",numIface); if(!(intinvon=sm.LoadSprite("invbutdn0.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intinvoff=sm.LoadSprite("intinvoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intmenuon=sm.LoadSprite("intmenuoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intmenuoff=sm.LoadSprite("intmenuoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intskillon=sm.LoadSprite("intskilloff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intskilloff=sm.LoadSprite("intskilloff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intmapon=sm.LoadSprite("intmapoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intmapoff=sm.LoadSprite("intmapoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intinfoon=sm.LoadSprite("intinfooff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intinfooff=sm.LoadSprite("intinfooff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intpipon=sm.LoadSprite("intpipoff.frm",PT_ART_IFACE))) return 0;
//	sprintf(key1,"ifacen%d.frm",numIface); if(!(intpipoff=sm.LoadSprite("intpipoff.frm",PT_ART_IFACE))) return 0;

//Инвентарь
	sprintf(key1,"invbox%d.frm",numIface); if(!(invbox=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"menudown%d.frm",numIface);  if(!(invokon=sm.LoadSprite(key1,PT_ART_IFACE))) return 0; 
	sprintf(key1,"menuup%d.frm",numIface); if(!(invokoff=sm.LoadSprite(key1,PT_ART_IFACE))) return 0; 
	sprintf(key1,"invupout%d.frm",numIface); if(!(invscrupout=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"invupin%d.frm",numIface); if(!(invscrupin=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"invupds%d.frm",numIface); if(!(invscrupoff=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"invdnout%d.frm",numIface); if(!(invscrdwout=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"invdnin%d.frm",numIface); if(!(invscrdwin=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"invdnds%d.frm",numIface); if(!(invscrdwoff=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;

//Логин/пасс
	sprintf(key1,"login%d.frm",numIface); if(!(loginpic=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;

//Регистрация
	sprintf(key1,"regist%d.png",numIface); if(!(registpic=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;

//Диалог
	sprintf(key1,"dialog_begin%d.png",numIface); if(!(dialog_begin=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"dialog_answ%d.png",numIface); if(!(dialog_answ=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"dialog_end%d.png",numIface); if(!(dialog_end=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;


	if(!(actdn=sm.LoadSprite("sattkbdn.frm",PT_ART_INTRFACE))) return 0;
	if(!(actup=sm.LoadSprite("sattkbup.frm",PT_ART_INTRFACE))) return 0;
	// курсоры
	if(!(cur_move=sm.LoadSprite("msef001.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_move_block=sm.LoadSprite("msef002.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_use_o=sm.LoadSprite("acttohit.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_use_s=sm.LoadSprite("crossuse.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_wait=sm.LoadSprite("wait2.frm",PT_ART_INTRFACE))) return 0;

	if(!(cur_hand=sm.LoadSprite("hand.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_def=sm.LoadSprite("actarrow.frm",PT_ART_INTRFACE))) return 0;

	if(!(cur_right=sm.LoadSprite("screast.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_left=sm.LoadSprite("scrwest.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_up=sm.LoadSprite("scrnorth.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_down=sm.LoadSprite("scrsouth.frm",PT_ART_INTRFACE))) return 0;

	if(!(cur_ru=sm.LoadSprite("scrneast.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_lu=sm.LoadSprite("scrnwest.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_rd=sm.LoadSprite("scrseast.frm",PT_ART_INTRFACE))) return 0;
	if(!(cur_ld=sm.LoadSprite("scrswest.frm",PT_ART_INTRFACE))) return 0;
	// карта
//	if(!(worldmap1=sm.LoadMiniSprite("worldmap1.frm",4,PT_ART_MISC))) return 0;
//	if(!(worldmap2=sm.LoadMiniSprite("worldmap2.frm",4,PT_ART_MISC))) return 0;
//	if(!(worldmap3=sm.LoadMiniSprite("worldmap3.frm",4,PT_ART_MISC))) return 0;
//	if(!(worldmap4=sm.LoadMiniSprite("worldmap4.frm",4,PT_ART_MISC))) return 0;
	if(!(Arroyo=sm.LoadMiniSprite("stonhead.frm",5,PT_ART_ITEMS))) return 0;
	if(!(Junk=sm.LoadMiniSprite("GIZDEAD1.frm",2,PT_ART_ITEMS))) return 0;
	if(!(Bos=sm.LoadMiniSprite("PWRARMOR.frm",2,PT_ART_INVEN))) return 0;
	if(!(Redding=sm.LoadMiniSprite("redsgn05.frm",2.5,PT_ART_SCENERY))) return 0;
	if(!(Staff=sm.LoadMiniSprite("staff.frm",3,PT_ART_SCENERY))) return 0;
	if(!(Statue2=sm.LoadMiniSprite("Statue.frm",4,PT_ART_SCENERY))) return 0;
	if(!(Flagncr=sm.LoadMiniSprite("flag.frm",2,PT_ART_SCENERY))) return 0;
	if(!(Pole=sm.LoadMiniSprite("flagpole.frm",4,PT_ART_SCENERY))) return 0;
	if(!(Flagencl=sm.LoadMiniSprite("encl41.frm",2,PT_ART_SCENERY))) return 0;
	if(!(Enclave=sm.LoadMiniSprite("encl40.frm",2,PT_ART_SCENERY))) return 0;
    if(!(Navarro1=sm.LoadMiniSprite("radar02.frm",5,PT_ART_SCENERY))) return 0;
	if(!(Navarro2=sm.LoadMiniSprite("radar01.frm",5,PT_ART_SCENERY))) return 0;
	if(!(Klamath=sm.LoadMiniSprite("posts.frm",2,PT_ART_SCENERY))) return 0;
	if(!(Vault13=sm.LoadMiniSprite("V13SECR5.frm",8,PT_ART_SCENERY))) return 0;
	if(!(Gecko=sm.LoadMiniSprite("mangsign.frm",3,PT_ART_SCENERY))) return 0;
	if(!(Vtcity=sm.LoadMiniSprite("vcturet1.frm",5,PT_ART_SCENERY))) return 0;
	if(!(Epa=sm.LoadMiniSprite("tree10.frm",4,PT_ART_SCENERY))) return 0;
	if(!(Modoc=sm.LoadMiniSprite("sbadge.frm",2,PT_ART_INVEN))) return 0;
	if(!(Den=sm.LoadMiniSprite("CARSPEC2.frm",5,PT_ART_SCENERY))) return 0;
	if(!(Reno1=sm.LoadMiniSprite("renosgn1.frm",6,PT_ART_SCENERY))) return 0;
	if(!(Reno2=sm.LoadMiniSprite("renosgn2.frm",6,PT_ART_SCENERY))) return 0;
	if(!(Reno3=sm.LoadMiniSprite("renosgn3.frm",6,PT_ART_SCENERY))) return 0;
	if(!(Reno4=sm.LoadMiniSprite("renosgn4.frm",6,PT_ART_SCENERY))) return 0;
	if(!(Reno5=sm.LoadMiniSprite("renosgn5.frm",6,PT_ART_SCENERY))) return 0;
	if(!(Reno6=sm.LoadMiniSprite("renosgn6.frm",6,PT_ART_SCENERY))) return 0;
	if(!(Mbase1=sm.LoadMiniSprite("shack.frm",4,PT_ART_SCENERY))) return 0;
	if(!(Mbase2=sm.LoadMiniSprite("milgate.frm",4,PT_ART_SCENERY))) return 0;
	if(!(Sfchina=sm.LoadMiniSprite("shipsc20.frm",2,PT_ART_SCENERY))) return 0;
	if(!(Vault15=sm.LoadMiniSprite("V13SECR4.frm",8,PT_ART_SCENERY))) return 0;
	if(!(Gathedral=sm.LoadMiniSprite("chdflag3.frm",4,PT_ART_SCENERY))) return 0;
	if(!(Hub=sm.LoadMiniSprite("bib.frm",4,PT_ART_ITEMS))) return 0;
	if(!(Bones13=sm.LoadMiniSprite("V13BONES.frm",1.5,PT_ART_SCENERY))) return 0;
	if(!(Glow=sm.LoadMiniSprite("radsign1.frm",1.5,PT_ART_SCENERY))) return 0;
    if(!(Hills=sm.LoadMiniSprite("minemach.frm",4,PT_ART_ITEMS))) return 0;
	if(!(Cart=sm.LoadMiniSprite("ccart02.frm",3,PT_ART_SCENERY))) return 0;
    // Загрузка Анимации
    // if(!(endanim=sm.LoadSprite("endanim.frm",PT_ART_INTRFACE))) return 0;
	if(!(sm.LoadAnyAnimation("endanim.frm",PT_ART_INTRFACE,end_id))) return 0;

//Графика LMenu
	sprintf(key1,"TALKN%d.frm",numIface);	if(!(lm_talk_off=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"TALKH%d.frm",numIface);	if(!(lm_talk_on=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"LOOKN%d.frm",numIface);	if(!(lm_look_off=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"LOOKH%d.frm",numIface);	if(!(lm_look_on=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"CANCELN%d.frm",numIface); if(!(lm_break_off=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;
	sprintf(key1,"CANCELH%d.frm",numIface); if(!(lm_break_on=sm.LoadSprite(key1,PT_ART_IFACE))) return 0;


//Звуки
TICK gtime=GetTickCount();
//	WORD snd1;
//	if(!(snd1=sdm.LoadSound("acm.acm",PT_ART_IFACE))) return 0;
	sdm.LPESound("05raider.acm",PT_SND_MUSIC);
WriteLog("time load sound:%d\n",GetTickCount()-gtime);
//	sdm.PlaySound(snd1);

	SetCur(CUR_DEFAULT);

	WriteLog("Инициализация интерфейса прошла успешно\n");

	return 1;
}

void CFEngine::InvDrawGraph() 
{
	sm.DrawSprite(invbox,InvX,InvY,COLOR_DEFAULT); //инвентарь

	if(scroll_items<=0) 
		sm.DrawSprite(invscrupoff,InvBtnUp[0]+InvX,InvBtnUp[1]+InvY,COLOR_DEFAULT); //кнопка вверх выключена
	else 
		if(InvHold==1) 
			sm.DrawSprite(invscrupin,InvBtnUp[0]+InvX,InvBtnUp[1]+InvY,COLOR_DEFAULT); //кнопка вверх нажата
		else 
			sm.DrawSprite(invscrupout,InvBtnUp[0]+InvX,InvBtnUp[1]+InvY,COLOR_DEFAULT); //кнопка вверх отпущена

	int count_items=lpChosen->obj.size();
	if(scroll_items>=count_items-(InvObl[3]-InvObl[1])/HeightItem)
		sm.DrawSprite(invscrdwoff,InvBtnDown[0]+InvX,InvBtnDown[1]+InvY,COLOR_DEFAULT); //кнопка вниз выключена
	else 
		if(InvHold==2) sm.DrawSprite(invscrdwin,InvBtnDown[0]+InvX,InvBtnDown[1]+InvY,COLOR_DEFAULT); //кнопка вниз нажата
		else 
			sm.DrawSprite(invscrdwout,InvBtnDown[0]+InvX,InvBtnDown[1]+InvY,COLOR_DEFAULT); //кнопка вниз отпущена

	if(InvHold==3) 
		sm.DrawSprite(invokon,InvBtnOk[0]+InvX,InvBtnOk[1]+InvY,COLOR_DEFAULT); //конопка ОК
	else 
		sm.DrawSprite(invokoff,InvBtnOk[0]+InvX,InvBtnOk[1]+InvY,COLOR_DEFAULT); //конопка ОК

	sm.DrawSprite(lpChosen->cur_id,InvChosen[0]+InvX,InvChosen[1]+InvY,COLOR_DEFAULT); //отрисовка чезена

	//отрисовка объектов в инвентаре
	int IB=0;
	int IB2=0;
	SpriteInfo* si;
	for(dyn_map::iterator it=lpChosen->obj.begin();it!=lpChosen->obj.end();it++)
	{
		if(IB>=scroll_items && IB<scroll_items+(InvObl[3]-InvObl[1])/HeightItem)
		{
			si=sm.GetSpriteInfo(inv_pic_s[(*it).second->object->p111[OBJ_PIC_INV]]);
			//sm.DrawSpriteSize(
			sm.DrawSprite(inv_pic_s[(*it).second->object->p111[OBJ_PIC_INV]],
				InvObl[0]+InvX+(InvObl[2]-InvObl[0]-si->w)/2,
				InvObl[1]+InvY+(IB2*HeightItem),COLOR_DEFAULT);
			IB2++;
		}
		IB++;
	}

	//отрисовка объектов в слот1
	if(lpChosen->a_obj->object->p111[OBJ_PIC_INV])
	{
		si=sm.GetSpriteInfo(inv_pic_b[lpChosen->a_obj->object->p111[OBJ_PIC_INV]]);
		sm.DrawSprite(inv_pic_b[lpChosen->a_obj->object->p111[OBJ_PIC_INV]],
			InvSlot1[0]+InvX+(InvSlot1[2]-InvSlot1[0]-si->w)/2,
			InvSlot1[1]+InvY+(InvSlot1[3]-InvSlot1[1]-si->h)/2,COLOR_DEFAULT);
	}
	//отрисовка объектов в слот2
	if(lpChosen->a_obj2)
		sm.DrawSprite(inv_pic_b[lpChosen->a_obj2->object->p111[OBJ_PIC_INV]],InvSlot2[0]+InvX,InvSlot2[1]+InvY,COLOR_DEFAULT);
	//отрисовка объектов в armor
	if(lpChosen->a_obj_arm->object->p111[OBJ_PIC_INV])
	{
		si=sm.GetSpriteInfo(inv_pic_b[lpChosen->a_obj_arm->object->p111[OBJ_PIC_INV]]);
		sm.DrawSprite(inv_pic_b[lpChosen->a_obj_arm->object->p111[OBJ_PIC_INV]],
			InvArmor[0]+InvX+(InvArmor[2]-InvArmor[0]-si->w)/2,
			InvArmor[1]+InvY+(InvArmor[3]-InvArmor[1]-si->h)/2,COLOR_DEFAULT);
	}
}

void CFEngine::InvDrawText()
{
	char playstr[1024];
	RECT r1={txtObject[0]+InvX,txtObject[1]+InvY,txtObject[2]+InvX,txtObject[3]+InvY};

	wsprintf(playstr,	"%d\n"
						"------------------------------------\n"
						"ST %d  Жизнь: %d\n"
						"PE %d  Броня: %d\n"
						"EN %d Нормально 0/0%\n"
						"CH %d  Лазер     0/0%\n"
						"IN %d  Огонь     0/0%\n"
						"AG %d  Плазма    0/0%\n"
						"LK %d  Взрвчатка 0/0%\n"
						"------------------------------------\n"
						"Нет предмета\n"
						"Безоружный повр.: 1-10\n"
						"\n"
						"------------------------------------\n"
						"Нет предмета\n"
						"Безоружный повр.: 1-10\n"
						"\n"
						"\n"
						"  Общий вес: 4/250",
	lpChosen->name,
	lpChosen->st[ST_STRENGHT	],lpChosen->st[ST_CURRENT_HP	],
	lpChosen->st[ST_PERCEPTION	],lpChosen->st[ST_ARMOR_CLASS	],
	lpChosen->st[ST_ENDURANCE	],
	lpChosen->st[ST_CHARISMA	],
	lpChosen->st[ST_INTELLECT	],
	lpChosen->st[ST_AGILITY		],
	lpChosen->st[ST_LUCK		]);

	fnt.MyDrawText(r1,playstr,0,COLOR_TEXT_DEFAULT);
}

int CFEngine::InvMouseDown()
{
	//hold 0-нигде 1-scrup 2-scrdown 3-ОК 4-main
	//основной инвентарь 
	if((cur_x>=InvObl[0]+InvX)&&(cur_y>=InvObl[1]+InvY)&&(cur_x<=InvObl[2]+InvX)&&(cur_y<=InvObl[3]+InvY))
	{
		int IB=0;
		int pos_cur=(cur_y-(InvObl[1]+InvY))/HeightItem;
		for(dyn_map::iterator it=lpChosen->obj.begin();it!=lpChosen->obj.end();it++)
		{
			if(IB-scroll_items==pos_cur)
			{
				lpChosen->m_obj=(*it).second;
				lpChosen->obj.erase(it);
				break;
			}
			IB++;
		}
		if(lpChosen->m_obj)
			cur_hold=inv_pic_b[lpChosen->m_obj->object->p111[OBJ_PIC_INV]];
		return 1;
	}
	//слот 1
	if(cur_x>=InvSlot1[0]+InvX && cur_y>=InvSlot1[1]+InvY && cur_x<=InvSlot1[2]+InvX && cur_y<=InvSlot1[3]+InvY && lpChosen->IsFree())
	{
		if(!lpChosen->a_obj->object->p111[OBJ_HIDDEN])
		{
			if(lpChosen->a_obj->object->type==OBJ_TYPE_WEAPON)
				lpChosen->Action(4,lpChosen->a_obj->object->p111[OBJ_TIME_HIDE]);
			else
				lpChosen->Action(12,lpChosen->a_obj->object->p111[OBJ_TIME_HIDE]);

			Net_SendChangeObject(0,0);
			
			lpChosen->m_obj=lpChosen->a_obj;
			lpChosen->a_obj=&lpChosen->def_obj1;
			lpChosen->rate_object=1;

			lpChosen->RefreshWeap();

			if(lpChosen->m_obj)
				cur_hold=inv_pic_b[lpChosen->m_obj->object->p111[OBJ_PIC_INV]];

			return 1;
		}
	}
	//слот 2
/*	if((cur_x>=InvSlot2[0]+InvX)&&(cur_y>=InvSlot2[1]+InvY)&&(cur_x<=InvSlot2[2]+InvX)&&(cur_y<=InvSlot2[3]+InvY))
	{
		if(lpChosen->a_obj2)
		{
			lpChosen->m_obj=lpChosen->a_obj2;
			lpChosen->a_obj2=NULL;
		}
		if(lpChosen->m_obj)
			cur_hold=inv_pic_b[lpChosen->m_obj->object->p[9]];
		return 1;
	}*///!@!
	//слот армор	
	if(cur_x>=InvArmor[0]+InvX && cur_y>=InvArmor[1]+InvY && cur_x<=InvArmor[2]+InvX && cur_y<=InvArmor[3]+InvY && lpChosen->IsFree())
	{
		if(!lpChosen->a_obj_arm->object->p111[OBJ_HIDDEN])
		{
			lpChosen->weapon=1;
			lpChosen->Action(12,lpChosen->a_obj_arm->object->p111[OBJ_TIME_HIDE]);

			lpChosen->m_obj=lpChosen->a_obj_arm;
			lpChosen->a_obj_arm=&lpChosen->def_obj2;

			Net_SendChangeObject(0,1);

			if(lpChosen->m_obj)
				cur_hold=inv_pic_b[lpChosen->m_obj->object->p111[OBJ_PIC_INV]];

			return 1;
		}
	}

	if((cur_x>=InvBtnUp[0]+InvX)	&&(cur_y>=InvBtnUp[1]+InvY)		&&(cur_x<=InvBtnUp[2]+InvX)		&&(cur_y<=InvBtnUp[3]+InvY))	{ InvHold=1; return 1; }
	if((cur_x>=InvBtnDown[0]+InvX)	&&(cur_y>=InvBtnDown[1]+InvY)	&&(cur_x<=InvBtnDown[2]+InvX)	&&(cur_y<=InvBtnDown[3]+InvY))	{ InvHold=2; return 1; }
	if((cur_x>=InvBtnOk[0]+InvX)	&&(cur_y>=InvBtnOk[1]+InvY)		&&(cur_x<=InvBtnOk[2]+InvX)		&&(cur_y<=InvBtnOk[3]+InvY))	{ InvHold=3; return 1; }

	if((cur_x>=InvMain[0]+InvX)&&(cur_y>=InvMain[1]+InvY)&&(cur_x<=InvMain[2]+InvX)&&(cur_y<=InvMain[3]+InvY))
	{ InvHold=4; invvectx=cur_x-InvX; invvecty=cur_y-InvY; return 1; }

return 0;
}

int CFEngine::InvMouseUp()
{
	if(lpChosen->m_obj)
	{
		//слот1
		if(cur_x>=InvSlot1[0]+InvX && cur_y>=InvSlot1[1]+InvY && cur_x<=InvSlot1[2]+InvX && cur_y<=InvSlot1[3]+InvY && lpChosen->m_obj->object->type!=OBJ_TYPE_ARMOR && lpChosen->a_obj->object->id<500 && lpChosen->IsFree()) 
		{
			lpChosen->a_obj=lpChosen->m_obj;
			lpChosen->RefreshWeap();
			lpChosen->m_obj=NULL;
			cur_hold=0;
			lpChosen->rate_object=1;

			if(lpChosen->a_obj->object->type==OBJ_TYPE_WEAPON)
				lpChosen->Action(3,lpChosen->a_obj->object->p111[OBJ_TIME_SHOW]);
			else
				lpChosen->Action(12,lpChosen->a_obj->object->p111[OBJ_TIME_SHOW]);

			Net_SendChangeObject(lpChosen->a_obj->id,0);
			return 1;
		}
/*		//слот2
		if((cur_x>=InvSlot2[0]+InvX)&&(cur_y>=InvSlot2[1]+InvY)&&(cur_x<=InvSlot2[2]+InvX)&&(cur_y<=InvSlot2[3]+InvY)&&(!lpChosen->m_obj->object->p[2])&&(!lpChosen->a_obj2)) 
		{
			lpChosen->a_obj2=lpChosen->m_obj;
			lpChosen->m_obj=NULL;
			cur_hold=0;
			return 1;
		}*///!@!
		//слот армор
		if(cur_x>=InvArmor[0]+InvX && cur_y>=InvArmor[1]+InvY && cur_x<=InvArmor[2]+InvX && cur_y<=InvArmor[3]+InvY && lpChosen->m_obj->object->type==OBJ_TYPE_ARMOR && lpChosen->a_obj_arm->object->id<500 && lpChosen->IsFree()) 
		{
			lpChosen->a_obj_arm=lpChosen->m_obj;

			lpChosen->weapon=1;
			lpChosen->Action(12,lpChosen->a_obj_arm->object->p111[OBJ_TIME_SHOW]);

			lpChosen->m_obj=NULL;
			cur_hold=0;

			Net_SendChangeObject(lpChosen->a_obj_arm->id,1);
			return 1;
		}
		//инвертарь
		if(lpChosen->m_obj) //все что нипопало никуда - поподает в инвентарь
		{
			lpChosen->obj[lpChosen->m_obj->id]=lpChosen->m_obj;
			lpChosen->m_obj=NULL;
			cur_hold=0;
			return 1;
		}
	}

	if((cur_x>=InvBtnUp[0]+InvX)&&(cur_y>=InvBtnUp[1]+InvY)&&(cur_x<=InvBtnUp[2]+InvX)&&(cur_y<=InvBtnUp[3]+InvY)&&(InvHold==1))
		if(scroll_items>0) scroll_items--;

	int count_items=lpChosen->obj.size();
	if((cur_x>=InvBtnDown[0]+InvX)&&(cur_y>=InvBtnDown[1]+InvY)&&(cur_x<=InvBtnDown[2]+InvX)&&(cur_y<=InvBtnDown[3]+InvY)&&(InvHold==2))
		if(scroll_items<count_items-(InvObl[3]-InvObl[1])/HeightItem) scroll_items++;

	if((cur_x>=InvBtnOk[0]+InvX)&&(cur_y>=InvBtnOk[1]+InvY)&&(cur_x<=InvBtnOk[2]+InvX)&&(cur_y<=InvBtnOk[3]+InvY)&&(InvHold==3))
		SetScreen(SCREEN_MAIN);

//	if(InvHold==4)
//	{
//		InvX=cur_x-invvectx; InvY=cur_y-invvecty;
//	}

	cur_hold=0;
	InvHold=0;

	return 1;
}

void CFEngine::InvMouseMove()
{
	if(InvHold!=4) return;

	InvX=cur_x-invvectx;
	InvY=cur_y-invvecty;

	if(InvX<0) InvX=0;
	if(InvX+InvMain[2]>MODE_WIDTH) InvX=MODE_WIDTH-InvMain[2];
	if(InvY<0) InvY=0;
	if(InvY+InvMain[3]>IntY) InvY=IntY-InvMain[3];
}

void CFEngine::IntDrawGraph()
{
	if(edit_mode) sm.DrawSprite(panel,IntX+5,IntY-40,COLOR_DEFAULT);

	sm.DrawSprite(ifacen,IntX,IntY,COLOR_DEFAULT);

	if(IntHold==1) sm.DrawSprite(intscrupon,IntX,IntY,COLOR_DEFAULT);
	//else sm.DrawSprite(intscrupoff,IntX,IntY);
	if(IntHold==2) sm.DrawSprite(intscrdownon,IntX,IntY,COLOR_DEFAULT);
	//else sm.DrawSprite(intscrdownoff,IntX,IntY);
	if(IntHold==3) sm.DrawSprite(intchangesloton,IntX,IntY,COLOR_DEFAULT);
	//else sm.DrawSprite(intchangeslotoff,IntX,IntY);
	if(IntHold==4) sm.DrawSprite(intinvon,IntBInv[0],IntBInv[1],COLOR_DEFAULT);
	//else sm.DrawSprite(intinvoff,IntX,IntY);
	if(IntHold==5) sm.DrawSprite(intmenuon,IntX,IntY,COLOR_DEFAULT);
	//else sm.DrawSprite(intmenuoff,IntX,IntY);
	if(IntHold==6) sm.DrawSprite(intskillon,IntX,IntY,COLOR_DEFAULT);
	//esle sm.DrawSprite(intskilloff,IntX,IntY);
	if(IntHold==7) sm.DrawSprite(intmapon,IntX,IntY,COLOR_DEFAULT);
	//else sm.DrawSprite(intmapoff,IntX,IntY);
	if(IntHold==8) sm.DrawSprite(intinfoon,IntX,IntY,COLOR_DEFAULT);
	//else sm.DrawSprite(intinfooff,IntX,IntY,COLOR_DEFAULT);
	if(IntHold==9) sm.DrawSprite(intpipon,IntX,IntY,COLOR_DEFAULT);
	//else sm.DrawSprite(inpipoff,IntX,IntY);

	if(lpChosen->a_obj->object->p111[OBJ_PIC_INV])
	{
		SpriteInfo* si=sm.GetSpriteInfo(inv_pic_b[lpChosen->a_obj->object->p111[OBJ_PIC_INV]]);
		sm.DrawSprite(inv_pic_b[lpChosen->a_obj->object->p111[OBJ_PIC_INV]],
			IntObject[0]+(IntObject[2]-IntObject[0]-si->w)/2,
			IntObject[1]+(IntObject[3]-IntObject[1]-si->h)/2,COLOR_DEFAULT);
	}

	if(lpChosen->a_obj->object->type==OBJ_TYPE_WEAPON)
	{
		switch(lpChosen->rate_object)
		{
		case 1: sm.DrawSprite(pic_use[lpChosen->a_obj->object->p111[OBJ_WEAP_PA_PIC]],IntObject[0]+5,IntObject[1],COLOR_DEFAULT); break;
		case 2: sm.DrawSprite(pic_use[lpChosen->a_obj->object->p111[OBJ_WEAP_SA_PIC]],IntObject[0]+5,IntObject[1],COLOR_DEFAULT); break;
		case 3: sm.DrawSprite(pic_use[lpChosen->a_obj->object->p111[OBJ_WEAP_TA_PIC]],IntObject[0]+5,IntObject[1],COLOR_DEFAULT); break;
		default: sm.DrawSprite(pic_use[10],IntObject[0]+5,IntObject[1],COLOR_DEFAULT); break;
		}
	}
	else
		sm.DrawSprite(pic_use[10],IntObject[0]+5,IntObject[1],COLOR_DEFAULT);
	
	//диодики
	if(!lpChosen->IsFree())
	{
		sm.DrawSprite(diodeG,IntAP[0],IntAP[1],COLOR_DEFAULT);
		int timed=lpChosen->Tick_count-(GetTickCount()-lpChosen->Tick_start);
		for(int num_d=1; num_d<=20; num_d++)
		{
			if(num_d<=timed/500 && num_d<=14)
				sm.DrawSprite(diodeG,IntAP[0]+num_d*IntAPstepX,IntAP[1]+num_d*IntAPstepY,COLOR_DEFAULT);
			else
			if(num_d>14 && num_d<=17 && num_d-15<=(timed-7500)/3000 && timed>7500)
				sm.DrawSprite(diodeY,IntAP[0]+num_d*IntAPstepX,IntAP[1]+num_d*IntAPstepY,COLOR_DEFAULT);
			else
			if(num_d>17 && num_d<20 && num_d-18<=(timed-16500)/10000 && timed>16500)
				sm.DrawSprite(diodeR,IntAP[0]+num_d*IntAPstepX,IntAP[1]+num_d*IntAPstepY,COLOR_DEFAULT);
		}
	}
}

void CFEngine::IntDrawText()
{
//вывод мессбоха
	if(!max_mess) return;

	char str[MAX_MESSBOX];
	RECT r0={IntTXT[0],IntTXT[1],IntTXT[2],IntTXT[3]};

	str[0]=0;

	if(opt_msgbox_invert==TRUE)
	{
		for(int m=0; m<MAX_MESS_IN_MESSBOX; m++)
		{
			if(max_mess-1-scr_mess-m<0) break;
			strcat(str,all_mess[max_mess-scr_mess-m]);
		}
		str[strlen(str)-1]=0;

		fnt.MyDrawText(r0,str,FT_COLORIZE,COLOR_TEXT_DEFAULT);
	}
	else
	{
		for(int m=MAX_MESS_IN_MESSBOX; m>=0; m--)
		{
			if(m>max_mess-1-scr_mess) continue;

		//	if(max_mess-1-scr_mess-m<0) continue;
			strcat(str,all_mess[max_mess-scr_mess-m]);
		}
		str[strlen(str)-1]=0;

		fnt.MyDrawText(r0,str,FT_UPPER|FT_BOTTOM|FT_COLORIZE,COLOR_TEXT_DEFAULT);	
	}
}

int CFEngine::IntMouseDown()
{	
	IntHold=0;
	if((cur_x>=IntBScrUp[0])		&&(cur_y>=IntBScrUp[1])			&&(cur_x<=IntBScrUp[2])			&&(cur_y<=IntBScrUp[3]))		{ IntHold=1; return 1;}
	if((cur_x>=IntBScrDown[0])		&&(cur_y>=IntBScrDown[1])		&&(cur_x<=IntBScrDown[2])		&&(cur_y<=IntBScrDown[3]))		{ IntHold=2; return 1;}
	if((cur_x>=IntBChangeSlot[0])	&&(cur_y>=IntBChangeSlot[1])	&&(cur_x<=IntBChangeSlot[2])	&&(cur_y<=IntBChangeSlot[3]))	{ IntHold=3; return 1;}
	if((cur_x>=IntBInv[0])			&&(cur_y>=IntBInv[1])			&&(cur_x<=IntBInv[2])			&&(cur_y<=IntBInv[3]))			{ IntHold=4; return 1;}
	if((cur_x>=IntBMenu[0])			&&(cur_y>=IntBMenu[1])			&&(cur_x<=IntBMenu[2])			&&(cur_y<=IntBMenu[3]))			{ IntHold=5; return 1;}
	if((cur_x>=IntBSkill[0])		&&(cur_y>=IntBSkill[1])			&&(cur_x<=IntBSkill[2])			&&(cur_y<=IntBSkill[3]))		{ IntHold=6; return 1;}
	if((cur_x>=IntBMap[0])			&&(cur_y>=IntBMap[1])			&&(cur_x<=IntBMap[2])			&&(cur_y<=IntBMap[3]))			{ IntHold=7; return 1;}
	if((cur_x>=IntBInfo[0])			&&(cur_y>=IntBInfo[1])			&&(cur_x<=IntBInfo[2])			&&(cur_y<=IntBInfo[3]))			{ IntHold=8; return 1;}
	if((cur_x>=IntBPip[0])			&&(cur_y>=IntBPip[1])			&&(cur_x<=IntBPip[2])			&&(cur_y<=IntBPip[3]))			{ IntHold=9; return 1;}
	if((cur_x>=IntObject[0])		&&(cur_y>=IntObject[1])			&&(cur_x<=IntObject[2])			&&(cur_y<=IntObject[3]))		{ IntHold=10; return 1;}
	if((cur_x>=IntMain[0])			&&(cur_y>=IntMain[1])			&&(cur_x<=IntMain[2])			&&(cur_y<=IntMain[3]))			{ return 1;}

	return 0;
}

int CFEngine::IntMouseUp()
{
	if(cur_x>=IntBScrUp[0] && cur_y>=IntBScrUp[1] && cur_x<=IntBScrUp[2] && cur_y<=IntBScrUp[3] && IntHold==1 )
	{
		if(opt_msgbox_invert && scr_mess>0) scr_mess--;
		if(!opt_msgbox_invert && scr_mess<max_mess-1) scr_mess++;
	}
	if(cur_x>=IntBScrDown[0] && cur_y>=IntBScrDown[1] && cur_x<=IntBScrDown[2] && cur_y<=IntBScrDown[3] && IntHold==2 )
	{
		if(opt_msgbox_invert && scr_mess<max_mess-1) scr_mess++;
		if(!opt_msgbox_invert && scr_mess>0) scr_mess--;
	}
	if(cur_x>=IntBChangeSlot[0] && cur_y>=IntBChangeSlot[1] && cur_x<=IntBChangeSlot[2] && cur_y<=IntBChangeSlot[3] && IntHold==3 && lpChosen->IsFree())
	{
/*		dyn_obj* temp_obj;
		temp_obj=lpChosen->a_obj2;
		lpChosen->a_obj2=lpChosen->a_obj;
		lpChosen->a_obj=temp_obj;

		if(temp_obj)
		{
			if(temp_obj->object->weap>1)
				lpChosen->ShowObject(temp_obj->object->weap);
			else
				if(lpChosen->a_obj2)
					if(lpChosen->a_obj2->object->weap>1)
						lpChosen->HideObject(lpChosen->a_obj2->object->weap);
		}
		else
		{
			if(lpChosen->a_obj2)
				if(lpChosen->a_obj2->object->weap>1)
					lpChosen->HideObject(lpChosen->a_obj2->object->weap);
		}*/
		}
	if((cur_x>=IntBInv[0])&&(cur_y>=IntBInv[1])&&(cur_x<=IntBInv[2])&&(cur_y<=IntBInv[3])&&(IntHold==4))
		{ SetScreen(SCREEN_INVENTORY); SetCur(CUR_DEFAULT); }
	if((cur_x>=IntBMenu[0])&&(cur_y>=IntBMenu[1])&&(cur_x<=IntBMenu[2])&&(cur_y<=IntBMenu[3])&&(IntHold==5))
		{}
	if((cur_x>=IntBSkill[0])&&(cur_y>=IntBSkill[1])&&(cur_x<=IntBSkill[2])&&(cur_y<=IntBSkill[3])&&(IntHold==6))
		{}
	if((cur_x>=IntBMap[0])&&(cur_y>=IntBMap[1])&&(cur_x<=IntBMap[2])&&(cur_y<=IntBMap[3])&&(IntHold==7))
		{}
	if((cur_x>=IntBInfo[0])&&(cur_y>=IntBInfo[1])&&(cur_x<=IntBInfo[2])&&(cur_y<=IntBInfo[3])&&(IntHold==8))
		{}
	if((cur_x>=IntBPip[0])&&(cur_y>=IntBPip[1])&&(cur_x<=IntBPip[2])&&(cur_y<=IntBPip[3])&&(IntHold==9))
		{}
	if(cur_x>=IntObject[0] && cur_y>=IntObject[1] && cur_x<=IntObject[2] && cur_y<=IntObject[3] && IntHold==10 && lpChosen->IsFree())
	{	
	//подготовка к использованию
		if(!IsCur(CUR_USE_OBJECT))
		{
			SetCur(CUR_USE_OBJECT);

			if(lpChosen->a_obj->object->type==OBJ_TYPE_WEAPON)
				if(lpChosen->a_obj->object->p111[OBJ_WEAP_TIME_ACTIV])
				{
					lpChosen->Action(8,lpChosen->a_obj->object->p111[OBJ_WEAP_TIME_ACTIV]);
					Net_SendUseObject(OBJ_TYPE_WEAPON,lpChosen->a_obj->id,lpChosen->cur_dir,ACT_ACTIVATE_OBJ,0);
					lpChosen->cond_ext=COND_LIFE_ACTWEAP;
				}
		}
		else
		{
			SetCur(CUR_DEFAULT);

			if(lpChosen->a_obj->object->type==OBJ_TYPE_WEAPON)
				if(lpChosen->a_obj->object->p111[OBJ_WEAP_TIME_UNACTIV])
				{
					lpChosen->Action(9,lpChosen->a_obj->object->p111[OBJ_WEAP_TIME_UNACTIV]);
					Net_SendUseObject(OBJ_TYPE_WEAPON,lpChosen->a_obj->id,lpChosen->cur_dir,ACT_DACTIVATE_OBJ,0);
					lpChosen->cond_ext=COND_LIFE_NONE;
				}
		}
	}

	IntHold=0;

	return 1;
}

void CFEngine::AddMess(DWORD text_color,char* message_text, ...)
{
	if(++max_mess>=MAX_MESS)
	{
		max_mess=1;
		scr_mess=0;
	}
	char temp_str[MAX_TEXT+128];

	va_list list;
	va_start(list, message_text);
	wvsprintf(temp_str,message_text,list);
	va_end(list);


	sprintf(all_mess[max_mess],"|%d - %s\n",text_color,temp_str);
}

void CFEngine::ShowLogIn()
{
	lpDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),1.0,0);
	lpDevice->BeginScene();
	//графика
	sm.DrawSprite(loginpic,LogX,LogY,COLOR_DEFAULT);

	sm.Flush();
	//текст
	RECT rlogin	={LogWLogin[0],LogWLogin[1],LogWLogin[2],LogWLogin[3]};
	RECT rpass	={LogWPass[0] ,LogWPass[1] ,LogWPass[2] ,LogWPass[3] };

	if(LogFocus==1)
		fnt.MyDrawText(rlogin,opt_login	,0,D3DCOLOR_XRGB(0,255,0));
	else
		fnt.MyDrawText(rlogin,opt_login	,0,D3DCOLOR_XRGB(0,100,0));
	if(LogFocus==2)
		fnt.MyDrawText(rpass,opt_pass	,0,D3DCOLOR_XRGB(0,255,0));
	else
		fnt.MyDrawText(rpass,opt_pass	,0,D3DCOLOR_XRGB(0,100,0));

	RECT rlogmess={0,0,MODE_WIDTH,15};
	fnt.MyDrawText(rlogmess,LoginMess[LogMsg],0,D3DCOLOR_XRGB(255,0,0));

	RECT rlogconn={0,15,MODE_WIDTH,30};
	switch (state)
	{
	case STATE_CONN:		fnt.MyDrawText(rlogconn,"СОСТОЯНИЕ: СОЕДИНЕНИЕ"				,0,D3DCOLOR_XRGB(0,0,255)); break;
	case STATE_DROPLINK:	fnt.MyDrawText(rlogconn,"СОСТОЯНИЕ: ОБРЫВ СВЯЗИ"			,0,D3DCOLOR_XRGB(0,0,255)); break;
	case STATE_DISCONNECT:	fnt.MyDrawText(rlogconn,"СОСТОЯНИЕ: ОТСОЕДИНЕН"				,0,D3DCOLOR_XRGB(0,0,255)); break;
	case STATE_GAME:		fnt.MyDrawText(rlogconn,"СОСТОЯНИЕ: В ИГРЕ"					,0,D3DCOLOR_XRGB(0,0,255)); break;
	case STATE_LOGINOK:		fnt.MyDrawText(rlogconn,"СОСТОЯНИЕ: АУНТЕФИКАЦИЯ ПРОЙДЕНА"	,0,D3DCOLOR_XRGB(0,0,255)); break;
	case STATE_INIT_NET:	fnt.MyDrawText(rlogconn,"СОСТОЯНИЕ: ИНИЦИАЛИЗАЦИЯ СЕТИ"		,0,D3DCOLOR_XRGB(0,0,255)); break;
	}
	//курсор
	SpriteInfo* si=sm.GetSpriteInfo(cur);
	int curx=cur_x-(si->w >> 1)+si->offs_x;
	int cury=cur_y-si->h+si->offs_y;
	sm.DrawSprite(cur,curx,cury,COLOR_DEFAULT);

	sm.Flush();
	lpDevice->EndScene();
	lpDevice->Present(NULL,NULL,NULL,NULL);
}

void CFEngine::LogInput()
{
	DIDEVICEOBJECTDATA didod[DI_BUF_SIZE];
	DWORD dwElements;

    dwElements = DI_BUF_SIZE;
    HRESULT hr = lpKeyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
	if(hr!=DI_OK)
	{
		dilost=1;
		WriteLog("LogParseInput> %s\n",(char*)DXGetErrorString8(hr));
		return;
	}

	for(int i=0;i<dwElements;i++) 
	{
		if(LogFocus==1)
		{
			int fnd=0;
			for(WORD tst=0;tst<DI_BUF_SIZE;tst++)
			{
				lang=LANG_ENG;
				DI_ONDOWN(tst, if(GetChar(tst,opt_login,NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
			}
			if(fnd) break;
		}
		if(LogFocus==2)
		{
			int fnd=0;
			for(WORD tst=0;tst<DI_BUF_SIZE;tst++)
			{
				lang=LANG_ENG;
				DI_ONDOWN(tst, if(GetChar(tst,opt_pass,NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
			}
			if(fnd) break;
		}

	}

	dwElements = DI_BUF_SIZE;
	hr = lpMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
	if(hr!=DI_OK)
	{
		dilost=1;
		WriteLog("LogParseInput mouse> %s\n",(char*)DXGetErrorString8(hr));
		return;
	}
		
	for(i=0;i<dwElements;i++) 
	{
		DI_ONMOUSE(DIMOFS_X, cur_x+=didod[i].dwData*opt_mouse_speed );
		DI_ONMOUSE(DIMOFS_Y, cur_y+=didod[i].dwData*opt_mouse_speed );

		DI_ONDOWN(DIMOFS_BUTTON0,
			LogFocus=0;
			if((cur_x>LogWLogin[0])&(cur_y>LogWLogin[1])&(cur_x<LogWLogin[2])&(cur_y<LogWLogin[3])) LogFocus=1;
			if((cur_x>LogWPass[0])&(cur_y>LogWPass[1])&(cur_x<LogWPass[2])&(cur_y<LogWPass[3])) LogFocus=2;
			if((cur_x>LogBOk[0])&(cur_y>LogBOk[1])&(cur_x<LogBOk[2])&(cur_y<LogBOk[3]))
			{
				//прверка на длинну если < 4 символов то ничего не произойдет
				if(opt_login[3]==NULL) { LogMsg=1; return; }
				if(opt_pass[3]==NULL) { LogMsg=2; return; }
				//сохраняем логин пасс
				WritePrivateProfileString("LOGIN","login",opt_login,CFG_FILE);
				WritePrivateProfileString("LOGIN","pass",opt_pass,CFG_FILE);
				//соединяемся с сервером
				LogMsg=7;
				state=STATE_INIT_NET;

				SetCur(CUR_WAIT);
			}
			if((cur_x>LogBReg[0])&(cur_y>LogBReg[1])&(cur_x<LogBReg[2])&(cur_y<LogBReg[3]))
			{
				SetScreen(SCREEN_REGISTRATION);
			}
		);
	}
}

void CFEngine::ShowRegistration()
{
	lpDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),1.0,0);
	lpDevice->BeginScene();
	//графика
	sm.DrawSprite(registpic,RegX,RegY,COLOR_DEFAULT);

	sm.Flush();
	//текст
//	RECT rwall={RegWAllS[0],RegWAllS[1],RegWAllS[2],RegWAllS[3]};
	RECT rws={RegWS[0],RegWS[1],RegWS[2],RegWS[3]};
	RECT rwp={RegWP[0],RegWP[1],RegWP[2],RegWP[3]};
	RECT rwe={RegWE[0],RegWE[1],RegWE[2],RegWE[3]};
	RECT rwc={RegWC[0],RegWC[1],RegWC[2],RegWC[3]};
	RECT rwi={RegWI[0],RegWI[1],RegWI[2],RegWI[3]};
	RECT rwa={RegWA[0],RegWA[1],RegWA[2],RegWA[3]};
	RECT rwl={RegWL[0],RegWL[1],RegWL[2],RegWL[3]};
	RECT rwlogin={RegWLogin[0],RegWLogin[1],RegWLogin[2],RegWLogin[3]};
	RECT rwpass={RegWPass[0],RegWPass[1],RegWPass[2],RegWPass[3]};
	RECT rwname={RegWName[0],RegWName[1],RegWName[2],RegWName[3]};
	RECT rwcases0={RegWCases0[0],RegWCases0[1],RegWCases0[2],RegWCases0[3]};
	RECT rwcases1={RegWCases1[0],RegWCases1[1],RegWCases1[2],RegWCases1[3]};
	RECT rwcases2={RegWCases2[0],RegWCases2[1],RegWCases2[2],RegWCases2[3]};
	RECT rwcases3={RegWCases3[0],RegWCases3[1],RegWCases3[2],RegWCases3[3]};
	RECT rwcases4={RegWCases4[0],RegWCases4[1],RegWCases4[2],RegWCases4[3]};
	RECT rwbtype={RegWBType[0],RegWBType[1],RegWBType[2],RegWBType[3]};
	RECT rwgender={RegWGender[0],RegWGender[1],RegWGender[2],RegWGender[3]};
	RECT rwage={RegWAge[0],RegWAge[1],RegWAge[2],RegWAge[3]};

	if(RegFocus==1) fnt.MyDrawText(rwlogin,New_cr.login,0,D3DCOLOR_XRGB(0,0,255));
		else fnt.MyDrawText(rwlogin,New_cr.login,0,D3DCOLOR_XRGB(0,255,0));
	if(RegFocus==2) fnt.MyDrawText(rwpass,New_cr.pass,0,D3DCOLOR_XRGB(0,0,255));
		else fnt.MyDrawText(rwpass,New_cr.pass,0,D3DCOLOR_XRGB(0,255,0));
	if(RegFocus==3) fnt.MyDrawText(rwname,New_cr.name,0,D3DCOLOR_XRGB(0,0,255));
		else fnt.MyDrawText(rwname,New_cr.name,0,D3DCOLOR_XRGB(0,255,0));
	if(RegFocus==4) fnt.MyDrawText(rwcases0,New_cr.cases[0],0,D3DCOLOR_XRGB(0,0,255));
		else fnt.MyDrawText(rwcases0,New_cr.cases[0],0,D3DCOLOR_XRGB(0,255,0));
	if(RegFocus==5) fnt.MyDrawText(rwcases1,New_cr.cases[1],0,D3DCOLOR_XRGB(0,0,255));
		else fnt.MyDrawText(rwcases1,New_cr.cases[1],0,D3DCOLOR_XRGB(0,255,0));
	if(RegFocus==6) fnt.MyDrawText(rwcases2,New_cr.cases[2],0,D3DCOLOR_XRGB(0,0,255));
		else fnt.MyDrawText(rwcases2,New_cr.cases[2],0,D3DCOLOR_XRGB(0,255,0));
	if(RegFocus==7) fnt.MyDrawText(rwcases3,New_cr.cases[3],0,D3DCOLOR_XRGB(0,0,255));
		else fnt.MyDrawText(rwcases3,New_cr.cases[3],0,D3DCOLOR_XRGB(0,255,0));
	if(RegFocus==8) fnt.MyDrawText(rwcases4,New_cr.cases[4],0,D3DCOLOR_XRGB(0,0,255));
		else fnt.MyDrawText(rwcases4,New_cr.cases[4],0,D3DCOLOR_XRGB(0,255,0));

	char stradd[20];
	sprintf(stradd,"%d",New_cr.st[ST_STRENGHT	]);
	fnt.MyDrawText(rws,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.st[ST_PERCEPTION	]);
	fnt.MyDrawText(rwp,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.st[ST_ENDURANCE	]);
	fnt.MyDrawText(rwe,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.st[ST_CHARISMA	]);
	fnt.MyDrawText(rwc,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.st[ST_INTELLECT	]);
	fnt.MyDrawText(rwi,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.st[ST_AGILITY	]);
	fnt.MyDrawText(rwa,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.st[ST_LUCK		]);
	fnt.MyDrawText(rwl,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.base_type);
	fnt.MyDrawText(rwbtype,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.st[ST_GENDER		]);
	fnt.MyDrawText(rwgender,stradd,0,D3DCOLOR_XRGB(0,255,0));
	sprintf(stradd,"%d",New_cr.st[ST_AGE		]);
	fnt.MyDrawText(rwage,stradd,0,D3DCOLOR_XRGB(0,255,0));

	RECT rlogmess={0,0,MODE_WIDTH,15};
	fnt.MyDrawText(rlogmess,LoginMess[LogMsg],0,D3DCOLOR_XRGB(255,0,0));
	//курсор
	SpriteInfo* si=sm.GetSpriteInfo(cur);
	int curx=cur_x-(si->w >> 1)+si->offs_x;
	int cury=cur_y-si->h+si->offs_y;
	sm.DrawSprite(cur,curx,cury,COLOR_DEFAULT);

	sm.Flush();
	lpDevice->EndScene();
	lpDevice->Present(NULL,NULL,NULL,NULL);
}

void CFEngine::RegInput()
{
	DIDEVICEOBJECTDATA didod[DI_BUF_SIZE];
	DWORD dwElements;

    dwElements = DI_BUF_SIZE;
    HRESULT hr = lpKeyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
	if(hr!=DI_OK)
	{
		dilost=1;
		WriteLog("RegParseInput> %s\n",(char*)DXGetErrorString8(hr));
		return;
	}

	for(int i=0;i<dwElements;i++) 
	{
		int fnd=0;
		for(WORD tst=0;tst<DI_BUF_SIZE;tst++)
		{
			switch (RegFocus)
			{
			case 1: //login
				lang=LANG_ENG; DI_ONDOWN(tst, if(GetChar(tst,New_cr.login,NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
				break;
			case 2: //pass
				lang=LANG_ENG; DI_ONDOWN(tst, if(GetChar(tst,New_cr.pass,NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
				break;
			case 3: //name
				lang=LANG_RUS; DI_ONDOWN(tst, if(GetChar(tst,New_cr.name,NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
				break;
			case 4: //cases0
				lang=LANG_RUS; DI_ONDOWN(tst, if(GetChar(tst,New_cr.cases[0],NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
				break;
			case 5: //cases1
				lang=LANG_RUS; DI_ONDOWN(tst, if(GetChar(tst,New_cr.cases[1],NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
				break;
			case 6: //cases2
				lang=LANG_RUS; DI_ONDOWN(tst, if(GetChar(tst,New_cr.cases[2],NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
				break;
			case 7: //cases3
				lang=LANG_RUS; DI_ONDOWN(tst, if(GetChar(tst,New_cr.cases[3],NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
				break;
			case 8: //cases4
				lang=LANG_RUS; DI_ONDOWN(tst, if(GetChar(tst,New_cr.cases[4],NULL,MAX_LOGIN,lang,ShiftDwn)) {fnd=1;break;});
				break;
			}
		}
		if(fnd) break;
	}

	dwElements = DI_BUF_SIZE;
	hr = lpMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
	if(hr!=DI_OK)
	{
		dilost=1;
		WriteLog("RegParseInput mouse> %s\n",(char*)DXGetErrorString8(hr));
		return;
	}
		
	for(i=0;i<dwElements;i++) 
	{
		DI_ONMOUSE(DIMOFS_X, cur_x+=didod[i].dwData*opt_mouse_speed );
		DI_ONMOUSE(DIMOFS_Y, cur_y+=didod[i].dwData*opt_mouse_speed );

	//левая кнопка мыши
		DI_ONDOWN(DIMOFS_BUTTON0,
			RegFocus=0;
			//login
			if((cur_x>RegWLogin[0])&(cur_y>RegWLogin[1])&(cur_x<RegWLogin[2])&(cur_y<RegWLogin[3])) RegFocus=1;
			//pass
			if((cur_x>RegWPass[0])&(cur_y>RegWPass[1])&(cur_x<RegWPass[2])&(cur_y<RegWPass[3])) RegFocus=2;
			//name
			if((cur_x>RegWName[0])&(cur_y>RegWName[1])&(cur_x<RegWName[2])&(cur_y<RegWName[3])) RegFocus=3;
			//cases0
			if((cur_x>RegWCases0[0])&(cur_y>RegWCases0[1])&(cur_x<RegWCases0[2])&(cur_y<RegWCases0[3])) RegFocus=4;
			//cases1
			if((cur_x>RegWCases1[0])&(cur_y>RegWCases1[1])&(cur_x<RegWCases1[2])&(cur_y<RegWCases1[3])) RegFocus=5;
			//cases2
			if((cur_x>RegWCases2[0])&(cur_y>RegWCases2[1])&(cur_x<RegWCases2[2])&(cur_y<RegWCases2[3])) RegFocus=6;
			//cases3
			if((cur_x>RegWCases3[0])&(cur_y>RegWCases3[1])&(cur_x<RegWCases3[2])&(cur_y<RegWCases3[3])) RegFocus=7;
			//cases4
			if((cur_x>RegWCases4[0])&(cur_y>RegWCases4[1])&(cur_x<RegWCases4[2])&(cur_y<RegWCases4[3])) RegFocus=8;
			//S
			if((cur_x>RegWS[0])&(cur_y>RegWS[1])&(cur_x<RegWS[2])&(cur_y<RegWS[3])) if((++New_cr.st[ST_STRENGHT		])>10) New_cr.st[ST_STRENGHT	]=10;
			//P
			if((cur_x>RegWP[0])&(cur_y>RegWP[1])&(cur_x<RegWP[2])&(cur_y<RegWP[3])) if((++New_cr.st[ST_PERCEPTION	])>10) New_cr.st[ST_PERCEPTION	]=10;
			//E
			if((cur_x>RegWE[0])&(cur_y>RegWE[1])&(cur_x<RegWE[2])&(cur_y<RegWE[3])) if((++New_cr.st[ST_ENDURANCE	])>10) New_cr.st[ST_ENDURANCE	]=10;
			//C
			if((cur_x>RegWC[0])&(cur_y>RegWC[1])&(cur_x<RegWC[2])&(cur_y<RegWC[3])) if((++New_cr.st[ST_CHARISMA		])>10) New_cr.st[ST_CHARISMA	]=10;
			//I
			if((cur_x>RegWI[0])&(cur_y>RegWI[1])&(cur_x<RegWI[2])&(cur_y<RegWI[3])) if((++New_cr.st[ST_INTELLECT	])>10) New_cr.st[ST_INTELLECT	]=10;
			//A
			if((cur_x>RegWA[0])&(cur_y>RegWA[1])&(cur_x<RegWA[2])&(cur_y<RegWA[3])) if((++New_cr.st[ST_AGILITY		])>10) New_cr.st[ST_AGILITY		]=10;
			//L
			if((cur_x>RegWL[0])&(cur_y>RegWL[1])&(cur_x<RegWL[2])&(cur_y<RegWL[3])) if((++New_cr.st[ST_LUCK			])>10) New_cr.st[ST_LUCK		]=10;
			//gender
			if((cur_x>RegWGender[0])&(cur_y>RegWGender[1])&(cur_x<RegWGender[2])&(cur_y<RegWGender[3])) if((++New_cr.st[ST_GENDER])>2) New_cr.st[ST_GENDER]=2;
			//age
			if((cur_x>RegWAge[0])&(cur_y>RegWAge[1])&(cur_x<RegWAge[2])&(cur_y<RegWAge[3])) if((++New_cr.st[ST_AGE])>80) New_cr.st[ST_AGE]=14;
			//base_type
			if((cur_x>RegWBType[0])&(cur_y>RegWBType[1])&(cur_x<RegWBType[2])&(cur_y<RegWBType[3])) if((++New_cr.base_type)>2) New_cr.base_type=2;
			//registration
			if((cur_x>RegBReg[0])&(cur_y>RegBReg[1])&(cur_x<RegBReg[2])&(cur_y<RegBReg[3])) 
				if(CheckRegData(&New_cr))
				{
					state=STATE_INIT_NET;
					SetCur(CUR_WAIT);
				}
			//back
			if((cur_x>RegBBack[0])&(cur_y>RegBBack[1])&(cur_x<RegBBack[2])&(cur_y<RegBBack[3])) SetScreen(SCREEN_LOGIN);
		);

	//правая кнопка мыши
		DI_ONDOWN(DIMOFS_BUTTON1,
			RegFocus=0;
			//S
			if((cur_x>RegWS[0])&(cur_y>RegWS[1])&(cur_x<RegWS[2])&(cur_y<RegWS[3])) if((--New_cr.st[ST_STRENGHT	])<1) New_cr.st[ST_STRENGHT		]=1;
			//P
			if((cur_x>RegWP[0])&(cur_y>RegWP[1])&(cur_x<RegWP[2])&(cur_y<RegWP[3])) if((--New_cr.st[ST_PERCEPTION	])<1) New_cr.st[ST_PERCEPTION	]=1;
			//E
			if((cur_x>RegWE[0])&(cur_y>RegWE[1])&(cur_x<RegWE[2])&(cur_y<RegWE[3])) if((--New_cr.st[ST_ENDURANCE	])<1) New_cr.st[ST_ENDURANCE	]=1;
			//C
			if((cur_x>RegWC[0])&(cur_y>RegWC[1])&(cur_x<RegWC[2])&(cur_y<RegWC[3])) if((--New_cr.st[ST_CHARISMA	])<1) New_cr.st[ST_CHARISMA		]=1;
			//I
			if((cur_x>RegWI[0])&(cur_y>RegWI[1])&(cur_x<RegWI[2])&(cur_y<RegWI[3])) if((--New_cr.st[ST_INTELLECT	])<1) New_cr.st[ST_INTELLECT	]=1;
			//A
			if((cur_x>RegWA[0])&(cur_y>RegWA[1])&(cur_x<RegWA[2])&(cur_y<RegWA[3])) if((--New_cr.st[ST_AGILITY	])<1) New_cr.st[ST_AGILITY		]=1;
			//L
			if((cur_x>RegWL[0])&(cur_y>RegWL[1])&(cur_x<RegWL[2])&(cur_y<RegWL[3])) if((--New_cr.st[ST_LUCK		])<1) New_cr.st[ST_LUCK			]=1;
			//gender
			if((cur_x>RegWGender[0])&(cur_y>RegWGender[1])&(cur_x<RegWGender[2])&(cur_y<RegWGender[3])) if(New_cr.st[ST_GENDER]) New_cr.st[ST_GENDER]--;
			//age
			if((cur_x>RegWAge[0])&(cur_y>RegWAge[1])&(cur_x<RegWAge[2])&(cur_y<RegWAge[3])) if((--New_cr.st[ST_AGE])<14) New_cr.st[ST_AGE]=80;
			//base_type
			if((cur_x>RegWBType[0])&(cur_y>RegWBType[1])&(cur_x<RegWBType[2])&(cur_y<RegWBType[3])) if(New_cr.base_type) New_cr.base_type--;
		);
	}

	if(cur_x>MODE_WIDTH) cur_x=MODE_WIDTH;
	if(cur_x<0) cur_x=0;
	if(cur_y>MODE_HEIGHT) cur_y=MODE_HEIGHT;
	if(cur_y<0) cur_y=0;
}

void CFEngine::DlgDrawGraph()
{
	SpriteInfo* spr_inf=sm.GetSpriteInfo(dialog_begin);
	if(!spr_inf) return;

	sm.DrawSprite(dialog_begin,DlgX,DlgY,COLOR_DEFAULT);
	for(int ddg=0; ddg<all_answers; ddg++)
	{
		sm.DrawSprite(dialog_answ,DlgX+DlgNextAnswX*ddg,DlgY+spr_inf->h+DlgNextAnswY*ddg,COLOR_DEFAULT);
	}
	sm.DrawSprite(dialog_end,DlgX+DlgNextAnswX*all_answers,DlgY+spr_inf->h+DlgNextAnswY*all_answers,COLOR_DEFAULT);
}

void CFEngine::DlgDrawText()
{
	RECT rmt={DlgText[0]+DlgX,DlgText[1]+DlgY,DlgText[2]+DlgX,DlgText[3]+DlgY};
	fnt.MyDrawText(rmt,text_dialog,0,D3DCOLOR_XRGB(0,200,200)); //текст нпц

	RECT rat={0,0,0,0};
	for(int ddt=0; ddt<all_answers; ddt++)
	{
		rat.left  =DlgAnsw[0]+DlgNextAnswX*ddt+DlgX;
		rat.top   =DlgAnsw[1]+DlgNextAnswY*ddt+DlgY;
		rat.right =DlgAnsw[2]+DlgNextAnswX*ddt+DlgX;
		rat.bottom=DlgAnsw[3]+DlgNextAnswY*ddt+DlgY;

		if(DlgHold==DLG_HOLD_ANSW)
		{
			if(ddt==DlgCurAnsw)
			{
				fnt.MyDrawText(rat,text_answer[ddt],0,D3DCOLOR_XRGB(0,0,220)); //выбор
			}
			else
				fnt.MyDrawText(rat,text_answer[ddt],0,D3DCOLOR_XRGB(0,150,0)); //обычный
		}
		else
		{
			if(ddt==DlgCurAnsw)
			{
				fnt.MyDrawText(rat,text_answer[ddt],0,D3DCOLOR_XRGB(0,240,0)); //подсветка
			}
			else
				fnt.MyDrawText(rat,text_answer[ddt],0,D3DCOLOR_XRGB(0,150,0)); //обычный
		}
	}
}

void CFEngine::DlgMouseMove()
{
	int dmm=0;

	switch (DlgHold)
	{
	case DLG_HOLD_NONE:
		for(dmm=0; dmm<all_answers; dmm++)
			if((cur_x>=DlgAnsw[0]+DlgNextAnswX*dmm+DlgX)&&(cur_y>=DlgAnsw[1]+DlgNextAnswY*dmm+DlgY)&&
				(cur_x<=DlgAnsw[2]+DlgNextAnswX*dmm+DlgX)&&(cur_y<=DlgAnsw[3]+DlgNextAnswY*dmm+DlgY))
			{
				DlgCurAnsw=dmm;
				return;
			} 
		DlgCurAnsw=-1;
		break;
	case DLG_HOLD_MAIN:
		DlgX=cur_x-dlgvectx;
		DlgY=cur_y-dlgvecty;

		if(DlgX<0) DlgX=0;
		if(DlgX+DlgMain[2]>MODE_WIDTH) DlgX=MODE_WIDTH-DlgMain[2];
		if(DlgY<0) DlgY=0;
		if(DlgY+DlgMain[3]>IntY) DlgY=IntY-DlgMain[3];
		break;
	case DLG_HOLD_ANSW:
		break;
	default:
		DlgHold=DLG_HOLD_NONE;
		break;
	}
}

void CFEngine::DlgMouseDown()
{
	DlgHold=DLG_HOLD_NONE;

	if((cur_x>=DlgAnsw[0]+DlgX)&&(cur_y>=DlgAnsw[1]+DlgY)&&
		(cur_x<=DlgAnsw[2]+DlgNextAnswX*all_answers+DlgX)&&(cur_y<=DlgAnsw[3]+DlgNextAnswY*all_answers+DlgY))
	{
		DlgHold=DLG_HOLD_ANSW;
		return;
	}

	if((cur_x>=DlgMain[0]+DlgX)&&(cur_y>=DlgMain[1]+DlgY)&&(cur_x<=DlgMain[2]+DlgX)&&(cur_y<=DlgMain[3]+DlgY))
	{
		dlgvectx=cur_x-DlgX;
		dlgvecty=cur_y-DlgY;
		DlgHold=DLG_HOLD_MAIN;
		return;
	}
	
	return;
}

void CFEngine::DlgMouseUp()
{
	if(DlgHold==DLG_HOLD_ANSW)
		if((DlgCurAnsw>=0)&&(DlgCurAnsw<all_answers))
			if((cur_x>=DlgAnsw[0]+DlgNextAnswX*DlgCurAnsw+DlgX)&&(cur_y>=DlgAnsw[1]+DlgNextAnswY*DlgCurAnsw+DlgY)&&
				(cur_x<=DlgAnsw[2]+DlgNextAnswX*DlgCurAnsw+DlgX)&&(cur_y<=DlgAnsw[3]+DlgNextAnswY*DlgCurAnsw+DlgY))
					Net_SendTalk(TargetID,DlgCurAnsw);

	DlgCurAnsw=-1;
	DlgHold=DLG_HOLD_NONE;
}

int CFEngine::LoadDialogFromFile(CrID id_npc, DWORD id_dialog, char* dialog)
{
//определяем путь
	char path_text[64];
	sprintf(path_text,"%s%d.dlg",PATH_TEXT_FILES,id_npc);
//читаем тексты
	char key1[24];
	sprintf(key1,"%d",id_dialog);
	GetPrivateProfileString("dialogs",key1,"Err",dialog,MAX_DIALOG_TEXT,path_text);
	if(!stricmp(dialog,"Err")) return 0;
//считаем кол-во вариантов
	int all_texts=0;
	int find_char=0;
	for(find_char=0; find_char<strlen(dialog); find_char++)
		if(dialog[find_char]=='%' && dialog[find_char+1]=='%')
		{
			all_texts++;
			find_char++;
		}
//	if(!all_texts) return 1;
//выбираем рандомный текст
	int find_text=random((all_texts+1));
//создаем временный диалог
	char temp_dialog[MAX_DIALOG_TEXT+1];
	strcpy(temp_dialog,dialog);
//форматируем текст
	/*char* text_f;
	bool none=true;
	while(void)
	{
		if((text_f=strstr(temp_dialog,"@pname"))) {  }
	}*/
//выдергиваем текст
	int cur_text=0;
	int cur_char=0;
	for(find_char=0; find_char<strlen(temp_dialog); find_char++)
	{
		if(cur_text==find_text)
		{
			if(temp_dialog[find_char]=='%' && temp_dialog[find_char+1]=='%') break;

			if(temp_dialog[find_char]=='#') { dialog[cur_char++]='\n'; continue; }

			if(temp_dialog[find_char]=='@')
			{
				char cr_textf[MAX_NAME+1];
				sscanf(&temp_dialog[++find_char],"%s",&cr_textf);

				int cr_textf_cur=strlen(cr_textf);
				for(;;) { if(!cr_textf_cur) break; if(cr_textf[--cr_textf_cur]=='@') break; }
				if(!cr_textf_cur) continue;
				cr_textf[cr_textf_cur]=0;

				if(!stricmp(cr_textf,"pname")) { strcpy(&dialog[cur_char],lpChosen->name); find_char+=6; cur_char+=strlen(lpChosen->name); }
				else if(!stricmp(cr_textf,"pcases0")) { strcpy(&dialog[cur_char],lpChosen->cases[0]); find_char+=8; cur_char+=strlen(lpChosen->cases[0]); }
				else if(!stricmp(cr_textf,"pcases1")) { strcpy(&dialog[cur_char],lpChosen->cases[1]); find_char+=8; cur_char+=strlen(lpChosen->cases[0]); }
				else if(!stricmp(cr_textf,"pcases2")) { strcpy(&dialog[cur_char],lpChosen->cases[2]); find_char+=8; cur_char+=strlen(lpChosen->cases[0]); }
				else if(!stricmp(cr_textf,"pcases3")) { strcpy(&dialog[cur_char],lpChosen->cases[3]); find_char+=8; cur_char+=strlen(lpChosen->cases[0]); }
				else if(!stricmp(cr_textf,"pcases4")) { strcpy(&dialog[cur_char],lpChosen->cases[4]); find_char+=8; cur_char+=strlen(lpChosen->cases[0]); }
			}

			dialog[cur_char++]=temp_dialog[find_char];

			continue;
		}

		if(temp_dialog[find_char]=='%' && temp_dialog[find_char+1]=='%')
		{
			find_char++;
			cur_text++;

			if(temp_dialog[find_char+1]=='#') find_char++;
		}
	}

	dialog[cur_char]=0;

	return 1;
}

void CFEngine::LMenuTryCreate()
{
	if(GetTickCount()-LMenu_start_time < LMENU_SHOW_TIME) return;
	if(IsLMenu()) return;

	SetLMenu(LMENU_OFF);
	LMenu_nodes=NULL;
	LMenu_cur_x=cur_x;
	LMenu_cur_y=cur_y;

	switch (screen_mode)
	{
	case SCREEN_MAIN:
		if((TargetID=GetMouseCritter(cur_x,cur_y)))
		{
			if(TargetID>MAX_NPC) { SetLMenu(LMENU_PLAYER);	LMenu_nodes=&LMenu_player_nodes;	}
			else if(TargetID>0)	{ SetLMenu(LMENU_NPC);		LMenu_nodes=&LMenu_npc_nodes;		}
		}
		else if((TargetID=GetMouseItem(cur_x,cur_y)))
		{
			SetLMenu(LMENU_ITEM);
			LMenu_nodes=&LMenu_item_nodes;
		}
		break;
	case SCREEN_INVENTORY:

		break;
	}

	if(!LMenu_nodes) 
	{
		LMenu_start_time=GetTickCount();
		SetLMenu(LMENU_OFF);
		return;
	}

	LMenu_cur_node=0;
	LMenu_try_activated=false;	
}

void CFEngine::LMenuDraw()
{
	if(!IsLMenu()) return;
	if(!LMenu_nodes) { WriteLog("!!!WARNING!!! Отрисовка ЛМеню - невалидный указатель\n"); return; }

	int count_node=0;
	for(LMenu_list::iterator it_l=LMenu_nodes->begin(); it_l!=LMenu_nodes->end(); it_l++)
	{
		switch (*it_l)
		{
		case LMENU_NODE_LOOK:
			if(count_node==LMenu_cur_node)
				sm.DrawSprite(lm_look_on,LMenu_cur_x,LMenu_cur_y+LMenu_node_height*count_node,COLOR_DEFAULT);
			else
				sm.DrawSprite(lm_look_off,LMenu_cur_x,LMenu_cur_y+LMenu_node_height*count_node,COLOR_DEFAULT);
			break;
		case LMENU_NODE_TALK:
			if(count_node==LMenu_cur_node)
				sm.DrawSprite(lm_talk_on,LMenu_cur_x,LMenu_cur_y+LMenu_node_height*count_node,COLOR_DEFAULT);
			else
				sm.DrawSprite(lm_talk_off,LMenu_cur_x,LMenu_cur_y+LMenu_node_height*count_node,COLOR_DEFAULT);
			break;
		case LMENU_NODE_BREAK:
			if(count_node==LMenu_cur_node)
				sm.DrawSprite(lm_break_on,LMenu_cur_x,LMenu_cur_y+LMenu_node_height*count_node,COLOR_DEFAULT);
			else
				sm.DrawSprite(lm_break_off,LMenu_cur_x,LMenu_cur_y+LMenu_node_height*count_node,COLOR_DEFAULT);
			break;
		default:
			WriteLog("!!!WORNING!!! Отрисовка ЛМеню - неизвестное состояние меню\n");
			break;
		}

		count_node++;
	}
}

void CFEngine::LMenuMouseMove()
{
	if(!LMenu_nodes) { WriteLog("!!!WARNING!!! Движение ЛМеню - невалидный указатель\n"); return; }

	LMenu_cur_node=(cur_y-LMenu_cur_y)/LMenu_node_height;
	if(LMenu_cur_node<0) LMenu_cur_node=0;
	if(LMenu_cur_node>LMenu_nodes->size()-1) LMenu_cur_node=LMenu_nodes->size()-1;
}

void CFEngine::LMenuMouseUp()
{
	LMenu_list::iterator it_l=LMenu_nodes->begin();
	it_l+=LMenu_cur_node;

	switch (screen_mode)
	{
	case SCREEN_MAIN:
		switch (*it_l)
		{
		case LMENU_NODE_LOOK:
			AddMess(COLOR_TEXT_DEFAULT,"Вы чёто увидели...");
			break;
		case LMENU_NODE_TALK:
			if(lpChosen->IsFree()) Net_SendTalk(TargetID,0);
			break;
		}
		break;
	case SCREEN_INVENTORY:
		
		break;
	}

	cur_x=LMenu_cur_x;
	cur_y=LMenu_cur_y;

	SetLMenu(LMENU_OFF);
	LMenu_try_activated=false;
}

void CFEngine::SetScreen(BYTE new_screen)
{
	screen_mode=new_screen;

	SetCur(CUR_DEFAULT);

	switch(screen_mode)
	{
	case SCREEN_LOGIN:
		break;
	case SCREEN_REGISTRATION:
		break;
	case SCREEN_MAIN:
		break;
	case SCREEN_INVENTORY:
		break;
	case SCREEN_LOCAL_MAP:
		break;
	case SCREEN_GLOBAL_MAP:
		break;
	case SCREEN_DIALOG_NPC:
		DlgMain[0]=0;
		DlgMain[1]=0;
		DlgMain[2]=DlgBegin[2];
		DlgMain[3]=DlgBegin[3]+DlgEnd[3]+DlgNextAnswY*all_answers;

		if(DlgY+DlgMain[3]>IntY) DlgY=IntY-DlgMain[3];
	case SCREEN_PIP_BOY:
		break;
	}
}

void CFEngine::SetCur(BYTE new_cur)
{
	cur_mode=new_cur;

	if(cur_mode==CUR_DEFAULT) cur=cur_def;
	if(cur_mode==CUR_WAIT) cur=cur_wait;

	switch(screen_mode)
	{
	case SCREEN_LOGIN:
		break;
	case SCREEN_REGISTRATION:
		break;
	case SCREEN_MAIN:
		break;
	case SCREEN_INVENTORY:
		break;
	case SCREEN_LOCAL_MAP:
		break;
	case SCREEN_GLOBAL_MAP:
		break;
	case SCREEN_DIALOG_NPC:
		break;
	case SCREEN_PIP_BOY:
		break;
	}
}

void CFEngine::CreateStringsParamsMaps()
{
	stats_str_map.insert(params_str_map::value_type(ST_STRENGHT,			"Сила"));
	stats_str_map.insert(params_str_map::value_type(ST_PERCEPTION,			"Восприятие"));
	stats_str_map.insert(params_str_map::value_type(ST_ENDURANCE,			"Выносливость"));
	stats_str_map.insert(params_str_map::value_type(ST_CHARISMA,			"Обояние"));
	stats_str_map.insert(params_str_map::value_type(ST_INTELLECT,			"Ум"));
	stats_str_map.insert(params_str_map::value_type(ST_AGILITY,				"Координация"));
	stats_str_map.insert(params_str_map::value_type(ST_LUCK,				"Удача"));
	stats_str_map.insert(params_str_map::value_type(ST_MAX_LIFE,			"Максимальные баллы жизней"));
	stats_str_map.insert(params_str_map::value_type(ST_MAX_COND,			"Максимальная кондиция"));
	stats_str_map.insert(params_str_map::value_type(ST_ARMOR_CLASS,			"Класс брони"));
	stats_str_map.insert(params_str_map::value_type(ST_MELEE_DAMAGE,		"Вред невооруженный"));
	stats_str_map.insert(params_str_map::value_type(ST_WEAPON_DAMAGE,		"Вред оружием"));
	stats_str_map.insert(params_str_map::value_type(ST_CARRY_WEIGHT,		"Максимальный груз"));
	stats_str_map.insert(params_str_map::value_type(ST_SEQUENCE,			"Реакция"));
	stats_str_map.insert(params_str_map::value_type(ST_HEALING_RATE,		"Лечение"));
	stats_str_map.insert(params_str_map::value_type(ST_CRITICAL_CHANCE,		"Критический шанс"));
	stats_str_map.insert(params_str_map::value_type(ST_MAX_CRITICAL,		"Максимальный критический шанс"));
	stats_str_map.insert(params_str_map::value_type(ST_INGURE_ABSORB,		"Порог ранения"));
	stats_str_map.insert(params_str_map::value_type(ST_LASER_ABSORB,		"Порог повреждения лазером"));
	stats_str_map.insert(params_str_map::value_type(ST_FIRE_ABSORB,			"Порог повреждения огнем"));
	stats_str_map.insert(params_str_map::value_type(ST_PLASMA_ABSORB,		"Порог повреждения плазмой"));
	stats_str_map.insert(params_str_map::value_type(ST_ELECTRO_ABSORB,		"Порог повреждения электричеством"));
	stats_str_map.insert(params_str_map::value_type(ST_EMP_ABSORB,			"Порог повреждения ЕМП"));
	stats_str_map.insert(params_str_map::value_type(ST_BLAST_ABSORB,		"Порог повреждения при взрыве"));
	stats_str_map.insert(params_str_map::value_type(ST_INGURE_RESIST,		"Сопротевляемость ранению"));
	stats_str_map.insert(params_str_map::value_type(ST_LASER_RESIST,		"Сопротивляемость ранению лазером"));
	stats_str_map.insert(params_str_map::value_type(ST_FIRE_RESIST,			"Сопротивляемость ранению огнем"));
	stats_str_map.insert(params_str_map::value_type(ST_PLASMA_RESIST,		"Сопротивляемость ранению плазмой"));
	stats_str_map.insert(params_str_map::value_type(ST_ELECTRO_RESIST,		"Сопротивляемость ранению электричеством"));
	stats_str_map.insert(params_str_map::value_type(ST_EMP_RESIST,			"Сопротивляемость ранению ЕМП"));
	stats_str_map.insert(params_str_map::value_type(ST_BLAST_RESIST,		"Сопротивляемость ранению при взрыве"));
	stats_str_map.insert(params_str_map::value_type(ST_RADIATION_RESISTANCE,"Сопротивляемость радиации"));
	stats_str_map.insert(params_str_map::value_type(ST_POISON_RESISTANCE,	"Сопротивляемость ядам"));
	stats_str_map.insert(params_str_map::value_type(ST_AGE,					"Возраст"));
	stats_str_map.insert(params_str_map::value_type(ST_GENDER,				"Пол"));
	stats_str_map.insert(params_str_map::value_type(ST_CURRENT_HP,			"Текущие баллы жизни"));
	stats_str_map.insert(params_str_map::value_type(ST_POISONING_LEVEL,		"Текужий уровень ядов"));
	stats_str_map.insert(params_str_map::value_type(ST_RADIATION_LEVEL,		"Текущий уровень радиации"));
	stats_str_map.insert(params_str_map::value_type(ST_CURRENT_STANDART,	"Текущая кондиция"));

	skills_str_map.insert(params_str_map::value_type(SK_SMALL_GUNS,			"Малое оружие"));
	skills_str_map.insert(params_str_map::value_type(SK_BIG_GUNS,			"Большое оружие"));
	skills_str_map.insert(params_str_map::value_type(SK_ENERGY_WEAPONS,		"Энергитическое оружие"));
	skills_str_map.insert(params_str_map::value_type(SK_UNARMED,			"Безоружный"));
	skills_str_map.insert(params_str_map::value_type(SK_MELEE_WEAPONS,		"Холодное оружие"));
	skills_str_map.insert(params_str_map::value_type(SK_THROWING,			"Метательное"));
	skills_str_map.insert(params_str_map::value_type(SK_FIRST_AID,			"Первая помощь"));
	skills_str_map.insert(params_str_map::value_type(SK_DOCTOR,				"Доктор"));
	skills_str_map.insert(params_str_map::value_type(SK_SNEAK,				"Скрытность"));
	skills_str_map.insert(params_str_map::value_type(SK_LOCKPICK,			"Взломщик"));
	skills_str_map.insert(params_str_map::value_type(SK_STEAL,				"Воровство"));
	skills_str_map.insert(params_str_map::value_type(SK_TRAPS,				"Ловушки"));
	skills_str_map.insert(params_str_map::value_type(SK_SCIENCE,			"Наука"));
	skills_str_map.insert(params_str_map::value_type(SK_REPAIR,				"Починка"));
	skills_str_map.insert(params_str_map::value_type(SK_SPEECH,				"Речь"));
	skills_str_map.insert(params_str_map::value_type(SK_BARTER,				"Бартер"));
	skills_str_map.insert(params_str_map::value_type(SK_GAMBLING,			"Игра"));
	skills_str_map.insert(params_str_map::value_type(SK_OUTDOORSMAN,		"Скаут"));

	perks_str_map.insert(params_str_map::value_type(PE_FAST_METABOLISM,		"1"));
	perks_str_map.insert(params_str_map::value_type(PE_BRUISER,				"1"));
	perks_str_map.insert(params_str_map::value_type(PE_SMALL_FRAME,			"1"));
	perks_str_map.insert(params_str_map::value_type(PE_ONE_HANDER,			"1"));
	perks_str_map.insert(params_str_map::value_type(PE_FINESSE,				"1"));
	perks_str_map.insert(params_str_map::value_type(PE_KAMIKAZE,			""));
	perks_str_map.insert(params_str_map::value_type(PE_HEAVY_HANDED,		""));
	perks_str_map.insert(params_str_map::value_type(PE_FAST_SHOT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_BLOODY_MESS,			""));
	perks_str_map.insert(params_str_map::value_type(PE_JINXED,				""));
	perks_str_map.insert(params_str_map::value_type(PE_GOOD_NATURED,		""));
	perks_str_map.insert(params_str_map::value_type(PE_CHEM_RELIANT,		""));
	perks_str_map.insert(params_str_map::value_type(PE_CHEM_RESISTANT,		""));
	perks_str_map.insert(params_str_map::value_type(PE_NIGHT_PERSON,		""));
	perks_str_map.insert(params_str_map::value_type(PE_SKILLED,				""));
	perks_str_map.insert(params_str_map::value_type(PE_GIFTED,				""));
	perks_str_map.insert(params_str_map::value_type(PE_AWARENESS,			""));
	perks_str_map.insert(params_str_map::value_type(PE_A_MELEE_ATT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_A_MELEE_DAM,			""));
	perks_str_map.insert(params_str_map::value_type(PE_A_MOVE,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_DAM,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_SPEED,				""));
	perks_str_map.insert(params_str_map::value_type(PE_PASS_FRONT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_RAPID_HEAL,			""));
	perks_str_map.insert(params_str_map::value_type(PE_MORE_CRIT_DAM,		""));
	perks_str_map.insert(params_str_map::value_type(PE_NIGHT_SIGHT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_PRESENCE,			""));
	perks_str_map.insert(params_str_map::value_type(PE_RES_NUKLEAR,			""));
	perks_str_map.insert(params_str_map::value_type(PE_ENDURENCE,			""));
	perks_str_map.insert(params_str_map::value_type(PE_STR_BACK,			""));
	perks_str_map.insert(params_str_map::value_type(PE_MARKSMAN,			""));
	perks_str_map.insert(params_str_map::value_type(PE_STEALHING,			""));
	perks_str_map.insert(params_str_map::value_type(PE_LIFEFULL,			""));
	perks_str_map.insert(params_str_map::value_type(PE_MERCHANT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_FORMED,				""));
	perks_str_map.insert(params_str_map::value_type(PE_HEALER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_TR_DIGGER,			""));
	perks_str_map.insert(params_str_map::value_type(PE_BEST_HITS,			""));
	perks_str_map.insert(params_str_map::value_type(PE_COMPASION,			""));
	perks_str_map.insert(params_str_map::value_type(PE_KILLER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_SNIPER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_SILENT_DEATH,		""));
	perks_str_map.insert(params_str_map::value_type(PE_C_FIGHTER,			""));
	perks_str_map.insert(params_str_map::value_type(PE_MIND_BLOCK,			""));
	perks_str_map.insert(params_str_map::value_type(PE_PROLONGATION_LIFE,	""));
	perks_str_map.insert(params_str_map::value_type(PE_RECOURCEFULNESS,		""));
	perks_str_map.insert(params_str_map::value_type(PE_SNAKE_EATER,			""));
	perks_str_map.insert(params_str_map::value_type(PE_REPEARER,			""));
	perks_str_map.insert(params_str_map::value_type(PE_MEDIC,				""));
	perks_str_map.insert(params_str_map::value_type(PE_SKILLED_THIEF,		""));
	perks_str_map.insert(params_str_map::value_type(PE_SPEAKER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_GUTCHER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_UNKNOWN_1,			""));
	perks_str_map.insert(params_str_map::value_type(PE_PICK_POCKER,			""));
	perks_str_map.insert(params_str_map::value_type(PE_GHOST,				""));
	perks_str_map.insert(params_str_map::value_type(PE_CHAR_CULT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_THIFER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_DISCOVER,			""));
	perks_str_map.insert(params_str_map::value_type(PE_OVERROAD,			""));
	perks_str_map.insert(params_str_map::value_type(PE_ANIMAL_FRIENDSHIP,	""));
	perks_str_map.insert(params_str_map::value_type(PE_SCOUT,				""));
	perks_str_map.insert(params_str_map::value_type(PE_MIST_CHAR,			""));
	perks_str_map.insert(params_str_map::value_type(PE_RANGER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_PICK_POCKET_2,		""));
	perks_str_map.insert(params_str_map::value_type(PE_INTERLOCUTER,		""));
	perks_str_map.insert(params_str_map::value_type(PE_NOVICE,				""));
	perks_str_map.insert(params_str_map::value_type(PE_PRIME_SKILL,			""));
	perks_str_map.insert(params_str_map::value_type(PE_MUTATION,			""));
	perks_str_map.insert(params_str_map::value_type(PE_NARC_NUKACOLA,		""));
	perks_str_map.insert(params_str_map::value_type(PE_NARC_BUFFOUT,		""));
	perks_str_map.insert(params_str_map::value_type(PE_NARC_MENTAT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_NARC_PSYHO,			""));
	perks_str_map.insert(params_str_map::value_type(PE_NARC_RADAWAY,		""));
	perks_str_map.insert(params_str_map::value_type(PE_DISTANT_WEAP,		""));
	perks_str_map.insert(params_str_map::value_type(PE_ACCURARY_WEAP,		""));
	perks_str_map.insert(params_str_map::value_type(PE_PENETRATION_WEAP,	""));
	perks_str_map.insert(params_str_map::value_type(PE_KILLER_WEAP,			""));
	perks_str_map.insert(params_str_map::value_type(PE_ENERGY_ARMOR,		""));
	perks_str_map.insert(params_str_map::value_type(PE_BATTLE_ARMOR,		""));
	perks_str_map.insert(params_str_map::value_type(PE_WEAP_RANGE,			""));
	perks_str_map.insert(params_str_map::value_type(PE_RAPID_RELOAD,		""));
	perks_str_map.insert(params_str_map::value_type(PE_NIGHT_SPYGLASS,		""));
	perks_str_map.insert(params_str_map::value_type(PE_FLAMER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_APA_I,				""));
	perks_str_map.insert(params_str_map::value_type(PE_APA_II,				""));
	perks_str_map.insert(params_str_map::value_type(PE_FORCEAGE,			""));
	perks_str_map.insert(params_str_map::value_type(PE_DEADLY_NARC,			""));
	perks_str_map.insert(params_str_map::value_type(PE_CHARMOLEANCE,		""));
	perks_str_map.insert(params_str_map::value_type(PE_GEKK_SKINER,			""));
	perks_str_map.insert(params_str_map::value_type(PE_SKIN_ARMOR,			""));
	perks_str_map.insert(params_str_map::value_type(PE_A_SKIN_ARMOR,		""));
	perks_str_map.insert(params_str_map::value_type(PE_SUPER_ARMOR,			""));
	perks_str_map.insert(params_str_map::value_type(PE_A_SUPER_ARMOR,		""));
	perks_str_map.insert(params_str_map::value_type(PE_VAULT_INOCUL,		""));
	perks_str_map.insert(params_str_map::value_type(PE_ADRENALIN_RUSH,		""));
	perks_str_map.insert(params_str_map::value_type(PE_CAREFULL,			""));
	perks_str_map.insert(params_str_map::value_type(PE_INTELEGENCE,			""));
	perks_str_map.insert(params_str_map::value_type(PE_PYROKASY,			""));
	perks_str_map.insert(params_str_map::value_type(PE_DUDE,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_STR,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_PER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_END,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_CHA,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_INT,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_AGL,				""));
	perks_str_map.insert(params_str_map::value_type(PE_A_LUC,				""));
	perks_str_map.insert(params_str_map::value_type(PE_PURERER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_IMAG,				""));
	perks_str_map.insert(params_str_map::value_type(PE_EVASION,				""));
	perks_str_map.insert(params_str_map::value_type(PE_DROSHKADRAT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_KARMA_GLOW,			""));
	perks_str_map.insert(params_str_map::value_type(PE_SILENT_STEPS,		""));
	perks_str_map.insert(params_str_map::value_type(PE_ANATOMY,				""));
	perks_str_map.insert(params_str_map::value_type(PE_CHAMER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_ORATOR,				""));
	perks_str_map.insert(params_str_map::value_type(PE_PACKER,				""));
	perks_str_map.insert(params_str_map::value_type(PE_EDD_GAYAN_MANIAC,	""));
	perks_str_map.insert(params_str_map::value_type(PE_FAST_REGENERATION,	""));
	perks_str_map.insert(params_str_map::value_type(PE_VENDOR,				""));
	perks_str_map.insert(params_str_map::value_type(PE_STONE_WALL,			""));
	perks_str_map.insert(params_str_map::value_type(PE_THIEF_AGAIN,			""));
	perks_str_map.insert(params_str_map::value_type(PE_WEAPON_SKILL,		""));
	perks_str_map.insert(params_str_map::value_type(PE_MAKE_VAULT,			""));
	perks_str_map.insert(params_str_map::value_type(PE_ALC_BUFF_1,			""));
	perks_str_map.insert(params_str_map::value_type(PE_ALC_BUFF_2,			""));
/*	perks_str_map.insert(params_str_map::value_type(!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	perks_str_map.insert(params_str_map::value_type("!",!));
	*/
	perks_str_map.insert(params_str_map::value_type(PE_HIDE_MODE,			""));

/*	object_map.insert(params_str_map::value_type(OBJ_TYPE_ARMOR",			OBJ_TYPE_ARMOR));
	object_map.insert(params_str_map::value_type(OBJ_TYPE_CONTAINER",		OBJ_TYPE_CONTAINER));
	object_map.insert(params_str_map::value_type(OBJ_TYPE_DRUG",			OBJ_TYPE_DRUG));
	object_map.insert(params_str_map::value_type(OBJ_TYPE_WEAPON",			OBJ_TYPE_WEAPON));
	object_map.insert(params_str_map::value_type(OBJ_TYPE_AMMO",			OBJ_TYPE_AMMO));
	object_map.insert(params_str_map::value_type(OBJ_TYPE_MISC",			OBJ_TYPE_MISC));
	object_map.insert(params_str_map::value_type(OBJ_TYPE_KEY",			OBJ_TYPE_KEY));
	object_map.insert(params_str_map::value_type(OBJ_NAME",				OBJ_NAME));
	object_map.insert(params_str_map::value_type(OBJ_INFO",				OBJ_INFO));
	object_map.insert(params_str_map::value_type(OBJ_TIME_SHOW",			OBJ_TIME_SHOW));
	object_map.insert(params_str_map::value_type(OBJ_TIME_HIDE",			OBJ_TIME_HIDE));
	object_map.insert(params_str_map::value_type(OBJ_DISTANCE_LIGHT",		OBJ_DISTANCE_LIGHT));
	object_map.insert(params_str_map::value_type(OBJ_INTENSITY_LIGHT",		OBJ_INTENSITY_LIGHT));
	object_map.insert(params_str_map::value_type(OBJ_PASSED",				OBJ_PASSED));
	object_map.insert(params_str_map::value_type(OBJ_RAKED",				OBJ_RAKED));
	object_map.insert(params_str_map::value_type(OBJ_TRANSPARENT",			OBJ_TRANSPARENT));
	object_map.insert(params_str_map::value_type(OBJ_CAN_USE",				OBJ_CAN_USE));
	object_map.insert(params_str_map::value_type(OBJ_CAN_PICK_UP",			OBJ_CAN_PICK_UP));
	object_map.insert(params_str_map::value_type(OBJ_CAN_USE_ON_SMTH",		OBJ_CAN_USE_ON_SMTH));
	object_map.insert(params_str_map::value_type(OBJ_HIDDEN",				OBJ_HIDDEN));
	object_map.insert(params_str_map::value_type(OBJ_WEIGHT",				OBJ_WEIGHT));
	object_map.insert(params_str_map::value_type(OBJ_SIZE",				OBJ_SIZE));
	object_map.insert(params_str_map::value_type(OBJ_TWOHANDS",			OBJ_TWOHANDS));
	object_map.insert(params_str_map::value_type(OBJ_PIC_MAP",			OBJ_PIC_MAP));
	object_map.insert(params_str_map::value_type(OBJ_PIC_INV",				OBJ_PIC_INV));
	object_map.insert(params_str_map::value_type(OBJ_SOUND",				OBJ_SOUND));
	object_map.insert(params_str_map::value_type(OBJ_LIVETIME",			OBJ_LIVETIME));
	object_map.insert(params_str_map::value_type(OBJ_COST",				OBJ_COST));
	object_map.insert(params_str_map::value_type(OBJ_MATERIAL",			OBJ_MATERIAL));
	object_map.insert(params_str_map::value_type(OBJ_ARM_ANIM0_MALE",		OBJ_ARM_ANIM0_MALE));
	object_map.insert(params_str_map::value_type(OBJ_ARM_ANIM0_MALE2",		OBJ_ARM_ANIM0_MALE2));
	object_map.insert(params_str_map::value_type(OBJ_ARM_ANIM0_FEMALE",	OBJ_ARM_ANIM0_FEMALE));
	object_map.insert(params_str_map::value_type(OBJ_ARM_ANIM0_FEMALE2",	OBJ_ARM_ANIM0_FEMALE2));
	object_map.insert(params_str_map::value_type(OBJ_ARM_AC",				OBJ_ARM_AC));
	object_map.insert(params_str_map::value_type(OBJ_ARM_PERK",			OBJ_ARM_PERK));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DR_NORMAL",		OBJ_ARM_DR_NORMAL));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DR_LASER",		OBJ_ARM_DR_LASER));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DR_FIRE",			OBJ_ARM_DR_FIRE));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DR_PLASMA",		OBJ_ARM_DR_PLASMA));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DR_ELECTR",		OBJ_ARM_DR_ELECTR));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DR_EMP",			OBJ_ARM_DR_EMP));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DR_EXPLODE",		OBJ_ARM_DR_EXPLODE));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DT_NORMAL",		OBJ_ARM_DT_NORMAL));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DT_LASER",		OBJ_ARM_DT_LASER));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DT_FIRE",			OBJ_ARM_DT_FIRE));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DT_PLASMA",		OBJ_ARM_DT_PLASMA));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DT_ELECTR",		OBJ_ARM_DT_ELECTR));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DT_EMP",			OBJ_ARM_DT_EMP));
	object_map.insert(params_str_map::value_type(OBJ_ARM_DT_EXPLODE",		OBJ_ARM_DT_EXPLODE));
	object_map.insert(params_str_map::value_type(OBJ_CONT_SIZE",			OBJ_CONT_SIZE));
	object_map.insert(params_str_map::value_type(OBJ_CONT_FLAG",			OBJ_CONT_FLAG));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_STAT0",			OBJ_DRUG_STAT0));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_STAT1",			OBJ_DRUG_STAT1));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_STAT2",			OBJ_DRUG_STAT2));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT0_S0",		OBJ_DRUG_AMOUNT0_S0));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT0_S1",		OBJ_DRUG_AMOUNT0_S1));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT0_S2",		OBJ_DRUG_AMOUNT0_S2));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_DURATION1",		OBJ_DRUG_DURATION1));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT1_S0",		OBJ_DRUG_AMOUNT1_S0));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT1_S1",		OBJ_DRUG_AMOUNT1_S1));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT1_S2",		OBJ_DRUG_AMOUNT1_S2));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_DURATION2",		OBJ_DRUG_DURATION2));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT2_S0",		OBJ_DRUG_AMOUNT2_S0));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT2_S1",		OBJ_DRUG_AMOUNT2_S1));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_AMOUNT2_S2",		OBJ_DRUG_AMOUNT2_S2));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_ADDICTION",		OBJ_DRUG_ADDICTION));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_W_EFFECT",		OBJ_DRUG_W_EFFECT));
	object_map.insert(params_str_map::value_type(OBJ_DRUG_W_ONSET",		OBJ_DRUG_W_ONSET));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_ANIM1",			OBJ_WEAP_ANIM1));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TIME_ACTIV",		OBJ_WEAP_TIME_ACTIV));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TIME_UNACTIV",	OBJ_WEAP_TIME_UNACTIV));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_VOL_HOLDER",		OBJ_WEAP_VOL_HOLDER));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_CALIBER",		OBJ_WEAP_CALIBER));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_VOL_HOLDER_E",	OBJ_WEAP_VOL_HOLDER_E));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_CALIBER_E",		OBJ_WEAP_CALIBER_E));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_CR_FAILTURE",	OBJ_WEAP_CR_FAILTURE));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TYPE_ATTACK",	OBJ_WEAP_TYPE_ATTACK));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_COUNT_ATTACK",	OBJ_WEAP_COUNT_ATTACK));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_SKILL",		OBJ_WEAP_PA_SKILL));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_HOLDER",		OBJ_WEAP_PA_HOLDER));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_PIC",			OBJ_WEAP_PA_PIC));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_DMG_MIN",		OBJ_WEAP_PA_DMG_MIN));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_DMG_MAX",		OBJ_WEAP_PA_DMG_MAX));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_MAX_DIST",	OBJ_WEAP_PA_MAX_DIST));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_EFF_DIST",	OBJ_WEAP_PA_EFF_DIST));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_ANIM2",		OBJ_WEAP_PA_ANIM2));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_TIME",		OBJ_WEAP_PA_TIME));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_AIM",			OBJ_WEAP_PA_AIM));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_ROUND",		OBJ_WEAP_PA_ROUND));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_PA_REMOVE",		OBJ_WEAP_PA_REMOVE));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_SKILL",		OBJ_WEAP_SA_SKILL));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_HOLDER",		OBJ_WEAP_SA_HOLDER));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_PIC",			OBJ_WEAP_SA_PIC));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_DMG_MIN",		OBJ_WEAP_SA_DMG_MIN));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_DMG_MAX",		OBJ_WEAP_SA_DMG_MAX));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_MAX_DIST",	OBJ_WEAP_SA_MAX_DIST));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_EFF_DIST",	OBJ_WEAP_SA_EFF_DIST));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_ANIM2",		OBJ_WEAP_SA_ANIM2));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_TIME",		OBJ_WEAP_SA_TIME));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_AIM",			OBJ_WEAP_SA_AIM));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_ROUND",		OBJ_WEAP_SA_ROUND));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_SA_REMOVE",		OBJ_WEAP_SA_REMOVE));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_SKILL",		OBJ_WEAP_TA_SKILL));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_HOLDER",		OBJ_WEAP_TA_HOLDER));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_PIC",			OBJ_WEAP_TA_PIC));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_DMG_MIN",		OBJ_WEAP_TA_DMG_MIN));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_DMG_MAX",		OBJ_WEAP_TA_DMG_MAX));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_MAX_DIST",	OBJ_WEAP_TA_MAX_DIST));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_EFF_DIST",	OBJ_WEAP_TA_EFF_DIST));	
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_ANIM2",		OBJ_WEAP_TA_ANIM2));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_TIME",		OBJ_WEAP_TA_TIME));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_AIM",			OBJ_WEAP_TA_AIM));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_ROUND",		OBJ_WEAP_TA_ROUND));
	object_map.insert(params_str_map::value_type(OBJ_WEAP_TA_REMOVE",		OBJ_WEAP_TA_REMOVE));
	object_map.insert(params_str_map::value_type(OBJ_AMMO_CALIBER",		OBJ_AMMO_CALIBER));
	object_map.insert(params_str_map::value_type(OBJ_AMMO_TYPE_DAMAGE",	OBJ_AMMO_TYPE_DAMAGE));
	object_map.insert(params_str_map::value_type(OBJ_AMMO_QUANTITY",		OBJ_AMMO_QUANTITY));
	object_map.insert(params_str_map::value_type(OBJ_AMMO_AC",				OBJ_AMMO_AC));
	object_map.insert(params_str_map::value_type(OBJ_AMMO_DR",				OBJ_AMMO_DR));
	object_map.insert(params_str_map::value_type(OBJ_AMMO_DM",				OBJ_AMMO_DM));
	object_map.insert(params_str_map::value_type(OBJ_AMMO_DD",				OBJ_AMMO_DD));
*/
}

//FEngine_Iface.cpp by Cvet