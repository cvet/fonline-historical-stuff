#ifndef __CFONT_H__
#define __CFONT_H__

/********************************************************************
	created:	01:12:2004   07:20

	author:		Oleg Mareskin
	edit:		Anton Tsvetinsky aka Cvet
	
	purpose:	
*********************************************************************/

#include "CSpriteManager.h"

struct Letter
{
  WORD dx,dy;
  BYTE w,h;
  short y_offs;
  Letter():dx(0),dy(0),w(0),h(0){};
};

#define FT_NOBREAK		1
#define FT_CENTERX		2
#define FT_CENTERY		4
#define FT_BOTTOM		8
//!Cvet +++
#define FT_UPPER		16 //����� ���������� ������� � ������ �������
#define FT_COLORIZE		32 //����������. ��� ����� ���� �������� �� ������������� ��������� col
//!Cvet ---

class CFOFont
{
public:

	int Init(LPDIRECT3DDEVICE8 lpD3Device,LPDIRECT3DVERTEXBUFFER8 aVB,LPDIRECT3DINDEXBUFFER8 aIB);
	void Clear();

	void PreRestore();
	void PostRestore(LPDIRECT3DVERTEXBUFFER8 aVB,LPDIRECT3DINDEXBUFFER8 aIB);

	void MyDrawText(RECT r,char* astr,DWORD flags, DWORD col);
  
	
     CFOFont();
	~CFOFont();
private:

	int Flush();

	bool crtd;

	int spr_cnt;//�� ������� �������� ���������. ��� ���������� �������� ���.

	LPDIRECT3DDEVICE8 lpDevice;

	LPDIRECT3DVERTEXBUFFER8 lpVB;//����� � ��������� ��� ��������
	LPDIRECT3DINDEXBUFFER8 lpIB;//����� � ���������

	//����� ������ � ��������� ������ ��� ��������� ���������� ��������
	MYVERTEX* lpWaitBuf;

	LPDIRECT3DTEXTURE8 font_surf;

	Letter let[256];
	int eth;
	int etw;
	int *maxx;
	USHORT max_cnt;
	int cur_pos;
	FLOAT xyarr[256][4];
};


#endif //__CFONT_H__