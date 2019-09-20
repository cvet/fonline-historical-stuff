/********************************************************************
	created:	10:02:2007   15:22;

	author:		Anton Tsvetinsky
	
	purpose:	
*********************************************************************/

#ifndef _CMAPMNGR_H_
#define _CMAPMNGR_H_

#include "CFileMngr.h"
#include "netproto.h"

#define MAX_MAPS 20
#define MAXTILEX 400
#define MAXTILEY 400

//Flags Move
//#define FM_STEP		BIN8(00000001)
//#define FM_FIRST_MOVE	BIN8(00000010)

//Flags Tile
#define FT_PASSED		BIN8(00000001)
#define FT_RAKED		BIN8(00000010)
#define FT_TRANSIT		BIN8(00000100)
#define FT_PLAYER		BIN8(00001000)
#define FT_CONTAINER	BIN8(00010000)

//Move Results
#define MR_FALSE		0
#define MR_STEP			1
#define MR_TRANSIT		2

//Transit Results
#define TR_FALSE		0
#define TR_OK			1

struct tile_info
{
	tile_info():flags(0){flags=FT_PASSED|FT_RAKED;};

	BYTE flags;

//	dyn_map obj; //карта динамических объектов на тайле
};

typedef map<WORD, string, less<WORD> > map_str_map;

typedef map<DHexTYPE, LONGLONG, less<DHexTYPE> > longlong_map;

typedef map<DHexTYPE, dyn_map*, less<DHexTYPE> > dyn_map_map; //контейнер контейнеров

//typedef set<WORD> word_set;
//typedef map<DHexTYPE, word_set, less<DHexTYPE> > word_set_map;

class CMapMngr
{
public:

	CMapMngr():active(FALSE){};
	~CMapMngr(){};

	int Init();
	void Clear();

	BOOL LoadAllMaps();

	int MoveToTile(crit_info* info, HexTYPE mx, HexTYPE my);
	int TransitToTile(crit_info* info, WORD tmap, HexTYPE tx, HexTYPE ty);

	void InsertObjOnMap(dyn_obj* ins_obj, MapTYPE omap, HexTYPE ox, HexTYPE oy);
	void EraseObjFromMap(dyn_obj* ers_obj);
//	void EraseObject(dyn_obj* ers_obj, dyn_map::iterator* it_obj);

	void ClearPlayer(crit_info* info);

	BOOL IsActive(){return active;};

	dyn_map_map objects_map[MAX_MAPS];

private:

	LONGLONG FindTransit(WORD num_map, HexTYPE num_x, HexTYPE num_y);

	map_str_map map_str;

	longlong_map transit_map[MAX_MAPS];

//	word_set_map pic_on_ground[MAX_MAPS];

	CFileMngr fm;

	BOOL active;

	tile_info tile[MAX_MAPS][MAXTILEX][MAXTILEY];
};

#endif //_CMAPMNGR_H_