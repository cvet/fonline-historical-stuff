#ifndef __STDAFX_H__
#define __STDAFX_H__

/********************************************************************
	created:	17:08:2004   18:47 updated: begin 2007

	author:		Oleg Mareskin
	add/edit:	Anton Cvetinsky (Cvet)
	
	purpose:	
*********************************************************************/


#include <windows.h>
#include "resource.h"

#define random(a) (rand()*a/(RAND_MAX+1))

#include <d3dx8.h>
#include <dinput.h>
#include <dxerr8.h>
#include <dsound.h> //!Cvet

#pragma warning (disable : 4786)

#include <map>
#include <string> //!Cvet
#include <set> //!Cvet
#include <vector>
using namespace std;

#include <crtdbg.h>

#define chSTR2(x) #x
#define chSTR(x) chSTR2(x)
#define chMSG(desc) message(__FILE__ "(" chSTR(__LINE__) "):" #desc)

#pragma comment (lib, "d3dx8.lib")
#pragma comment (lib, "d3d8.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dxerr8.lib")
#pragma comment (lib, "wsock32.lib")
#pragma comment (lib, "dsound.lib") //!Cvet
#pragma comment (lib, "dxguid.lib") //!Cvet


#define SAFEREL(x) {if(x) (x)->Release();(x)=NULL;}
#define SAFEDEL(x) {if(x) delete (x);(x)=NULL;}
#define SAFEDELA(x) {if(x) delete[] (x);(x)=NULL;}

struct INTRECT
{
	int l;
	int t;
	int r;
	int b;
	INTRECT():l(0),t(0),r(0),b(0){};
	INTRECT(int al,int at,int ar,int ab):l(al),t(at),r(ar),b(ab){};

	int& operator[](int index) //!Cvet +++
	{ 
		switch(index)
		{
		case 0: return l;
		case 1: return t;
		case 2: return r;
		case 3: return b;
		}
		return l;
	} //!Cvet ---
};

//#define MODE_WIDTH (opt_screen_mode?1024:800)
//#define MODE_HEIGHT (opt_screen_mode?768:600)
#define MODE_WIDTH (screen_width[opt_screen_mode]) //!Cvet
#define MODE_HEIGHT (screen_height[opt_screen_mode]) //!Cvet

//!Cvet ++++
//��� ������ � ��������� �����������
#define BIN__N(x) (x) | x>>3 | x>>6 | x>>9
#define BIN__B(x) (x) & 0xf | (x)>>12 & 0xf0
#define BIN8(v) (BIN__B(BIN__N(0x##v)))

//��� ������ � ������ (�� ���� - �������)
#define BITS(x,y) ((x)&(y))
#define FLAG BITS

#define SET_BITS(x,y) (x)=(x)|(y)
#define SETFLAG SET_BITS

//#define UNSET_BITS(x,y) {if((x)&(y)) (x)=(x)^(y);}
#define UNSET_BITS(x,y) (x)=((x)|(y))^(y)
#define UNSETFLAG UNSET_BITS
//!Cvet ----

#endif //__STDAFX_H__