/********************************************************************
	created:	end of 2006; updated: begin of 2007

	author:		Anton Tsvetinsky
	
	purpose:	
*********************************************************************/

#include "stdafx.h"
#include "SQL.h"

SQL::SQL()
{
	Active=false;
}

SQL::~SQL()
{

}

int SQL::Init_mySQL()
{
	if(Active==true) return 1;

	LogExecStr("mySQL init begin\n");

	char load_char[256];

	GetPrivateProfileString("SQL","true_char","@",load_char,256,".\\foserv.cfg");

	if(load_char[0]=='@')
	{
		LogExecStr("mySQL wrong_char failed. see foserv.cfg!\nmySQL init FALSE\n");
		return 0;
	}

	LogExecStr("Разрешенные символы:|%s|. Все остальные будут заменяться на пробел.\n",load_char);

	for(int i=0; load_char[i]; i++) true_char.insert(true_char_set::value_type(load_char[i]));

	mySQL=mysql_init(NULL);
	if(!mySQL)
	{
		LogExecStr("ошибка инициализации\n");
		return 0;
	}

	if(!mysql_real_connect(mySQL,"localhost","root","","fonline",3306,0,0))
	{
		LogExecStr("ошибка подключения (%s)\n",mysql_error(mySQL));
		return 0;
	}

	Query("DROP TABLE IF EXISTS `users`");
	Query("DROP TABLE IF EXISTS `npc`");
	Query("DROP TABLE IF EXISTS `objects`");

	Query("DROP TABLE IF EXISTS `npc_vars_templates`");
	Query("DROP TABLE IF EXISTS `npc_vars`");

	Query("DROP TABLE IF EXISTS `users_cheat`");

	Query("CREATE TABLE IF NOT EXISTS `users` ("
	"`id`			int(11)			NOT NULL	auto_increment,"
	"`login`		varchar(20)		NOT NULL	default '0',"
	"`pass`			varchar(20)		NOT NULL	default '0',"
	"`reg`			tinyint(4)		NOT NULL	default '0',"
	"`base_type`	tinyint(4)		NOT NULL	default '0',"
	"`map`			smallint(6)		NOT NULL	default '0',"
	"`x`			tinyint(4)		NOT NULL	default '0',"
	"`y`			tinyint(4)		NOT NULL	default '0',"
	"`ori`			tinyint(4)		NOT NULL	default '0',"
	"`name`			varchar(20)		NOT NULL	default '0',"
	"`cases0`		varchar(20)		NOT NULL	default '0',"
	"`cases1`		varchar(20)		NOT NULL	default '0',"
	"`cases2`		varchar(20)		NOT NULL	default '0',"
	"`cases3`		varchar(20)		NOT NULL	default '0',"
	"`cases4`		varchar(20)		NOT NULL	default '0',"
	"`stats`		varchar(%d)		NOT NULL	default '0',"
	"`skills`		varchar(%d)		NOT NULL	default '0',"
	"`perks`		varchar(%d)		NOT NULL	default '0',"
	"PRIMARY KEY  (`id`),"
	"UNIQUE KEY `login` (`login`)"
	") TYPE=MyISAM;",
	(ALL_STATS*4+1), (ALL_SKILLS*3+1), (ALL_PERKS+1));

	Query("INSERT INTO users (id) VALUES ('%d');",MAX_NPC);

	Query("CREATE TABLE IF NOT EXISTS `npc` ("
	"`id`			int(11)			NOT NULL	default '0',"
	"`base_type`	tinyint(4)		NOT NULL	default '0',"
	"`map`			smallint(6)		NOT NULL	default '0',"
	"`x`			tinyint(4)		NOT NULL	default '0',"
	"`y`			tinyint(4)		NOT NULL	default '0',"
	"`ori`			tinyint(4)		NOT NULL	default '0',"
	"`name`			varchar(20)		NOT NULL	default '0',"
	"`cases0`		varchar(20)		NOT NULL	default '0',"
	"`cases1`		varchar(20)		NOT NULL	default '0',"
	"`cases2`		varchar(20)		NOT NULL	default '0',"
	"`cases3`		varchar(20)		NOT NULL	default '0',"
	"`cases4`		varchar(20)		NOT NULL	default '0',"
	"`stats`		varchar(%d)		NOT NULL	default '0',"
	"`skills`		varchar(%d)		NOT NULL	default '0',"
	"`perks`		varchar(%d)		NOT NULL	default '0',"
	"PRIMARY KEY  (`id`)"
	") TYPE=MyISAM;",
	(ALL_STATS*4+1), (ALL_SKILLS*3+1), (ALL_PERKS+1));

	Query("CREATE TABLE IF NOT EXISTS `objects` ("
	"`id`			int(11)		NOT NULL	auto_increment,"
	"`num_st`		smallint(6) NOT NULL	default '0',"
	"`player`		int(11)		NOT NULL	default '0',"
	"`active`		tinyint(4)	NOT NULL	default '0',"
	"`map`			smallint(6)	NOT NULL	default '0',"
	"`x`			tinyint(4)	NOT NULL	default '0',"
	"`y`			tinyint(4)	NOT NULL	default '0',"
	"`tick`			int(11)		NOT NULL	default '0',"
	"`last_tick`	int(11)		NOT NULL	default '0',"
	"`broken`		tinyint(4)	NOT NULL	default '0',"
	"`holder`		smallint(6) NOT NULL	default '0',"
	"`id_bull`		smallint(6) NOT NULL	default '0',"
	"`holder_ext`	smallint(6) NOT NULL	default '0',"
	"`id_bull_ext`	smallint(6) NOT NULL	default '0',"
	"PRIMARY KEY  (`id`)"
	") TYPE=MyISAM;");

	Query("CREATE TABLE IF NOT EXISTS `npc_vars_templates` ("
	"`id`			int(11)		NOT NULL	auto_increment,"
	"`npc_id`		int(11)		NOT NULL	default '0',"
	"`name`			varchar(20)	NOT NULL	default 'none',"
	"`count`		int(11)		NOT NULL	default '0',"
	"`min`			int(11)		NOT NULL	default '0',"
	"`max`			int(11)		NOT NULL	default '0',"
	"PRIMARY KEY  (`id`)"
	") TYPE=MyISAM;");

	Query("CREATE TABLE IF NOT EXISTS `npc_vars` ("
	"`id`			int(11)		NOT NULL	auto_increment,"
	"`uniq_name`	varchar(40)	NOT NULL    default 'error',"
	"`npc_id`		int(11)		NOT NULL	default '0',"
	"`name`			varchar(20)	NOT NULL	default 'none',"
	"`player_id`	int(11)		NOT NULL	default '0',"
	"`count`		int(11)		NOT NULL	default '0',"
	"`min`			int(11)		NOT NULL	default '0',"
	"`max`			int(11)		NOT NULL	default '0',"
	"PRIMARY KEY  (`id`),"
	"UNIQUE KEY `uniq_name` (`uniq_name`)"
	") TYPE=MyISAM;");

	Query("CREATE TRIGGER `var_change` BEFORE UPDATE ON `npc_vars` "
	"FOR EACH ROW BEGIN "
	"IF NEW.count > NEW.max THEN SET NEW.count = NEW.max; END IF;"
	"IF NEW.count < NEW.min THEN SET NEW.count = NEW.min; END IF;"
	"END;");

//	Query("CREATE TRIGGER `var_change` BEFORE UPDATE ON `npc_vars` "
//	"forEACH ROW BEGIN "
//	"ifNEW.count > NEW.max THEN SET NEW.count = NEW.max; END IF;"
//	"ifNEW.count < NEW.min THEN SET NEW.count = NEW.min; END IF;"
//	"END;");

	Query("CREATE TABLE IF NOT EXISTS `users_cheat` ("
	"`id`			int(11)		NOT NULL	auto_increment,"
	"`user_id`		int(11)		NOT NULL	default '0',"
	"`text_cheat`	varchar(128)NOT NULL    default 'err',"
	"PRIMARY KEY  (`id`)"
	") TYPE=MyISAM;");

	Active=true;

	LogExecStr("mySQL init OK\n");

	return 1;
}

void SQL::Finish_mySQL()
{
	mysql_close(mySQL);
	mySQL=NULL;

	Active=false;
}

int SQL::Query(char* query, ...)
{
	char str[2048];

	va_list list;
	va_start(list, query);
	wvsprintf(str, query, list);
	va_end(list);

	if(mysql_query(mySQL, str))
	{
		LogExecStr("mySQL error: %s\n", mysql_error(mySQL));
		return 0;
	}
    return 1;
}

int SQL::Check(char* str)
{
	int i=0;
	bool cheat_fix=false;

	while(str[i])
	{
		if(!true_char.count(str[i]))
		{
			str[i]=0x20;
			cheat_fix=true;
		}
		i++;
	}

	if(cheat_fix==true) return 1;

	return 0;
}

int SQL::GetInt(char* table, char* find_row, char* row, char* row_vol)
{
	char stradd[120];
	sprintf(stradd,"SELECT %s FROM %s WHERE %s='%s'", find_row, table, row, row_vol);

	if(mysql_query(mySQL,stradd)) return 0;

	if(!(mySQL_res=mysql_store_result(mySQL))) return 0;

	if(!mysql_num_rows(mySQL_res)) return 0;

	mySQL_row=mysql_fetch_row(mySQL_res);

	return atoi(mySQL_row[0]);
}

int SQL::GetInt(char* table, char* find_row, char* row, int row_vol)
{
	char stradd[120];
	sprintf(stradd,"SELECT %s FROM %s WHERE %s='%d'", find_row, table, row, row_vol);

	if(mysql_query(mySQL,stradd)) return 0;

	if(!(mySQL_res=mysql_store_result(mySQL))) return 0;

	if(!mysql_num_rows(mySQL_res)) return 0;

	mySQL_row=mysql_fetch_row(mySQL_res);

	return atoi(mySQL_row[0]);
}

char* SQL::GetChar(char* table, char* find_row, char* row, char* row_vol)
{
	char stradd[120];
	sprintf(stradd,"SELECT %s FROM %s WHERE %s='%s'", find_row, table, row, row_vol);

	if(mysql_query(mySQL,stradd)) return "E";

	if(!(mySQL_res=mysql_store_result(mySQL))) return "E";

	if(!mysql_num_rows(mySQL_res)) return "E";

	mySQL_row=mysql_fetch_row(mySQL_res);

	return mySQL_row[0];
}

char* SQL::GetChar(char* table, char* find_row, char* row, int row_vol)
{
	char stradd[120];
	sprintf(stradd,"SELECT %s FROM %s WHERE %s='%d'", find_row, table, row, row_vol);

	if(mysql_query(mySQL,stradd)) return "E";

	if(!(mySQL_res=mysql_store_result(mySQL))) return "E";

	if(!mysql_num_rows(mySQL_res)) return "E";

	mySQL_row=mysql_fetch_row(mySQL_res);

	return mySQL_row[0];
}

int SQL::CountRows(char* table, char* column, char* count_vol)
{
	char stradd[120];
	sprintf(stradd,"SELECT * FROM %s WHERE %s='%s'", table, column, count_vol);

	if(mysql_query(mySQL,stradd)) return 0;

	if(!(mySQL_res=mysql_store_result(mySQL))) return 0;

	return mysql_num_rows(mySQL_res);
}

int SQL::CountRows(char* table, char* column, int count_vol)
{
	char stradd[120];
	sprintf(stradd,"SELECT * FROM %s WHERE %s='%d'", table, column, count_vol);

	if(mysql_query(mySQL,stradd)) return 0;

	if(!(mySQL_res=mysql_store_result(mySQL))) return 0;

	return mysql_num_rows(mySQL_res);
}

int SQL::NewPlayer(crit_info* info)
{
	Query("INSERT INTO users (login) VALUES ('%s')", info->login);

	if(!(info->id=mysql_insert_id(mySQL))) return 0;

	if(!SaveDataPlayer(info)) return 0;

	return 1;
}

int SQL::SaveDataPlayer(crit_info* info)
{
	if(!CodeParams(stats,skills,perks,info))
	{
		LogExecStr("!!!WORNING!!! SQL. Ошибка кодировки данных криттера\n");
		return 0;
	}

	Query("UPDATE users SET login='%s',pass='%s',base_type='%d',map='%d',x='%d',y='%d',ori='%d',"
	"name='%s',cases0='%s',cases1='%s',cases2='%s',cases3='%s',cases4='%s',"
	"stats='%s',skills='%s',perks='%s' WHERE id='%d'",
	info->login,info->pass,info->base_type,info->map,info->x,info->y,info->ori,info->name,
	info->cases[0],info->cases[1],info->cases[2],info->cases[3],info->cases[4],
	stats,skills,perks,info->id);

	return 1;
}

int SQL::LoadDataPlayer(crit_info* info)
{
	char str[64];

	sprintf(str, "SELECT * FROM users WHERE login='%s'", info->login);
	mysql_query(mySQL, str);

	if(!(mySQL_res=mysql_store_result(mySQL))) return 0;
	if(!mysql_num_rows(mySQL_res)) return 0;
	mySQL_row=mysql_fetch_row(mySQL_res);

	info->id=atoi(mySQL_row[0]);
	info->base_type=atoi(mySQL_row[4]);
	info->map=atoi(mySQL_row[5]);
	info->x=atoi(mySQL_row[6]);
	info->y=atoi(mySQL_row[7]);
	info->ori=atoi(mySQL_row[8]);
	strcpy(info->name,mySQL_row[9]);
	strcpy(info->cases[0],mySQL_row[10]);
	strcpy(info->cases[1],mySQL_row[11]);
	strcpy(info->cases[2],mySQL_row[12]);
	strcpy(info->cases[3],mySQL_row[13]);
	strcpy(info->cases[4],mySQL_row[14]);
	strcpy(stats,mySQL_row[15]);
	strcpy(skills,mySQL_row[16]);
	strcpy(perks,mySQL_row[17]);

	if(!info->name) return 0;
	if(!info->cases[0]) return 0;
	if(!info->cases[1]) return 0;
	if(!info->cases[2]) return 0;
	if(!info->cases[3]) return 0;
	if(!info->cases[4]) return 0;

	if(!DecodeParams(stats,skills,perks,info)) return 0;

	return 1;
}

void SQL::DeleteDataPlayer(crit_info* info)
{
	Query("DELETE FROM users WHERE id='%d'",info->id);
}

int SQL::NewNPC(crit_info* info)
{
	Query("INSERT INTO npc (id) VALUES (%d)", info->id);

//	if(!(info->id=mysql_insert_id(mySQL))) return 0;

	if(!SaveDataNPC(info)) return 0;

	return 1;
}

int SQL::SaveDataNPC(crit_info* info)
{
	if(!CodeParams(stats,skills,perks,info))
	{
		LogExecStr("!!!WORNING!!! SQL. Ошибка кодировки данных криттера\n");
		return 0;
	}

	Query("UPDATE npc SET base_type='%d',map='%d',x='%d',y='%d',ori='%d',"
	"name='%s',cases0='%s',cases1='%s',cases2='%s',cases3='%s',cases4='%s',"
	"stats='%s',skills='%s',perks='%s' WHERE id='%d'",
	info->base_type,info->map,info->x,info->y,info->ori,info->name,
	info->cases[0],info->cases[1],info->cases[2],info->cases[3],info->cases[4],
	stats,skills,perks,info->id);

	return 1;
}

int SQL::LoadDataNPC(crit_info* info)
{
	char str[64];

	sprintf(str, "SELECT * FROM npc WHERE id='%d'", info->id);
	mysql_query(mySQL, str);

	if(!(mySQL_res=mysql_store_result(mySQL))) return 0;
	if(!mysql_num_rows(mySQL_res)) return 0;
	mySQL_row=mysql_fetch_row(mySQL_res);

	info->id=atoi(mySQL_row[0]);
	info->base_type=atoi(mySQL_row[1]);
	info->map=atoi(mySQL_row[2]);
	info->x=atoi(mySQL_row[3]);
	info->y=atoi(mySQL_row[4]);
	info->ori=atoi(mySQL_row[5]);
	strcpy(info->name,mySQL_row[6]);
	strcpy(info->cases[0],mySQL_row[7]);
	strcpy(info->cases[1],mySQL_row[8]);
	strcpy(info->cases[2],mySQL_row[9]);
	strcpy(info->cases[3],mySQL_row[10]);
	strcpy(info->cases[4],mySQL_row[11]);
	strcpy(stats,mySQL_row[12]);
	strcpy(skills,mySQL_row[13]);
	strcpy(perks,mySQL_row[14]);

	if(!info->name) return 0;
	if(!info->cases[0]) return 0;
	if(!info->cases[1]) return 0;
	if(!info->cases[2]) return 0;
	if(!info->cases[3]) return 0;
	if(!info->cases[4]) return 0;

	if(!DecodeParams(stats,skills,perks,info)) return 0;

	return 1;
}

void SQL::DeleteDataNPC(crit_info* info)
{
	Query("DELETE FROM npc WHERE id='%d'",info->id);
}

int SQL::CheckVarNPC(CrID npc_id, string var_name, CrID player_id, char oper, int count)
{
	char uniq_name[80];

	if(var_name[0]=='l')
		sprintf(uniq_name,"%d_%s_%d",npc_id,var_name.c_str(),player_id);
	else if(var_name[0]=='g')
		sprintf(uniq_name,"%d_%s",npc_id,var_name.c_str());
	else
	{
		LogExecStr("Неизвестная переменная %s у %d при попытке проверки\n",var_name.c_str(),npc_id);
		return 0;
	}

	char get_count_ch[10];
	strcpy(get_count_ch,GetChar("npc_vars","count","uniq_name",uniq_name));

	int get_count=0;

	if(get_count_ch[0]=='E')
	{
		char str_f[120];

		sprintf(str_f, "SELECT * FROM npc_vars_templates WHERE npc_id='%d' AND name='%s'", npc_id, var_name.c_str());
		mysql_query(mySQL, str_f);

		if(!(mySQL_res=mysql_store_result(mySQL))) { LogExecStr("Проверка - Ошибка в записи переменной\n"); return 0; }
		if(!mysql_num_rows(mySQL_res)) { LogExecStr("Проверка - Шаблон переменной не найден\n"); return 0; }
		mySQL_row=mysql_fetch_row(mySQL_res);

		int var_count=atoi(mySQL_row[3]);
		int var_min=atoi(mySQL_row[4]);
		int var_max=atoi(mySQL_row[5]);

		Query("INSERT INTO npc_vars (uniq_name,npc_id,name,player_id,count,min,max) "
		"VALUES ('%s','%d','%s','%d','%d','%d','%d');",uniq_name,npc_id,var_name.c_str(),player_id,var_count,var_min,var_max);

		get_count=var_count;
	}
	else
		get_count=atoi(get_count_ch);

	if(oper=='>') { if(get_count>count) return 1; }
	else if(oper=='<') { if(get_count<count) return 1; }
	else if(oper=='=') { if(get_count==count) return 1; }

	return 0;
}

void SQL::ChangeVarNPC(CrID npc_id, string var_name, CrID player_id, char oper, int count)
{
	char uniq_name[80];

	if(var_name[0]=='l')
		sprintf(uniq_name,"%d_%s_%d",npc_id,var_name.c_str(),player_id);
	else if(var_name[0]=='g')
		sprintf(uniq_name,"%d_%s",npc_id,var_name.c_str());
	else
	{
		LogExecStr("Неизвестная переменная %s у %d при попытке изменения\n",var_name.c_str(),npc_id);
		return;
	}

	if(CountRows("npc_vars","uniq_name",uniq_name))
	{
		if(oper=='=')
			Query("UPDATE npc_vars SET count='%d' WHERE uniq_name='%s';",count,uniq_name);
		else
			Query("UPDATE npc_vars SET count=count%c'%d' WHERE uniq_name='%s';",oper,count,uniq_name);
	}
	else
	{
		char str_f[120];

		sprintf(str_f, "SELECT * FROM npc_vars_templates WHERE npc_id='%d' AND name='%s'", npc_id, var_name.c_str());
		mysql_query(mySQL, str_f);

		if(!(mySQL_res=mysql_store_result(mySQL))) { LogExecStr("Изменение - Ошибка в записи переменной\n"); return; }
		if(!mysql_num_rows(mySQL_res)) { LogExecStr("Изменение - Шаблон переменной не найден\n"); return; }
		mySQL_row=mysql_fetch_row(mySQL_res);

		int var_count=atoi(mySQL_row[3]);
		int var_min=atoi(mySQL_row[4]);
		int var_max=atoi(mySQL_row[5]);

		if(oper=='+') var_count+=count;
		else if(oper=='-') var_count-=count;
		else if(oper=='=') var_count=count;
		else if(oper=='*') var_count*=count;
		else if(oper=='/') var_count/=count;

		Query("INSERT INTO npc_vars (uniq_name,npc_id,name,player_id,count,min,max) "
		"VALUES ('%s','%d','%s','%d','%d','%d','%d');",uniq_name,npc_id,var_name.c_str(),player_id,var_count,var_min,var_max);
	}
}

int SQL::CodeParams(char* stats, char* skills, char* perks, crit_info* info)
{
	int cc;
	char tmpstr[6];

	stats[0]=0;
	for(cc=0; cc<ALL_STATS; cc++)
	{
		if(info->st[cc]>9999) info->st[cc]=0;

		sprintf(tmpstr,"%04d",info->st[cc]);

		strcat(stats,tmpstr);
	}
//	LogExecStr("stats:%s\n",stats);

	skills[0]=0;
	for(cc=0; cc<ALL_SKILLS; cc++)
	{
		if(info->sk[cc]>300) info->sk[cc]=299;

		sprintf(tmpstr,"%03d",info->sk[cc]);

		strcat(skills,tmpstr);
	}
//	LogExecStr("skills:%s\n",skills);

	perks[0]=0;
	for(cc=0; cc<ALL_PERKS; cc++)
	{
		if(info->pe[cc]>9) info->pe[cc]=0;

		sprintf(tmpstr,"%d",info->pe[cc]);

		strcat(perks,tmpstr);
	}
//	LogExecStr("perks:%s\n",perks);

	return 1;
}

int SQL::DecodeParams(char* stats, char* skills, char* perks, crit_info* info)
{
	int cc;
	char tmpstr[6];

	if(strlen(stats)!=(ALL_STATS*4)) return 0;
	else
	{
		for(cc=0; cc<ALL_STATS; cc++)
		{
			sprintf(tmpstr,"%c%c%c%c",stats[cc*4],stats[cc*4+1],stats[cc*4+2],stats[cc*4+3]);
			info->st[cc]=atoi(tmpstr);
		}
	}

	if(strlen(skills)!=(ALL_SKILLS*3)) return 0;
	else
	{
		for(cc=0; cc<ALL_SKILLS; cc++)
		{
			sprintf(tmpstr,"%c%c%c",skills[cc*3],skills[cc*3+1],skills[cc*3+2]);
			info->sk[cc]=atoi(tmpstr);
		}
	}

	if(strlen(perks)!=ALL_PERKS) return 0;
	else
	{
		for(cc=0; cc<ALL_PERKS; cc++)
		{
			sprintf(tmpstr,"%c",perks[cc]);
			info->pe[cc]=atoi(tmpstr);
		}
	}

	return 1;
}

DWORD SQL::NewObject(dyn_obj* obj)
{
	Query("INSERT INTO objects (num_st) VALUES ('0')");

	obj->id=mysql_insert_id(mySQL);

	SaveDataObject(obj);

	return obj->id;
}

void SQL::SaveDataObject(dyn_obj* obj)
{
	Query("UPDATE objects SET num_st='%d',player='%d',active='%d',map='%d',x='%d',y='%d',"
	"tick='%d',last_tick='%d',broken='%d',holder='%d',id_bull='%d',holder_ext='%d',id_bull_ext='%d' WHERE id='%d';",
	obj->object->id,obj->player,obj->active,obj->map,obj->x,obj->y,
	obj->tick,obj->last_tick,obj->broken,obj->holder,obj->id_bull,obj->holder_ext,obj->id_bull_ext,
	obj->id);
}

int SQL::LoadDataObject(dyn_obj* obj)
{
//	obj->object->p[0]=GetInt("objects","num_st","id",obj->id);
	char str[64];

	sprintf(str, "SELECT * FROM objects WHERE id='%d'", obj->id);
	mysql_query(mySQL, str);

	if(!(mySQL_res=mysql_store_result(mySQL))) return 0;
	if(!mysql_num_rows(mySQL_res)) return 0;
	mySQL_row=mysql_fetch_row(mySQL_res);

	obj->player=atoi(mySQL_row[2]);
	obj->active=atoi(mySQL_row[3]);
	obj->map=atoi(mySQL_row[4]);
	obj->x=atoi(mySQL_row[5]);
	obj->y=atoi(mySQL_row[6]);

	obj->tick=atoi(mySQL_row[7]);
	obj->last_tick=atoi(mySQL_row[8]);
	obj->broken=atoi(mySQL_row[9]);

	obj->holder=atoi(mySQL_row[10]);
	obj->id_bull=atoi(mySQL_row[11]);
	obj->holder_ext=atoi(mySQL_row[12]);
	obj->id_bull_ext=atoi(mySQL_row[13]);

	return 1;
}

void SQL::DeleteDataObject(dyn_obj* obj)
{
	Query("DELETE FROM objects WHERE id='%d'",obj->id);
}

void SQL::AddCheat(CrID user_id, char* text_cheat)
{
	Query("INSERT INTO users_cheat (user_id,text_cheat) VALUES ('%d','%s');",user_id,text_cheat);
}

int SQL::CountTable(char* table, char* row)
{
	if(!Query("SELECT %s FROM %s", row, table)) return 0;

	if(!(mySQL_res=mysql_store_result(mySQL))) return 0;

	return mysql_num_rows(mySQL_res);
}


void SQL::PrintTableInLog(char* table, char* rows)
{
	LogExecStr("Выводим таблицу: %s\n", table);

	if(!Query("SELECT %s FROM %s",rows,table)) return;
	
	if(!(mySQL_res=mysql_store_result(mySQL))) return;

	LogExecStr("Всего записей: %d \n",mysql_num_rows(mySQL_res));

	while((mySQL_row=mysql_fetch_row(mySQL_res)))
	{
		for(int pt=0; pt<mysql_num_fields(mySQL_res); pt++)
			LogExecStr("%s - ",mySQL_row[pt]); 
		LogExecStr(" | \n ");
	}

}
