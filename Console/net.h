#include "StdAfx.h"
#include "BufMngr.h"
#include "zlib\zlib.h"

// ������ ��� ������ � �����
// Author: Gefke Denis [!Deniska]

#define BUF_SIZE 2048

//////////////////////////////////////////////////////////////////////////
// ���������� ���������� ������
extern z_stream net_zstrm;							// ��������� ��� �������
extern SOCKET net_sockfd;							// �������������� �����
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool Output(CBufMngr & buf);						// ������� ������
bool Input(CBufMngr & buf);							// ����� ������
bool InputCompressed(CBufMngr & buf);				// ����� ������ ������
SOCKET InitNet(void);								// ������������� �������� ����������
int Connect(char * host, WORD port);				// �������. host, port - �����
void Disconnect(void);								// ����������
void FinishNet(void);								// ���������� ������ ������
//SOCKET GetSocket(void) {return net_sockfd;};		// ���������� �����

void CheckNet(timeval tv);							// �������� �� ���������� ������
int ISSetRead(void);								// ����� ����� � ������
int ISSetWrite(void);								// ����� ����� � ������
int ISSetExc(void);									// ��������� �� ������

#define NET_OK						 1				// ��� ��
#define NET_ERR_NOT_INIT			-1				// ��������� �� ���������������
#define NET_ERR_INVALID_ADDRESS		-2				// �������� �����
#define NET_ERR_SERVER_IS_DOWN		-3				// ���������� �������������� � �������

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////