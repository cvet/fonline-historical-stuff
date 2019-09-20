#include "stdafx.h"

#include "FEngine.h"
#include "common.h"
#include "version.h"
#include "keyb.h"

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
    return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
    free(address);
}

CFEngine::CFEngine(): crtd(0),hWnd(NULL),lpD3D(NULL),lpDevice(NULL),islost(0),lpDInput(NULL),lpKeyboard(NULL),lpMouse(NULL)
{
	dilost=0;
	
	cmn_di_left=0;
	cmn_di_right=0;
	cmn_di_up=0;
	cmn_di_down=0;

	cmn_di_mleft=0;
	cmn_di_mright=0;
	cmn_di_mup=0;
	cmn_di_mdown=0;

	CtrlDwn=0;
	AltDwn=0;
	ShiftDwn=0;
	edit_mode=0;
//!Cvet +++++++++++++++++++++++
	SetScreen(SCREEN_LOGIN);
	SetCur(CUR_DEFAULT);
//!Cvet -----------------------

	ed_str[0]=0;
	cur_edit=0; //!Cvet
	lang=LANG_RUS;

	comlen=4096;
	compos=0;
	ComBuf=new char[comlen];
	zstrmok=0;
	state=0;
	stat_com=0;
	stat_decom=0;

	lpChosen=NULL; // ����� ������� ���������
	lpNPC0=NULL;  // ��� ������� ���������
	lpNPC1=NULL;
}

int CFEngine::Init(HWND _hWnd)
{
	WriteLog("\nFEngine Initialization...\n");
	
	HRESULT hr;

	hWnd=_hWnd;

	InitKeyb();

	WriteLog("������ Direct3D.....");
	lpD3D=Direct3DCreate8(D3D_SDK_VERSION);
	if(!lpD3D){
		ErrMsg("Engine Init","�� ���� ������� Direct3D.\n���������, ��� ���������� DirectX ������ 8.1 � ����");
		return 0;
	}
	WriteLog("OK\n");

	WriteLog("��������� ������.....");
	D3DDISPLAYMODE d3ddm;
	hr=lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3ddm);
	if(hr!=D3D_OK){
		ErrMsg("GetAdapterDisplayMode",(char*)DXGetErrorString8(hr));
		return 0;
	}
	WriteLog("OK\n");

	WriteLog("Create device.....");

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp,sizeof(d3dpp));
	d3dpp.Windowed=opt_fullscr?0:1;
	d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
	//d3dpp.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;	//??? lockable need
	if(!opt_fullscr)
		d3dpp.BackBufferFormat=d3ddm.Format;
		else
		{
			d3dpp.BackBufferWidth=MODE_WIDTH;
			d3dpp.BackBufferHeight=MODE_HEIGHT;
			d3dpp.BackBufferFormat=D3DFMT_A8R8G8B8;
			if(!opt_vsync) d3dpp.FullScreen_PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
		}

	hr=lpD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&lpDevice);
	if(hr!=D3D_OK){
		ErrMsg("CreateDevice",(char*)DXGetErrorString8(hr));
		return 0;
	}
	WriteLog("OK\n");

	WriteLog("��������� �������.....");
	lpDevice->SetRenderState(D3DRS_LIGHTING,FALSE);//��������� ����
	lpDevice->SetRenderState(D3DRS_ZENABLE, FALSE); // Disable Z-Buffer
    lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // Disable Culling
    //�������� ������������ - Alpha blending
	lpDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

//	lpDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCCOLOR );
//	lpDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCCOLOR );

	lpDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
	lpDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
    //�������� alpha testing
	lpDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
	lpDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
	lpDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

	//�������� ����������
//lpDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
//lpDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
//lpDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        
	
//lpDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
//lpDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	lpDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
	lpDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);

	lpDevice->SetTextureStageState(0,D3DTSS_COLOROP ,D3DTOP_MODULATE2X);
//	lpDevice->SetTextureStageState(0,D3DTSS_ALPHAOP ,D3DTOP_SELECTARG1);
	lpDevice->SetTextureStageState(0,D3DTSS_ALPHAOP ,D3DTOP_MODULATE); //!Cvet


	lpDevice->SetVertexShader(D3DFVF_MYVERTEX);
	WriteLog("OK\n");

	lpDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),1.0,0);


	if(!InitDInput()) return 0;

	if(!sm.Init(lpDevice)) return 0;

	if(!sdm.Init()) return 0;

	WriteLog("Loading splash...");

	//if(!(splash=sm.LoadRix("new1.rix",PT_ART_SPLASH))) return 0;
	if(!(splash=sm.LoadRix("splash2.rix",PT_ART_SPLASH))) return 0;
	sm.NextSurface();
	lpDevice->BeginScene();
	sm.DrawSprite(splash,0,0,COLOR_DEFAULT);
	sm.Flush();
	lpDevice->EndScene();
	lpDevice->Present(NULL,NULL,NULL,NULL);

	WriteLog("OK\n");

	if(!fnt.Init(lpDevice,sm.GetVB(),sm.GetIB())) return 0;

//!Cvet++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	CreateParamsMaps();

	if(!Init_Iface())
	{
		WriteLog("FALSE\n");
		return 0;
	}

//��������� ��������, ����, ����� ����������� ��������
	int t1;
	char t2[20];
	char stradd[32];
	FILE *cf;
	//����� ��������
	//name_obj[100][MAX_OBJECT_NAME];
	//���� ��������
	//info_obj[100][MAX_OBJECT_INFO];
	//������� � ����� �������� � ���������
	WriteLog("�������� �������� � ��������� ����������� ��������...");
	if((cf = fopen("data\\objects\\pic_inv.txt", "rt")) != NULL)
	{
		t1=0;
		while(!feof(cf))
		{
			fscanf(cf, "%s", &t2);
			inv_pic_b[t1]=sm.LoadSprite(t2,PT_ART_INVEN);
			inv_pic_s[t1]=sm.LoadMiniSprite(t2,2,PT_ART_INVEN);

			if((!inv_pic_b[t1])||(!inv_pic_s[t1]))
			{
				WriteLog("������ %d=%s\n",t1,t2);
				return 0;
			}

			t1++;
		}
		fclose(cf);
		WriteLog("���������\n");
	} 
	else
	{
		WriteLog("���� �� ������\n");
		return 0;
	}
	//�������� �������������
	WriteLog("�������� �������� ����� ������������� ��������...");
	if((cf = fopen("data\\objects\\pic_use.txt", "rt")) != NULL)
	{
		t1=0;
		while(!feof(cf))
		{
			fscanf(cf, "%s", &t2);
			sprintf(stradd,"%s%d.png",t2,numIface);
			pic_use[t1]=sm.LoadSprite(stradd,PT_ART_IFACE);

			if(!pic_use[t1])
			{
				WriteLog("������ �������� %s\n",stradd);
				return 0;
			}

			t1++;
		}
		fclose(cf);
		WriteLog("���������\n");
	} 
	else
	{
		WriteLog("���� �� ������\n");
		return 0;
	}

	//��������� ����������� �������
	WriteLog("�������� ����������� ��������...");

	FILE *o_cf;
	FILE *o_cf2;
	params_map::iterator it_o;

	for(stat_map::iterator it=all_s_obj.begin(); it!=all_s_obj.end(); it++)
	{
		SAFEDEL((*it).second);
		all_s_obj.erase(it);
	}

	if((o_cf=fopen("data\\objects\\all_obj.st","rt"))==NULL)
	{
		WriteLog("���� all_obj.st �� ������\n");
		return 0;
	}

	int cnt_obj=0;
	int tmpi=0;
	char tmpc[120];

	while(!feof(o_cf))
	{
		tmpi=0;
		fscanf(o_cf,"%d",&tmpi);
		if(!tmpi) break;

		sprintf(tmpc,"data\\objects\\%d.st",tmpi);
		if((o_cf2=fopen(tmpc,"rt"))==NULL)
		{
			WriteLog("���� |%s| �� ������\n",tmpc);
			return 0;
		}

		stat_obj* new_obj;
		new_obj= new stat_obj;

		new_obj->id=tmpi;
		fscanf(o_cf2,"%s",&tmpc);
		it_o=object_map.find(tmpc);
		if(it_o==object_map.end())
		{
			WriteLog("�������� |%s| �� ������",tmpc);
			return 0;
		}
		new_obj->type=(*it_o).second;

		while(!feof(o_cf2))
		{
			fscanf(o_cf2,"%s%d",&tmpc,&tmpi);

			it_o=object_map.find(tmpc);
			if(it_o==object_map.end())
			{
				WriteLog("�������� |%s| �� ������",tmpc);
				return 0;
			}
			new_obj->p111[(*it_o).second]=tmpi;
		}

		all_s_obj[new_obj->id]=new_obj;

		fclose(o_cf2);
		cnt_obj++;
	}
	
	fclose(o_cf);

	WriteLog("OK (%d ��������)\n",cnt_obj);

//!Cvet ------------------------------------------------------------------------------------

	if(!hf.Init(&sm)) return 0;

	state=STATE_DISCONNECT; //!Cvet

	SetColor(COLOR_DEFAULT);

	if(!opt_fullscr)
	{
		RECT r;
		GetWindowRect(hWnd,&r);
		SetCursorPos(r.left+2+320,r.top+22+240);
	}
	else 
	{
		SetCursorPos(320,240);
	}
	ShowCursor(0);
	cur_x=320;cur_y=240;

	WriteLog("����� �������� ���������: %d\n",sm.GetLoadedCnt());

	WriteLog("FEngine Initialization complete\n");
	crtd=1;

	return 1;
}

int CFEngine::InitDInput()
{
	WriteLog("DInput init...\n");
    HRESULT hr = DirectInput8Create(GetModuleHandle(NULL),DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&lpDInput,NULL);
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","�� ���� ������� DirectInput");
		return 0;
	}

    // Obtain an interface to the system keyboard device.
    hr = lpDInput->CreateDevice(GUID_SysKeyboard,&lpKeyboard,NULL);
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","�� ���� ������� GUID_SysKeyboard");
		return 0;
	}

    hr = lpDInput->CreateDevice(GUID_SysMouse,&lpMouse,NULL);
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","�� ���� ������� GUID_SysMouse");
		return 0;
	}
    // Set the data format to "keyboard format" - a predefined data format 
    // This tells DirectInput that we will be passing an array
    // of 256 bytes to IDirectInputDevice::GetDeviceState.
    hr = lpKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","�� ���� ���������� ������ ������ ��� ����������");
		return 0;
	}

    hr = lpMouse->SetDataFormat(&c_dfDIMouse2);
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","�� ���� ���������� ������ ������ ��� �����");
		return 0;
	}


    hr = lpKeyboard->SetCooperativeLevel( hWnd,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","������ SetCooperativeLevel ��� ����������");
		return 0;
	}

    hr = lpMouse->SetCooperativeLevel( hWnd,DISCL_FOREGROUND | (opt_fullscr?DISCL_EXCLUSIVE:DISCL_NONEXCLUSIVE));
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","������ SetCooperativeLevel ��� �����");
		return 0;
	}

    // IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
    DIPROPDWORD dipdw;

    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = DI_BUF_SIZE; // Arbitary buffer size

    hr = lpKeyboard->SetProperty(DIPROP_BUFFERSIZE,&dipdw.diph);
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","������ ��������� ������ ������ ��� ����������");
		return 0;
	}

    hr = lpMouse->SetProperty(DIPROP_BUFFERSIZE,&dipdw.diph);
	if(hr!=DI_OK)
	{
		ErrMsg("CFEngine InitDInput","������ ��������� ������ ������ ��� �����");
		return 0;
	}

    // Acquire the newly created device
    lpKeyboard->Acquire();
    lpMouse->Acquire();

	WriteLog("DInput init OK\n");

	return 1;
}

void CFEngine::Clear()
{
	WriteLog("\nFEngine Clear...\n");

	ClearKeyb();

	NetDiscon();

	hf.Clear();
	sm.Clear();
	fnt.Clear();
	sdm.Clear();

	SAFEREL(lpDevice);
	SAFEREL(lpD3D);

	if(lpKeyboard) lpKeyboard->Unacquire();
	SAFEREL(lpKeyboard);
	if(lpMouse) lpMouse->Unacquire();
	SAFEREL(lpMouse);
	SAFEREL(lpDInput);

	SAFEDELA(ComBuf);

	crtd=0;

	WriteLog("FEngine Clear complete\n");
}

void CFEngine::ClearCritters() //!Cvet
{
	for(crit_map::iterator it=critters.begin();it!=critters.end();it++)
	{
		hf.RemoveCrit((*it).second);
		delete (*it).second;
	}
	critters.clear();

	lpChosen=NULL;
}

void CFEngine::RemoveCritter(CrID remid) //!Cvet
{
	crit_map::iterator it=critters.find(remid);
	if(it==critters.end()) return;

	hf.RemoveCrit((*it).second);
	delete (*it).second;
	critters.erase(it);

	if(lpChosen->id==remid) lpChosen=NULL;
}

// ����� ������
int CFEngine::Console()
{
	// 1100
	RECT r4={0,MODE_HEIGHT-move,100,MODE_HEIGHT-15};
	RECT r5={100,MODE_HEIGHT-move,200,MODE_HEIGHT-15}; //cmn_scr_ox, cmn_scr_oy - �������� �� �����
	RECT r6={200,MODE_HEIGHT-move,400,MODE_HEIGHT-15};
	char str[256],str1[256],str2[256];
	wsprintf(str1,"���� ���������� x=%d,y=%d, rit=%d, \ntx=%d, ty=%d\n.  ���� x=%d, y=%d" 
	,lpChosen->hex_x,lpChosen->hex_y,lpChosen->rit,TargetX,TargetY,cur_x,cur_y);
	wsprintf(str2,"������������ ����: ������ %d, cur_id %d, cur_ox %d, cur_oy %d",lpChosen->weapon,lpChosen->cur_id,lpChosen->cur_ox,lpChosen->cur_oy);
//	wsprintf(str,"������� ������ �������: ��������:%d, �������������:%d, ������:%d, �����:%d"
//	,LstMoveId,LstAddCritId,LstSayCritId,LstDelCritId);
//	wsprintf(str,"Pwleft:%d, Phbegin:%d"
//	,hf.Pwleft , hf.Phbegin);

	wsprintf(str,"R=%d,G=%d,B=%d---�����:%d:%d",dayR,dayG,dayB,Game_Hours,Game_Mins);

	fnt.MyDrawText(r4,str1,FT_CENTERX|FT_CENTERY,D3DCOLOR_XRGB(255,240,0));
	fnt.MyDrawText(r5,str2,FT_CENTERX|FT_CENTERY,D3DCOLOR_XRGB(255,240,0));
	fnt.MyDrawText(r6,str,FT_CENTERX|FT_CENTERY,D3DCOLOR_XRGB(255,240,0));
	return 1;
}
// ����� ������
int CFEngine::Worldmap()
{
/*	sm.DrawSprite(worldmap1,250,MODE_HEIGHT-752);
	sm.DrawSprite(worldmap2,512,MODE_HEIGHT-752);
	sm.DrawSprite(worldmap3,250,MODE_HEIGHT-390);
	sm.DrawSprite(worldmap4,512,MODE_HEIGHT-390);
			// ���������������������
	sm.DrawSprite(Arroyo,280,MODE_HEIGHT-735);
	sm.DrawSprite(Junk,600,MODE_HEIGHT-305);
	sm.DrawSprite(Bos,550,MODE_HEIGHT-340);
	sm.DrawSprite(Den,350,MODE_HEIGHT-700);
	sm.DrawSprite(Redding,420,MODE_HEIGHT-650);
	sm.DrawSprite(Staff,545,MODE_HEIGHT-350);
	sm.DrawSprite(Statue2,605,MODE_HEIGHT-425);
	sm.DrawSprite(Flagncr,600,MODE_HEIGHT-440);
	sm.DrawSprite(Pole,595,MODE_HEIGHT-435);
	sm.DrawSprite(Enclave,260,MODE_HEIGHT-440);
	sm.DrawSprite(Flagencl,275,MODE_HEIGHT-445);
	sm.DrawSprite(Navarro2,290,MODE_HEIGHT-560);
	sm.DrawSprite(Navarro1,270,MODE_HEIGHT-580);
	sm.DrawSprite(Klamath,350,MODE_HEIGHT-735);
	sm.DrawSprite(Vault13,535,MODE_HEIGHT-405);
	sm.DrawSprite(Gecko,545,MODE_HEIGHT-710);
	sm.DrawSprite(Vtcity,545,MODE_HEIGHT-690);
	sm.DrawSprite(Epa,630,MODE_HEIGHT-690);
	sm.DrawSprite(Modoc,445,MODE_HEIGHT-710);
	sm.DrawSprite(Reno2,462,MODE_HEIGHT-567);
	sm.DrawSprite(Reno6,500,MODE_HEIGHT-570);
	sm.DrawSprite(Reno1,460,MODE_HEIGHT-540);
	sm.DrawSprite(Reno5,500,MODE_HEIGHT-543);
	sm.DrawSprite(Reno3,468,MODE_HEIGHT-560);
	sm.DrawSprite(Reno4,490,MODE_HEIGHT-563);
	sm.DrawSprite(Mbase1,400,MODE_HEIGHT-415);
	sm.DrawSprite(Mbase2,400,MODE_HEIGHT-410);
	sm.DrawSprite(Sfchina,340,MODE_HEIGHT-450);
	sm.DrawSprite(Vault15,675,MODE_HEIGHT-405);
	sm.DrawSprite(Gathedral,595,MODE_HEIGHT-185);
	sm.DrawSprite(Hub,605,MODE_HEIGHT-235);
	sm.DrawSprite(Bones13,700,MODE_HEIGHT-105);
	sm.DrawSprite(Glow,690,MODE_HEIGHT-105);
	sm.DrawSprite(Hills,530,MODE_HEIGHT-580);
    sm.DrawSprite(Cart,635,MODE_HEIGHT-650);
    sm.DrawSprite(lpChosen->miniplayer,645,MODE_HEIGHT-670);
*/
	return 1;
}

// �������� �������� ������ ����
int CFEngine::Render()
{
	static TICK LastCall=GetTickCount();
	static WORD call_cnt=0;
	static WORD fps=0;

    if((GetTickCount()-LastCall)>=1000)
	{
		fps=call_cnt;
		call_cnt=0;
		LastCall=GetTickCount();
	}
	else call_cnt++;

	ParseInput();
	if(!crtd || islost) return 0;
	
	//������������� ����
	if(state==STATE_INIT_NET)
	{
		if(!InitNet()) 
		{
			WriteLog("EXIT\n");
			DestroyWindow(hWnd);
			return 0;
		}
		if(IsScreen(SCREEN_REGISTRATION )) Net_SendCreatePlayer(&New_cr);
		if(IsScreen(SCREEN_LOGIN		)) Net_SendLogIn(opt_login, opt_pass);
	}

	//Parse NET
	if(state!=STATE_DISCONNECT)
		ParseSocket();

//!Cvet �����/����
	if(IsScreen(SCREEN_LOGIN		)) { ShowLogIn();			return 1; }
//!Cvet �����������
	if(IsScreen(SCREEN_REGISTRATION	)) { ShowRegistration();	return 1; }

	if(state==STATE_DISCONNECT) { SetScreen(SCREEN_LOGIN); return 0; }

	if(!hf.IsMapLoaded()) return 1;
	if(!lpChosen) return 1;

	hf.Scroll();

	if(opt_dbgclear) lpDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,255,0),1.0,0);

	ChosenProcess();

	for(crit_map::iterator it=critters.begin();it!=critters.end();it++)
		(*it).second->Process();

	lpDevice->BeginScene();

	if(cmn_show_onlys) sm.DrawSprite(splash,0,0,COLOR_DEFAULT);
	else
	{	
		SetWeather();

		hf.DrawMap2();
		sm.Flush();

		for(it=critters.begin();it!=critters.end();it++)
			(*it).second->DrawText(&fnt);

		IntDrawGraph(); //��������� ����������

		if(IsScreen(SCREEN_INVENTORY )) InvDrawGraph(); //��������� ���������

		if(IsScreen(SCREEN_GLOBAL_MAP)) Worldmap();// ������� �����

		if(IsScreen(SCREEN_DIALOG_NPC)) DlgDrawGraph(); //��������� �������

		if(IsLMenu()) LMenuDraw(); //��������� LMenu

		sm.Flush();

		IntDrawText(); //��������� ������ � ���������

		if(IsScreen(SCREEN_INVENTORY)) InvDrawText(); //��������� ������ � ���������

		if(IsScreen(SCREEN_DIALOG_NPC)) DlgDrawText(); //��������� ������ � �������

		if(edit_mode)
		{
			RECT r3={IntX+15,IntY-40,632,IntY-16};
			char str_to_edit[2048];
			static bool show_cur=true;
			static TICK show_cur_last_time=GetTickCount();

			if(GetTickCount()>show_cur_last_time+400)
			{
				show_cur=!show_cur;
				show_cur_last_time=GetTickCount();
			}

			str_to_edit[0]=0;
			strcpy(str_to_edit,ed_str);
			if(show_cur)
				str_to_edit[cur_edit]='|';
			else
				str_to_edit[cur_edit]='.';
			str_to_edit[cur_edit+1]=0;
			strcat(str_to_edit,&ed_str[cur_edit]);
			fnt.MyDrawText(r3,str_to_edit,FT_NOBREAK|FT_BOTTOM,COLOR_TEXT_DEFAULT);
		}
	}

// console � ����������� ��� ����� �������� ���������� ����������
	if(cmn_console) 
	{
		RECT r2={0,MODE_HEIGHT-move+150,190,MODE_HEIGHT-15};
		Console();	char verstr[256];
		wsprintf(verstr,"by Gamers from FOdev\nversion %s\nfps: %d",FULLVERSTR,fps);
		fnt.MyDrawText(r2,verstr,FT_CENTERX|FT_CENTERY,D3DCOLOR_XRGB(255,248,0));
	}
  
	if(newplayer) if((GetTickCount()-LastCall)>=1000) newplayer=0;

//��������� �������
	if((IsCur(CUR_DEFAULT) || IsCur(CUR_USE_OBJECT) || IsCur(CUR_USE_SKILL)) && !IsLMenu())
	{
		SpriteInfo* si=sm.GetSpriteInfo(cur);
		int x=cur_x-(si->w >> 1)+si->offs_x;
		int y=cur_y-si->h+si->offs_y;
		sm.DrawSprite(cur,x,y,COLOR_DEFAULT);
	}
	else if(IsCur(CUR_MOVE))
	{
		if(!GetMouseTile(cur_x,cur_y))
			sm.DrawSprite(cur_def,cur_x,cur_y,COLOR_DEFAULT);
		else
		{
			if(hf.hex_field[TargetY][TargetX].passed==TRUE)
				sm.DrawSprite(cur_move,hf.hex_field[TargetY][TargetX].scr_x+1,
					hf.hex_field[TargetY][TargetX].scr_y-1,COLOR_DEFAULT);
			else if(hf.hex_field[TargetY][TargetX].passed==FALSE)
				sm.DrawSprite(cur_move_block,hf.hex_field[TargetY][TargetX].scr_x+1,
					hf.hex_field[TargetY][TargetX].scr_y-1,COLOR_DEFAULT);
		}
	}

	sm.Flush();

	lpDevice->EndScene();

	if(lpDevice->Present(NULL,NULL,NULL,NULL)==D3DERR_DEVICELOST)
	{
		WriteLog("D3dDevice is lost\n");
		islost=1;
		cmn_lost=1;
	}

	return 1;
}

void CFEngine::ParseInput()
{
	if(dilost) RestoreDI();
	if(dilost) return;

	DIDEVICEOBJECTDATA didod[DI_BUF_SIZE];  // Receives buffered data 
	DWORD dwElements;
	HRESULT hr;
	int i;

//!Cvet +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if(IsCur(CUR_WAIT))
	{
		dwElements = DI_BUF_SIZE;
		hr = lpKeyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
										 didod, &dwElements, 0 );
		if(hr!=DI_OK)
		{
			dilost=1;
			WriteLog("WAIT ParseInput keyboard> %s\n",(char*)DXGetErrorString8(hr));
			return;
		}

		for(i=0;i<dwElements;i++) 
		{
//			DI_ONDOWN( DIK_ESCAPE , NetDiscon(); return );
		}

		dwElements = DI_BUF_SIZE;
		hr = lpMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
										 didod, &dwElements, 0 );
		if(hr!=DI_OK)
		{
			dilost=1;
			WriteLog("WAIT ParseInput mouse> %s\n",(char*)DXGetErrorString8(hr));
			return;
		}

		for(i=0;i<dwElements;i++) 
		{
			DI_ONMOUSE( DIMOFS_X, cur_x+=didod[i].dwData*opt_mouse_speed );
			DI_ONMOUSE( DIMOFS_Y, cur_y+=didod[i].dwData*opt_mouse_speed );
		}
	}
	else
	{
	//�����/����
		if(IsScreen(SCREEN_LOGIN)) LogInput();
	//�����������
		if(IsScreen(SCREEN_REGISTRATION)) RegInput();
	}

	if(IsScreen(SCREEN_LOGIN) || IsScreen(SCREEN_REGISTRATION) || IsCur(CUR_WAIT))
	{
		if(!opt_fullscr)
		{
			RECT r;
			GetWindowRect(hWnd,&r);
			POINT p;
			GetCursorPos(&p);
			cur_x=p.x-(r.left+2);
			cur_y=p.y-(r.top+22);
		}

		if(cur_x>MODE_WIDTH) cur_x=MODE_WIDTH;
		if(cur_x<0) cur_x=0;
		if(cur_y>MODE_HEIGHT) cur_y=MODE_HEIGHT;
		if(cur_y<0) cur_y=0;

		return;
	}
//!Cvet -------------------------------------------------------------

    dwElements = DI_BUF_SIZE;
    hr = lpKeyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
                                     didod, &dwElements, 0 );
	if(hr!=DI_OK)
	{
		dilost=1;
		WriteLog("ParseInput keyboard> %s\n",(char*)DXGetErrorString8(hr));
		return;
	}

//!Cvet +++
	static TICK tick_press=GetTickCount();
	static int time_press=1000;
	static WORD last_key=NULL;
	if(last_key && GetTickCount()>tick_press+time_press)
	{
		if(!GetChar(last_key,ed_str,&cur_edit,EDIT_LEN,lang,ShiftDwn)) last_key=NULL;
		tick_press=GetTickCount();
		if((time_press-=time_press/2)<30) time_press=30;
	}
//!Cvet ---

    for(i=0;i<dwElements;i++) 
    {
	//	DI_ONDOWN( DIK_ESCAPE , DestroyWindow(hWnd);islost=1;return );//Exit from program
//		DI_ONDOWN( DIK_ESCAPE , NetDiscon(); return );
		//DI_ONDOWN( DIK_ESCAPE , NetDiscon(); return; );

		DI_ONDOWN( DIK_RETURN , 
			if(edit_mode)
			{
				if(!ed_str[0])
					edit_mode=0;
				else
					Net_SendText(ed_str);
				
				ed_str[0]=0;
				cur_edit=0;
			}
			else 
			{
				edit_mode=1;
				ed_str[0]=0;
				cur_edit=0;
			}
		);

		if(edit_mode)
		{
			int fnd=0;
			for(WORD tst=0;tst<256;tst++) //!Cvet ���� DI_BUF_SIZE
			{
				DI_ONDOWN(tst, last_key=NULL; if(GetChar(tst,ed_str,&cur_edit,EDIT_LEN,lang,ShiftDwn)) {fnd=1;break;});
				DI_ONUP(tst, last_key=NULL;);
			}
			if(fnd) { last_key=tst; tick_press=GetTickCount(); time_press=600; break; }
		}
		else if(!CtrlDwn)
		{
			DI_ONDOWN( DIK_LEFT ,cmn_di_left=1 );
			DI_ONDOWN( DIK_RIGHT ,cmn_di_right=1 );
			DI_ONDOWN( DIK_UP ,cmn_di_up=1 );
			DI_ONDOWN( DIK_DOWN ,cmn_di_down=1 );

			DI_ONUP( DIK_LEFT ,cmn_di_left=0 );
			DI_ONUP( DIK_RIGHT ,cmn_di_right=0 );
			DI_ONUP( DIK_UP ,cmn_di_up=0 );
			DI_ONUP( DIK_DOWN ,cmn_di_down=0 );
		}
		else
		{
			DI_ONDOWN( DIK_LEFT ,Net_SendRot(0) );
			DI_ONDOWN( DIK_RIGHT ,Net_SendRot(1) );
		}

		DI_ONDOWN( DIK_G ,hf.SwitchShowHex() ); 

//		DI_ONUP( DIK_Q ,opt_scroll_delay-=10;WriteLog("scroll_delay=%d\n",opt_scroll_delay) );
//		DI_ONUP( DIK_W ,opt_scroll_delay+=10;WriteLog("scroll_delay=%d\n",opt_scroll_delay) );
//
//		DI_ONUP( DIK_A ,opt_scroll_step>>=1;WriteLog("scroll_step=%d\n",opt_scroll_step) );
//		DI_ONUP( DIK_S ,opt_scroll_step<<=1;WriteLog("scroll_step=%d\n",opt_scroll_step) );

		DI_ONDOWN( DIK_RCONTROL ,CtrlDwn=1;
			if(opt_change_lang==CHANGE_LANG_RCTRL) lang=(lang==LANG_RUS)?LANG_ENG:LANG_RUS;
		);
		DI_ONUP( DIK_RCONTROL ,CtrlDwn=0 );
		DI_ONDOWN( DIK_LCONTROL ,CtrlDwn=1 );
		DI_ONUP( DIK_LCONTROL ,CtrlDwn=0 );

		DI_ONDOWN( DIK_LMENU ,AltDwn=1 );
		DI_ONUP( DIK_LMENU ,AltDwn=0 );
		DI_ONDOWN( DIK_RMENU ,AltDwn=1 );
		DI_ONUP( DIK_RMENU ,AltDwn=0 );

		DI_ONDOWN( DIK_LSHIFT ,ShiftDwn=1;
			if(CtrlDwn && opt_change_lang==CHANGE_LANG_CTRL_SHIFT) lang=(lang==LANG_RUS)?LANG_ENG:LANG_RUS;
			if(AltDwn && opt_change_lang==CHANGE_LANG_ALT_SHIFT) lang=(lang==LANG_RUS)?LANG_ENG:LANG_RUS;
		);
		DI_ONUP( DIK_LSHIFT ,ShiftDwn=0 );
		DI_ONDOWN( DIK_RSHIFT ,ShiftDwn=1;
			if(CtrlDwn && opt_change_lang==CHANGE_LANG_CTRL_SHIFT) lang=(lang==LANG_RUS)?LANG_ENG:LANG_RUS;
			if(AltDwn && opt_change_lang==CHANGE_LANG_ALT_SHIFT) lang=(lang==LANG_RUS)?LANG_ENG:LANG_RUS;
		);
		DI_ONUP( DIK_RSHIFT ,ShiftDwn=0 );

/*		DI_ONUP( DIK_5 ,cmn_show_tiles=!cmn_show_tiles );
		DI_ONUP( DIK_6 ,cmn_show_roof=!cmn_show_roof );
		DI_ONUP( DIK_7 ,cmn_show_scen=!cmn_show_scen );
		DI_ONUP( DIK_8 ,cmn_show_walls=!cmn_show_walls );
		DI_ONUP( DIK_9 ,cmn_show_items=!cmn_show_items );
		DI_ONUP( DIK_0 ,cmn_show_crit=!cmn_show_crit );

		DI_ONDOWN( DIK_3 , Game_Time=0;);
		DI_ONDOWN( DIK_1 , Game_Time+=20;);
		DI_ONDOWN( DIK_2 , Game_Time-=20;);
*/
//		DI_ONDOWN( DIK_1 , opt_light+=5;);
//		DI_ONDOWN( DIK_2 , opt_light-=5;);
		
//		DI_ONDOWN( DIK_M ,
//			SetScreenCastling(SCREEN_MAIN,SCREEN_GLOBAL_MAP);
//		);

		DI_ONDOWN( DIK_O , cmn_console=!cmn_console;);
//		DI_ONDOWN( DIK_I ,
//			SetScreenCastling(SCREEN_MAIN,SCREEN_INVENTORY);			
//		);

		//!Cvet ��������� ������ - �������� �� ���������
//		DI_ONDOWN( DIK_C, 
//			lpChosen->AddObject(44,1100,10000000,0,0);
//		);
		
    }

	//formouse
    dwElements = DI_BUF_SIZE;
    hr = lpMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
                                     didod, &dwElements, 0 );
	if(hr!=DI_OK)
	{
		dilost=1;
		WriteLog("ParseInput mouse> %s\n",(char*)DXGetErrorString8(hr));
		return;
	}
//!Cvet ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if(!lpChosen) return;
	//ChosenMouseInput();
//���� �����
	if(lpChosen->cond==COND_LIFE)
	{
		for(i=0;i<dwElements;i++) 
		{
			DI_ONMOUSE( DIMOFS_X, cur_x+=didod[i].dwData*opt_mouse_speed );
			DI_ONMOUSE( DIMOFS_Y, cur_y+=didod[i].dwData*opt_mouse_speed );

			if(IsLMenu()) LMenuMouseMove();

			DI_ONDOWN( DIMOFS_BUTTON1,
				if(IsScreen(SCREEN_MAIN))
				{
					if(IsCur(CUR_DEFAULT)  || IsCur(CUR_MOVE))
						if((cur_x>=IntObject[0])&&(cur_y>=IntObject[1])&&
							(cur_x<=IntObject[2])&&(cur_y<=IntObject[3]))
						{
							if(lpChosen->a_obj->object->type==OBJ_TYPE_WEAPON)
							{
								lpChosen->rate_object++;
 								if(lpChosen->rate_object>lpChosen->a_obj->object->p111[OBJ_WEAP_COUNT_ATTACK]) lpChosen->rate_object=1;
							}
						}
				}
			);

			DI_ONUP( DIMOFS_BUTTON1,
				if(IsScreen(SCREEN_MAIN))
				{
					SetCurCastling(CUR_DEFAULT,CUR_MOVE);
				}
			);

			DI_ONDOWN( DIMOFS_BUTTON0,
				if(IsScreen(SCREEN_MAIN))
				if(!IntMouseDown())
				{
				//��������� �������
					//�������� ����� ��� �������
					if(IsCur(CUR_DEFAULT))
					{
						LMenu_try_activated=true;
						LMenu_start_time=GetTickCount();
					}
					//�����
					if(IsCur(CUR_MOVE))
					{
						if(GetMouseTile(cur_x,cur_y))
						{
							if(PathMoveX==TargetX && PathMoveY==TargetY)
								lpChosen->move_type=MOVE_RUN;
							else
								lpChosen->move_type=MOVE_WALK;

							PathMoveX=TargetX;
							PathMoveY=TargetY;
							SetChosenAction(ACTION_MOVE);
						}
					}
					//���������� ������
					if(IsCur(CUR_USE_OBJECT))
					{
						if((TargetID=GetMouseCritter(cur_x,cur_y)))
							SetChosenAction(ACTION_USE_OBJ_ON_CRITTER);
						else if((TargetID=GetMouseItem(cur_x,cur_y)))
							SetChosenAction(ACTION_USE_OBJ_ON_ITEM);

					}
					//���������� �����
					if(IsCur(CUR_USE_SKILL))
					{
						;;
					}
				}

				if(IsScreen(SCREEN_INVENTORY )) InvMouseDown();
				if(IsScreen(SCREEN_DIALOG_NPC)) DlgMouseDown();
			);

			DI_ONUP( DIMOFS_BUTTON0,
				if(IsScreen(SCREEN_MAIN	  )) IntMouseUp();
				if(IsScreen(SCREEN_INVENTORY )) InvMouseUp();
				if(IsScreen(SCREEN_DIALOG_NPC)) DlgMouseUp();

				if(IsLMenu()) LMenuMouseUp();
				LMenu_try_activated=false;
			);

			if(IsScreen(SCREEN_DIALOG_NPC)) DlgMouseMove();
			if(IsScreen(SCREEN_INVENTORY )) InvMouseMove();
		}

		if(LMenu_try_activated==true) LMenuTryCreate();
	}
	else
	{
		for(i=0;i<dwElements;i++) 
		{
			DI_ONMOUSE( DIMOFS_X, cur_x+=didod[i].dwData*opt_mouse_speed );
			DI_ONMOUSE( DIMOFS_Y, cur_y+=didod[i].dwData*opt_mouse_speed );
		}
	}
//!Cvet ----------------------------------------------------------

	if(!opt_fullscr)
	{
		RECT r;
		GetWindowRect(hWnd,&r);
		POINT p;
		GetCursorPos(&p);
		cur_x=p.x-(r.left+2);
		cur_y=p.y-(r.top+22);
	}

	if(IsScreen(SCREEN_MAIN) && !IsLMenu())
	{
		if(cur_x>=MODE_WIDTH)
		{
			cur_x=MODE_WIDTH;
			cmn_di_mright=1;
			cur=cur_right;
		}
		else cmn_di_mright=0;

		if(cur_x<=0)
		{
			cur_x=0;
			cmn_di_mleft=1;
			cur=cur_left;
		}
		else cmn_di_mleft=0;

		if(cur_y>=MODE_HEIGHT)
		{
			cur_y=MODE_HEIGHT;
			cmn_di_mdown=1;
			cur=cur_down;
		}
		else cmn_di_mdown=0;

		if(cur_y<=0)
		{
			cur_y=0;
			cmn_di_mup=1;
			cur=cur_up;
		}
		else cmn_di_mup=0;

		if		(cmn_di_mright	&& cmn_di_mup	) cur=cur_ru;
		else if	(cmn_di_mleft	&& cmn_di_mup	) cur=cur_lu;
		else if	(cmn_di_mright	&& cmn_di_mdown	) cur=cur_rd;
		else if	(cmn_di_mleft	&& cmn_di_mdown	) cur=cur_ld;

		if(cmn_di_mright || cmn_di_mleft || cmn_di_mup || cmn_di_mdown) return;

		if(IsCur(CUR_DEFAULT	)) cur=cur_def;
		if(IsCur(CUR_MOVE		)) cur=cur_move;
		if(IsCur(CUR_USE_OBJECT )) cur=cur_use_o;
		if(IsCur(CUR_USE_SKILL	)) cur=cur_use_s;
	}

	if(IsScreen(SCREEN_INVENTORY))
		if(cur_hold)
			cur=cur_hold; 
		else
			cur=cur_hand;

	if(IsScreen(SCREEN_DIALOG_NPC)) cur=cur_def;

	if(IsLMenu()) cur=cur_ru; //!!!!!!!
}

void CFEngine::Restore()
{
	if(!crtd || !islost) return;
	WriteLog("Restoring...\n");
	WriteLog("��������� ������.....");
	D3DDISPLAYMODE d3ddm;
	HRESULT hr=lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3ddm);
	if(hr!=D3D_OK){
		ErrMsg("GetAdapterDisplayMode",(char*)DXGetErrorString8(hr));
		return;
	}
	WriteLog("OK\n");

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp,sizeof(d3dpp));
	d3dpp.Windowed=opt_fullscr?0:1;
	d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
	d3dpp.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;	//??? lockable need
	if(!opt_fullscr)
		d3dpp.BackBufferFormat=d3ddm.Format;
		else
		{
			d3dpp.BackBufferWidth=MODE_WIDTH;
			d3dpp.BackBufferHeight=MODE_HEIGHT;
			d3dpp.BackBufferFormat=D3DFMT_A8R8G8B8;
			if(!opt_vsync) d3dpp.FullScreen_PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
		}

	hf.PreRestore();
	sm.PreRestore();
	fnt.PreRestore();
	hr=lpDevice->Reset(&d3dpp);
	if(hr!=D3D_OK)
	{
		//ErrMsg("Device Reset","�� ���� ������������� ����������");
		WriteLog("�� ���� �������������\n");
		return;
	}
	lpDevice->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),1.0,0);

	
	WriteLog("��������� �������.....");
	lpDevice->SetRenderState(D3DRS_LIGHTING,FALSE);//��������� ����
	lpDevice->SetRenderState(D3DRS_ZENABLE, FALSE); // Disable Z-Buffer
    lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // Disable Culling
    //�������� ������������ - Alpha blending
	lpDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    lpDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
    lpDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
    //�������� alpha testing
	lpDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
    lpDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
    lpDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );


	//�������� ����������
	lpDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
	lpDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);

	lpDevice->SetTextureStageState(0,D3DTSS_COLOROP ,D3DTOP_MODULATE2X);
//	lpDevice->SetTextureStageState(0,D3DTSS_ALPHAOP ,D3DTOP_SELECTARG1);
	lpDevice->SetTextureStageState(0,D3DTSS_ALPHAOP ,D3DTOP_MODULATE); //!Cvet


	lpDevice->SetVertexShader(D3DFVF_MYVERTEX);
	WriteLog("OK\n");

	
	sm.SetColor(D3DCOLOR_ARGB(255,103+opt_light,95+opt_light,86+opt_light));
	sm.PostRestore(); //������ � ����� �������. ��� sm ������ ������ ������� IB
	hf.PostRestore();
	fnt.PostRestore(sm.GetVB(),sm.GetIB());

	WriteLog("Restoring complete\n");
	islost=0;
	cmn_lost=0;
}

void CFEngine::RestoreDI()
{
	if(!crtd || !lpKeyboard) return;
	WriteLog("Restoring DI...\n");

	//���������� �������� ����������
	HRESULT hr=lpKeyboard->Acquire();
	if(hr==DI_OK) WriteLog("RestoringDI Keyboard OK\n");
		else WriteLog("RestoringDI Keyboard error %s\n",(char*)DXGetErrorString8(hr));

	hr=lpMouse->Acquire();
	if(hr==DI_OK) WriteLog("RestoringDI Mouse OK\n");
		else WriteLog("RestoringDI Mouse error %s\n",(char*)DXGetErrorString8(hr));
//	cur_x=320;cur_y=240;
//	if(!opt_fullscr)
//	{
//		RECT r;
//		GetWindowRect(hWnd,&r);
//		SetCursorPos(r.left+2+320,r.top+22+240);
//	}else SetCursorPos(320,240);
	dilost=0;
}

int CFEngine::InitNet()
{
	WriteLog("Network init...\n");

	WSADATA WsaData;
	if(WSAStartup(0x0101,&WsaData))
	{
		ErrMsg("CFEngine::InitNet","WSAStartup error!");
		return 0;
	}

	remote.sin_family=AF_INET;
	remote.sin_port=htons(opt_rport);
	if((remote.sin_addr.s_addr=inet_addr(opt_rhost))==-1)
	{
		hostent *h=gethostbyname(opt_rhost);

		if(!h)
		{
			ErrMsg("CFEngine::InitNet","cannot resolve remote host %s!",opt_rhost);
			return 0;
		}

		memcpy(&remote.sin_addr,h->h_addr,sizeof(in_addr));
	}

	#pragma chMSG("����� ������ ������ � ������ ����� �����������")
	if(!NetCon()) return 0;

	state=STATE_CONN;

	WriteLog("Network init OK\n");

	return 1;
}

int CFEngine::NetCon()
{
	WriteLog("Connecting to server %s:%d\n",opt_rhost,opt_rport);
	sock=socket(AF_INET,SOCK_STREAM,0);
	if(connect(sock,(sockaddr*)&remote,sizeof(SOCKADDR_IN)))
	{
		ErrMsg("CFEngine::NetCon","�� ���� ������������ � ������� ����!\r\n");
		return 0;
	}
	WriteLog("Connecting OK\n");

    zstrm.zalloc = zlib_alloc;
    zstrm.zfree = zlib_free;
    zstrm.opaque = NULL;

	if(inflateInit(&zstrm)!=Z_OK)
	{
		ErrMsg("CFEngine::NetCon","InflateInit error!\r\n");
		return 0;
	}
	zstrmok=1;

	return 1;
    // ������� ������� ���������
    WriteLog("Net_Con");
}

void CFEngine::NetDiscon()
{
	WriteLog("������������ ");

	if(zstrmok) inflateEnd(&zstrm);
	closesocket(sock);
	state=STATE_DISCONNECT;

	WriteLog("--> �������� ���������� ������ �������: %d -> %d\n",stat_com,stat_decom);

	ClearCritters(); //!Cvet
	hf.UnLoadMap(); //!Cvet
}

void CFEngine::ParseSocket(WORD wait)
{
	timeval tv;
	tv.tv_sec=wait;
	tv.tv_usec=0;

	if(sock==-1) return;

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&exc_set);
	
	FD_SET(sock,&read_set);
	//FD_SET(sock,&write_set);
	//FD_SET(sock,&exc_set);

	select(0,&read_set,&write_set,&exc_set,&tv);

	if(FD_ISSET(sock,&read_set))
	{
		if(!NetInput())
		{
		//	ErrMsg("CFEngine::ParseSocket","������� ������ �������� �����!\r\n");
			WriteLog("CFEngine::ParseSocket","������� ������ �������� �����!\r\n");
			sock=-1;
			state=STATE_DISCONNECT;
			return;
		}
	}

	NetProcess();

	NetOutput();
}

void CFEngine::NetProcess()
{
	if(!bin.pos) return;

	MSGTYPE msg;

	while(bin.NeedProcess())
	{
		bin >> msg;
		
		switch(msg) 
		{
		case NETMSG_LOGINOK:
			state=STATE_LOGINOK;
			WriteLog("������������ ��������\n");
			LogMsg=0;
			break;
		case NETMSG_ADDCRITTER:
			Net_OnAddCritter();
			break;
		case NETMSG_REMOVECRITTER:
			Net_OnRemoveCritter();
			break;
		case NETMSG_CRITTERTEXT:
			Net_OnCritterText();
			break;
		case NETMSG_CRITTER_ROTCW:
			Net_OnCritterRot(0);
			break;
		case NETMSG_CRITTER_ROTCCW:
			Net_OnCritterRot(1);
			break;

//!Cvet +++++++++++++++++++++++++++++++++++++++++++
		case NETMSG_XY:
			Net_OnChosenXY();
			break;
		case NETMSG_ALL_PARAMS:
			Net_OnChosenParams();
			break;
		case NETMSG_PARAM:
			Net_OnChosenParam();
			break;
        case NETMSG_ADD_OBJECT:
			Net_OnChosenAddObject();
			break;
		case NETMSG_LOGMSG:
			Net_OnChosenLogin();
			break;
		case NETMSG_TALK_NPC:
			Net_OnChosenTalk();
			break;

		case NETMSG_GAME_TIME:
			Net_OnGameTime();
			break;

		case NETMSG_LOADMAP:
			Net_OnLoadMap();
			break;
		case NETMSG_MAP:
			Net_OnMap();
			break;
		case NETMSG_ADD_OBJECT_ON_MAP:
			Net_OnAddObjOnMap();
			break;
		case NETMSG_REMOVE_OBJECT_FROM_MAP:
			Net_OnRemObjFromMap();
			break;

		case NETMSG_CRITTER_ACTION:
			Net_OnCritterAction();
			break;
        case NETMSG_CRITTER_MOVE:
			Net_OnCritterMove();
			break;
//!Cvet -------------------------------------------

		case NETMSG_NAMEERR:
			WriteLog("Name: %s: ERR!\n",opt_name);
			state=STATE_DISCONNECT;
			break;
		default:
			WriteLog("Wrong MSG: %d!\n",msg);
			state=STATE_DISCONNECT;
		}
	}
	bin.reset();
}

/*void CFEngine::Net_SendName(char* name)
{
	WriteLog("Net_SendName...");

	MSGTYPE msg=NETMSG_NAME;

	bout << msg;
	bout.push(name,MAX_NAME);
	for(int i=0;i<5;i++)
		bout.push(opt_cases[i],MAX_NAME);
	BYTE gen=opt_gender[0];
	bout << gen;

	state=STATE_NAMESEND;
    // ������� ������� ���������
    WriteLog("OK\n");
}*/
//!Cvet ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void CFEngine::Net_SendLogIn(char* login, char* pass)
{
	WriteLog("Net_SendLogIn...");

	MSGTYPE msg=NETMSG_LOGIN;

	bout << msg;
	bout.push(login,MAX_LOGIN);
	bout.push(pass,MAX_LOGIN);

	LogMsg=10;
    // ������� ������� ���������
    WriteLog("OK\n");
}

void CFEngine::Net_SendCreatePlayer(crit_info* newcr)
{
	int bi;
	WriteLog("����������� ������\n");

//������������ �������
	MSGTYPE msg=NETMSG_CREATE_CLIENT;
	bout << msg;
	bout.push(newcr->login,MAX_LOGIN);
	bout.push(newcr->pass,MAX_LOGIN);
	bout.push(newcr->name,MAX_NAME);
	for(bi=0; bi<5; bi++)
		bout.push(newcr->cases[bi],MAX_NAME);
	//SPECIAL
	bout << newcr->st[ST_STRENGHT	];
	bout << newcr->st[ST_PERCEPTION	];
	bout << newcr->st[ST_ENDURANCE	];
	bout << newcr->st[ST_CHARISMA	];
	bout << newcr->st[ST_INTELLECT	];
	bout << newcr->st[ST_AGILITY	];
	bout << newcr->st[ST_LUCK		];
	//�������
	bout << newcr->st[ST_AGE];
	//���
	bout << newcr->st[ST_GENDER];

	WriteLog("����������� ������ ������ �������\n");
}
//!Cvet ----------------------------------------------------------------
void CFEngine::Net_SendText(char* str)
{
	WriteLog("Net_SendText...");

	WORD len=strlen(str);
	if(len>=MAX_TEXT) len=MAX_TEXT;
	MSGTYPE msg=NETMSG_TEXT;

	bout << msg;
	bout << len;
	bout.push(str,len);

	WriteLog("OK\n");
}

void CFEngine::Net_SendRot(BYTE rot)
{
	WriteLog("Net_SendRot...");

	MSGTYPE msg=NETMSG_ROTATE;

	bout << msg;
	bout << rot;
    // ������� ������� ���������
    WriteLog("OK\n");
}

//!Cvet +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void CFEngine::Net_SendMove(BYTE dir) //��������� �������
{
	WriteLog("Net_SendMove...");

	BYTE how_move=lpChosen->Move(dir,NULL);;

	if(how_move==MOVE_ERROR)
	{
		WriteLog("FALSE\n");
		return;
	}
	hf.MoveCritter(lpChosen,dir);

	BYTE move_params=0;

	switch(dir)
	{
	case 0: SETFLAG(move_params,BIN8(00000000)); break;
	case 1: SETFLAG(move_params,BIN8(00000001)); break;
	case 2: SETFLAG(move_params,BIN8(00000010)); break;
	case 3: SETFLAG(move_params,BIN8(00000011)); break;
	case 4: SETFLAG(move_params,BIN8(00000100)); break;
	case 5: SETFLAG(move_params,BIN8(00000101)); break;
	default: return;
	}

	if(how_move==MOVE_RUN) move_params+=BIN8(00001000) & BIN8(00001000);

	if(lpChosen->hex_x!=PathMoveX || lpChosen->hex_y!=PathMoveY)
		SETFLAG(move_params,BIN8(00010000));

	MSGTYPE msg=NETMSG_SEND_MOVE;

	bout << msg;
	bout << move_params;

	WriteLog("OK\n");
}

void CFEngine::Net_SendUseObject(BYTE crit_obj_type, DWORD targ_id, BYTE crit_ori, BYTE crit_num_action, BYTE crit_rate_object)
{
	WriteLog("�������� �������� ��� ���������...");

	MSGTYPE msg=NETMSG_SEND_USE_OBJECT;

	bout << msg;
	bout << crit_obj_type;
	bout << targ_id;
	bout << crit_ori;
	bout << crit_num_action;
	bout << crit_rate_object;

	WriteLog("OK\n");
}

void CFEngine::Net_SendChangeObject(DWORD idobj, BYTE num_slot)
{
	WriteLog("�������� ����� �������� id=%d, slot=%d...",idobj, num_slot);

	MSGTYPE msg=NETMSG_SEND_CHANGE_OBJECT;

	bout << msg;
	bout << idobj;
	bout << num_slot;

	WriteLog("OK\n");
}

void CFEngine::Net_SendTalk(CrID id_to_talk, BYTE answer)
{
	WriteLog("������� ��������� �� ������� � ��� id_NPC=%d, �����=%d...", id_to_talk, answer);

	MSGTYPE msg=NETMSG_SEND_TALK_NPC;

	bout << msg;
	bout << id_to_talk;
	bout << answer;

	lpChosen->Tick_Start(TALK_MAX_TIME);

	WriteLog("OK\n");
}

void CFEngine::Net_SendGetTime()
{
	WriteLog("������� ��������� �� ������� �������� �������...");

	MSGTYPE msg=NETMSG_SEND_GET_TIME;

	bout << msg;

	WriteLog("OK\n");
}

void CFEngine::Net_SendGiveMeMap(WORD map_num)
{
	WriteLog("������� ��������� �� ������� �������� �����...");

	MSGTYPE msg=NETMSG_SEND_GIVE_ME_MAP;

	bout << msg;
	bout << map_num;

	WriteLog("OK\n");
}

void CFEngine::Net_SendLoadMapOK()
{
	WriteLog("������� ��������� �� �������� �������� �����...");

	MSGTYPE msg=NETMSG_SEND_LOAD_MAP_OK;

	bout << msg;

	WriteLog("OK\n");
}
//!Cvet -----------------------------------------------------------------------------

void CFEngine::Net_OnAddCritter()
{
	WriteLog("������������� ����� �����...");
	crit_info info;

	info.a_obj=&info.def_obj1;
	info.a_obj_arm=&info.def_obj2;

	WORD id_obj;

	bin >> info.id;
	bin >> info.base_type;

	bin >> id_obj;
	info.a_obj->object=all_s_obj[id_obj];

	bin >> id_obj;
	info.a_obj_arm->object=all_s_obj[id_obj];

	bin >> info.x;
	bin >> info.y;
	bin >> info.ori;
	bin >> info.st[ST_GENDER];
	bin >> info.cond;
	bin >> info.cond_ext;
	bin >> info.flags;
	bin.pop(info.name,MAX_NAME);

//WriteLog("���: %s, cond=%d,cond_ext=%d\n",info.name,info.cond,info.cond_ext);
	info.name[MAX_NAME]=0;

	for(int i=0;i<5;i++)
	{
		bin.pop(info.cases[i],MAX_NAME);
		info.cases[i][MAX_NAME]=0;
	}

	if(bin.IsError())
	{
		WriteLog("Net_OnAddCritter","Wrong MSG data forNet_OnAddCritter!\n"); //ErrMsg ����
		state=STATE_DISCONNECT;
		return;
	}

	LstAddCritId=info.id; // �������
	strcpy(newbie,info.name); // �������� ����������
	newplayer=1; // ���� ����� ������ ������

	CCritter* pc=AddCritter(&info);
	if(!lpChosen && FLAG(info.flags,FCRIT_CHOSEN))
	{
		lpChosen=pc;
		//hf.SetCenter2(lpChosen->hex_x,lpChosen->hex_y);
	}
//	else AddMess(COLOR_TEXT_DEFAULT,"�� ������� %s",info.cases[2]);

//	hf.PostRestore();

    WriteLog("�������� ������ ���������. %s, id=%d.\n",info.name,info.id);
}

void CFEngine::Net_OnRemoveCritter()
{
	CrID remid;

	bin >> remid;

	if(bin.IsError())
	{
		ErrMsg("Net_OnRemoveCritter","Wrong MSG data forNet_OnRemoveCritter!\n");
		state=STATE_DISCONNECT;
		return;
	}
    LstDelCritId=remid;// �������
    WriteLog("%d ������ ������� ������ ������� �����������...",remid);

	crit_map::iterator it=critters.find(remid);
	if(it==critters.end()) return;
	CCritter* prem=(*it).second;

	WriteLog("������� � hexfield...");
	hf.RemoveCrit(prem);

	WriteLog("������� � critters...");
	delete (*it).second;

	critters.erase(it);
    WriteLog("OK\n",remid);
}

void CFEngine::Net_OnCritterText()
{
	CrID crid;
	BYTE how_say;

	bin >> crid;

	bin >> how_say;

	if(bin.IsError())
	{
		ErrMsg("Net_OnRemoveCritter","Wrong MSG data forNet_OnRemoveCritter!\n");
		state=STATE_DISCONNECT;
		return;
	}
    LstSayCritId=crid;// �������

	WORD len;
	char str[MAX_TEXT+256];

	bin >> len;

	if(bin.IsError() || len>(MAX_TEXT+256))
	{
		WriteLog("Wrong MSG data or too long forProcess_GetText!(len=%d)\n",len);
		state=STATE_DISCONNECT;
		return;
	}

	bin.pop(str,len);
	str[len]=0;

	if(bin.IsError())
	{
		WriteLog("Wrong MSG data forProcess_GetText - partial recv!\n");
		state=STATE_DISCONNECT;
		return;
	}

	DWORD text_color=COLOR_TEXT_DEFAULT;
	switch(how_say)
	{
	default:
	case SAY_NORM:
		break;
	case SAY_SHOUT:
		text_color=COLOR_TEXT_SHOUT;
		break;
	case SAY_EMOTE:
		text_color=COLOR_TEXT_EMOTE;
		break;
	case SAY_WHISP:
		text_color=COLOR_TEXT_WHISP;
		break;
	case SAY_SOCIAL:
		text_color=COLOR_TEXT_SOCIAL;
		break;
	}

	AddMess(text_color,"%s",str);

	crit_map::iterator it=critters.find(crid);
	CCritter* pcrit=NULL;
	if(it!=critters.end())
		pcrit=(*it).second;

	if(pcrit) pcrit->SetText(str,text_color);
	// ������� ������� ���������

	WriteLog("Net_OnCritterText");
}

void CFEngine::Net_OnCritterRot(BYTE rot)
{
	CrID crid;

	bin >> crid;

	if(bin.IsError())
	{
		ErrMsg("Net_OnRemoveCritter","Wrong MSG data forNet_OnRemoveCritter!\n");
		state=STATE_DISCONNECT;
		return;
	}

	crit_map::iterator it=critters.find(crid);
	CCritter* pcrit=NULL;
	if(it!=critters.end())
		pcrit=(*it).second;

	if(pcrit) rot?pcrit->RotCCW():pcrit->RotCW();
    // ������� ������� ���������
    WriteLog("Net_OnCritterRot");
}

//!Cvet +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void CFEngine::Net_OnCritterMove()
{
	WriteLog("����� ����� id=");

	CrID crid=0;
	BYTE dir=0;
	BYTE how_move=0;
	HexTYPE new_x=0;
	HexTYPE new_y=0;

	BYTE move_params=BIN8(00000000);

	//���������� ������
	bin >> crid;
	bin >> move_params;

	bin >> new_x;
	bin >> new_y;

	if(bin.IsError())
	{
		ErrMsg("Net_OnRemoveCritter","Wrong MSG data forNet_OnRemoveCritter!\n");
		state=STATE_DISCONNECT;
		return;
	}

	dir=move_params & BIN8(00000111);
	how_move=MOVE_WALK;

//�������� ������
	if(move_params & BIN8(00001000)) how_move=MOVE_RUN;
	if(new_x>=MAXTILEX || new_y>=MAXTILEY) return;
	if(!crid) return;
	if(dir>5) return;

    WriteLog("%d dir=%d move_type=%d...", crid, dir, how_move);
    
	LstMoveId=crid; // ��� ������� ��������
	
	crit_map::iterator it=critters.find(crid);
	CCritter* pcrit=NULL;
	if(it==critters.end()) return;
	pcrit=(*it).second;

	hf.MoveCritter(pcrit,new_x,new_y);
	dir=pcrit->cur_dir;

	pcrit->move_type=how_move;

	if(move_params & BIN8(00010000))
		{if(!pcrit->Move(dir,PAUSE)) pcrit->SetAnimation();}
	else
		{if(!pcrit->Move(dir,1)) pcrit->SetAnimation();}

	WriteLog("OK\n");
}

void CFEngine::Net_OnCritterAction()
{
	WriteLog("����: %d\n",GetTickCount()-Ping);

	CrID crid;
	BYTE num_action;
	WORD id_st_obj;
	WORD id_st_obj_arm;
	BYTE rate_obj;
	BYTE ori;

	bin >> crid;
	bin >> num_action; //����� ��������
	bin >> id_st_obj; //id ������������ ������ � ����
	bin >> id_st_obj_arm; //id ������������ ������ � ���� �����
	bin >> rate_obj; //����� �������������
	bin >> ori; //���������� ��������
    
	WriteLog("����� id=%d ���������...",crid);

	if(bin.IsError())
	{
		ErrMsg("Net_OnRemoveCritter","Wrong MSG data forNet_OnCritterAction!\n");
		state=STATE_DISCONNECT;
		return;
	}

//������� ��������
	crit_map::iterator it=critters.find(crid);
	CCritter* pcrit=NULL;
	if(it!=critters.end()) pcrit=(*it).second;
	else return;
//������� ������ � ����
	stat_map::iterator it2=all_s_obj.find(id_st_obj);
	stat_obj* pobj=NULL;
	if(it2!=all_s_obj.end()) pobj=(*it2).second;
	else return;
	pcrit->a_obj->object=pobj;
//������� ������� � ���� �����
	it2=all_s_obj.find(id_st_obj_arm);
	stat_obj* pobj_arm=NULL;
	if(it2!=all_s_obj.end()) pobj_arm=(*it2).second;
	else return;
	pcrit->a_obj_arm->object=pobj_arm;

	pcrit->cur_dir=ori;

	WriteLog("Net_OnCritterAction - act=%d, rate=%d\n", num_action, rate_obj);

//������������ ��������
//����������/��������� �������
	if(num_action==ACT_SHOW_OBJ)
	{
		if(!pobj->p111[OBJ_TIME_SHOW]) return;

		pcrit->RefreshWeap();

		if(pobj->type==OBJ_TYPE_WEAPON)
			pcrit->Action(3,pobj->p111[OBJ_TIME_SHOW]);
		else
			pcrit->Action(12,pobj->p111[OBJ_TIME_SHOW]);

		return;
	}

	if(num_action==ACT_HIDE_OBJ)
	{
		if(!pobj->p111[OBJ_TIME_HIDE]) return;

		pcrit->RefreshWeap();

		if(pobj->type==OBJ_TYPE_WEAPON)
			pcrit->Action(3,pobj->p111[OBJ_TIME_HIDE]);
		else
			pcrit->Action(12,pobj->p111[OBJ_TIME_HIDE]);

		return;
	}
//���������/�����������/������������� �������
	if(num_action==ACT_ACTIVATE_OBJ)
	{
		if(pobj->type!=OBJ_TYPE_WEAPON) return;
		if(!pobj->p111[OBJ_WEAP_TIME_ACTIV]) return;

		pcrit->cond_ext=COND_LIFE_ACTWEAP;

		pcrit->Action(8,pobj->p111[OBJ_WEAP_TIME_ACTIV]);

		return;
	}

	if(num_action==ACT_DACTIVATE_OBJ)
	{
		if(pobj->type!=OBJ_TYPE_WEAPON) return;
		if(!pobj->p111[OBJ_WEAP_TIME_UNACTIV]) return;

		pcrit->cond_ext=COND_LIFE_NONE;

		pcrit->Action(9,pobj->p111[OBJ_WEAP_TIME_UNACTIV]);

		return;
	}

	if(num_action==ACT_USE_OBJ)
	{
//		pcrit->cond_ext=COND_LIFE_USEOBJ;

		if(pobj->type==OBJ_TYPE_WEAPON)
		{
			switch(rate_obj)
			{
			case 0:
				break;
			case 1:
				pcrit->Action(pobj->p111[OBJ_WEAP_PA_ANIM2],pobj->p111[OBJ_WEAP_PA_TIME]);
				break;
			case 2:
				pcrit->Action(pobj->p111[OBJ_WEAP_SA_ANIM2],pobj->p111[OBJ_WEAP_SA_TIME]);
				break;
			case 3:
				pcrit->Action(pobj->p111[OBJ_WEAP_TA_ANIM2],pobj->p111[OBJ_WEAP_TA_TIME]);
				break;
			default:
				break;
			}
		}
		else
			pcrit->Action(12,pobj->p111[rate_obj]);

		return;
	}
//���������
	if(num_action==ACT_NULL)
	{
		pcrit->a_obj->object=all_s_obj[pcrit->base_type];
		pcrit->a_obj_arm->object=all_s_obj[pcrit->base_type+200];

		pcrit->SetAnimation();

		return;
	}
//����������
	if(num_action==ACT_REFRESH)
	{
		pcrit->SetAnimation();

		return;
	}
//�������� �����������
	if(num_action==ACT_DEFEAT)
	{
		if(!pcrit->IsFree()) return;

		switch(rate_obj)
		{
		case ACT_DEFEAT_MISS:
			if(pcrit->weapon==1)
				pcrit->Action(14,0);
			else
				pcrit->Action(5,0);
			break;
		case ACT_DEFEAT_FRONT:
			if(pcrit->weapon==1)
				pcrit->Action(15,0);
			else
				pcrit->Action(5,0);
			break;
		case ACT_DEFEAT_REAR:
			pcrit->Action(16,0);
			break;
		case ACT_DEFEAT_KO_FRONT:
			break;
		case ACT_DEFEAT_KO_REAR:
			break;
		default:
			break;
		}

		return;
	}
//����� �����
	if(num_action==ACT_CHANGE_ARM)
	{
		pcrit->SetAnimation();

		return;
	}
//������
	if(num_action==ACT_DEAD)
	{
		pcrit->cond=COND_DEAD;
		pcrit->cond_ext=rate_obj;
		pcrit->weapon=2;

		switch(pcrit->cond_ext)
		{
		case COND_DEAD_NORMAL_UP:
			pcrit->Action(1,1000);//15
			break;
		case COND_DEAD_NORMAL_DOWN:
			pcrit->Action(2,1000);//16
			break;
		case COND_DEAD_CR_NORMAL_UP:
			pcrit->Action(4,1000);
			break;
		case COND_DEAD_BRUST:
			pcrit->Action(7,1000);
			break;
		case COND_DEAD_CR_BRUST:
			pcrit->Action(6,1000);
			break;
		default:
			pcrit->Action(1,1000);
			break;
		}

		return;
	}

	if(num_action==ACT_DISCONNECT)
	{
		pcrit->flags=pcrit->flags | FCRIT_DISCONNECT;

		strcat(pcrit->name,"_off");
		for(int in=0; in<5; in++)
			strcat(pcrit->cases[in],"_off");
	}
}

void CFEngine::Net_OnChosenLogin()
{
	bin >> LogMsg;

	SetCur(CUR_DEFAULT);
}

void CFEngine::Net_OnChosenXY() // ����������� ���� ����������?!cvet
{ 
	HexTYPE Chex_x;
	HexTYPE Chex_y;
	BYTE Cori;

	bin >> Chex_x;
	bin >> Chex_y;
	bin >> Cori;
	
	if(bin.IsError())
	{
		ErrMsg("Net_OnCritterNewXY","Wrong MSG data forNet_OnCritterNewXY!\n");
		state=STATE_DISCONNECT;
		return;
	}

	WriteLog("�������� ��� ��������� �����...");

	if(Chex_x>=MAXTILEX || Chex_y>=MAXTILEY || Cori>5)
	{
		WriteLog("������ � �������� ������ |x=%d,y=%d,ori=%d|\n",Chex_x,Chex_y,Cori);
		return;
	}

	if(lpChosen->cur_dir!=Cori)
	{
		lpChosen->cur_dir=Cori;
		WriteLog("�����������/���������� �����������...");
	}

	if(lpChosen->hex_x!=Chex_x || lpChosen->hex_y!=Chex_y)
	{
		hf.TransitCritter(lpChosen,Cori,Chex_x,Chex_y,true);
		WriteLog("�����������/���������� ��������������...");

		SetChosenAction(ACTION_NONE);
		lpChosen->SetAnimation();
	}

    // ������� ������� ���������
    WriteLog("OK\n");
}

void CFEngine::Net_OnChosenParams()
{
	WriteLog("��������� ���������...");

	BYTE type_param=0;
	BYTE all_send_params=0;
	BYTE num_param=0;
	int go=0;

	bin >> type_param;
	bin >> all_send_params;

	switch (type_param)
	{
	case TYPE_STAT:
		for(go=0; go<ALL_STATS; go++) lpChosen->st[go]=0;
		for(go=0; go<all_send_params; go++)
		{
			bin >> num_param;
			bin >> lpChosen->st[num_param];

			params_str_map::iterator it=stats_str_map.find(num_param);
			if(it!=stats_str_map.end()) AddMess(COLOR_TEXT_DEFAULT,"���� %s = %d",(*it).second.c_str(),lpChosen->st[num_param]);
		}
		break;
	case TYPE_SKILL:
		for(go=0; go<ALL_SKILLS; go++) lpChosen->sk[go]=0;
		for(go=0; go<all_send_params; go++)
		{
			bin >> num_param;
			bin >> lpChosen->sk[num_param];

			params_str_map::iterator it=skills_str_map.find(num_param);
			if(it!=skills_str_map.end()) AddMess(COLOR_TEXT_DEFAULT,"����� %s = %d",(*it).second.c_str(),lpChosen->st[num_param]);
		}
		break;
	case TYPE_PERK:
		for(go=0; go<ALL_PERKS; go++) lpChosen->pe[go]=0;
		for(go=0; go<all_send_params; go++)
		{
			bin >> num_param;
			bin >> lpChosen->pe[num_param];

			params_str_map::iterator it=perks_str_map.find(num_param);
			if(it!=perks_str_map.end()) AddMess(COLOR_TEXT_DEFAULT,"���� %s",(*it).second.c_str());
		}
		break;
	default:
		WriteLog("������. ������������ ��� ��������� �%d\n", type_param);
		return;
	}

	if(bin.IsError())
	{
		WriteLog("Bin Error - Net_OnChosenParams\n");
		state=STATE_DISCONNECT;
		return;
	}

	WriteLog("�������� ���������� ��������� - ����� ���������� ���� %d �������� %d\n", type_param, all_send_params);
}

void CFEngine::Net_OnChosenParam()
{
	WriteLog("������� ��������...");

	BYTE type_param=0;
	BYTE num_param=0;
	WORD old_count=0;
	params_str_map::iterator it;

	bin >> type_param;
	bin >> num_param;

	switch (type_param)
	{
	case TYPE_STAT:
		old_count=lpChosen->st[num_param];

		bin >> lpChosen->st[num_param];

		it=stats_str_map.find(num_param);
		if(it!=stats_str_map.end())
		{
			if(lpChosen->st[num_param]>old_count)
				AddMess(COLOR_TEXT_DEFAULT,"���� %s ���������� �� %d (�����:%d)",(*it).second.c_str(),lpChosen->st[num_param]-old_count,lpChosen->st[num_param]);
			else if(lpChosen->st[num_param]<old_count)
				AddMess(COLOR_TEXT_DEFAULT,"���� %s ���������� �� %d (�����:%d)",(*it).second.c_str(),old_count-lpChosen->st[num_param],lpChosen->st[num_param]);
		}

		break;
	case TYPE_SKILL:
		old_count=lpChosen->sk[num_param];

		bin >> lpChosen->sk[num_param];

		it=skills_str_map.find(num_param);
		if(it!=skills_str_map.end())
		{
			if(lpChosen->sk[num_param]>old_count)
				AddMess(COLOR_TEXT_DEFAULT,"����� %s ���������� �� %d (�����:%d)",(*it).second.c_str(),lpChosen->sk[num_param]-old_count,lpChosen->sk[num_param]);
			else if(lpChosen->sk[num_param]<old_count)
				AddMess(COLOR_TEXT_DEFAULT,"����� %s ���������� �� %d (�����:%d)",(*it).second.c_str(),old_count-lpChosen->sk[num_param],lpChosen->sk[num_param]);
		}

		break;
	case TYPE_PERK:
		old_count=lpChosen->pe[num_param];

		bin >> lpChosen->pe[num_param];

		it=perks_str_map.find(num_param);
		if(it!=perks_str_map.end())
		{
			if(lpChosen->pe[num_param]>old_count)
				AddMess(COLOR_TEXT_DEFAULT,"�������� ���� %s",(*it).second.c_str());
			else if(lpChosen->sk[num_param]<old_count)
				AddMess(COLOR_TEXT_DEFAULT,"���� ���� %s",(*it).second.c_str());
		}

		break;
	default:
		WriteLog("������. ������������ ��� ��������� �%d\n", type_param);
		return;
	}

	if(bin.IsError())
	{
		WriteLog("Bin Error - Net_OnChosenParam\n");
		state=STATE_DISCONNECT;
		return;
	}

	WriteLog("OK\n");
}

void CFEngine::Net_OnChosenAddObject()
{
	WriteLog("������������ �������...");

	DWORD A_id_d;
	WORD A_id_s;
	BYTE A_slot;
	WORD A_holder;
	WORD A_id_bull;
	WORD A_holder_ext;
	WORD A_id_bull_ext;
	TICK A_tick;
	BYTE A_broken;
	

	bin >> A_id_d;
	bin >> A_id_s;
	bin >> A_slot;
	bin >> A_holder;
	bin >> A_id_bull;
	bin >> A_holder_ext;
	bin >> A_id_bull_ext;
	bin >> A_tick;
	bin >> A_broken;

	if(bin.IsError())
	{
		ErrMsg("Net_OnCritterNewXY","Wrong MSG data forNet_OnCritterNewXY!\n");
		state=STATE_DISCONNECT;
		return;
	}

	lpChosen->AddObject(A_slot,A_id_d,A_holder,A_id_bull,A_holder_ext,A_id_bull_ext,A_broken,A_tick,all_s_obj[A_id_s]);

	WriteLog("id_dyn=%d, id_stat=%d, a_slot=%d, tick=%d, broken=%d\n", 
		A_id_d, A_id_s, A_slot, A_tick, A_broken);
}

void CFEngine::Net_OnAddObjOnMap()
{
	WriteLog("������������ ������� �� �����...");

	HexTYPE obj_x;
	HexTYPE obj_y;
	WORD obj_id;

	bin >> obj_x;
	bin >> obj_y;
	bin >> obj_id;

	if(obj_x>=MAXTILEX || obj_y>=MAXTILEY)
	{
		WriteLog("������ ������\n");
		return;
	}

	stat_map::iterator so=all_s_obj.find(obj_id);
	if(so==all_s_obj.end())
	{
		WriteLog("�������� �������\n");
		return;
	}

	if(!hf.AddItemObj((*so).second->p111[OBJ_PIC_MAP],obj_x,obj_y))
	{
		WriteLog("������ ����������\n");
		return;
	}

//	AddMess(0xFF00FFFF,"�������� ���������");

	WriteLog("OK\n");
}

void CFEngine::Net_OnRemObjFromMap()
{
	WriteLog("���������� ������� � �����...");

	HexTYPE obj_x;
	HexTYPE obj_y;
	WORD obj_id;

	bin >> obj_x;
	bin >> obj_y;
	bin >> obj_id;

	if(obj_x>=MAXTILEX || obj_y>=MAXTILEY)
	{
		WriteLog("������ ������\n");
		return;
	}

	stat_map::iterator so=all_s_obj.find(obj_id);
	if(so==all_s_obj.end())
	{
		WriteLog("�������� �������\n");
		return;
	}

	hf.DelItemObj((*so).second->p111[OBJ_PIC_MAP],obj_x,obj_y);

//	AddMess(0xFF00AAFF,"������ ���������");

	WriteLog("OK\n");
}

void CFEngine::Net_OnChosenTalk()
{
	WriteLog("����� ���������� �����...");

	DWORD main_text;
	DWORD answer[MAX_ANSWERS];

	bin >> all_answers;
	
	if(!all_answers)
	{
		SetScreen(SCREEN_MAIN);
		lpChosen->Tick_Null();
		WriteLog("������ ��������...OK\n");
		return;
	}

	bin >> main_text;
	if(!LoadDialogFromFile(TargetID,main_text,text_dialog)) WriteLog("������ �������� ������� �%d...", main_text);

	for(int ct=0; ct<all_answers; ct++)
	{
		bin >> answer[ct];
		if(!LoadDialogFromFile(TargetID,answer[ct],text_answer[ct]))
			WriteLog("������ �������� ������ �%d...", answer[ct]);
	}

	lpChosen->Tick_Start(TALK_MAX_TIME);

	SetScreen(SCREEN_DIALOG_NPC);

	WriteLog("OK\n");
}

void CFEngine::Net_OnGameTime()
{
	WriteLog("�������� � ������� �������...");

	bin >> Game_Time;
	bin >> Game_Day;
	bin >> Game_Month;
	bin >> Game_Year;

	Game_LastTime=Game_Time*60*1000;

	WriteLog("OK\n");
}

void CFEngine::Net_OnLoadMap()
{
	WORD num_map;
	char name_map[30];

	bin >> num_map;

	SetCur(CUR_WAIT);

	ClearCritters();

	sprintf(name_map,"%d.map",num_map);

	if(hf.LoadMap(name_map))
	{
		Net_SendLoadMapOK();
		Net_SendGetTime();
		SetScreen(SCREEN_MAIN);
		SetCur(CUR_DEFAULT);
	}
	else
		Net_SendGiveMeMap(num_map);
}

void CFEngine::Net_OnMap()
{

}
//!Cvet -----------------------------------------------------------------------------

int CFEngine::NetOutput()
{
	if(!bout.pos) return 1;
	int tosend=bout.pos;
	int sendpos=0;
	while(sendpos<tosend)
	{
		int bsent=send(sock,bout.data+sendpos,tosend-sendpos,0);
		sendpos+=bsent;
		if(bsent==SOCKET_ERROR)
		{
			ErrMsg("NetOutput", "SOCKET_ERROR whilesend forserver\n");
			state=STATE_DISCONNECT;
			return 0;
		}
	}

	bout.reset();
    // ������� ������� ���������
    WriteLog("NetOutput\n");

	Ping=GetTickCount();

	return 1;
}

int CFEngine::NetInput()
{

	UINT len=recv(sock,ComBuf,comlen,0);
	if(len==SOCKET_ERROR || !len) 
	{
		//ErrMsg("CFEngine::NetInput","Socket error!\r\n");
		WriteLog("CFEngine::NetInput","Socket error!\r\n");
		return 0;
	}
	bool rebuf=0;

	compos=len;

	while(compos==comlen)
	{
		rebuf=1;
		DWORD newcomlen=comlen<<2;
		char * NewCOM=new char[newcomlen];
		memcpy(NewCOM,ComBuf,comlen);
		SAFEDELA(ComBuf);
		ComBuf=NewCOM;
		comlen=newcomlen;

		len=recv(sock,ComBuf+compos,comlen-compos,0);
		compos+=len;
	}

	if(rebuf)
		bin.grow_buf(comlen<<1);
	bin.reset();

	zstrm.next_in=(UCHAR*)ComBuf;
	zstrm.avail_in=compos;
	zstrm.next_out=(UCHAR*)bin.data;
	zstrm.avail_out=bin.len;
			

	if(inflate(&zstrm,Z_SYNC_FLUSH)!=Z_OK)
	{
		ErrMsg("CFEngine::NetInput","Inflate error!\r\n");
		return 0;
	}

	bin.pos=zstrm.next_out-(UCHAR*)bin.data;

	while(zstrm.avail_in)
	{
		bin.grow_buf(bin.len<<2);
		
		zstrm.next_out=(UCHAR*)bin.data+bin.pos;
		zstrm.avail_out=bin.len-bin.pos;
			

		if(inflate(&zstrm,Z_SYNC_FLUSH)!=Z_OK)
		{
			ErrMsg("CFEngine::NetInput","Inflate continue error!\r\n");
			return 0;
		}
		bin.pos+=zstrm.next_out-(UCHAR*)bin.data;
	}
	WriteLog("\nrecv %d->%d bytes\n",compos,bin.pos);
	stat_com+=compos;
	stat_decom+=bin.pos;
	

	return 1;
}

/*
// �������� �������� ����� ��������
int CFEngine::LoadAnyAnims(int atype,AnyAnimData* pany)
{
	if(!pany) return 0;

	char path[1024];

	switch(atype)
	{
	case engage:
		if(pany->eng) return 1;
		pany->eng=new AnyFrames;
		strcpy(path,"endanim.frm");
		if(!(sm.LoadAnyAnimation(path,PT_ART_INTRFACE,pany->eng))) return 0;
		pany->eng->ticks=800;
		break;
	default:
		return 0;
	}
	return 1;
}

// �������� ����������
void CFEngine::IntAnim()
{   
    endanim=lpAnyData->eng;
	endanim->cnt_frames=5; // �������
	end_id=endanim->ind[1];
	//anm_tkr=GetTickCount();
	//chng_tkr=GetTickCount();
}
*/

void CFEngine::SetColor(BYTE r,BYTE g,BYTE b)
{
	sm.SetColor(D3DCOLOR_ARGB(255,r+opt_light,g+opt_light,b+opt_light));
	hf.OnChangeCol();
}

// !Cvet ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void CFEngine::SetColor(DWORD color)
{
	sm.SetColor(color);
	hf.OnChangeCol();
}

CCritter* CFEngine::AddCritter(crit_info* pinfo)
{
	CCritter* pcrit=new CCritter(&sm);

	RemoveCritter(pinfo->id);

	critters.insert(crit_map::value_type(pinfo->id,pcrit));

	pcrit->hex_x=pinfo->x;
	pcrit->hex_y=pinfo->y;
	pcrit->base_type=pinfo->base_type;
	pcrit->cond=pinfo->cond;
	pcrit->st[ST_GENDER]=pinfo->st[ST_GENDER];
	pcrit->cur_dir=pinfo->ori;

	pcrit->def_obj1=pinfo->def_obj1;
	pcrit->def_obj2=pinfo->def_obj2;
	pcrit->a_obj=&pcrit->def_obj1;
	pcrit->a_obj_arm=&pcrit->def_obj2;

	pcrit->cond=pinfo->cond;
	pcrit->cond_ext=pinfo->cond_ext;
	pcrit->flags=pinfo->flags;

	pcrit->id=pinfo->id;

	pcrit->SetName(pinfo->name);
	for(int in=0; in<5; in++)
		strcpy(pcrit->cases[in],pinfo->cases[in]);
	
	if(pcrit->flags & FCRIT_DISCONNECT)
	{
		strcat(pcrit->name,"_off");
		for(int in=0; in<5; in++)
			strcat(pcrit->cases[in],"_off");
	}

	pcrit->Initialization();

	if(!lpChosen)
	{
		pcrit->def_obj1.object=all_s_obj[pcrit->base_type];
		pcrit->def_obj2.object=all_s_obj[pcrit->base_type+200];
		SetChosenAction(ACTION_NONE);

		if(pcrit->cond==COND_LIFE && pcrit->cond_ext==COND_LIFE_ACTWEAP) SetCur(CUR_USE_OBJECT);

		hf.SetCenter2(pcrit->hex_x,pcrit->hex_y);
	}

	hf.SetCrit(pcrit->hex_x,pcrit->hex_y,pcrit);

//WriteLog("x=%d y=%d type=%d, weap=%d\n",pcrit->hex_x,pcrit->hex_y,pcrit->type,pcrit->weapon);

	return pcrit;
}

int CFEngine::GetMouseTile(int cursor_x, int cursor_y)
{
	TargetX=0;
	TargetY=0;

	if(cursor_x>=IntMain[0] && cursor_y>=IntMain[1] && cursor_x<=IntMain[2] && cursor_y<=IntMain[3]) return 0;

	return hf.GetTilePixel(cursor_x,cursor_y,&TargetX,&TargetY);
}

int CFEngine::GetMouseCritter(int cursor_x, int cursor_y)
{
	CCritter* cr;

	for(crit_map::iterator it=critters.begin();it!=critters.end();it++)
	{
		cr=(*it).second;

		if(cursor_x > cr->drect.l && cursor_x < cr->drect.r)
			if(cursor_y > cr->drect.t && cursor_y < cr->drect.b)
				return cr->id;
	}

	return 0;
}

int CFEngine::GetMouseScenery(int cursor_x, int cursor_y)
{
	
	return 0;
}

int CFEngine::GetMouseItem(int cursor_x, int cursor_y)
{
	
	return 0;
}

int CFEngine::CheckRegData(crit_info* newcr)
{
//�������� ������
	WriteLog("�������� ������ �����������... ");
	int bi;
	//�������� �� ������ ������
	if(strlen(newcr->login)<4 || strlen(newcr->login)>10)
	{
		WriteLog("Err - LOGIN\n");
		LogMsg=1;
		return 0;
	}
	//�������� �� ������ �����
	if(strlen(newcr->pass)<4 || strlen(newcr->pass)>10)
	{
		WriteLog("Err - PASS\n");
		LogMsg=2;
		return 0;
	}
	//�������� �� ������ �����
	if(strlen(newcr->name)<4 || strlen(newcr->name)>MAX_NAME)
	{
		WriteLog("Err - NAME\n");
		LogMsg=13;
		return 0;
	}
	//�������� �� ������ cases
	for(bi=0; bi<5; bi++)
		if(strlen(newcr->cases[bi])<4 || strlen(newcr->cases[bi])>MAX_NAME)
		{
			WriteLog("Err - CASES%d\n",bi);
			LogMsg=14;
			return 0;
		}
	//�������� �������� ����
	if(newcr->base_type<0 || newcr->base_type>2)
	{ 
		WriteLog("Err - ������� ���\n");
		LogMsg=5;
		return 0;
	}
	//�������� ����
	if(newcr->st[ST_GENDER]<0 || newcr->st[ST_GENDER]>1) 
	{ 
		WriteLog("Err - ���\n");
		LogMsg=15;
		return 0;
	}
	//�������� ��������
	if(newcr->st[ST_AGE]<14 || newcr->st[ST_AGE]>80) 
	{ 
		WriteLog("Err - �������\n");
		LogMsg=16;
		return 0;
	}
	//�������� SPECIAL
	if((newcr->st[ST_STRENGHT	]<1)||(newcr->st[ST_STRENGHT	]>10)||
		(newcr->st[ST_PERCEPTION]<1)||(newcr->st[ST_PERCEPTION	]>10)||
		(newcr->st[ST_ENDURANCE	]<1)||(newcr->st[ST_ENDURANCE	]>10)||
		(newcr->st[ST_CHARISMA	]<1)||(newcr->st[ST_CHARISMA	]>10)||
		(newcr->st[ST_INTELLECT	]<1)||(newcr->st[ST_INTELLECT	]>10)||
		(newcr->st[ST_AGILITY	]<1)||(newcr->st[ST_AGILITY		]>10)||
		(newcr->st[ST_LUCK		]<1)||(newcr->st[ST_LUCK		]>10))
		{
			WriteLog("Err - SPECIAL �%d\n", bi);
			LogMsg=5;
			return 0;
		}
	if((newcr->st[ST_STRENGHT]+newcr->st[ST_PERCEPTION]+newcr->st[ST_ENDURANCE]+
		newcr->st[ST_CHARISMA]+newcr->st[ST_INTELLECT]+
		newcr->st[ST_AGILITY]+newcr->st[ST_LUCK])!=40)
		{ 
			WriteLog("Err - SPECIAL sum\n");
			LogMsg=5;
			return 0;
		}
		
	WriteLog("OK\n");
	return 1;
}

void CFEngine::SetWeather()
{
	static const DWORD DAY_MS=24*60*60*1000;
	static const WORD DAY_MIN=24*60;

	Game_CurTime=GetTickCount()-Game_LastTime;
	Game_CurTime*=TIME_MULTIPLER;

	DWORD time_day=(Game_CurTime/1000/60+Game_Time)%DAY_MIN;

	Game_Hours=time_day/60;
	Game_Mins=time_day%60;

	static const BYTE countR[]={18 ,128,103,51 };
	static const BYTE countG[]={18 ,128,95 ,40 };
	static const BYTE countB[]={53 ,128,86 ,29 };

	if(time_day<(DAY_MIN/4)) //����
	{
		dayR=countR[0]+(float)(countR[1]-countR[0])/(DAY_MIN/4)*time_day;
		dayG=countG[0]+(float)(countG[1]-countG[0])/(DAY_MIN/4)*time_day;
		dayB=countB[0]+(float)(countB[1]-countB[0])/(DAY_MIN/4)*time_day;
	}
	else if(time_day<(DAY_MIN/2)) //����
	{
		time_day-=(DAY_MIN/4);
		dayR=countR[1]-(float)(countR[1]-countR[2])/(DAY_MIN/4)*time_day;
		dayG=countG[1]-(float)(countG[1]-countG[2])/(DAY_MIN/4)*time_day;
		dayB=countB[1]-(float)(countB[1]-countB[2])/(DAY_MIN/4)*time_day;
	}
	else if(time_day<(DAY_MIN/4*3)) //����
	{
		time_day-=(DAY_MIN/2);
		dayR=countR[2]-(float)(countR[2]-countR[3])/(DAY_MIN/4)*time_day;
		dayG=countG[2]-(float)(countG[2]-countG[3])/(DAY_MIN/4)*time_day;
		dayB=countB[2]-(float)(countB[2]-countB[3])/(DAY_MIN/4)*time_day;
	}
	else if(time_day<DAY_MIN) //�����
	{
		time_day-=(DAY_MIN/4*3);
		dayR=countR[3]-(float)(countR[3]-countR[0])/(DAY_MIN/4)*time_day;
		dayG=countG[3]-(float)(countG[3]-countG[0])/(DAY_MIN/4)*time_day;
		dayB=countB[3]+(float)(countB[0]-countB[3])/(DAY_MIN/4)*time_day;
	}

	static WORD sum_RGB=0;
	if(sum_RGB!=(dayR+dayG+dayB))
	{
		SetColor(dayR,dayG,dayB);
		sum_RGB=dayR+dayG+dayB;
	}
}

void CFEngine::ChosenProcess()
{
	if(!lpChosen) return;

	int rofx=lpChosen->hex_x;
	int rofy=lpChosen->hex_y;
	if(rofx%2) rofx--;
	if(rofy%2) rofy--;

	if(hf.hex_field[rofy][rofx].roof_id)
		cmn_show_roof=FALSE;
	else
		if(!cmn_show_roof)
		{
			cmn_show_roof=TRUE;
			hf.RebuildTiles();
		}

	if(!lpChosen->IsFree()) return;

	if(lpChosen->cond!=COND_LIFE) return;

	switch (chosen_action)
	{
	case ACTION_NONE:
		break;
	case ACTION_MOVE:
		if(lpChosen->cond_ext!=COND_LIFE_NONE) break;

		if(lpChosen->hex_x!=PathMoveX || lpChosen->hex_y!=PathMoveY)
		{ 
		//���� �����������
//WriteLog("hx=%d,px=%d,hy=%d,py=%d\n",lpChosen->hex_x,PathMoveX,lpChosen->hex_y,PathMoveY);

			BYTE MoveDir=hf.FindStep(lpChosen->hex_x,lpChosen->hex_y,PathMoveX,PathMoveY);
		//�������
			if(MoveDir>=0 && MoveDir<=5) 
			{
				Net_SendMove(MoveDir);
				return;
			}
		//��������� ������ ��������
			if(MoveDir==FINDPATH_DEADLOCK		) lpChosen->SetText("� ���� �� ������",COLOR_TEXT_DEFAULT);
			if(MoveDir==FINDPATH_ERROR			) WriteLog("!!!WORNING!!! ������ ������ ����\n",COLOR_TEXT_DEFAULT);
			if(MoveDir==FINDPATH_TOOFAR			) lpChosen->SetText("����������",COLOR_TEXT_DEFAULT);
			if(MoveDir==FINDPATH_ALREADY_HERE	) lpChosen->SetText("��� ���",COLOR_TEXT_DEFAULT);
		}

		break;
//	case ACTION_ACTIVATE_OBJECT:
//		break;
//	case ACTION_DACTIVATE_OBJECT:
//		break;
	case ACTION_USE_OBJ_ON_CRITTER:
	{
		if(lpChosen->a_obj->object->type==OBJ_TYPE_WEAPON && lpChosen->id==TargetID) break;

		crit_map::iterator it=critters.find(TargetID);
		if(it==critters.end()) break;

		BYTE AttDir=hf.FindTarget(lpChosen->hex_x,lpChosen->hex_y,(*it).second->hex_x,(*it).second->hex_y,lpChosen->GetMaxDistance());

		if(AttDir>5) 
		{
			if(AttDir==FINDTARGET_BARRIER		) lpChosen->SetText("� ���� �� ������",COLOR_TEXT_DEFAULT);
			if(AttDir==FINDPATH_ERROR			) WriteLog("!!!WORNING!!! ������ ������ ����\n",COLOR_TEXT_DEFAULT);
			if(AttDir==FINDPATH_TOOFAR			) lpChosen->SetText("����������",COLOR_TEXT_DEFAULT);
			if(AttDir==FINDTARGET_INVALID_TARG	) lpChosen->SetText("������������ ����",COLOR_TEXT_DEFAULT);

			break;
		}

		lpChosen->cur_dir=AttDir;

		if(lpChosen->a_obj->object->type==OBJ_TYPE_WEAPON)
		{
			if(lpChosen->a_obj->object->p111[OBJ_WEAP_TIME_ACTIV] && lpChosen->cond_ext!=COND_LIFE_ACTWEAP) break;

			float mod_TypeAttack=0;
			BYTE attack_skill=0;
			BYTE num_anim2=0;

			switch(lpChosen->rate_object)
			{
			case 1:
				attack_skill=lpChosen->a_obj->object->p111[OBJ_WEAP_PA_SKILL];
				mod_TypeAttack=lpChosen->a_obj->object->p111[OBJ_WEAP_PA_TIME]/1000;
				num_anim2=lpChosen->a_obj->object->p111[OBJ_WEAP_PA_ANIM2];
				break;
			case 2:
				attack_skill=lpChosen->a_obj->object->p111[OBJ_WEAP_SA_SKILL];
				mod_TypeAttack=lpChosen->a_obj->object->p111[OBJ_WEAP_SA_TIME]/1000;
				num_anim2=lpChosen->a_obj->object->p111[OBJ_WEAP_SA_ANIM2];
				break;
			case 3:
				attack_skill=lpChosen->a_obj->object->p111[OBJ_WEAP_TA_SKILL];
				mod_TypeAttack=lpChosen->a_obj->object->p111[OBJ_WEAP_TA_TIME]/1000;
				num_anim2=lpChosen->a_obj->object->p111[OBJ_WEAP_TA_ANIM2];
				break;
			default:
				SetChosenAction(ACTION_NONE);
				return;
			}

			if(!num_anim2) { SetChosenAction(ACTION_NONE); return; } //@@@cheat@@@

			float mod_TypeWeap=0;
			switch(attack_skill)
			{
			case SK_UNARMED:		mod_TypeWeap=0; break;
			case SK_THROWING:		mod_TypeWeap=0.1f; break;
			case SK_MELEE_WEAPONS:	mod_TypeWeap=0.3f; break;
			case SK_SMALL_GUNS:		mod_TypeWeap=2.0f; break;
			case SK_BIG_GUNS:		mod_TypeWeap=3.2f; break;
			case SK_ENERGY_WEAPONS: mod_TypeWeap=4.5f; break;
			default:				SetChosenAction(ACTION_NONE); return;
			}

			Net_SendUseObject(OBJ_TYPE_WEAPON,TargetID,lpChosen->cur_dir,ACT_USE_OBJ,lpChosen->rate_object);

			float tick_flt=(3.01f-lpChosen->sk[attack_skill]/100+mod_TypeWeap+mod_TypeAttack)*
				(13/6-lpChosen->st[ST_AGILITY]/6)*1000;

			lpChosen->Action(num_anim2,WORD(tick_flt));
		}
		else
		{
			Net_SendUseObject(lpChosen->a_obj->object->type,TargetID,lpChosen->cur_dir,ACT_USE_OBJ,lpChosen->rate_object);

			lpChosen->Action(12,1000);
		}

		break;
	}
	case ACTION_USE_OBJ_ON_ITEM:
		break;
	case ACTION_USE_SKL_ON_CRITTER:
		break;
	case ACTION_USE_SKL_ON_ITEM:
		break;
	case ACTION_TALK_NPC:
		if(!TargetID) break;
		Net_SendTalk(TargetID,0);
		break;
	}

	PathMoveX=lpChosen->hex_x;
	PathMoveY=lpChosen->hex_y;
//	TargetID=0;
	SetChosenAction(ACTION_NONE);
}

void CFEngine::CreateParamsMaps()
{
	stats_map.insert(params_map::value_type("ST_STRENGHT",				ST_STRENGHT));
	stats_map.insert(params_map::value_type("ST_PERCEPTION",			ST_PERCEPTION));
	stats_map.insert(params_map::value_type("ST_ENDURANCE",				ST_ENDURANCE));
	stats_map.insert(params_map::value_type("ST_CHARISMA",				ST_CHARISMA));
	stats_map.insert(params_map::value_type("ST_INTELLECT",				ST_INTELLECT));
	stats_map.insert(params_map::value_type("ST_AGILITY",				ST_AGILITY));
	stats_map.insert(params_map::value_type("ST_LUCK",					ST_LUCK));
	stats_map.insert(params_map::value_type("ST_MAX_LIFE",				ST_MAX_LIFE));
	stats_map.insert(params_map::value_type("ST_MAX_COND",				ST_MAX_COND));
	stats_map.insert(params_map::value_type("ST_ARMOR_CLASS",			ST_ARMOR_CLASS));
	stats_map.insert(params_map::value_type("ST_MELEE_DAMAGE",			ST_MELEE_DAMAGE));
	stats_map.insert(params_map::value_type("ST_WEAPON_DAMAGE",			ST_WEAPON_DAMAGE));
	stats_map.insert(params_map::value_type("ST_CARRY_WEIGHT",			ST_CARRY_WEIGHT));
	stats_map.insert(params_map::value_type("ST_SEQUENCE",				ST_SEQUENCE));
	stats_map.insert(params_map::value_type("ST_HEALING_RATE",			ST_HEALING_RATE));
	stats_map.insert(params_map::value_type("ST_CRITICAL_CHANCE",		ST_CRITICAL_CHANCE));
	stats_map.insert(params_map::value_type("ST_MAX_CRITICAL",			ST_MAX_CRITICAL));
	stats_map.insert(params_map::value_type("ST_INGURE_ABSORB",			ST_INGURE_ABSORB));
	stats_map.insert(params_map::value_type("ST_LASER_ABSORB",			ST_LASER_ABSORB));
	stats_map.insert(params_map::value_type("ST_FIRE_ABSORB",			ST_FIRE_ABSORB));
	stats_map.insert(params_map::value_type("ST_PLASMA_ABSORB",			ST_PLASMA_ABSORB));
	stats_map.insert(params_map::value_type("ST_ELECTRO_ABSORB",		ST_ELECTRO_ABSORB));
	stats_map.insert(params_map::value_type("ST_EMP_ABSORB",			ST_EMP_ABSORB));
	stats_map.insert(params_map::value_type("ST_BLAST_ABSORB",			ST_BLAST_ABSORB));
	stats_map.insert(params_map::value_type("ST_INGURE_RESIST",			ST_INGURE_RESIST));
	stats_map.insert(params_map::value_type("ST_LASER_RESIST",			ST_LASER_RESIST));
	stats_map.insert(params_map::value_type("ST_FIRE_RESIST",			ST_FIRE_RESIST));
	stats_map.insert(params_map::value_type("ST_PLASMA_RESIST",			ST_PLASMA_RESIST));
	stats_map.insert(params_map::value_type("ST_ELECTRO_RESIST",		ST_ELECTRO_RESIST));
	stats_map.insert(params_map::value_type("ST_EMP_RESIST",			ST_EMP_RESIST));
	stats_map.insert(params_map::value_type("ST_BLAST_RESIST",			ST_BLAST_RESIST));
	stats_map.insert(params_map::value_type("ST_RADIATION_RESISTANCE",	ST_RADIATION_RESISTANCE));
	stats_map.insert(params_map::value_type("ST_POISON_RESISTANCE",		ST_POISON_RESISTANCE));
	stats_map.insert(params_map::value_type("ST_AGE",					ST_AGE));
	stats_map.insert(params_map::value_type("ST_GENDER",				ST_GENDER));
	stats_map.insert(params_map::value_type("ST_CURRENT_HP",			ST_CURRENT_HP));
	stats_map.insert(params_map::value_type("ST_POISONING_LEVEL",		ST_POISONING_LEVEL));
	stats_map.insert(params_map::value_type("ST_RADIATION_LEVEL",		ST_RADIATION_LEVEL));
	stats_map.insert(params_map::value_type("ST_CURRENT_STANDART",		ST_CURRENT_STANDART));

	skills_map.insert(params_map::value_type("SK_SMALL_GUNS",			SK_SMALL_GUNS));
	skills_map.insert(params_map::value_type("SK_BIG_GUNS",				SK_BIG_GUNS));
	skills_map.insert(params_map::value_type("SK_ENERGY_WEAPONS",		SK_ENERGY_WEAPONS));
	skills_map.insert(params_map::value_type("SK_UNARMED",				SK_UNARMED));
	skills_map.insert(params_map::value_type("SK_MELEE_WEAPONS",		SK_MELEE_WEAPONS));
	skills_map.insert(params_map::value_type("SK_THROWING",				SK_THROWING));
	skills_map.insert(params_map::value_type("SK_FIRST_AID",			SK_FIRST_AID));
	skills_map.insert(params_map::value_type("SK_DOCTOR",				SK_DOCTOR));
	skills_map.insert(params_map::value_type("SK_SNEAK",				SK_SNEAK));
	skills_map.insert(params_map::value_type("SK_LOCKPICK",				SK_LOCKPICK));
	skills_map.insert(params_map::value_type("SK_STEAL",				SK_STEAL));
	skills_map.insert(params_map::value_type("SK_TRAPS",				SK_TRAPS));
	skills_map.insert(params_map::value_type("SK_SCIENCE",				SK_SCIENCE));
	skills_map.insert(params_map::value_type("SK_REPAIR",				SK_REPAIR));
	skills_map.insert(params_map::value_type("SK_SPEECH",				SK_SPEECH));
	skills_map.insert(params_map::value_type("SK_BARTER",				SK_BARTER));
	skills_map.insert(params_map::value_type("SK_GAMBLING",				SK_GAMBLING));
	skills_map.insert(params_map::value_type("SK_OUTDOORSMAN",			SK_OUTDOORSMAN));

	perks_map.insert(params_map::value_type("PE_FAST_METABOLISM",		PE_FAST_METABOLISM));
	perks_map.insert(params_map::value_type("PE_BRUISER",				PE_BRUISER));
	perks_map.insert(params_map::value_type("PE_SMALL_FRAME",			PE_SMALL_FRAME));
	perks_map.insert(params_map::value_type("PE_ONE_HANDER",			PE_ONE_HANDER));
	perks_map.insert(params_map::value_type("PE_FINESSE",				PE_FINESSE));
	perks_map.insert(params_map::value_type("PE_KAMIKAZE",				PE_KAMIKAZE));
	perks_map.insert(params_map::value_type("PE_HEAVY_HANDED",			PE_HEAVY_HANDED));
	perks_map.insert(params_map::value_type("PE_FAST_SHOT",				PE_FAST_SHOT));
	perks_map.insert(params_map::value_type("PE_BLOODY_MESS",			PE_BLOODY_MESS));
	perks_map.insert(params_map::value_type("PE_JINXED",				PE_JINXED));
	perks_map.insert(params_map::value_type("PE_GOOD_NATURED",			PE_GOOD_NATURED));
	perks_map.insert(params_map::value_type("PE_CHEM_RELIANT",			PE_CHEM_RELIANT));
	perks_map.insert(params_map::value_type("PE_CHEM_RESISTANT",		PE_CHEM_RESISTANT));
	perks_map.insert(params_map::value_type("PE_NIGHT_PERSON",			PE_NIGHT_PERSON));
	perks_map.insert(params_map::value_type("PE_SKILLED",				PE_SKILLED));
	perks_map.insert(params_map::value_type("PE_GIFTED",				PE_GIFTED));
	perks_map.insert(params_map::value_type("PE_AWARENESS",				PE_AWARENESS));
	perks_map.insert(params_map::value_type("PE_A_MELEE_ATT",			PE_A_MELEE_ATT));
	perks_map.insert(params_map::value_type("PE_A_MELEE_DAM",			PE_A_MELEE_DAM));
	perks_map.insert(params_map::value_type("PE_A_MOVE",				PE_A_MOVE));
	perks_map.insert(params_map::value_type("PE_A_DAM",					PE_A_DAM));
	perks_map.insert(params_map::value_type("PE_A_SPEED",				PE_A_SPEED));
	perks_map.insert(params_map::value_type("PE_PASS_FRONT",			PE_PASS_FRONT));
	perks_map.insert(params_map::value_type("PE_RAPID_HEAL",			PE_RAPID_HEAL));
	perks_map.insert(params_map::value_type("PE_MORE_CRIT_DAM",			PE_MORE_CRIT_DAM));
	perks_map.insert(params_map::value_type("PE_NIGHT_SIGHT",			PE_NIGHT_SIGHT));
	perks_map.insert(params_map::value_type("PE_PRESENCE",				PE_PRESENCE));
	perks_map.insert(params_map::value_type("PE_RES_NUKLEAR",			PE_RES_NUKLEAR));
	perks_map.insert(params_map::value_type("PE_ENDURENCE",				PE_ENDURENCE));
	perks_map.insert(params_map::value_type("PE_STR_BACK",				PE_STR_BACK));
	perks_map.insert(params_map::value_type("PE_MARKSMAN",				PE_MARKSMAN));
	perks_map.insert(params_map::value_type("PE_STEALHING",				PE_STEALHING));
	perks_map.insert(params_map::value_type("PE_LIFEFULL",				PE_LIFEFULL));
	perks_map.insert(params_map::value_type("PE_MERCHANT",				PE_MERCHANT));
	perks_map.insert(params_map::value_type("PE_FORMED",				PE_FORMED));
	perks_map.insert(params_map::value_type("PE_HEALER",				PE_HEALER));
	perks_map.insert(params_map::value_type("PE_TR_DIGGER",				PE_TR_DIGGER));
	perks_map.insert(params_map::value_type("PE_BEST_HITS",				PE_BEST_HITS));
	perks_map.insert(params_map::value_type("PE_COMPASION",				PE_COMPASION));
	perks_map.insert(params_map::value_type("PE_KILLER",				PE_KILLER));
	perks_map.insert(params_map::value_type("PE_SNIPER",				PE_SNIPER));
	perks_map.insert(params_map::value_type("PE_SILENT_DEATH",			PE_SILENT_DEATH));
	perks_map.insert(params_map::value_type("PE_C_FIGHTER",				PE_C_FIGHTER));
	perks_map.insert(params_map::value_type("PE_MIND_BLOCK",			PE_MIND_BLOCK));
	perks_map.insert(params_map::value_type("PE_PROLONGATION_LIFE",		PE_PROLONGATION_LIFE));
	perks_map.insert(params_map::value_type("PE_RECOURCEFULNESS",		PE_RECOURCEFULNESS));
	perks_map.insert(params_map::value_type("PE_SNAKE_EATER",			PE_SNAKE_EATER));
	perks_map.insert(params_map::value_type("PE_REPEARER",				PE_REPEARER));
	perks_map.insert(params_map::value_type("PE_MEDIC",					PE_MEDIC));
	perks_map.insert(params_map::value_type("PE_SKILLED_THIEF",			PE_SKILLED_THIEF));
	perks_map.insert(params_map::value_type("PE_SPEAKER",				PE_SPEAKER));
	perks_map.insert(params_map::value_type("PE_GUTCHER",				PE_GUTCHER));
	perks_map.insert(params_map::value_type("PE_UNKNOWN_1",				PE_UNKNOWN_1));
	perks_map.insert(params_map::value_type("PE_PICK_POCKER",			PE_PICK_POCKER));
	perks_map.insert(params_map::value_type("PE_GHOST",					PE_GHOST));
	perks_map.insert(params_map::value_type("PE_CHAR_CULT",				PE_CHAR_CULT));
	perks_map.insert(params_map::value_type("PE_THIFER",				PE_THIFER));
	perks_map.insert(params_map::value_type("PE_DISCOVER",				PE_DISCOVER));
	perks_map.insert(params_map::value_type("PE_OVERROAD",				PE_OVERROAD));
	perks_map.insert(params_map::value_type("PE_ANIMAL_FRIENDSHIP",		PE_ANIMAL_FRIENDSHIP));
	perks_map.insert(params_map::value_type("PE_SCOUT",					PE_SCOUT));
	perks_map.insert(params_map::value_type("PE_MIST_CHAR",				PE_MIST_CHAR));
	perks_map.insert(params_map::value_type("PE_RANGER",				PE_RANGER));
	perks_map.insert(params_map::value_type("PE_PICK_POCKET_2",			PE_PICK_POCKET_2));
	perks_map.insert(params_map::value_type("PE_INTERLOCUTER",			PE_INTERLOCUTER));
	perks_map.insert(params_map::value_type("PE_NOVICE",				PE_NOVICE));
	perks_map.insert(params_map::value_type("PE_PRIME_SKILL",			PE_PRIME_SKILL));
	perks_map.insert(params_map::value_type("PE_MUTATION",				PE_MUTATION));
	perks_map.insert(params_map::value_type("PE_NARC_NUKACOLA",			PE_NARC_NUKACOLA));
	perks_map.insert(params_map::value_type("PE_NARC_BUFFOUT",			PE_NARC_BUFFOUT));
	perks_map.insert(params_map::value_type("PE_NARC_MENTAT",			PE_NARC_MENTAT));
	perks_map.insert(params_map::value_type("PE_NARC_PSYHO",			PE_NARC_PSYHO));
	perks_map.insert(params_map::value_type("PE_NARC_RADAWAY",			PE_NARC_RADAWAY));
	perks_map.insert(params_map::value_type("PE_DISTANT_WEAP",			PE_DISTANT_WEAP));
	perks_map.insert(params_map::value_type("PE_ACCURARY_WEAP",			PE_ACCURARY_WEAP));
	perks_map.insert(params_map::value_type("PE_PENETRATION_WEAP",		PE_PENETRATION_WEAP));
	perks_map.insert(params_map::value_type("PE_KILLER_WEAP",			PE_KILLER_WEAP));
	perks_map.insert(params_map::value_type("PE_ENERGY_ARMOR",			PE_ENERGY_ARMOR));
	perks_map.insert(params_map::value_type("PE_BATTLE_ARMOR",			PE_BATTLE_ARMOR));
	perks_map.insert(params_map::value_type("PE_WEAP_RANGE",			PE_WEAP_RANGE));
	perks_map.insert(params_map::value_type("PE_RAPID_RELOAD",			PE_RAPID_RELOAD));
	perks_map.insert(params_map::value_type("PE_NIGHT_SPYGLASS",		PE_NIGHT_SPYGLASS));
	perks_map.insert(params_map::value_type("PE_FLAMER",				PE_FLAMER));
	perks_map.insert(params_map::value_type("PE_APA_I",					PE_APA_I));
	perks_map.insert(params_map::value_type("PE_APA_II",				PE_APA_II));
	perks_map.insert(params_map::value_type("PE_FORCEAGE",				PE_FORCEAGE));
	perks_map.insert(params_map::value_type("PE_DEADLY_NARC",			PE_DEADLY_NARC));
	perks_map.insert(params_map::value_type("PE_CHARMOLEANCE",			PE_CHARMOLEANCE));
	perks_map.insert(params_map::value_type("PE_GEKK_SKINER",			PE_GEKK_SKINER));
	perks_map.insert(params_map::value_type("PE_SKIN_ARMOR",			PE_SKIN_ARMOR));
	perks_map.insert(params_map::value_type("PE_A_SKIN_ARMOR",			PE_A_SKIN_ARMOR));
	perks_map.insert(params_map::value_type("PE_SUPER_ARMOR",			PE_SUPER_ARMOR));
	perks_map.insert(params_map::value_type("PE_A_SUPER_ARMOR",			PE_A_SUPER_ARMOR));
	perks_map.insert(params_map::value_type("PE_VAULT_INOCUL",			PE_VAULT_INOCUL));
	perks_map.insert(params_map::value_type("PE_ADRENALIN_RUSH",		PE_ADRENALIN_RUSH));
	perks_map.insert(params_map::value_type("PE_CAREFULL",				PE_CAREFULL));
	perks_map.insert(params_map::value_type("PE_INTELEGENCE",			PE_INTELEGENCE));
	perks_map.insert(params_map::value_type("PE_PYROKASY",				PE_PYROKASY));
	perks_map.insert(params_map::value_type("PE_DUDE",					PE_DUDE));
	perks_map.insert(params_map::value_type("PE_A_STR",					PE_A_STR));
	perks_map.insert(params_map::value_type("PE_A_PER",					PE_A_PER));
	perks_map.insert(params_map::value_type("PE_A_END",					PE_A_END));
	perks_map.insert(params_map::value_type("PE_A_CHA",					PE_A_CHA));
	perks_map.insert(params_map::value_type("PE_A_INT",					PE_A_INT));
	perks_map.insert(params_map::value_type("PE_A_AGL",					PE_A_AGL));
	perks_map.insert(params_map::value_type("PE_A_LUC",					PE_A_LUC));
	perks_map.insert(params_map::value_type("PE_PURERER",				PE_PURERER));
	perks_map.insert(params_map::value_type("PE_IMAG",					PE_IMAG));
	perks_map.insert(params_map::value_type("PE_EVASION",				PE_EVASION));
	perks_map.insert(params_map::value_type("PE_DROSHKADRAT",			PE_DROSHKADRAT));
	perks_map.insert(params_map::value_type("PE_KARMA_GLOW",			PE_KARMA_GLOW));
	perks_map.insert(params_map::value_type("PE_SILENT_STEPS",			PE_SILENT_STEPS));
	perks_map.insert(params_map::value_type("PE_ANATOMY",				PE_ANATOMY));
	perks_map.insert(params_map::value_type("PE_CHAMER",				PE_CHAMER));
	perks_map.insert(params_map::value_type("PE_ORATOR",				PE_ORATOR));
	perks_map.insert(params_map::value_type("PE_PACKER",				PE_PACKER));
	perks_map.insert(params_map::value_type("PE_EDD_GAYAN_MANIAC",		PE_EDD_GAYAN_MANIAC));
	perks_map.insert(params_map::value_type("PE_FAST_REGENERATION",		PE_FAST_REGENERATION));
	perks_map.insert(params_map::value_type("PE_VENDOR",				PE_VENDOR));
	perks_map.insert(params_map::value_type("PE_STONE_WALL",			PE_STONE_WALL));
	perks_map.insert(params_map::value_type("PE_THIEF_AGAIN",			PE_THIEF_AGAIN));
	perks_map.insert(params_map::value_type("PE_WEAPON_SKILL",			PE_WEAPON_SKILL));
	perks_map.insert(params_map::value_type("PE_MAKE_VAULT",			PE_MAKE_VAULT));
	perks_map.insert(params_map::value_type("PE_ALC_BUFF_1",			PE_ALC_BUFF_1));
	perks_map.insert(params_map::value_type("PE_ALC_BUFF_2",			PE_ALC_BUFF_2));
/*	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	perks_map.insert(params_map::value_type("!",!));
	*/
	perks_map.insert(params_map::value_type("PE_HIDE_MODE",				PE_HIDE_MODE));

	object_map.insert(params_map::value_type("OBJ_TYPE_ARMOR",			OBJ_TYPE_ARMOR));
	object_map.insert(params_map::value_type("OBJ_TYPE_CONTAINER",		OBJ_TYPE_CONTAINER));
	object_map.insert(params_map::value_type("OBJ_TYPE_DRUG",			OBJ_TYPE_DRUG));
	object_map.insert(params_map::value_type("OBJ_TYPE_WEAPON",			OBJ_TYPE_WEAPON));
	object_map.insert(params_map::value_type("OBJ_TYPE_AMMO",			OBJ_TYPE_AMMO));
	object_map.insert(params_map::value_type("OBJ_TYPE_MISC",			OBJ_TYPE_MISC));
	object_map.insert(params_map::value_type("OBJ_TYPE_KEY",			OBJ_TYPE_KEY));
	object_map.insert(params_map::value_type("OBJ_NAME",				OBJ_NAME));
	object_map.insert(params_map::value_type("OBJ_INFO",				OBJ_INFO));
	object_map.insert(params_map::value_type("OBJ_TIME_SHOW",			OBJ_TIME_SHOW));
	object_map.insert(params_map::value_type("OBJ_TIME_HIDE",			OBJ_TIME_HIDE));
	object_map.insert(params_map::value_type("OBJ_DISTANCE_LIGHT",		OBJ_DISTANCE_LIGHT));
	object_map.insert(params_map::value_type("OBJ_INTENSITY_LIGHT",		OBJ_INTENSITY_LIGHT));
	object_map.insert(params_map::value_type("OBJ_PASSED",				OBJ_PASSED));
	object_map.insert(params_map::value_type("OBJ_RAKED",				OBJ_RAKED));
	object_map.insert(params_map::value_type("OBJ_TRANSPARENT",			OBJ_TRANSPARENT));
	object_map.insert(params_map::value_type("OBJ_CAN_USE",				OBJ_CAN_USE));
	object_map.insert(params_map::value_type("OBJ_CAN_PICK_UP",			OBJ_CAN_PICK_UP));
	object_map.insert(params_map::value_type("OBJ_CAN_USE_ON_SMTH",		OBJ_CAN_USE_ON_SMTH));
	object_map.insert(params_map::value_type("OBJ_HIDDEN",				OBJ_HIDDEN));
	object_map.insert(params_map::value_type("OBJ_WEIGHT",				OBJ_WEIGHT));
	object_map.insert(params_map::value_type("OBJ_SIZE",				OBJ_SIZE));
	object_map.insert(params_map::value_type("OBJ_TWOHANDS",			OBJ_TWOHANDS));
	object_map.insert(params_map::value_type("OBJ_PIC_MAP",				OBJ_PIC_MAP));
	object_map.insert(params_map::value_type("OBJ_PIC_INV",				OBJ_PIC_INV));
	object_map.insert(params_map::value_type("OBJ_SOUND",				OBJ_SOUND));
	object_map.insert(params_map::value_type("OBJ_LIVETIME",			OBJ_LIVETIME));
	object_map.insert(params_map::value_type("OBJ_COST",				OBJ_COST));
	object_map.insert(params_map::value_type("OBJ_MATERIAL",			OBJ_MATERIAL));
	object_map.insert(params_map::value_type("OBJ_ARM_ANIM0_MALE",		OBJ_ARM_ANIM0_MALE));
	object_map.insert(params_map::value_type("OBJ_ARM_ANIM0_MALE2",		OBJ_ARM_ANIM0_MALE2));
	object_map.insert(params_map::value_type("OBJ_ARM_ANIM0_FEMALE",	OBJ_ARM_ANIM0_FEMALE));
	object_map.insert(params_map::value_type("OBJ_ARM_ANIM0_FEMALE2",	OBJ_ARM_ANIM0_FEMALE2));
	object_map.insert(params_map::value_type("OBJ_ARM_AC",				OBJ_ARM_AC));
	object_map.insert(params_map::value_type("OBJ_ARM_PERK",			OBJ_ARM_PERK));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_NORMAL",		OBJ_ARM_DR_NORMAL));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_LASER",		OBJ_ARM_DR_LASER));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_FIRE",			OBJ_ARM_DR_FIRE));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_PLASMA",		OBJ_ARM_DR_PLASMA));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_ELECTR",		OBJ_ARM_DR_ELECTR));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_EMP",			OBJ_ARM_DR_EMP));
	object_map.insert(params_map::value_type("OBJ_ARM_DR_EXPLODE",		OBJ_ARM_DR_EXPLODE));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_NORMAL",		OBJ_ARM_DT_NORMAL));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_LASER",		OBJ_ARM_DT_LASER));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_FIRE",			OBJ_ARM_DT_FIRE));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_PLASMA",		OBJ_ARM_DT_PLASMA));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_ELECTR",		OBJ_ARM_DT_ELECTR));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_EMP",			OBJ_ARM_DT_EMP));
	object_map.insert(params_map::value_type("OBJ_ARM_DT_EXPLODE",		OBJ_ARM_DT_EXPLODE));
	object_map.insert(params_map::value_type("OBJ_CONT_SIZE",			OBJ_CONT_SIZE));
	object_map.insert(params_map::value_type("OBJ_CONT_FLAG",			OBJ_CONT_FLAG));
	object_map.insert(params_map::value_type("OBJ_DRUG_STAT0",			OBJ_DRUG_STAT0));
	object_map.insert(params_map::value_type("OBJ_DRUG_STAT1",			OBJ_DRUG_STAT1));
	object_map.insert(params_map::value_type("OBJ_DRUG_STAT2",			OBJ_DRUG_STAT2));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT0_S0",		OBJ_DRUG_AMOUNT0_S0));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT0_S1",		OBJ_DRUG_AMOUNT0_S1));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT0_S2",		OBJ_DRUG_AMOUNT0_S2));
	object_map.insert(params_map::value_type("OBJ_DRUG_DURATION1",		OBJ_DRUG_DURATION1));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT1_S0",		OBJ_DRUG_AMOUNT1_S0));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT1_S1",		OBJ_DRUG_AMOUNT1_S1));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT1_S2",		OBJ_DRUG_AMOUNT1_S2));
	object_map.insert(params_map::value_type("OBJ_DRUG_DURATION2",		OBJ_DRUG_DURATION2));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT2_S0",		OBJ_DRUG_AMOUNT2_S0));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT2_S1",		OBJ_DRUG_AMOUNT2_S1));
	object_map.insert(params_map::value_type("OBJ_DRUG_AMOUNT2_S2",		OBJ_DRUG_AMOUNT2_S2));
	object_map.insert(params_map::value_type("OBJ_DRUG_ADDICTION",		OBJ_DRUG_ADDICTION));
	object_map.insert(params_map::value_type("OBJ_DRUG_W_EFFECT",		OBJ_DRUG_W_EFFECT));
	object_map.insert(params_map::value_type("OBJ_DRUG_W_ONSET",		OBJ_DRUG_W_ONSET));
	object_map.insert(params_map::value_type("OBJ_WEAP_ANIM1",			OBJ_WEAP_ANIM1));
	object_map.insert(params_map::value_type("OBJ_WEAP_TIME_ACTIV",		OBJ_WEAP_TIME_ACTIV));
	object_map.insert(params_map::value_type("OBJ_WEAP_TIME_UNACTIV",	OBJ_WEAP_TIME_UNACTIV));
	object_map.insert(params_map::value_type("OBJ_WEAP_VOL_HOLDER",		OBJ_WEAP_VOL_HOLDER));
	object_map.insert(params_map::value_type("OBJ_WEAP_CALIBER",		OBJ_WEAP_CALIBER));
	object_map.insert(params_map::value_type("OBJ_WEAP_VOL_HOLDER_E",	OBJ_WEAP_VOL_HOLDER_E));
	object_map.insert(params_map::value_type("OBJ_WEAP_CALIBER_E",		OBJ_WEAP_CALIBER_E));
	object_map.insert(params_map::value_type("OBJ_WEAP_CR_FAILTURE",	OBJ_WEAP_CR_FAILTURE));
	object_map.insert(params_map::value_type("OBJ_WEAP_TYPE_ATTACK",	OBJ_WEAP_TYPE_ATTACK));
	object_map.insert(params_map::value_type("OBJ_WEAP_COUNT_ATTACK",	OBJ_WEAP_COUNT_ATTACK));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_SKILL",		OBJ_WEAP_PA_SKILL));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_HOLDER",		OBJ_WEAP_PA_HOLDER));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_PIC",			OBJ_WEAP_PA_PIC));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_DMG_MIN",		OBJ_WEAP_PA_DMG_MIN));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_DMG_MAX",		OBJ_WEAP_PA_DMG_MAX));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_MAX_DIST",	OBJ_WEAP_PA_MAX_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_EFF_DIST",	OBJ_WEAP_PA_EFF_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_ANIM2",		OBJ_WEAP_PA_ANIM2));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_TIME",		OBJ_WEAP_PA_TIME));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_AIM",			OBJ_WEAP_PA_AIM));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_ROUND",		OBJ_WEAP_PA_ROUND));
	object_map.insert(params_map::value_type("OBJ_WEAP_PA_REMOVE",		OBJ_WEAP_PA_REMOVE));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_SKILL",		OBJ_WEAP_SA_SKILL));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_HOLDER",		OBJ_WEAP_SA_HOLDER));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_PIC",			OBJ_WEAP_SA_PIC));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_DMG_MIN",		OBJ_WEAP_SA_DMG_MIN));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_DMG_MAX",		OBJ_WEAP_SA_DMG_MAX));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_MAX_DIST",	OBJ_WEAP_SA_MAX_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_EFF_DIST",	OBJ_WEAP_SA_EFF_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_ANIM2",		OBJ_WEAP_SA_ANIM2));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_TIME",		OBJ_WEAP_SA_TIME));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_AIM",			OBJ_WEAP_SA_AIM));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_ROUND",		OBJ_WEAP_SA_ROUND));
	object_map.insert(params_map::value_type("OBJ_WEAP_SA_REMOVE",		OBJ_WEAP_SA_REMOVE));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_SKILL",		OBJ_WEAP_TA_SKILL));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_HOLDER",		OBJ_WEAP_TA_HOLDER));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_PIC",			OBJ_WEAP_TA_PIC));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_DMG_MIN",		OBJ_WEAP_TA_DMG_MIN));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_DMG_MAX",		OBJ_WEAP_TA_DMG_MAX));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_MAX_DIST",	OBJ_WEAP_TA_MAX_DIST));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_EFF_DIST",	OBJ_WEAP_TA_EFF_DIST));	
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_ANIM2",		OBJ_WEAP_TA_ANIM2));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_TIME",		OBJ_WEAP_TA_TIME));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_AIM",			OBJ_WEAP_TA_AIM));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_ROUND",		OBJ_WEAP_TA_ROUND));
	object_map.insert(params_map::value_type("OBJ_WEAP_TA_REMOVE",		OBJ_WEAP_TA_REMOVE));
	object_map.insert(params_map::value_type("OBJ_AMMO_CALIBER",		OBJ_AMMO_CALIBER));
	object_map.insert(params_map::value_type("OBJ_AMMO_TYPE_DAMAGE",	OBJ_AMMO_TYPE_DAMAGE));
	object_map.insert(params_map::value_type("OBJ_AMMO_QUANTITY",		OBJ_AMMO_QUANTITY));
	object_map.insert(params_map::value_type("OBJ_AMMO_AC",				OBJ_AMMO_AC));
	object_map.insert(params_map::value_type("OBJ_AMMO_DR",				OBJ_AMMO_DR));
	object_map.insert(params_map::value_type("OBJ_AMMO_DM",				OBJ_AMMO_DM));
	object_map.insert(params_map::value_type("OBJ_AMMO_DD",				OBJ_AMMO_DD));
}
// !Cvet ----------------------------------------------------------------