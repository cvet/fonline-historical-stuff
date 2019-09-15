//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TypeServer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTypeServerForm *TypeServerForm;
//---------------------------------------------------------------------------
__fastcall TTypeServerForm::TTypeServerForm(TComponent* Owner)
	: TForm(Owner)
{
	DoubleBuffered = true;
	Caption = LANG("������", "Server");
	LUpdaterHost->Caption = LANG("����� ������� ����������", "Updater server host");
	BtnDone->Caption = LANG("������", "Done");
	BtnCancel->Caption = LANG("������", "Cancel");
}
//---------------------------------------------------------------------------
void TTypeServerForm::GetResult(AnsiString& updaterHost, AnsiString& updaterPort)
{
	updaterHost = EditUpdaterHost->Text;
	updaterPort = EditUpdaterPort->Text;
}
//---------------------------------------------------------------------------
void __fastcall TTypeServerForm::BtnDoneClick(TObject *Sender)
{
	if(EditUpdaterHost->Text == "" || EditUpdaterPort->Text == "") return;
	ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TTypeServerForm::BtnCancelClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TTypeServerForm::EditUpdaterHostKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
	if(Key == VK_RETURN) BtnDoneClick(Sender);
}
//---------------------------------------------------------------------------

