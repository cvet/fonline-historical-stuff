#include "stdafx.h"

/********************************************************************
	created:	18:08:2004   23:02

	author:		Oleg Mareskin
	
	purpose:	
*********************************************************************/


#include "main.h"
#include "FOserv.h"

#include <iostream>

int bQuit=0;
int FOQuit=0;
BOOL logging=TRUE;

HANDLE hGameThread=NULL;
DWORD dwGameThreadID=0;
int NumClients=0;
int NumCritters=0; //!Cvet ����� ��������� � ����
HANDLE hUpdateEvent;


DWORD WINAPI GameLoopThread(void *);


#ifndef FOSERVICE_VERSION


	BOOL CALLBACK DlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
	
	void UpdateInfo();
	
	HINSTANCE hInstance;//����������
	HWND hWnd;
	HWND hDlg=NULL;
	
	CServer* serv;
	
	int APIENTRY WinMain(HINSTANCE hCurrentInst,
		HINSTANCE hPreviousInst,LPSTR lpCmdLine,int nCmdShow)
	{
		MSG msg;//���������
	
		LoadLibrary("RICHED32.dll");
	
		hDlg=CreateDialog(hInstance,MAKEINTRESOURCE(IDD_DLG),hWnd,DlgProc);
		hUpdateEvent=CreateEvent(NULL,1,0,NULL);
		hGameThread=CreateThread(NULL,0,GameLoopThread,NULL,0,&dwGameThreadID);

		serv=new CServer;

	//����������� ����� ��������� ���������
		while(!bQuit)
		{
			if(MsgWaitForMultipleObjects(1,&hUpdateEvent,0,INFINITE,QS_ALLINPUT)==(WAIT_OBJECT_0+1))
				while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else UpdateInfo();
		}

		SAFEDEL(serv);
		_CrtDumpMemoryLeaks();

		return 0;
	}
	
	void UpdateInfo()
	{
		char Str[300];
		ResetEvent(hUpdateEvent);
		wsprintf(Str,"���������� ��������: %d",NumClients);
		SetDlgItemText(hDlg,IDC_COUNT,Str);
	}

	BOOL CALLBACK DlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			PostMessage(hWnd,WM_SIZE,0,0);
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDCANCEL:
				FOQuit=1;
				bQuit=1;
				if(WaitForSingleObject(hGameThread,10000)==WAIT_TIMEOUT)
					MessageBox(NULL,"Wait timeout 10 sec passed","Error",MB_OK|MB_ICONERROR);
				CloseHandle(hGameThread);
				CloseHandle(hUpdateEvent);
				CloseWindow(hDlg);
				return 0;
			case IDC_STOP:
				if(!FOQuit)
				{
					FOQuit=1;
					SetDlgItemText(hDlg,IDC_STOP,"���������");
					EnableWindow(GetDlgItem(hDlg,IDC_RELOAD),0);
				}
				else if(!hGameThread)
					{
						FOQuit=0;
						SetDlgItemText(hDlg,IDC_STOP,"����������");
						EnableWindow(GetDlgItem(hDlg,IDC_RELOAD),1);
						hGameThread=CreateThread(NULL,0,GameLoopThread,NULL,0,&dwGameThreadID);
					}
				break;
			}
			return 0;
		}
		return 0;
	}

	void LogExecStr(char* frmt, ...)
	{
		if(bQuit) return;
		
		char logstr[2048];
	
	    va_list list;
	
	    va_start(list, frmt);
	    wvsprintf(logstr, frmt, list);
	    va_end(list);

		int len=GetWindowTextLength(GetDlgItem(hDlg,IDC_EXECLOG));
		
		SendDlgItemMessage(hDlg,IDC_EXECLOG,EM_SETSEL,len-1,len);
		
		char str[2060];
		SendDlgItemMessage(hDlg,IDC_EXECLOG,EM_GETSELTEXT,0,(LPARAM) str);
		strcat(str,logstr);
		SendDlgItemMessage(hDlg,IDC_EXECLOG,EM_REPLACESEL,0,(LPARAM) str); 
	}

	DWORD WINAPI GameLoopThread(void *)
	{
		if(!serv->Init()) goto GAMELOOPEND;
		
		serv->RunGameLoop();
	
		serv->Finish();

	GAMELOOPEND:	
		hGameThread=NULL;
		ExitThread(0);
		return 0;
	}

#else

//////////////////////////////////////////////////////////////////////////
//					SERVICE VERSION
//////////////////////////////////////////////////////////////////////////

	CServer serv;
	HANDLE hLogFile=NULL;


	SERVICE_STATUS          FOServiceStatus;  
	SERVICE_STATUS_HANDLE   FOServiceStatusHandle; 
	 
	VOID  WINAPI FOServiceStart (DWORD argc, LPTSTR *argv); 
	VOID  WINAPI FOServiceCtrlHandler (DWORD opcode); 
	 
	VOID _CRTAPI1 main()
	{
		SC_HANDLE SCManager = OpenSCManager( 
			NULL,                    // local machine 
			NULL,                    // ServicesActive database 
		    SC_MANAGER_ALL_ACCESS);  // 

		if(!SCManager) return;

		SC_HANDLE SCServ=OpenService(SCManager,"FOService",SERVICE_QUERY_CONFIG);
		if(!SCServ)
		{
			//������������ ������
			HMODULE hm=GetModuleHandle(NULL);
			char pathstr[1024];
			GetModuleFileName(hm,pathstr,1024);
			SCServ=CreateService(SCManager,"FOService","FOService",SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS,SERVICE_AUTO_START,SERVICE_ERROR_NORMAL,pathstr,NULL,NULL,NULL,NULL,NULL);
			CloseServiceHandle(SCServ);
			CloseServiceHandle(SCManager);
			MessageBox(NULL,"Registering FOserver service: OK.\nStart service from the control panel","Registering",MB_OK|MB_ICONEXCLAMATION);
			return;
		}
		LPQUERY_SERVICE_CONFIG lpqscBuf; 
	    DWORD dwBytesNeeded; 

	    lpqscBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, 4096); 
	 
	    // Get and print the information configuration. 
	 
	    if (!QueryServiceConfig(SCServ,lpqscBuf,4096,&dwBytesNeeded)) 
	    {
			CloseServiceHandle(SCServ);
			CloseServiceHandle(SCManager);
			LocalFree(lpqscBuf);     
			return;
	    }
		int last=0;
		for(int i=0;lpqscBuf->lpBinaryPathName[i];i++)
			if(lpqscBuf->lpBinaryPathName[i]=='\\') last=i;
		lpqscBuf->lpBinaryPathName[last+1]=0;
		SetCurrentDirectory(lpqscBuf->lpBinaryPathName);
		CloseServiceHandle(SCManager);
		LocalFree(lpqscBuf);     

		SERVICE_TABLE_ENTRY   DispatchTable[] = 
	    { 
	        { TEXT("FOService"), FOServiceStart}, 
	        { NULL, NULL }, 
	    };

		StartServiceCtrlDispatcher( DispatchTable);
	}

	void WINAPI FOServiceStart (DWORD argc, LPTSTR *argv)  
	{
	    FOServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS; 
	    FOServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
	    FOServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP; 
	    FOServiceStatus.dwWin32ExitCode      = 0; 
	    FOServiceStatus.dwServiceSpecificExitCode = 0; 
	    FOServiceStatus.dwCheckPoint         = 0; 

	    FOServiceStatus.dwWaitHint           = 0; 

	    FOServiceStatusHandle = RegisterServiceCtrlHandler( 
	        TEXT("FOService"), 
	        FOServiceCtrlHandler); 
	 
	    if (!FOServiceStatusHandle) 
	    { 
	        return;
	    } 
	 
		hGameThread=CreateThread(NULL,0,GameLoopThread,NULL,0,&dwGameThreadID);
	 
	    // Initialization complete - report running status. 
	    FOServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
	    FOServiceStatus.dwCheckPoint         = 0; 
	    FOServiceStatus.dwWaitHint           = 0; 
	 
	    SetServiceStatus (FOServiceStatusHandle, &FOServiceStatus);
	 
	    return; 
	} 
	
	VOID WINAPI FOServiceCtrlHandler (DWORD Opcode)  
	{ 
	    switch(Opcode) 
	    {
		case SERVICE_CONTROL_STOP: 
	
	        // Do whatever it takes to stop here. 
	            FOServiceStatus.dwWin32ExitCode = 0; 
	            FOServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING; 
	            FOServiceStatus.dwCheckPoint    = 0; 
	            FOServiceStatus.dwWaitHint      = 0; 
	 
	            SetServiceStatus (FOServiceStatusHandle, 
	                &FOServiceStatus);
	
				FOQuit=1;
				bQuit=1;
				CloseHandle(hLogFile);
				hLogFile=NULL;
				if(hGameThread)
					WaitForSingleObject(hGameThread,5000);
				CloseHandle(hGameThread);

	            FOServiceStatus.dwWin32ExitCode = 0; 
	            FOServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
	            FOServiceStatus.dwCheckPoint    = 0; 
	            FOServiceStatus.dwWaitHint      = 0; 

	            SetServiceStatus (FOServiceStatusHandle, 
	                &FOServiceStatus);

	            return; 
	         case SERVICE_CONTROL_INTERROGATE: 
	        // Fall through to send current status. 
	            break;
	        default:
				break;
	    }
	 
	    // Send current status. 
	    SetServiceStatus (FOServiceStatusHandle,  &FOServiceStatus);

	    return;
	}


	void LogExecStr(char* frmt, ...)
	{
		if(!logging) return;

	//	if(bQuit) return;

		if(!hLogFile) //!Cvet �������
		{
			hLogFile=CreateFile(bQuit?".\\FOserv!.log":".\\FOserv.log",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_FLAG_WRITE_THROUGH,NULL);
			if(hLogFile==INVALID_HANDLE_VALUE) return;
		}
		
		char str[2048];
	
	    va_list list;
	
	    va_start(list, frmt);
	    wvsprintf(str, frmt, list);
	    va_end(list);
	
		DWORD br;
		WriteFile(hLogFile,str,strlen(str),&br,NULL);
	}


	DWORD WINAPI GameLoopThread(void *)
	{
		if(!serv.Init()) goto GAMELOOPEND;
		
		logging=GetPrivateProfileInt("LOGGING","logging",TRUE,".\\foserv.cfg");
		if(logging!=FALSE && logging!=TRUE) logging=TRUE;

		serv.RunGameLoop();

		logging=TRUE;

		LogExecStr("����� ������:%d\n"
		"������� ����������������� �����:%d\n"
		"����������� ����������������� �����:%d\n"
		"������������ ����������������� �����:%d\n"
		"����� ����� (>100):%d\n",
		serv.loop_cycles,
		serv.loop_time/serv.loop_cycles,
		serv.loop_min,
		serv.loop_max,
		serv.lags_count);

		LogExecStr("������� ����������������� �� ���������:\n"
			"����:%d\n"
			"����������:%d\n"
			"�����:%d\n"
			"��������� �������:%d\n"
			"��������� ���:%d\n"
			"�������:%d\n"
			"����������:%d\n",
			serv.lt_FDsel/serv.loop_cycles,
			serv.lt_conn/serv.loop_cycles,
			serv.lt_input/serv.loop_cycles,
			serv.lt_proc_cl/serv.loop_cycles,
			serv.lt_proc_pc/serv.loop_cycles,
			serv.lt_output/serv.loop_cycles,
			serv.lt_discon/serv.loop_cycles);

		serv.Finish();
		
	GAMELOOPEND:	
		hGameThread=NULL;
		ExitThread(0);
		return 0;
	}
#endif

