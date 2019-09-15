//---------------------------------------------------------------------------
#ifndef frmsetH
#define frmsetH

#include <classes.hpp>
#include <graphics.hpp>
#include "frame.h"
#include "utilites.h"
#include "log.h"
#include "objset.h"
#include "lists.h"

extern CUtilites    *pUtil;
extern CLog         *pLog;
extern CListFiles   *pLstFiles;


class CHeader;
//---------------------------------------------------------------------------
class CFrmSet
{
public:
   bool lError;

   CFrame       *pFRM[8];               // ������ ���� 8 �����
//   CFrame *pFRM_HEX;
   int          nFrmCount[8];           // �������� ������� ��������� ����� ( � ��� ����� ���������)
   DWORD        dwDirectionOffset[6];   // �������� ������ ������� ������� i���� �����������
                                        // ������������ ������ ������� ������
   signed short doffX[6];               // ��������� ����� ����� ��� X ������ ����������� i
   signed short doffY[6];               // ��������� ����� ����� ��� Y ������ ����������� i

   // ����� ���� ��������� �������
   void         ClearLocals(void);
   // �������� ���� ��������� �������
   void         LoadLocalFRMs(void);
   // �������� �������
   void         LoadFRM(BYTE nFrmType, DWORD nFrmID, bool bLocal);
   // ������������ ������ ������
   void         FreeUpFRM(CFrame *l_pFRM);
   void         GetCritterFName(String* filename, DWORD frmPID, WORD *frmID);
   bool         getSuffixes(DWORD ID1, DWORD ID2, char& Suffix1, char& Suffix2);

   CFrmSet(void);
   virtual ~CFrmSet();

protected:
};
//---------------------------------------------------------------------------
#endif
