//---------------------------------------------------------------------------
#ifndef listsH
#define listsH

#include <classes.hpp>
#include "utilites.h"
extern CUtilites *pUtil;

class CListFiles
{
public:
   bool lError;

   int nFRM_count;              // ����� ���-�� �������
   int nPRO_count;              // ����� ���-�� ����������

   TStringList *pFRMlst[8];
   TStringList *pPROlst[6];
   TStringList *pScript;        // �������

   CListFiles(void);
   virtual ~CListFiles();

protected:
};
//---------------------------------------------------------------------------
#endif
