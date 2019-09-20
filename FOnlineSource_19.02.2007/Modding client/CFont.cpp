#include "stdafx.h"

#include "CFont.h"
#include "common.h"


CFOFont::CFOFont(): crtd(0),lpVB(NULL),lpIB(NULL),lpDevice(NULL),lpWaitBuf(NULL),font_surf(NULL)
{
	max_cnt=16;
	maxx=new int[max_cnt];
}

CFOFont::~CFOFont()
{
	SAFEDELA(maxx);
}


int CFOFont::Init(LPDIRECT3DDEVICE8 lpD3Device,LPDIRECT3DVERTEXBUFFER8 aVB,LPDIRECT3DINDEXBUFFER8 aIB)
{
	if(crtd) return 0;
	WriteLog("CFont Initialization...\n");

	lpDevice=lpD3Device;
	lpIB=aIB;
	lpVB=aVB;

	spr_cnt=opt_flushval;
	lpWaitBuf=new MYVERTEX[spr_cnt*4];


	char path[1024];
	strcpy(path,opt_fopath);
	if(path[strlen(path)-1]!='\\') strcat(path,"\\");
	strcat(path,"art\\misc\\font.bmp");
	HRESULT hr=D3DXCreateTextureFromFileEx(lpDevice,path,D3DX_DEFAULT,D3DX_DEFAULT,1,0,
		D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT,D3DCOLOR_ARGB(255,0,0,0),NULL,NULL,&font_surf);
	if(hr!=D3D_OK){
		ErrMsg("CFont CreateFontSurf","Не могу создать текстуру %s",path);
		return 0;
	} 

	HANDLE hFile;
	DWORD ByteWritten;

	strcpy(path,opt_fopath);
	if(path[strlen(path)-1]!='\\') strcat(path,"\\");
	strcat(path,"art\\misc\\font.fnt");
	hFile=CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

	if(hFile==INVALID_HANDLE_VALUE)
	{
		ErrMsg("CFont LoadLetters","Не могу найти %s",path);
		return 0;
	} 

	ReadFile(hFile,let,sizeof(Letter)*256,&ByteWritten,NULL);

	CloseHandle(hFile);

	eth=let[(BYTE)'а'].h;
	etw=let[(BYTE)'а'].w;
	D3DSURFACE_DESC sd;
	font_surf->GetLevelDesc(0,&sd);
	UINT wd=sd.Width;

	for(int i=0;i<256;i++)
	{
			int w=let[i].w;
			int h=let[i].h;
			xyarr[i][0]=(FLOAT)let[i].dx/wd;
			xyarr[i][1]=(FLOAT)let[i].dy/wd;
			xyarr[i][2]=(FLOAT)(let[i].dx+w)/wd;
			xyarr[i][3]=(FLOAT)(let[i].dy+h)/wd;
	}

	WriteLog("CFont Initialization complete\n");
	crtd=1;
	return 1;

}

void CFOFont::Clear()
{
	WriteLog("CFont Clear...\n");

	SAFEREL(font_surf);
	SAFEDELA(lpWaitBuf);

	crtd=0;
	WriteLog("CFont Clear complete\n");
}

void CFOFont::PreRestore()
{
}

void CFOFont::PostRestore(LPDIRECT3DVERTEXBUFFER8 aVB,LPDIRECT3DINDEXBUFFER8 aIB)
{
	lpIB=aIB;
	lpVB=aVB;
}

void CFOFont::MyDrawText(RECT r,char* astr,DWORD flags, DWORD col)
{
	if(!astr) return;

	int curx=r.left+1;
	int cury=r.top+4;
	int strcnt=1;
	char* str=new char[strlen(astr)*2];
	char* alloc_str=str;
	strcpy(str,astr);

	//сначала делаем перенос строк или обрезку
	//если перенести нельзя или не помещается

//!Cvet переделал+++++++++++++
	int strcnt_r=0; //кол-во строк в прямоугольнике

	for(int i=0;str[i];i++)
	{
		if(FLAG(flags,FT_COLORIZE) && str[i]=='|')
		{
			for(;str[++i];) if(str[i]==' ') break;
			continue;
		}

		if(curx+etw>=r.right)
		{
			if(!FLAG(flags,FT_NOBREAK))
			{
				for(int j=i;j>=0;j--)
				{
					if(str[j]==' ')
					{
						str[j]='\n';
						i=j;
						break;
					}
					else if(str[j]=='\n')
					{
						j=-1;
						break;
					}
				}
				
				if(j<0) str[i]='\n';
			}
			else
				str[i]=0;
		}

		switch(str[i]) 
		{
		case ' ':
			curx+=etw-1;
			continue;
		case '\n':
			cury+=eth+4;

			if(cury+eth+4>=r.bottom) 
			{
				if(!strcnt_r) strcnt_r=strcnt;

				if(!FLAG(flags,FT_UPPER))
				{
					str[i]=0;
					break;
				}
			}

			curx=r.left+1;
			strcnt++;
			continue;
		case '\r':
			continue;
		default:
			curx+=let[(BYTE)str[i]].w+1;
		}

		if(!str[i]) break;
	}

	if(!strcnt_r) strcnt_r=strcnt;

	//выбираем нужные строки исходя из дополнительных флагов
	if((flags & FT_UPPER) && strcnt>strcnt_r)
	{
		for(int line_cur=0,i=0;str[i],line_cur<(strcnt-strcnt_r);i++)
			if(str[i]=='\n') line_cur++;
		
		str=&str[i];

		strcnt=strcnt_r;
	}
//!Cvet -------------

	//хорошо, теперь прикинем в какие размеры мы поместились после всего этого
	curx=r.left+1;
	cury=r.top+4;

	if(strcnt>max_cnt)
	{
		while(strcnt>max_cnt) max_cnt*=2;
		SAFEDELA(maxx);
		maxx=new int[max_cnt];
	}

	//memset(maxx,curx,sizeof(int)*strcnt);
	for(i=0;i<strcnt;i++) maxx[i]=curx;
	int curstr=0;
	for(i=0;str[i];i++)
	{
		switch(str[i]) 
		{
		case ' ':
			curx+=etw-1;
			continue;
		case '\n':
			cury+=eth+4;
			curx=r.left+1;
			curstr++;
			continue;
		case '\r':
			continue;
		default:
			curx+=let[(BYTE)str[i]].w+1;
			if(curx>maxx[curstr]) maxx[curstr]=curx;
		}
	}

	//а вот теперь рисуем взаправду
	curstr=0;
	curx=r.left+((flags & FT_CENTERX)?(r.right-maxx[curstr])/2:1);

	if(flags & FT_CENTERY)
		cury=r.top+4+(r.bottom-r.top-strcnt*(eth+4))/2;
	else
		if(flags & FT_BOTTOM)
			cury=r.bottom-strcnt*(eth+4);
		else
			cury=r.top+4;

	cur_pos=0;
	lpDevice->SetTexture(0,font_surf);

	DWORD colorize=col;
	for(i=0;str[i];i++)
	{
		if(FLAG(flags,FT_COLORIZE))
		{
			if(str[i]=='|')
			{
				colorize=0;
				sscanf(&str[++i],"%d",&colorize);
				if(!colorize)
				{
					colorize=col;
					continue;
				}

				for(;str[i]!=' ';i++);
				continue;
			}
		}

		switch(str[i]) 
		{
		case ' ':
			curx+=etw-1;
			continue;
		case '\n':
			cury+=eth+4;
			curx=r.left+((flags & FT_CENTERX)?(r.right-maxx[++curstr])/2:1);
			continue;
		case '\r':
			continue;
		default:

			int mulpos=cur_pos*4;
			int x=curx;
			int y=cury-let[(BYTE)str[i]].y_offs;
			int w=let[(BYTE)str[i]].w;
			int h=let[(BYTE)str[i]].h;

			FLOAT x1=xyarr[(BYTE)str[i]][0];
			FLOAT y1=xyarr[(BYTE)str[i]][1];
			FLOAT x2=xyarr[(BYTE)str[i]][2];
			FLOAT y2=xyarr[(BYTE)str[i]][3];
		
			lpWaitBuf[mulpos].x=x-0.5f;
			lpWaitBuf[mulpos].y=y+h-0.5f;
			lpWaitBuf[mulpos].tu=x1;
			lpWaitBuf[mulpos].tv=y2;
			lpWaitBuf[mulpos++].Diffuse=colorize;
		
			lpWaitBuf[mulpos].x=x-0.5f;
			lpWaitBuf[mulpos].y=y-0.5f;
			lpWaitBuf[mulpos].tu=x1;
			lpWaitBuf[mulpos].tv=y1;
			lpWaitBuf[mulpos++].Diffuse=colorize;
		
			lpWaitBuf[mulpos].x=x+w-0.5f;
			lpWaitBuf[mulpos].y=y-0.5f;
			lpWaitBuf[mulpos].tu=x2;
			lpWaitBuf[mulpos].tv=y1;
			lpWaitBuf[mulpos++].Diffuse=colorize;
				
			lpWaitBuf[mulpos].x=x+w-0.5f;
			lpWaitBuf[mulpos].y=y+h-0.5f;
			lpWaitBuf[mulpos].tu=x2;
			lpWaitBuf[mulpos].tv=y2;
			lpWaitBuf[mulpos++].Diffuse=colorize;
		
			cur_pos++;
		
			if(cur_pos==spr_cnt) Flush();
			curx+=let[(BYTE)str[i]].w+1;
		}
	}

	Flush();
	SAFEDELA(alloc_str);
}

int CFOFont::Flush()
{
	//который потом разом сливается в буфер вершин
	if(!crtd) return 0;
	void* pBuffer;
	int mulpos=4*cur_pos;
	lpVB->Lock(0,sizeof(MYVERTEX)*mulpos,(BYTE**)&pBuffer,D3DLOCK_DISCARD);
		memcpy(pBuffer,lpWaitBuf,sizeof(MYVERTEX)*mulpos);
	lpVB->Unlock();

	//рисуем спрайты
	lpDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,mulpos,0,2*cur_pos);	

	cur_pos=0;
	return 1;
}
