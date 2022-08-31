/*********************************************************F*******************
* Name: psc1_target_panel_onclick.cpp
* Psc1_TargetPanel class
*
* JLP
* Version 13/01/2016
****************************************************************************/
#include "psc1_frame_id.h"
#include "psc1_target_panel.h"
#include "psc1_pisco_panel.h"
#include "read_WDS_catalog.h"
#include "jlp_string.h"          // jlp_compact_string,...
#include "tav_utils.h"           // convert_coord()

// Contained (and used) here:
static void AddToFoundTargets(Binary_Parameters bpa1, Binary_Parameters *bpa_found,
                              int *ntargets_found, const int nmax_targets);
static void CopyBinaryParameters(Binary_Parameters *bpa0, Binary_Parameters bpa1);

BEGIN_EVENT_TABLE(Psc1_TargetPanel, wxPanel)

// Buttons
EVT_BUTTON(ID_TARGET_VALIDATE, Psc1_TargetPanel::OnValidate)
EVT_BUTTON(ID_TARGET_VALIDATE, Psc1_TargetPanel::OnCancel)
EVT_BUTTON(ID_TARGET_NEXT, Psc1_TargetPanel::OnNext)
EVT_BUTTON(ID_TARGET_SELECT, Psc1_TargetPanel::OnSelectTarget)
EVT_BUTTON(ID_TARGET_PREV, Psc1_TargetPanel::OnPrevious)
EVT_BUTTON(ID_TARGET_FIND_OBJECT, Psc1_TargetPanel::OnFindObjectInWdsCatalog)
EVT_BUTTON(ID_TARGET_FIND_ALL, Psc1_TargetPanel::OnFindAllInCatalog1)
EVT_BUTTON(ID_TARGET_OPEN_WDS, Psc1_TargetPanel::OnOpenWdsCatalog)

EVT_TEXT(ID_TARGET_STAR_NAME, Psc1_TargetPanel::OnChangeText)
EVT_TEXT(ID_TARGET_ALPHA, Psc1_TargetPanel::OnChangeText)
EVT_TEXT(ID_TARGET_DELTA, Psc1_TargetPanel::OnChangeText)
EVT_TEXT(ID_TARGET_EQUINOX, Psc1_TargetPanel::OnChangeText)
EVT_COMBOBOX(ID_TARGET_ALPHA0, Psc1_TargetPanel::OnSelectLimits)
EVT_COMBOBOX(ID_TARGET_ALPHARANG, Psc1_TargetPanel::OnSelectLimits)
EVT_COMBOBOX(ID_TARGET_DELTAMIN, Psc1_TargetPanel::OnSelectLimits)
EVT_COMBOBOX(ID_TARGET_DELTAMAX, Psc1_TargetPanel::OnSelectLimits)
EVT_COMBOBOX(ID_TARGET_RHOMIN, Psc1_TargetPanel::OnSelectLimits)
EVT_COMBOBOX(ID_TARGET_RHOMAX, Psc1_TargetPanel::OnSelectLimits)
EVT_COMBOBOX(ID_TARGET_MAGMIN, Psc1_TargetPanel::OnSelectLimits)
EVT_COMBOBOX(ID_TARGET_MAGMAX, Psc1_TargetPanel::OnSelectLimits)
EVT_COMBOBOX(ID_TARGET_TYPE, Psc1_TargetPanel::OnSelectTargetType)

END_EVENT_TABLE()

/********************************************************************
*
********************************************************************/
void Psc1_TargetPanel::DisplayNewValues()
{
wxString buffer;
int ih, im;
double ws;
char my_Hour[16], my_Dec[16];

strcpy(my_Hour, "H");
strcpy(my_Dec, "D");

// Target type:
  PscCmb_TargetType.combo->SetSelection(target_type);

// Name:
  if(target_type == 0) {
// Star name:
    PscCtrl_StarName->SetValue(target_name);
    PscStatic_TargetName->SetLabel(wxT(""));
  } else {
// Offset, etc :
    PscCtrl_StarName->SetValue(wxT(""));
    PscStatic_TargetName->SetLabel(target_name);
  }

// Right ascension:
  convert_coord(alpha_value, &ih, &im, &ws, my_Hour);
  buffer.Printf(wxT("%02d %02d %05.2f"), ih, im, ws);
  PscCtrl_Alpha->SetValue(buffer);

// Declination:
  convert_coord(delta_value, &ih, &im, &ws, my_Dec);
    if(ih == 0 && delta_value < 0.)
     buffer.Printf(wxT("-00 %02d %05.2f"), im, ws);
   else
     buffer.Printf(wxT("%02d %02d %04.1f"), ih, im, ws);
  PscCtrl_Delta->SetValue(buffer);

// Equinox:
  buffer.Printf(wxT("%.1f"), equin_value);
  PscCtrl_Equinox->SetValue(buffer);

// Selection subpanel:
  DisplayNewSelectionLimits();
  DisplayFoundSelection();

return;
}
/********************************************************************
* Update the values of selection limits from the combo boxes
********************************************************************/
void Psc1_TargetPanel::ReadSelectionLimits()
{
int i;

// Alpha0:
//  for(i = 0; i < 24; i++) PscCmb_Alpha0.choices[i].Printf(wxT("%d"), i);
i = PscCmb_Alpha0.combo->GetSelection();
bpr1.alpha_min = (double)i;

// AlphaRange:
// for(i = 0; i < 24; i++) PscCmb_AlphaRange.choices[i].Printf(wxT("%d"), i + 1);
i = PscCmb_AlphaRange.combo->GetSelection();
bpr1.alpha_range = (double)i + 1.;

// Delta min:
// Max=80 Min=-20
//  for(i = 0; i < 11; i++) PscCmb_DeltaMin.choices[i].Printf(wxT("%d"), -20 + 10*i);
i = PscCmb_DeltaMin.combo->GetSelection();
bpr1.delta_min = (double)(i * 10) - 20.;

// Delta max:
// Max=90 Min=-10
// for(i = 0; i < 11; i++) PscCmb_DeltaMax.choices[i].Printf(wxT("%d"), 90 - 10*i);
i = PscCmb_DeltaMax.combo->GetSelection();
bpr1.delta_max = 90. - (double)(i * 10);

// Mag min:
// Min=0 Max=16
// for(i = 0; i < 17; i++) PscCmb_MagMin.choices[i].Printf(wxT("%d"), i);
i = PscCmb_MagMin.combo->GetSelection();
bpr1.mag_min = (double)i;

// Mag max:
// Min=0 Max=16
// for(i = 0; i < 17; i++) PscCmb_MagMax.choices[i].Printf(wxT("%.1f"), (float)i);
i = PscCmb_MagMax.combo->GetSelection();
bpr1.mag_max = (double)i;

// RhoMin
// Min=0. Max=0.9
// for(i = 0; i < 10; i++) PscCmb_RhoMin.choices[i].Printf(wxT("%.1f"), 0.1 * (double)i);
i = PscCmb_RhoMin.combo->GetSelection();
bpr1.rho_min = ((double)i) / 10.;

// RhoMax
// Min=1. Max=10. (no intersection with min values !)
// for(i = 0; i < 10; i++) PscCmb_RhoMax.choices[i].Printf(wxT("%d"), 1 + i);
i = PscCmb_RhoMax.combo->GetSelection();
bpr1.rho_max = 1. + (double)i;

return;
}
/********************************************************************
* Update the combo boxes with the values of the selection limits
********************************************************************/
void Psc1_TargetPanel::DisplayNewSelectionLimits()
{
int i;

// Alpha0:
//  for(i = 0; i < 24; i++) PscCmb_Alpha0.choices[i].Printf(wxT("%d"), i);
i = (int)bpr1.alpha_min;
PscCmb_Alpha0.combo->SetSelection(i);

// AlphaRange:
// for(i = 0; i < 24; i++) PscCmb_AlphaRange.choices[i].Printf(wxT("%d"), i + 1);
i = (int)bpr1.alpha_range - 1;
PscCmb_AlphaRange.combo->SetSelection(i);

// Delta min:
// Max=50 Min=-20
//  for(i = 0; i < 8; i++) PscCmb_DeltaMin.choices[i].Printf(wxT("%d"), -20 + 10*i);
i = (int)(bpr1.delta_min + 20.) /10;
PscCmb_DeltaMin.combo->SetSelection(i);

// Delta max:
// Max=90 Min=20
// for(i = 0; i < 8; i++) PscCmb_DeltaMax.choices[i].Printf(wxT("%d"), 90 - 10*i);
i = (int)(90. - bpr1.delta_max) /10;
PscCmb_DeltaMax.combo->SetSelection(i);

// Mag min:
// Min=0 Max=16
// for(i = 0; i < 17; i++) PscCmb_MagMin.choices[i].Printf(wxT("%d"), i);
i = (int)(bpr1.mag_min);
PscCmb_MagMin.combo->SetSelection(i);

// Mag max:
// Min=0 Max=16
// for(i = 0; i < 17; i++) PscCmb_MagMax.choices[i].Printf(wxT("%.1f"), (float)i);
i = (int)(bpr1.mag_max);
PscCmb_MagMax.combo->SetSelection(i);

// RhoMin
// Min=0. Max=0.9
// for(i = 0; i < 10; i++) PscCmb_RhoMin.choices[i].Printf(wxT("%.1f"), 0.1 * (double)i);
i = (int)(bpr1.rho_min * 10.);
PscCmb_RhoMin.combo->SetSelection(i);

// RhoMax
// Min=1. Max=10. (no intersection with min values !)
// for(i = 0; i < 10; i++) PscCmb_RhoMax.choices[i].Printf(wxT("%d"), 1 + i);
i = (int)(bpr1.rho_max - 1.);
PscCmb_RhoMax.combo->SetSelection(i);

return;
}
/********************************************************************
* Display the parameters of the object found by the automated search
*
********************************************************************/
void Psc1_TargetPanel::DisplayFoundSelection()
{
wxString buffer;

// Name:
  if(!(bpa1.Companion_name).IsEmpty())
    bpa1.Star_name = bpa1.Discover_name + bpa1.Companion_name;
  else
    bpa1.Star_name = bpa1.Discover_name;

  PscStatic_SelectionStarName->SetLabel(bpa1.Star_name);
  PscStatic_SelectionWDSName->SetLabel(bpa1.WDS_name);
  PscStatic_SelectionWDS2Name->SetLabel(bpa1.WDS2_name);
  PscStatic_SelectionSpecType->SetLabel(bpa1.Spectral_type);

// Measurements:
  buffer.Printf(wxT("%.2f %d"), bpa1.rho, (int)bpa1.theta);
  PscStatic_SelectionRhoTheta->SetLabel(buffer);

// Magnitude/Delta mag:
  if(bpa1.dmag < 0)
    buffer.Printf(wxT("%.1f"), bpa1.mag);
  else
    buffer.Printf(wxT("%.1f (Dm=%.1f)"), bpa1.mag, bpa1.dmag);
  PscStatic_SelectionMagDmag->SetLabel(buffer);

// Epoch:
  buffer.Printf(wxT("%d"), (int)bpa1.epoch);
  PscStatic_SelectionEpoch->SetLabel(buffer);

return;
}
/****************************************************************************
* Text Control
****************************************************************************/
void Psc1_TargetPanel::OnChangeText(wxCommandEvent& WXUNUSED(event))
{
if(initialized != 1234) return;
  ChangesDone1 = true;
  ValidatedChanges1 = false;
return;
}
/****************************************************************************
* TargetType combo box
****************************************************************************/
void Psc1_TargetPanel::OnSelectTargetType(wxCommandEvent& WXUNUSED(event))
{
wxString buffer;

if(initialized != 1234) return;
  ChangesDone1 = true;
  ValidateChanges();

return;
}
/****************************************************************************
* Target limits with spin controls
****************************************************************************/
void Psc1_TargetPanel::OnSelectLimits(wxCommandEvent& WXUNUSED(event))
{
if(initialized != 1234) return;
  ChangesDone1 = true;
  ValidatedChanges1 = false;
return;
}
/****************************************************************************
*
****************************************************************************/
void Psc1_TargetPanel::OnCancel(wxCommandEvent& event)
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
void Psc1_TargetPanel::CancelNonValidatedChanges()
{
/*
 SetDefaultValues();
// Update display
 DisplayNewValues();
*/

// Reset following values
// (needed when this routines is called by OnPageChanging
// when leaving this panel)
  ChangesDone1 = false;
  ValidatedChanges1 = false;
}
/****************************************************************************
*
****************************************************************************/
void Psc1_TargetPanel::OnValidate(wxCommandEvent& event)
{
if(initialized != 1234) return;
ValidateChanges();
return;
}
/****************************************************************************
* Search forwards
*
****************************************************************************/
void Psc1_TargetPanel::OnNext(wxCommandEvent& event)
{
 if(initialized != 1234) return;

 FindNextObjectInWdsCatalog();
}
/****************************************************************************
* Search forwards
*
****************************************************************************/
void Psc1_TargetPanel::FindNextObjectInWdsCatalog()
{
int found = 0, status;
char cat_fname[128], save_fname[128], err_message[128];
bool search_all_and_save;

// First validate limits and update bpr1 :
 ReadSelectionLimits();

// If object was previously selected with this selection,
// increase alpha_min of selection to go forwards:
 if(ntargets_found > 0) {
   if((bpa1.alpha > bpr1.alpha_min) 
      && (bpa1.alpha < bpr1.alpha_min + bpr1.alpha_range))
      bpr1.alpha_min = bpa1.alpha;
 }

// Then performs search:
 search_all_and_save = false;
 strcpy(save_fname, "");
 strcpy(cat_fname, (const char *)WdsCatalog_fname.mb_str());

 status = search_in_WDS_catalog(cat_fname, bpr1, &bpa1, &found,
                                search_all_and_save, save_fname,
                                err_message);
if(status == 0 && found) {
// Display the results:
  DisplayFoundSelection();

// Add to internal array (used for "Previous" button)
  AddToFoundTargets(bpa1, bpa_found, &ntargets_found, NMAX_TARGETS);
}

return;
}
/****************************************************************************
* Search forwards
*
****************************************************************************/
void Psc1_TargetPanel::OnFindAllInCatalog1(wxCommandEvent& event)
{
wxString save_filename;
int status, found = 0;
char cat_fname[128], save_fname[128], err_message[128];
bool search_all_and_save;

if(initialized != 1234) return;

// Select name for output logbook file:
wxFileDialog saveFileDialog(this, wxT("Save selection to WDS-formatted file"),
                            wxT(""), wxT(""), wxT("Logbook files (*.txt)|*.txt"),
                            wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

// To avoid conflicts with modal dialog, stop the timers:
if((initialized == 1234) && (m_psc1_pisco_panel != NULL))
    m_psc1_pisco_panel->PauseMyTimers();

status = saveFileDialog.ShowModal();

// Restart the timers:
if((initialized == 1234) && (m_psc1_pisco_panel != NULL))
    m_psc1_pisco_panel->ResumeMyTimers();

if (status == wxID_CANCEL) return;
 save_filename = saveFileDialog.GetFilename();

// First validate limits and update bpr1 :
 ReadSelectionLimits();

// Then performs search:
 search_all_and_save = true;
 strcpy(save_fname, (const char *)save_filename.mb_str());
 strcpy(cat_fname, (const char *)WdsCatalog_fname.mb_str());
 status = search_in_WDS_catalog(cat_fname, bpr1, &bpa1, &found,
                                search_all_and_save, save_fname,
                                err_message);

return;
}
/****************************************************************************
* Search backwards
****************************************************************************/
void Psc1_TargetPanel::OnPrevious(wxCommandEvent& event)
{
if(initialized != 1234) return;
 if(ntargets_found > 1) {
  CopyBinaryParameters(&bpa1, bpa_found[ntargets_found - 2]);
  DisplayFoundSelection();
  ntargets_found--;
 }
return;
}
/****************************************************************************
* Select the found astronomical object
****************************************************************************/
void Psc1_TargetPanel::OnSelectTarget(wxCommandEvent& event)
{
if(initialized != 1234) return;

// SelectTargetType to astronomical:
target_type = 0;
PscCmb_TargetType.combo->SetSelection(0);

// Copy the parameters of the found object to the interactive panel:
target_name = bpa1.Discover_name + bpa1.Companion_name;
alpha_value = bpa1.alpha;
delta_value = bpa1.delta;
equin_value = 2000.;

DisplayNewValues();

ValidateChanges();

return;
}
/****************************************************************************
*
****************************************************************************/
void Psc1_TargetPanel::ValidateChanges()
{
wxString buffer;
char cbuffer[128], my_Hour[16], my_Dec[16];
int ih, im;
double ws, dvalue;

strcpy(my_Hour, "H");
strcpy(my_Dec, "D");

// Target type:
target_type = PscCmb_TargetType.combo->GetSelection();

// Get Name:
if(target_type == 0) {
 target_name = PscCtrl_StarName->GetValue();
} else {
  switch(target_type) {
    case 1:
     target_name = wxT("Offset");
     break;
    case 2:
     target_name = wxT("Flat field");
     break;
    default:
    case 3:
     target_name = wxT("Test");
     break;
  }
}
// Update static text:
PscStatic_TargetName->SetLabel(target_name);

// Alpha:
buffer = PscCtrl_Alpha->GetValue();
strcpy(cbuffer, (const char*)buffer.mb_str());

if(sscanf(cbuffer,"%d %d %lf", &ih, &im, &ws) == 3) {
  alpha_value = (double)ih + (double)im / 60. + ws / 3600.;
 } else {
  wxMessageBox(wxT("Error reading alpha"), wxT("ValidateChanges"),
               wxOK | wxICON_ERROR);
  convert_coord(alpha_value, &ih, &im, &ws, my_Hour);
  buffer.Printf(wxT("%02d h %02d m %02d s"), ih, im, (int)ws);
  PscCtrl_Alpha->SetValue(buffer);
  return;
 }

// Delta:
buffer = PscCtrl_Delta->GetValue();
strcpy(cbuffer, (const char*)buffer.mb_str());

if(sscanf(cbuffer,"%d %d %lf", &ih, &im, &ws) == 3) {
  if(ih >= 0)
    delta_value = (double)ih + (double)im / 60. + ws / 3600.;
  else
    delta_value = (double)ih - (double)im / 60. - ws / 3600.;
// Handle case of -00 32 45 for example:
  if(cbuffer[0] == '-') delta_value = (double)ih - (double)im / 60. - ws / 3600.;
 } else {
  wxMessageBox(wxT("Error reading delta"), wxT("ValidateChanges"),
               wxOK | wxICON_ERROR);
  convert_coord(delta_value, &ih, &im, &ws, my_Dec);
   if(ih == 0 && delta_value < 0.)
     buffer.Printf(wxT("-00 d %02d m %02d s"), im, (int)ws);
   else
     buffer.Printf(wxT("%02d d %02d m %02d s"), ih, im, (int)ws);
  PscCtrl_Delta->SetValue(buffer);
  return;
 }

// Equinox:
buffer = PscCtrl_Equinox->GetValue();
  if(buffer.ToDouble(&dvalue)) {
   if(dvalue >= 0. && dvalue <= 3000.) {
    equin_value = dvalue;
   } else {
    wxMessageBox(wxT("Error reading equinox"),
                 wxT("ValidateChanges"), wxOK | wxICON_ERROR);
    buffer.Printf(wxT("%.1f"), equin_value);
    PscCtrl_Equinox->SetValue(buffer);
    return;
   }
  }

// Set following values
// (needed when this routines is called by OnPageChanging
// when leaving this panel)
  ChangesDone1 = false;
  ValidatedChanges1 = true;
return;
}
/****************************************************************************
* Look for an object in JLP's double star catalog
****************************************************************************/
void Psc1_TargetPanel::OnFindObjectInWdsCatalog(wxCommandEvent& event)
{
int status;
wxString error_messg;

 if(initialized != 1234) return;

// Load object name from Text Control:
 target_name = PscCtrl_StarName->GetValue();

   status = FindObjectInWdsCatalog(error_messg);

 if(status) {
  wxMessageBox(error_messg, wxT("OnFindInWdsCatalog"), wxOK | wxICON_ERROR);
 } else {
// Update all fields with the coordinates found in this catalog:
  DisplayNewValues();
 }

return;
}
/*******************************************************************
*
*********************************************************************/
int Psc1_TargetPanel::FindObjectInWdsCatalog(wxString& error_messg)
{
int status, found;
char cat_fname[128], discov_name2[64], comp_name2[64], WDS_name2[64];
char err_message[128], *pc;
double alpha2, delta2;

if(initialized != 1234) return(-1);

// First get the name :
 target_name = PscCtrl_StarName->GetValue();

// Then performs search:
 strcpy(cat_fname, (const char *)WdsCatalog_fname.mb_str());
 strcpy(discov_name2, (const char *)target_name.mb_str());

// Look for companion if present:
 comp_name2[0] = '\0';
 pc = discov_name2;
 while(*pc) {
  if(!strncmp(pc, "AB", 2)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "AC", 2)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "BC", 2)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "CD", 2)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "Aa,", 3)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "Ab,", 3)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "Ba,", 3)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "Bb,", 3)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "Ca,", 3)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  else if(!strncmp(pc, "Cb,", 3)){
    strcpy(comp_name2, pc);
    *pc = '\0';
    break;
  }
  pc++;
 }
/* DEBUG
{
  wxString buff;
  buff.Printf(wxT("(disc= %s)(comp=%s)"), discov_name2, comp_name2);
  wxMessageBox(buff, wxT("ZZZ"), wxOK);
}
*/
 status = search_discov_name_in_WDS_catalog(cat_fname, discov_name2, comp_name2,
                                            WDS_name2, &alpha2, &delta2,
                                            &found, err_message);

 if(found) {
  alpha_value = alpha2;
  delta_value = delta2;
  DisplayNewValues();
 } else {
   wxMessageBox(wxT("Object not found in catalog"), wxT("OnFindInWdsCatalog"), wxOK);
  alpha_value = 0.;
  delta_value = 0.;
  DisplayNewValues();
 }

return(status);
}
/*******************************************************************************
*
*******************************************************************************/
static void AddToFoundTargets(Binary_Parameters bpa1, Binary_Parameters *bpa_found,
                              int *ntargets_found, const int nmax_targets)
{
int k;

// When upper limit is reached, shift all the parameters of all the previously found objects
if(*ntargets_found >= nmax_targets - 1) {
  for(k = 0; k < nmax_targets - 1; k++){
   CopyBinaryParameters(&bpa_found[k], bpa_found[k + 1]);
   }
   *ntargets_found = nmax_targets - 2;
 }

 CopyBinaryParameters(&bpa_found[*ntargets_found], bpa1);
 (*ntargets_found)++;
}
/******************************************************************************
*
******************************************************************************/
static void CopyBinaryParameters(Binary_Parameters *bpa0, Binary_Parameters bpa1)
{
 bpa0->Star_name = bpa1.Star_name;
 bpa0->WDS_name = bpa1.WDS_name;
 bpa0->WDS2_name = bpa1.WDS2_name;
 bpa0->Spectral_type = bpa1.Spectral_type;
 bpa0->Discover_name = bpa1.Discover_name;
 bpa0->Companion_name = bpa1.Companion_name;
 bpa0->alpha = bpa1.alpha;
 bpa0->delta = bpa1.delta;
 bpa0->rho = bpa1.rho;
 bpa0->theta = bpa1.theta;
 bpa0->mag = bpa1.mag;
 bpa0->dmag = bpa1.dmag;
 bpa0->epoch = bpa1.epoch;
}
