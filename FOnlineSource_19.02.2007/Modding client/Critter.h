#ifndef __CRITTER_H__
#define __CRITTER_H__

/********************************************************************
	created:	21:08:2004   20:06

	author:		Oleg Mareskin
	
	purpose:	
*********************************************************************/
#include "CSpriteManager.h"
#include "CFont.h"
#include "netproto.h"
#include "BufMngr.h"

const BYTE FIRST_FRAME	=0;
const BYTE LAST_FRAME	=200;

const BYTE ANIM1_STAY	=1;

const int PAUSE			=400;

class CCritter
{
public:
	void SetDir(BYTE dir);
	void RotCW();
	void RotCCW();

	void Animate(BYTE action, BYTE num_frame); //!Cvet �������� ������ � �������� weapon
	void SetAnimation(); //!Cvet
	void Action(Byte action, WORD action_tick); //!Cvet

	void Process(); //!Cvet CBufMngr ��� ��������� ���� ����� ��� �������� � �����

	DWORD text_color; //!Cvet
	void SetText(char* str, DWORD color); //!Cvet DWORD text_color
	void SetName(char* str) {strncpy(name,str,31);name[30]=0;};
	void DrawText(CFOFont* lpfnt);
	void SetVisible(bool av) {visible=av;};
    
    // �������� ���������� ������ � �������� �����������
	WORD cur_id,miniplayer;
	short cur_ox;
	short cur_oy;
    // ���������� ���� � ������ Den Baster
    HexTYPE hex_x;
	HexTYPE hex_y;
	BYTE cur_dir; // �����������

	CrID id;
// ������� �� �������
	BYTE weapon; // ��� ������ � ����� ��� �������� Den Baster !Cvet (������������� �����)

	char name[MAX_NAME+1];
	char cases[5][MAX_NAME+1];
	
	bool human; //!Cvet �������� �� ����� ������� 0-��� 1-��
	WORD st[ALL_STATS]; //!Cvet ����� 4-� ������� XXXX
	WORD sk[ALL_SKILLS]; //!Cvet ����� 3-� ������� XXX
	BYTE pe[ALL_PERKS]; //!Cvet ����� 1-� ������� X

	BYTE cond; //!Cvet
	BYTE cond_ext; //!Cvet
	WORD flags; //!Cvet

	CrTYPE base_type; //!Cvet
	CrTYPE type;
	CrTYPE type_ext; //!Cvet

//!Cvet ++++++++++++++++++++++++++++++++++  ���������
	dyn_map obj; //��� ������� � ������
	dyn_obj* a_obj; //�������� ������ � ����1
	dyn_obj* a_obj2; //�������� ������ � ����2
	dyn_obj* a_obj_arm; //�������� ������ � ����� �����
	dyn_obj* m_obj; //mouse object

	void AddObject(BYTE a_slot,DWORD a_id,WORD a_holder,BYTE a_id_bull,WORD a_holder_ext,BYTE a_id_bull_ext,BYTE a_broken,TICK a_tick,stat_obj* a_s_obj);
	int GetMaxDistance();

	void Initialization(); //������������� ��������� ���������

	void RefreshWeap();
	void RefreshType();

	int Move(BYTE dir, WORD after_move_pause); // �������� ��������

	void SetCur_offs(int set_ox, int set_oy);
	void ChangeCur_offs(int change_ox, int change_oy);

	BYTE move_type; //0- 1-������

	BYTE rate_object; //��� ������������� �������

	dyn_obj def_obj1;
	dyn_obj def_obj2;
//!Cvet ----------------------------------

	INTRECT drect;
	
	dtree_map::iterator rit; // ������ �� �������� ����� ����� ������ �����
//	BYTE rx,ry; // x,y ���������� ������ � ������

	CCritter(CSpriteManager* alpSM):lpSM(alpSM),cur_anim(NULL),cur_dir(0),cur_id(0),stay_wait(0),stay_tkr(0),text_str(NULL),visible(0),weapon(0){strcpy(name,"none");};
	~CCritter(){SAFEDELA(text_str);};

//!Cvet ++++++++++++++++++++++++++++
	int Tick_count; //����������������� ��������
	TICK Tick_start; //����� ������ ��������

	void Tick_Start(TICK tick_count) { Tick_count=tick_count; Tick_start=GetTickCount(); };
	void Tick_Null(){ Tick_count=0; };
	//���������� 1 ���� ��������
	int IsFree() { if(GetTickCount()-Tick_start>=Tick_count) return 1; return 0; };

	BYTE cnt_per_turn;
	WORD ticks_per_turn;
//!Cvet ----------------------------
	
private:

	CSpriteManager* lpSM;
	
	
    CritFrames* cur_anim; // ������� ��� ��������
	TICK anim_tkr;//����� ������ ��������
	WORD cur_afrm;
	TICK change_tkr;//����� �� ����� �����

	int pause,pause_cur; //!Cvet

	int stay_wait; // ��� ������� ��������

	TICK stay_tkr;

	char base_fname[256];

	char* text_str;
	TICK SetTime;
	int text_delay; //!Cvet

	bool visible;
};

//������ critters, ������� ������������
typedef map<CrID, CCritter*, less<CrID> > crit_map;

#endif//__CRITTER_H__