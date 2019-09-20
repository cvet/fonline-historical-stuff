#ifndef _CHEXFIELD_H_
#define _CHEXFIELD_H_

#include "CSpriteManager.h"
#include "Critter.h"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

//!Cvet +++++++++++++++++++++
#define MAXTILEX 400
#define MAXTILEY 400

const BYTE HEX_IMPASSABLE			=1;
const BYTE HEX_IMPASSABLE_NOT_RAKED	=2;

//поиск пути
//максимальный путь
const BYTE FINDPATH_MAX_PATH		=50;
//возвращаемые ошибки
const BYTE FINDPATH_DEADLOCK		=10;
const BYTE FINDPATH_ERROR			=11;
const BYTE FINDPATH_TOOFAR			=12;
const BYTE FINDPATH_ALREADY_HERE	=13;

//поиск цели
const BYTE FINDTARGET_BARRIER		=10;
const BYTE FINDTARGET_TOOFAR		=11;
const BYTE FINDTARGET_ERROR			=12;
const BYTE FINDTARGET_INVALID_TARG	=13;

//прозрачность
#define TILE_ALPHA	255
#define ROOF_ALPHA	opt_roof_alpha
//!Cvet ----------------------

#define ADD_X 2
#define ADD_Y 2

// Область видимости 
//#define VIEW_WIDTH  (opt_screen_mode?35:27)
//#define VIEW_HEIGHT (opt_screen_mode?65:51)
//#define VIEW_CX (opt_screen_mode?16:14)
//#define VIEW_CY (opt_screen_mode?34:26)
//!Cvet
#define VIEW_WIDTH  (view_width[opt_screen_mode])
#define VIEW_HEIGHT (view_height[opt_screen_mode])
#define VIEW_CX (view_cx[opt_screen_mode])
#define VIEW_CY (view_cy[opt_screen_mode])

#define MAX_WALL_CNT 3 // флаг стен

struct ViewField
{
	ViewField():mod_x(0),mod_y(0),cur_x(0),cur_y(0),scr_x(0),scr_y(0){};

	int mod_x, mod_y;
	int cur_x, cur_y;
	int scr_x, scr_y;
};

struct ScenObj
{
	WORD spr_id;

	ScenObj(): spr_id(0){};
	ScenObj(WORD _spr_id): spr_id(_spr_id){};
};

typedef vector<ScenObj*> scen_vect;

struct ItemObj
{
	WORD spr_id;

	ItemObj(): spr_id(0){};
	ItemObj(WORD _spr_id): spr_id(_spr_id){};
};

typedef vector<ItemObj*> item_vect;
/*
struct MiscObj //!Cvet
{
	WORD spr_id;

	MiscObj(): spr_id(0){};
	MiscObj(WORD _spr_id): spr_id(_spr_id){};
};

typedef vector<MiscObj*> misc_vect;
*/
// структура поля для рисования
struct Field
{
	CCritter* lpcrit; //криттер

	int to_draw;

	int scr_x, scr_y;

	WORD tile_id; //тайл земли
	WORD roof_id; //тайл крыши
	WORD wall_id[MAX_WALL_CNT]; //стены
	scen_vect sc_obj; //сценери
	item_vect itm_obj; //итемы
//	misc_vect msc_obj; //!Cvet другое

	DWORD pos;

	BOOL passed; //!Cvet - проходимый тайл
	BOOL raked; //!Cvet - простреливаемый тайл
	BOOL scroll_block; //!Cvet - скролл-локер	

	Field(): scr_x(0),scr_y(0),tile_id(0),roof_id(0),lpcrit(NULL),to_draw(0),passed(TRUE),
		raked(TRUE),scroll_block(FALSE) {for(int i=0;i<MAX_WALL_CNT;i++) wall_id[i]=0;}
};


typedef map<WORD, char*, less<WORD> > char_map;
typedef map<WORD, WORD, less<WORD> > word_map;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

class CHexField
{
public:
//!Cvet++++++++++++++++++++
	
	Field hex_field[MAXTILEX][MAXTILEY];

	int FindStep(HexTYPE start_x, HexTYPE start_y, HexTYPE end_x, HexTYPE end_y);
	int FindTarget(HexTYPE start_x, HexTYPE start_y, HexTYPE end_x, HexTYPE end_y, BYTE max_weapon_distance);

//загрузка карт
	BOOL MapLoaded;
	BOOL IsMapLoaded() { if(MapLoaded==TRUE) return 1; return 0; };

//добавление/удаление объектов по ходу игры
	int AddItemObj(DWORD pic_id, HexTYPE x, HexTYPE y);
	void DelItemObj(DWORD pic_id, HexTYPE x, HexTYPE y);
//!Cvet--------------------


	int Init(CSpriteManager* lpsm);
	void Clear();

	void PreRestore();
	void PostRestore();
 
	int LoadMap(char* fname); //загрузка карты
	int UnLoadMap(); //!Cvet

	void SetCenter2(int x, int y);

	void OnChangeCol(){RebuildTiles();};

	void DrawMap2();

	int Scroll();

	void SwitchShowHex();

	void SetCrit(int x,int y,CCritter* pcrit);

//!Cvet +++++
	void TransitCritter(CCritter* pcrit, int dir, int x, int y, bool null_offs);

	void MoveCritter(CCritter* pcrit, int move_dir, int new_x, int new_y);
	void MoveCritter(CCritter* pcrit, int new_x, int new_y);
	void MoveCritter(CCritter* pcrit, int move_dir);

	int GetDir(int old_x, int old_y, int new_x, int new_y);
	void SetMoveOffs(CCritter* pcrit,int dir);
//!Cvet -----

	void RemoveCrit(CCritter* pcrit);

	void RebuildTiles(); //!Cvet вынес из привата

	int GetTilePixel(int pix_x, int pix_y, HexTYPE* tile_x, HexTYPE* tile_y);
	
	CHexField();
private:
	bool crtd;

	bool ShowHex;
	WORD hex,hexb;

	int cnt_x,cnt_y;

//tiles
	LPDIRECT3DVERTEXBUFFER8 lpVBpr_tile; //подгототвленный буфер
	onesurf_vec prep_vec_tile; //указывается очередность смен текстур при прорисовке буфера
	WORD prep_cnt_tile;
//!Cvet roof
	LPDIRECT3DVERTEXBUFFER8 lpVBpr_roof;
	onesurf_vec prep_vec_roof;
	WORD prep_cnt_roof;

	ViewField* view2;

	char_map item_proto; //!Cvet
	char_map scen_proto;

	word_map loaded_item; //!Cvet
	word_map loaded_scen;
	word_map loaded_wall;
	word_map loaded_tile;
//	word_map loaded_misc; //!Cvet

	char_map item_fnames; //!Cvet
	char_map scen_fnames;
	char_map wall_fnames;
	char_map tile_fnames;
//	char_map misc_fnames; //!Cvet
	
	dtree_map dtree; //дерево порядка отрисовки объектов.

	CFileMngr fm;
	//когда fm загружает файл, он стирает кэш старого, а при загрузке карты он должен оставаться.
	CFileMngr fm_map; 

	CSpriteManager* lpSM;

	int LoadList(char* lname,int PathType,char_map* pmap);
	int DropScript();
	int LoadObj();

	int hbegin;
	int hend;
	int wright;
	int wleft;

	int v2h,v2w,v2c_x,v2c_y;

	void InitView2(int cx,int cy); // Инициализация области видимости

	int ParseItemObj(DWORD proto_id,DWORD id,DWORD x,DWORD y,WORD* hbmax,WORD* hemax,WORD* wrmax,WORD* wlmax); //!Cvet
	int ParseItemObjCont(DWORD proto_id,DWORD x,DWORD y); //!Cvet
	int ParseMiscObj(DWORD proto_id,DWORD id,DWORD x,DWORD y,WORD* hbmax,WORD* hemax,WORD* wrmax,WORD* wlmax); //!Cvet
	int ParseScenObj(DWORD proto_id,DWORD id,DWORD x,DWORD y,WORD* hbmax,WORD* hemax,WORD* wrmax,WORD* wlmax);
	int ParseWallObj(DWORD proto_id,DWORD id,DWORD x,DWORD y,WORD* hbmax,WORD* hemax,WORD* wrmax,WORD* wlmax);

	int IsVisible(int nx, int ny,WORD id);

};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
#endif
