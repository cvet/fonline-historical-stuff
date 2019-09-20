#include "stdafx.h"

/********************************************************************
	created:	18:08:2004   23:48; updated: begin of 2007

	author:		Oleg Mareskin
	add/edit:	Anton Tsvetinsky
	
	purpose:	
*********************************************************************/


#include "FOServ.h"

#include "main.h"
#include "socials.h"

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size);
void zlib_free(void *opaque, void *address);

#define MAX_CLIENTS 100

CrID busy[MAX_CLIENTS];

//!Cvet изменил концепцию ++++
#define CMD_SAY		0

//команды имеют префикс ~
#define CMD_EXIT	1 //выход

struct cmdlist_def
{
	char cmd[15];
	int id;
};

const cmdlist_def cmdlist[]=
{
	{"",0},
	{"конец",CMD_EXIT},
	{"",0},
};
//!Cvet ----

HANDLE hDump;

CServer::CServer()
{
	Active=0;
	sql.mySQL=NULL;
	outLEN=4096;
	outBUF=new char[outLEN];
	last_id=0; // Никто не присоединился
	for(int i=0;i<MAX_CLIENTS;i++) busy[i]=0;
}

CServer::~CServer()
{
	Finish();
	SAFEDELA(outBUF);
	CloseHandle(hDump);
}

void CServer::ClearClients() //!Cvet edit
{
	//!Cvet сохраняем данные объектов !!!!!!!!!!!!!!!!!!! надо вынести из деструктора
	SaveAllObj();

	//!Cvet сохраняем данные клиентов !!!!!!!!!!!!!!!! надо вынести из деструктора
	SaveAllDataPlayers();
	
	//!Cvet удаляем объекты
	for(dyn_map::iterator it2=all_obj.begin();it2!=all_obj.end();it2++)
	{
		delete (*it2).second;
	}
	all_obj.clear();

	//!Cvet удаляем клиентов
	cl_map::iterator it;
	for(it=cl.begin();it!=cl.end();it++)
	{
		if((*it).second->s!=NULL)
		{
			closesocket((*it).second->s);
			deflateEnd(&(*it).second->zstrm);
		}
		delete (*it).second;
	}
	cl.clear();

	//!Cvet удаляем НПЦ !!!!!!!!!!!!!!!!!!!!
	for(it=pc.begin();it!=pc.end();it++)
	{
//		delete (*it).second;
	}
	pc.clear();

	NumClients=0;
#ifndef FOSERVICE_VERSION
	SetEvent(hUpdateEvent);
#endif
}

//!Cvet ++++ изменил много чего
int CServer::ConnectClient(SOCKET serv)
{
	LogExecStr("Попытка соеденить нового клиента...");

    SOCKADDR_IN from;
	int addrsize=sizeof(from);
	
	SOCKET NewCl=accept(serv,(sockaddr*)&from,&addrsize);

	if(NewCl==INVALID_SOCKET) { LogExecStr("INVALID_SOCKET №%d\n",NewCl); return 0; }

	CLIENT* ncl=new CLIENT;
	ncl->s=NewCl;
	ncl->from=from;

    ncl->zstrm.zalloc = zlib_alloc;
    ncl->zstrm.zfree = zlib_free;
    ncl->zstrm.opaque = NULL;

	if(deflateInit(&ncl->zstrm,Z_DEFAULT_COMPRESSION)!=Z_OK)
	{
		LogExecStr("DeflateInit error forSockID=%d\n",NewCl);
		ncl->state=STATE_DISCONNECT; //!!!!!
		return 0;
	}

	int free_place=-1;
	for(int i=0;i<MAX_CLIENTS;i++) //проверяем есть ли свободный канал Для Игрока
	{
		if(!busy[i])
		{
			free_place=i; //опре-ся не занятый номер канала
			ncl->info.idchannel=i;
			busy[ncl->info.idchannel]=1;
			break;
		}
	}

	if(free_place==-1)
	{
		LogExecStr("Нет свободного канала\n",NewCl);
		ncl->state=STATE_DISCONNECT;
		return 0;
	}

	ncl->state=STATE_CONN;
	
	cl.insert(cl_map::value_type(ncl->info.idchannel,ncl));

   	NumClients++; //инкремент кол-ва подключенных клиентов

	LogExecStr("OK. Канал=%d. Всего клиентов в игре: %d\n",ncl->info.idchannel,NumClients);

#ifndef FOSERVICE_VERSION
	SetEvent(hUpdateEvent);
#endif

	return 1;
}

void CServer::DisconnectClient(CrID idchannel)
{
	LogExecStr("Отсоединяется клиент. Номер канала %d...", idchannel);

	cl_map::iterator it_ds=cl.find(idchannel);
	if(it_ds==cl.end())
	{
		LogExecStr("!!!WORNING!!! Клиент не найден\n");
		return;
	}

	closesocket((*it_ds).second->s);
	deflateEnd(&(*it_ds).second->zstrm);

	//Освобождение канала
	busy[idchannel]=0;

	if((*it_ds).second->info.cond!=COND_NOT_IN_GAME)
	{
		SETFLAG((*it_ds).second->info.flags,FCRIT_DISCONNECT);
		Send_Action((*it_ds).second,ACT_DISCONNECT,NULL);
	}
	else
	{
		LogExecStr(".1.");
		delete (*it_ds).second;
		LogExecStr(".2.");
	}

	//Удаление клиента из списка
	cl.erase(it_ds);

	NumClients--;

	LogExecStr("Отсоединение завершено. Всего клиентов в игре: %d\n",NumClients);
}

void CServer::RemoveCritter(CrID id)
{
	LogExecStr("Удаляем криттера id=%d\n",id);

	cl_map::iterator it=cr.find(id);
	if(it==cr.end()) { LogExecStr("!!!WORNING!!! RemoveCritter - клиент не найден id=%d\n",id); return; } // Значит не нашел обьекта на карте

	if((*it).second->info.map)
	{
		//Удаляем с тайла
		mm.ClearPlayer(&(*it).second->info);

		//удаляем со списка для клиентов на карте
		map_cr[(*it).second->info.map].erase(id);
	}

	delete (*it).second;
	cr.erase(it);

//	NumCritters--;

	LogExecStr("Криттер удален\n");

#ifndef FOSERVICE_VERSION
	SetEvent(hUpdateEvent);
#endif
}
//!Cvet ----

void CServer::RunGameLoop()
{
	if(!Active) return;

	TICK ticks;
	int delta;
	timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=0;
	CLIENT* c;

	ticks=GetTickCount();

	LogExecStr("***   Starting Game loop   ***\n");

//!Cvet сбор статистики +++
	loop_time=0;
	loop_cycles=0;
	loop_min=100;
	loop_max=0;

	lt_FDsel=0;
	lt_conn=0;
	lt_input=0;
	lt_proc_cl=0;
	lt_proc_pc=0;
	lt_output=0;
	lt_discon=0;

	lt_FDsel_max=0;
	lt_conn_max=0;
	lt_input_max=0;
	lt_proc_cl_max=0;
	lt_proc_pc_max=0;
	lt_output_max=0;
	lt_discon_max=0;

	lags_count=0;

	TICK lt_ticks,lt_ticks2;
//!Cvet ---

	while(!FOQuit)
	{
		ticks=GetTickCount()+100;

		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_ZERO(&exc_set);
		FD_SET(s,&read_set);
	
		for(cl_map::iterator it=cl.begin();it!=cl.end();it++)
		{
			c=(*it).second;
			if(c->state!=STATE_DISCONNECT)
			{
				FD_SET(c->s,&read_set);
				FD_SET(c->s,&write_set);
				FD_SET(c->s,&exc_set);
			}
		}

		select(0,&read_set,&write_set,&exc_set,&tv);

		lt_ticks=GetTickCount();
		lt_FDsel+=lt_ticks-(ticks-100);

	//Новое подключение клиента
		if(FD_ISSET(s,&read_set))
		{
			ConnectClient(s);
		}

		lt_ticks2=lt_ticks;
		lt_ticks=GetTickCount();
		lt_conn+=lt_ticks-lt_ticks2;
	
	//!Cvet Прием данных от клиентов
		for(it=cl.begin();it!=cl.end();)
		{
			c=(*it).second;
			if((FD_ISSET(c->s,&read_set))&&(c->state!=STATE_DISCONNECT))
				if(!Input(c)) c->state=STATE_DISCONNECT;
			it++;
		}

		lt_ticks2=lt_ticks;
		lt_ticks=GetTickCount();
		lt_input+=lt_ticks-lt_ticks2;

	//Обработка данных клиентов
		for(it=cl.begin();it!=cl.end();it++)
		{
			c=(*it).second;
			if(c->state==STATE_DISCONNECT) continue;

			if(c->bin.pos) Process(c);
		}

		lt_ticks2=lt_ticks;
		lt_ticks=GetTickCount();
		lt_proc_cl+=lt_ticks-lt_ticks2;

	//Обработка НПЦ
		for(it=pc.begin();it!=pc.end();it++)
		{
			c=(*it).second;
			NPC_Process(c);
		}

		lt_ticks2=lt_ticks;
		lt_ticks=GetTickCount();
		lt_proc_pc+=lt_ticks-lt_ticks2;

	//Посылка данных клиентов
		for(it=cl.begin();it!=cl.end();it++)
		{
			c=(*it).second;
			if(FD_ISSET(c->s,&write_set)) Output(c);
		}

		lt_ticks2=lt_ticks;
		lt_ticks=GetTickCount();
		lt_output+=lt_ticks-lt_ticks2;

	//Убирание отключенных клиентов 
		for(it=cl.begin();it!=cl.end();)
		{
			c=(*it).second;
			if(c->state==STATE_DISCONNECT) 
			{
				it++;
				DisconnectClient(c->info.idchannel);
				continue; 
			}
			it++;
		}

		lt_discon+=GetTickCount()-lt_ticks;

	//!Cvet сбор статистики
		DWORD loop_cur=GetTickCount()-(ticks-100);
		loop_time+=loop_cur;
		loop_cycles++;
		if(loop_cur > loop_max) loop_max=loop_cur;
		if(loop_cur < loop_min) loop_min=loop_cur;

	//если быстро справились, то спим
		delta=ticks-GetTickCount();
		if(delta>0)
		{
			Sleep(delta);
		}
		else lags_count++;//LogExecStr("\nLag for%d ms\n",-delta);
	}

	LogExecStr("***   Finishing Game loop   ***\n\n");
}

int CServer::Input(CLIENT* acl)
{
	UINT len=recv(acl->s,inBUF,2048,0);
	if(len==SOCKET_ERROR || !len) // если клиент отвалился
	{
		LogExecStr("SOCKET_ERROR forSockID=%d\n",acl->s);
		return 0;
	}

	if(len==2048 || (acl->bin.pos+len>=acl->bin.len))
	{
		LogExecStr("FLOOD_CONTROL forSockID=%d\n",acl->s);
		return 0; // если флудит игрок
	}

	acl->bin.push(inBUF,len);

	return 1;
}

void CServer::Process(CLIENT* acl) // Лист Событий
{
	MSGTYPE msg;

	if(acl->state==STATE_CONN) //!Cvet ++++
	{
		if(acl->bin.NeedProcess())
		{
			acl->bin >> msg;
		
			switch(msg) 
			{
			case NETMSG_LOGIN:
				Process_GetLogIn(acl);
				break;
			case NETMSG_CREATE_CLIENT:
				Process_CreateClient(acl);
				break;
			default:
				LogExecStr("Неправильное MSG: %d от SockID %d при приеме LOGIN или CREATE_CLIENT!\n",msg,acl->s);
				acl->state=STATE_DISCONNECT;
				Send_LoginMsg(acl,8);
				acl->bin.reset(); //!Cvet при неправильном пакете данных  - удаляеться весь список
				return;
			}
		}
		acl->bin.reset();
		return;
	} //!Cvet ----

	if(acl->state==STATE_LOGINOK) //!Cvet ++++
	{
		if(acl->bin.NeedProcess())
		{
			acl->bin >> msg;
		
			switch(msg) 
			{
			case NETMSG_SEND_GIVE_ME_MAP:
				Send_Map(acl,acl->info.map);
				break;
			case NETMSG_SEND_LOAD_MAP_OK:
				Process_MapLoaded(acl);
				break;
			default:
				LogExecStr("Неправильное MSG: %d от SockID %d при приеме NETMSG_SEND_GIVE_ME_MAP или NETMSG_SEND_LOAD_MAP_OK!\n",msg,acl->s);
				acl->state=STATE_DISCONNECT;
				Send_LoginMsg(acl,8);
				acl->bin.reset(); //!Cvet при неправильном пакете данных  - удаляеться весь список
				return;
			}
		}
		acl->bin.reset();
		return;
	} //!Cvet ----

	//!Cvet если игрок мертв
	if(acl->info.cond!=COND_LIFE)
	{
		acl->bin.reset();
		return;
	}

	while(acl->bin.NeedProcess())
	{
		acl->bin >> msg;
		
		switch(msg) 
		{
		case NETMSG_TEXT:
			Process_GetText(acl);
			break;
		case NETMSG_ROTATE:
			Process_Rotate(acl);
			break;
		case NETMSG_SEND_MOVE: // Перехватываем сообщение Клиента о Заявке на шаг
			Process_Move(acl);
			break;
		case NETMSG_SEND_USE_OBJECT: //!Cvet
			Process_UseObject(acl);
			break;
		case NETMSG_SEND_CHANGE_OBJECT: //!Cvet
			Process_ChangeObject(acl);
			break;
		case NETMSG_SEND_USE_SKILL: //!Cvet
			Process_UseSkill(acl);
			break;
		case NETMSG_SEND_TALK_NPC: //!Cvet
			Process_Talk_NPC(acl);
			break;
		case NETMSG_SEND_GET_TIME: //!Cvet
			Process_GetTime(acl);
			break;
//		case NETMSG_SEND_LOAD_MAP_OK: //!Cvet
//			Process_MapLoaded(acl);
//			break;
		default:
			LogExecStr("Wrong MSG: %d from SockID %d при приеме игровых сообщений!\n",msg,acl->s);
			//acl->state=STATE_DISCONNECT;
			acl->bin.reset(); //!Cvet при неправильном пакете данных  - удаляеться весь список
			return;
		}
	}
	acl->bin.reset();
}

//!Cvet ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void CServer::Process_CreateClient(CLIENT* acl)
{
	LogExecStr("РЕГИСТРАЦИЯ ИГРОКА\n");
	TICK tickStart=GetTickCount();

	int bi;

//ПРИЕМ ДАННЫХ
	//логин
	acl->bin.pop(acl->info.login,MAX_LOGIN);
	//пасс
	acl->bin.pop(acl->info.pass,MAX_LOGIN);
	//имя
	acl->bin.pop(acl->info.name,MAX_NAME);

	acl->info.name[MAX_NAME]=0;
	my_strlwr(acl->info.name);
	sql.Check(acl->info.name);
	//cases
	for(bi=0; bi<5; bi++)
	{
		acl->bin.pop(acl->info.cases[bi],MAX_NAME);

		acl->info.cases[bi][MAX_NAME]=0;
		my_strlwr(acl->info.cases[bi]);
		sql.Check(acl->info.cases[bi]);
	}
	//Обнуляем все статы, скиллы, перки
	for(bi=0; bi<ALL_STATS;  bi++) acl->info.st[bi]=0;
	for(bi=0; bi<ALL_SKILLS; bi++) acl->info.sk[bi]=0;
	for(bi=0; bi<ALL_PERKS;  bi++) acl->info.pe[bi]=0;
	//SPECIAL
	acl->bin >> acl->info.st[ST_STRENGHT	];
	acl->bin >> acl->info.st[ST_PERCEPTION	];
	acl->bin >> acl->info.st[ST_ENDURANCE	];
	acl->bin >> acl->info.st[ST_CHARISMA	];
	acl->bin >> acl->info.st[ST_INTELLECT	];
	acl->bin >> acl->info.st[ST_AGILITY		];
	acl->bin >> acl->info.st[ST_LUCK		];
	//возраст
	acl->bin >> acl->info.st[ST_AGE];
	//пол
	acl->bin >> acl->info.st[ST_GENDER];
	if(acl->bin.IsError())
	{
		LogExecStr("Wrong MSG data forProcess_CreateClient from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,8);
		return;
	}
//ПРОВЕРКА ДАННЫХ
	if(sql.Check(acl->info.login) || sql.Check(acl->info.pass))
	{
		LogExecStr("Запрещенные символы при регистрации игрока: LOGIN или PASSWORD\n");
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,1);
		return;
	}
	//проверка на длинну логина
	if((strlen(acl->info.login)<4)||(strlen(acl->info.login)>10))
	{
		LogExecStr("Неправильные данные при регистрации игрока: LOGIN\n");
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,1);
		return;
	}
	//проверка на наличие созданного по такому логину игрока
	if(!stricmp(sql.GetChar("users","login","login",acl->info.login),acl->info.login))
	{
		LogExecStr("Клиент под таким логином уже существует\n");
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,3);
		return;
	}
	//проверка на длинну пасса
	if(strlen(acl->info.pass)<4 || strlen(acl->info.pass)>10)
	{
		LogExecStr("Неправильные данные при регистрации игрока: PASS\n");
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,2);
		return;
	}
	//проверка на длинну имени
	if(strlen(acl->info.name)<4 || strlen(acl->info.name)>MAX_NAME)
	{
		LogExecStr("Неправильные данные при регистрации игрока: NAME\n");
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,13);
		return;
	}
	//проверка на длинну cases
	for(bi=0; bi<5; bi++)
		if(strlen(acl->info.cases[bi])<4 || strlen(acl->info.cases[bi])>MAX_NAME)
		{
			LogExecStr("Неправильные данные при регистрации игрока: CASES%d\n",bi);
			acl->state=STATE_DISCONNECT;
			Send_LoginMsg(acl,14);
			return;
		}
	//проверка пола
	if(acl->info.st[ST_GENDER]<0 || acl->info.st[ST_GENDER]>1) 
	{ 
		LogExecStr("Неправильные данные при регистрации игрока: ПОЛ\n");
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,15);
		return;
	}
	//проверка возраста
	if(acl->info.st[ST_AGE]<14 || acl->info.st[ST_AGE]>80) 
	{ 
		LogExecStr("Неправильные данные при регистрации игрока: ВОЗРАСТ\n");
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,16);
		return;
	}
	//проверка SPECIAL
	if((acl->info.st[ST_STRENGHT	]<1)||(acl->info.st[ST_STRENGHT		]>10)||
		(acl->info.st[ST_PERCEPTION	]<1)||(acl->info.st[ST_PERCEPTION	]>10)||
		(acl->info.st[ST_ENDURANCE	]<1)||(acl->info.st[ST_ENDURANCE	]>10)||
		(acl->info.st[ST_CHARISMA	]<1)||(acl->info.st[ST_CHARISMA		]>10)||
		(acl->info.st[ST_INTELLECT	]<1)||(acl->info.st[ST_INTELLECT	]>10)||
		(acl->info.st[ST_AGILITY	]<1)||(acl->info.st[ST_AGILITY		]>10)||
		(acl->info.st[ST_LUCK		]<1)||(acl->info.st[ST_LUCK			]>10))
		{
			LogExecStr("Неправильные данные при регистрации игрока: SPECIAL №%d\n", bi);
			acl->state=STATE_DISCONNECT;
			Send_LoginMsg(acl,5);
			return;
		}
	if((acl->info.st[ST_STRENGHT]+acl->info.st[ST_PERCEPTION]+acl->info.st[ST_ENDURANCE]+
		acl->info.st[ST_CHARISMA]+acl->info.st[ST_INTELLECT]+
		acl->info.st[ST_AGILITY]+acl->info.st[ST_LUCK])!=40)
		{
			LogExecStr("Неправильные данные при регистрации игрока: SPECIAL сумма\n");
			acl->state=STATE_DISCONNECT;
			Send_LoginMsg(acl,5);
			return;
		}

//СОЗДАНИЕ ПЕРСОНАЖА
	//инфа по игроку
	//stats,skills,start_perks,perks
	acl->info.sk[SK_SMALL_GUNS		]=5	+4*acl->info.st[ST_AGILITY];
	acl->info.sk[SK_BIG_GUNS		]=0	+2*acl->info.st[ST_AGILITY];
	acl->info.sk[SK_ENERGY_WEAPONS	]=0	+2*acl->info.st[ST_AGILITY];
	acl->info.sk[SK_UNARMED			]=30+2*(acl->info.st[ST_AGILITY]+acl->info.st[ST_AGILITY]);
	acl->info.sk[SK_MELEE_WEAPONS	]=20+2*(acl->info.st[ST_AGILITY]+acl->info.st[ST_AGILITY]);
	acl->info.sk[SK_THROWING		]=0	+4*acl->info.st[ST_AGILITY];
	acl->info.sk[SK_FIRST_AID		]=0	+2*(acl->info.st[ST_PERCEPTION]+acl->info.st[ST_INTELLECT]);
	acl->info.sk[SK_DOCTOR			]=5	+acl->info.st[ST_PERCEPTION]+acl->info.st[ST_INTELLECT];
	acl->info.sk[SK_SNEAK			]=5	+3*acl->info.st[ST_AGILITY];
	acl->info.sk[SK_LOCKPICK		]=10+acl->info.st[ST_PERCEPTION]+acl->info.st[ST_AGILITY];
	acl->info.sk[SK_STEAL			]=0	+3*acl->info.st[ST_AGILITY];
	acl->info.sk[SK_TRAPS			]=10+acl->info.st[ST_PERCEPTION]+acl->info.st[ST_AGILITY];
	acl->info.sk[SK_SCIENCE			]=0	+4*acl->info.st[ST_INTELLECT];
	acl->info.sk[SK_REPAIR			]=0	+3*acl->info.st[ST_INTELLECT];
	acl->info.sk[SK_SPEECH			]=0	+5*acl->info.st[ST_CHARISMA];
	acl->info.sk[SK_BARTER			]=0	+4*acl->info.st[ST_CHARISMA];
	acl->info.sk[SK_GAMBLING		]=0	+5*acl->info.st[ST_LUCK];
	acl->info.sk[SK_OUTDOORSMAN		]=0	+2*(acl->info.st[ST_ENDURANCE]+acl->info.st[ST_INTELLECT]);

	GenParam(acl);
	acl->info.st[ST_CURRENT_HP]=acl->info.st[ST_MAX_LIFE];
	acl->info.st[ST_POISONING_LEVEL]=0;
	acl->info.st[ST_RADIATION_LEVEL]=0;
	acl->info.st[ST_CURRENT_STANDART]=acl->info.st[ST_MAX_COND];

	//map,x,y,base_type,ori
	acl->info.map=11;
	acl->info.x=64;
	acl->info.y=107;
	acl->info.ori=4;
	acl->info.base_type=10;

	if(!sql.NewPlayer(&acl->info))
	{
		LogExecStr("!!!WORNING!!!: траблы с mySQL - неудалось сохранить игрока\n");
		sql.DeleteDataPlayer(&acl->info);
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,8);
		return;
	}

	//вывод инфы в лог
	//sql.PrintTableInLog("users");
	//посылка подтверждения
	Send_LoginMsg(acl,6);
	//отключение клиента
	acl->state=STATE_DISCONNECT;

	LogExecStr("РЕГИСТРАЦИЯ ИГРОКА ПРОШЛА УСПЕШНО ЗА %d МСЕК\n", GetTickCount()-tickStart);
}

void CServer::Process_GetLogIn(CLIENT* acl)
{
	LogExecStr("Клиент логиниться...");

	acl->bin.pop(acl->info.login,MAX_LOGIN);
	acl->bin.pop(acl->info.pass,MAX_LOGIN);
	if(acl->bin.IsError())
	{
		LogExecStr("Wrong MSG data for Process_GetLogIn from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return;
	}

	if(acl->state!=STATE_CONN)
	{
		LogExecStr("Client not STATE_CONN for Process_GetLogIn from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return;
	}

	if(sql.Check(acl->info.login) || sql.Check(acl->info.pass))
	{
		LogExecStr("Запрещенные символы при логине игрока: LOGIN или PASSWORD\n");
		acl->state=STATE_DISCONNECT;
		return;
	}

//LogExecStr("Проверка логина и пароля:...");

	if((stricmp(acl->info.login,sql.GetChar("users","login","login",acl->info.login)))||
		(stricmp(acl->info.pass,sql.GetChar("users","pass","login",acl->info.login))))
	{
		LogExecStr("Проверка логина и пароля:...FALSE\n");
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,9);
		return;
	}
//LogExecStr("OK\n");

	//проверка на длинну логина и пасса
	if((strlen(acl->info.login)<4)|(strlen(acl->info.login)>10))
	{
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,1);
		return;
	}
	if((strlen(acl->info.pass)<4)|(strlen(acl->info.pass)>10))
	{
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,2);
		return;
	}

	acl->info.id=sql.GetInt("users","id","login",acl->info.login);
	if(!acl->info.id)
	{
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,8);
		return;
	}

	cl_map::iterator it=cr.find(acl->info.id);
	if(it!=cr.end())
	{
		CrID old_idchannel=acl->info.idchannel;
		acl->info=(*it).second->info;
		acl->info.idchannel=old_idchannel;

		if(map_cr[acl->info.map].count(acl->info.id)) mm.ClearPlayer(&acl->info);
	
//		delete (*it).second; !!!!!!!!!!

		cr.erase(it);
		map_cr[acl->info.map].erase(acl->info.id);
	} 
	else if(!sql.LoadDataPlayer(&acl->info))
	{
		acl->state=STATE_DISCONNECT;
		Send_LoginMsg(acl,8);
		return;
	}
	else
	{
		acl->info.cond=COND_LIFE;
		acl->info.cond_ext=COND_LIFE_NONE;
	}

	if(mm.TransitToTile(&acl->info,acl->info.map,acl->info.x,acl->info.y)!=TR_OK) //чтоб друг другу на головы не высаживались
	{
		Send_LoginMsg(acl,11);
		acl->state=STATE_DISCONNECT;
		return;
	}

	acl->info.flags=FCRIT_PLAYER;

	acl->info.a_obj=&acl->info.def_obj1;
	acl->info.a_obj_arm=&acl->info.def_obj2;
	acl->info.a_obj->object=all_s_obj[acl->info.base_type];
	acl->info.a_obj_arm->object=all_s_obj[acl->info.base_type+200];

	for(dyn_map::iterator it2=acl->info.obj.begin();it2!=acl->info.obj.end();it2++)
		if((*it2).second->active)
			if((*it2).second->object->type==OBJ_TYPE_ARMOR)
				acl->info.a_obj_arm=(*it2).second;
			else
				acl->info.a_obj=(*it2).second;

	MSGTYPE msg=NETMSG_LOGINOK;
	acl->bout << msg;

	acl->state=STATE_LOGINOK;

	Send_LoadMap(acl);

	LogExecStr("OK\n");
}

void CServer::Process_MapLoaded(CLIENT* acl)
{
	LogExecStr("Карта загружена. Отправка данных игроку...");

	cr[acl->info.id]=acl;
	map_cr[acl->info.map][acl->info.id]=acl;

	SETFLAG(acl->info.flags,FCRIT_CHOSEN);
	Send_AddCritter(acl,&acl->info);
	UNSETFLAG(acl->info.flags,FCRIT_CHOSEN);

	Send_AllParams(acl,TYPE_STAT ); //отправка всех статов игроку
	Send_AllParams(acl,TYPE_SKILL); //отправка всех скиллов игроку
	Send_AllParams(acl,TYPE_PERK ); //отправка всех перков игроку

	if(!acl->info.obj.size())
	{
		CreateObjToPl(acl->info.idchannel,1301);
		CreateObjToPl(acl->info.idchannel,2016);
	}

	//отправка объектов игроку
	for(dyn_map::iterator it_o=acl->info.obj.begin();it_o!=acl->info.obj.end();it_o++)
	{
//		GenWear((*it_o).second,0);
//		if(!(*it_o).second->tick) трабла с итератором. исправить!!!!!!!!!!!!!!!!
//		{
//			DeleteObj((*it_o).second->id);
//			continue;
//		}
		Send_AddObject(acl,(*it_o).second);
	}

	DelClFromAllListId(acl->info.id,acl->info.map);
	acl->info.cl_id.clear();
	acl->info.obj_id.clear();

	GenLook(acl);
	SetVisibleCl(acl);
	SetVisibleObj(acl);

//	//отправка оппонентов
//	for(cl_map::iterator it=map_cr[acl->info.map].begin();it!=map_cr[acl->info.map].end();it++)
//		if((*it).second->info.id!=acl->info.id)
//			if((*it).second->info.cl_id.count(acl->info.id))
//				Send_AddCritter(acl,&(*it).second->info);

	acl->state=STATE_GAME;

	LogExecStr("OK\n");
}

//!Cvet ---------------------------------------------------------------

void CServer::Process_GetText(CLIENT* acl)
{
	WORD len;
	char str[MAX_TEXT+1];

	acl->bin >> len;

// 	if(acl->state!=STATE_GAME)
	if(acl->bin.IsError() || len>MAX_TEXT)
	{
		LogExecStr("Wrong MSG data forProcess_GetText from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return;
	}

	acl->bin.pop(str,len);
	str[len]=0;

	if(acl->bin.IsError())
	{
		LogExecStr("Wrong MSG data forProcess_GetText - partial recv from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return;
	}

	LogExecStr("GetText: %s\n",str);

	char* param;
	char* next;

	WORD self_len=0;
	WORD o_len=0;
	char self_str[MAX_TEXT+255+1]="";
	char o_str[MAX_TEXT+255+1]="";
	char mname[MAX_NAME+1];

//!Cvet переделал концепцию +++++++++++++++++++++++++
	WORD cmd=CMD_SAY;
	BYTE say_param=SAY_NORM;

	if(str[0]=='~')
	{
		param=GetParam(str,&next);
		if(!param) return;

		cmd=GetCmdNum(param);
	}
	else
		next=str;

	switch(cmd)
	{
	case CMD_SAY:
		if(next[0]=='/' || next[0]=='.') //??? next[0]=='!'
		{
			next++;
			if(!next)
			{
				strcpy(self_str, "эээ?!");
				break;
			}

			if(next[0]=='к' || next[0]=='К' || next[0]=='s' || next[0]=='S') say_param=SAY_SHOUT;
//			else if(next[0]=='о' || next[0]=='О' || next[0]=='m' || next[0]=='M') say_param=SAY_MSHOUT;
			else if(next[0]=='э' || next[0]=='Э' || next[0]=='e' || next[0]=='E') say_param=SAY_EMOTE;
			else if(next[0]=='ш' || next[0]=='Ш' || next[0]=='w' || next[0]=='W') say_param=SAY_WHISP;
			else if(next[0]=='с' || next[0]=='С' || next[0]=='$') say_param=SAY_SOCIAL;
		}

		if(say_param!=SAY_NORM)
		{
			next++;
			if(next[0]==' ') next++;
		}

		switch(say_param)
		{
		case SAY_NORM:
			if(!next)
				strcpy(self_str, "А чего сказать то?!");
			else
			{
				sprintf(self_str, "%s",next);
				sprintf(o_str, "%s: %s",MakeName(acl->info.name,mname),next);
			//	sprintf(o_str, "%s",next);
			}
			break;
		case SAY_SHOUT:
			if(!next)
				strcpy(self_str, "Покричу, только скажи что?!");
			else
			{
				sprintf(self_str, "Вы закричали: !!!%s!!!",my_strupr(next));
				sprintf(o_str, "%s закричал%s: !!!%s!!!",MakeName(acl->info.name,mname),(acl->info.st[ST_GENDER]==0)?"":"а",next);
			//	sprintf(self_str, "!!!%s!!!",my_strupr(next));
			//	sprintf(o_str, "!!!%s!!!",next);
			}
			break;
//		case SAY_MSHOUT:
//			if(!next)
//				strcpy(self_str, "Что орем?!");
//			else
//			{
//				sprintf(self_str, "Вы заорали: !!!%s!!!",my_strupr(next));							//!Cvet изм. .gender=='m'
//				sprintf(o_str, "%s заорал%s: !!!%s!!!",MakeName(acl->info.name,mname),(acl->info.st[ST_GENDER]==0)?"":"а",next);
//			}
//			break;
		case SAY_EMOTE:
			if(!next)
				strcpy(self_str, "Никаких эмоций!");
			else
			{
				sprintf(self_str, "**%s %s**",MakeName(acl->info.name,mname),next);
				sprintf(o_str, "**%s %s**",mname,next);
			}
			break;
		case SAY_WHISP: //добавил шепет
			if(!next)
				strcpy(self_str, "Че шептать будем?...");
			else
			{
				sprintf(self_str, "Вы прошептали: ...%s...",my_strlwr(next));							//!Cvet изм. .gender=='m'
				sprintf(o_str, "%s прошептал%s: ...%s...",MakeName(acl->info.name,mname),(acl->info.st[ST_GENDER]==0)?"":"а",next);
			//	sprintf(self_str, "...%s...",my_strlwr(next));
			//	sprintf(o_str, "...%s...",next);
			}
			break;
		case SAY_SOCIAL:
			int socid=GetSocialId(next);
			if(socid>=0)
			{
				ProcessSocial(acl,socid,next);
				return;
			}
			else
				strcpy(self_str, "Хмм?!");
			break;
		}
		break;
	case CMD_EXIT:
		LogExecStr("CMD_EXIT for%s\n",acl->info.name);
		acl->state=STATE_DISCONNECT;
		break;
	default:
		return;
	}
//!Cvet ------------------------------------------

	LogExecStr("self: %s\not: %s\n",self_str,o_str);
	self_len=strlen(self_str);
	o_len=strlen(o_str);

	MSGTYPE msg=NETMSG_CRITTERTEXT;

	CLIENT* c;
	for(cl_map::iterator it=cl.begin();it!=cl.end();it++)
	{
		c=(*it).second;

		if(c==acl && self_len)
		{
			c->bout << msg;
			c->bout << acl->info.id;
			c->bout << say_param;
			c->bout << self_len;
			c->bout.push(self_str,self_len);
		}
		else if(c!=acl && o_len)
		{
			c->bout << msg;
			c->bout << acl->info.id;
			c->bout << say_param;
			c->bout << o_len;
			c->bout.push(o_str,o_len);
		}
	}
/*
	if(!acl->info.cl_id.empty())
	{
		MSGTYPE msg=NETMSG_CRITTER_MOVE;
		CLIENT* c=NULL;
		cl_map::iterator it;
		DWORD find_id;
		for(it_li=acl->info.cl_id.begin(); it_li!=acl->info.cl_id.end();)
		{
			find_id=(*it_li);
			it=cr.find(find_id);
			if(it==cr.end()) { it_li++; DelClFromListId(acl,find_id); continue; }
			c=(*it).second;
			c->bout << msg; 
			c->bout << acl->info.id;
			c->bout << move_params;

			c->bout << c->info.x;
			c->bout << c->info.y;

			it_li++;
		}
	}
*/
}

void CServer::ProcessSocial(CLIENT* sender,WORD socid,char* aparam)
{
	char* param;
	char* next;

	WORD self_len=0;
	WORD vic_len=0;
	WORD all_len=0;

	char SelfStr[MAX_TEXT+255+1]="";
	char VicStr[MAX_TEXT+255+1]="";
	char AllStr[MAX_TEXT+255+1]="";
	
	CLIENT* victim=NULL;
	param=GetParam(aparam,&next);

	LogExecStr("ProcessSocial: %s\n",param?param:"NULL");
	
	if(param && param[0] && GetPossParams(socid)!=SOC_NOPARAMS)
	{
		my_strlwr(param);
		if(!strcmp(param,"я") && GetPossParams(socid)!=SOC_NOSELF)
		{
			GetSocSelfStr(socid,SelfStr,AllStr,&sender->info);
		}
		else
			{
				victim=LocateByPartName(param);
				if(!victim) 
					GetSocVicErrStr(socid,SelfStr,&sender->info);
				else
					GetSocVicStr(socid,SelfStr,VicStr,AllStr,&sender->info,&victim->info);
			}
	}
	else
		GetSocNoStr(socid,SelfStr,AllStr,&sender->info);

	LogExecStr("self: %s\nvic: %s\nall: %s\n",SelfStr,VicStr,AllStr);
	self_len=strlen(SelfStr);
	vic_len=strlen(VicStr);
	all_len=strlen(AllStr);

	MSGTYPE msg=NETMSG_CRITTERTEXT;

	CLIENT* c;
	for(cl_map::iterator it=cl.begin();it!=cl.end();it++)
	{
		c=(*it).second;

		if(c==sender && self_len)
		{
			c->bout << msg;
			c->bout << sender->info.id;
			c->bout << (BYTE)(SAY_SOCIAL);
			c->bout << self_len;
			c->bout.push(SelfStr,self_len);
		}
		else if(c==victim && vic_len)
		{
			c->bout << msg;
			c->bout << sender->info.id;
			c->bout << (BYTE)(SAY_SOCIAL);
			c->bout << vic_len;
			c->bout.push(VicStr,vic_len);
		}
		else if(all_len)
		{
			c->bout << msg;
			c->bout << sender->info.id;
			c->bout << (BYTE)(SAY_SOCIAL);
			c->bout << all_len;
			c->bout.push(AllStr,all_len);
		}
	}
}

CLIENT* CServer::LocateByPartName(char* name)
{
	bool found=0;
	CLIENT* c;
	for(cl_map::iterator it=cl.begin();it!=cl.end();it++)
	{
		c=(*it).second;

		if(PartialRight(name,c->info.name)) 
		{
			found=1;
			break;
		}
	}

	return found?c:NULL;
}


void CServer::Process_Rotate(CLIENT* acl)
{
	
	BYTE rot;
	acl->bin >> rot;

	if(acl->bin.IsError())
	{
		LogExecStr("Wrong MSG data forProcess_Rotate from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return;
	}

	MSGTYPE msg=rot?NETMSG_CRITTER_ROTCCW:NETMSG_CRITTER_ROTCW;

	CLIENT* c;
	for(cl_map::iterator it=cl.begin();it!=cl.end();it++)
	{
		c=(*it).second;

		c->bout << msg;
		c->bout << acl->info.id;
	}

}

//!Cvet +++++++++++++++++++++++++++++++++++++++++++++
void CServer::Process_Move(CLIENT* acl)
{
	BYTE move_params;

	acl->bin >> move_params;

	BYTE dir=move_params & BIN8(00000111);

//	LogExecStr("Process_Move move_params=%d, dir=%d, how_move=%d\n",move_params,dir,how_move);

	if(acl->bin.IsError())
	{
		LogExecStr("Wrong MSG data for Process_Move from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return;
	}

	if(dir>5)
	{
		SetCheat(acl,"Process_Move - неправильное направление движения");
		Send_XY(acl);
		return;
	}

	WORD old_map=acl->info.map;

	BYTE move_res=0;

	switch(dir)
	{
	case 0:
		if(!(acl->info.x%2))
			move_res=mm.MoveToTile(&acl->info,acl->info.x-1,acl->info.y);
		else
			move_res=mm.MoveToTile(&acl->info,acl->info.x-1,acl->info.y-1);
		break;
	case 1:
		if(acl->info.x%2)
			move_res=mm.MoveToTile(&acl->info,acl->info.x-1,acl->info.y);
		else
			move_res=mm.MoveToTile(&acl->info,acl->info.x-1,acl->info.y+1);
		break;
	case 2:
		move_res=mm.MoveToTile(&acl->info,acl->info.x,acl->info.y+1);
		break;
	case 3:
		if(acl->info.x%2)
			move_res=mm.MoveToTile(&acl->info,acl->info.x+1,acl->info.y);
		else
			move_res=mm.MoveToTile(&acl->info,acl->info.x+1,acl->info.y+1);
		break;
	case 4:
		if(!(acl->info.x%2))
			move_res=mm.MoveToTile(&acl->info,acl->info.x+1,acl->info.y);
		else
			move_res=mm.MoveToTile(&acl->info,acl->info.x+1,acl->info.y-1);
		break;
	case 5:
		move_res=mm.MoveToTile(&acl->info,acl->info.x,acl->info.y-1);
		break;
	}

	switch(move_res)
	{
	case MR_STEP:
		SetVisibleCl(acl);
		SetVisibleObj(acl);
		Send_Move(acl,move_params);
		break;
	case MR_TRANSIT:
		map_cr[old_map].erase(acl->info.id);

//		map_cr[acl->info.map][acl->info.id]=acl;
		DelClFromAllListId(acl->info.id,old_map);

		Send_LoadMap(acl);
		acl->state=STATE_LOGINOK;
		break;
//	case MR_TOGLOBAL:
//		break;
	default:
	case MR_FALSE:
		SetCheat(acl,"Process_Move - попытка походить в занятую клетку");
		Send_XY(acl);
		break;
	}
}

void CServer::Send_XY(CLIENT* acl)
{
    MSGTYPE msg=NETMSG_XY;

	acl->bout << msg;
	acl->bout << acl->info.x;
	acl->bout << acl->info.y;
	acl->bout << acl->info.ori;
//LogExecStr("Try connect! Send_XY id=%d\n", acl->info.id);
}

void CServer::Send_AllParams(CLIENT* acl, BYTE type_param)
{
	//отсылаем ствты которые не равны 0
	BYTE all_send_params=0;
	BYTE param=0;

	switch (type_param)
	{
	case TYPE_STAT:
		for(param=0; param<ALL_STATS; param++)
			if(acl->info.st[param]) all_send_params++;
		break;
	case TYPE_SKILL:
		for(param=0; param<ALL_SKILLS; param++)
			if(acl->info.sk[param]) all_send_params++;
		break;
	case TYPE_PERK:
		for(param=0; param<ALL_PERKS; param++)
			if(acl->info.pe[param]) all_send_params++;
		break;
	}

	if(all_send_params)
	{
		MSGTYPE msg=NETMSG_ALL_PARAMS;
		acl->bout << msg;
		acl->bout << type_param;
		acl->bout << all_send_params;
		
		switch (type_param)
		{
		case TYPE_STAT:
			for(param=0; param<ALL_STATS; param++)
				if(acl->info.st[param])
				{
					acl->bout << param;
					acl->bout << acl->info.st[param];
				}
			break;
		case TYPE_SKILL:
			for(param=0; param<ALL_SKILLS; param++)
				if(acl->info.sk[param])
				{
					acl->bout << param;
					acl->bout << acl->info.sk[param];
				}
			break;
		case TYPE_PERK:
			for(param=0; param<ALL_PERKS; param++)
				if(acl->info.pe[param])
				{
					acl->bout << param;
					acl->bout << acl->info.pe[param];
				}
			break;
		}
	}
}

void CServer::Send_Param(CLIENT* acl, BYTE type_param, BYTE num_param)
{
	MSGTYPE msg=NETMSG_PARAM;
	acl->bout << msg;
	acl->bout << type_param;
	acl->bout << num_param;

	switch (type_param)
	{
	case TYPE_STAT:
		acl->bout << acl->info.st[num_param];
		break;
	case TYPE_SKILL:
		acl->bout << acl->info.sk[num_param];
		break;
	case TYPE_PERK:
		acl->bout << acl->info.pe[num_param];
		break;
	}
}

void CServer::Send_Talk(CLIENT* acl, npc_dialog* send_dialog)
{
	MSGTYPE msg=NETMSG_TALK_NPC;
	acl->bout << msg;
	if(send_dialog==NULL)
	{
		const BYTE zero_byte=0;
		acl->bout << zero_byte;
		return;
	}
//всего вариантов ответа
	BYTE all_answers=send_dialog->answers.size();
	acl->bout << all_answers;
	if(!all_answers) return;
//основной текст
	acl->bout << send_dialog->id_text;
//варианты ответов
	for(answers_list::iterator it_a=send_dialog->answers.begin(); it_a!=send_dialog->answers.end(); it_a++)
		acl->bout << (*it_a)->id_text;
}

void CServer::Send_LoadMap(CLIENT* acl)
{
	MSGTYPE msg=NETMSG_LOADMAP;

	acl->bout << msg;
	acl->bout << acl->info.map;
}

void CServer::Process_ChangeObject(CLIENT* acl)
{
	DWORD idobj;
	BYTE num_slot;
	
	acl->bin >> idobj;
	acl->bin >> num_slot;

//LogExecStr("Process_ChangeObject id=%d, num_slot=%d\n", idobj, num_slot);

	//деактивируем предмет в активном слоте
	if(!idobj && !num_slot)
	{
		Send_Action(acl,ACT_HIDE_OBJ,0);
		acl->info.a_obj->active=NULL;
		acl->info.a_obj=&acl->info.def_obj1;
		return;
	}
	//активируем предмет в активном слоте
	if(idobj && !num_slot)
	{
		dyn_map::iterator it=acl->info.obj.find(idobj);
		if(it!=acl->info.obj.end())
			if((*it).second->object->type!=OBJ_TYPE_ARMOR)
			{
				(*it).second->active=1;
				acl->info.a_obj=(*it).second;
				Send_Action(acl,ACT_SHOW_OBJ,0);
				return;
			}
	}
	//деактивируем предмет в слоте брони
	if(!idobj && num_slot)
	{
		acl->info.a_obj_arm->active=NULL;
		acl->info.a_obj_arm=&acl->info.def_obj2;
		Send_Action(acl,ACT_CHANGE_ARM,0);
		return;
	}
	//активируем предмет в слоте брони
	if(idobj && num_slot)
	{
		dyn_map::iterator it=acl->info.obj.find(idobj);
		if(it!=acl->info.obj.end())
			if((*it).second->object->type==OBJ_TYPE_ARMOR)
			{
				(*it).second->active=1;
				acl->info.a_obj_arm=(*it).second;
				Send_Action(acl,ACT_CHANGE_ARM,0);
				return;
			}
	}
	//здесь надо обновлять игрока при неправильных мессагах
	//обнуление
	acl->info.a_obj=&acl->info.def_obj1;
	acl->info.a_obj_arm=&acl->info.def_obj2;
	MSGTYPE msg=NETMSG_CRITTER_ACTION;
	acl->bout << msg;
	acl->bout << acl->info.id;
	acl->bout << ACT_NULL;
	acl->bout << acl->info.a_obj->object->id;
	acl->bout << acl->info.a_obj_arm->object->id;
	acl->bout << BYTE(0);
	acl->bout << acl->info.ori;
}

void CServer::Process_UseObject(CLIENT* acl)
{
//LogExecStr("Process_UseObject...");

	BYTE c_obj_type;
	DWORD t_id;
	BYTE c_ori;
	BYTE c_action;
	BYTE c_rate_object;

	acl->bin >> c_obj_type;
	acl->bin >> t_id;
	acl->bin >> c_ori;
	acl->bin >> c_action;
	acl->bin >> c_rate_object;

//LogExecStr("Цель ID=%d, Тайл Х=%d, Тайл У=%d, Ориентация=%d, Режим использования=%d...",t_id,t_x,t_y,c_ori,t_action);

	if(acl->bin.IsError())
	{
		LogExecStr("Wrong MSG data forProcess_Move from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return;
	}

	acl->info.ori=c_ori;

	if(acl->info.cond!=COND_LIFE)
	{
		LogExecStr("!WORNING! - читерство - попытка применить объект мертвым игроком. ид=|%d|\n",acl->info.id);
		return;
	}
//Оружие
	if(c_obj_type==OBJ_TYPE_WEAPON)
	{
	//использование оружия
		if(c_action==ACT_USE_OBJ)
			if(acl->info.a_obj->object->p111[OBJ_WEAP_TIME_ACTIV])
			{
				if(acl->info.cond_ext==COND_LIFE_ACTWEAP)
				{
					Act_Attack(acl,c_rate_object,t_id);
					return;
				}
				else c_action=ACT_ACTIVATE_OBJ;
			}
			else
			{
				Act_Attack(acl,c_rate_object,t_id);
				return;
			}

	//активация оружия
		if(c_action==ACT_ACTIVATE_OBJ && acl->info.a_obj->object->p111[OBJ_WEAP_TIME_ACTIV])
		{
			acl->info.cond_ext=COND_LIFE_ACTWEAP;
			Send_Action(acl,c_action,0);
			return;
		}
	//деактивация оружия
		if(c_action==ACT_DACTIVATE_OBJ)
		{
			acl->info.cond_ext=COND_LIFE_NONE;
			Send_Action(acl,c_action,0);
			return;
		}

	//не выходит ли изза границ режим использования
	//	if(c_action>acl->info.a_obj->object->p[129])
	//		{ LogExecStr("ОШИБКА - режим использования выходит изза границ\n"); return; }

	}
	else if(c_obj_type==OBJ_TYPE_DRUG)
	{

	}
	else if(c_obj_type==OBJ_TYPE_AMMO)
	{

	}
	else if(c_obj_type==OBJ_TYPE_MISC)
	{

	}
	else if(c_obj_type==OBJ_TYPE_KEY)
	{

	}
	else if(c_obj_type==OBJ_TYPE_CONTAINER)
	{

	}


//достаточно ли у нас навыка чтоб использовать объект в данном режиме
//	if(acl->info.s[acl->info.a_obj->object->p[t_action+1]]<acl->info.a_obj->object->p[t_action+2]) return;

//	//находим номер запускаемого скрипта
//	BYTE num_script=acl->info.a_obj->object->p[t_action+14];

//	//обновляем ориентацию игрока
	

//LogExecStr("Скрипт=%d\n",num_script);

//	switch (num_script)
//	{
//	case 0: Scr_SkillUD(t_action,acl,t_id); return; //повышение/понижение
//	case 1: Scr_Attack(t_action,acl,t_id); return; //атака
//	case 2: Scr_Reload(t_action,acl); return; //перезарядка
//	}
}

void CServer::Process_UseSkill(CLIENT* acl)
{
	BYTE numWeap;

	acl->bin >> numWeap;
	
	if(acl->bin.IsError())
	{
		LogExecStr("Wrong MSG data forProcess_Move from SockID %d!\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return;
	}

	//посылает всем сведения о достовании оружия (ессно кроме самого себя*1)
//	Send_HideWpn(acl);
}

void CServer::Process_GetTime(CLIENT* acl)
{
	MSGTYPE msg=NETMSG_GAME_TIME;

	GetSystemTime(&sys_time);
	Game_Time=(sys_time.wHour*60+sys_time.wMinute)*TIME_MULTIPLER;

	Game_Day=15;
	Game_Month=5;
	Game_Year=2155;

	acl->bout << msg;
	acl->bout << Game_Time;
	acl->bout << Game_Day;
	acl->bout << Game_Month;
	acl->bout << Game_Year;
}

void CServer::Process_Talk_NPC(CLIENT* acl)
{
	CrID id_npc_talk;
	BYTE num_answer;

	acl->bin >> id_npc_talk;
	acl->bin >> num_answer;
//находим непися
	CLIENT* npc;
	cl_map::iterator it=pc.find(id_npc_talk);
	if(it==pc.end())
	{
		SetCheat(acl,"Process_Talk_NPC - не найден НПЦ");
		return;
	}
	npc=(*it).second;

	if(npc->info.cond!=COND_LIFE)
	{
		SetCheat(acl,"Process_Talk_NPC - попытка заговорить с неживым НПЦ");
		NPC_Dialog_Close(npc,acl,NPC_SAY_NONE);
		return;
	}

	int dist=sqrt(pow(acl->info.x-npc->info.x,2)+pow(acl->info.y-npc->info.y,2));

	if(dist>TALK_NPC_DISTANCE)
	{
		SetCheat(acl,"Process_Talk_NPC - дистанция разговора превышает максимальную");
		NPC_Dialog_Close(npc,acl,NPC_SAY_NONE);
		return;
	}

	if(!npc->i_npc->dialogs.size())
	{
		NPC_Dialog_Close(npc,acl,NPC_SAY_HELLO);
		return;
	}

	dialogs_map::iterator it_d;
	answers_list::iterator it_a;
	npc_dialog* send_dialog=&npc->i_npc->compiled_dialog;

//продолжаем разговор
	if(npc->i_npc->talking==acl->info.id)
	{
		if(!send_dialog)
		{
			LogExecStr("Диалог - Ошибка. Пустой указатель на предыдущий диалог\n");
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
		if(send_dialog->id==0 || send_dialog->id==1)
		{
			LogExecStr("Диалог - Ошибка. ID диалога равна %d\n", send_dialog->id);
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
		if(num_answer+1 > send_dialog->answers.size())
		{
			LogExecStr("Диалог - Ошибка. Ответ первышает максимальное значение ответов\n");
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
	//находим ид ответа
		it_a=send_dialog->answers.begin()+num_answer;
		if(!(*it_a))
		{
			LogExecStr("Диалог - Ошибка. Пустой указатель ответа\n");
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
	//выполняем результат ответа
		NPC_Use_result(npc,acl,(*it_a));
	//проверяем на нулевой линк, т.е. выход
		if(!(*it_a)->link)
		{
			NPC_Dialog_Close(npc,acl,NPC_SAY_NONE);
			return;
		}
	//проверяем на еденичный линк, т.е. возврат к предыдущему
		if((*it_a)->link==1)
		{
			//!!!!!!!!!!!
		}
	//ищем диалог
		it_d=npc->i_npc->dialogs.find((*it_a)->link);
		if(it_d==npc->i_npc->dialogs.end())
		{
			LogExecStr("Диалог - Ошибка. Не найден диалог по ответу\n");
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
		
	//компануем диалог
		if(!NPC_Dialog_Compile(npc,acl,(*it_d).second))
		{
			LogExecStr("Диалог - Ошибка. Неудалось скомпоновать диалог\n");
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
	}
//начинаем разговор
	else if(!npc->i_npc->talking)// && !npc->info.break_time)
	{
	//впервые
		it_d=npc->i_npc->dialogs.begin();

		int go_dialog=0;
		for(it_a=(*it_d).second->answers.begin(); it_a!=(*it_d).second->answers.end(); it_a++)
		{
			if(NPC_Check_demand(npc,acl,(*it_a)))
			{
				go_dialog=(*it_a)->link;
				break;
			}
		}
		if(it_a==(*it_d).second->answers.end())
		{
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
	//выполняем результат
		NPC_Use_result(npc,acl,(*it_a));

		if(!go_dialog)
		{
			NPC_Dialog_Close(npc,acl,NPC_SAY_FUCKOFF);
			return;
		}

		it_d=npc->i_npc->dialogs.find(go_dialog);

		if(it_d==npc->i_npc->dialogs.end())
		{
			LogExecStr("Диалог - Ошибка. Не найден диалог по предустановкам\n");
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
	//компануем диалог
		if(!NPC_Dialog_Compile(npc,acl,(*it_d).second))
		{
			LogExecStr("Диалог - Ошибка. Неудалось скомпоновать диалог\n");
			NPC_Dialog_Close(npc,acl,NPC_SAY_ERROR);
			return;
		}
	}
//нпц занят
	else 
	{
		NPC_Dialog_Close(npc,acl,NPC_SAY_IMBYSY); 
		return;
	}

//посылаем
	Send_Talk(acl,&npc->i_npc->compiled_dialog);

//	npc->info.start_bt=GetTickCount();
//	npc->info.break_time=TALK_MAX_TIME; //!!!!!!

	npc->i_npc->talking=acl->info.id;
}

void CServer::NPC_Dialog_Close(CLIENT* npc, CLIENT* acl, BYTE onhead_say)
{
	npc->i_npc->talking=0;

//	npc->info.break_time<=0;

	npc->i_npc->compiled_dialog.id=0;
	if(acl) Send_Talk(acl,NULL);
}

int CServer::NPC_Dialog_Compile(CLIENT* npc, CLIENT* acl, npc_dialog* new_dlg)
{
	npc->i_npc->compiled_dialog=(*new_dlg); //конструктор копирования должен??? уничтожить все старые объекты !!!!!!!!!!
	npc_dialog* cmp_dlg=&npc->i_npc->compiled_dialog;

	if(cmp_dlg->id==1) return 0;

//составляем ветку ответов
	for(answers_list::iterator it_a=cmp_dlg->answers.begin(); it_a!=cmp_dlg->answers.end(); it_a++)
	{
	//смотрим требование
		if(!NPC_Check_demand(npc,acl,(*it_a)))	cmp_dlg->answers.erase(it_a);
		
		if(it_a==cmp_dlg->answers.end()) break;
	}

	if(!cmp_dlg->answers.size()) return 0;

	return 1;
}

int CServer::NPC_Check_demand(CLIENT* npc, CLIENT* acl, answer* check_answ)
{
	if(!check_answ->demands.size()) return 1;

	for(demand_list::iterator it_d=check_answ->demands.begin(); it_d!=check_answ->demands.end(); it_d++)
	{
		switch ((*it_d)->type)
		{
		case DEMAND_NONE:
			//SAFEDEL((*it_d));
			//check_answ->demands.erase(it_d);
			continue;
		case DEMAND_STAT:
			if((*it_d)->oper=='>')		 { if(acl->info.st[(*it_d)->param]> (*it_d)->count) continue; }
			else if((*it_d)->oper=='<') { if(acl->info.st[(*it_d)->param]< (*it_d)->count) continue; }
			else if((*it_d)->oper=='=') { if(acl->info.st[(*it_d)->param]==(*it_d)->count) continue; }
			return 0;
		case DEMAND_SKILL:
			if((*it_d)->oper=='>')		 { if(acl->info.sk[(*it_d)->param]> (*it_d)->count) continue; }
			else if((*it_d)->oper=='<') { if(acl->info.sk[(*it_d)->param]< (*it_d)->count) continue; }
			else if((*it_d)->oper=='=') { if(acl->info.sk[(*it_d)->param]==(*it_d)->count) continue; }
			return 0;
		case DEMAND_PERK:
			if((*it_d)->oper=='>')		 { if(acl->info.pe[(*it_d)->param]> (*it_d)->count) continue; }
			else if((*it_d)->oper=='<') { if(acl->info.pe[(*it_d)->param]< (*it_d)->count) continue; }
			else if((*it_d)->oper=='=') { if(acl->info.pe[(*it_d)->param]==(*it_d)->count) continue; }
			return 0;
		case DEMAND_VAR:
			if(sql.CheckVarNPC(npc->info.id,(*it_d)->var_name,acl->info.id,(*it_d)->oper,(*it_d)->count)) continue;
			return 0;
		default:
			continue;
		}
	}
	return 1;
}

void CServer::NPC_Use_result(CLIENT* npc, CLIENT* acl, answer* use_answ)
{
	if(!use_answ->results.size()) return;

	for(result_list::iterator it_r=use_answ->results.begin(); it_r!=use_answ->results.end(); it_r++)
	{
		switch ((*it_r)->type)
		{
		case RESULT_NONE:
			//SAFEDEL((*it_r));
			//check_answ->demands.erase(it_r);
			continue;
		case RESULT_STAT:
			if((*it_r)->oper=='+')		 acl->info.st[(*it_r)->param]+=(*it_r)->count;
			else if((*it_r)->oper=='-') acl->info.st[(*it_r)->param]-=(*it_r)->count; 
			else if((*it_r)->oper=='*') acl->info.st[(*it_r)->param]*=(*it_r)->count;
			else if((*it_r)->oper=='/') acl->info.st[(*it_r)->param]/=(*it_r)->count;
			else if((*it_r)->oper=='=') acl->info.st[(*it_r)->param] =(*it_r)->count;
		//посылаем уведомление
			Send_Param(acl,TYPE_STAT,(*it_r)->param);
			continue;
		case RESULT_SKILL:
			if((*it_r)->oper=='+')		 acl->info.sk[(*it_r)->param]+=(*it_r)->count;
			else if((*it_r)->oper=='-') acl->info.sk[(*it_r)->param]-=(*it_r)->count;
			else if((*it_r)->oper=='*') acl->info.sk[(*it_r)->param]*=(*it_r)->count;
			else if((*it_r)->oper=='/') acl->info.sk[(*it_r)->param]/=(*it_r)->count;
			else if((*it_r)->oper=='=') acl->info.sk[(*it_r)->param] =(*it_r)->count;
		//посылаем уведомление
			Send_Param(acl,TYPE_STAT,(*it_r)->param);
			continue;
		case RESULT_PERK:
			if((*it_r)->oper=='+')		 acl->info.pe[(*it_r)->param]+=(*it_r)->count;
			else if((*it_r)->oper=='-') acl->info.pe[(*it_r)->param]-=(*it_r)->count;
			else if((*it_r)->oper=='*') acl->info.pe[(*it_r)->param]*=(*it_r)->count;
			else if((*it_r)->oper=='/') acl->info.pe[(*it_r)->param]/=(*it_r)->count;
			else if((*it_r)->oper=='=') acl->info.pe[(*it_r)->param] =(*it_r)->count;
		//посылаем уведомление
			Send_Param(acl,TYPE_STAT,(*it_r)->param);
			continue;
		case RESULT_VAR:
			sql.ChangeVarNPC(npc->info.id,(*it_r)->var_name,acl->info.id,(*it_r)->oper,(*it_r)->count);
			continue;
		default:
			continue;
		}
	}
}
/*
void CServer::Scr_SkillUD(BYTE act, CLIENT* acl, CrID target_id)
{

}
*/
int CServer::Act_Attack(CLIENT* acl, BYTE rate_object, CrID target_id)
{
//LogExecStr("Выполняется действие №1: АТАКА...");
	//находим атакуемого
	if(acl->info.id==target_id) return 0;
	cl_map::iterator it=map_cr[acl->info.map].find(target_id);
	if(it==map_cr[acl->info.map].end()) return 0;
	CLIENT* t_acl=(*it).second;
	//проверяем не мертв ли атакуемый
	if(t_acl->info.cond==COND_DEAD) return 0;
	//находим дистанцию
	WORD dist=sqrt(pow(acl->info.x-t_acl->info.x,2)+pow(acl->info.y-t_acl->info.y,2));
//LogExecStr("дистанция до цели =%d...",dist);
	//проверяем дистанцию
	int wpn_max_dist=0;
	int wpn_eff_dist=0;
	int dmg_max=0;
	int dmg_min=0;
	switch(rate_object)
	{
	case 1:
		wpn_max_dist=acl->info.a_obj->object->p111[OBJ_WEAP_PA_MAX_DIST];
		wpn_eff_dist=acl->info.a_obj->object->p111[OBJ_WEAP_PA_EFF_DIST];
		dmg_max=acl->info.a_obj->object->p111[OBJ_WEAP_PA_DMG_MAX];
		dmg_min=acl->info.a_obj->object->p111[OBJ_WEAP_PA_DMG_MIN];
		break;
	case 2:
		wpn_max_dist=acl->info.a_obj->object->p111[OBJ_WEAP_SA_MAX_DIST];
		wpn_eff_dist=acl->info.a_obj->object->p111[OBJ_WEAP_SA_EFF_DIST];
		dmg_max=acl->info.a_obj->object->p111[OBJ_WEAP_SA_DMG_MAX];
		dmg_min=acl->info.a_obj->object->p111[OBJ_WEAP_SA_DMG_MIN];
		break;
	case 3:
		wpn_max_dist=acl->info.a_obj->object->p111[OBJ_WEAP_TA_MAX_DIST];
		wpn_eff_dist=acl->info.a_obj->object->p111[OBJ_WEAP_TA_EFF_DIST];
		dmg_max=acl->info.a_obj->object->p111[OBJ_WEAP_TA_DMG_MAX];
		dmg_min=acl->info.a_obj->object->p111[OBJ_WEAP_TA_DMG_MIN];
		break;
	default: return 0;
	}

	if(dist>wpn_max_dist) return 0;

	bool eff_attack=true;
	if(dist>wpn_eff_dist) eff_attack=false;

	//рассчитываем вероятность попадания
	//Skill+ammo_mod-AC_target-range
//	WORD ammo_mod=0;
//	WORD ammo_DR=0;
//	if(acl->info.a_obj->object->p[25])
//	{
//		ammo_mod=1;
//		ammo_DR=1;
//	}
//	WORD Skill=acl->info.s[acl->info.a_obj->object->p[act+1]];
//	WORD AC_target=t_acl->info.s[9];
//	WORD min_dmg=acl->info.a_obj->object->p[act+17];
//	WORD max_dmg=acl->info.a_obj->object->p[act+16];
//	WORD luck=acl->info.s[6];
//	WORD DR=acl->info.s[acl->info.a_obj->object->p[act+15]+24];

//	int hit=Skill+ammo_mod-AC_target-dist;
//	if(hit<10) hit=(100-hit-luck)/10;

//	int dmg=max_dmg*((100-DR-ammo_DR)/100);
//	if(dmg<min_dmg) dmg=min_dmg;

//LogExecStr("лифе олд =%d...",t_acl->info.st[ST_CURRENT_HP]);
	int t_cur_hp=t_acl->info.st[ST_CURRENT_HP];
	int t_max_hp=t_acl->info.st[ST_MAX_LIFE];

	int dmg=(dmg_max+dmg_min)/2;

	t_cur_hp-=dmg;
	if(t_cur_hp<0) t_cur_hp=0;

	//t_acl->info.st[ST_CURRENT_HP]=t_cur_hp;
	CHANGE_STAT(t_acl,ST_CURRENT_HP,=,t_cur_hp);
//LogExecStr("лифе нев =%d...",t_acl->info.st[ST_CURRENT_HP]);

	//обновляем ориентацию атакуемого
//	BYTE new_ori=acl->info.ori+3;
//	if(new_ori>5) new_ori-=6;

	bool attack_miss=false;

	bool attack_front=false;
	if(int(acl->info.ori-t_acl->info.ori)>2 || int(acl->info.ori-t_acl->info.ori)<-1) attack_front=true;
//	t_acl->info.ori=new_ori;

	//рассылаем всем итоги стычки
	MSGTYPE msg;
	//отсылает наподавший инфу о атаке
	Send_Action(acl,ACT_USE_OBJ,rate_object);
	if(!t_acl->info.st[ST_CURRENT_HP]) //отыгрываем смерть
	{
//LogExecStr("смерть\n",dmg);
		//устанавливаем флаг смерти игроку
		t_acl->info.cond=COND_DEAD;
		if(attack_front==true)
			t_acl->info.cond_ext=COND_DEAD_NORMAL_UP;
		else
			t_acl->info.cond_ext=COND_DEAD_NORMAL_DOWN;

		//отылаем ему результаты
//		Send_Stat(t_acl,50,t_acl->info.s[50]); //смерть
//		Send_Stat(t_acl,ST_CURRENT_HP,t_acl->info.st[ST_CURRENT_HP]); //жизнь
		msg=NETMSG_CRITTER_ACTION;
		t_acl->bout << msg;
		t_acl->bout << t_acl->info.id;
		t_acl->bout << ACT_DEAD;
		t_acl->bout << t_acl->info.a_obj->object->id;
		t_acl->bout << t_acl->info.a_obj_arm->object->id;
		t_acl->bout << t_acl->info.cond_ext;
		t_acl->bout << t_acl->info.ori;
		//отсылаем результаты смерти всем кого он видит
		Send_Action(t_acl,ACT_DEAD,t_acl->info.cond_ext);
	}
	else //отыгрываем повреждение
	{
		BYTE defeat_type=0;

		if(attack_front==true)
			defeat_type=ACT_DEFEAT_FRONT;
		else
			defeat_type=ACT_DEFEAT_REAR;

		if(attack_miss==true) defeat_type=ACT_DEFEAT_MISS;

		//отсылаем поврежденному инфу
		//Send_Stat(t_acl,ST_CURRENT_HP,t_acl->info.s[ST_CURRENT_HP]); //жизнь

		msg=NETMSG_CRITTER_ACTION;
		t_acl->bout << msg;
		t_acl->bout << t_acl->info.id;
		t_acl->bout << ACT_DEFEAT;
		t_acl->bout << t_acl->info.a_obj->object->id;
		t_acl->bout << t_acl->info.a_obj_arm->object->id;
		t_acl->bout << defeat_type;
		t_acl->bout << t_acl->info.ori;
		//отсылает атакуемумый инфу о приниятии повреждения
		Send_Action(t_acl,ACT_DEFEAT,defeat_type);
	}

	return 1;
}
/*
void CServer::Scr_Reload(BYTE act, CLIENT* acl)
{

}
*/
//!Cvet ---------------------------------------------
void CServer::Send_AddCritter(CLIENT* acl,crit_info* pinfo)
{
    MSGTYPE msg=NETMSG_ADDCRITTER;
//	char mname[MAX_NAME+1];
	acl->bout << msg;
	acl->bout << pinfo->id;
	acl->bout << pinfo->base_type;
	acl->bout << pinfo->a_obj->object->id;
	acl->bout << pinfo->a_obj_arm->object->id;
	acl->bout << pinfo->x;
	acl->bout << pinfo->y;
	acl->bout << pinfo->ori;
	acl->bout << pinfo->st[ST_GENDER];
	acl->bout << pinfo->cond;
	acl->bout << pinfo->cond_ext;
	acl->bout << pinfo->flags;
//	acl->bout.push(MakeName(pinfo->name,mname),MAX_NAME);
	acl->bout.push(pinfo->name,MAX_NAME);
	for(int i=0;i<5;i++)
		acl->bout.push(pinfo->cases[i],MAX_NAME);

//	LogExecStr("Посылаю данные id=%d о обноружении id=%d\n", acl->info.id, pinfo->id);
}

int CServer::Output(CLIENT* acl)
{

	if(!acl->bout.pos) return 1;

	if(acl->bout.len>=outLEN)
	{
		while(acl->bout.len>=outLEN) outLEN<<=1;
		SAFEDELA(outBUF);
		outBUF=new char[outLEN];
	}

	acl->zstrm.next_in=(UCHAR*)acl->bout.data;
	acl->zstrm.avail_in=acl->bout.pos;
	acl->zstrm.next_out=(UCHAR*)outBUF;
	acl->zstrm.avail_out=outLEN;

	DWORD br;
	WriteFile(hDump,acl->bout.data,acl->bout.pos,&br,NULL);
			
	if(deflate(&acl->zstrm,Z_SYNC_FLUSH)!=Z_OK)
	{
		LogExecStr("Deflate error forSockID=%d\n",acl->s);
		acl->state=STATE_DISCONNECT;
		return 0;
	}

	int tosend=acl->zstrm.next_out-(UCHAR*)outBUF;
	LogExecStr("id=%d, send %d->%d bytes\n",acl->info.id,acl->bout.pos,tosend);
	int sendpos=0;
	while(sendpos<tosend)
	{
		int bsent=send(acl->s,outBUF+sendpos,tosend-sendpos,0);
		sendpos+=bsent;
		if(bsent==SOCKET_ERROR)
		{
			LogExecStr("SOCKET_ERROR whilesend forSockID=%d\n",acl->s);
			acl->state=STATE_DISCONNECT;
			return 0;
		}
	}

	acl->bout.reset();

	return 1;
}

int CServer::Init()
{
	if(Active) return 1;

	Active=0;

	LogExecStr("***   Starting initialization   ****\n");

	hDump=CreateFile(".\\dump.dat",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	WSADATA WsaData;
	if(WSAStartup(0x0101,&WsaData))
	{
		LogExecStr("WSAStartup error!");
		goto SockEND;
	}
	s=socket(AF_INET,SOCK_STREAM,0);

	UINT port;
	port=GetPrivateProfileInt("server","port",4000,".\\foserv.cfg");
	
	SOCKADDR_IN sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(port);
	sin.sin_addr.s_addr=INADDR_ANY;

	LogExecStr("Starting local server on port %d: ",port);

	if(bind(s,(sockaddr*)&sin,sizeof(sin))==SOCKET_ERROR)
	{
		LogExecStr("Bind error!");
		goto SockEND;
	}

	LogExecStr("OK\n");

	if(listen(s,5)==SOCKET_ERROR)
	{
		LogExecStr("listen error!");
		goto SockEND;
	}

	if(!sql.Init_mySQL())
		goto SockEND;

	LoadSocials(sql.mySQL);

//!Cvet ++++++++++++++++++++++++++++++++++++++++
	CreateParamsMaps();

	if(!mm.Init()) goto SockEND;

	if(!mm.LoadAllMaps()) goto SockEND;

	//загрузка объектов
	if(!LoadAllStaticObjects())
	{
		LogExecStr("Загрузка статических объектов прошла со сбоями!!!\n");
		goto SockEND;
	}
	//создаем всех клиентов
	if(!LoadAllPlayers())
	{
		LogExecStr("Создание игроков прошли со сбоями!!!\n");
		goto SockEND;
	}
	//создаем всю динамику
	if(!LoadAllObj())
	{
		LogExecStr("Создание динамических объектов прошла со сбоями!!!\n");
		goto SockEND;
	}
	//загружаем НПЦ
	if(!NPC_LoadAll())
	{
		LogExecStr("Загрузка НПЦ прошла со сбоями!!!\n");
		goto SockEND;
	}

//	LogExecStr("Создаем объекты\n");
	
//	CreateObjToPl(101,1200);
//	CreateObjToPl(102,1100);
//	CreateObjToPl(102,1200);
//	CreateObjToPl(103,1100);
//	CreateObjToPl(103,1200);
//	CreateObjToPl(104,1100);
//	CreateObjToPl(104,1200);

	CreateObjToTile(11,61,112,1301);
	CreateObjToTile(11,61,112,1301);
	CreateObjToTile(11,61,112,1301);
	CreateObjToTile(11,61,112,1301);

	CreateObjToTile(11,61,113,2016);
//	CreateObjToTile(11,61,113,1301);

	CreateObjToTile(11,58,114,1301);
	CreateObjToTile(11,64,115,1301);

//!Cvet ---------------------------------------

	sql.PrintTableInLog("objects","*");

	Active=1;
	LogExecStr("***   Initializing complete   ****\n\n");
	return 1;

SockEND:
	closesocket(s);
	ClearClients();
	return 0;
	
}

void CServer::Finish()
{
	if(!Active) return;
	
	closesocket(s);

	ClearClients();
	UnloadSocials();
	sql.Finish_mySQL();
	mm.Clear();

	LogExecStr("Server stopped\n");

	Active=0;
}

int CServer::GetCmdNum(char* cmd)
{
	my_strlwr(cmd);

	if(!strcmp(cmd,cmdlist[CMD_EXIT].cmd))
		return CMD_EXIT;

	for(int i=CMD_EXIT+1;cmdlist[i].cmd[0];i++)
		if(PartialRight(cmd,(char*)cmdlist[i].cmd))
			return i;

	return 0;
}

char* CServer::GetParam(char* cmdstr,char** next)
{
	if(!cmdstr)
	{
	 *next=NULL;
	 return NULL;
	}
	
	char* ret=NULL;
	int stop=0;
	for(int i=0;cmdstr[i];i++)
		if(cmdstr[i]!=' ') break;
	if(!cmdstr[i]) //нет первого параметра
	{
		*next=NULL;
		return ret;
	}
	ret=cmdstr+i;
	stop=i+1;
	for(i=stop;cmdstr[i];i++)
		if(cmdstr[i]==' ') break;
	if(!cmdstr[i]) //нет следующего параметра
	{
		*next=NULL;
		return ret;
	}
	cmdstr[i]=0;
	stop=i+1;
	for(i=stop;cmdstr[i];i++)
		if(cmdstr[i]!=' ') break;
	
	*next=cmdstr[i]?cmdstr+i:NULL;
	return ret;
}

char* CServer::my_strlwr(char* str)
{
	strlwr(str);
	for(int i=0;str[i];i++)
		if(str[i]>='А' && str[i]<='Я') str[i]+=0x20;
	return str;
}

char* CServer::my_strupr(char* str)
{
	strupr(str);
	for(int i=0;str[i];i++)
		if(str[i]>='а' && str[i]<='я') str[i]-=0x20;
	return str;
}


int CServer::PartialRight(char* str,char* et)
{
	int res=1;

	for(int i=0;str[i];i++)
		if(!et[i] || str[i]!=et[i]) return 0;
		
	return res;
}

char* CServer::MakeName(char* str,char* res)
{
	strcpy(res,str);
	res[0]-=0x20;
	return res;
}


void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
    return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
    free(address);
}

//!Cvet ++++++++++++++++++++++++++++++++++++++++++++++

void CServer::Send_LoginMsg(CLIENT* acl, BYTE LogMsg)
{
	MSGTYPE msg=NETMSG_LOGMSG;
	acl->bout << msg;
	acl->bout << LogMsg;
}

void CServer::Send_Move(CLIENT* acl, BYTE move_params)
{
	acl->info.ori=move_params & BIN8(00000111);

	if(!FLAG(move_params,BIN8(00010000)))
		if(FLAG(acl->info.flags,FCRIT_PLAYER)) Send_XY(acl);

	if(!acl->info.cl_id.empty())
	{
		MSGTYPE msg=NETMSG_CRITTER_MOVE;
		CLIENT* c=NULL;
		cl_map::iterator it;
		DWORD find_id;
		for(it_li=acl->info.cl_id.begin(); it_li!=acl->info.cl_id.end();)
		{
			find_id=(*it_li);
			it=map_cr[acl->info.map].find(find_id);
			if(it==map_cr[acl->info.map].end()){it_li++; DelClFromListId(acl,find_id); continue;}
			c=(*it).second;

			c->bout << msg; 
			c->bout << acl->info.id;
			c->bout << move_params;

			c->bout << acl->info.x;
			c->bout << acl->info.y;

			it_li++;
//LogExecStr("Посылаю данные id=%d о ходе игроком id=%d\n",c->info.id, acl->info.id);
		}
	}
}

void CServer::Send_Action(CLIENT* acl, BYTE num_action, BYTE rate_action)
{
//LogExecStr("Send_Action - BEGIN %d\n",acl->info.id);
	if(!acl->info.cl_id.empty())
	{
		MSGTYPE msg=NETMSG_CRITTER_ACTION;
		CLIENT* c=NULL;
		cl_map::iterator it;
		DWORD find_id;
		for(it_li=acl->info.cl_id.begin(); it_li!=acl->info.cl_id.end();)
		{
			find_id=(*it_li);
			it=map_cr[acl->info.map].find(find_id);
			if(it==map_cr[acl->info.map].end()){it_li++; DelClFromListId(acl,find_id); continue;}
			c=(*it).second;

			c->bout << msg;
			c->bout << acl->info.id;
			c->bout << num_action;
			c->bout << acl->info.a_obj->object->id;
			c->bout << acl->info.a_obj_arm->object->id;
			c->bout << rate_action;
			c->bout << acl->info.ori;

			it_li++;
//LogExecStr("Посылаю данные id=%d о действии id=%d\n", c->info.id, acl->info.id);
		}
	}
//LogExecStr("Send_Action - END\n");
}

void CServer::Send_RemoveCritter(CLIENT* acl, CrID remid)
{
	MSGTYPE msg=NETMSG_REMOVECRITTER;

	acl->bout << msg;
	acl->bout << remid;
//LogExecStr("Посылаю данные id=%d о скрывании id=%d\n", acl->info.id, remid);
}

void CServer::Send_AddObjOnMap(CLIENT* acl, dyn_obj* o)
{
	MSGTYPE msg=NETMSG_ADD_OBJECT_ON_MAP;

	acl->bout << msg;
	acl->bout << o->x;
	acl->bout << o->y;
	acl->bout << o->object->id;
}

void CServer::Send_RemObjFromMap(CLIENT* acl, dyn_obj* o)
{
	MSGTYPE msg=NETMSG_REMOVE_OBJECT_FROM_MAP;

	acl->bout << msg;
	acl->bout << o->x;
	acl->bout << o->y;
	acl->bout << o->object->id;
}

void CServer::Send_AddObject(CLIENT* acl, dyn_obj* send_obj)
{
	//отсылаем объект игроку
	MSGTYPE msg=NETMSG_ADD_OBJECT;

	acl->bout << msg;
	//ид динамического и статического объекта
	acl->bout << send_obj->id;
	acl->bout << send_obj->object->id;
	//активный или в инвентаре
	acl->bout << send_obj->active;
	//динамические покозатели
	acl->bout << send_obj->holder;
	acl->bout << send_obj->id_bull;
	acl->bout << send_obj->holder_ext;
	acl->bout << send_obj->id_bull_ext;
	acl->bout << send_obj->tick;
	acl->bout << send_obj->broken;
}

void CServer::Send_RemObject(CLIENT* acl, dyn_obj* send_obj)
{
	//отсылаем запрос игроку на удаление объекта
	MSGTYPE msg=NETMSG_REMOVE_OBJECT;

	acl->bout << msg;
	//ид динамического
	acl->bout << send_obj->id;
}

void CServer::Send_WearObject(CLIENT* acl, dyn_obj* send_obj)
{
	//извещаем игрока о износе объекта
	MSGTYPE msg=NETMSG_WEAR_OBJECT;

	acl->bout << msg;
	acl->bout << send_obj->id;
	acl->bout << send_obj->tick;
	acl->bout << send_obj->broken;
}

void CServer::Send_Map(CLIENT* acl, WORD map_num)
{
	LogExecStr("Отправка карты №%d игроку ID №%d...",map_num,acl->info.id);

	//acl->bout << map_size;
	//acl->bout.push(

	LogExecStr("OK\n");
}

void CServer::SetVisibleCl(CLIENT* acl)
{
	int vis;
	int dist;

	CLIENT* c=NULL;	
	for(cl_map::iterator it=map_cr[acl->info.map].begin();it!=map_cr[acl->info.map].end();it++)
	{
		if((*it).second->info.id==acl->info.id) continue;

		c=(*it).second;
		dist=DISTANCE(acl,c);

		//для себя
	//	if(c->info.s[423])
	//		vis=acl->info.look-(c->info.s[108]/10);
	//	else
			vis=acl->info.look;
		
		if(vis>=dist)
		{
			if(AddClIntoListId(c,acl->info.id)) Send_AddCritter(acl,&c->info);
		}
		else
		{
			if(DelClFromListId(c,acl->info.id)) Send_RemoveCritter(acl,c->info.id);
		}

		//для оппонента
		if(c->info.flags & FCRIT_NPC) continue;
		if(c->info.flags & FCRIT_MOB) continue;
		if(c->info.flags & FCRIT_DISCONNECT) continue;
	//	if(acl->info.s[423])
	//		vis=c->info.look-(acl->info.s[108]/10);
	//	else
			vis=c->info.look;

		if(vis>=dist)
		{
			if(AddClIntoListId(acl,c->info.id)) Send_AddCritter(c,&acl->info);
		}
		else
		{
			if(DelClFromListId(acl,c->info.id)) Send_RemoveCritter(c,acl->info.id);
		}
	}
}

void CServer::SetVisibleObj(CLIENT* acl)
{
	int hx;
	int hy;
	int dist;
	dyn_map::iterator it2;
	for(dyn_map_map::iterator it=mm.objects_map[acl->info.map].begin();it!=mm.objects_map[acl->info.map].end();it++)
	{
		hx=(*it).first >> 16;
		hy=(*it).first & 0xFFFF;

		dist=sqrt(pow(acl->info.x-hx,2)+pow(acl->info.y-hy,2));

		if(acl->info.look>=dist)
		{
			for(it2=(*it).second->begin();it2!=(*it).second->end();it2++)
				if(AddObjIntoListInd(acl,(*it2).second->id)) Send_AddObjOnMap(acl,(*it2).second);
		}
		else
		{
			for(it2=(*it).second->begin();it2!=(*it).second->end();it2++)
				if(DelObjFromListInd(acl,(*it2).second->id)) Send_RemObjFromMap(acl,(*it2).second);
		}
	}
}

inline int CServer::AddClIntoListId(CLIENT* acl, CrID add_id)
{
	if(!acl->info.cl_id.count(add_id))
	{
//		LogExecStr("ADDED %d\n",add_id);
		acl->info.cl_id.insert(add_id);
		return 1;
	}
	return 0;
}

inline int CServer::DelClFromListId(CLIENT* acl, CrID del_id)
{
	if(acl->info.cl_id.count(del_id))
	{
//		LogExecStr("DELETED\n");
		acl->info.cl_id.erase(del_id);
		return 1;
	}
	return 0;
}

void CServer::DelClFromAllListId(CrID del_id, WORD num_map)
{
	for(cl_map::iterator it=map_cr[num_map].begin();it!=map_cr[num_map].end();it++)
	{
		if((*it).second->info.id==del_id) continue;

		if((*it).second->info.cl_id.count(del_id))
			(*it).second->info.cl_id.erase(del_id);
	}
}

int CServer::AddObjIntoListInd(CLIENT* acl, DWORD add_ind)
{
	if(!acl->info.obj_id.count(add_ind))
	{
		acl->info.obj_id.insert(add_ind);
		return 1;
	}
	return 0;
}

int CServer::DelObjFromListInd(CLIENT* acl, DWORD del_ind)
{
	if(acl->info.obj_id.count(del_ind))
	{
		acl->info.obj_id.erase(del_ind);
		return 1;
	}
	return 0;
}

int CServer::LoadAllStaticObjects()
{
	LogExecStr("Загрузка статических объектов...");

	FILE *cf;
	FILE *cf2;
	params_map::iterator it_o;

	for(stat_map::iterator it=all_s_obj.begin(); it!=all_s_obj.end(); it++)
	{
		SAFEDEL((*it).second);
		all_s_obj.erase(it);
	}

	if((cf=fopen("objects\\all_obj.st","rt"))==NULL)
	{
		LogExecStr("Файл all_obj.st не найден\n");
		return 0;
	}

	int cnt_obj=0;
	int tmpi=0;
	char tmpc[120];

	while(!feof(cf))
	{
		tmpi=0;
		fscanf(cf,"%d",&tmpi);
		if(!tmpi) break;

		sprintf(tmpc,"objects\\%d.st",tmpi);
		if((cf2=fopen(tmpc,"rt"))==NULL)
		{
			LogExecStr("Файл |%s| не найден\n",tmpc);
			return 0;
		}

		stat_obj* new_obj;
		new_obj= new stat_obj;

		new_obj->id=tmpi;
		fscanf(cf2,"%s",&tmpc);
		it_o=object_map.find(tmpc);
		if(it_o==object_map.end())
		{
			LogExecStr("Параметр |%s| не найден",tmpc);
			return 0;
		}
		new_obj->type=(*it_o).second;

		while(!feof(cf2))
		{
			fscanf(cf2,"%s%d",&tmpc,&tmpi);

			it_o=object_map.find(tmpc);
			if(it_o==object_map.end())
			{
				LogExecStr("Параметр |%s| не найден",tmpc);
				return 0;
			}
			new_obj->p111[(*it_o).second]=tmpi;
		}

		all_s_obj[new_obj->id]=new_obj;

		fclose(cf2);
		cnt_obj++;
	}
	
	fclose(cf);

	LogExecStr("OK (%d объектов)\n",cnt_obj);

	return 1;
}

int CServer::LoadAllObj() //загрузка динамических объектов из mySQL
{
	LogExecStr("Загрузка динамических объектов\n");
	//узнаем кол-во записей всего
	int find_obj=sql.CountTable("objects","id");

	int add_obj=0;
	int num_obj=1;
	if(find_obj)
		while(add_obj<find_obj)
		{
			if(num_obj==sql.GetInt("objects","id","id",num_obj))
			{
				dyn_obj* new_obj=new dyn_obj;
				
				new_obj->id=num_obj;
				sql.LoadDataObject(new_obj);
				new_obj->object=all_s_obj[sql.GetInt("objects","num_st","id",new_obj->id)];

				//присваеваем тайлу ссылку на динамический объект
				if(new_obj->map) mm.InsertObjOnMap(new_obj,new_obj->map,new_obj->x,new_obj->y);
				//присваеваем игроку ссылку на динамический объект
				else if(new_obj->player)
				{
					for(cl_map::iterator it=cl.begin(); it!=cl.end(); it++)
						if((*it).second->info.id==new_obj->player)
						{
							(*it).second->info.obj[num_obj]=new_obj;

							if(new_obj->active)
								if(!new_obj->object->type==OBJ_TYPE_ARMOR)
									(*it).second->info.a_obj_arm=new_obj;
								else
									(*it).second->info.a_obj=new_obj;
							break;
						}
				}
				//присваеваем общему списку ссылку на динамический объект
				all_obj[num_obj]=new_obj;

				add_obj++;
			}
			num_obj++;
		}

	LogExecStr("Загрузка динамических объектов прошла успешно\n");
	return 1;
}

void CServer::SaveAllObj() //запись динамических объектов в mySQL
{
	for(dyn_map::iterator it=all_obj.begin(); it!=all_obj.end(); it++)
		sql.SaveDataObject((*it).second);
}

void CServer::CreateObjToTile(MapTYPE c_map, HexTYPE c_x, HexTYPE c_y, WORD num_st_obj)
{
	if(!c_map || c_map>MAX_MAPS) return;
	if(c_x>MAXTILEX || c_y>MAXTILEY) return;

	dyn_obj* new_obj=new dyn_obj;

	stat_map::iterator it_st=all_s_obj.find(num_st_obj);
	if(it_st==all_s_obj.end())
	{
		delete new_obj;
		return;
	}

	new_obj->object=(*it_st).second;

	new_obj->map=c_map;
	new_obj->x=c_x;
	new_obj->y=c_y;

	new_obj->tick=new_obj->object->p111[OBJ_LIVETIME]*1000000;
	new_obj->last_tick=GetTickCount();

	DWORD cur_obj_id=sql.NewObject(new_obj);

	all_obj[cur_obj_id]=new_obj;

	mm.InsertObjOnMap(new_obj,c_map,c_x,c_y);

LogExecStr("Item Create to TL =%d\n",cur_obj_id);
}

void CServer::CreateObjToPl(CrID c_pl_idchannel, WORD num_st_obj)
{
	dyn_obj* new_obj=new dyn_obj;

	stat_map::iterator it_st=all_s_obj.find(num_st_obj);
	if(it_st==all_s_obj.end())
	{
		delete new_obj;
		return;
	}

	new_obj->object=(*it_st).second;

	new_obj->tick=new_obj->object->p111[OBJ_LIVETIME]*1000000;
	new_obj->last_tick=GetTickCount();

	cl_map::iterator it=cl.find(c_pl_idchannel);
	if(it==cl.end())
	{
		delete new_obj;
		return;
	}

	new_obj->player=(*it).second->info.id;

	DWORD cur_obj_id=sql.NewObject(new_obj);

	(*it).second->info.obj[cur_obj_id]=new_obj;
	all_obj[cur_obj_id]=new_obj;

LogExecStr("Item Create to PL =%d\n",cur_obj_id);
}

void CServer::DeleteObj(DWORD id_obj)
{
	dyn_map::iterator it=all_obj.find(id_obj);
	if(it==all_obj.end()) return;

	sql.DeleteDataObject((*it).second);

	if((*it).second->map)
	{
	//	mm.EraseObject((*it).second,&it);
		mm.EraseObjFromMap((*it).second);
	}
	else
	{
		cl_map::iterator it2=cl.find((*it).second->player);
		if(it2==cl.end()) return;
		(*it2).second->info.obj.erase(it);
	}

	delete (*it).second;
	all_obj.erase(it);
}

void CServer::TransferDynObjPlPl(CLIENT* from_acl, CLIENT* to_acl, DWORD id_obj)
{

}

void CServer::TransferDynObjTlPl(tile_info* from_tile, CLIENT* to_acl, DWORD id_obj)
{

}

void CServer::TransferDynObjPlTl(CLIENT* from_acl, tile_info* to_tile, DWORD id_obj)
{
	
}

int CServer::LoadAllPlayers()
{
	LogExecStr("Загрузка игроков\n");
	//узнаем кол-во записей всего
	int find_cl=sql.CountTable("users","id")-1;
		
	sql.PrintTableInLog("users","*");

	int add_cl=0;
	int num_cl=10001;
	if(find_cl)
		while(add_cl<find_cl)
		{
			if(num_cl==sql.GetInt("users","id","id",num_cl))
			{
				CLIENT* ncl=new CLIENT;
				ncl->s=NULL;
				ncl->info.id=num_cl;
				ncl->state=STATE_DISCONNECT;
				strcpy(ncl->info.login,sql.GetChar("users","login","id",num_cl));
				strcpy(ncl->info.pass,sql.GetChar("users","pass","id",num_cl));
				sql.LoadDataPlayer(&ncl->info);

				GenLook(ncl);

				ncl->info.cond=COND_LIFE;
				ncl->info.cond_ext=COND_LIFE_NONE;
				ncl->info.st[ST_CURRENT_HP]=ncl->info.st[ST_MAX_LIFE]; //!!!!!!!!
				
				ncl->info.a_obj=&ncl->info.def_obj1;
				ncl->info.a_obj_arm=&ncl->info.def_obj2;

				ncl->info.a_obj->object=all_s_obj[ncl->info.base_type];
				ncl->info.a_obj_arm->object=all_s_obj[ncl->info.base_type+200];
				//добавляем в список карты
				if(ncl->info.map)
					map_cr[ncl->info.map][ncl->info.id]=ncl;
				//добовляем в общий список
				cr[ncl->info.id]=ncl;
				add_cl++;

//				NumCritters++;
			}
			num_cl++;
		}

	LogExecStr("Загрузка игроков прошло успешно\n");
	return 1;
}

void CServer::SaveAllDataPlayers()
{
	cl_map::iterator it;
//сохраняем клиентов
	for(it=cl.begin();it!=cl.end();it++)
	{
		sql.SaveDataPlayer(&(*it).second->info);
	}
//сохраняем нпц
	for(it=pc.begin();it!=pc.end();it++)
	{
		sql.SaveDataNPC(&(*it).second->info);
	}
}

void CServer::GenParam(CLIENT* acl)
{
//s[7]..s[38]
	acl->info.st[ST_MAX_LIFE			]=(acl->info.st[ST_STRENGHT]*1)+(acl->info.st[ST_ENDURANCE]*2)+15;
	acl->info.st[ST_MAX_COND			]=0;
	acl->info.st[ST_ARMOR_CLASS			]=acl->info.st[ST_AGILITY];
	acl->info.st[ST_MELEE_DAMAGE		]=acl->info.st[ST_STRENGHT]/2+1;
	acl->info.st[ST_WEAPON_DAMAGE		]=0;
	acl->info.st[ST_CARRY_WEIGHT		]=acl->info.st[ST_STRENGHT]*8;
	acl->info.st[ST_SEQUENCE			]=0;
	acl->info.st[ST_HEALING_RATE		]=acl->info.st[ST_ENDURANCE];
	acl->info.st[ST_CRITICAL_CHANCE		]=0;
	acl->info.st[ST_MAX_CRITICAL		]=0;
	acl->info.st[ST_INGURE_ABSORB		]=0;
	acl->info.st[ST_LASER_ABSORB		]=0;
	acl->info.st[ST_FIRE_ABSORB			]=0;
	acl->info.st[ST_PLASMA_ABSORB		]=0;
	acl->info.st[ST_ELECTRO_ABSORB		]=0;
	acl->info.st[ST_EMP_ABSORB			]=0;
	acl->info.st[ST_BLAST_ABSORB		]=0;
	acl->info.st[ST_INGURE_RESIST		]=0;
	acl->info.st[ST_LASER_RESIST		]=0;
	acl->info.st[ST_FIRE_RESIST			]=0;
	acl->info.st[ST_PLASMA_RESIST		]=0;
	acl->info.st[ST_ELECTRO_RESIST		]=0;
	acl->info.st[ST_EMP_RESIST			]=0;
	acl->info.st[ST_BLAST_RESIST		]=0;
	acl->info.st[ST_RADIATION_RESISTANCE]=0;
	acl->info.st[ST_POISON_RESISTANCE	]=0;
}

void CServer::GenLook(CLIENT* acl)
{
	acl->info.look=(acl->info.st[ST_PERCEPTION]*5);//*(1-(0.4*acl->info.pe[422]+
		//0.4*acl->info.pe[423]))+(10*acl->info.pe[353]);
}

void CServer::GenWear(dyn_obj* wear_obj, bool use_obj)
{
//	wear_obj->tick-=GetTickCount()-wear_obj->last_tick;
//	if(use_obj)
//		wear_obj->tick-=wear_obj->object->p[17]/2000; //штраф за использование
//	wear_obj->last_tick=wear_obj->tick;
}

int CServer::NPC_LoadAll()
{
	char file_name[64];
	char key[16];
	char name_npc[21];

	int loaded_npc=0;

	CLIENT* npc;

	sprintf(file_name,"%slist_npc.npc", PATH_NPC);
	int max_npc=0;
	max_npc=GetPrivateProfileInt("settings","max_npc",0,file_name);

	if(max_npc<=0)
	{
		LogExecStr("Неправильное значение максимального кол-ва НПЦ: %d\n", max_npc);
		return 0;
	}

	for(int num_npc=1; loaded_npc<max_npc && num_npc<MAX_NPC; num_npc++)
	{
		sprintf(file_name,"%slist_npc.npc", PATH_NPC);
		sprintf(key,"%d",num_npc);
		GetPrivateProfileString("npc",key,"e",name_npc,20,file_name);
		if(name_npc[0]=='e') continue;

		LogExecStr("Инициализация НПЦ: %s...", name_npc);

		npc=new CLIENT;
		npc->i_npc=new npc_info;

		sprintf(file_name,"%s%s.npc", PATH_NPC, name_npc);

		bool Err_load=false;

		Err_load=false;
	//основные параметры
		if((npc->info.id		=GetPrivateProfileInt("info","id"		,-1,file_name))==-1) Err_load=true;
		if((npc->info.base_type	=GetPrivateProfileInt("info","base_type",-1,file_name))==-1) Err_load=true;
		if((npc->info.map		=GetPrivateProfileInt("info","map"		,-1,file_name))==-1) Err_load=true;
		if((npc->info.x			=GetPrivateProfileInt("info","x"		,-1,file_name))==-1) Err_load=true;
		if((npc->info.y			=GetPrivateProfileInt("info","y"		,-1,file_name))==-1) Err_load=true;
		if((npc->info.ori		=GetPrivateProfileInt("info","ori"		,-1,file_name))==-1) Err_load=true;

		GetPrivateProfileString("info","name"	,"e",npc->info.name		,MAX_NAME,file_name);
		if(npc->info.name[0]		=='e') Err_load=true;
		GetPrivateProfileString("info","cases0"	,"e",npc->info.cases[0]	,MAX_NAME,file_name);
		if(npc->info.cases[0][0]	=='e') Err_load=true;
		GetPrivateProfileString("info","cases1"	,"e",npc->info.cases[1]	,MAX_NAME,file_name);
		if(npc->info.cases[1][0]	=='e') Err_load=true;
		GetPrivateProfileString("info","cases2"	,"e",npc->info.cases[2]	,MAX_NAME,file_name);
		if(npc->info.cases[2][0]	=='e') Err_load=true;
		GetPrivateProfileString("info","cases3"	,"e",npc->info.cases[3]	,MAX_NAME,file_name);
		if(npc->info.cases[3][0]	=='e') Err_load=true;
		GetPrivateProfileString("info","cases4"	,"e",npc->info.cases[4]	,MAX_NAME,file_name);
		if(npc->info.cases[4][0]	=='e') Err_load=true;

	//объекты
		npc->info.a_obj=&npc->info.def_obj1;
		npc->info.a_obj_arm=&npc->info.def_obj2;
		npc->info.a_obj->object=all_s_obj[npc->info.base_type];
		npc->info.a_obj_arm->object=all_s_obj[npc->info.base_type+200];

	//статы, скиллы, перки
		int go=0;
		for(go=0; go<ALL_STATS ; go++) npc->info.st[go]=5;
		for(go=0; go<ALL_SKILLS; go++) npc->info.sk[go]=5;
		for(go=0; go<ALL_PERKS ; go++) npc->info.pe[go]=0;
		npc->info.st[ST_GENDER]=0;
		npc->info.st[ST_AGE]=50;

		npc->info.st[ST_CURRENT_HP]=50;
		npc->info.st[ST_MAX_LIFE]=50;

		GenParam(npc);
		GenLook(npc);

	//состояния
		npc->info.cond=COND_LIFE;
		npc->info.cond_ext=COND_LIFE_NONE;
		npc->info.flags=FCRIT_NPC;
		npc->state=STATE_GAME;

		if(Err_load==true)
		{
			LogExecStr("Ошибка при загрузке основных параметров\n");
			return 0;
		}

	//переменные
		FILE* cf;
		char p_tmp1[200];
		int p_tmpi=0;

		if((cf=fopen(file_name, "rt"))==NULL)
		{
			LogExecStr("Файл не найден |%s|\n", file_name);
			return 0;
		}
		
		while(!feof(cf))
		{
			fscanf(cf, "%s", &p_tmp1);
			if(!stricmp(p_tmp1,"[vars]"))
			{
				char var_name[30];
				int var_count=0;
				int var_min=0;
				int var_max=0;

				while(!feof(cf))
				{
					fscanf(cf, "%s", &p_tmp1);
					if(!stricmp(p_tmp1,"[end_vars]")) break;
				//имя
			//		if(sql.GetInt("npc_vars_templates","COUNT(*)","name",p_tmp1))
			//			{ LogExecStr("Ошибка в переменных - реиндитификация\n"); return 0; }
					strcpy(var_name,p_tmp1);

				//count
					fscanf(cf, "%d", &p_tmpi);
					var_count=p_tmpi;

				//min
					fscanf(cf, "%s%d", &p_tmp1, &p_tmpi);
					var_min=p_tmpi;

				//max
					fscanf(cf, "%s%d", &p_tmp1, &p_tmpi);
					var_max=p_tmpi;

					if(var_count<var_min || var_count>var_max)
						{ LogExecStr("Ошибка в переменных - неверные данные\n"); return 0; }

					sql.Query("INSERT INTO npc_vars_templates (npc_id,name,count,min,max) VALUES('%d','%s','%d','%d','%d')",
						npc->info.id,var_name,var_count,var_min,var_max);
				}
				break;
			}
		}
		fclose(cf);

	//диалоги
		int read_int=0;
		int read_int2=0;
		int read_int3=0;
		char read_str[100];
		char read_str2[100];
		char read_str3[100];
		char ch[20];
		BOOL read_proc=FALSE;

		if((cf=fopen(file_name, "rt"))!=NULL)
		{
			while(!feof(cf))
			{
				fscanf(cf, "%c", &ch);
				if(ch[0]=='&')
				{
					read_proc=TRUE;
					break;
				}
			}

			npc_dialog* dlg;
			answer* answ;
//ДИАЛОГИ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			while((!feof(cf))&&(read_proc))
			{
				dlg=new npc_dialog;
			//ID диалога
				fscanf(cf, "%d", &read_int);
				dlg->id=read_int;
			//ID текста
				fscanf(cf, "%d", &read_int);
				dlg->id_text=read_int;
			//действия при неответе
				fscanf(cf, "%s", &read_str);
				dlg->not_answer=0;
			//время на прочтение
				fscanf(cf, "%d", &read_int);
				if(read_int) dlg->time_break=read_int;
//ДИАЛОГИ----------------------------------------------------------------------------------------
				fscanf(cf, "%c", &ch);
				if(ch[0]=='@') continue;
				else if(ch[0]=='&') break;
				else if(ch[0]!='#')
				{
					read_proc=FALSE;
					break;
				}
//ОТВЕТЫ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				while(!feof(cf))
				{
					answ=new answer;
				//ID линка
					fscanf(cf, "%d", &read_int);
					answ->link=read_int;
				//ID текста
					fscanf(cf, "%d", &read_int);
					answ->id_text=read_int;
//ОТВЕТЫ----------------------------------------------------------------------------------------
					fscanf(cf, "%c", &ch);
//УСЛОВИЯ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					if(ch[0]=='u')
					{
						demand* new_demand;
						params_map::iterator it_d;
						while(!feof(cf))
						{
							fscanf(cf, "%c", &ch);
							if(ch[0]!='*') break;
							new_demand=new demand;
						//название требования
							fscanf(cf, "%s", &read_str);

							if(!stricmp(read_str,"stat"))
							{
								fscanf(cf, "%s", &read_str2); //название стата
								fscanf(cf, "%1s", &read_str3); //оператор сравнения
								fscanf(cf, "%d", &read_int); //значение

								new_demand->type=DEMAND_STAT;
								it_d=stats_map.find(read_str2);
								if(it_d==stats_map.end()) {SAFEDEL(new_demand); LogExecStr("Неизвестный стат %s\n", read_str2); return 0;}
								new_demand->param=(*it_d).second;
								new_demand->oper=read_str3[0];
								new_demand->count=read_int;
							}
							else if(!stricmp(read_str,"skill"))
							{
								fscanf(cf, "%s", &read_str2); //название скилла
								fscanf(cf, "%1s", &read_str3); //оператор сравнения
								fscanf(cf, "%d", &read_int); //значение

								new_demand->type=DEMAND_SKILL;
								it_d=skills_map.find(read_str2);
								if(it_d==skills_map.end()) {SAFEDEL(new_demand); LogExecStr("Неизвестный скилл %s\n", read_str2); return 0;}
								new_demand->param=(*it_d).second;
								new_demand->oper=read_str3[0];
								new_demand->count=read_int;
							}
							else if(!stricmp(read_str,"perk"))
							{
								fscanf(cf, "%s", &read_str2); //название перка
								fscanf(cf, "%1s", &read_str3); //оператор сравнения
								fscanf(cf, "%d", &read_int); //значение

								new_demand->type=DEMAND_PERK;
								it_d=perks_map.find(read_str2);
								if(it_d==perks_map.end()) {SAFEDEL(new_demand); LogExecStr("Неизвестный перк %s\n", read_str2); return 0;}
								new_demand->param=(*it_d).second;
								new_demand->oper=read_str3[0];
								new_demand->count=read_int;
							}
							else if(!stricmp(read_str,"var"))
							{
								fscanf(cf, "%s", &read_str2); //название переменной
								fscanf(cf, "%1s", &read_str3); //оператор сравнения
								fscanf(cf, "%d", &read_int); //значение

								new_demand->type=DEMAND_VAR;
								
						//		if(!sql.GetInt("npc_vars_templates","COUNT(*)","name",read_str2)) {SAFEDEL(new_result); LogExecStr("Неизвестная переменная %s\n", read_str2); return 0;}
								new_demand->var_name=read_str2;
								new_demand->oper=read_str3[0];
								new_demand->count=read_int;
							}
							else if(!stricmp(read_str,"quest"))
							{
								fscanf(cf, "%d", &read_int); //номер квеста
								fscanf(cf, "%1s", &read_str2); //оператор сравнения

								SAFEDEL(new_demand);
								continue;
							}
							else if(!stricmp(read_str,"item"))
							{
								fscanf(cf, "%d", &read_int); //номер итема
								fscanf(cf, "%1s", &read_str2); //оператор сравнения

								SAFEDEL(new_demand);
								continue;
							}
							else
							{
								SAFEDEL(new_demand);
								LogExecStr("Неизвестное условие %s\n", read_str);
								continue;
							}
							answ->demands.push_back(new_demand);
						}
					}
//УСЛОВИЯ----------------------------------------------------------------------------------------
//РЕЗУЛЬТАТ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					if(ch[0]=='r')
					{
						result* new_result;
						params_map::iterator it_r;
						while(!feof(cf))
						{
							fscanf(cf, "%c", &ch);
							if(ch[0]!='*') break;
							new_result=new result;
						//название результата
							fscanf(cf, "%s", &read_str);
							if(!stricmp(read_str,"stat"))
								{
								fscanf(cf, "%s", &read_str2); //название стата
								fscanf(cf, "%1s", &read_str3); //оператор присваивания
								fscanf(cf, "%d", &read_int); //значение

								new_result->type=RESULT_STAT;
								it_r=stats_map.find(read_str2);
								if(it_r==stats_map.end()) {SAFEDEL(new_result); LogExecStr("Неизвестный стат %s\n", read_str2); return 0;}
								new_result->param=(*it_r).second;
								new_result->oper=read_str3[0];
								new_result->count=read_int;
							}
							else if(!stricmp(read_str,"skill"))
							{
								fscanf(cf, "%s", &read_str2); //название скилла
								fscanf(cf, "%1s", &read_str3); //оператор присваивания
								fscanf(cf, "%d", &read_int); //значение

								new_result->type=RESULT_SKILL;
								it_r=skills_map.find(read_str2);
								if(it_r==skills_map.end()) {SAFEDEL(new_result); LogExecStr("Неизвестный скилл %s\n", read_str2); return 0;}
								new_result->param=(*it_r).second;
								new_result->oper=read_str3[0];
								new_result->count=read_int;
							}
							else if(!stricmp(read_str,"perk"))
							{
								fscanf(cf, "%s", &read_str2); //название перка
								fscanf(cf, "%1s", &read_str3); //оператор присваивания
								fscanf(cf, "%d", &read_int); //значение

								new_result->type=RESULT_PERK;
								it_r=perks_map.find(read_str2);
								if(it_r==perks_map.end()) {SAFEDEL(new_result); LogExecStr("Неизвестный перк %s\n", read_str2); return 0;}
								new_result->param=(*it_r).second;
								new_result->oper=read_str3[0];
								new_result->count=read_int;
							}
							else if(!stricmp(read_str,"var"))
							{
								fscanf(cf, "%s", &read_str2); //название переменной
								fscanf(cf, "%1s", &read_str3); //оператор присваивания
								fscanf(cf, "%d", &read_int); //значение

								new_result->type=RESULT_VAR;
								
						//		if(!sql.GetInt("npc_vars_templates","COUNT(*)","name",read_str2)) {SAFEDEL(new_result); LogExecStr("Неизвестная переменная %s\n", read_str2); return 0;}
								new_result->var_name=read_str2;
								new_result->oper=read_str3[0];
								new_result->count=read_int;
							}
							else if(!stricmp(read_str,"quest"))
							{
								fscanf(cf, "%d", &read_int); //номер квеста
								fscanf(cf, "%1s", &read_str2); //оператор присваивания

								SAFEDEL(new_result);
								continue;
							}
							else if(!stricmp(read_str,"item"))
							{
								fscanf(cf, "%d", &read_int); //номер итема
								fscanf(cf, "%2s", &read_str2); //оператор присваивания

								SAFEDEL(new_result);
								continue;
							}
							else if(!stricmp(read_str,"lock"))
							{
								fscanf(cf, "%d", &read_int); //время блокировки

								SAFEDEL(new_result);
								continue;
							}
							else
							{
								SAFEDEL(new_result);
								LogExecStr("Неизвестный результат %s\n", read_str);
								continue;
							}
							answ->results.push_back(new_result);
						}
					}
//РЕЗУЛЬТАТ----------------------------------------------------------------------------------------
				//проверки
					if(feof(cf))
					{
						read_proc=FALSE;
						break;
					}
					if(ch[0]=='#')
					{
						dlg->answers.push_back(answ);
						continue;
					}
					if((ch[0]=='@')||(ch[0]=='&'))
					{
						dlg->answers.push_back(answ);
						break;
					}
					read_proc=FALSE;
					break;
				}
				npc->i_npc->dialogs[dlg->id]=dlg;
				if(ch[0]=='&') break;
			}

			fclose(cf);
		} 
		else
		{
			LogExecStr("Файл не найден\n");
			return 0;
		}

		if(!read_proc)
		{
			LogExecStr("Ошибка при инициализации\n");
			return 0;
		}

		if(!sql.NewNPC(&npc->info))
		{
			LogExecStr("Ошибка регистрации НПЦ\n");
			return 0;
		}

		if(!mm.MoveToTile(&npc->info,npc->info.x,npc->info.y))
		{
			LogExecStr("Не удалось высадить НПЦ\n");
			return 0;
		}
		pc[npc->info.id]=npc;
		cr[npc->info.id]=npc;
		map_cr[npc->info.map][npc->info.id]=npc;

		loaded_npc++;
		LogExecStr("OK\n");
	}

	if(loaded_npc<max_npc)
	{
		LogExecStr("Ошибка. Загружены не все НПЦ, либо ошибка в настройках\n");
		return 0;
	}

	return 1;
/*
//Отладка
	LogExecStr("\n");
	LogExecStr("Отладочная инфа:\n");

	LogExecStr("Имя:%s, к0:%s, к2:%s, к3:%s, к4:%s\n", npc->info.name, npc->info.cases[0],
		npc->info.cases[1], npc->info.cases[2], npc->info.cases[3], npc->info.cases[4]);
	LogExecStr("id:%d, карта:%d, x:%d, y:%d, ориентация:%d, тип нпц:%d\n", npc->info.id,
		npc->info.map, npc->info.x, npc->info.y, npc->info.ori, npc->info.base_type);

	npc_dialog* dlg=NULL;
	answer* answ=NULL;

	LogExecStr("Всего диалогов: %d\n", npc->i_npc->dialogs.size());

	for(dialogs_map::iterator it=npc->i_npc->dialogs.begin(); it!=npc->i_npc->dialogs.end(); it++)
	{
		LogExecStr("\n");

		dlg=(*it).second;

		LogExecStr("Диалог:%d, текст №%d, время на прочтение:%d, нет ответа:%d\n", dlg->id, dlg->id_text,
			dlg->time_break, dlg->not_answer);
		
		for(answers_list::iterator it_a=dlg->answers.begin(); it_a!=dlg->answers.end(); it_a++)
		{
			answ=(*it_a);
			LogExecStr("Основной текст:%d, линк:%d, блокировка:%d, ", answ->id_text,
				answ->link_dialog, answ->lock_answered);

			if(answ->demand.size())
			{
				LogExecStr("требования |");

				for(demand_map::iterator it_dm=answ->demand.begin(); it_dm!=answ->demand.end(); it_dm++)
					LogExecStr(" %d > %d |", (*it_dm).first, (*it_dm).second);
			}
			else
				LogExecStr("требований нет");

			LogExecStr(", ");

			if(answ->overpatching.size())
			{
				LogExecStr("результат |");

				for(overpatching_map::iterator it_op=answ->overpatching.begin(); it_op!=answ->overpatching.end(); it_op++)
					LogExecStr(" %d > %d |", (*it_op).first, (*it_op).second);
			}
			else
				LogExecStr("результатов нет");

			LogExecStr("\n");
		}
	}

	LogExecStr("\n");

	return 1;
*/
}

void CServer::NPC_Remove(CLIENT* npc)
{
	
}

void CServer::NPC_Process(CLIENT* npc)
{
	if(npc->info.cond!=COND_LIFE) return;
	if(npc->info.cl_id.empty()) return;
	if(npc->info.break_time>GetTickCount()-npc->info.start_bt) return;

//	npc->info.break_time=0;

	BYTE move_params=BIN8(00000010);

	HexTYPE stepy;
	if(npc->info.y%2)
		stepy=1;
	else
		stepy=-1;

	BYTE move_res=mm.MoveToTile(&npc->info,npc->info.x,npc->info.y+stepy);

	switch(move_res)
	{
	case MR_STEP:
		SetVisibleCl(npc);
		//SetVisibleObj(acl);
		Send_Move(npc,move_params);
		break;
/*	case MR_TRANSIT:
		map_cr[old_map].erase(acl->info.id);
		DelClFromAllListId(acl->info.id,old_map);
		Send_LoadMap(acl);
		acl->state=STATE_LOGINOK;
		break;*/
	default:
	case MR_FALSE:
		SetCheat(npc,"Process_Move - попытка походить в занятую клетку");
		break;
	}

	npc->info.start_bt=GetTickCount();
	npc->info.break_time=2000;
}

void CServer::CreateParamsMaps()
{
	stats_map.insert(params_map::value_type("ST_STRENGHT",				ST_STRENGHT));
	stats_map.insert(params_map::value_type("ST_PERCEPTION",			ST_PERCEPTION));
	stats_map.insert(params_map::value_type("ST_ENDURANCE",				ST_ENDURANCE));
	stats_map.insert(params_map::value_type("ST_CHARISMA",				ST_CHARISMA));
	stats_map.insert(params_map::value_type("ST_INTELLECT",				ST_INTELLECT));
	stats_map.insert(params_map::value_type("ST_AGILITY",				ST_AGILITY));
	stats_map.insert(params_map::value_type("ST_LUCK",					ST_LUCK));
	stats_map.insert(params_map::value_type("ST_MAX_LIFE",				ST_MAX_LIFE));
	stats_map.insert(params_map::value_type("ST_MAX_COND",				ST_MAX_COND));
	stats_map.insert(params_map::value_type("ST_ARMOR_CLASS",			ST_ARMOR_CLASS));
	stats_map.insert(params_map::value_type("ST_MELEE_DAMAGE",			ST_MELEE_DAMAGE));
	stats_map.insert(params_map::value_type("ST_WEAPON_DAMAGE",			ST_WEAPON_DAMAGE));
	stats_map.insert(params_map::value_type("ST_CARRY_WEIGHT",			ST_CARRY_WEIGHT));
	stats_map.insert(params_map::value_type("ST_SEQUENCE",				ST_SEQUENCE));
	stats_map.insert(params_map::value_type("ST_HEALING_RATE",			ST_HEALING_RATE));
	stats_map.insert(params_map::value_type("ST_CRITICAL_CHANCE",		ST_CRITICAL_CHANCE));
	stats_map.insert(params_map::value_type("ST_MAX_CRITICAL",			ST_MAX_CRITICAL));
	stats_map.insert(params_map::value_type("ST_INGURE_ABSORB",			ST_INGURE_ABSORB));
	stats_map.insert(params_map::value_type("ST_LASER_ABSORB",			ST_LASER_ABSORB));
	stats_map.insert(params_map::value_type("ST_FIRE_ABSORB",			ST_FIRE_ABSORB));
	stats_map.insert(params_map::value_type("ST_PLASMA_ABSORB",			ST_PLASMA_ABSORB));
	stats_map.insert(params_map::value_type("ST_ELECTRO_ABSORB",		ST_ELECTRO_ABSORB));
	stats_map.insert(params_map::value_type("ST_EMP_ABSORB",			ST_EMP_ABSORB));
	stats_map.insert(params_map::value_type("ST_BLAST_ABSORB",			ST_BLAST_ABSORB));
	stats_map.insert(params_map::value_type("ST_INGURE_RESIST",			ST_INGURE_RESIST));
	stats_map.insert(params_map::value_type("ST_LASER_RESIST",			ST_LASER_RESIST));
	stats_map.insert(params_map::value_type("ST_FIRE_RESIST",			ST_FIRE_RESIST));
	stats_map.insert(params_map::value_type("ST_PLASMA_RESIST",			ST_PLASMA_RESIST));
	stats_map.insert(params_map::value_type("ST_ELECTRO_RESIST",		ST_ELECTRO_RESIST));
	stats_map.insert(params_map::value_type("ST_EMP_RESIST",			ST_EMP_RESIST));
	stats_map.insert(params_map::value_type("ST_BLAST_RESIST",			ST_BLAST_RESIST));
	stats_map.insert(params_map::value_type("ST_RADIATION_RESISTANCE",	ST_RADIATION_RESISTANCE));
	stats_map.insert(params_map::value_type("ST_POISON_RESISTANCE",		ST_POISON_RESISTANCE));
	stats_map.insert(params_map::value_type("ST_AGE",					ST_AGE));
	stats_map.insert(params_map::value_type("ST_GENDER",				ST_GENDER));
	stats_map.insert(params_map::value_type("ST_CURRENT_HP",			ST_CURRENT_HP));
	stats_map.insert(params_map::value_type("ST_POISONING_LEVEL",		ST_POISONING_LEVEL));
	stats_map.insert(params_map::value_type("ST_RADIATION_LEVEL",		ST_RADIATION_LEVEL));
	stats_map.insert(params_map::value_type("ST_CURRENT_STANDART",		ST_CURRENT_STANDART));

	skills_map.insert(params_map::value_type("SK_SMALL_GUNS",			SK_SMALL_GUNS));
	skills_map.insert(params_map::value_type("SK_BIG_GUNS",				SK_BIG_GUNS));
	skills_map.insert(params_map::value_type("SK_ENERGY_WEAPONS",		SK_ENERGY_WEAPONS));
	skills_map.insert(params_map::value_type("SK_UNARMED",				SK_UNARMED));
	skills_map.insert(params_map::value_type("SK_MELEE_WEAPONS",		SK_MELEE_WEAPONS));
	skills_map.insert(params_map::value_type("SK_THROWING",				SK_THROWING));
	skills_map.insert(params_map::value_type("SK_FIRST_AID",			SK_FIRST_AID));
	skills_map.insert(params_map::value_type("SK_DOCTOR",				SK_DOCTOR));
	skills_map.insert(params_map::value_type("SK_SNEAK",				SK_SNEAK));
	skills_map.insert(params_map::value_type("SK_LOCKPICK",				SK_LOCKPICK));
	skills_map.insert(params_map::value_type("SK_STEAL",				SK_STEAL));
	skills_map.insert(params_map::value_type("SK_TRAPS",				SK_TRAPS));
	skills_map.insert(params_map::value_type("SK_SCIENCE",				SK_SCIENCE));
	skills_map.insert(params_map::value_type("SK_REPAIR",				SK_REPAIR));
	skills_map.insert(params_map::value_type("SK_SPEECH",				SK_SPEECH));
	skills_map.insert(params_map::value_type("SK_BARTER",				SK_BARTER));
	skills_map.insert(params_map::value_type("SK_GAMBLING",				SK_GAMBLING));
	skills_map.insert(params_map::value_type("SK_OUTDOORSMAN",			SK_OUTDOORSMAN));

	perks_map.insert(params_map::value_type("PE_FAST_METABOLISM",		PE_FAST_METABOLISM));
	perks_map.insert(params_map::value_type("PE_BRUISER",				PE_BRUISER));
	perks_map.insert(params_map::value_type("PE_SMALL_FRAME",			PE_SMALL_FRAME));
	perks_map.insert(params_map::value_type("PE_ONE_HANDER",			PE_ONE_HANDER));
	perks_map.insert(params_map::value_type("PE_FINESSE",				PE_FINESSE));
	perks_map.insert(params_map::value_type("PE_KAMIKAZE",				PE_KAMIKAZE));
	perks_map.insert(params_map::value_type("PE_HEAVY_HANDED",			PE_HEAVY_HANDED));
	perks_map.insert(params_map::value_type("PE_FAST_SHOT",				PE_FAST_SHOT));
	perks_map.insert(params_map::value_type("PE_BLOODY_MESS",			PE_BLOODY_MESS));
	perks_map.insert(params_map::value_type("PE_JINXED",				PE_JINXED));
	perks_map.insert(params_map::value_type("PE_GOOD_NATURED",			PE_GOOD_NATURED));
	perks_map.insert(params_map::value_type("PE_CHEM_RELIANT",			PE_CHEM_RELIANT));
	perks_map.insert(params_map::value_type("PE_CHEM_RESISTANT",		PE_CHEM_RESISTANT));
	perks_map.insert(params_map::value_type("PE_NIGHT_PERSON",			PE_NIGHT_PERSON));
	perks_map.insert(params_map::value_type("PE_SKILLED",				PE_SKILLED));
	perks_map.insert(params_map::value_type("PE_GIFTED",				PE_GIFTED));
	perks_map.insert(params_map::value_type("PE_AWARENESS",				PE_AWARENESS));
	perks_map.insert(params_map::value_type("PE_A_MELEE_ATT",			PE_A_MELEE_ATT));
	perks_map.insert(params_map::value_type("PE_A_MELEE_DAM",			PE_A_MELEE_DAM));
	perks_map.insert(params_map::value_type("PE_A_MOVE",				PE_A_MOVE));
	perks_map.insert(params_map::value_type("PE_A_DAM",					PE_A_DAM));
	perks_map.insert(params_map::value_type("PE_A_SPEED",				PE_A_SPEED));
	perks_map.insert(params_map::value_type("PE_PASS_FRONT",			PE_PASS_FRONT));
	perks_map.insert(params_map::value_type("PE_RAPID_HEAL",			PE_RAPID_HEAL));
	perks_map.insert(params_map::value_type("PE_MORE_CRIT_DAM",			PE_MORE_CRIT_DAM));
	perks_map.insert(params_map::value_type("PE_NIGHT_SIGHT",			PE_NIGHT_SIGHT));
	perks_map.insert(params_map::value_type("PE_PRESENCE",				PE_PRESENCE));
	perks_map.insert(params_map::value_type("PE_RES_NUKLEAR",			PE_RES_NUKLEAR));
	perks_map.insert(params_map::value_type("PE_ENDURENCE",				PE_ENDURENCE));
	perks_map.insert(params_map::value_type("PE_STR_BACK",				PE_STR_BACK));
	perks_map.insert(params_map::value_type("PE_MARKSMAN",				PE_MARKSMAN));
	perks_map.insert(params_map::value_type("PE_STEALHING",				PE_STEALHING));
	perks_map.insert(params_map::value_type("PE_LIFEFULL",				PE_LIFEFULL));
	perks_map.insert(params_map::value_type("PE_MERCHANT",				PE_MERCHANT));
	perks_map.insert(params_map::value_type("PE_FORMED",				PE_FORMED));
	perks_map.insert(params_map::value_type("PE_HEALER",				PE_HEALER));
	perks_map.insert(params_map::value_type("PE_TR_DIGGER",				PE_TR_DIGGER));
	perks_map.insert(params_map::value_type("PE_BEST_HITS",				PE_BEST_HITS));
	perks_map.insert(params_map::value_type("PE_COMPASION",				PE_COMPASION));
	perks_map.insert(params_map::value_type("PE_KILLER",				PE_KILLER));
	perks_map.insert(params_map::value_type("PE_SNIPER",				PE_SNIPER));
	perks_map.insert(params_map::value_type("PE_SILENT_DEATH",			PE_SILENT_DEATH));
	perks_map.insert(params_map::value_type("PE_C_FIGHTER",				PE_C_FIGHTER));
	perks_map.insert(params_map::value_type("PE_MIND_BLOCK",			PE_MIND_BLOCK));
	perks_map.insert(params_map::value_type("PE_PROLONGATION_LIFE",		PE_PROLONGATION_LIFE));
	perks_map.insert(params_map::value_type("PE_RECOURCEFULNESS",		PE_RECOURCEFULNESS));
	perks_map.insert(params_map::value_type("PE_SNAKE_EATER",			PE_SNAKE_EATER));
	perks_map.insert(params_map::value_type("PE_REPEARER",				PE_REPEARER));
	perks_map.insert(params_map::value_type("PE_MEDIC",					PE_MEDIC));
	perks_map.insert(params_map::value_type("PE_SKILLED_THIEF",			PE_SKILLED_THIEF));
	perks_map.insert(params_map::value_type("PE_SPEAKER",				PE_SPEAKER));
	perks_map.insert(params_map::value_type("PE_GUTCHER",				PE_GUTCHER));
	perks_map.insert(params_map::value_type("PE_UNKNOWN_1",				PE_UNKNOWN_1));
	perks_map.insert(params_map::value_type("PE_PICK_POCKER",			PE_PICK_POCKER));
	perks_map.insert(params_map::value_type("PE_GHOST",					PE_GHOST));
	perks_map.insert(params_map::value_type("PE_CHAR_CULT",				PE_CHAR_CULT));
	perks_map.insert(params_map::value_type("PE_THIFER",				PE_THIFER));
	perks_map.insert(params_map::value_type("PE_DISCOVER",				PE_DISCOVER));
	perks_map.insert(params_map::value_type("PE_OVERROAD",				PE_OVERROAD));
	perks_map.insert(params_map::value_type("PE_ANIMAL_FRIENDSHIP",		PE_ANIMAL_FRIENDSHIP));
	perks_map.insert(params_map::value_type("PE_SCOUT",					PE_SCOUT));
	perks_map.insert(params_map::value_type("PE_MIST_CHAR",				PE_MIST_CHAR));
	perks_map.insert(params_map::value_type("PE_RANGER",				PE_RANGER));
	perks_map.insert(params_map::value_type("PE_PICK_POCKET_2",			PE_PICK_POCKET_2));
	perks_map.insert(params_map::value_type("PE_INTERLOCUTER",			PE_INTERLOCUTER));
	perks_map.insert(params_map::value_type("PE_NOVICE",				PE_NOVICE));
	perks_map.insert(params_map::value_type("PE_PRIME_SKILL",			PE_PRIME_SKILL));
	perks_map.insert(params_map::value_type("PE_MUTATION",				PE_MUTATION));
	perks_map.insert(params_map::value_type("PE_NARC_NUKACOLA",			PE_NARC_NUKACOLA));
	perks_map.insert(params_map::value_type("PE_NARC_BUFFOUT",			PE_NARC_BUFFOUT));
	perks_map.insert(params_map::value_type("PE_NARC_MENTAT",			PE_NARC_MENTAT));
	perks_map.insert(params_map::value_type("PE_NARC_PSYHO",			PE_NARC_PSYHO));
	perks_map.insert(params_map::value_type("PE_NARC_RADAWAY",			PE_NARC_RADAWAY));
	perks_map.insert(params_map::value_type("PE_DISTANT_WEAP",			PE_DISTANT_WEAP));
	perks_map.insert(params_map::value_type("PE_ACCURARY_WEAP",			PE_ACCURARY_WEAP));
	perks_map.insert(params_map::value_type("PE_PENETRATION_WEAP",		PE_PENETRATION_WEAP));
	perks_map.insert(params_map::value_type("PE_KILLER_WEAP",			PE_KILLER_WEAP));
	perks_map.insert(params_map::value_type("PE_ENERGY_ARMOR",			PE_ENERGY_ARMOR));
	perks_map.insert(params_map::value_type("PE_BATTLE_ARMOR",			PE_BATTLE_ARMOR));
	perks_map.insert(params_map::value_type("PE_WEAP_RANGE",			PE_WEAP_RANGE));
	perks_map.insert(params_map::value_type("PE_RAPID_RELOAD",			PE_RAPID_RELOAD));
	perks_map.insert(params_map::value_type("PE_NIGHT_SPYGLASS",		PE_NIGHT_SPYGLASS));
	perks_map.insert(params_map::value_type("PE_FLAMER",				PE_FLAMER));
	perks_map.insert(params_map::value_type("PE_APA_I",					PE_APA_I));
	perks_map.insert(params_map::value_type("PE_APA_II",				PE_APA_II));
	perks_map.insert(params_map::value_type("PE_FORCEAGE",				PE_FORCEAGE));
	perks_map.insert(params_map::value_type("PE_DEADLY_NARC",			PE_DEADLY_NARC));
	perks_map.insert(params_map::value_type("PE_CHARMOLEANCE",			PE_CHARMOLEANCE));
	perks_map.insert(params_map::value_type("PE_GEKK_SKINER",			PE_GEKK_SKINER));
	perks_map.insert(params_map::value_type("PE_SKIN_ARMOR",			PE_SKIN_ARMOR));
	perks_map.insert(params_map::value_type("PE_A_SKIN_ARMOR",			PE_A_SKIN_ARMOR));
	perks_map.insert(params_map::value_type("PE_SUPER_ARMOR",			PE_SUPER_ARMOR));
	perks_map.insert(params_map::value_type("PE_A_SUPER_ARMOR",			PE_A_SUPER_ARMOR));
	perks_map.insert(params_map::value_type("PE_VAULT_INOCUL",			PE_VAULT_INOCUL));
	perks_map.insert(params_map::value_type("PE_ADRENALIN_RUSH",		PE_ADRENALIN_RUSH));
	perks_map.insert(params_map::value_type("PE_CAREFULL",				PE_CAREFULL));
	perks_map.insert(params_map::value_type("PE_INTELEGENCE",			PE_INTELEGENCE));
	perks_map.insert(params_map::value_type("PE_PYROKASY",				PE_PYROKASY));
	perks_map.insert(params_map::value_type("PE_DUDE",					PE_DUDE));
	perks_map.insert(params_map::value_type("PE_A_STR",					PE_A_STR));
	perks_map.insert(params_map::value_type("PE_A_PER",					PE_A_PER));
	perks_map.insert(params_map::value_type("PE_A_END",					PE_A_END));
	perks_map.insert(params_map::value_type("PE_A_CHA",					PE_A_CHA));
	perks_map.insert(params_map::value_type("PE_A_INT",					PE_A_INT));
	perks_map.insert(params_map::value_type("PE_A_AGL",					PE_A_AGL));
	perks_map.insert(params_map::value_type("PE_A_LUC",					PE_A_LUC));
	perks_map.insert(params_map::value_type("PE_PURERER",				PE_PURERER));
	perks_map.insert(params_map::value_type("PE_IMAG",					PE_IMAG));
	perks_map.insert(params_map::value_type("PE_EVASION",				PE_EVASION));
	perks_map.insert(params_map::value_type("PE_DROSHKADRAT",			PE_DROSHKADRAT));
	perks_map.insert(params_map::value_type("PE_KARMA_GLOW",			PE_KARMA_GLOW));
	perks_map.insert(params_map::value_type("PE_SILENT_STEPS",			PE_SILENT_STEPS));
	perks_map.insert(params_map::value_type("PE_ANATOMY",				PE_ANATOMY));
	perks_map.insert(params_map::value_type("PE_CHAMER",				PE_CHAMER));
	perks_map.insert(params_map::value_type("PE_ORATOR",				PE_ORATOR));
	perks_map.insert(params_map::value_type("PE_PACKER",				PE_PACKER));
	perks_map.insert(params_map::value_type("PE_EDD_GAYAN_MANIAC",		PE_EDD_GAYAN_MANIAC));
	perks_map.insert(params_map::value_type("PE_FAST_REGENERATION",		PE_FAST_REGENERATION));
	perks_map.insert(params_map::value_type("PE_VENDOR",				PE_VENDOR));
	perks_map.insert(params_map::value_type("PE_STONE_WALL",			PE_STONE_WALL));
	perks_map.insert(params_map::value_type("PE_THIEF_AGAIN",			PE_THIEF_AGAIN));
	perks_map.insert(params_map::value_type("PE_WEAPON_SKILL",			PE_WEAPON_SKILL));
	perks_map.insert(params_map::value_type("PE_MAKE_VAULT",			PE_MAKE_VAULT));
	perks_map.insert(params_map::value_type("PE_ALC_BUFF_1",			PE_ALC_BUFF_1));
	perks_map.insert(params_map::value_type("PE_ALC_BUFF_2",			PE_ALC_BUFF_2));
/*	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
*/
	perks_map.insert(params_map::value_type("PE_HIDE_MODE",				PE_HIDE_MODE));

	object_map.insert(params_map::value_type("OBJ_TYPE_ARMOR",			OBJ_TYPE_ARMOR));
	object_map.insert(params_map::value_type("OBJ_TYPE_CONTAINER",		OBJ_TYPE_CONTAINER));
	object_map.insert(params_map::value_type("OBJ_TYPE_DRUG",			OBJ_TYPE_DRUG));
	object_map.insert(params_map::value_type("OBJ_TYPE_WEAPON",			OBJ_TYPE_WEAPON));
	object_map.insert(params_map::value_type("OBJ_TYPE_AMMO",			OBJ_TYPE_AMMO));
	object_map.insert(params_map::value_type("OBJ_TYPE_MISC",			OBJ_TYPE_MISC));
	object_map.insert(params_map::value_type("OBJ_TYPE_KEY",			OBJ_TYPE_KEY));
	object_map.insert(params_map::value_type("OBJ_NAME",				OBJ_NAME));
	object_map.insert(params_map::value_type("OBJ_INFO",				OBJ_INFO));
	object_map.insert(params_map::value_type("OBJ_TIME_SHOW",			OBJ_TIME_SHOW));
	object_map.insert(params_map::value_type("OBJ_TIME_HIDE",			OBJ_TIME_HIDE));
	object_map.insert(params_map::value_type("OBJ_DISTANCE_LIGHT",		OBJ_DISTANCE_LIGHT));
	object_map.insert(params_map::value_type("OBJ_INTENSITY_LIGHT",		OBJ_INTENSITY_LIGHT));
	object_map.insert(params_map::value_type("OBJ_PASSED",				OBJ_PASSED));
	object_map.insert(params_map::value_type("OBJ_RAKED",				OBJ_RAKED));
	object_map.insert(params_map::value_type("OBJ_TRANSPARENT",			OBJ_TRANSPARENT));
	object_map.insert(params_map::value_type("OBJ_CAN_USE",				OBJ_CAN_USE));
	object_map.insert(params_map::value_type("OBJ_CAN_PICK_UP",			OBJ_CAN_PICK_UP));
	object_map.insert(params_map::value_type("OBJ_CAN_USE_ON_SMTH",		OBJ_CAN_USE_ON_SMTH));
	object_map.insert(params_map::value_type("OBJ_HIDDEN",				OBJ_HIDDEN));
	object_map.insert(params_map::value_type("OBJ_WEIGHT",				OBJ_WEIGHT));
	object_map.insert(params_map::value_type("OBJ_SIZE",				OBJ_SIZE));
	object_map.insert(params_map::value_type("OBJ_TWOHANDS",			OBJ_TWOHANDS));
	object_map.insert(params_map::value_type("OBJ_PIC_MAP",				OBJ_PIC_MAP));
	object_map.insert(params_map::value_type("OBJ_PIC_INV",				OBJ_PIC_INV));
	object_map.insert(params_map::value_type("OBJ_SOUND",				OBJ_SOUND));
	object_map.insert(params_map::value_type("OBJ_LIVETIME",			OBJ_LIVETIME));
	object_map.insert(params_map::value_type("OBJ_COST",				OBJ_COST));
	object_map.insert(params_map::value_type("OBJ_MATERIAL",			OBJ_MATERIAL));
	object_map.insert(params_map::value_type("OBJ_ARM_ANIM0_MALE",		OBJ_ARM_ANIM0_MALE));
	object_map.insert(params_map::value_type("OBJ_ARM_ANIM0_MALE2",		OBJ_ARM_ANIM0_MALE2));
	object_map.insert(params_map::value_type("OBJ_ARM_ANIM0_FEMALE",	OBJ_ARM_ANIM0_FEMALE));
	object_map.insert(params_map::value_type("OBJ_ARM_ANIM0_FEMALE2",	OBJ_ARM_ANIM0_FEMALE2));
	object_map.insert(params_map::value_type("OBJ_ARM_AC",				OBJ_ARM_AC));
	object_map.insert(params_map::value_type("OBJ_ARM_PERK",			OBJ_ARM_PERK));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_NORMAL",		OBJ_ARM_DR_NORMAL));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_LASER",		OBJ_ARM_DR_LASER));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_FIRE",			OBJ_ARM_DR_FIRE));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_PLASMA",		OBJ_ARM_DR_PLASMA));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_ELECTR",		OBJ_ARM_DR_ELECTR));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_EMP",			OBJ_ARM_DR_EMP));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_EXPLODE",		OBJ_ARM_DR_EXPLODE));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_NORMAL",		OBJ_ARM_DT_NORMAL));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_LASER",		OBJ_ARM_DT_LASER));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_FIRE",			OBJ_ARM_DT_FIRE));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_PLASMA",		OBJ_ARM_DT_PLASMA));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_ELECTR",		OBJ_ARM_DT_ELECTR));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_EMP",			OBJ_ARM_DT_EMP));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_EXPLODE",		OBJ_ARM_DT_EXPLODE));
	object_map.insert(params_map::value_type("OBJ_CONT_SIZE",			OBJ_CONT_SIZE));
	object_map.insert(params_map::value_type("OBJ_CONT_FLAG",			OBJ_CONT_FLAG));
	object_map.insert(params_map::value_type("OBJ_DRUG_STAT0",			OBJ_DRUG_STAT0));
	object_map.insert(params_map::value_type("OBJ_DRUG_STAT1",			OBJ_DRUG_STAT1));
	object_map.insert(params_map::value_type("OBJ_DRUG_STAT2",			OBJ_DRUG_STAT2));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT0_S0",		OBJ_DRUG_AMOUNT0_S0));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT0_S1",		OBJ_DRUG_AMOUNT0_S1));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT0_S2",		OBJ_DRUG_AMOUNT0_S2));
	object_map.insert(params_map::value_type("OBJ_DRUG_DURATION1",		OBJ_DRUG_DURATION1));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT1_S0",		OBJ_DRUG_AMOUNT1_S0));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT1_S1",		OBJ_DRUG_AMOUNT1_S1));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT1_S2",		OBJ_DRUG_AMOUNT1_S2));
	object_map.insert(params_map::value_type("OBJ_DRUG_DURATION2",		OBJ_DRUG_DURATION2));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT2_S0",		OBJ_DRUG_AMOUNT2_S0));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT2_S1",		OBJ_DRUG_AMOUNT2_S1));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT2_S2",		OBJ_DRUG_AMOUNT2_S2));
	object_map.insert(params_map::value_type("OBJ_DRUG_ADDICTION",		OBJ_DRUG_ADDICTION));
	object_map.insert(params_map::value_type("OBJ_DRUG_W_EFFECT",		OBJ_DRUG_W_EFFECT));
	object_map.insert(params_map::value_type("OBJ_DRUG_W_ONSET",		OBJ_DRUG_W_ONSET));
	object_map.insert(params_map::value_type("OBJ_WEAP_ANIM1",			OBJ_WEAP_ANIM1));
	object_map.insert(params_map::value_type("OBJ_WEAP_TIME_ACTIV",		OBJ_WEAP_TIME_ACTIV));
	object_map.insert(params_map::value_type("OBJ_WEAP_TIME_UNACTIV",	OBJ_WEAP_TIME_UNACTIV));
	object_map.insert(params_map::value_type("OBJ_WEAP_VOL_HOLDER",		OBJ_WEAP_VOL_HOLDER));
	object_map.insert(params_map::value_type("OBJ_WEAP_CALIBER",		OBJ_WEAP_CALIBER));
	object_map.insert(params_map::value_type("OBJ_WEAP_VOL_HOLDER_E",	OBJ_WEAP_VOL_HOLDER_E));
	object_map.insert(params_map::value_type("OBJ_WEAP_CALIBER_E",		OBJ_WEAP_CALIBER_E));
	object_map.insert(params_map::value_type("OBJ_WEAP_CR_FAILTURE",	OBJ_WEAP_CR_FAILTURE));
	object_map.insert(params_map::value_type("OBJ_WEAP_TYPE_ATTACK",	OBJ_WEAP_TYPE_ATTACK));
	object_map.insert(params_map::value_type("OBJ_WEAP_COUNT_ATTACK",	OBJ_WEAP_COUNT_ATTACK));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_SKILL",		OBJ_WEAP_PA_SKILL));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_HOLDER",		OBJ_WEAP_PA_HOLDER));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_PIC",			OBJ_WEAP_PA_PIC));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_DMG_MIN",		OBJ_WEAP_PA_DMG_MIN));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_DMG_MAX",		OBJ_WEAP_PA_DMG_MAX));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_MAX_DIST",	OBJ_WEAP_PA_MAX_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_EFF_DIST",	OBJ_WEAP_PA_EFF_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_ANIM2",		OBJ_WEAP_PA_ANIM2));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_TIME",		OBJ_WEAP_PA_TIME));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_AIM",			OBJ_WEAP_PA_AIM));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_ROUND",		OBJ_WEAP_PA_ROUND));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_REMOVE",		OBJ_WEAP_PA_REMOVE));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_SKILL",		OBJ_WEAP_SA_SKILL));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_HOLDER",		OBJ_WEAP_SA_HOLDER));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_PIC",			OBJ_WEAP_SA_PIC));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_DMG_MIN",		OBJ_WEAP_SA_DMG_MIN));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_DMG_MAX",		OBJ_WEAP_SA_DMG_MAX));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_MAX_DIST",	OBJ_WEAP_SA_MAX_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_EFF_DIST",	OBJ_WEAP_SA_EFF_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_ANIM2",		OBJ_WEAP_SA_ANIM2));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_TIME",		OBJ_WEAP_SA_TIME));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_AIM",			OBJ_WEAP_SA_AIM));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_ROUND",		OBJ_WEAP_SA_ROUND));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_REMOVE",		OBJ_WEAP_SA_REMOVE));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_SKILL",		OBJ_WEAP_TA_SKILL));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_HOLDER",		OBJ_WEAP_TA_HOLDER));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_PIC",			OBJ_WEAP_TA_PIC));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_DMG_MIN",		OBJ_WEAP_TA_DMG_MIN));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_DMG_MAX",		OBJ_WEAP_TA_DMG_MAX));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_MAX_DIST",	OBJ_WEAP_TA_MAX_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_EFF_DIST",	OBJ_WEAP_TA_EFF_DIST));	
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_ANIM2",		OBJ_WEAP_TA_ANIM2));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_TIME",		OBJ_WEAP_TA_TIME));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_AIM",			OBJ_WEAP_TA_AIM));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_ROUND",		OBJ_WEAP_TA_ROUND));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_REMOVE",		OBJ_WEAP_TA_REMOVE));
	object_map.insert(params_map::value_type("OBJ_AMMO_CALIBER",		OBJ_AMMO_CALIBER));
	object_map.insert(params_map::value_type("OBJ_AMMO_TYPE_DAMAGE",	OBJ_AMMO_TYPE_DAMAGE));
	object_map.insert(params_map::value_type("OBJ_AMMO_QUANTITY",		OBJ_AMMO_QUANTITY));
	object_map.insert(params_map::value_type("OBJ_AMMO_AC",				OBJ_AMMO_AC));
	object_map.insert(params_map::value_type("OBJ_AMMO_DR",				OBJ_AMMO_DR));
	object_map.insert(params_map::value_type("OBJ_AMMO_DM",				OBJ_AMMO_DM));
	object_map.insert(params_map::value_type("OBJ_AMMO_DD",				OBJ_AMMO_DD));
}

void CServer::SetCheat(CLIENT* acl, char* cheat_message)
{
	sql.AddCheat(acl->info.id,cheat_message);
}
//!Cvet ----------------------------------------------