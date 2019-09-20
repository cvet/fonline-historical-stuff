#ifndef _CSPRITEMGR_H_
#define _CSPRITEMGR_H_

#include "CFileMngr.h"
#include "netproto.h"

//!Cvet ++++
#define COLOR_DEFAULT		D3DCOLOR_ARGB(255,((opt_gcolor_default >> 16) & 0xFF)+opt_light,\
							((opt_gcolor_default >> 8) & 0xFF)+opt_light,(opt_gcolor_default & 0xFF)+opt_light)

#define COLOR_CRITNAME		opt_tcolor_namecrit

#define COLOR_TEXT_DEFAULT	opt_tcolor_default
#define COLOR_TEXT_SHOUT	opt_tcolor_shout
#define COLOR_TEXT_WHISP	opt_tcolor_whisp
#define COLOR_TEXT_EMOTE	opt_tcolor_emote
#define COLOR_TEXT_SOCIAL	opt_tcolor_social

//#define COLOR_TEXT_AST	D3DCOLOR_ARGB(255,255,0,0)
//!Cvet ----

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

struct MYVERTEX
{
	FLOAT x,y,z,rhw;
	DWORD Diffuse;
	FLOAT tu,tv;

	MYVERTEX(): x(0),y(0),z(0),rhw(1),tu(0),tv(0),Diffuse(0){};
};
#define D3DFVF_MYVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

struct FLTRECT
{
	FLOAT x1,y1,x2,y2;
	FLTRECT():x1(0),y1(0),x2(0),y2(0){};
	FLTRECT(FLOAT _x1,FLOAT _y1,FLOAT _x2,FLOAT _y2):x1(_x1),y1(_y1),x2(_x2),y2(_y2){};

	void operator() (FLOAT _x1,FLOAT _y1,FLOAT _x2,FLOAT _y2) {x1=_x1;y1=_y1;x2=_x2;y2=_y2;};
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

typedef vector<LPDIRECT3DTEXTURE8> surf_vect;
typedef map<CrTYPE, char*, less<CrTYPE> > ctypes_map; //!Cvet ������� ����

// ��������� ������� ��� DX8
struct SpriteInfo
{
	LPDIRECT3DTEXTURE8 lpSurf;
	FLTRECT spr_rect;
	WORD w,h;
	short offs_x,offs_y;
	SpriteInfo(): lpSurf(NULL),w(0),h(0),offs_x(0),offs_y(0){};
};

typedef map<WORD, SpriteInfo*, less<WORD> > sprinfo_map;

struct OneSurface
{
	LPDIRECT3DTEXTURE8 lpSurf;
	WORD cnt;
	OneSurface(LPDIRECT3DTEXTURE8 lps):lpSurf(lps),cnt(1){};
};
typedef vector<OneSurface*> onesurf_vec;

struct PrepSprite
{
	int scr_x, scr_y;
	WORD spr_id;
	WORD* lp_sprid;
	short* lp_ox;
	short* lp_oy;
	PrepSprite() {};
	PrepSprite(int x, int y, WORD id,WORD* lpid=NULL,short* alp_ox=NULL,short* alp_oy=NULL): scr_x(x), scr_y(y), spr_id(id),lp_sprid(lpid),lp_ox(alp_ox),lp_oy(alp_oy) {};
};
typedef multimap<DWORD, PrepSprite*, less<DWORD> > dtree_map;

typedef vector<PrepSprite*> spr_vec; //!Cvet
typedef vector<spr_vec> spr_vec_vec; //!Cvet

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

struct CritFrames 
{
	WORD* ind; //������� ������ ��������
	WORD dir_offs[6]; //�������� � �������� ��� ���� 6-�� �����������.
	short *next_x;//�������� ������ ������������ ����������
	short *next_y;//�������� ������ ������������ ����������
	BYTE cnt_frames; //���-�� ������ � �������� �� ������ �����������
	WORD ticks;

	CritFrames(): ind(NULL),next_x(NULL),next_y(NULL){};
	~CritFrames(){SAFEDELA(ind);SAFEDELA(next_x);SAFEDELA(next_y);};
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
// ������ �����
struct AnyFrames 
{
	WORD* ind; //������� ������ ��������
	short *next_x;//�������� ������ ������������ ����������
	short *next_y;//�������� ������ ������������ ����������
	BYTE cnt_frames; //���-�� ������ � ��������
	WORD ticks;// ������� ��������

	AnyFrames(): ind(NULL),next_x(NULL),next_y(NULL){};
	~AnyFrames(){SAFEDELA(ind);SAFEDELA(next_x);SAFEDELA(next_y);};
};
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

class CSpriteManager
{
public:

	int Init(LPDIRECT3DDEVICE8 lpD3Device);
	void Clear();

	void PreRestore();
	void PostRestore();

	int LoadRix(char *fname, int PathType);
	int LoadMiniSprite(char *fname,double size,int PathType,SpriteInfo** ppInfo=NULL); // ��� ��������� ����������� �������
	int LoadSprite(char *fname,int PathType,SpriteInfo** ppInfo=NULL);
	int LoadAnimation(char *fname,int PathType,CritFrames* pframes);
	int LoadAnimationD(char *fname,int PathType,CritFrames* pframes);
	int LoadAnyAnimation(char *fname,int PathType,WORD end_id[4],SpriteInfo** ppInfo = NULL);

	void NextSurface();
	SpriteInfo* GetSpriteInfo(WORD id) {return spr_data[id];};

	int PrepareBuffer(dtree_map* lpdtree,LPDIRECT3DVERTEXBUFFER8* lplpBuf,onesurf_vec* lpsvec, BYTE alpha); //!Cvet BYTE alpha

	int Flush();

	int DrawSprite(WORD id, int x, int y, DWORD color); //!Cvet DWORD color
    int DrawSpriteSize(WORD id, int x, int y,double size, DWORD color); // ����������� ������������ //!Cvet DWORD color
	void DrawTreeCntr(dtree_map* lpdtree);
	void DrawPrepared(LPDIRECT3DVERTEXBUFFER8 lpBuf,onesurf_vec* lpsvec, WORD cnt);

	DWORD GetLoadedCnt() {return spr_data.size();};

	void SetColor(DWORD c){col=c;};

	LPDIRECT3DVERTEXBUFFER8 GetVB() {return lpVB;};
	LPDIRECT3DINDEXBUFFER8 GetIB() {return lpIB;};

	void GetDrawCntrRect(PrepSprite* prep, INTRECT* prect);

	CSpriteManager();
	~CSpriteManager(){for(int d1=0;d1<100;d1++)
						for(int d2=0;d2<27;d2++)
							for(int d3=0;d3<27;d3++)
								SAFEDEL(CrAnim[d1][d2][d3]);}; //!Cvet

//!Cvet ++++++++++++++++++++++++++++++++++++++
	ctypes_map crit_types;
	CritFrames* CrAnim[150][27][27];
	int LoadCritTypes();
	int LoadAnimCr(CrTYPE anim_type, BYTE anim_ind1, BYTE anim_ind2);
	int EraseAnimCr(CrTYPE anim_type, BYTE anim_ind1, BYTE anim_ind2);
//!Cvet --------------------------------------
private:
	bool crtd;

	int LoadSpriteAlt(char *fname,int PathType,SpriteInfo** ppInfo=NULL); //!Cvet �������� �������������� �������

	surf_vect surf_list;
	sprinfo_map spr_data;
	
	onesurf_vec call_vec;
	OneSurface* last_call;

	DWORD next_id;

	LPDIRECT3DTEXTURE8 last_surf;
	LPDIRECT3DTEXTURE8 cur_surf;
	WORD last_w,last_h;//������� ����������� �������
	WORD busy_w,busy_h;//������� ������� �������
	WORD free_x,free_y;//������� ������� �������
  
	int spr_cnt;//�� ������� �������� ���������. ��� ���������� �������� ���.
	int cur_pos;//������� ������� ������.

	LPDIRECT3DDEVICE8 lpDevice;

	LPDIRECT3DVERTEXBUFFER8 lpVB;//����� � ��������� ��� ��������
	LPDIRECT3DINDEXBUFFER8 lpIB;//����� � ���������

	//����� ������ � ��������� ������ ��� ��������� ���������� ��������
	MYVERTEX* lpWaitBuf;

	LPDIRECT3DTEXTURE8 CreateNewSurf(WORD w, WORD h);

	CFileMngr fm;

	DWORD col;
};

#endif
