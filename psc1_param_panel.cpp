/****************************************************************************
* Name: psc1_param_panel.cpp
* Psc1_ParamPanel class
*
* JLP
* Version 17/07/2015
****************************************************************************/
#include "psc1_param_panel.h"

BEGIN_EVENT_TABLE(Psc1_ParamPanel, wxPanel)

// Buttons
EVT_BUTTON(ID_PARAM_VALIDATE, Psc1_ParamPanel::OnValidate)
EVT_BUTTON(ID_PARAM_CANCEL, Psc1_ParamPanel::OnCancel)

EVT_TEXT(ID_PARAM_PRESSURE, Psc1_ParamPanel::OnTextChange)
EVT_TEXT(ID_PARAM_TEMPERATURE, Psc1_ParamPanel::OnTextChange)
EVT_TEXT(ID_PARAM_HYGROMETRY, Psc1_ParamPanel::OnTextChange)
EVT_TEXT(ID_PARAM_RA_OFFSET, Psc1_ParamPanel::OnTextChange)
EVT_TEXT(ID_PARAM_RB_OFFSET, Psc1_ParamPanel::OnTextChange)
EVT_COMBOBOX(ID_PARAM_UTSHIFT, Psc1_ParamPanel::OnSelectCombo)
EVT_COMBOBOX(ID_PARAM_RA_SIGN, Psc1_ParamPanel::OnSelectCombo)
EVT_COMBOBOX(ID_PARAM_RA_SIGN, Psc1_ParamPanel::OnSelectCombo)

END_EVENT_TABLE()


// ============================================================================
// implementation
// ============================================================================

/*******************************************************************************
* Constructor from wxFrame:
*******************************************************************************/
Psc1_ParamPanel::Psc1_ParamPanel(wxFrame *frame, wxString *str_messg,
                                 const int n_messg)
                  : wxPanel( frame )
{
int k;
wxString input_filename;

 initialized = 0;

// Transform coma into point for numbers:
 setlocale(LC_NUMERIC, "C");

// Background colour:
 SetBackgroundColour(*wxLIGHT_GREY);

// Smaller fonts (needed for Windows, to have a similar appearance as Linux)
// SetWindowVariant(wxWINDOW_VARIANT_SMALL);

// Messages (in different languages)
 m_messg = new wxString[n_messg];
 for(k = 0; k < n_messg; k++) m_messg[k] = str_messg[k];

 ParamPanel_Setup();

 initialized = 1234;

 SetDefaultValues();

// Update display (after setting initialized to 1234) :
 DisplayNewValues();

// Initialize those values (at the end !)
 ChangesDone1 = false;
 ValidatedChanges1 = false;

return;
}
/********************************************************************
*
********************************************************************/
void Psc1_ParamPanel::SetDefaultValues()
{
 hygrometry = 50.;
 temperature = 10.;
 pressure = 1013.;
// Valid in January 2016 : UT_shift=-1
 UT_shift = -1;
/* From september 1994 until december 1995:
OFFSET_RA 569
OFFSET_RB 247
*/
/* February 1996: we get a better correction with an
offset of (-50,-44) (equiv to -17 degrees for the bonnette):
#define OFFSET_RA 519
#define OFFSET_RB 203
*/
/* January 1997: null dispersion with 57,214,
and alignment to North with 511,156 (measured with E. Aristidi) */
/*
#define OFFSET_RA 511
#define OFFSET_RB 156
*/
// CALERN: Red to North with RA=987 RB=597 : measured with Marco January 2016
 ra_offset = 987;
 rb_offset = 597;
/* CAR September 1994: increasing code number to the West (North on top) */
/* And both prisms have the same orientation */
 ra_sign = 1;
 rb_sign = 1;
}
/********************************************************************
*
********************************************************************/
void Psc1_ParamPanel::DisplayNewValues()
{
int index;
wxString buffer;

buffer.Printf(wxT("%.1f"), temperature);
PscCtrl_Temperature->SetValue(buffer);

buffer.Printf(wxT("%.1f"), pressure);
PscCtrl_Pressure->SetValue(buffer);

buffer.Printf(wxT("%.1f"), hygrometry);
PscCtrl_Hygrometry->SetValue(buffer);

index = UT_shift + 4;
 PscCmb_UTShift.combo->SetSelection(index);

buffer.Printf(wxT("%d"), ra_offset);
PscCtrl_RA_offset->SetValue(buffer);

buffer.Printf(wxT("%d"), rb_offset);
PscCtrl_RB_offset->SetValue(buffer);

if(ra_sign == -1)
  PscCmb_RA_sign.combo->SetSelection(0);
else
  PscCmb_RA_sign.combo->SetSelection(1);

if(rb_sign == -1)
  PscCmb_RB_sign.combo->SetSelection(0);
else
  PscCmb_RB_sign.combo->SetSelection(1);

return;
}
/********************************************************************
* Pisco Setup panel
********************************************************************/
int Psc1_ParamPanel::ParamPanel_Setup()
{
wxBoxSizer *w_topsizer, *w_hsizer1, *w_hsizer2;
wxBoxSizer *param_left_sizer, *param_right_sizer;
wxButton *button_validate, *button_cancel;

  w_topsizer = new wxBoxSizer(wxVERTICAL);

  w_hsizer1 = new wxBoxSizer( wxHORIZONTAL );

  param_left_sizer = new wxBoxSizer(wxVERTICAL);
  ParamPanel_LeftSetup(param_left_sizer);
  w_hsizer1->Add(param_left_sizer, 0, wxTOP, 20);

  param_right_sizer = new wxBoxSizer(wxVERTICAL);
  ParamPanel_RightSetup(param_right_sizer);
  w_hsizer1->Add(param_right_sizer, 0, wxLEFT | wxTOP, 20);

  w_topsizer->Add(w_hsizer1, 0, wxALIGN_CENTER);

// Add two buttons at bottom:
// i=192 "Validate"
  button_validate = new wxButton(this, ID_PARAM_VALIDATE, m_messg[192]);
// i=193 "Cancel"
  button_cancel = new wxButton(this, ID_PARAM_CANCEL, m_messg[193]);
  w_hsizer2 = new wxBoxSizer( wxHORIZONTAL );
  w_hsizer2->Add(button_validate);
  w_hsizer2->Add(button_cancel, 0, wxLEFT, 30);
  w_topsizer->Add(w_hsizer2, 0, wxALIGN_CENTER | wxTOP, 30);

  this->SetSizer(w_topsizer);

  Centre();

return(0);
}
/********************************************************************
* Setup parameter panel (left side)
********************************************************************/
void Psc1_ParamPanel::ParamPanel_LeftSetup(wxBoxSizer *param_left_sizer)
{
wxStaticBoxSizer *atm_sizer;
int irows, icols, vgap = 20, hgap = 16, wwidth = 100;
wxFlexGridSizer *fgs1;

// Sizer surrounded with a rectangle, with a title on top:

// i=180 "Atmosphere"
 atm_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[180]);
 irows = 3;
 icols = 2;
 fgs1 = new wxFlexGridSizer(irows, icols, vgap, hgap);

// i=181 "Pressure (mbar)"
 fgs1->Add(new wxStaticText(this, -1, m_messg[181]));
 PscCtrl_Pressure = new wxTextCtrl(this, ID_PARAM_PRESSURE, wxT("1013"),
                                   wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs1->Add(PscCtrl_Pressure);

// i=182 "Temperature (deg C)"
 fgs1->Add(new wxStaticText(this, -1, m_messg[182]));
 PscCtrl_Temperature = new wxTextCtrl(this, ID_PARAM_TEMPERATURE, wxT("10."),
                                   wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs1->Add(PscCtrl_Temperature);

// i=183 "Hygrometry (percent)"
 fgs1->Add(new wxStaticText(this, -1, m_messg[183]));
 PscCtrl_Hygrometry = new wxTextCtrl(this, ID_PARAM_HYGROMETRY, wxT("80"),
                                   wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs1->Add(PscCtrl_Hygrometry);

 atm_sizer->Add(fgs1, 0, wxALL, 20);
 param_left_sizer->Add(atm_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 20);

return;
}
/********************************************************************
* Setup parameter panel (right side)
********************************************************************/
void Psc1_ParamPanel::ParamPanel_RightSetup(wxBoxSizer *param_right_sizer)
{
wxStaticBoxSizer *time_sizer, *risley_sizer;
int irows, icols, vgap = 12, hgap = 12, wwidth = 100;
wxFlexGridSizer *fgs1, *fgs2;

// Sizer surrounded with a rectangle, with a title on top:
// i=190 "Universal Time"
 time_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[190]);
 irows = 2;
 icols = 2;
 fgs1 = new wxFlexGridSizer(irows, icols, vgap, hgap);

// i=191 "U.T. Shift (h)"
 PscCmb_UTShift.label = m_messg[191];
 PscCmb_UTShift.nchoices = 5;
 PscCmb_UTShift.choices[0] = wxT("-4");
 PscCmb_UTShift.choices[1] = wxT("-3");
 PscCmb_UTShift.choices[2] = wxT("-2");
 PscCmb_UTShift.choices[3] = wxT("-1");
 PscCmb_UTShift.choices[4] = wxT("0");
 PscCmb_UTShift.combo = new wxComboBox(this, ID_PARAM_UTSHIFT, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_UTShift.nchoices,
                                       PscCmb_UTShift.choices);
 fgs1->Add(new wxStaticText(this, -1, PscCmb_UTShift.label));
 fgs1->Add(PscCmb_UTShift.combo);

 time_sizer->Add(fgs1, 0, wxALL, 20);
 param_right_sizer->Add(time_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 20);

// i=194 "Risley prisms"
 risley_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[194]);
 irows = 4;
 icols = 2;
 fgs2 = new wxFlexGridSizer(irows, icols, vgap, hgap);

// RA_offset
 fgs2->Add(new wxStaticText(this, -1, wxT("RA_offset")));
 PscCtrl_RA_offset = new wxTextCtrl(this, ID_PARAM_RA_OFFSET, wxT(""),
                                   wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs2->Add(PscCtrl_RA_offset);

// RB_offset
 fgs2->Add(new wxStaticText(this, -1, wxT("RB_offset")));
 PscCtrl_RB_offset = new wxTextCtrl(this, ID_PARAM_RB_OFFSET, wxT(""),
                                   wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs2->Add(PscCtrl_RB_offset);

// RA_sign
 PscCmb_RA_sign.label = wxT("RA_sign");
 PscCmb_RA_sign.nchoices = 2;
 PscCmb_RA_sign.choices[0] = wxT("-1");
 PscCmb_RA_sign.choices[1] = wxT("+1");
 PscCmb_RA_sign.combo = new wxComboBox(this, ID_PARAM_RA_SIGN, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_RA_sign.nchoices,
                                       PscCmb_RA_sign.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_RA_sign.label));
 fgs2->Add(PscCmb_RA_sign.combo);

// RB_sign
 PscCmb_RB_sign.label = wxT("RB_sign");
 PscCmb_RB_sign.nchoices = 2;
 PscCmb_RB_sign.choices[0] = wxT("-1");
 PscCmb_RB_sign.choices[1] = wxT("+1");
 PscCmb_RB_sign.combo = new wxComboBox(this, ID_PARAM_RB_SIGN, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_RB_sign.nchoices,
                                       PscCmb_RB_sign.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_RB_sign.label));
 fgs2->Add(PscCmb_RB_sign.combo);

 risley_sizer->Add(fgs2, 0, wxALL, 20);
 param_right_sizer->Add(risley_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 20);

return;
}
/****************************************************************************
*
****************************************************************************/
void Psc1_ParamPanel::OnTextChange(wxCommandEvent& WXUNUSED(event))
{
  if(initialized != 1234) return;
  ChangesDone1 = true;
  ValidatedChanges1 = false;
}
/****************************************************************************
*
****************************************************************************/
void Psc1_ParamPanel::OnSelectCombo(wxCommandEvent& event)
{
  if(initialized != 1234) return;
  ChangesDone1 = true;
  ValidatedChanges1 = false;
}
/****************************************************************************
*
****************************************************************************/
void Psc1_ParamPanel::OnCancel(wxCommandEvent& event)
{

if(initialized != 1234) return;
CancelNonValidatedChanges();

return;
}
/**************************************************************************
* Handle "Config" buttons:
*
* (also called by OnPageChanging when leaving this panel)
*
**************************************************************************/
void Psc1_ParamPanel::CancelNonValidatedChanges()
{

  SetDefaultValues();

// Update display
  DisplayNewValues();

// Reset following values
// (needed when this routines is called by OnPageChanging
// when leaving this panel)
  ChangesDone1 = false;
  ValidatedChanges1 = false;
}
/****************************************************************************
*
****************************************************************************/
void Psc1_ParamPanel::OnValidate(wxCommandEvent& event)
{
if(initialized != 1234) return;
ValidateChanges();
return;
}
/****************************************************************************
*
****************************************************************************/
void Psc1_ParamPanel::ValidateChanges()
{
int index, status;
double dvalue;
long ivalue;
wxString buffer;

// Get temperature:
buffer = PscCtrl_Temperature->GetValue();
  if(buffer.ToDouble(&dvalue)) {
   status = 1;
     if(dvalue > -20. && dvalue < 50.) {
       temperature = dvalue;
       status = 0;
     }
  } else {
   status = -1;
  }

if(status) {
  wxMessageBox(wxT("Error reading temperature"), wxT("ValidateChanges"),
               wxOK | wxICON_ERROR);
  buffer.Printf(wxT("%.1f"), temperature);
  PscCtrl_Temperature->SetValue(buffer);
  return;
}

// Get pressure:
buffer = PscCtrl_Pressure->GetValue();
  if(buffer.ToDouble(&dvalue)) {
   status = 1;
     if(dvalue > 0. && dvalue < 2000.) {
       pressure = dvalue;
       status = 0;
     }
  } else {
   status = -1;
  }

if(status) {
  wxMessageBox(wxT("Error reading pressure"), wxT("ValidateChanges"),
               wxOK | wxICON_ERROR);
  buffer.Printf(wxT("%.1f"), pressure);
  PscCtrl_Pressure->SetValue(buffer);
  return;
}

// Get hygrometry:
buffer = PscCtrl_Hygrometry->GetValue();
  if(buffer.ToDouble(&dvalue)) {
   status = 1;
     if(dvalue >= 0. && dvalue <= 100.) {
       hygrometry = dvalue;
       status = 0;
     }
  } else {
   status = -1;
  }

if(status) {
  wxMessageBox(wxT("Error reading hygrometry"), wxT("ValidateChanges"),
               wxOK | wxICON_ERROR);
  buffer.Printf(wxT("%.1f"), hygrometry);
  PscCtrl_Hygrometry->SetValue(buffer);
  return;
}

index = PscCmb_UTShift.combo->GetSelection();
UT_shift = index - 4;

// Get RA_offset
buffer = PscCtrl_RA_offset->GetValue();
  if(buffer.ToLong(&ivalue)) {
   status = 1;
     if(ivalue >= 0 && ivalue <= 1023) {
       ra_offset = ivalue;
       status = 0;
     }
  } else {
   status = -1;
  }

if(status) {
  wxMessageBox(wxT("Error reading RA_offset"), wxT("ValidateChanges"),
               wxOK | wxICON_ERROR);
  buffer.Printf(wxT("%d"), ra_offset);
  PscCtrl_RA_offset->SetValue(buffer);
  return;
}

// Get RB_offset
buffer = PscCtrl_RB_offset->GetValue();
  if(buffer.ToLong(&ivalue)) {
   status = 1;
     if(ivalue >= 0 && ivalue <= 1023) {
       rb_offset = ivalue;
       status = 0;
     }
  } else {
   status = -1;
  }

if(status) {
  wxMessageBox(wxT("Error reading RB_offset"), wxT("ValidateChanges"),
               wxOK | wxICON_ERROR);
  buffer.Printf(wxT("%d"), rb_offset);
  PscCtrl_RB_offset->SetValue(buffer);
  return;
}

index = PscCmb_RA_sign.combo->GetSelection();
if(index == 0) ra_sign = -1;
else ra_sign = + 1;

index = PscCmb_RB_sign.combo->GetSelection();
if(index == 0) rb_sign = -1;
else rb_sign = + 1;

// Set following values
// (needed when this routines is called by OnPageChanging
// when leaving this panel)
  ChangesDone1 = false;
  ValidatedChanges1 = true;
return;
}
