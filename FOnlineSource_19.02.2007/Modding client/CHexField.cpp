#include "stdafx.h"

#include "CHexField.h"
#include "common.h"
// ����������� ������� �����
#define DRAW_ORDER_HEX  0
//#define DRAW_ORDER_MISC 1 //!Cvet
#define DRAW_ORDER_WALL 1
#define DRAW_ORDER_SCEN 2
#define DRAW_ORDER_CRIT 3
#define DRAW_ORDER_ITEM 4

//����������� �������
#define SP_SCEN_LIGHT  141 //Light Source
#define SP_SCEN_MARKER  49 //Exit Grid Map Marker
#define SP_SCEN_BLOCK   67 //Secret Blocking Hex
#define SP_SCEN_IBLOCK 344 //Block Hex Auto Inviso

#define SP_WALL_BLOCK 622 //Wall s.t.

CHexField::CHexField()
{
	view2=NULL;
	lpVBpr_tile=NULL;
	lpVBpr_roof=NULL;
	crtd=0;
	lpSM=NULL;
	ShowHex=0;

	for(int i=0;i<MAXTILEX;i++)
		for(int j=0;j<MAXTILEY;j++)
			hex_field[i][j].pos=i*2000+j*10;

	hbegin=0;
	hend=0;
	wright=0;
	wleft=0;
}

int CHexField::Init(CSpriteManager* lpsm)
{
	WriteLog("CHexField Initialization...\n");

	if(!lpsm)
	{
		ErrMsg("CHexField Init","lpsm=NULL");
		return 0;
	}
	lpSM=lpsm;

	if(!fm.Init()) return 0;
	if(!fm_map.Init()) return 0;

	if(!LoadList("tiles.lst",PT_ART_TILES,&tile_fnames)) return 0;
	if(!LoadList("items.lst",PT_ART_ITEMS,&item_fnames)) return 0; //!Cvet
	if(!LoadList("items.lst",PT_PRO_ITEMS,&item_proto)) return 0; //!Cvet
	if(!LoadList("scenery.lst",PT_ART_SCENERY,&scen_fnames)) return 0;
	if(!LoadList("scenery.lst",PT_PRO_SCENERY,&scen_proto)) return 0;
	if(!LoadList("walls.lst",PT_ART_WALLS,&wall_fnames)) return 0;
//	if(!LoadList("misc.lst",PT_ART_MISC,&misc_fnames)) return 0; //!Cvet

	if(!(hex=lpSM->LoadSprite("hex.frm",PT_ART_MISC))) return 0;
	if(!(hexb=lpSM->LoadSprite("hexb.frm",PT_ART_MISC))) return 0;

	WriteLog("CHexField Initialization complete\n");
	crtd=1;

	MapLoaded=FALSE; //!Cvet

	return 1;
}

void CHexField::Clear()
{
	WriteLog("CHexField Clear...\n");

	fm.Clear();
	fm_map.Clear();

	for(char_map::iterator it=item_proto.begin();it!=item_proto.end();it++) //!Cvet
		delete[] (*it).second;
	item_proto.clear();

	for(it=scen_proto.begin();it!=scen_proto.end();it++)
		delete[] (*it).second;
	scen_proto.clear();


	for(it=item_fnames.begin();it!=item_fnames.end();it++)
		delete[] (*it).second;
	item_fnames.clear();

	for(it=scen_fnames.begin();it!=scen_fnames.end();it++)
		delete[] (*it).second;
	scen_fnames.clear();

	for(it=wall_fnames.begin();it!=wall_fnames.end();it++)
		delete[] (*it).second;
	wall_fnames.clear();

	for(it=tile_fnames.begin();it!=tile_fnames.end();it++)
		delete[] (*it).second;
	tile_fnames.clear();

//	for(it=misc_fnames.begin();it!=misc_fnames.end();it++)
//		delete[] (*it).second;
//	misc_fnames.clear();


	for(dtree_map::iterator jt=dtree.begin();jt!=dtree.end();jt++)
		SAFEDEL((*jt).second);
	dtree.clear();

	for(int y=0;y<MAXTILEY;y++)
		for(int x=0;x<MAXTILEX;x++)
		{
			for(scen_vect::iterator si=hex_field[y][x].sc_obj.begin();si!=hex_field[y][x].sc_obj.end();si++)
				delete (*si);

			for(item_vect::iterator ti=hex_field[y][x].itm_obj.begin();ti!=hex_field[y][x].itm_obj.end();ti++)
				delete (*ti);

//			for(misc_vect::iterator mi=hex_field[y][x].msc_obj.begin();mi!=hex_field[y][x].msc_obj.end();mi++)
//				delete (*mi);
		}

	for(onesurf_vec::iterator oi=prep_vec_tile.begin();oi!=prep_vec_tile.end();oi++)
		delete (*oi);
	prep_vec_tile.clear();

	for(oi=prep_vec_roof.begin();oi!=prep_vec_roof.end();oi++)
		delete (*oi);
	prep_vec_roof.clear();

	SAFEREL(lpVBpr_tile);
	SAFEREL(lpVBpr_roof);
	SAFEDELA(view2);

	MapLoaded=FALSE; //!Cvet

	crtd=0;

	WriteLog("CHexField Clear complete\n");
}

int CHexField::LoadList(char* lname,int PathType,char_map* pmap)
{
	if(!fm.LoadFile(lname,PathType))
		return 0;
	char str[1024];
	char *rec=NULL;
	int i=0;

	while(fm.GetStr(str,1023))
	{
		strlwr(str);
		char* pos=strstr(str,".");
		if(!pos)
		{	
			i++;
			WriteLog("CHexField LoadList> � list ����� ������� ������ ��� ����� �����: %s\n",str);
			continue; 
		}
		pos[4]=0;//�������� �����������
		rec=new char[strlen(str)+1];
		strcpy(rec,str);
		(*pmap)[i++]=rec;
	}

	fm.UnloadFile();

	return 1;
}

int CHexField::LoadMap(char* fname)
{
	WriteLog("�������� ����� %s...\n",fname);
	TICK gc=GetTickCount();

	UnLoadMap(); //!Cvet

//	ac1=0;ac2=0;ac3=0;ac4=0;ac5=0;ac6=0;ac7=0;acres=0;

	if(!fm_map.LoadFile(fname,PT_MAPS)) return 0;

	fm_map.SetCurPos(0x20);
	DWORD nlv=fm_map.GetDWord();//localvars
	fm_map.SetCurPos(0x30);
	DWORD ngv=fm_map.GetDWord();//global vars

	fm_map.SetCurPos(0x28); //!Cvet ������� �����
	DWORD count_tiles_y=fm_map.GetDWord(); //!Cvet

	fm_map.SetCurPos(0xEC+4*(nlv+ngv)); //������ ������ ������

	WORD tile; //ground_tile
	WORD rtile; //roof_tile
	word_map::iterator it;

	for(int y=0;y<100;y++)
		for(int x=0;x<100;x++)
		{
			rtile=fm_map.GetWord();
			tile=fm_map.GetWord();
			if(tile>1) //1 ������ ������ ����
			{
				it=loaded_tile.find(tile);
				if(it==loaded_tile.end()) //���� ���� �� ��������
				{
					WORD id=lpSM->LoadSprite(tile_fnames[tile],PT_ART_TILES);
					if(!id) return 0;
					loaded_tile[tile]=id;
					hex_field[2*y][2*x].tile_id=id;
				}
				else //�������� � ���� ������ �� ID SpriteMngr
				{
					hex_field[2*y][2*x].tile_id=(*it).second;
				}
			}

			if(rtile>1) //!Cvet Roof
			{
				it=loaded_tile.find(rtile);
				if(it==loaded_tile.end())
				{
					WORD id=lpSM->LoadSprite(tile_fnames[rtile],PT_ART_TILES);
					if(!id) return 0;
					loaded_tile[rtile]=id;
					hex_field[2*y][2*x].roof_id=id;
				}
				else
				{
					hex_field[2*y][2*x].roof_id=(*it).second;
				}
			}
		}

//!Cvet ++++ ������������ ����� ���������� ������
	switch (count_tiles_y)
	{
		case 0x00:
			fm_map.GoForward(80000);
			break;
		case 0x08:
			fm_map.GoForward(40000);
			break;
	}
//!Cvet ----

	if(!DropScript()) return 0; //��������� ����� ������ ��������

	if(!LoadObj()) return 0;

	fm_map.UnloadFile();

	MapLoaded=TRUE;

//	for(int i=0;i<200;i+=20)
//		for(int i2=0;i2<200;i2+=20)
//			SetCenter2(i,i2);

	WriteLog("����� ��������� �� %d ms\n",GetTickCount()-gc);
//	WriteLog("ac1=%d ac2=%d ac3=%d ac4=%d ac5=%d ac6=%d ac7=%d acres=%d\n",ac1,ac2,ac3,ac4,ac5,ac6,ac7,acres);
	return 1;
}

int CHexField::DropScript()
{
	int cnt=0;
	int all_cnt=0; //!Cvet ���-�� ���� �������

	for(int i=0;i<5;i++)
	{
		cnt=fm_map.GetDWord();

		if(!i) continue;

		if(cnt>0)
		{
			DWORD loop;

			for(loop=16;loop<cnt;loop+=16);
			DWORD check=0;

			for(int j=0;j<loop;j++)
			{
				BYTE group=fm_map.GetByte();

			//!Cvet ��������� ��� ����� ����� ++++
				switch (group)
				{
				case 1:
					fm_map.GoForward(0x47);
					break;
				case 2:
					fm_map.GoForward(0x43);
					break;
				case 3:
					fm_map.GoForward(0x3F);
					break;
				case 4:
					fm_map.GoForward(0x3F);
					break;
				default:
					fm_map.GoForward(0x3F);
					break;
				}
			//!Cvet ��������� ��� ����� ����� ----

				if((j % 16) == 15)
				{
					DWORD v=fm_map.GetDWord();

					check+=v;

					v=fm_map.GetDWord(); //CRC
		        }

			}//script loop

			if(check!=cnt)
			{
				ErrMsg("CHexField DropScript","������ ��� ����������� ������ ��������");
				return 0;
			}

		}

		all_cnt+=cnt; //!Cvet
	}

	WriteLog("Scripts> %d\n",all_cnt); 
	return 1;
}

int CHexField::LoadObj()
{
	DWORD cnt1=fm_map.GetDWord();
	DWORD cnt2=fm_map.GetDWord();

	WORD hbmax=0; //���������� ������������ ������� ��������
	WORD hemax=0; //����� ��������� ������� ���������� ������� ���������
	WORD wrmax=0;
	WORD wlmax=0;

	DWORD itm_cnt=0; //!Cvet ���-�� ������
	DWORD cr_cnt=0; //!Cvet ���-�� ���������
	DWORD sc_cnt=0; //���������� scenery
	DWORD wl_cnt=0; //���������� walls

	for(int cic=0;cic<cnt2;cic++)
	{
		DWORD buf=fm_map.GetDWord();

		DWORD pos=fm_map.GetDWord();//�������
		DWORD y=pos/200;
		DWORD x=pos%200; //���������� � ����� ����� (�� � �������)

		DWORD offs_x=fm_map.GetDWord(); //!Cvet
		DWORD offs_y=fm_map.GetDWord(); //!Cvet

		fm_map.GoForward(8); //??? ��� ��������� �� ����������

		DWORD frm_num=fm_map.GetDWord(); //frame number
		DWORD ori=fm_map.GetDWord(); //����������

		buf=fm_map.GetDWord(); //frm pid
		DWORD type=buf >> 0x18; //��� ������� (scenery, walls etc)
		DWORD id=buf & 0xFFFF; //�������������

		DWORD flags=fm_map.GetDWord();

		DWORD elev=fm_map.GetDWord();

		DWORD proto_id=fm_map.GetDWord();

		fm_map.GoForward(4);

		DWORD lights=fm_map.GetDWord();
		DWORD lighti=fm_map.GetDWord();

		fm_map.GoForward(12); //�������� ����� Uncknown ����

		DWORD num_obj=fm_map.GetDWord(); //���-�� �������� � ����������

		fm_map.GoForward(12); //�������� ����� Uncknown ����

		switch(type)
		{
		case OBJ_ITEM: //!Cvet items
			if(!ParseItemObj(proto_id,id,x,y,&hbmax,&hemax,&wrmax,&wlmax)) return 0;
			itm_cnt++;
			break;
		case OBJ_CRIT: //!Cvet critters
			fm_map.GoForward(40);
			cr_cnt++;
			break;
		case OBJ_SCEN: //scenery
			if(!ParseScenObj(proto_id,id,x,y,&hbmax,&hemax,&wrmax,&wlmax)) return 0;
			sc_cnt++;
			break;
		case OBJ_WALL: //wall
			if(!ParseWallObj(proto_id,id,x,y,&hbmax,&hemax,&wrmax,&wlmax)) return 0;
			wl_cnt++;
			break;
		case OBJ_MISC:
			if(!ParseMiscObj(proto_id,id,x,y,&hbmax,&hemax,&wrmax,&wlmax)) return 0;
//			msc_cnt++;
			break;
		default:
			continue;
		}

		for(int o=0; o<num_obj; o++)
		{
			
			fm_map.GoForward(4);

			fm_map.GoForward(44);

			DWORD proto_id=fm_map.GetDWord();

			fm_map.GoForward(40);

			ParseItemObjCont(proto_id,x,y);
		}
	}

	WriteLog("Critters> %d\n",cr_cnt); //!Cvet
	WriteLog("Items> %d\n",itm_cnt); //!Cvet
	WriteLog("Scenery> %d\n",sc_cnt);
	WriteLog("Walls> %d\n",wl_cnt);

	hbegin=(hbmax-24-6)/12; //������� �������
	hbegin+=hbegin%2;
	
	hend=(hemax-24-6)/12; //�������
	hend+=hend%2;

	wright=(wrmax-32-16)/32; //������
	wright+=wright%2;

	wleft=(wlmax-0-16)/32; //�����
	wleft+=wleft%2+2;

	if(hbegin<0) hbegin=0;
	if(hend<0) hend=0;
	if(wright<0) wright=0;
	if(wleft<0) wleft=0;

	v2h=VIEW_HEIGHT+hbegin+hend; //������ ������� �������
	v2w=VIEW_WIDTH+wright+wleft; //������

	v2c_x=VIEW_CX+wright; //������
	v2c_y=VIEW_CY+hbegin;

	SAFEDELA(view2);

	view2=new ViewField[v2h*v2w]; //������� ����� ������� �������

	WriteLog("hbegin=%d,hend=%d,wright=%d,wleft=%d\n",hbegin,hend,wright,wleft);

	return 1;
}

//!Cvet ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int CHexField::ParseItemObj(DWORD proto_id,DWORD id,DWORD x,DWORD y,WORD* hbmax,WORD* hemax,WORD* wrmax,WORD* wlmax)
{
	DWORD sub_type;

	proto_id&=0xFFFF; //???Cvet 0xFFFFFF

	if(!fm.LoadFile(item_proto[(proto_id)-1],PT_PRO_ITEMS)) return 0;
	fm.SetCurPos(0x20); //������ ������
	sub_type=fm.GetDWord();
	fm.UnloadFile();

	switch (sub_type)
	{
	case SUB_ITEM_ARMOR:
	case SUB_ITEM_CONT:
	case SUB_ITEM_DRUG:
		break;
	case SUB_ITEM_WEAPON:
		fm_map.GoForward(8);
		break;
	case SUB_ITEM_AMMO:
	case SUB_ITEM_MISC:
	case SUB_ITEM_KEY:
		fm_map.GoForward(4);
		break;
	}

	return 1; //�� ������!!!

	SpriteInfo* lpinf;
	word_map::iterator it;
	WORD spr_id;

	it=loaded_item.find(id);
	if(it==loaded_item.end())
	{
		spr_id=lpSM->LoadSprite(item_fnames[id],PT_ART_ITEMS,&lpinf);
		if(!spr_id) return 0;

		loaded_item[id]=spr_id;
	}
	else
	{
		spr_id=(*it).second;
		lpinf=lpSM->GetSpriteInfo(spr_id);
	}

	if(lpinf->offs_y>(*hbmax)) (*hbmax)=lpinf->offs_y;
	if( (lpinf->h-lpinf->offs_y) > (*hemax)) (*hemax)=lpinf->h-lpinf->offs_y;
	if( ((lpinf->w >> 1)-lpinf->offs_x)>(*wrmax)) (*wrmax)=(lpinf->w >> 1)-lpinf->offs_x;
	if( ((lpinf->w >> 1)+lpinf->offs_x)>(*wlmax)) (*wlmax)=(lpinf->w >> 1)+lpinf->offs_x;

	ItemObj* newitm;
	newitm=new ItemObj(spr_id);
	hex_field[y][x].itm_obj.push_back(newitm);
	
	return 1;
}

int CHexField::ParseItemObjCont(DWORD proto_id,DWORD x,DWORD y)
{
	DWORD sub_type;
	DWORD frm_inv;

	proto_id&=0xFFFF; //???0xFFFFFF

	if(!fm.LoadFile(item_proto[(proto_id)-1],PT_PRO_ITEMS)) return 0;
	fm.SetCurPos(0x20); //������ ������
	sub_type=fm.GetDWord();
	fm.SetCurPos(0x34); //Inv FID
	frm_inv=fm.GetDWord();
	fm.UnloadFile();

	switch (sub_type)
	{
	case SUB_ITEM_ARMOR:
	case SUB_ITEM_CONT:
	case SUB_ITEM_DRUG:
		break;
	case SUB_ITEM_WEAPON:
		fm_map.GoForward(8);
		break;
	case SUB_ITEM_AMMO:
	case SUB_ITEM_MISC:
	case SUB_ITEM_KEY:
		fm_map.GoForward(4);
		break;
	}

	return 1;
}

int CHexField::ParseMiscObj(DWORD proto_id,DWORD id,DWORD x,DWORD y,WORD* hbmax,WORD* hemax,WORD* wrmax,WORD* wlmax)
{
//	DWORD sub_type=SUB_MISC_MISC;

//	WriteLog("misc - %d - %d,%d\n",id,x,y);

	if(id==1)
	{
		hex_field[y][x].scroll_block=TRUE;
		hex_field[y][x].passed=FALSE;
		hex_field[y][x].raked=FALSE;
	}

	if((id>=33 && id<=48) || (id>=17 && id<=24)) fm_map.GoForward(16);

//	if((id>=33 && id<=48) || (id>=17 && id<=24)) sub_type=SUB_MISC_EXITGR;

//	if(sub_type==SUB_MISC_EXITGR) fm_map.GoForward(16);

//	if(sub_type==SUB_MISC_MISC) return 1;

	return 1;
/*
	SpriteInfo* lpinf;
	word_map::iterator it;
	WORD spr_id;

	it=loaded_misc.find(id);
	if(it==loaded_misc.end())
	{
		spr_id=lpSM->LoadSprite(misc_fnames[id],PT_ART_MISC,&lpinf);
		if(!spr_id) return 0;

		loaded_misc[id]=spr_id;
	}
	else
	{
		spr_id=(*it).second;
		lpinf=lpSM->GetSpriteInfo(spr_id);
	}

	if(lpinf->offs_y>(*hbmax)) (*hbmax)=lpinf->offs_y;
	if( (lpinf->h-lpinf->offs_y) > (*hemax)) (*hemax)=lpinf->h-lpinf->offs_y;
	if( ((lpinf->w >> 1)-lpinf->offs_x)>(*wrmax)) (*wrmax)=(lpinf->w >> 1)-lpinf->offs_x;
	if( ((lpinf->w >> 1)+lpinf->offs_x)>(*wlmax)) (*wlmax)=(lpinf->w >> 1)+lpinf->offs_x;

	MiscObj* newmsc;
	newmsc=new MiscObj(spr_id);
	hex_field[y][x].msc_obj.push_back(newmsc);

	return 1;
*/
}

int CHexField::AddItemObj(DWORD pic_id, HexTYPE x, HexTYPE y)
{
	SpriteInfo* lpinf;
	word_map::iterator it;
	WORD spr_id;

	it=loaded_item.find(pic_id);
	if(it==loaded_item.end())
	{
		spr_id=lpSM->LoadSprite(item_fnames[pic_id],PT_ART_ITEMS,&lpinf);
		if(!spr_id) return 0;

		loaded_item[pic_id]=spr_id;
	}
	else
	{
		spr_id=(*it).second;
		lpinf=lpSM->GetSpriteInfo(spr_id);
	}

	ItemObj* newitm;
	newitm=new ItemObj(spr_id);
	hex_field[y][x].itm_obj.push_back(newitm);

	if(IsVisible(x, y, spr_id))
		dtree.insert(dtree_map::value_type(hex_field[y][x].pos+DRAW_ORDER_ITEM, new PrepSprite(hex_field[y][x].scr_x+16,hex_field[y][x].scr_y+6,spr_id)));

	return 1;
}

void CHexField::DelItemObj(DWORD pic_id, HexTYPE x, HexTYPE y)
{
	word_map::iterator it=loaded_item.find(pic_id);

	if(it==loaded_item.end()) return;

	for(item_vect::iterator it_i=hex_field[y][x].itm_obj.begin();it_i!=hex_field[y][x].itm_obj.end();it_i++)
		if((*it_i)->spr_id==(*it).second)
		{
			for(dtree_map::iterator it_dt=dtree.find(hex_field[y][x].pos+DRAW_ORDER_ITEM);it_dt!=dtree.end();it_dt++)
				if((*it_dt).second->spr_id==(*it).second) break;

			if(it_dt!=dtree.end())
			{
				delete (*it_dt).second;
				dtree.erase(it_dt);
			}

			delete (*it_i);
			hex_field[y][x].itm_obj.erase(it_i);
			break;
		}
}

//Cvet --------------------------------------------------------------------

//proto_id - ���� ���������� �� �������. id - ��� frm_id - ������������� ��������
int CHexField::ParseScenObj(DWORD proto_id,DWORD id,DWORD x,DWORD y,WORD* hbmax,WORD* hemax,WORD* wrmax,WORD* wlmax)
{
	DWORD sub_type;

	proto_id&=0xFFFF;

	if(!fm.LoadFile(scen_proto[(proto_id)-1],PT_PRO_SCENERY)) return 0;
	fm.SetCurPos(0x20); //Scenery Subtype
	sub_type=fm.GetDWord();
	fm.UnloadFile();

	switch(sub_type)
	{
	case SUB_SCEN_PORTAL:
		fm_map.GoForward(4);
		break;
	case SUB_SCEN_MISC:
		break;
	case SUB_SCEN_STAIR:
	case SUB_SCEN_ELEV:
	case SUB_SCEN_LADDWN:
	case SUB_SCEN_LADTOP:
		fm_map.GoForward(8);
		break;
	}

	bool noload=0;
	switch(proto_id)
	{
	case SP_SCEN_BLOCK:
	case SP_SCEN_IBLOCK:
		hex_field[y][x].passed=FALSE; //!Cvet ������������
	case SP_SCEN_LIGHT:
	case SP_SCEN_MARKER:
		noload=1; //�� ��������� ���� ����-�������
		break;
	default:
		break;
	}

	if(noload) return 1;

	SpriteInfo* lpinf;
	word_map::iterator it;
	WORD spr_id;

	it=loaded_scen.find(id);
	if(it==loaded_scen.end()) //�������� ����� ������� �� ��������� ���
	{
		spr_id=lpSM->LoadSprite(scen_fnames[id],PT_ART_SCENERY,&lpinf);
		if(!spr_id) return 0;
			
		loaded_scen[id]=spr_id;//��������� � ������ �����������
	}
	else
	{
		spr_id=(*it).second;
		lpinf=lpSM->GetSpriteInfo(spr_id);
	}

	if(lpinf->offs_y>(*hbmax)) (*hbmax)=lpinf->offs_y;
	if( (lpinf->h-lpinf->offs_y) > (*hemax)) (*hemax)=lpinf->h-lpinf->offs_y;
	if( ((lpinf->w >> 1)-lpinf->offs_x)>(*wrmax)) (*wrmax)=(lpinf->w >> 1)-lpinf->offs_x;
	if( ((lpinf->w >> 1)+lpinf->offs_x)>(*wlmax)) (*wlmax)=(lpinf->w >> 1)+lpinf->offs_x;

	ScenObj* newsc;
	newsc=new ScenObj(spr_id);
	hex_field[y][x].sc_obj.push_back(newsc);

	return 1;
}

//proto_id - ���� ���������� �� �������. id - ��� frm_id - ������������� ��������
int CHexField::ParseWallObj(DWORD proto_id,DWORD id,DWORD x,DWORD y,WORD* hbmax,WORD* hemax,WORD* wrmax,WORD* wlmax)
{
	proto_id&=0xFFFF;

	hex_field[y][x].passed=FALSE; //!Cvet ������������
	hex_field[y][x].raked=FALSE; //!Cvet �����������������

	bool noload=0;
	switch(proto_id) 
	{
	case SP_WALL_BLOCK:
		noload=1;
		break;
	default:
		break;
	}

	if(noload) return 1;

	SpriteInfo* lpinf;
	word_map::iterator it;
	WORD spr_id;

	it=loaded_wall.find(id);
	if(it==loaded_wall.end()) //�������� ����� ������� �� ��������� ���
	{
		spr_id=lpSM->LoadSprite(wall_fnames[id],PT_ART_WALLS,&lpinf);
		if(!spr_id) return 0;

		loaded_wall[id]=spr_id;//��������� � ������ �����������
	}
	else
	{
		spr_id=(*it).second;
		lpinf=lpSM->GetSpriteInfo(spr_id);
	}

//��������� ������������ �������
	if(lpinf->offs_y>(*hbmax)) (*hbmax)=lpinf->offs_y;
	if( (lpinf->h-lpinf->offs_y) > (*hemax)) (*hemax)=lpinf->h-lpinf->offs_y;
	if( ((lpinf->w >> 1)-lpinf->offs_x)>(*wrmax)) (*wrmax)=(lpinf->w >> 1)-lpinf->offs_x;
	if( ((lpinf->w >> 1)+lpinf->offs_x)>(*wlmax)) (*wlmax)=(lpinf->w >> 1)+lpinf->offs_x;

	for(int i=0;i<MAX_WALL_CNT;i++)
		if(!hex_field[y][x].wall_id[i])
		{
			hex_field[y][x].wall_id[i]=spr_id;//� ������� � hex id ��������
			break;
		}

	return 1;
}

void CHexField::SetCenter2(int x, int y)
{
	if(x<0 || y<0 || x>=MAXTILEX || y>=MAXTILEY) return;
	int my=y+y%2;

	InitView2(x,my);

	int ty;

	int y2=0;
	int vpos;

	//������ ����� ��������� �� ������ �������
	for(ty=0;ty<v2h;ty++)
	{
		for(int tx=0;tx<v2w;tx++)
		{
			vpos=y2+tx;
			int mod_x=view2[vpos].cur_x;
			int mod_y=view2[vpos].cur_y;
			if(mod_x<0 || mod_y<0 || mod_x>=MAXTILEX || mod_y>=MAXTILEY) continue;
			hex_field[mod_y][mod_x].to_draw=0;
		}
		y2+=v2w;
	}

	y2=0;

	//�������� ������� �����, ������� ������ �� ������ ������. � �� ���������� �� ������.
	for(ty=0;ty<v2h;ty++)
	{
		for(int tx=0;tx<v2w;tx++)
		{
			vpos=y2+tx;
			int mod_x=view2[vpos].mod_x+x;
			int mod_y=view2[vpos].mod_y+y;
			view2[vpos].cur_x=mod_x;
			view2[vpos].cur_y=mod_y;
			if(mod_x<0 || mod_y<0 || mod_x>=MAXTILEX || mod_y>=MAXTILEY) continue;

			hex_field[mod_y][mod_x].to_draw=1;
			hex_field[mod_y][mod_x].scr_x=view2[vpos].scr_x;
			hex_field[mod_y][mod_x].scr_y=view2[vpos].scr_y;
		}
		y2+=v2w;
	}

	//���������� ������ �� ������ � �� ���� ���������� ����� ������, �������� � ���� �����
	RebuildTiles();

	//������� ������, � ������� ����� �������.
	for(dtree_map::iterator jt=dtree.begin();jt!=dtree.end();jt++)
		SAFEDEL((*jt).second);
	dtree.clear();

	y2=0;
	
	for(ty=0;ty<v2h;ty++)
	{
		for(int x=0;x<v2w;x++)
		{
			vpos=y2+x;
			int ny=view2[vpos].cur_y;
			int nx=view2[vpos].cur_x;
			if(ny<0 || nx<0 || ny>=MAXTILEY || nx>=MAXTILEX) continue;

			if(ShowHex)
			{
				if(x>wright && x<(v2w-wleft) && ty>hbegin && ty<(v2h-hend))
//					dtree[hex_field[ny][nx].pos+DRAW_ORDER_HEX]=new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,(ny==100 && nx==100)?hexb:hex);
					dtree.insert(dtree_map::value_type(hex_field[ny][nx].pos+DRAW_ORDER_HEX,
					new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,(ny==100 && nx==100)?hexb:hex)));
			}

	/*		if(!hex_field[ny][nx].msc_obj.empty() && cmn_show_misc)
				for(misc_vect::iterator mi=hex_field[ny][nx].msc_obj.begin();mi!=hex_field[ny][nx].msc_obj.end();mi++)
				{
					if(!IsVisible(nx, ny, (*mi)->spr_id)) continue;
					
					//����� ������� ��� � ������. ����� ������ �������� ���� ����� ��� ��� � ����� � ��������� ��������� �������
					//dtree[hex_field[ny][nx].pos+DRAW_ORDER_SCEN]=new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,(*si)->spr_id);
					dtree.insert(dtree_map::value_type(hex_field[ny][nx].pos+DRAW_ORDER_MISC, new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,(*mi)->spr_id)));
				}
*/
			if(!hex_field[ny][nx].sc_obj.empty() && cmn_show_scen)
				for(scen_vect::iterator si=hex_field[ny][nx].sc_obj.begin();si!=hex_field[ny][nx].sc_obj.end();si++)
				{
					if(!IsVisible(nx, ny, (*si)->spr_id)) continue;
					
					//����� ������� ��� � ������. ����� ������ �������� ���� ����� ��� ��� � ����� � ��������� ��������� �������
					//dtree[hex_field[ny][nx].pos+DRAW_ORDER_SCEN]=new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,(*si)->spr_id);
					dtree.insert(dtree_map::value_type(hex_field[ny][nx].pos+DRAW_ORDER_SCEN, new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,(*si)->spr_id)));
				}

			if(!hex_field[ny][nx].itm_obj.empty() && cmn_show_items)
				for(item_vect::iterator ti=hex_field[ny][nx].itm_obj.begin();ti!=hex_field[ny][nx].itm_obj.end();ti++)
				{
					if(!IsVisible(nx, ny, (*ti)->spr_id)) continue;
					
					//����� ������� ��� � ������. ����� ������ �������� ���� ����� ��� ��� � ����� � ��������� ��������� �������
					//dtree[hex_field[ny][nx].pos+DRAW_ORDER_SCEN]=new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,(*si)->spr_id);
					dtree.insert(dtree_map::value_type(hex_field[ny][nx].pos+DRAW_ORDER_ITEM, new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,(*ti)->spr_id)));
				}
			
			if(hex_field[ny][nx].wall_id[0] && cmn_show_walls)
			{
					for(int i=0;i<MAX_WALL_CNT;i++)
					{
						if(!hex_field[ny][nx].wall_id[i]) break;
						if(IsVisible(nx, ny, hex_field[ny][nx].wall_id[i]))
							dtree.insert(dtree_map::value_type(hex_field[ny][nx].pos+DRAW_ORDER_WALL, new PrepSprite(hex_field[ny][nx].scr_x+16,hex_field[ny][nx].scr_y+6,hex_field[ny][nx].wall_id[i])));
					}
			}

			if(hex_field[ny][nx].lpcrit && cmn_show_crit)
			{
				if(!IsVisible(nx, ny, hex_field[ny][nx].lpcrit->cur_id)) {hex_field[ny][nx].lpcrit->SetVisible(0);hex_field[ny][nx].lpcrit->rit=NULL;continue;} //!Cvet memory leak???!!!!!!!!!!
					hex_field[ny][nx].lpcrit->SetVisible(1);

				PrepSprite* prep=new PrepSprite(
					hex_field[ny][nx].scr_x+16,
					hex_field[ny][nx].scr_y+6,0,&hex_field[ny][nx].lpcrit->cur_id,
					&hex_field[ny][nx].lpcrit->cur_ox,
					&hex_field[ny][nx].lpcrit->cur_oy); 

				hex_field[ny][nx].lpcrit->rit=dtree.insert(dtree_map::value_type(hex_field[ny][nx].pos+DRAW_ORDER_CRIT, prep));

					// �� ���� ��� ���? �������� �� ������
				//	hex_field[ny][nx].lpcrit->rx=x;
				//	hex_field[ny][nx].lpcrit->ry=y;
				lpSM->GetDrawCntrRect(prep, &hex_field[ny][nx].lpcrit->drect);
                
			//	//!Cvet +++++++++
			//	hex_field[ny][nx].lpcrit->drect.l-=hex_field[ny][nx].lpcrit->cur_ox;
			//	hex_field[ny][nx].lpcrit->drect.r-=hex_field[ny][nx].lpcrit->cur_ox;
			//	hex_field[ny][nx].lpcrit->drect.t-=hex_field[ny][nx].lpcrit->cur_oy;
			//	hex_field[ny][nx].lpcrit->drect.b-=hex_field[ny][nx].lpcrit->cur_oy;
				//!Cvet ---------
			}
		}
		y2+=v2w;
	}

	ac1=dtree.size();

	cnt_x=x;
	cnt_y=my;
}

void CHexField::RebuildTiles() //!Cvet ���������. ������� ���������� �����
{
	//���������� ������ �� ������ � �� ���� ���������� ����� ������, �������� � ���� �����
	dtree_map ttree_t;
	dtree_map ttree_r; //!Cvet

	if(!cmn_show_tiles && !cmn_show_roof) //!Cvet ++++
	{
		prep_cnt_tile=0;
		prep_cnt_roof=0;
		lpSM->PrepareBuffer(&ttree_t, &lpVBpr_tile, &prep_vec_tile, NULL);
		lpSM->PrepareBuffer(&ttree_r, &lpVBpr_roof, &prep_vec_roof, NULL);
		return;
	} //!Cvet ----

	int vpos;
	int y2=(hbegin-1)*v2w;
	for(int ty=0;ty<(v2h-hend);ty++)
	{
		for(int x=wright-1;x<(v2w-wleft+1);x++)
		{
			vpos=y2+x;
			int ny=view2[vpos].cur_y;
			int nx=view2[vpos].cur_x;
			if(ny<0 || nx<0 || ny>=MAXTILEY || nx>=MAXTILEX) continue;

			if(cmn_show_tiles)
				if(hex_field[ny][nx].tile_id)
					ttree_t.insert(dtree_map::value_type(hex_field[ny][nx].pos, 
						new PrepSprite(hex_field[ny][nx].scr_x-49,hex_field[ny][nx].scr_y-3,hex_field[ny][nx].tile_id)));

			if(cmn_show_roof) //!Cvet roof
				if(hex_field[ny][nx].roof_id)
					ttree_r.insert(dtree_map::value_type(hex_field[ny][nx].pos, 
						new PrepSprite(hex_field[ny][nx].scr_x-49,hex_field[ny][nx].scr_y-3-100,hex_field[ny][nx].roof_id)));
		}
		y2+=v2w;
	}

	prep_cnt_tile=ttree_t.size();
	prep_cnt_roof=ttree_r.size(); //!Cvet
	lpSM->PrepareBuffer(&ttree_t, &lpVBpr_tile, &prep_vec_tile, TILE_ALPHA);
	lpSM->PrepareBuffer(&ttree_r, &lpVBpr_roof, &prep_vec_roof, ROOF_ALPHA); //!Cvet

	for(dtree_map::iterator jt=ttree_t.begin();jt!=ttree_t.end();jt++)
		delete (*jt).second;
	ttree_t.clear();
	for(jt=ttree_r.begin();jt!=ttree_r.end();jt++) //!Cvet
		delete (*jt).second;
	ttree_r.clear();
}

int CHexField::IsVisible(int nx, int ny,WORD id)
{
	if(!hex_field[ny][nx].to_draw) return 0; //!Cvet ���� ����� ����� ��� ���� �������...

	SpriteInfo* lpinf=lpSM->GetSpriteInfo(id);
	if(!lpinf) return 0;
	int chx=hex_field[ny][nx].scr_x+16-(lpinf->w >> 1)+lpinf->offs_x;
	int chy=hex_field[ny][nx].scr_y+6-lpinf->h+lpinf->offs_y;

	//���� ������ �� �������� �� ����� - �� ������ ���
	if(chx+lpinf->w+32<0 || chx-32>MODE_WIDTH || chy-24>(MODE_HEIGHT-100) || chy+lpinf->h+24<0) return 0;
		else return 1;
}

void CHexField::SwitchShowHex()
{
// ������ ���� 0 �� ���������� �������������� 1 ����� � 0
	ShowHex=ShowHex?0:1;
	SetCenter2(cnt_x,cnt_y);
}

//����� �� ����� �������� � view ���������� ������
//������� ��� � ������ ������ ������������
void CHexField::InitView2(int cx,int cy)
{
	int x;
	int xa=32-((ADD_X+wright) << 5);
	int xb=16-((ADD_X+wright) << 5);
	int y=-10-12*(ADD_Y+hbegin);
	int mody;
	int modpos=cx%2;

	int y2=0;
	int vpos;

	for(int j=0;j<v2h;j++)
	{
		x=(j%2)?xa:xb;
		for(int i=0;i<v2w;i++)
		{
			vpos=y2+i;
			view2[vpos].scr_x=MODE_WIDTH-x;
			view2[vpos].scr_y=y;

			if(j>v2c_y) mody=(j-v2c_y+1)/2;
				else mody=(j-v2c_y)/2;
			int x1=i-v2c_x+mody;
			int y1=i-v2c_x+modpos-2*(j-v2c_y)+mody;
			if(y1<0) y1--;
			y1=-y1/2;

			view2[vpos].mod_x=x1;
			view2[vpos].mod_y=y1;
				
			x+=32;
		}
		y+=12;
		y2+=v2w;
	}
}

void  CHexField::DrawMap2()
{
	//tiles
	if(cmn_show_tiles)
		lpSM->DrawPrepared(lpVBpr_tile,&prep_vec_tile,prep_cnt_tile);

	//objects
	lpSM->DrawTreeCntr(&dtree);

	//roof
	if(cmn_show_roof) //!Cvet
		lpSM->DrawPrepared(lpVBpr_roof,&prep_vec_roof,prep_cnt_roof);

	lpSM->Flush();
}

int offs_arrx[]={1,2,4,8,16,32};
int offs_arry[]={1,2,3,6,12,24};

int CHexField::Scroll()
{
	if(opt_scroll_delay<0) opt_scroll_delay=0;
	static TICK LastCall=GetTickCount();
	if((GetTickCount()-LastCall)<opt_scroll_delay) return 1;
		else LastCall=GetTickCount();

	int xmod=0,ymod=0;

	if(cmn_di_mleft || cmn_di_left) xmod+=1;
	if(cmn_di_mright || cmn_di_right) xmod-=1;
	if(cmn_di_mup || cmn_di_up) ymod-=1;
	if(cmn_di_mdown || cmn_di_down) ymod+=1;

	bool both_hold=xmod && ymod;

	if(!xmod && !ymod) return 0;

	if(!opt_scroll_step) opt_scroll_step=1;
	if(opt_scroll_step>32) opt_scroll_step=32;

	int step_y=1;
	for(int i=0;i<6;i++)
		if(offs_arrx[i]==opt_scroll_step)
		{
			step_y=offs_arry[i];
			break;
		}

	//�� ����� �� ������������ ������� ��������
	cmn_scr_ox+=xmod*opt_scroll_step;
	cmn_scr_oy+=-ymod*step_y;

	//� ������ ���������� ������������ CenterView
	int set=0;

	if(cmn_scr_ox>=32)
	{
		xmod=1;
		cmn_scr_ox=0;
		set=1;
	}
	else if(cmn_scr_ox<=-32)
		{
			xmod=-1;
			cmn_scr_ox=0;
			set=1;
		}

	if(set && both_hold)
		if(cmn_scr_oy)
		{
			if(cmn_scr_oy>0) cmn_scr_oy=(cmn_scr_oy>12)?24:0;
				else cmn_scr_oy=(cmn_scr_oy<-12)?-24:0;
		}

	if(cmn_scr_oy>=24)
	{
		ymod=-2;
		cmn_scr_oy=0;
		set=1;
	}
	else if(cmn_scr_oy<=-24)
		{
			ymod=2;
			cmn_scr_oy=0;
			set=1;
		}

	if(set && both_hold)
		if(cmn_scr_ox)
		{
			if(cmn_scr_ox>0) cmn_scr_ox=(cmn_scr_ox>16)?32:0;
				else cmn_scr_ox=(cmn_scr_ox<-16)?-32:0;
		}

	if(set)
	{
	//!Cvet ++++++
		int rt_pos=(hbegin+ymod)*v2w+wright+xmod;
		int rb_pos=(VIEW_CY*2+hbegin+ymod)*v2w+wright+xmod;
		int lb_pos=(VIEW_CY*2+hbegin+ymod)*v2w+VIEW_CX*2+wright+xmod;
		int lt_pos=(hbegin+ymod)*v2w+VIEW_CX*2+wright+xmod;

		//!!!��. �� ���.

		int rt_x=view2[rt_pos].cur_x;
		int rt_y=view2[rt_pos].cur_y;
		int rb_x=view2[rb_pos].cur_x;
		int rb_y=view2[rb_pos].cur_y;
		int lb_x=view2[lb_pos].cur_x;
		int lb_y=view2[lb_pos].cur_y;
		int lt_x=view2[lt_pos].cur_x;
		int lt_y=view2[lt_pos].cur_y;

		if(hex_field[rt_y][rt_x].scroll_block) return 1;
		if(hex_field[rb_y][rb_x].scroll_block) return 1;
		if(hex_field[lb_y][lb_x].scroll_block) return 1;
		if(hex_field[lt_y][lt_x].scroll_block) return 1;

		int vpos=(VIEW_CY+hbegin+ymod)*v2w+VIEW_CX+wright+xmod;
		cnt_x=view2[vpos].cur_x;
		cnt_y=view2[vpos].cur_y;

		//if(cnt_x)

//WriteLog("�����-����	x=%d,y=%d\n",rt_x,rt_y);
//WriteLog("�����-���		x=%d,y=%d\n",rb_x,rb_y);
//WriteLog("����-���		x=%d,y=%d\n",lb_x,lb_y);
//WriteLog("����-����		x=%d,y=%d\n",lt_x,lt_y);
//WriteLog("�����			x=%d,y=%d\n",cnt_x,cnt_y);

	//!Cvet ------

		SetCenter2(cnt_x,cnt_y);
	} else RebuildTiles();

	return 1;
}

void CHexField::PreRestore()
{
	SAFEREL(lpVBpr_tile);
	SAFEREL(lpVBpr_roof); //!Cvet
}

void CHexField::PostRestore()
{
	SetCenter2(cnt_x,cnt_y);
}

void CHexField::SetCrit(int x,int y,CCritter* pcrit)
{
	hex_field[y][x].lpcrit=pcrit;
	hex_field[y][x].passed=FALSE;

//	SpriteInfo* ii=lpSM->GetSpriteInfo(hex_field[y][x].lpcrit->cur_id);
	WriteLog("x=%d,y=%d\n",x,y);

	if(!IsVisible(x, y, hex_field[y][x].lpcrit->cur_id))
	{
WriteLog("Added not visible!\n");
		hex_field[y][x].lpcrit->SetVisible(0);
		pcrit->rit=NULL;
		return;
	}
WriteLog("Added visible!\n");

	hex_field[y][x].lpcrit->SetVisible(1);

	PrepSprite* prep=new PrepSprite(hex_field[y][x].scr_x+16,hex_field[y][x].scr_y+6,0,&hex_field[y][x].lpcrit->cur_id,&hex_field[y][x].lpcrit->cur_ox,&hex_field[y][x].lpcrit->cur_oy);
	pcrit->rit=dtree.insert(dtree_map::value_type(hex_field[y][x].pos+DRAW_ORDER_CRIT, prep));

//	pcrit->hex_x=x; //!Cvet ����� � AddCritter // ������� ���������� ���� 
//	pcrit->hex_y=y;
	lpSM->GetDrawCntrRect(prep, &hex_field[y][x].lpcrit->drect);
}

void CHexField::RemoveCrit(CCritter* pcrit)
{
	//hex_field[pcrit->ry][pcrit->rx].lpcrit=NULL;
	hex_field[pcrit->hex_y][pcrit->hex_x].lpcrit=NULL;
	hex_field[pcrit->hex_y][pcrit->hex_x].passed=TRUE; //!Cvet

	if(pcrit->rit!=NULL) 
	{ //!Cvet ��������. ��� ���
		dtree_map::iterator it=dtree.find((*pcrit->rit).first);

		if(it!=dtree.end())
		{

	WriteLog("R1=");
			SAFEDEL((*it).second);
	WriteLog("R2...");
			dtree.erase(it);
		}

		pcrit->rit=NULL;

	}
    // ������� ������� ���������
    WriteLog("CritRemoved...\n");
}

//!Cvet +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*void CHexField::DrawTiles()
{

}

void CHexField::DrawTiles()
{

}
*/
void CHexField::TransitCritter(CCritter* pcrit, int dir, int x, int y, bool null_offs)
{
	if(null_offs==true) SetMoveOffs(pcrit,-1);

	if(hex_field[y][x].lpcrit!=NULL) return;

	hex_field[pcrit->hex_y][pcrit->hex_x].passed=TRUE;
	hex_field[y][x].passed=FALSE;

	hex_field[y][x].lpcrit=pcrit;
	hex_field[pcrit->hex_y][pcrit->hex_x].lpcrit=NULL;

	pcrit->hex_x=x;
	pcrit->hex_y=y;
	pcrit->cur_dir=dir;

	if(pcrit->rit!=NULL) 
	{ //!Cvet ��������. ��� ���
		dtree_map::iterator it=dtree.find((*pcrit->rit).first);

		if(it!=dtree.end())
		{
	WriteLog("R1=");
			SAFEDEL((*it).second);
	WriteLog("R2...");
			dtree.erase(it);
		}

		pcrit->rit=NULL;
	}
		
	if(!IsVisible(x, y, hex_field[y][x].lpcrit->cur_id))
	{
		pcrit->SetVisible(0);
		return;
	}

	pcrit->SetVisible(1);

	PrepSprite* prep=new PrepSprite(hex_field[y][x].scr_x+16,hex_field[y][x].scr_y+6,0,&hex_field[y][x].lpcrit->cur_id,&hex_field[y][x].lpcrit->cur_ox,&hex_field[y][x].lpcrit->cur_oy);
	pcrit->rit=dtree.insert(dtree_map::value_type(hex_field[y][x].pos+DRAW_ORDER_CRIT, prep));

	lpSM->GetDrawCntrRect(prep, &pcrit->drect);
}

void CHexField::MoveCritter(CCritter* pcrit, int move_dir, int new_x, int new_y)
{
	if(new_x<0 || new_x>=MAXTILEX || new_y<0 || new_y>=MAXTILEY) return;

	if(move_dir<0 || move_dir>5) move_dir=GetDir(pcrit->hex_x,pcrit->hex_y,new_x,new_y);

	SetMoveOffs(pcrit,move_dir);

	TransitCritter(pcrit,move_dir,new_x,new_y,false);
}

void CHexField::MoveCritter(CCritter* pcrit, int new_x, int new_y)
{
	if(new_x<0 || new_x>=MAXTILEX || new_y<0 || new_x>=MAXTILEY) return;

	int move_dir=GetDir(pcrit->hex_x,pcrit->hex_y,new_x,new_y);

	SetMoveOffs(pcrit,move_dir);

	TransitCritter(pcrit,move_dir,new_x,new_y,false);
}

void CHexField::MoveCritter(CCritter* pcrit, int move_dir)
{
	if(move_dir<0 || move_dir>5) return;

	int cur_x=pcrit->hex_x;
	int cur_y=pcrit->hex_y;

	switch(move_dir)
	{
	case 0:
		cur_x--;
		if(!(cur_x%2)) cur_y--;
		break;
	case 1:
		cur_x--;
		if(cur_x%2) cur_y++;
		break;
	case 2:
		cur_y++;
		break;
	case 3:
		cur_x++;
		if(cur_x%2) cur_y++;
		break;
	case 4:
		cur_x++;
		if(!(cur_x%2)) cur_y--;
		break;
	case 5:
		cur_y--;
		break;
	default:
		return;
	}

	SetMoveOffs(pcrit,move_dir);

	TransitCritter(pcrit,move_dir,cur_x,cur_y,false);
}

int CHexField::GetDir(int old_x, int old_y, int new_x, int new_y)
{
	int dir=0;

	if(old_x%2)
	{
		if(old_x< new_x && old_y< new_y) dir=0;
		if(old_x< new_x && old_y==new_y) dir=1;
		if(old_x==new_x && old_y< new_y) dir=2;
		if(old_x< new_x && old_y==new_y) dir=3;
		if(old_x< new_x && old_y> new_y) dir=4;
		if(old_x==new_x && old_y> new_y) dir=5;
	}
	else
	{
		if(old_x> new_x && old_y==new_y) dir=0;
		if(old_x> new_x && old_y> new_y) dir=1;
		if(old_x==new_x && old_y< new_y) dir=2;
		if(old_x< new_x && old_y< new_y) dir=3;
		if(old_x< new_x && old_y==new_y) dir=4;
		if(old_x==new_x && old_y> new_y) dir=5;
	}

	return dir;
}

void CHexField::SetMoveOffs(CCritter* pcrit,int dir)
{
	switch(dir)
	{
	case 0:
		pcrit->cur_ox=-16;
		pcrit->cur_oy=16;
		break;
	case 1:
		pcrit->cur_ox=-32;
		pcrit->cur_oy=0;
		break;
	case 2:
		pcrit->cur_ox=-16;
		pcrit->cur_oy=-16;
		break;
	case 3:
		pcrit->cur_ox=16;
		pcrit->cur_oy=-16;
		break;
	case 4:
		pcrit->cur_ox=32;
		pcrit->cur_oy=0;
		break;
	case 5:
		pcrit->cur_ox=16;
		pcrit->cur_oy=16;
		break;
	default:
		pcrit->cur_ox=0;
		pcrit->cur_oy=0;
		break;
	}
}

int CHexField::UnLoadMap()
{
	if(MapLoaded==FALSE) return 1;

	SAFEREL(lpVBpr_tile);
	SAFEREL(lpVBpr_roof);
	SAFEDELA(view2);

	lpVBpr_tile=NULL;
	lpVBpr_roof=NULL;
	view2=NULL;

	hbegin=0;
	hend=0;
	wright=0;
	wleft=0;

	v2h=0;
	v2w=0;
	v2c_x=0;
	v2c_y=0;

	cnt_x=0;
	cnt_y=0;
/*
	loaded_item.clear();
	loaded_scen.clear();
	loaded_wall.clear();
	loaded_tile.clear();
*/
	for(dtree_map::iterator jt=dtree.begin();jt!=dtree.end();jt++)
		SAFEDEL((*jt).second);
	dtree.clear();

	for(int x=0;x<MAXTILEX;x++)
		for(int y=0;y<MAXTILEY;y++)
		{
			for(scen_vect::iterator si=hex_field[y][x].sc_obj.begin();si!=hex_field[y][x].sc_obj.end();si++)
				delete (*si);
			hex_field[y][x].sc_obj.clear();

			for(item_vect::iterator ti=hex_field[y][x].itm_obj.begin();ti!=hex_field[y][x].itm_obj.end();ti++)
				delete (*ti);
			hex_field[y][x].itm_obj.clear();

	//		for(misc_vect::iterator mi=hex_field[y][x].msc_obj.begin();mi!=hex_field[y][x].msc_obj.end();mi++)
	//			delete (*mi);
	//		hex_field[y][x].msc_obj.clear();

			hex_field[y][x].lpcrit=NULL;

			hex_field[y][x].scr_x;
			hex_field[y][x].scr_y;
			hex_field[y][x].tile_id=0;
			hex_field[y][x].roof_id=0;
			hex_field[y][x].to_draw=0;
			hex_field[y][x].passed=TRUE;
			hex_field[y][x].raked=TRUE;
			for(int i=0;i<MAX_WALL_CNT;i++) hex_field[y][x].wall_id[i]=0;
		}

	MapLoaded=FALSE;

	WriteLog("UnloadMap...OK\n");

	return 1;
}

int CHexField::GetTilePixel(int pix_x, int pix_y, HexTYPE* tile_x, HexTYPE* tile_y)
{
	int y2=0;
	int vpos=0;

	for(int ty=0;ty<v2h;ty++)
	{
		for(int tx=0;tx<v2w;tx++)
		{
			vpos=y2+tx;

			int hx=view2[vpos].scr_x;
			int hy=view2[vpos].scr_y;

			if(pix_x<=hx) continue;
			if(pix_x>hx+32) continue;
			if(pix_y<=hy) continue;
			if(pix_y>hy+16) continue;

			*tile_x=view2[vpos].cur_x;
			*tile_y=view2[vpos].cur_y;

			return 1;
		}
		y2+=v2w;
	}

	return 0;
}

int CHexField::FindStep(HexTYPE start_x, HexTYPE start_y, HexTYPE end_x, HexTYPE end_y)
{
	int FindPath[MAXTILEX][MAXTILEY];
	int numindex=1; //������� ������
	bool indexing; // �������� ���� ���� ���� ���� ����� ���� ��������
	int x1=0;
	int y1=0;
	int tx1=start_x;
	int ty1=start_y;
	int tx2=end_x;
	int ty2=end_y;

	//��������� �������
	if(tx1<0 || ty1>=MAXTILEX || tx2<0 || ty2>=MAXTILEY) return FINDPATH_ERROR;

	if(tx1==tx2 && ty1==ty2) return FINDPATH_ALREADY_HERE;

	if(!hex_field[ty2][tx2].passed) return FINDPATH_DEADLOCK;

	//������� ������ ��� ���� 
	for(x1=0; x1 < MAXTILEX; x1++)
		for(y1=0; y1 < MAXTILEY; y1++) FindPath[x1][y1]=0;
	//�� ��� ����� ��� ����� ��� ������ ����� 1, �� ���� � ����� �������������
	FindPath[tx1][ty1]=numindex;
	//������� ���� - �������� ��� �� ����� ��������� ������, ���������� ������ ���
	while(true)
	{
		//���������� ������, ��� � ������ ���������� �� �� � ����� �����
		indexing=false;
		//����������� ����� �������� �����
		for(x1=1; x1 < MAXTILEX-1; x1++)
		{
			for(y1=1; y1 < MAXTILEY-1; y1++)
			{
				if(x1==tx2 && y1==ty2 && FindPath[x1][y1]==numindex) break; //1)����� ����� ���������� - �������

				if(FindPath[x1][y1]==numindex)
				{
				//� ������ ����� �� ���� ������������ � ������� ���������
					if(hex_field[y1-1][x1].passed && !FindPath[x1][y1-1]) { FindPath[x1][y1-1]=numindex+1; indexing=true; }
					if(hex_field[y1][x1+1].passed && !FindPath[x1+1][y1]) { FindPath[x1+1][y1]=numindex+1; indexing=true; }
					if(hex_field[y1+1][x1].passed && !FindPath[x1][y1+1]) { FindPath[x1][y1+1]=numindex+1; indexing=true; }
					if(hex_field[y1][x1-1].passed && !FindPath[x1-1][y1]) { FindPath[x1-1][y1]=numindex+1; indexing=true; }
				//����� ���� �������� �� ���-����� �� ��� �
					if(x1%2!=0) //���
					{
						if(hex_field[y1-1][x1-1].passed && !FindPath[x1-1][y1-1]) { FindPath[x1-1][y1-1]=numindex+1; indexing=true; }
						if(hex_field[y1-1][x1+1].passed && !FindPath[x1+1][y1-1]) { FindPath[x1+1][y1-1]=numindex+1; indexing=true; }
					}
					if(x1%2==0) //�����
					{
						if(hex_field[y1+1][x1+1].passed && !FindPath[x1+1][y1+1]) { FindPath[x1+1][y1+1]=numindex+1; indexing=true; }
						if(hex_field[y1+1][x1-1].passed && !FindPath[x1-1][y1+1]) { FindPath[x1-1][y1+1]=numindex+1; indexing=true; }
					}
				}
			}
			if(x1==tx2 && y1==ty2 && FindPath[x1][y1]==numindex) break; //2)���������� ��������
		}
		if(x1==tx2 && y1==ty2 && FindPath[x1][y1]==numindex) break; //3)� ��������� ���
		//���������� ������, ��� � ������ ���������� �� �� � ����� �����
		if(indexing) numindex++;
		else return FINDPATH_DEADLOCK; //������ �� ���������� - ������ �����

		if(numindex-1 > FINDPATH_MAX_PATH) return FINDPATH_TOOFAR;
	}

	//���� � ����� � ������
	bool switcher=false;
	while(numindex>2)
	{
		switcher=!switcher;

		numindex--;

		//���-�����
		if(switcher==true)
		{
			if(FindPath[x1][y1-1]==numindex) { y1--; continue; }
			if(FindPath[x1+1][y1]==numindex) { x1++; continue; }
			if(FindPath[x1][y1+1]==numindex) { y1++; continue; }
			if(FindPath[x1-1][y1]==numindex) { x1--; continue; }

			if(x1%2!=0) //���
			{
				if(FindPath[x1-1][y1-1]==numindex) { x1--; y1--; continue; }

				if(FindPath[x1+1][y1-1]==numindex) { x1++; y1--; continue; }
			}
			if(x1%2==0) //�����
			{
				if(FindPath[x1+1][y1+1]==numindex) { x1++; y1++; continue; }

				if(FindPath[x1-1][y1+1]==numindex) { x1--; y1++; continue; }
			}
		}
		else
		{
			if(x1%2!=0) //���
			{
				if(FindPath[x1-1][y1-1]==numindex) { x1--; y1--; continue; }

				if(FindPath[x1+1][y1-1]==numindex) { x1++; y1--; continue; }
			}
			if(x1%2==0) //�����
			{
				if(FindPath[x1+1][y1+1]==numindex) { x1++; y1++; continue; }

				if(FindPath[x1-1][y1+1]==numindex) { x1--; y1++; continue; }
			}

			if(FindPath[x1][y1-1]==numindex) { y1--; continue; }
			if(FindPath[x1+1][y1]==numindex) { x1++; continue; }
			if(FindPath[x1][y1+1]==numindex) { y1++; continue; }
			if(FindPath[x1-1][y1]==numindex) { x1--; continue; }
		}
	}

	//���� � ���� � ������ 6 �������
	if(tx1%2==0)
	{
		x1=x1-tx1;
		y1=y1-ty1;
		if(x1==-1 && y1== 0) return 0;
		if(x1==-1 && y1==+1) return 1;
		if(x1== 0 && y1==+1) return 2;
		if(x1==+1 && y1==+1) return 3;
		if(x1==+1 && y1== 0) return 4;
		if(x1== 0 && y1==-1) return 5;
	}
	//���� � ���� � ������� 6 �������
	if(tx1%2!=0)
	{
		x1=x1-tx1;
		y1=y1-ty1;
		if(x1==-1 && y1==-1) return 0;
		if(x1==-1 && y1== 0) return 1;
		if(x1== 0 && y1==+1) return 2;
		if(x1==+1 && y1== 0) return 3;
		if(x1==+1 && y1==-1) return 4;
		if(x1== 0 && y1==-1) return 5;
	}
	//�� ���� �� ������ �������� :)
	return FINDPATH_ERROR;
} 

int CHexField::FindTarget(HexTYPE start_x, HexTYPE start_y, HexTYPE end_x, HexTYPE end_y, BYTE max_weapon_distance)
{
	int FindPath[MAXTILEX][MAXTILEY];
	int numindex=1; //������� ������
	int x1=0;
	int y1=0;
	int tx1=start_x;
	int ty1=start_y;
	int tx2=end_x;
	int ty2=end_y;

	//��������� �������
	if(tx1<0 || ty1>=MAXTILEX || tx2<0 || ty2>=MAXTILEY) return FINDTARGET_ERROR;

	if(tx1==tx2 && ty1==ty2) return FINDTARGET_INVALID_TARG;

	//������� ������ ��� ���� 
	for(x1=0; x1<MAXTILEX; x1++)
		for(y1=0; y1<MAXTILEY; y1++) FindPath[x1][y1]=0;

	//�� ��� ����� ��� ����� ��� ������ ����� 1, �� ���� � ����� �������������
	FindPath[tx1][ty1]=numindex;

	//������� ���� - �������� ��� �� ����� ��������� ������, ���������� ������ ���
	while(true)
	{
		//����������� ����� �������� �����
		for(x1=1; x1 < MAXTILEX-1; x1++)
		{
			for(y1=1; y1 < MAXTILEY-1; y1++)
			{
				if(x1==tx2 && y1==ty2 && FindPath[x1][y1]==numindex) break; //1)����� ����� ���������� - �������
				if(FindPath[x1][y1]==numindex)
				{
					//� ������ ����� �� ���� ������������ � ������� ���������
					if(!FindPath[x1][y1-1]) { FindPath[x1][y1-1]=numindex+1; }
					if(!FindPath[x1+1][y1]) { FindPath[x1+1][y1]=numindex+1; }
					if(!FindPath[x1][y1+1]) { FindPath[x1][y1+1]=numindex+1; }
					if(!FindPath[x1-1][y1]) { FindPath[x1-1][y1]=numindex+1; }
					//����� ���� �������� �� ���-����� �� ��� �
					if(x1%2!=0) //���
					{
						if(!FindPath[x1-1][y1-1]) { FindPath[x1-1][y1-1]=numindex+1; }
						if(!FindPath[x1+1][y1-1]) { FindPath[x1+1][y1-1]=numindex+1; }
					}
					if(x1%2==0) //�����
					{
						if(!FindPath[x1+1][y1+1]) { FindPath[x1+1][y1+1]=numindex+1; }
						if(!FindPath[x1-1][y1+1]) { FindPath[x1-1][y1+1]=numindex+1; }
					}
				}
			}
			if(x1==tx2 && y1==ty2 && FindPath[x1][y1]==numindex) break; //2)���������� ��������
		}
		if(x1==tx2 && y1==ty2 && FindPath[x1][y1]==numindex) break; //3)� ��������� ���
		//����������� ������
		numindex++;
		//��������� ���������
		if(numindex-1 > max_weapon_distance) return FINDTARGET_TOOFAR; //������� ��������� ������
		//a_obj->object->p[rate_object*30+100+5]
	}

	//���� � ����� � ������
	while(numindex>2)
	{
		numindex--;
		//���-�����
		if(x1%2!=0) //���
		{
			if(FindPath[x1-1][y1-1]==numindex && hex_field[y1-1][x1-1].raked) { x1--; y1--; continue; }
			if(FindPath[x1+1][y1-1]==numindex && hex_field[y1-1][x1+1].raked) { x1++; y1--; continue; }
		}
		if(x1%2==0) //�����
		{
			if(FindPath[x1+1][y1+1]==numindex && hex_field[y1+1][x1+1].raked) { x1++; y1++; continue; }
			if(FindPath[x1-1][y1+1]==numindex && hex_field[y1+1][x1-1].raked) { x1--; y1++; continue; }
		}

		if(FindPath[x1][y1-1]==numindex && hex_field[y1-1][x1].raked) { y1--; continue; }
		if(FindPath[x1+1][y1]==numindex && hex_field[y1][x1+1].raked) { x1++; continue; }
		if(FindPath[x1][y1+1]==numindex && hex_field[y1+1][x1].raked) { y1++; continue; }
		if(FindPath[x1-1][y1]==numindex && hex_field[y1][x1-1].raked) { x1--; continue; }

		return FINDTARGET_BARRIER; //������� �������� �� ���� ��������
	}

	//���� � ���� � ������ 6 �������
	if(tx1%2==0)
	{
		x1=x1-tx1;
		y1=y1-ty1;
		if(x1==-1 && y1== 0) return 0;
		if(x1==-1 && y1==+1) return 1;
		if(x1== 0 && y1==+1) return 2;
		if(x1==+1 && y1==+1) return 3;
		if(x1==+1 && y1== 0) return 4;
		if(x1== 0 && y1==-1) return 5;
	}
	//���� � ���� � ������� 6 �������
	if(tx1%2!=0)
	{
		x1=x1-tx1;
		y1=y1-ty1;
		if(x1==-1 && y1==-1) return 0;
		if(x1==-1 && y1== 0) return 1;
		if(x1== 0 && y1==+1) return 2;
		if(x1==+1 && y1== 0) return 3;
		if(x1==+1 && y1==-1) return 4;
		if(x1== 0 && y1==-1) return 5;
	}
	//�� ���� �� ������ ��������
	return FINDTARGET_ERROR;
}

//!Cvet ---------------------------------------------------------------------------------