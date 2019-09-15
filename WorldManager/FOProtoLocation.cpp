#include "stdafx.h"
#include "FOProtoLocation.h"

/********************************************************************
	created:	22:05:2007   19:39

	author:		Anton Tvetinsky aka Cvet
	
	purpose:	
*********************************************************************/

BOOL CProtoLocation::Init(WORD _pid, CFileMngr* _fm)
{
	if(pid) return FALSE;
	if(!_pid || !_fm) return FALSE;

	pid=_pid;
	fm=_fm;

	proto_maps.clear();

	return TRUE;
}

void CProtoLocation::Clear()
{
	pid=NULL;
	fm=NULL;
}

BOOL CProtoLocation::Load(int PathType)
{
	if(!pid || !fm) return FALSE;

	WriteLog("�������� ��������� ������� �%d...",pid);
	
	if(!pid || pid>=MAX_PROTO_LOCATIONS)
	{
		WriteLog("�������� ����� ��������� ��� �������� �� ������������������\n");
		return FALSE;
	}
	
	char name_proto[256];
	sprintf(name_proto,"%d%s",pid,LOCATION_PROTO_EXT);
	
	if(!fm->LoadFile(name_proto,PathType))
	{
		WriteLog("���� ��������� |%s| �� ������\n",name_proto);
		return FALSE;
	}
	
	fm->SetCurPos(0x00);
	DWORD proto_ver=fm->GetBEDWord();
	if(proto_ver!=VERSION_PROTOTYPE_LOCATION)
	{
		WriteLog("������ ��������� �� ��������������\n");
		fm->UnloadFile();
		return FALSE;
	}
	
	BYTE proto_type=fm->GetByte();
	
	BREAK_BEGIN
		
		if(proto_type==LOCATION_TYPE_CITY) break;
		if(proto_type==LOCATION_TYPE_ENCAUNTER) break;
		if(proto_type==LOCATION_TYPE_SPECIAL) break;
		if(proto_type==LOCATION_TYPE_QUEST) break;
	
		fm->UnloadFile();
		WriteLog("�������� ��� ���������\n");
		return FALSE;
		
	BREAK_END;
		
	type=proto_type;
		
	fm->SetCurPos(0x0A);

	radius=fm->GetByte();
	max_players=fm->GetBEDWord();

	WORD count_maps=fm->GetBEWord();
	if(!count_maps)
	{
		WriteLog("� ������� ��� �� ����� �����\n");
		return FALSE;
	}

	proto_maps.clear();
	for(int i=0;i<count_maps;++i) proto_maps.push_back(fm->GetBEWord());

	switch(type)
	{
	case LOCATION_TYPE_CITY:
		CITY.steal=fm->GetBEWord();
		CITY.count_starts_maps=fm->GetBEWord();
		CITY.population=fm->GetBEDWord();
		CITY.life_level=fm->GetBEDWord();
		break;
	case LOCATION_TYPE_ENCAUNTER:
		{
			ENCAUNTER.district=fm->GetBEWord();

			ENCAUNTER.max_groups=fm->GetBEWord();
			if(ENCAUNTER.max_groups>MAX_GROUPS_ON_MAP)
			{
				WriteLog("����������� ����� ��������� ������������ � ����������\n");
				return FALSE;
			}
		}
		break;
	case LOCATION_TYPE_SPECIAL:
		SPECIAL.reserved=0;
		break;
	case LOCATION_TYPE_QUEST:
		QUEST.reserved=0;
		break;
	}

	if(fm->GetBEDWord()!=0xFFFFFFFF) //����� ��������� � ����������� �����
	{
		WriteLog("�� ������� ����������� �����\n");
		fm->UnloadFile();
		return FALSE;		
	}

	fm->UnloadFile();
	WriteLog("OK\n");

	return TRUE;
}

BOOL CProtoLocation::Save(int PathType)
{
	WriteLog("������ ��������� ������� �%d...",pid);

	if(!pid || !fm)
	{
		WriteLog("�������� �� ���������������\n");
		return FALSE;
	}

	WORD count_maps=(WORD)proto_maps.size();
	if(!count_maps)
	{
		WriteLog("� ������� ��� �� ����� �����\n");
		return FALSE;
	}
	
	fm->ClearOutBuf();

	fm->SetPosOutBuf(0x00);

	fm->SetBEDWord(VERSION_PROTOTYPE_LOCATION);
	
	fm->SetByte(GetType());

	fm->SetPosOutBuf(0x0A);

	fm->SetByte(radius);
	fm->SetBEDWord(max_players);
	
	fm->SetBEWord(count_maps);

	for(int i=0;i<count_maps;++i) fm->SetBEWord(proto_maps[i]);

	switch(type)
	{
	case LOCATION_TYPE_CITY:
		fm->SetBEWord(CITY.steal);
		fm->SetBEWord(CITY.count_starts_maps);
		fm->SetBEDWord(CITY.population);
		fm->SetBEDWord(CITY.life_level);
		break;
	case LOCATION_TYPE_ENCAUNTER:
		{
			fm->SetBEWord(ENCAUNTER.district);

			fm->SetBEWord(ENCAUNTER.max_groups);
		}
		break;
	case LOCATION_TYPE_SPECIAL:
		SPECIAL.reserved=0;
		break;
	case LOCATION_TYPE_QUEST:
		QUEST.reserved=0;
		break;
	}

	fm->SetBEDWord(0xFFFFFFFF); //����� ��������� � ����������� �����

	char proto_fname[128];
	sprintf(proto_fname,"%d%s",GetPid(),LOCATION_PROTO_EXT);

	if(!fm->SaveOutBufToFile(&proto_fname[0],PathType))
	{
		fm->ClearOutBuf();
		return FALSE;
	}

	fm->ClearOutBuf();

	WriteLog("OK\n");

	return TRUE;
}