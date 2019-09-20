#include "stdafx.h"

#include "common.h"
#include "FEngine.h"

LRESULT APIENTRY WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
HINSTANCE hInstance=NULL;//����������
HWND hWnd=NULL;//����

CFEngine* engine;


int APIENTRY WinMain(HINSTANCE hCurrentInst,
	HINSTANCE hPreviousInst,LPSTR lpCmdLine,int nCmdShow)
{
	WNDCLASS wndClass;//������������ ��� ����������� ������ ����
	MSG msg;//���������
	wndClass.style=CS_HREDRAW|CS_VREDRAW;//���������� �������� ����
	wndClass.lpfnWndProc=WndProc;//���������� ����� ������� ����
	wndClass.cbClsExtra=0;//����� ����, ������� ���������� ��������� � Windows. ������ ����� 0
	wndClass.cbWndExtra=0;//����� ����, ������� ���������� ��������� � Windows. ������ ����� 0
	wndClass.hInstance =hCurrentInst;//�������� Windows � ���, ��� ������� ����������� ������
	wndClass.hIcon =LoadIcon(hCurrentInst,MAKEINTRESOURCE(IDI_ICON));//��������� ������, � ������ ������ �� ���
	wndClass.hCursor =LoadCursor(NULL,IDC_ARROW);//����������� ������
	wndClass.hbrBackground=(HBRUSH)GetStockObject(LTGRAY_BRUSH);//��� ����������
	wndClass.lpszMenuName=NULL;//���������� ����. � ������ �������� ���� �����������
	wndClass.lpszClassName="FOnline";//��������� �� ������, ���������� ��� ������
	RegisterClass(&wndClass);//����������� ����
	hInstance=hCurrentInst;

	if(!StartLogFile())
	{
		DestroyWindow(hWnd);
		return 0;
	}

	GetOptions();

	hWnd= CreateWindow( //�������� ����
		"FOnline",
		"Fallout Online",
		WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX) & (~WS_SIZEBOX),
		0,0,MODE_WIDTH+5,MODE_HEIGHT+25,
		NULL,
		NULL,
		hCurrentInst,
		NULL);

	ShowWindow(hWnd,SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	WriteLog("Starting FOnline...\n");

	srand(GetTickCount());

	engine=new CFEngine;

	if(!engine->Init(hWnd))
	{
		WriteLog("Fengine not init\n");
		DestroyWindow(hWnd);
		return 0;
	}
	
    //����������� ����� ��������� ���������
	while(!cmn_Quit)
	{
		if(!cmn_lost)
		{
			if(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				engine->Render();

				if(opt_sleep) Sleep(opt_sleep);
			}
		}
		else 
		{
			GetMessage(&msg,NULL,NULL,NULL);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	WriteLog("\nFOnline Closed\n");
	CloseLogFile();

	delete engine;

	_CrtDumpMemoryLeaks();
	return 0;
}

LRESULT APIENTRY WndProc(HWND hWnd,
UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY://���������� ��� ���������� ����
		if(engine) engine->Clear();
		cmn_Quit=1;
		return 0;
	case WM_KEYDOWN:
		if(wParam==VK_ESCAPE) 
		{
			//!Cvet +++++
			if(engine->state==STATE_DISCONNECT)
			{
				WriteLog("Quit user on ESCAPE\n");
				DestroyWindow(hWnd);
			}
			engine->NetDiscon();
			//!Cvet -----
		}
		return 0;
	case WM_ACTIVATE:
		if(WA_INACTIVE != wParam)
		{
			if(engine) engine->Restore();
			if(engine) engine->RestoreDI();
		}
		break;
/*	case WM_SETCURSOR:
		// Turn off window cursor 
	    SetCursor( NULL );
	    return TRUE; // prevent Windows from setting cursor to window class cursor
	break;*/
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

