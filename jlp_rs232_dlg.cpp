/******************************************************************************
* jlp_rs232_dlg.cpp
* Dialog box used to select RS232 serial port and settings
*
* Author:      JLP
* Version:     03/11/2015
******************************************************************************/
#include "jlp_rs232_dlg.h"
#include "jlp_rs232_in_c.h"

//*************************************************************************
enum
{
   ID_RS232_DLG_OK = 8300,
   ID_RS232_DLG_CANCEL,
   ID_RS232_DLG_COMSEL,
   ID_RS232_DLG_BAUDRATE,
};

BEGIN_EVENT_TABLE(JLP_RS232_Dlg, wxDialog)
EVT_BUTTON  (ID_RS232_DLG_OK, JLP_RS232_Dlg::OnOKButton)
EVT_BUTTON  (ID_RS232_DLG_CANCEL, JLP_RS232_Dlg::OnCancelButton)
EVT_COMBOBOX(ID_RS232_DLG_COMSEL, JLP_RS232_Dlg::OnRS232ComSelection)
EVT_COMBOBOX(ID_RS232_DLG_BAUDRATE, JLP_RS232_Dlg::OnRS232BaudrateSelection)
END_EVENT_TABLE()


/********************************************************************
* Constructor:
********************************************************************/
JLP_RS232_Dlg::JLP_RS232_Dlg(wxFrame *parent, const wxString &title,
                             wxString *str_messg, const int n_messg)
        : wxDialog(parent, -1, title, wxPoint(400,100), wxDefaultSize,
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
int portnum;
wxString buffer;
int iports[NPORTS_MAXI];

// To avoid initialization problems with Windows:
// (An event is sent to "OnSelectLanguage"
//  as soon as a Text control is created...)
  initialized = 0;

// Messages (in different languages)
 m_messg = new wxString[n_messg];
 for(int i = 0; i < n_messg; i++) m_messg[i] = str_messg[i];

// Initialize baudrate, and port number:
  baudrate1 = 9600;

  PanelSetup();

  initialized = 1234;

 // Find the available ports:
 portnum = RS232_Find_Ports(iports, NPORTS_MAXI);

 for(int i = 0; i < portnum; i++) {
    buffer.Printf(wxT("COM%d"), iports[i]);
    buffer.Trim();
    PscCombo_rs232_comSel->Append(buffer); // Showing the available ports in the comSel combo box
    }

// First choice is selected for port:
// comport_nber1=0 corresponds to COM1:
comport_nber1 = 0;
PscCombo_rs232_comSel->SetSelection(0);

// 9600 corresponds to index=4:
PscCmb_rs232_baudrate.combo->SetSelection(4);

return;
}
/********************************************************************
* Panel setup:
********************************************************************/
void JLP_RS232_Dlg::PanelSetup()
{
wxBoxSizer *topsizer, *button_sizer;
wxFlexGridSizer *fgs1;
wxStaticBoxSizer *rs232_sizer;
int nrows, ncols, vgap = 12, hgap = 12, wwidth = 100;

// Flexible grid sizer:
 nrows = 2;
 ncols = 2;
 fgs1 = new wxFlexGridSizer(nrows, ncols, vgap, hgap);

 topsizer = new wxBoxSizer( wxVERTICAL );

// Sizer surrounded with a rectangle, with a title on top:
// i=151 "Settings"
 rs232_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[151]);

// fgs1->Add(LangRadioButton[i]);
 PscCombo_rs232_comSel = new wxComboBox(this, ID_RS232_DLG_COMSEL, wxEmptyString,
                        wxDefaultPosition, wxSize(wwidth, 28));
// i=152 "Port"
 fgs1->Add(new wxStaticText(this, -1, m_messg[152]), 0, wxALIGN_CENTER_VERTICAL);
 fgs1->Add(PscCombo_rs232_comSel);

// Baudrate selection in ComboBox:
// 1200, 1800, 2400, 4800, 9600, 19200
// i=153 "Speed (baudrate)"
 PscCmb_rs232_baudrate.label = m_messg[153];
 PscCmb_rs232_baudrate.nchoices = 6;
 PscCmb_rs232_baudrate.choices[0] = wxT("1200");
 PscCmb_rs232_baudrate.choices[1] = wxT("1800");
 PscCmb_rs232_baudrate.choices[2] = wxT("2400");
 PscCmb_rs232_baudrate.choices[3] = wxT("4800");
 PscCmb_rs232_baudrate.choices[4] = wxT("9600");
 PscCmb_rs232_baudrate.choices[5] = wxT("19200");
 PscCmb_rs232_baudrate.combo = new wxComboBox(this, ID_RS232_DLG_BAUDRATE,
                                        wxT(""),
                                        wxDefaultPosition, wxSize(wwidth, 28),
                                        PscCmb_rs232_baudrate.nchoices,
                                        PscCmb_rs232_baudrate.choices);
 fgs1->Add(new wxStaticText(this, -1, PscCmb_rs232_baudrate.label),
           0, wxALIGN_CENTER_VERTICAL);
 fgs1->Add(PscCmb_rs232_baudrate.combo);

 rs232_sizer->Add(fgs1, 0, wxALIGN_CENTER|wxALL, 20);
 topsizer->Add(rs232_sizer, 0, wxALIGN_CENTER|wxALL, 20);

 button_sizer = new wxBoxSizer( wxHORIZONTAL );

//create two buttons that are horizontally unstretchable,
// with an all-around border with a width of 10 and implicit top alignment
 button_sizer->Add(
    new wxButton(this, ID_RS232_DLG_OK, _T("OK") ), 0, wxALIGN_LEFT|wxALL, 10);

 button_sizer->Add(
   new wxButton(this, ID_RS232_DLG_CANCEL, _T("Cancel") ), 0, wxALIGN_CENTER|wxALL, 10);

  //create a sizer with no border and centered horizontally
  topsizer->Add(button_sizer, 0, wxALIGN_CENTER);

  SetSizer(topsizer);   // use the sizer for layout

  topsizer->SetSizeHints( this );   // set size hints to honour minimum size

return;
}
/**************************************************************************
* Handle "OK" button:
**************************************************************************/
void JLP_RS232_Dlg::OnOKButton( wxCommandEvent& WXUNUSED(event) )
{
wxString buffer;
// 8bits No parity 1 stop bit 0=eof string
char mode1[] = {'8', 'N', '1', 0};

if(initialized != 1234) return;

// Open connection:
    if(!RS232_OpenComport(comport_nber1, baudrate1, mode1)) {
    buffer.Printf(wxT("Serial COM%d port successfuly opened\n\
 (%d bauds, 8 bits, no parity, 1 stop-bit)"),
                    comport_nber1 + 1, baudrate1);
    wxMessageBox(buffer, wxT("JLP_RS232_Dlg"), wxOK);
    } else {
    buffer.Printf(wxT("Error opening serial COM%d port\n\
 (%d bauds, 8 bits, no parity, 1 stop-bit)"),
                    comport_nber1 + 1, baudrate1);
    wxMessageBox(buffer, wxT("JLP_RS232_Dlg"), wxOK | wxICON_ERROR);
    return;
    }

// Close dialog and return status = 0:
  EndModal(0);
}
/**************************************************************************
* Handle "Cancel" button:
**************************************************************************/
void JLP_RS232_Dlg::OnCancelButton( wxCommandEvent& WXUNUSED(event) )
{
// Close dialog and return status = 1:
  EndModal(1);
}
/***************************************************************
* Serial port selection
***************************************************************/
void JLP_RS232_Dlg::OnRS232ComSelection(wxCommandEvent& WXUNUSED(event) )
{
wxString comSelected, buffer;
int comport_aux;

comSelected  = PscCombo_rs232_comSel->GetStringSelection();
comport_aux = (int)comSelected[3] - 48;
// comport_nber1 = 0 corresponds to COM1 :
  comport_nber1 = comport_aux - 1;
#ifdef DEBUG
buffer.Printf("comport_aux = %d", comport_aux);
wxMessageBox(buffer, comSelected, wxOK);
#endif
return;
}
/***************************************************************
* Baudrate selection in ComboBox:
* 1200, 1800, 2400, 4800, 9600, 19200
*
***************************************************************/
void JLP_RS232_Dlg::OnRS232BaudrateSelection(wxCommandEvent& WXUNUSED(event) )
{
int index;
  index = PscCmb_rs232_baudrate.combo->GetSelection();
  switch(index) {
    case 0:
      baudrate1 = 1200;
      break;
    case 1:
      baudrate1 = 1800;
      break;
    case 2:
      baudrate1 = 2400;
      break;
    case 3:
      baudrate1 = 4800;
      break;
    default:
    case 4:
      baudrate1 = 9600;
      break;
    case 5:
      baudrate1 = 19200;
      break;
  }
return;
}
