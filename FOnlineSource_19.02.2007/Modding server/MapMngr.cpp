/********************************************************************
	created:	10:02:2007   15:22;

	author:		Anton Tsvetinsky
	
	purpose:	
*********************************************************************/

#include "stdafx.h"
#include "MapMngr.h"

int CMapMngr::Init()
{
	if(active) return 1;

	if(!fm.Init()) return 0;

	transit_map[11][0x10006D]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x6D].flags,FT_TRANSIT);
	transit_map[11][0x10006E]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x6E].flags,FT_TRANSIT);
	transit_map[11][0x10006F]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x6F].flags,FT_TRANSIT);
	transit_map[11][0x100070]=0x0C01008C00057;
	SETFLAG(tile[11][0x10][0x70].flags,FT_TRANSIT);
	transit_map[11][0x100071]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x71].flags,FT_TRANSIT);
	transit_map[11][0x100072]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x72].flags,FT_TRANSIT);
	transit_map[11][0x100073]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x73].flags,FT_TRANSIT);
	transit_map[11][0x100074]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x74].flags,FT_TRANSIT);
	transit_map[11][0x100075]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x75].flags,FT_TRANSIT);
	transit_map[11][0x100076]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x76].flags,FT_TRANSIT);
	transit_map[11][0x100077]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x77].flags,FT_TRANSIT);
	transit_map[11][0x100078]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x78].flags,FT_TRANSIT);
	transit_map[11][0x100079]=0x0C01008C0057;
	SETFLAG(tile[11][0x10][0x79].flags,FT_TRANSIT);

	transit_map[12][0x8E0054]=0x0B0300160073;
	SETFLAG(tile[12][0x8E][0x54].flags,FT_TRANSIT);
	transit_map[12][0x8E0055]=0x0B0300160073;
	SETFLAG(tile[12][0x8E][0x55].flags,FT_TRANSIT);
	transit_map[12][0x8E0056]=0x0B0300160073;
	SETFLAG(tile[12][0x8E][0x56].flags,FT_TRANSIT);
	transit_map[12][0x8E0057]=0x0B0300160073;
	SETFLAG(tile[12][0x8E][0x57].flags,FT_TRANSIT);
	transit_map[12][0x8E0058]=0x0B0300160073;
	SETFLAG(tile[12][0x8E][0x58].flags,FT_TRANSIT);
	transit_map[12][0x8E0059]=0x0B0300160073;
	SETFLAG(tile[12][0x8E][0x59].flags,FT_TRANSIT);
	transit_map[12][0x8E005A]=0x0B0300160073;
	SETFLAG(tile[12][0x8E][0x5A].flags,FT_TRANSIT);
	transit_map[12][0x8E005B]=0x0B0300160073;
	SETFLAG(tile[12][0x8E][0x5B].flags,FT_TRANSIT);

	active=TRUE;
	return 1;
}

void CMapMngr::Clear()
{
	for(int i=0; i<MAX_MAPS; i++)
		transit_map[i].clear();

	active=FALSE;
}

int CMapMngr::LoadAllMaps()
{
	LogExecStr("Загрузка статики карт\n");

	FILE *cf;

	if(!(cf=fopen("maps\\data_maps.txt","rt")))
	{
		LogExecStr("Файл data_maps не найден");
		return 0;
	}

	char ch;

	WORD tmp_wrd;
	char tmp_str[32];
	
	while(!feof(cf))
	{
		fscanf(cf,"%c",&ch);

		if(ch!='@') continue;

		fscanf(cf,"%d%s",&tmp_wrd,&tmp_str);

		map_str.insert(map_str_map::value_type(tmp_wrd,string(tmp_str)));
	}

	char fnam[32];

	for(map_str_map::iterator it_m=map_str.begin();it_m!=map_str.end();it_m++)
	{
		strcpy(fnam,(*it_m).second.c_str());
		LogExecStr("Загрузка карты |%s|...",fnam);

		if(fm.LoadFile(fnam,PT_MAPS))
		{
			
		}
		else
		{
			LogExecStr("FALSE\n");
			return 0;
		}

		LogExecStr("OK\n");
	}

	LogExecStr("Загрузка статики карт прошла успешно\n");

	return 1;
}

int CMapMngr::MoveToTile(crit_info* info, HexTYPE mx, HexTYPE my)
{
	if(mx>=MAXTILEX || my>=MAXTILEY) return MR_FALSE;

	if(!FLAG(tile[info->map][mx][my].flags,FT_PASSED)) return MR_FALSE;
	if(FLAG(tile[info->map][mx][my].flags,FT_PLAYER)) return MR_FALSE;

	UNSETFLAG(tile[info->map][info->x][info->y].flags,FT_PLAYER);

	if(FLAG(tile[info->map][mx][my].flags,FT_TRANSIT))
	{
		LONGLONG find_transit=FindTransit(info->map,mx,my);

		if(find_transit!=0xFFFFFFFFFFFFFFFF)
		{
			WORD new_map= (find_transit & 0xFFFF0000000000) >> 40;
			BYTE new_ori= (find_transit & 0x0000FF00000000) >> 32;
			HexTYPE new_x=(find_transit & 0x000000FFFF0000) >> 16;
			HexTYPE new_y=(find_transit & 0x0000000000FFFF);

			if(TransitToTile(info,new_map,new_x,new_y)==TR_OK)
			{
				info->ori=new_ori;

				SETFLAG(tile[new_map][new_x][new_y].flags,FT_PLAYER);

				return MR_TRANSIT;
			}
		}
	}

	info->x=mx;
	info->y=my;

	SETFLAG(tile[info->map][mx][my].flags,FT_PLAYER);

	//LogExecStr("x=%d,y=%d\n",acl->info.x,acl->info.y);

	return MR_STEP;
}

int CMapMngr::TransitToTile(crit_info* info, WORD tmap, HexTYPE tx, HexTYPE ty)
{
	if(tx>=MAXTILEX || ty>=MAXTILEY) return TR_FALSE;

	bool find_place=false;

	if(FLAG(tile[tmap][tx][ty].flags,FT_PASSED))
		if(!FLAG(tile[tmap][tx][ty].flags,FT_PLAYER)) find_place=true;

	if(find_place==false)
	{
		int i,i2;

		for(i=-2;i<3;i++)
			for(i2=-2;i2<3;i2++)
			{
				if(!i && !i2) continue;
				if(tx+i<0 || tx+i>=MAXTILEX) continue;
				if(ty+i2<0 || ty+i2>=MAXTILEY) continue;

				if(FLAG(tile[tmap][tx+i][ty+i2].flags,FT_PASSED))
					if(!FLAG(tile[tmap][tx+i][ty+i2].flags,FT_PLAYER))
					{
						find_place=true;
						break;
					}
			}

		if(find_place==true)
		{
			tx+=(i);
			ty+=(i2);
		}
		else return TR_FALSE;
	}

	SETFLAG(tile[tmap][tx][ty].flags=tile[tmap][tx][ty].flags,FT_PLAYER);

	info->map=tmap;
	info->x=tx;
	info->y=ty;
	return TR_OK;
}

LONGLONG CMapMngr::FindTransit(WORD num_map, HexTYPE num_x, HexTYPE num_y)
{
	LONGLONG find_transit=0;

	find_transit+=num_x << 16;
	find_transit+=num_y;

	longlong_map::iterator it=transit_map[num_map].find(find_transit);

	if(it==transit_map[num_map].end()) return 0xFFFFFFFFFF;

	if((((*it).second >> 40) & 0xFFFF)	>= MAX_MAPS) return 0xFFFFFFFFFFFFFFFF;
	if((((*it).second >> 32) & 0xFF)	> 5)		 return 0xFFFFFFFFFFFFFFFF;
	if((((*it).second >> 16) & 0xFFFF)	>= MAXTILEX) return 0xFFFFFFFFFFFFFFFF;
	if(((*it).second		 & 0xFFFF)	>= MAXTILEY) return 0xFFFFFFFFFFFFFFFF;

	return (*it).second;
}

void CMapMngr::InsertObjOnMap(dyn_obj* ins_obj, MapTYPE omap, HexTYPE ox, HexTYPE oy)
{
	if(!omap || omap>=MAX_MAPS || ox>=MAXTILEX || oy>=MAXTILEY)
	{
		ins_obj->map=0;
		ins_obj->x=0;
		ins_obj->y=0;
		return;
	}

	ins_obj->map=omap;
	ins_obj->x=ox;
	ins_obj->y=oy;

	DHexTYPE hex_ind;
	hex_ind=ins_obj->x<<16;
	hex_ind+=ins_obj->y;

	dyn_map* obj_cont;
	dyn_map_map::iterator it_tile_objects=objects_map[omap].find(hex_ind);
	if(it_tile_objects==objects_map[omap].end())
	{
		obj_cont=new dyn_map;
		objects_map[omap].insert(dyn_map_map::value_type(hex_ind,obj_cont));
		SETFLAG(tile[omap][ox][oy].flags,FT_CONTAINER);
	}
	else
		obj_cont=(*it_tile_objects).second;

	obj_cont->insert(dyn_map::value_type(ins_obj->id,ins_obj));
/*
	word_set* pics;
	word_set_map::iterator it_pics_cont=pic_on_ground[omap].find(hex_ind);
	if(it_pics_cont==pic_on_ground[omap].end())
	{
		word_set new_pics;
		pic_on_ground[omap].insert(word_set_map::value_type(hex_ind,new_pics));
		pics=&pic_on_ground[omap][hex_ind];
	}
	else
		pics=&(*it_pics_cont).second;

	if(!pics->count(ins_obj->object->p111[OBJ_PIC_MAP]))
		pics->insert(ins_obj->object->p111[OBJ_PIC_MAP]);
*/
}

void CMapMngr::EraseObjFromMap(dyn_obj* ers_obj)
{
	DHexTYPE hex_ind;
	hex_ind=ers_obj->x<<16;
	hex_ind+=ers_obj->y;

	dyn_map_map::iterator it_tile_objects=objects_map[ers_obj->map].find(hex_ind);
	if(it_tile_objects==objects_map[ers_obj->map].end())
	{
		UNSETFLAG(tile[ers_obj->map][ers_obj->x][ers_obj->y].flags,FT_CONTAINER);
		ers_obj->map=0;
		ers_obj->x=0;
		ers_obj->y=0;
		return;
	}
	dyn_map* tile_objects=(*it_tile_objects).second;

	tile_objects->erase(ers_obj->id);

	if(tile_objects->empty())
	{
		UNSETFLAG(tile[ers_obj->map][ers_obj->x][ers_obj->y].flags,FT_CONTAINER);

		objects_map[ers_obj->map].erase(hex_ind);
		delete tile_objects;
	}

	ers_obj->map=0;
	ers_obj->x=0;
	ers_obj->y=0;
}
/*
void CMapMngr::EraseObject(dyn_obj* ers_obj, dyn_map::iterator* it_obj)
{
//	delete (**it_obj).second;

//	tile[ers_obj->map][ers_obj->x][ers_obj->y].obj.erase((*it_obj));
}
*/
void CMapMngr::ClearPlayer(crit_info* info)
{
//	if(tile[info->map][info->x][info->y].flags & FT_PLAYER)
//		tile[info->map][info->x][info->y].flags=tile[info->map][info->x][info->y].flags^FT_PLAYER;

	UNSETFLAG(tile[info->map][info->x][info->y].flags,FT_PLAYER);
}