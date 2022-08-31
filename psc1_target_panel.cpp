/****************************************************************************
* Name: psc1_target_panel.cpp
* Psc1_TargetPanel class
*
* JLP
* Version 14/01/2015
****************************************************************************/
#include "psc1_frame_id.h"
#include "psc1_target_panel.h"
//#include "tav_utils.h"   // convert_coord()

// ============================================================================
// implementation
// ============================================================================

/*******************************************************************************
* Constructor from wxFrame:
*******************************************************************************/
Psc1_TargetPanel::Psc1_TargetPanel(wxFrame *frame, Psc1_PiscoPanel *psc_pisco_panel,
                                   wxString *str_messg, const int n_messg)
                                   : wxPanel( frame )
{
int k;
wxString input_filename;

 initialized = 0;
 m_psc1_pisco_panel = psc_pisco_panel;

// Transform coma into point for numbers:
 setlocale(LC_NUMERIC, "C");

// Background colour:
 SetBackgroundColour(*wxLIGHT_GREY);

// Smaller fonts (needed for Windows, to have a similar appearance as Linux)
// SetWindowVariant(wxWINDOW_VARIANT_SMALL);

// Messages (in different languages)
 m_messg = new wxString[n_messg];
 for(k = 0; k < n_messg; k++) m_messg[k] = str_messg[k];

 TargetPanel_Setup();

 initialized = 1234;

// Initialize all private variables:
 InitPrivateParameters();

// Update display (after setting initialized to 1234) :
 DisplayNewValues();

// Initialize those values (at the end !)
 ChangesDone1 = false;
 ValidatedChanges1 = false;

return;
}
/********************************************************************
* Init all private parameters
********************************************************************/
void Psc1_TargetPanel::InitPrivateParameters()
{

// Interactive input:
alpha_value = 2.34;
delta_value = -1.2;
equin_value = 2000.;
target_name = wxT("SCA9876Aa,Ab");
target_type = 0;

ntargets_found = 0;

// WDS catalog:
WdsCatalog_fname = wxT("");

// Selection parameters:
bpr1.alpha_range = 1.;
bpr1.alpha_min = 0.;
bpr1.delta_max = 90.;
bpr1.delta_min = 0.;
bpr1.mag_max = 10.;
bpr1.mag_min = 5.;
bpr1.rho_min = 0.1;
bpr1.rho_max = 2.0;

// Selected object:
bpa1.Star_name = wxT("");
bpa1.WDS_name = wxT("");
bpa1.WDS2_name = wxT("");
bpa1.Spectral_type = wxT("");
bpa1.Discover_name = wxT("");
bpa1.Companion_name = wxT("");
bpa1.alpha = 0.;
bpa1.delta = 0.;
bpa1.dmag = 0.;
bpa1.epoch = 2000.;
bpa1.mag = 0.;
bpa1.rho = 0.;
bpa1.theta = 0.;

return;
}
/********************************************************************
* Setup target panel
********************************************************************/
int Psc1_TargetPanel::TargetPanel_Setup()
{
wxBoxSizer *w_topsizer, *w_hsizer1;
wxBoxSizer *target_left_sizer, *target_right_sizer;

 w_topsizer = new wxBoxSizer(wxVERTICAL);

 w_hsizer1 = new wxBoxSizer( wxHORIZONTAL );

 target_left_sizer = new wxBoxSizer(wxVERTICAL);
 TargetPanel_LeftSetup(target_left_sizer);
 w_hsizer1->Add(target_left_sizer, 0, wxTOP | wxALL, 20);

 target_right_sizer = new wxBoxSizer(wxVERTICAL);
 TargetPanel_RightSetup(target_right_sizer);
 w_hsizer1->Add(target_right_sizer, 0, wxTOP | wxALL, 20);

 w_topsizer->Add(w_hsizer1, 0, wxALIGN_CENTER);

 this->SetSizer(w_topsizer);

 Centre();

return(0);
}
/********************************************************************
* Setup target panel (left side)
********************************************************************/
void Psc1_TargetPanel::TargetPanel_LeftSetup(wxBoxSizer *target_left_sizer)
{
wxStaticBoxSizer *type_sizer, *limits_sizer, *found_sizer;
wxFlexGridSizer *fgs1, *fgs2, *fgs3;
wxBoxSizer *w_hsizer1, *w_hsizer2;
wxButton *button_next, *button_prev;
wxButton *button_open_wds, *button_select, *button_find_all;
int i, irows, icols, vgap = 10, hgap = 12, wwidth = 120;

// *************** Target-type: ***************************************

// Sizer surrounded with a rectangle, with a title on top:
// i=300 "Target type"
 type_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[300]);
 irows = 4;
 icols = 2;
 fgs1 = new wxFlexGridSizer(irows, icols, vgap, hgap);
// Target type
// i=301 "Type:"
 PscCmb_TargetType.label = m_messg[301];
 PscCmb_TargetType.nchoices = 4;
// i=303 "Astronomical object"
 PscCmb_TargetType.choices[0] = m_messg[303];
 PscCmb_TargetType.choices[1] = wxT("Offset");
 PscCmb_TargetType.choices[2] = wxT("Flat field");
 PscCmb_TargetType.choices[3] = wxT("Test");
 PscCmb_TargetType.combo = new wxComboBox(this, ID_TARGET_TYPE, wxT(""),
                                       wxDefaultPosition, wxSize(240, 28),
                                       PscCmb_TargetType.nchoices,
                                       PscCmb_TargetType.choices);
 fgs1->Add(new wxStaticText(this, -1, PscCmb_TargetType.label));
 fgs1->Add(PscCmb_TargetType.combo);

// i=302 "Name:"
 wwidth = 160;
 fgs1->Add(new wxStaticText(this, -1, m_messg[302]));
// Target name:
 PscStatic_TargetName = new wxStaticText(this, -1, wxT(""),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs1->Add(PscStatic_TargetName);

 type_sizer->Add(fgs1, 0, wxALL, 10);
 target_left_sizer->Add(type_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 10);

 w_hsizer1 = new wxBoxSizer( wxHORIZONTAL );

// i=308 "Open WDS catalog"
 button_open_wds = new wxButton(this, ID_TARGET_OPEN_WDS, m_messg[308]);
 w_hsizer1->Add(button_open_wds);

// WDS catalog filename:
 PscStatic_WdsCatalog = new wxStaticText(this, wxID_ANY, wxT(""),
                                       wxPoint(-1,-1), wxSize(wwidth, 28));
 w_hsizer1->Add(PscStatic_WdsCatalog, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 20);


 target_left_sizer->Add(w_hsizer1, 0, wxALL, 30);

// *************** Limits for automatic selection: ************************

// Sizer surrounded with a rectangle, with a title on top:
// i=310 "Automatic search"
 limits_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[310]);
 irows = 4;
 icols = 8;
 wwidth = 80;
 fgs2 = new wxFlexGridSizer(irows, icols, vgap, hgap);
// Alpha0
// i=311 "Alpha min"
 PscCmb_Alpha0.label = m_messg[311];
 PscCmb_Alpha0.nchoices = 24;
 for(i = 0; i < 24; i++) PscCmb_Alpha0.choices[i].Printf(wxT("%d"), i);
 PscCmb_Alpha0.combo = new wxComboBox(this, ID_TARGET_ALPHA0, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_Alpha0.nchoices,
                                       PscCmb_Alpha0.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_Alpha0.label));
 fgs2->Add(PscCmb_Alpha0.combo);

// AlphaRange
// i=312 "Alpha range"
 PscCmb_AlphaRange.label = m_messg[312];
 PscCmb_AlphaRange.nchoices = 24;
 for(i = 0; i < 24; i++) PscCmb_AlphaRange.choices[i].Printf(wxT("%d"), i + 1);
 PscCmb_AlphaRange.combo = new wxComboBox(this, ID_TARGET_ALPHARANG, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_AlphaRange.nchoices,
                                       PscCmb_AlphaRange.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_AlphaRange.label));
 fgs2->Add(PscCmb_AlphaRange.combo);

// DeltaMin
 PscCmb_DeltaMin.label = wxT("Delta min.");
 PscCmb_DeltaMin.nchoices = 11;
// Max=80 Min=-20
 for(i = 0; i < 11; i++) PscCmb_DeltaMin.choices[i].Printf(wxT("%d"), -20 + 10*i);
 PscCmb_DeltaMin.combo = new wxComboBox(this, ID_TARGET_DELTAMIN, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_DeltaMin.nchoices,
                                       PscCmb_DeltaMin.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_DeltaMin.label));
 fgs2->Add(PscCmb_DeltaMin.combo);

// DeltaMax
 PscCmb_DeltaMax.label = wxT("Delta max.");
 PscCmb_DeltaMax.nchoices = 11;
// Max=90 Min=-10
 for(i = 0; i < 11; i++) PscCmb_DeltaMax.choices[i].Printf(wxT("%d"), 90 - 10*i);
 PscCmb_DeltaMax.combo = new wxComboBox(this, ID_TARGET_DELTAMAX, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_DeltaMax.nchoices,
                                       PscCmb_DeltaMax.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_DeltaMax.label));
 fgs2->Add(PscCmb_DeltaMax.combo);

// MagMin
 PscCmb_MagMin.label = wxT("m_V min.");
 PscCmb_MagMin.nchoices = 17;
// Min=0 Max=16
 for(i = 0; i < 17; i++) PscCmb_MagMin.choices[i].Printf(wxT("%.1f"), (double)i);
 PscCmb_MagMin.combo = new wxComboBox(this, ID_TARGET_MAGMIN, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_MagMin.nchoices,
                                       PscCmb_MagMin.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_MagMin.label));
 fgs2->Add(PscCmb_MagMin.combo);

// MagMax
 PscCmb_MagMax.label = wxT("m_V max.");
 PscCmb_MagMax.nchoices = 17;
// Min=0 Max=16
 for(i = 0; i < 17; i++) PscCmb_MagMax.choices[i].Printf(wxT("%.1f"), (float)i);
 PscCmb_MagMax.combo = new wxComboBox(this, ID_TARGET_MAGMAX, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_MagMax.nchoices,
                                       PscCmb_MagMax.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_MagMax.label));
 fgs2->Add(PscCmb_MagMax.combo);

// RhoMin
 PscCmb_RhoMin.label = wxT("rho min.");
 PscCmb_RhoMin.nchoices = 10;
// Min=0. Max=0.9
 for(i = 0; i < 10; i++) PscCmb_RhoMin.choices[i].Printf(wxT("%.1f"), 0.1 * (double)i);
 PscCmb_RhoMin.combo = new wxComboBox(this, ID_TARGET_RHOMIN, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_RhoMin.nchoices,
                                       PscCmb_RhoMin.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_RhoMin.label));
 fgs2->Add(PscCmb_RhoMin.combo);

// RhoMax
 PscCmb_RhoMax.label = wxT("rho max.");
 PscCmb_RhoMax.nchoices = 10;
// Min=1. Max=10. (no intersection with min values !)
 for(i = 0; i < 10; i++) PscCmb_RhoMax.choices[i].Printf(wxT("%.1f"), 1. + (double)i);
 PscCmb_RhoMax.combo = new wxComboBox(this, ID_TARGET_RHOMAX, wxT(""),
                                       wxDefaultPosition, wxSize(wwidth, 28),
                                       PscCmb_RhoMax.nchoices,
                                       PscCmb_RhoMax.choices);
 fgs2->Add(new wxStaticText(this, -1, PscCmb_RhoMax.label));
 fgs2->Add(PscCmb_RhoMax.combo);

 limits_sizer->Add(fgs2, 0, wxALL, 10);

 target_left_sizer->Add(limits_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 10);

// *************** Found object with the chosen limits: ************************

// Sizer surrounded with a rectangle, with a title on top:
// i=313 "Object found"
 found_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[313]);
 irows = 4;
 icols = 8;
 vgap = 10, hgap = 8, wwidth = 160;
 fgs3 = new wxFlexGridSizer(irows, icols, vgap, hgap);

// i=302 "Name:"
 fgs3->Add(new wxStaticText(this, -1, m_messg[302]));
// Selection star name:
 PscStatic_SelectionStarName = new wxStaticText(this, -1, wxT(""),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs3->Add(PscStatic_SelectionStarName);

// WDS
 fgs3->Add(new wxStaticText(this, -1, wxT("WDS :")));
// Selection WDS name:
 PscStatic_SelectionWDSName = new wxStaticText(this, -1, wxT(""),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs3->Add(PscStatic_SelectionWDSName);

// WDS2
 fgs3->Add(new wxStaticText(this, -1, wxT("WDS2 :")));
// Selection WDS2 name:
 PscStatic_SelectionWDS2Name = new wxStaticText(this, -1, wxT(""),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs3->Add(PscStatic_SelectionWDS2Name);

// "Mag, Delta mag:"
 fgs3->Add(new wxStaticText(this, -1, wxT("m_V :")));
// Mag, Delta mag:
 PscStatic_SelectionMagDmag = new wxStaticText(this, -1, wxT(""),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs3->Add(PscStatic_SelectionMagDmag);

// "rho, theta:"
 fgs3->Add(new wxStaticText(this, -1, wxT("rho, theta:")));
// Selection rho:
 PscStatic_SelectionRhoTheta = new wxStaticText(this, -1, wxT(""),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs3->Add(PscStatic_SelectionRhoTheta);

// i=314 "Epoch of last obs. :"
 fgs3->Add(new wxStaticText(this, -1, m_messg[314]));
// Selection epoch:
 PscStatic_SelectionEpoch = new wxStaticText(this, -1, wxT(""),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs3->Add(PscStatic_SelectionEpoch);
 found_sizer->Add(fgs3, 0, wxALL, 10);

// "SpecType:"
 fgs3->Add(new wxStaticText(this, -1, wxT("SpecType :")));
// SpecType:
 PscStatic_SelectionSpecType = new wxStaticText(this, -1, wxT(""),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs3->Add(PscStatic_SelectionSpecType);

 w_hsizer2 = new wxBoxSizer( wxHORIZONTAL );

// i=320 "Find/Next"
 button_next = new wxButton(this, ID_TARGET_NEXT, m_messg[320]);
 w_hsizer2->Add(button_next);
// i=321 "Previous"
 button_prev = new wxButton(this, ID_TARGET_PREV, m_messg[321]);
 w_hsizer2->Add(button_prev, 0, wxLEFT, 20);

// i=323 "Select"
 button_select = new wxButton(this, ID_TARGET_SELECT, m_messg[323]);
 w_hsizer2->Add(button_select, 0, wxLEFT, 20);

// i=324 "Save the whole selection"
 button_find_all = new wxButton(this, ID_TARGET_FIND_ALL, m_messg[324]);
 w_hsizer2->Add(button_find_all, 0, wxLEFT, 20);

 found_sizer->Add(w_hsizer2, 0, wxALL, 20);

 target_left_sizer->Add(found_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 10);


return;
}
/********************************************************************
* Setup target panel (right side)
********************************************************************/
void Psc1_TargetPanel::TargetPanel_RightSetup(wxBoxSizer *target_right_sizer)
{
wxStaticBoxSizer *astro_sizer;
wxBoxSizer *hsizer2;
int irows, icols, vgap = 12, hgap = 12, wwidth = 250;
wxFlexGridSizer *fgs1;
wxButton *button_find_object, *button_validate;

// *************** Astro-object: ***************************************

// Sizer surrounded with a rectangle, with a title on top:
// i=303 "Direct input"
 astro_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[303]);
 irows = 5;
 icols = 2;
 fgs1 = new wxFlexGridSizer(irows, icols, vgap, hgap);
// i=302 "Name"
 fgs1->Add(new wxStaticText(this, -1, m_messg[302]));
 PscCtrl_StarName = new wxTextCtrl(this, ID_TARGET_STAR_NAME, wxT("SKF1061"),
                               wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs1->Add(PscCtrl_StarName);
// i=309 "Search in catalog"
 button_find_object = new wxButton(this, ID_TARGET_FIND_OBJECT, m_messg[309]);
 fgs1->Add(new wxStaticText(this, -1, wxT("")));
 fgs1->Add(button_find_object);

// i=223 "Right ascension"
 fgs1->Add(new wxStaticText(this, -1, m_messg[223]));
 PscCtrl_Alpha = new wxTextCtrl(this, ID_TARGET_ALPHA, wxT("1 23 45.3"),
                                wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs1->Add(PscCtrl_Alpha);
// i=222 "Declination"
 fgs1->Add(new wxStaticText(this, -1, m_messg[222]));
 PscCtrl_Delta = new wxTextCtrl(this, ID_TARGET_DELTA, wxT("-5 43 21"),
                                wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs1->Add(PscCtrl_Delta);
// i=232 "Equinox"
 fgs1->Add(new wxStaticText(this, -1, m_messg[232]));
 PscCtrl_Equinox = new wxTextCtrl(this, ID_TARGET_EQUINOX, wxT("2000"),
                                  wxPoint(-1,-1), wxSize(wwidth, 28));
 fgs1->Add(PscCtrl_Equinox);

 astro_sizer->Add(fgs1, 0, wxALL, 10);
 target_right_sizer->Add(astro_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 10);

// Add some buttons at bottom:
// i=192 "Validate"
 button_validate = new wxButton(this, ID_TARGET_VALIDATE, m_messg[192]);
// i=193 "Cancel"
// button_cancel = new wxButton(this, ID_TARGET_CANCEL, m_messg[193]);
 hsizer2 = new wxBoxSizer( wxHORIZONTAL );
 hsizer2->Add(button_validate, 0, wxLEFT, 30);
 target_right_sizer->Add(hsizer2, 0, wxALIGN_CENTER | wxTOP, 30);

return;
}
