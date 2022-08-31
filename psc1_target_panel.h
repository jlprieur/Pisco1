/************************************************************************
* "psc1_target_panel.h"
* Psc1_TargetPanel class
*
* JLP
* Version 17/08/2015
*************************************************************************/
#ifndef psc1_target_panel_h_
#define psc1_target_panel_h_

#include "psc1_defs.h"       // JLP_ComboBox

#define NMAX_TARGETS 128

class Psc1_PiscoPanel;

class Psc1_TargetPanel : wxPanel
{

public:

// In "psc1_target_panel.cpp":

  Psc1_TargetPanel(wxFrame *frame, Psc1_PiscoPanel *psc_pisco_panel,
                   wxString *str_messg, const int n_messg);
  ~Psc1_TargetPanel(){};

  int TargetPanel_Setup();
  void InitPrivateParameters();
  void TargetPanel_LeftSetup(wxBoxSizer *target_left_sizer);
  void TargetPanel_RightSetup(wxBoxSizer *target_right_sizer);


// Called when changing pages:
  bool ChangesDone(){return(ChangesDone1);}
  bool ValidatedChanges(){return(ValidatedChanges1);}

  void GetTargetParam(wxString& target_name0, int *target_type0,
                      double *alpha0,
                      double *delta0, double *equin0) {
    target_name0 = target_name;
    *target_type0 = target_type;
    *alpha0 = alpha_value;
    *delta0 = delta_value;
    *equin0 = equin_value;
    return;
    };

// In psc1_target_catalogs.cpp
  int FindObjectInWdsCatalog(wxString& err_messg);
  void OnOpenWdsCatalog(wxCommandEvent& event);
  void OnFindObjectInWdsCatalog(wxCommandEvent& event);

// psc1_target_panel_onclick.cpp
  void ValidateChanges();
  void CancelNonValidatedChanges();

protected:
// psc1_target_panel_onclick.cpp
  void DisplayNewValues();
  void DisplayNewSelectionLimits();
  void ReadSelectionLimits();
  void DisplayFoundSelection();
  void OnSelectWheelPosition(wxCommandEvent& event);
  void OnValidate(wxCommandEvent& event);
  void OnCancel(wxCommandEvent& event);
  void OnNext(wxCommandEvent& event);
  void FindNextObjectInWdsCatalog();
  void OnPrevious(wxCommandEvent& event);
  void OnSelectTarget(wxCommandEvent& event);
  void OnChangeText(wxCommandEvent& event);
  void OnSelectTargetType(wxCommandEvent& event);
  void OnSelectLimits(wxCommandEvent& event);
  void OnFindAllInCatalog1(wxCommandEvent& event);

private:

  Psc1_PiscoPanel *m_psc1_pisco_panel;

// Messages in different languages:
  wxString *m_messg;

  int initialized, target_type, ntargets_found;
  bool ChangesDone1, ValidatedChanges1;
  double alpha_value, delta_value, equin_value;
  wxString target_name, WdsCatalog_fname;
  Binary_Parameters bpa1, bpa_found[NMAX_TARGETS];
  Binary_Profile bpr1;

  wxTextCtrl *PscCtrl_StarName, *PscCtrl_Alpha, *PscCtrl_Delta, *PscCtrl_Equinox;
  wxStaticText *PscStatic_Date_LastObs, *PscStatic_Rho_LastObs, *PscStatic_Theta_LastObs;
  wxStaticText *PscStatic_TargetName, *PscStatic_WdsCatalog;
  wxStaticText *PscStatic_SelectionWDSName, *PscStatic_SelectionStarName;
  wxStaticText *PscStatic_SelectionWDS2Name, *PscStatic_SelectionSpecType;
  wxStaticText *PscStatic_SelectionEpoch;
  wxStaticText *PscStatic_SelectionRhoTheta, *PscStatic_SelectionMagDmag;
  JLP_ComboBox PscCmb_TargetType;
  JLP_ComboBox PscCmb_Alpha0, PscCmb_AlphaRange, PscCmb_DeltaMin, PscCmb_DeltaMax;
  JLP_ComboBox PscCmb_MagMin, PscCmb_MagMax, PscCmb_RhoMin, PscCmb_RhoMax;

  DECLARE_EVENT_TABLE()
};

#endif // EOF sentry
