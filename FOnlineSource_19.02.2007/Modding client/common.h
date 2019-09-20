#ifndef _COMMON_H_
#define _COMMON_H_

#define CFG_FILE ".\\FOnline.cfg"

//!Cvet ++++++
#define CFG_INT_FILE ".\\iface.cfg"
#define PATH_MAP_FLAGS ".\\data\\maps\\"
#define PATH_TEXT_FILES ".\\data\\text\\"
//!Cvet ------

#include "netproto.h"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
extern bool cmn_Quit;
extern bool cmn_lost;

extern int cmn_scr_ox;
extern int cmn_scr_oy;

extern bool cmn_show_tiles;
extern bool cmn_show_roof;
extern bool cmn_show_items;
extern bool cmn_show_scen;
extern bool cmn_show_walls;
extern bool cmn_show_crit;
extern bool cmn_show_misc; //!Cvet
extern bool cmn_show_onlys;

//holded keys
extern bool cmn_di_left;
extern bool cmn_di_right;
extern bool cmn_di_up;
extern bool cmn_di_down;

extern bool cmn_di_mleft;
extern bool cmn_di_mright;
extern bool cmn_di_mup;
extern bool cmn_di_mdown;

// console test
extern bool cmn_console;
extern bool newplayer; // ��� ��������� ����� �����
extern bool cmn_show_iface;
// ��� ��������� ������ ������� ��������� �����
extern int move;

extern int ac1,ac2,ac3,ac4,ac5,ac6,ac7,acres;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

extern int opt_fullscr;
extern int opt_vsync;

extern int opt_flushval;
extern int opt_basetex;

extern int opt_dbgclear;
extern int opt_light;

extern int opt_scroll_delay;
extern int opt_scroll_step;

extern int opt_mouse_speed;

extern char opt_masterpath[1024];
extern char opt_critterpath[1024];
extern char opt_fopath[1024];
extern char opt_crfol[1024];

extern DWORD opt_rport;
extern char opt_rhost[1024];
extern char opt_name[MAX_NAME+1];
extern char opt_cases[5][MAX_NAME+1];
extern char opt_gender[10];

extern DWORD opt_text_delay;

extern DWORD opt_screen_mode;
extern int screen_width[]; //!Cvet
extern int screen_height[]; //!Cvet
extern int view_width[]; //!Cvet
extern int view_height[]; //!Cvet
extern int view_cx[]; //!Cvet
extern int view_cy[]; //!Cvet

extern int opt_sleep; //!Cvet sleep
extern BOOL opt_msgbox_invert; //!Cvet message box invert

extern int opt_change_lang; //!Cvet change language
#define CHANGE_LANG_CTRL_SHIFT	0 //!Cvet change language
#define CHANGE_LANG_ALT_SHIFT	1 //!Cvet change language
#define CHANGE_LANG_RCTRL		2 //!Cvet change language

//!Cvet colors ++++++++++++++++++++++++
extern DWORD opt_gcolor_default;

extern DWORD opt_tcolor_namecrit;

extern DWORD opt_tcolor_default;
extern DWORD opt_tcolor_shout;
extern DWORD opt_tcolor_whisp;
extern DWORD opt_tcolor_emote;
extern DWORD opt_tcolor_social;


extern DWORD opt_roof_alpha;
//!Cvet ------------------------------

void GetOptions();

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

int StartLogFile();
void CloseLogFile();
void WriteLog(char* frmt, ...);

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

void ErrMsg(char* hdr, char* frmt, ...); //����� ��������� �� ������
void InfoMsg(char* frmt, ...);

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

#define OBJ_ITEM  0
#define OBJ_CRIT  1
#define OBJ_SCEN  2
#define OBJ_WALL  3
#define OBJ_TILE  4
#define OBJ_MISC  5
#define OBJ_IFACE 6
#define OBJ_INV   7
#define OBJ_HEADS 8
#define OBJ_BACK  9

#define SUB_SCEN_PORTAL 0
#define SUB_SCEN_STAIR  1
#define SUB_SCEN_ELEV   2
#define SUB_SCEN_LADDWN 3
#define SUB_SCEN_LADTOP 4
#define SUB_SCEN_MISC   5

//!Cvet ++++++++++++++++++
#define SUB_ITEM_ARMOR	0
#define SUB_ITEM_CONT	1
#define SUB_ITEM_DRUG   2
#define SUB_ITEM_WEAPON	3
#define SUB_ITEM_AMMO	4
#define SUB_ITEM_MISC	5
#define SUB_ITEM_KEY	6

#define SUB_MISC_EXITGR	0
#define SUB_MISC_MISC	1
//!Cvet ------------------

// �������� ��� ����������
#define engage 0
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

#endif

