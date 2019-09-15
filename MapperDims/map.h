//---------------------------------------------------------------------------
#ifndef mapH
#define mapH

#include <classes.hpp>
#include "utilites.h"
#include "macros.h"

extern CUtilites    *pUtil;

class CHeader
{
private:
   struct
   {
		DWORD ver;               // 0x00  ������ �����.

		WORD MapPID;             // 0x04 ID ��������� �����

		//!Cvet ��� ������, �������, �������� ���� ������
		DWORD ver_tile;         //0x06
		DWORD ver_scen;   		//0x0A
		DWORD ver_obj;          //0x0E

		DWORD PlLimit;          //0x12

		DWORD Reserved;			//0x16 CRC of header

		WORD  DefPlX[MAX_GROUPS_ON_MAP];			//0x1A..0x2C ����� ������ �� X
		WORD  DefPlY[MAX_GROUPS_ON_MAP];            //0x2E..0x40 ����� ������ �� Y
		BYTE  DefPlDirection[MAX_GROUPS_ON_MAP];    //0x42..0x4B ����������� ������ �� ���������.

		DWORD buffer[5];       //0x4C..0x60 !Cvet ����������� ��������� �� ������� 4

		DWORD CRC;			   //0x16 CRC of header

		//0x64 end of header
   } header;                    // ��������� FOnline

   WORD width, height;          // �������.

// ��� ������������� � ����. �������� ����
   struct
   {
	  DWORD ver;                // 0x0000  ������ �����.
	  char mapname[16];         // 0x0004  ��� �����
	  DWORD DefPlPos;           // 0x0014  ��������� ������� NPC
	  DWORD DefMapElev;         // 0x0018  ������� �� �������.
	  DWORD DefPlDirection;     // 0x001C  ����������� ������ �� ���������.
	  DWORD Num1;               // 0x0020  ���������� ��������� ����������
	  DWORD MapScriptID;        // 0x0024  ������ �����.
	  DWORD TilesCountID;       // 0x0028
	  DWORD Unknown4;           // 0x002C  1
	  DWORD Num2;               // 0x0030 :: #Map Global Vars (*.gam)
	  DWORD MapID;              // 0x0034
	  DWORD GameStartDate;      // 0x0038 :: Time since the epoch.
	  DWORD Unknown6[44];       // 0x003C -> 0x00EB
   } mapvars;                   // ��������� ������������ Fallout 1/2

   DWORD        *pMapGVars;     // ���������� ����������
   DWORD        *pMapLVars;     // ��������� ����������

   DWORD        Levels;         // ����� ������� �� �����

    // �������� ����������� �����
    DWORD ReCalcCRC(void);
    void  ResetAll(void);
    bool  LoadFromFile_Fallout(HANDLE h_map);
    bool LoadFromFile_FOnline(HANDLE h_map);
 public:
    bool bError;                         // ���� ������� ������

    CHeader(void);                       // �����������
    virtual ~CHeader(void);              // ����������
    void CreateNewMap(void);             // ������� ����� �����
    void SaveToFile(HANDLE h_map);       // ���������� ����� � �����
    void LoadFromFile(HANDLE h_map);     // �������� ����� �� �����

	DWORD GetVersion()			const {return header.ver;};

	WORD GetMapID()             const {return header.MapPID;};
 
//    DWORD GetMapScriptID()       const {return header.MapScriptID; };
//    void  GetMapName(char * out) const {strcpy(out, header.mapname);};

	WORD GetStartX(WORD num)    const {return num>=MAX_GROUPS_ON_MAP?0:header.DefPlX[num];}; //!Cvet add MAX_GROUPS_ON_MAP
	WORD GetStartY(WORD num)    const {return num>=MAX_GROUPS_ON_MAP?0:header.DefPlY[num];}; //!Cvet add MAX_GROUPS_ON_MAP
	DWORD GetStartDir(WORD num) const {return num>=MAX_GROUPS_ON_MAP?0:header.DefPlDirection[num];}; //!Cvet add MAX_GROUPS_ON_MAP

	//!Cvet ���� ������ ��� �����
	DWORD GetVersion_tile()     const {return header.ver_tile;};
	DWORD GetVersion_scen()     const {return header.ver_scen;};
	DWORD GetVersion_obj()      const {return header.ver_obj;};

	DWORD GetWidth()            const {return width;};
	DWORD GetHeight()           const {return height;};

	DWORD GetLevels()           const {return Levels;};

    // ������ �����
    void SetVersion(DWORD version) {header.ver = version;};
    // ������������� ��� ��������� ������ ������� ����� (� ������).
    // � ������ �������������� �� ������� Fallout � FOnline
    void  SetWidth(WORD w);
    void  SetHeight(WORD h);
   // ����� �������� �����
//   void SetMapName(char * name);
   // ����� �������
   void SetMapScriptID(DWORD id);
   // ����� �����
   void SetMapID(WORD id);
	// ������� ������ �� �����
   void SetStartPos(WORD num, WORD x, WORD y, WORD dir); //!Cvet add WORD num
};
//---------------------------------------------------------------------------

#endif
