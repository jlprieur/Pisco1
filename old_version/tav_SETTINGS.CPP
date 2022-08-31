//----------------------------------------------------------------------------
// Settings of the RS232 link
//
// From Borland Visual Solutions Pack
// (C) Copyright 1993 by Borland International
//
// JLP
// Version of 20/11/2005
//----------------------------------------------------------------------------
#include <owl\owlpch.h>
#include <owl\applicat.h>
#include <owl\framewin.h>
/* For TStatic and TButton as external variables: */
#include <owl\static.h>
#include <owl\button.h>
/* For Comm as external variable: */
#include <stdio.h>

#include "tav.h"
#include "settings.h"

DEFINE_RESPONSE_TABLE1(SaxTermSettingsDlg, TDialog)
  EV_BN_CLICKED(IDOK, OkClicked),
END_RESPONSE_TABLE;

static char *szParity = "Parity";
static char *szSpeed = "Speed";
static char *szDataBits = "DataBits";
static char *szStopBits = "StopBits";
static struct {
   char *szProp;
   int nValue;
   int nID;
} aLookup[] = {
   { szParity, 0, IDC_PARITYNONE },
   { szParity, 1, IDC_PARITYEVEN },
   { szParity, 2, IDC_PARITYODD },
   { szParity, 3, IDC_PARITYMARK },
   { szParity, 4, IDC_PARITYSPACE },
   { szSpeed,  300, IDC_BAUD300 },
   { szSpeed,  600, IDC_BAUD600 },
   { szSpeed, 1200, IDC_BAUD1200 },
   { szSpeed, 2400, IDC_BAUD2400 },
   { szSpeed, 4800, IDC_BAUD4800 },
   { szSpeed, 9600, IDC_BAUD9600 },
   { szSpeed,19200, IDC_BAUD19200 },
   { szDataBits, 5, IDC_DATA5 },
   { szDataBits, 6, IDC_DATA6 },
   { szDataBits, 7, IDC_DATA7 },
   { szDataBits, 8, IDC_DATA8 },
   { szStopBits, 0, IDC_STOP15 },
   { szStopBits, 1, IDC_STOP1 },
   { szStopBits, 2, IDC_STOP2 },
   { NULL, 0, 0 }
};

SaxTermSettingsDlg::SaxTermSettingsDlg(TWindow* parent,
    TResId resId, TModule* module):
    TDialog(parent, resId, module)
{
  List = new TListBox(this, IDC_PORTLIST);
  Comm = TYPESAFE_DOWNCAST(parent, TVbxControl);
}
//************************************************************************
SaxTermSettingsDlg::~SaxTermSettingsDlg()
{
  Destroy();
  delete List;
}
//************************************************************************
void SaxTermSettingsDlg::SetupWindow()
{
char buffer[50];
int index, n = 0;
  TDialog::SetupWindow();


//  Default values (COM1 for the new computer in 2011):
  Comm->SetProp("Port", "COM1:");
  Comm->SetProp("Speed", 9600);
  Comm->SetProp("Parity", 0);

  for (int i = 0 ; aLookup[i].szProp; ++i) {
    Comm->GetProp(aLookup[i].szProp, n);
    if (n == aLookup[i].nValue)
      CheckDlgButton(aLookup[i].nID, 1);
  }
  Comm->GetProp("CtsRts", n);
  if (n)
    CheckDlgButton(IDC_HANDSHAKINGHARDWARE, 1);
  else {
    Comm->GetProp("XonXoff", n);
    CheckDlgButton(n ? IDC_HANDSHAKINGXONXOFF : IDC_HANDSHAKINGNONE, 1);
  }
// Fill the items of the port ComboBox:
  List->AddString("(none)");
  List->AddString("COM1:");
  List->AddString("COM2:");
  List->AddString("COM3:");
  List->AddString("COM4:");
// Select default value for the port ComboBox:
  string s;
  Comm->GetProp("Port", s);
  index = (s.length() > 3) ? s[3] - '0' : 0;
  List->SetSelIndex(index);
//  sprintf(buffer,"string[3]=%c select=%d \n",s[3], index);
//  ::MessageBox(NULL,buffer,"Received", MB_OK);

return;
}

void SaxTermSettingsDlg::OkClicked()
{
int n;
char scrap[10];
  for (int i = 0; aLookup[i].szProp; ++i) {
    if (IsDlgButtonChecked(aLookup[i].nID))
      Comm->SetProp(aLookup[i].szProp, aLookup[i].nValue);
  }
  Comm->SetProp("CtsRts", IsDlgButtonChecked(IDC_HANDSHAKINGHARDWARE) ? 1 : 0);
  Comm->SetProp("XonXoff", IsDlgButtonChecked(IDC_HANDSHAKINGXONXOFF) ? 1 : 0);
  n = List->GetSelIndex();
  if (n) {
    sprintf(scrap, "COM%d", n);
    no_rs232link_wanted = 0;
/* "None" port selected: */
    }
  else {
    sprintf(scrap, " ");
    no_rs232link_wanted = 1;
    }

   Comm->SetProp("Port", scrap);

  TDialog::CmOk();
return;
}

