#include "stdafx.h"

#include "CSpriteManager.h"
#include "common.h"

//#define TEX_FRMT D3DFMT_A1R5G5B5
//#define TEX_FRMT D3DFMT_A4R4G4B4
#define TEX_FRMT D3DFMT_A8R8G8B8

#include "frmload.h"
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

CSpriteManager::CSpriteManager(): crtd(0),spr_cnt(0),cur_pos(0),
	lpDevice(NULL),lpVB(NULL),lpIB(NULL),lpWaitBuf(NULL),last_surf(NULL),next_id(1),cur_surf(NULL)
{
	col=D3DCOLOR_ARGB(255,128,128,128);
}

int CSpriteManager::Init(LPDIRECT3DDEVICE8 lpD3Device)
{
	if(crtd) return 0; //������������ � ������ ��������� ������ ����� ������������ ������.
	WriteLog("CSpriteManager Initialization...\n");

	spr_cnt=opt_flushval;
	cur_pos=0;

	lpDevice=lpD3Device;

	//������� ����� ������
	WriteLog("������ VB �� %d ��������\n",spr_cnt);
	HRESULT hr=lpDevice->CreateVertexBuffer(spr_cnt*4*sizeof(MYVERTEX),D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
		D3DFVF_MYVERTEX,D3DPOOL_DEFAULT,&lpVB);
	if(hr!=D3D_OK){
		ErrMsg("SM::CreateVertexBuffer",(char*)DXGetErrorString8(hr));
		return 0;
	}

	//� ��������
	hr=lpDevice->CreateIndexBuffer(spr_cnt*6*sizeof(WORD),D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,D3DPOOL_DEFAULT,&lpIB);
	if(hr!=D3D_OK){
		ErrMsg("SM::CreateIndexBuffer",(char*)DXGetErrorString8(hr));
		return 0;
	}
	

	WORD* IndexList=new WORD[6*spr_cnt];
	for(int i=0;i<spr_cnt;i++)
	{
		IndexList[6*i+0]=4*i+0;
		IndexList[6*i+1]=4*i+1;
		IndexList[6*i+2]=4*i+3;
		IndexList[6*i+3]=4*i+1;
		IndexList[6*i+4]=4*i+2;
		IndexList[6*i+5]=4*i+3;
	}
	
	void* pBuffer;
	lpIB->Lock(0,0,(BYTE**)&pBuffer,0);
		memcpy(pBuffer,IndexList,spr_cnt*6*sizeof(WORD));
	lpIB->Unlock();

	delete[] IndexList;

	lpDevice->SetIndices(lpIB,0);
	lpDevice->SetStreamSource(0,lpVB,sizeof(MYVERTEX));
	lpDevice->SetVertexShader(D3DFVF_MYVERTEX);


	lpWaitBuf=new MYVERTEX[spr_cnt*4];

	if(!fm.Init()) return 0;

	if(!LoadCritTypes()) return 0; //!Cvet

	WriteLog("CSpriteManager Initialization complete\n");
	crtd=1;
	return 1;
}

void CSpriteManager::Clear()
{
	WriteLog("CSprMan Clear...\n");

	fm.Clear();

	for(surf_vect::iterator it=surf_list.begin();it!=surf_list.end();it++)
		(*it)->Release();
	surf_list.clear();
	
	for(sprinfo_map::iterator ii=spr_data.begin();ii!=spr_data.end();ii++)
		delete (*ii).second;
	spr_data.clear();
	
	for(onesurf_vec::iterator iv=call_vec.begin();iv!=call_vec.end();iv++)
		delete (*iv);
	call_vec.clear();

	SAFEREL(lpVB);
	SAFEREL(lpIB);
	SAFEDELA(lpWaitBuf);

	crtd=0;
	WriteLog("CSprMan Clear complete\n");
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
int CSpriteManager::LoadMiniSprite(char *fname,double size,int PathType,SpriteInfo** ppInfo)
{
	if(!crtd) return 0;
	if(!fname[0]) return 0;

	if(!fm.LoadFile(fname,PathType))
		return 0;

	SpriteInfo* lpinf=new SpriteInfo;

	short offs_x, offs_y;
	fm.SetCurPos(0xA);
	offs_x=fm.GetWord();
	fm.SetCurPos(0x16);
	offs_y=fm.GetWord();

	lpinf->offs_x=offs_x;
	lpinf->offs_y=offs_y;


	fm.SetCurPos(0x3e);
	WORD w=fm.GetWord();
	WORD h=fm.GetWord();
	lpinf->w=w;
	lpinf->h=h;
	if(!last_surf)
	{
		lpinf->lpSurf=CreateNewSurf(w,h);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,w,h);
		last_surf=lpinf->lpSurf;
	}
	else if( (last_w-free_x)>=w && (last_h-free_y)>=h )
	{
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
	}
	else if( last_w>=w && (last_h-busy_h)>=h )
	{
		free_x=0;
		free_y=busy_h;
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
	}
	else
	{
		lpinf->lpSurf=CreateNewSurf(w,h);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,w,h);
		last_surf=lpinf->lpSurf;
	}

	int aligned_width = (4 - w%4)%4;

	// let's write the bmp 
	DWORD wpos=sizeof(bmpheader);
	BYTE* res=new BYTE[wpos+h*(w+aligned_width)];
	memcpy(res,bmpheader,wpos);
			
	
	DWORD ptr = 0x4A+w*(h-1);
	for(int i=0;i < h; i++) 
	{
		fm.SetCurPos(ptr);
		fm.CopyMem(res+wpos,w);
		wpos+=w;
		memset(res+wpos,0,aligned_width);
		wpos+=aligned_width;
		ptr -= w;
	}

	DWORD* spos=(DWORD*)(res+18);
	spos[0]=w;
	spos[1]=h;


	LPDIRECT3DSURFACE8 lpsurf;
	LPDIRECT3DSURFACE8 lptexsurf;
	//HRESULT hr=lpDevice->CreateImageSurface(w,h,D3DFMT_A8R8G8B8,&lpsurf);
	HRESULT hr=lpDevice->CreateImageSurface(w,h,TEX_FRMT,&lpsurf);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","�� ���� ������� Surface ��� ����� %s",fname);
		return 0;
	}  

	last_surf->GetSurfaceLevel(0,&lptexsurf);

	hr=D3DXLoadSurfaceFromFileInMemory(lpsurf,NULL,NULL,res,wpos,NULL,D3DX_FILTER_NONE,D3DCOLOR_ARGB(255,0,0,0),NULL);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","�� ���� ��������� Surface �� ������ ��� ����� %s",fname);
		return 0;
	} 
			
	POINT p={free_x,free_y};
	RECT r={0,0,w,h};

	lpDevice->CopyRects(lpsurf,&r,1,lptexsurf,&p);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","������ ��� ����������� ������������ ��� ����� %s",fname);
		return 0;
	}  

	lpsurf->Release();
	lptexsurf->Release();

	delete[] res;

	free_x+=w;
	if((free_y+h)>busy_h) busy_h=free_y+h;
	lpinf->spr_rect.x1/=last_w;
	lpinf->spr_rect.x2/=last_w;
	lpinf->spr_rect.y1/=last_h;
	lpinf->spr_rect.y2/=last_h;
    lpinf->h/=size; // ��������� ��������� ��������� ��� ���������
    lpinf->w/=size; // ����������� ���������������
    
	WriteLog("size %d",size);

	spr_data[next_id++]=lpinf;

	fm.UnloadFile();

	if(ppInfo) (*ppInfo)=lpinf;

	return next_id-1;
}

int CSpriteManager::LoadSprite(char *fname,int PathType,SpriteInfo** ppInfo) //!Cvet ������������
{
	if(!crtd) return 0;
	if(!fname[0]) return 0;

	if(!fm.LoadFile(fname,PathType))
		return 0;

	char* ext=strstr(fname,".");

	if(!ext)
	{
		fm.UnloadFile();
		WriteLog("��� ���������� � �����:|%s|\n",fname);
		return 0;
	}

	if(stricmp(ext,".frm")) return LoadSpriteAlt(fname,PathType);

	SpriteInfo* lpinf=new SpriteInfo;

	short offs_x, offs_y;
	fm.SetCurPos(0xA);
	offs_x=fm.GetWord();
	fm.SetCurPos(0x16);
	offs_y=fm.GetWord();

	lpinf->offs_x=offs_x;
	lpinf->offs_y=offs_y;


	fm.SetCurPos(0x3e);
	WORD w=fm.GetWord();
	WORD h=fm.GetWord();
	lpinf->w=w;
	lpinf->h=h;
	if(!last_surf)
	{
		lpinf->lpSurf=CreateNewSurf(w,h);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,w,h);
		last_surf=lpinf->lpSurf;
	}
	else if( (last_w-free_x)>=w && (last_h-free_y)>=h )
	{
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
	}
	else if( last_w>=w && (last_h-busy_h)>=h )
	{
		free_x=0;
		free_y=busy_h;
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
	}
	else
	{
		lpinf->lpSurf=CreateNewSurf(w,h);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,w,h);
		last_surf=lpinf->lpSurf;
	}

	int aligned_width = (4 - w%4)%4;

	// let's write the bmp 
	DWORD wpos=sizeof(bmpheader);
	BYTE* res=new BYTE[wpos+h*(w+aligned_width)];
	memcpy(res,bmpheader,wpos);
			
	
	DWORD ptr = 0x4A+w*(h-1);
	for(int i=0;i < h; i++) 
	{
		fm.SetCurPos(ptr);
		fm.CopyMem(res+wpos,w);
		wpos+=w;
		memset(res+wpos,0,aligned_width);
		wpos+=aligned_width;
		ptr -= w;
	}

	DWORD* spos=(DWORD*)(res+18);
	spos[0]=w;
	spos[1]=h;


	LPDIRECT3DSURFACE8 lpsurf;
	LPDIRECT3DSURFACE8 lptexsurf;
	//HRESULT hr=lpDevice->CreateImageSurface(w,h,D3DFMT_A8R8G8B8,&lpsurf);
	HRESULT hr=lpDevice->CreateImageSurface(w,h,TEX_FRMT,&lpsurf);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","�� ���� ������� Surface ��� ����� %s",fname);
		return 0;
	}  

	last_surf->GetSurfaceLevel(0,&lptexsurf);

	hr=D3DXLoadSurfaceFromFileInMemory(lpsurf,NULL,NULL,res,wpos,NULL,D3DX_FILTER_NONE,D3DCOLOR_ARGB(255,0,0,0),NULL);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","�� ���� ��������� Surface �� ������ ��� ����� %s",fname);
		return 0;
	} 
			
	POINT p={free_x,free_y};
	RECT r={0,0,w,h};

	lpDevice->CopyRects(lpsurf,&r,1,lptexsurf,&p);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","������ ��� ����������� ������������ ��� ����� %s",fname);
		return 0;
	}  

	lpsurf->Release();
	lptexsurf->Release();

	delete[] res;

	free_x+=w;
	if((free_y+h)>busy_h) busy_h=free_y+h;
	lpinf->spr_rect.x1/=last_w;
	lpinf->spr_rect.x2/=last_w;
	lpinf->spr_rect.y1/=last_h;
	lpinf->spr_rect.y2/=last_h;

	spr_data[next_id++]=lpinf;

	fm.UnloadFile();

	if(ppInfo) (*ppInfo)=lpinf;

	return next_id-1;
}

//!Cvet +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int CSpriteManager::LoadSpriteAlt(char *fname,int PathType,SpriteInfo** ppInfo)
{
	char* ext=strstr(fname,".");

	if(!ext)
	{
		fm.UnloadFile();
		WriteLog("��� ���������� � �����:|%s|\n",fname);
		return 0;
	}

	SpriteInfo* lpinf=new SpriteInfo;
	DWORD w=0;
	DWORD h=0;

//.bmp+, .dds-, .dib-, .hdr-, .jpg+, .pfm-, .png+, .ppm-, .tga-

	if(!stricmp(ext,".bmp"))
	{
		fm.SetCurPos(0x12); //�������� ��� ��������
		w=fm.GetRDWord(); //���� �������� �������� �������
		h=fm.GetRDWord();	
	}
	else if(!stricmp(ext,".png"))
	{
		fm.SetCurPos(0x10);
		w=fm.GetDWord();
		h=fm.GetDWord();
	}
	else if(!stricmp(ext,".jpg") || !stricmp(ext,".jpeg"))
	{
		fm.SetCurPos(0xBB);
		h=fm.GetWord();
		w=fm.GetWord();
	}
	else
	{
		SAFEDEL(lpinf);
		fm.UnloadFile();
		WriteLog("����������� ������ �����:|%s|\n",ext);
		return 0;
	}

	fm.UnloadFile();

	lpinf->w=w;
	lpinf->h=h;

	lpinf->offs_x=0;
	lpinf->offs_y=0;

	if(!last_surf)
	{
		lpinf->lpSurf=CreateNewSurf(w,h);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,w,h);
		last_surf=lpinf->lpSurf;
	}
	else if( (last_w-free_x)>=w && (last_h-free_y)>=h )
	{
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
	}
	else if( last_w>=w && (last_h-busy_h)>=h )
	{
		free_x=0;
		free_y=busy_h;
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
	}
	else
	{
		lpinf->lpSurf=CreateNewSurf(w,h);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,w,h);
		last_surf=lpinf->lpSurf;
	}

	LPDIRECT3DSURFACE8 lpsurf;
	LPDIRECT3DSURFACE8 lptexsurf;
	//HRESULT hr=lpDevice->CreateImageSurface(w,h,D3DFMT_A8R8G8B8,&lpsurf);
	HRESULT hr=lpDevice->CreateImageSurface(w,h,TEX_FRMT,&lpsurf);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","�� ���� ������� Surface ��� ����� %s",fname);
		return 0;
	}

	last_surf->GetSurfaceLevel(0,&lptexsurf);

	char full_path[1024];
	if(!fm.GetFullPath(fname,PathType,full_path)) return 0;

	hr=D3DXLoadSurfaceFromFile(lpsurf,NULL,NULL,full_path,NULL,D3DX_FILTER_NONE,D3DCOLOR_ARGB(255,0,0,255),NULL);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","�� ���� ��������� Surface ��� ����� %s",fname);
		return 0;
	}
			
	POINT p={free_x,free_y};
	RECT r={0,0,w,h};

	lpDevice->CopyRects(lpsurf,&r,1,lptexsurf,&p);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","������ ��� ����������� ������������ ��� ����� %s",fname);
		return 0;
	}

	lpsurf->Release();
	lptexsurf->Release();

	free_x+=w;
	if((free_y+h)>busy_h) busy_h=free_y+h;
	lpinf->spr_rect.x1/=last_w;
	lpinf->spr_rect.x2/=last_w;
	lpinf->spr_rect.y1/=last_h;
	lpinf->spr_rect.y2/=last_h;

	spr_data[next_id++]=lpinf;

	if(ppInfo) (*ppInfo)=lpinf;

	return next_id-1;
}
//!Cvet -----------------------------------------------------------------------

int CSpriteManager::LoadAnimation(char *fname,int PathType,CritFrames* pframes)
{
	if(!crtd) return 0;
	if(!fname[0]) return 0;

//	WriteLog("Loading animation %s\n",fname);

	if(!fm.LoadFile(fname,PathType))
		return 0;

	fm.SetCurPos(0x4); //!Cvet
	WORD frm_fps=fm.GetWord(); //!Cvet
	if(!frm_fps) frm_fps=10; //!Cvet

	fm.SetCurPos(0x8);
	WORD frm_num=fm.GetWord();

	//�������� ����� �������� �� ���� ������������
	short offs_x[6], offs_y[6];
	fm.SetCurPos(0xA);
	for(int i=0;i<6;i++)
		offs_x[i]=fm.GetWord();
	fm.SetCurPos(0x16);
	for(i=0;i<6;i++)
		offs_y[i]=fm.GetWord();

	pframes->ticks=1000/frm_fps*frm_num; //!Cvet
	pframes->cnt_frames=frm_num;
	pframes->ind=new WORD[frm_num*6];
	pframes->next_x=new short[frm_num*6];
	pframes->next_y=new short[frm_num*6];
	for(i=0;i<6;i++)
		pframes->dir_offs[i]=i*frm_num;

	DWORD cur_ptr=0x3E;
	for(int or=0;or<6;or++)
		for(int frm=0;frm<frm_num;frm++)
		{
			SpriteInfo* lpinf=new SpriteInfo;
			lpinf->offs_x=offs_x[or];
			lpinf->offs_y=offs_y[or];
		
			fm.SetCurPos(cur_ptr);
			WORD w=fm.GetWord();
			WORD h=fm.GetWord();
			lpinf->w=w;
			lpinf->h=h;
			fm.GoForward(4);
			pframes->next_x[or*frm_num+frm]=fm.GetWord();
			pframes->next_y[or*frm_num+frm]=fm.GetWord();
	
			if(!last_surf)
			{
				//������ ����� ������� � ����� �����������
				lpinf->lpSurf=CreateNewSurf(w,h);
				if(!lpinf->lpSurf) return 0;
				lpinf->spr_rect(0,0,w,h);
				last_surf=lpinf->lpSurf;
			}
			else if( (last_w-free_x)>=w && (last_h-free_y)>=h )
			{
				//������ ����� ������� � �������������� ���
				lpinf->lpSurf=last_surf;
				lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
			}
			else if( last_w>=w && (last_h-busy_h)>=h )
			{
				//�������� ����� �������������� ��� ���� ��������
				free_x=0;
				free_y=busy_h;
				lpinf->lpSurf=last_surf;
				lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
			}
			else
			{
				//������ ������ �� ���������� �� ��� �����������
				lpinf->lpSurf=CreateNewSurf(w,h);
				if(!lpinf->lpSurf) return 0;
				lpinf->spr_rect(0,0,w,h);
				last_surf=lpinf->lpSurf;
			}

			int aligned_width = (4 - w%4)%4;

			// ������� ���� ������� bmp � ������ 
			DWORD wpos=sizeof(bmpheader);
			BYTE* res=new BYTE[wpos+h*(w+aligned_width)];
			memcpy(res,bmpheader,wpos);
			
	
			DWORD ptr = cur_ptr+12+w*(h-1);
			cur_ptr+=w*h+12;
			for(int i=0;i < h; i++) 
			{
				fm.SetCurPos(ptr);
				fm.CopyMem(res+wpos,w);
				wpos+=w;
				memset(res+wpos,0,aligned_width);
				wpos+=aligned_width;
				ptr -= w;
			}

			DWORD* spos=(DWORD*)(res+18);
			spos[0]=w;
			spos[1]=h;


			//� ������ ��������� ��� �� ��������� �����������
			LPDIRECT3DSURFACE8 lpsurf;
			LPDIRECT3DSURFACE8 lptexsurf;
//			HRESULT hr=lpDevice->CreateImageSurface(w,h,D3DFMT_A8R8G8B8,&lpsurf);
			HRESULT hr=lpDevice->CreateImageSurface(w,h,TEX_FRMT,&lpsurf);
			if(hr!=D3D_OK){
				ErrMsg("CSpriteManager LoadSprite","Anim �� ���� ������� Surface ��� ����� %s",fname);
				return 0;
			}  

			last_surf->GetSurfaceLevel(0,&lptexsurf);

			hr=D3DXLoadSurfaceFromFileInMemory(lpsurf,NULL,NULL,res,wpos,NULL,D3DX_FILTER_NONE,D3DCOLOR_ARGB(255,0,0,0),NULL);
			if(hr!=D3D_OK){
				ErrMsg("CSpriteManager LoadSprite","�� ���� ��������� Surface �� ������ ��� ����� %s",fname);
				return 0;
			} 
			
			POINT p={free_x,free_y};
			RECT r={0,0,w,h};

			//� ������� ��� ��������� ������ � ������ �����
			lpDevice->CopyRects(lpsurf,&r,1,lptexsurf,&p);
			if(hr!=D3D_OK){
				ErrMsg("CSpriteManager LoadSprite","������ ��� ����������� ������������ ��� ����� %s",fname);
				return 0;
			}  

			lpsurf->Release();
			lptexsurf->Release();

			delete[] res;

			free_x+=w;
			if((free_y+h)>busy_h) busy_h=free_y+h;
			lpinf->spr_rect.x1/=last_w;
			lpinf->spr_rect.x2/=last_w;
			lpinf->spr_rect.y1/=last_h;
			lpinf->spr_rect.y2/=last_h;

			spr_data[next_id++]=lpinf;

			pframes->ind[or*frm_num+frm]=next_id-1;
		}

		fm.UnloadFile();
//		WriteLog("%d frames loaded\n",frm_num*6);
		return 1;
}

int CSpriteManager::LoadRix(char *fname, int PathType)
{
	if(!crtd) return 0;
	if(!fname[0]) return 0;
	if(!fm.LoadFile(fname,PathType))
		return 0;

	SpriteInfo* lpinf=new SpriteInfo;
	fm.SetCurPos(0x4);
	WORD w;fm.CopyMem(&w,2);
	WORD h;fm.CopyMem(&h,2);
	if(w!=640 || h!=480) return 0;
	lpinf->w=MODE_WIDTH;
	lpinf->h=MODE_HEIGHT;
	
	if(!last_surf)
	{
		lpinf->lpSurf=CreateNewSurf(MODE_WIDTH,MODE_HEIGHT);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,MODE_WIDTH,MODE_HEIGHT);
		last_surf=lpinf->lpSurf;
	}
	else if( (last_w-free_x)>=MODE_WIDTH && (last_h-free_y)>=MODE_HEIGHT )
	{
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+MODE_WIDTH,free_y+MODE_HEIGHT);
	}
	else if( last_w>=MODE_WIDTH && (last_h-busy_h)>=MODE_HEIGHT )
	{
		free_x=0;
		free_y=busy_h;
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+MODE_WIDTH,free_y+MODE_HEIGHT);
	}
	else
	{
		lpinf->lpSurf=CreateNewSurf(MODE_WIDTH,MODE_HEIGHT);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,MODE_WIDTH,MODE_HEIGHT);
		last_surf=lpinf->lpSurf;
	}

	int aligned_width = (4 - w%4)%4;

	// let's write the bmp 
	DWORD wpos=sizeof(bmpheader);
	BYTE* res=new BYTE[wpos+h*(w+aligned_width)];
	memcpy(res,bmpheader,wpos);

	int i;

	//������� �������
	fm.SetCurPos(0xA);
	BYTE* ppos=res+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	for(i=0;i<256;i++)
	{
		fm.CopyMem(ppos,3);
		for(int j=0;j<3;j++)
			ppos[j]*=4;
		BYTE t=ppos[2];
		ppos[2]=ppos[0];
		ppos[0]=t;
		ppos+=4;
	}

			
	DWORD ptr = 0xA+256*3+w*(h-1);
	for(i=0;i < h; i++) 
	{
		fm.SetCurPos(ptr);
		fm.CopyMem(res+wpos,w);
		wpos+=w;
		memset(res+wpos,0,aligned_width);
		wpos+=aligned_width;
		ptr -= w;
	}

	DWORD* spos=(DWORD*)(res+18);
	spos[0]=w;
	spos[1]=h;

	LPDIRECT3DSURFACE8 lpsurf;
	LPDIRECT3DSURFACE8 lptexsurf;
	//HRESULT hr=lpDevice->CreateImageSurface(w,h,D3DFMT_A8R8G8B8,&lpsurf);
	HRESULT hr=lpDevice->CreateImageSurface(MODE_WIDTH,MODE_HEIGHT,TEX_FRMT,&lpsurf);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadRix","�� ���� ������� Surface ��� ����� %s",fname);
		return 0;
	}  

	last_surf->GetSurfaceLevel(0,&lptexsurf);

	hr=D3DXLoadSurfaceFromFileInMemory(lpsurf,NULL,NULL,res,wpos,NULL,D3DX_FILTER_LINEAR,D3DCOLOR_ARGB(255,0,0,0),NULL);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadRix","�� ���� ��������� Surface �� ������ ��� ����� %s",fname);
		return 0;
	} 
			
	POINT p={free_x,free_y};
	RECT r={0,0,MODE_WIDTH,MODE_HEIGHT};

	lpDevice->CopyRects(lpsurf,&r,1,lptexsurf,&p);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadRix","������ ��� ����������� ������������ ��� ����� %s",fname);
		return 0;
	}  

	lpsurf->Release();
	lptexsurf->Release();

	delete[] res;

	free_x+=MODE_WIDTH;
	if((free_y+MODE_HEIGHT)>busy_h) busy_h=free_y+MODE_HEIGHT;
	lpinf->spr_rect.x1/=last_w;
	lpinf->spr_rect.x2/=last_w;
	lpinf->spr_rect.y1/=last_h;
	lpinf->spr_rect.y2/=last_h;

	spr_data[next_id++]=lpinf;

	fm.UnloadFile();
	return next_id-1;
}

// ��� 1-�� ������������ ��������
int CSpriteManager::LoadAnimationD(char *fname,int PathType,CritFrames* pframes)
{
    
	if(!crtd) return 0;
	if(!fname[0]) return 0;

	char path[1024];
	WORD frm_num;
	short offs_x[6], offs_y[6];

	for(int or=0;or<6;or++)
    {strcpy(path,fname);
    if(or==0) strcat(path,"0");
	if(or==1) strcat(path,"1");
	if(or==2) strcat(path,"2");
	if(or==3) strcat(path,"3");
	if(or==4) strcat(path,"4");
	if(or==5) strcat(path,"5");

//	WriteLog("Loading animation %s\n",path);
	if(!fm.LoadFile(path,PathType)) return 0;
//	WriteLog("Loading offsets animation %s\n",path);

	fm.SetCurPos(0x4); //!Cvet
	WORD frm_fps=fm.GetWord(); //!Cvet
	if(!frm_fps) frm_fps=10; //!Cvet

	fm.SetCurPos(0x8);
	frm_num=fm.GetWord();;

	//�������� ����� �������� �� ���� ������������
	fm.SetCurPos(0xA);
		offs_x[or]=fm.GetWord();
	fm.SetCurPos(0x16);
		offs_y[or]=fm.GetWord();

	// ����� ������� ������� � 6 ������������
	if(or==0) {
	pframes->ticks=1000/frm_fps*frm_num; //!Cvet
	pframes->cnt_frames=frm_num;
	pframes->ind=new WORD[frm_num*6];
	pframes->next_x=new short[frm_num*6];
	pframes->next_y=new short[frm_num*6];}
	
	 pframes->dir_offs[or]=or*frm_num;


	DWORD cur_ptr=0x3E;
		for(int frm=0;frm<frm_num;frm++)
		{
			SpriteInfo* lpinf=new SpriteInfo;
			lpinf->offs_x=offs_x[or];
			lpinf->offs_y=offs_y[or];
		
			fm.SetCurPos(cur_ptr);
			WORD w=fm.GetWord();
			WORD h=fm.GetWord();
			lpinf->w=w;
			lpinf->h=h;
			fm.GoForward(4);
			pframes->next_x[or*frm_num+frm]=fm.GetWord();
			pframes->next_y[or*frm_num+frm]=fm.GetWord();
	
			if(!last_surf)
			{
				//������ ����� ������� � ����� �����������
				lpinf->lpSurf=CreateNewSurf(w,h);
				if(!lpinf->lpSurf) return 0;
				lpinf->spr_rect(0,0,w,h);
				last_surf=lpinf->lpSurf;
			}
			else if( (last_w-free_x)>=w && (last_h-free_y)>=h )
			{
				//������ ����� ������� � �������������� ���
				lpinf->lpSurf=last_surf;
				lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
			}
			else if( last_w>=w && (last_h-busy_h)>=h )
			{
				//�������� ����� �������������� ��� ���� ��������
				free_x=0;
				free_y=busy_h;
				lpinf->lpSurf=last_surf;
				lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
			}
			else
			{
				//������ ������ �� ���������� �� ��� �����������
				lpinf->lpSurf=CreateNewSurf(w,h);
				if(!lpinf->lpSurf) return 0;
				lpinf->spr_rect(0,0,w,h);
				last_surf=lpinf->lpSurf;
			}

			int aligned_width = (4 - w%4)%4;

			// ������� ���� ������� bmp � ������ 
			DWORD wpos=sizeof(bmpheader);
			BYTE* res=new BYTE[wpos+h*(w+aligned_width)];
			memcpy(res,bmpheader,wpos);
			
	
			DWORD ptr = cur_ptr+12+w*(h-1);
			cur_ptr+=w*h+12;
			for(int i=0;i < h; i++) 
			{
				fm.SetCurPos(ptr);
				fm.CopyMem(res+wpos,w);
				wpos+=w;
				memset(res+wpos,0,aligned_width);
				wpos+=aligned_width;
				ptr -= w;
			}

			DWORD* spos=(DWORD*)(res+18);
			spos[0]=w;
			spos[1]=h;


			//� ������ ��������� ��� �� ��������� �����������
			LPDIRECT3DSURFACE8 lpsurf;
			LPDIRECT3DSURFACE8 lptexsurf;
//			HRESULT hr=lpDevice->CreateImageSurface(w,h,D3DFMT_A8R8G8B8,&lpsurf);
			HRESULT hr=lpDevice->CreateImageSurface(w,h,TEX_FRMT,&lpsurf);
			if(hr!=D3D_OK){
				ErrMsg("CSpriteManager LoadSprite","Anim �� ���� ������� Surface ��� ����� %s",fname);
				return 0;
			}  

			last_surf->GetSurfaceLevel(0,&lptexsurf);

			hr=D3DXLoadSurfaceFromFileInMemory(lpsurf,NULL,NULL,res,wpos,NULL,D3DX_FILTER_NONE,D3DCOLOR_ARGB(255,0,0,0),NULL);
			if(hr!=D3D_OK){
				ErrMsg("CSpriteManager LoadSprite","�� ���� ��������� Surface �� ������ ��� ����� %s",fname);
				return 0;
			} 
			
			POINT p={free_x,free_y};
			RECT r={0,0,w,h};

			//� ������� ��� ��������� ������ � ������ �����
			lpDevice->CopyRects(lpsurf,&r,1,lptexsurf,&p);
			if(hr!=D3D_OK){
				ErrMsg("CSpriteManager LoadSprite","������ ��� ����������� ������������ ��� ����� %s",fname);
				return 0;
			}  

			lpsurf->Release();
			lptexsurf->Release();

			delete[] res;

			free_x+=w;
			if((free_y+h)>busy_h) busy_h=free_y+h;
			lpinf->spr_rect.x1/=last_w;
			lpinf->spr_rect.x2/=last_w;
			lpinf->spr_rect.y1/=last_h;
			lpinf->spr_rect.y2/=last_h;

			spr_data[next_id++]=lpinf;

			pframes->ind[or*frm_num+frm]=next_id-1;
		}

		fm.UnloadFile();}
//		WriteLog("%d frames loaded\n",frm_num*6);
		return 1;
}

// ��� ����� ���������� �������� 
int CSpriteManager::LoadAnyAnimation(char *fname,int PathType,WORD end_id[4],SpriteInfo** ppInfo)
{
	if(!crtd) return 0;
	if(!fname[0]) return 0;

	char path[1024];
	WORD frm_num;
	short offs_x, offs_y;

	strcpy(path,fname);

	WriteLog("Loading animation %s\n",path);
	if(!fm.LoadFile(path,PathType)) return 0;

	fm.SetCurPos(0x8);
	frm_num=fm.GetWord();
    DWORD cur_ptr;
	// ����� �������� ���� �� ���������� ���-��
		for(int frm=0;frm<frm_num;frm++)
	{SpriteInfo* lpinf=new SpriteInfo;

	fm.SetCurPos(0xA);
	offs_x=fm.GetWord();
	fm.SetCurPos(0x16);
	offs_y=fm.GetWord();

	lpinf->offs_x=offs_x;
	lpinf->offs_y=offs_y;

	fm.SetCurPos(0x3e);
	WORD w=fm.GetWord();
	WORD h=fm.GetWord();
	lpinf->w=w;
	lpinf->h=h;
	if(!last_surf)
	{
		lpinf->lpSurf=CreateNewSurf(w,h);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,w,h);
		last_surf=lpinf->lpSurf;
	}
	else if( (last_w-free_x)>=w && (last_h-free_y)>=h )
	{
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
	}
	else if( last_w>=w && (last_h-busy_h)>=h )
	{
		free_x=0;
		free_y=busy_h;
		lpinf->lpSurf=last_surf;
		lpinf->spr_rect(free_x,free_y,free_x+w,free_y+h);
	}
	else
	{
		lpinf->lpSurf=CreateNewSurf(w,h);
		if(!lpinf->lpSurf) return 0;
		lpinf->spr_rect(0,0,w,h);
		last_surf=lpinf->lpSurf;
	}

	int aligned_width = (4 - w%4)%4;

	// let's write the bmp 
	DWORD wpos=sizeof(bmpheader);
	BYTE* res=new BYTE[wpos+h*(w+aligned_width)];
	memcpy(res,bmpheader,wpos);
			
            // ����� �� �������� � ������ �����������
				DWORD ptr = cur_ptr+13+w*(h-1);
			cur_ptr+=w*h+13;
			for(int i=0;i < h; i++) 
			{
				fm.SetCurPos(ptr);
				fm.CopyMem(res+wpos,w);
				wpos+=w;
				memset(res+wpos,0,aligned_width);
				wpos+=aligned_width;
				ptr -= w;
			}
/*
	DWORD ptr = 0x4A+w*(h-1);
	for(int i=0;i < h; i++) 
	{
		fm.SetCurPos(ptr);
		fm.CopyMem(res+wpos,w);
		wpos+=w;
		memset(res+wpos,0,aligned_width);
		wpos+=aligned_width;
		ptr -= w;
	}*/

	DWORD* spos=(DWORD*)(res+18);
	spos[0]=w;
	spos[1]=h;


	LPDIRECT3DSURFACE8 lpsurf;
	LPDIRECT3DSURFACE8 lptexsurf;
	//HRESULT hr=lpDevice->CreateImageSurface(w,h,D3DFMT_A8R8G8B8,&lpsurf);
	HRESULT hr=lpDevice->CreateImageSurface(w,h,TEX_FRMT,&lpsurf);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","�� ���� ������� Surface ��� ����� %s",fname);
		return 0;
	}  

	last_surf->GetSurfaceLevel(0,&lptexsurf);

	hr=D3DXLoadSurfaceFromFileInMemory(lpsurf,NULL,NULL,res,wpos,NULL,D3DX_FILTER_NONE,D3DCOLOR_ARGB(255,0,0,0),NULL);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","�� ���� ��������� Surface �� ������ ��� ����� %s",fname);
		return 0;
	} 
			
	POINT p={free_x,free_y};
	RECT r={0,0,w,h};

	lpDevice->CopyRects(lpsurf,&r,1,lptexsurf,&p);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager LoadSprite","������ ��� ����������� ������������ ��� ����� %s",fname);
		return 0;
	}  

	lpsurf->Release();
	lptexsurf->Release();

	delete[] res;

	free_x+=w;
	if((free_y+h)>busy_h) busy_h=free_y+h;
	lpinf->spr_rect.x1/=last_w;
	lpinf->spr_rect.x2/=last_w;
	lpinf->spr_rect.y1/=last_h;
	lpinf->spr_rect.y2/=last_h;
    
	spr_data[next_id++]=lpinf;
    end_id[frm]=next_id-1;
	WriteLog("Loading animation!%d\n",frm);}
    // ����� �������� ���� ��� �� �������������� �����������

	
	fm.UnloadFile();


	WriteLog("%d frames loaded\n",frm_num);
	return 1;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

LPDIRECT3DTEXTURE8 CSpriteManager::CreateNewSurf(WORD w, WORD h)
{
	if(!crtd) return 0;
	if(w>opt_basetex || h>opt_basetex)
	{
		for(last_w=opt_basetex;last_w<w;last_w*=2);
		for(last_h=opt_basetex;last_h<h;last_h*=2);
	}
	else
		{
			last_w=opt_basetex;
			last_h=opt_basetex;
		}

	LPDIRECT3DTEXTURE8 lpSurf=NULL;

	HRESULT hr=lpDevice->CreateTexture(last_w,last_h,1,0,TEX_FRMT,D3DPOOL_MANAGED,&lpSurf);

	if(hr!=D3D_OK){
		ErrMsg("CSpriteManager CreateNewSurf","�� ���� ������� ����� ��������");
		return NULL;
	} 
	surf_list.push_back(lpSurf);
	busy_w=busy_h=free_x=free_y=0;

	return lpSurf;
}

void CSpriteManager::NextSurface()
{
	last_surf=NULL;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

int CSpriteManager::Flush()
{
	//������� ����� ����� ��������� � ����� ������
	if(!crtd) return 0;
	void* pBuffer;
	int mulpos=4*cur_pos;
	lpVB->Lock(0,sizeof(MYVERTEX)*mulpos,(BYTE**)&pBuffer,D3DLOCK_DISCARD);
		memcpy(pBuffer,lpWaitBuf,sizeof(MYVERTEX)*mulpos);
	lpVB->Unlock();

	//������ �������
	if(!call_vec.empty())
	{
		WORD rpos=0;
		for(onesurf_vec::iterator iv=call_vec.begin();iv!=call_vec.end();iv++)
		{
			lpDevice->SetTexture(0,(*iv)->lpSurf);
			lpDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,mulpos,rpos,2*(*iv)->cnt);
			rpos+=6*(*iv)->cnt;
			delete (*iv);
		}

		call_vec.clear();
		last_call=NULL;
		cur_surf=NULL;
	}
	else lpDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,mulpos,0,2*cur_pos);	

	cur_pos=0;
	return 1;
}

int CSpriteManager::DrawSprite(WORD id, int x, int y, DWORD color) //!Cvet DWORD color
{
	SpriteInfo* lpinf=spr_data[id];
	if(!lpinf) return 0;

	if(cur_surf!=lpinf->lpSurf)
	{
		last_call=new OneSurface(lpinf->lpSurf);
		call_vec.push_back(last_call);
		cur_surf=lpinf->lpSurf;
	}
	else if(last_call) last_call->cnt++;

	int mulpos=cur_pos*4;

	if(!color) color=col; //!Cvet

	lpWaitBuf[mulpos].x=x-0.5f;
	lpWaitBuf[mulpos].y=y+lpinf->h-0.5f;
	lpWaitBuf[mulpos].tu=lpinf->spr_rect.x1;
	lpWaitBuf[mulpos].tv=lpinf->spr_rect.y2;
	lpWaitBuf[mulpos++].Diffuse=color;

	lpWaitBuf[mulpos].x=x-0.5f;
	lpWaitBuf[mulpos].y=y-0.5f;
	lpWaitBuf[mulpos].tu=lpinf->spr_rect.x1;
	lpWaitBuf[mulpos].tv=lpinf->spr_rect.y1;
	lpWaitBuf[mulpos++].Diffuse=color;

	lpWaitBuf[mulpos].x=x+lpinf->w-0.5f;
	lpWaitBuf[mulpos].y=y-0.5f;
	lpWaitBuf[mulpos].tu=lpinf->spr_rect.x2;
	lpWaitBuf[mulpos].tv=lpinf->spr_rect.y1;
	lpWaitBuf[mulpos++].Diffuse=color;
		
	lpWaitBuf[mulpos].x=x+lpinf->w-0.5f;
	lpWaitBuf[mulpos].y=y+lpinf->h-0.5f;
	lpWaitBuf[mulpos].tu=lpinf->spr_rect.x2;
	lpWaitBuf[mulpos].tv=lpinf->spr_rect.y2;
	lpWaitBuf[mulpos++].Diffuse=color;

	cur_pos++;

	if(cur_pos==spr_cnt) Flush();

	return 1;
}

int CSpriteManager::DrawSpriteSize(WORD id, int x, int y,double size, DWORD color) //!Cvet DWORD color
{
	SpriteInfo* lpinf=spr_data[id];
	if(!lpinf) return 0;
    //lpinf->spr_rect.x1/=size;
    //lpinf->spr_rect.x1/=size;

	if(cur_surf!=lpinf->lpSurf)
	{
		last_call=new OneSurface(lpinf->lpSurf);
		call_vec.push_back(last_call);
		cur_surf=lpinf->lpSurf;
	}
	else if(last_call) last_call->cnt++;

	int mulpos=cur_pos*4;

	if(!color) color=col; //!Cvet

	lpWaitBuf[mulpos].x=x-0.5f;
	lpWaitBuf[mulpos].y=y+lpinf->h-0.5f;
	lpWaitBuf[mulpos].tu=lpinf->spr_rect.x1;
	lpWaitBuf[mulpos].tv=lpinf->spr_rect.y2;
	lpWaitBuf[mulpos++].Diffuse=color;

	lpWaitBuf[mulpos].x=x-0.5f;
	lpWaitBuf[mulpos].y=y-0.5f;
	lpWaitBuf[mulpos].tu=lpinf->spr_rect.x1;
	lpWaitBuf[mulpos].tv=lpinf->spr_rect.y1;
	lpWaitBuf[mulpos++].Diffuse=color;

	lpWaitBuf[mulpos].x=x+lpinf->w-0.5f;
	lpWaitBuf[mulpos].y=y-0.5f;
	lpWaitBuf[mulpos].tu=lpinf->spr_rect.x2;
	lpWaitBuf[mulpos].tv=lpinf->spr_rect.y1;
	lpWaitBuf[mulpos++].Diffuse=color;
		
	lpWaitBuf[mulpos].x=x+lpinf->w-0.5f;
	lpWaitBuf[mulpos].y=y+lpinf->h-0.5f;
	lpWaitBuf[mulpos].tu=lpinf->spr_rect.x2;
	lpWaitBuf[mulpos].tv=lpinf->spr_rect.y2;
	lpWaitBuf[mulpos++].Diffuse=color;

	cur_pos++;

	if(cur_pos==spr_cnt) Flush();

	return 1;
}

int CSpriteManager::PrepareBuffer(dtree_map* lpdtree,LPDIRECT3DVERTEXBUFFER8* lplpBuf,onesurf_vec* lpsvec, BYTE alpha)
{
	SAFEREL((*lplpBuf));
	for(onesurf_vec::iterator iv=lpsvec->begin();iv!=lpsvec->end();iv++)
		delete (*iv);
	lpsvec->clear();

	WORD cnt=lpdtree->size();

	if(!cnt) return 1;

	//������� ����� ������
	HRESULT hr=lpDevice->CreateVertexBuffer(cnt*4*sizeof(MYVERTEX),D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
		D3DFVF_MYVERTEX,D3DPOOL_DEFAULT,lplpBuf);
	if(hr!=D3D_OK){
		ErrMsg("CSpriteMngr PrepareBuffer","������ ��� �������� ������: %s",DXGetErrorString8(hr));
		return 0;
	}

	DWORD need_size=cnt*6*sizeof(WORD);
	D3DINDEXBUFFER_DESC ibdesc;

	lpIB->GetDesc(&ibdesc);

	if(ibdesc.Size<need_size)
	{
		SAFEREL(lpIB);
		//� ��������
		hr=lpDevice->CreateIndexBuffer(need_size,D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,D3DPOOL_DEFAULT,&lpIB);
		if(hr!=D3D_OK){
		ErrMsg("CSpriteMngr PrepareBuffer","������ ��� �������� ������: %s",DXGetErrorString8(hr));
			return 0;
		}

		WORD* IndexList=new WORD[6*cnt];
		for(int i=0;i<cnt;i++)
		{
			IndexList[6*i+0]=4*i+0;
			IndexList[6*i+1]=4*i+1;
			IndexList[6*i+2]=4*i+3;
			IndexList[6*i+3]=4*i+1;
			IndexList[6*i+4]=4*i+2;
			IndexList[6*i+5]=4*i+3;
		}
	
		void* pBuffer;
		lpIB->Lock(0,0,(BYTE**)&pBuffer,0);
			memcpy(pBuffer,IndexList,need_size);
		lpIB->Unlock();

		delete[] IndexList;

		lpDevice->SetIndices(lpIB,0);
	}

	WORD mulpos=0;
	OneSurface* lc=NULL;
	MYVERTEX* localBuf=new MYVERTEX[cnt*4];

	DWORD new_alpha=col; //!Cvet
	if(alpha) new_alpha+=(alpha << 24) & 0xFF000000; //!Cvet

//!Cvet �������������. ������� ������������ �� �����������. ������������������ ������� �� 300-400% +++++++++++++++++++++
//� ���������� ����� �������������� ���
	spr_vec_vec spr_vectors;
	spr_vec_vec::iterator it_vec;
	for(dtree_map::iterator jt=lpdtree->begin();jt!=lpdtree->end();jt++)
	{
		SpriteInfo* sinf=spr_data[(*jt).second->spr_id];
		if(!sinf) return 0;

		for(it_vec=spr_vectors.begin();it_vec!=spr_vectors.end();it_vec++)
		{
			spr_vec* cur_spr_vec=&(*it_vec);
			PrepSprite* cur_ps=*cur_spr_vec->begin();
			SpriteInfo* cur_sinf=spr_data[cur_ps->spr_id];
			if(cur_sinf->lpSurf==sinf->lpSurf)
			{
				cur_spr_vec->push_back((*jt).second);
				break;
			}
		}

		if(it_vec==spr_vectors.end())
		{
			spr_vec n_vec;
			n_vec.push_back((*jt).second);
			spr_vectors.push_back(n_vec);
		}
	}

	for(it_vec=spr_vectors.begin();it_vec!=spr_vectors.end();it_vec++)
		for(spr_vec::iterator it_spr=(*it_vec).begin();it_spr!=(*it_vec).end();it_spr++) //Cvet -------------------------
		{
			SpriteInfo* lpinf=spr_data[(*it_spr)->spr_id];
			int x=(*it_spr)->scr_x+cmn_scr_ox;
			int y=(*it_spr)->scr_y+cmn_scr_oy;

			if(!lc || lc->lpSurf!=lpinf->lpSurf)
			{
				lc=new OneSurface(lpinf->lpSurf);
				lpsvec->push_back(lc);
			}
			else lc->cnt++;

			localBuf[mulpos].x=x-0.5f;
			localBuf[mulpos].y=y+lpinf->h-0.5f;
			localBuf[mulpos].tu=lpinf->spr_rect.x1;
			localBuf[mulpos].tv=lpinf->spr_rect.y2;
			localBuf[mulpos++].Diffuse=new_alpha;

			localBuf[mulpos].x=x-0.5f;
			localBuf[mulpos].y=y-0.5f;
			localBuf[mulpos].tu=lpinf->spr_rect.x1;
			localBuf[mulpos].tv=lpinf->spr_rect.y1;
			localBuf[mulpos++].Diffuse=new_alpha;

			localBuf[mulpos].x=x+lpinf->w-0.5f;
			localBuf[mulpos].y=y-0.5f;
			localBuf[mulpos].tu=lpinf->spr_rect.x2;
			localBuf[mulpos].tv=lpinf->spr_rect.y1;
			localBuf[mulpos++].Diffuse=new_alpha;
			
			localBuf[mulpos].x=x+lpinf->w-0.5f;
			localBuf[mulpos].y=y+lpinf->h-0.5f;
			localBuf[mulpos].tu=lpinf->spr_rect.x2;
			localBuf[mulpos].tv=lpinf->spr_rect.y2;
			localBuf[mulpos++].Diffuse=new_alpha;
		}

	void* pBuffer;
	(*lplpBuf)->Lock(0,sizeof(MYVERTEX)*mulpos,(BYTE**)&pBuffer,D3DLOCK_DISCARD);
		memcpy(pBuffer,localBuf,sizeof(MYVERTEX)*mulpos);
	(*lplpBuf)->Unlock();

	SAFEDELA(localBuf);

	return 1;
}

void CSpriteManager::DrawPrepared(LPDIRECT3DVERTEXBUFFER8 lpBuf,onesurf_vec* lpsvec, WORD cnt)
{
	if(!cnt) return;
	Flush();

	lpDevice->SetStreamSource(0,lpBuf,sizeof(MYVERTEX));

	WORD rpos=0;
	for(onesurf_vec::iterator iv=lpsvec->begin();iv!=lpsvec->end();iv++)
	{
		lpDevice->SetTexture(0,(*iv)->lpSurf);
		lpDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,cnt*4,rpos,2*(*iv)->cnt);
		rpos+=6*(*iv)->cnt;
	}

	lpDevice->SetStreamSource(0,lpVB,sizeof(MYVERTEX));
}

void CSpriteManager::GetDrawCntrRect(PrepSprite* prep, INTRECT* prect)
{
	WORD id;
	if(prep->lp_sprid) id=*prep->lp_sprid;
	else id=prep->spr_id;
	SpriteInfo* lpinf=spr_data[id];
	if(!lpinf) return;
	int x=prep->scr_x-(lpinf->w >> 1)+lpinf->offs_x;
	int y=prep->scr_y-lpinf->h+lpinf->offs_y;

	if(prep->lp_ox) x+=*prep->lp_ox;
	if(prep->lp_oy) y+=*prep->lp_oy;

	prect->l=x;
	prect->t=y;
	prect->r=x+lpinf->w;
	prect->b=y+lpinf->h;
}

void CSpriteManager::DrawTreeCntr(dtree_map* lpdtree)
{
	for(dtree_map::iterator jt=lpdtree->begin();jt!=lpdtree->end();jt++)
	{
		WORD id;
		if((*jt).second->lp_sprid) id=*(*jt).second->lp_sprid;
		else id=(*jt).second->spr_id;
		SpriteInfo* lpinf=spr_data[id];
		if(!lpinf) continue;
		int x=(*jt).second->scr_x-(lpinf->w >> 1)+lpinf->offs_x+cmn_scr_ox;
		int y=(*jt).second->scr_y-lpinf->h+lpinf->offs_y+cmn_scr_oy;

		if((*jt).second->lp_ox) x+=*(*jt).second->lp_ox;
		if((*jt).second->lp_oy) y+=*(*jt).second->lp_oy;

		if(cur_surf!=lpinf->lpSurf)
		{
			last_call=new OneSurface(lpinf->lpSurf);
			call_vec.push_back(last_call);
			cur_surf=lpinf->lpSurf;
		}
		else if(last_call) last_call->cnt++;

		int mulpos=cur_pos*4;

		lpWaitBuf[mulpos].x=x-0.5f;
		lpWaitBuf[mulpos].y=y+lpinf->h-0.5f;
		lpWaitBuf[mulpos].tu=lpinf->spr_rect.x1;
		lpWaitBuf[mulpos].tv=lpinf->spr_rect.y2;
		lpWaitBuf[mulpos++].Diffuse=col;

		lpWaitBuf[mulpos].x=x-0.5f;
		lpWaitBuf[mulpos].y=y-0.5f;
		lpWaitBuf[mulpos].tu=lpinf->spr_rect.x1;
		lpWaitBuf[mulpos].tv=lpinf->spr_rect.y1;
		lpWaitBuf[mulpos++].Diffuse=col;

		lpWaitBuf[mulpos].x=x+lpinf->w-0.5f;
		lpWaitBuf[mulpos].y=y-0.5f;
		lpWaitBuf[mulpos].tu=lpinf->spr_rect.x2;
		lpWaitBuf[mulpos].tv=lpinf->spr_rect.y1;
		lpWaitBuf[mulpos++].Diffuse=col;
		
		lpWaitBuf[mulpos].x=x+lpinf->w-0.5f;
		lpWaitBuf[mulpos].y=y+lpinf->h-0.5f;
		lpWaitBuf[mulpos].tu=lpinf->spr_rect.x2;
		lpWaitBuf[mulpos].tv=lpinf->spr_rect.y2;
		lpWaitBuf[mulpos++].Diffuse=col;

		cur_pos++;
	
		if(cur_pos==spr_cnt) Flush();
	}
}

void CSpriteManager::PreRestore()
{
	SAFEREL(lpVB);
	SAFEREL(lpIB);
}

void CSpriteManager::PostRestore()
{
	//������� ����� ������
	WriteLog("���������� VB �� %d ��������\n",spr_cnt);
	HRESULT hr=lpDevice->CreateVertexBuffer(spr_cnt*4*sizeof(MYVERTEX),D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
		D3DFVF_MYVERTEX,D3DPOOL_DEFAULT,&lpVB);
	if(hr!=D3D_OK){
		ErrMsg("SM::CreateVertexBuffer",(char*)DXGetErrorString8(hr));
		return;
	}

	//� ��������
	hr=lpDevice->CreateIndexBuffer(spr_cnt*6*sizeof(WORD),D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,D3DPOOL_DEFAULT,&lpIB);
	if(hr!=D3D_OK){
		ErrMsg("SM::CreateIndexBuffer",(char*)DXGetErrorString8(hr));
		return;
	}
	

	WORD* IndexList=new WORD[6*spr_cnt];
	for(int i=0;i<spr_cnt;i++)
	{
		IndexList[6*i+0]=4*i+0;
		IndexList[6*i+1]=4*i+1;
		IndexList[6*i+2]=4*i+3;
		IndexList[6*i+3]=4*i+1;
		IndexList[6*i+4]=4*i+2;
		IndexList[6*i+5]=4*i+3;
	}
	
	void* pBuffer;
	lpIB->Lock(0,0,(BYTE**)&pBuffer,0);
		memcpy(pBuffer,IndexList,spr_cnt*6*sizeof(WORD));
	lpIB->Unlock();

	delete[] IndexList;

	lpDevice->SetIndices(lpIB,0);
	lpDevice->SetStreamSource(0,lpVB,sizeof(MYVERTEX));
	lpDevice->SetVertexShader(D3DFVF_MYVERTEX);
}

//!Cvet +++++++++++++++++++++++++++++++++++++++++++
int CSpriteManager::LoadCritTypes()
{

	//��������� ������ ���������
	char str[1024];
	char key[64];
	CrTYPE cur=0;
	CrTYPE cnt=GetPrivateProfileInt("critters","id_cnt",0,opt_crfol);
	if(!cnt) return 0;

	for(cur=0;cur<cnt;cur++)
	{
		wsprintf(key,"%d",cur);
		GetPrivateProfileString("critters",key,"",str,1023,opt_crfol);
		if(!str[0]) continue;
		char* str2=new char[strlen(str)+1];
		strcpy(str2,str);
		crit_types[cur]=str2;
	}
	return 1;
}

int CSpriteManager::LoadAnimCr(CrTYPE anim_type, BYTE anim_ind1, BYTE anim_ind2)
{
	if(CrAnim[anim_type][anim_ind1][anim_ind2]) return 1;

	WriteLog("�������� �������� type=%d,ind1=%d,ind2=%d...",anim_type,anim_ind1,anim_ind2);

	TICK loadA=GetTickCount();
	char frm_ind1[]="_ABCDEFGHIJKLMN___R";
	char frm_ind2[]="_ABCDEFGHIJKLMNOPQRST";
	char path[1024];//12345678901234567890

	sprintf(path,"%s%c%c.frm",crit_types[anim_type],frm_ind1[anim_ind1],frm_ind2[anim_ind2]);
	WriteLog("1 ������� |%s|...",path);
	CrAnim[anim_type][anim_ind1][anim_ind2]=new CritFrames;
	if(!LoadAnimation(path,PT_ART_CRITTERS,CrAnim[anim_type][anim_ind1][anim_ind2]))
	{
		sprintf(path,"%s%c%c.fr",crit_types[anim_type],frm_ind1[anim_ind1],frm_ind2[anim_ind2]);
		WriteLog("2 ������� |%s|...",path);
		if(!LoadAnimationD(path,PT_ART_CRITTERS,CrAnim[anim_type][anim_ind1][anim_ind2]))
		{
			SAFEDEL(CrAnim[anim_type][anim_ind1][anim_ind2]);
			WriteLog("�������� �� �������\n");
			return 0;
		}
	}
	WriteLog("OK - ����� �������� �������� %s =%d\n",path,GetTickCount()-loadA);
	return 1;
}

int CSpriteManager::EraseAnimCr(CrTYPE anim_type, BYTE anim_ind1, BYTE anim_ind2)
{
	if(!CrAnim[anim_type][anim_ind1][anim_ind2]) return 1;
	TICK loadA=GetTickCount();

	sprinfo_map::iterator it=NULL;
	int num_frm=CrAnim[anim_type][anim_ind1][anim_ind2]->cnt_frames;
	for(int or=0;or<6;or++)
		for(int frm=0;frm<num_frm;frm++)
		{
			DWORD num_sprite=or*num_frm+frm;
			it=spr_data.find(CrAnim[anim_type][anim_ind1][anim_ind2]->ind[num_sprite]);
			if(it==spr_data.end()) return 0;
			delete (*it).second;
			spr_data.erase(it);
		}
	SAFEDEL(CrAnim[anim_type][anim_ind1][anim_ind2]);
	WriteLog("����� �������� �������� =%d\n",GetTickCount()-loadA);
	return 1;
}
//!Cvet -----------------------------------------