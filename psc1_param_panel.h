/************************************************************************
* "psc1_param_panel.h"
* Psc1_ParamPanel class
*
* JLP
* Version 17/08/2015
*************************************************************************/
#ifndef psc1_param_panel_h_
#define psc1_param_panel_h_

#include "psc1_frame_id.h"
#include "psc1_defs.h"       // JLP_ComboBox

class Psc1_ParamPanel : wxPanel
{

public:

// In "psc1_param_panel.cpp":

  Psc1_ParamPanel(wxFrame *frame, wxString *str_messg,
                  const int n_messg);
  ~Psc1_ParamPanel(){};

  int ParamPanel_Setup();
  void ParamPanel_LeftSetup(wxBoxSizer *param_left_sizer);
  void ParamPanel_RightSetup(wxBoxSizer *param_right_sizer);
  void SetDefaultValues();
  void DisplayNewValues();

  void GetAtmosphericParam(double *temp0, double *press0,
                           double *hygro0, int *shift0,
                           int *ra_offset0, int *rb_offset0,
                           int *ra_sign0, int *rb_sign0) {
    *temp0 = temperature;
    *press0 = pressure;
    *hygro0 = hygrometry;
    *shift0 = UT_shift;
    *ra_offset0 = ra_offset;
    *rb_offset0 = rb_offset;
    *ra_sign0 = ra_sign;
    *rb_sign0 = rb_sign;
    return;
    };

// Called when changing pages:
  bool ChangesDone(){return(ChangesDone1);}
  bool ValidatedChanges(){return(ValidatedChanges1);}
  void ValidateChanges();
  void CancelNonValidatedChanges();

protected:
  void OnValidate(wxCommandEvent& event);
  void OnCancel(wxCommandEvent& event);
  void OnTextChange(wxCommandEvent& event);
  void OnSelectCombo(wxCommandEvent& event);

private:

  // Messages in different languages:
  wxString *m_messg;

  bool ChangesDone1, ValidatedChanges1;

  int initialized;
  int UT_shift, ra_offset, rb_offset, ra_sign, rb_sign;
  double temperature, pressure, hygrometry;
  wxTextCtrl *PscCtrl_Temperature, *PscCtrl_Pressure, *PscCtrl_Hygrometry;
  wxTextCtrl *PscCtrl_RA_offset, *PscCtrl_RB_offset;
  JLP_ComboBox PscCmb_UTShift, PscCmb_RA_sign, PscCmb_RB_sign;

  DECLARE_EVENT_TABLE()
};

#endif // EOF sentry
