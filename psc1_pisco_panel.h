/************************************************************************
* "psc1_pisco_panel.h"
* Psc1_PiscoPanel class
*
* JLP
* Version 17/08/2015
*************************************************************************/
#ifndef psc1_pisco_panel_h_
#define psc1_pisco_panel_h_

#include "psc1_defs.h"       // JLP_ComboBox
#include "jlp_wxlogbook.h"       // JLP_wxLogbook class

// #include "wx/timer.h"

class JLP_Rs232Thread;

class Psc1_PiscoPanel : wxPanel
{

public:

// In "psc1_pisco_panel.cpp":

  Psc1_PiscoPanel(wxFrame *panel_frame,
                  JLP_wxLogbook *jlp_logbook0, FILE *fp_logbook0,
                  wxString *str_messg, const int n_messg);
  ~Psc1_PiscoPanel();
  bool Close();
  void MyShutdown();

  void DisplayNewValues();
  int PiscoPanel_Setup();
  void PiscoPanel_LeftSetup(wxBoxSizer *pisco_left_sizer);
  void PiscoPanel_CenterSetup(wxBoxSizer *pisco_center_sizer);
  void PiscoPanel_RightSetup(wxBoxSizer *pisco_right_sizer);
  int InitPisco();
  int BootPisco();
  bool PiscoIsAlive();
  int LoadWheelPositionCodes(char *posi_fname);
  int LoadWheelChoices_from_file(char* tavconfig_fname);

// In "psc1_pisco_panel_onclick.cpp":
  void OnSelectWheelPosition(wxCommandEvent& event);
  void OnCheckPositions(wxCommandEvent& event);
  void CheckPositions();
  void OnLampSelect(wxCommandEvent& event);
  void OnAutoRisleySelect(wxCommandEvent& event);
  void OnStartStopExposure(wxCommandEvent& event);
  void OnValidateRisley(wxCommandEvent& event);
  int CurrentStatusFromRS232();
  int RS232Status_Decode_Nextlines(char *c1, int c1_len, int *ra_is_found,
                                   int *rb_is_found, int *nwheels_found,
                                   char *rs232_wheel_names,
                                   char *rs232_wheel_is_there);
  int RS232Status_Decode_First_TwoLines(char *rs232_wheel_names,
                                        char *rs232_wheel_is_there);


// In "psc1_pisco_logbook.cpp":
  void AddCommentsToLogbook();
  int GetCommentsForTheLogbook(wxString &comments0);
  int WriteToLogbook(wxString str1, bool SaveToFile);
  int Logbook_StartExposure();
  void Logbook_StopExposure();
  void PauseMyTimers();
  void ResumeMyTimers();

// In psc1_pisco_risley.cpp:
  void StartRisleyTimer();
  void StopRisleyTimer();
  void Init_RisleyPrisms();
  void OnRisleyTimer(wxTimerEvent& event);
  int ComputeCorrection(double *cross_angle);
  void ApplyCorrection(wxString &error_message);

// In psc1_pisco_filters.cpp:
  int FiltersFromConfigFile(char *tavconfig_fname);
  int FilterIndex();

// In psc1_pisco_rs232.cpp:
  void StartRs232Timer();
  void StopRs232Timer();
  void OnRS232ReceiverTimer(wxTimerEvent& event);
  int OpenRS232LinkAndStartPisco();
  void RS232_window_setup(wxBoxSizer *vsizer1);
  void OnRS232Send(wxCommandEvent& event);
  void RS232_SendCommand(char *command0);
  void RS232_SendCommand2(char *command1, int n_wanted,
                          wxString& answer_1, int *n_received,
                          wxString& error_message1);
  void UpdateReceiveBox(wxString buffer);
  void OnRS232Connection(wxCommandEvent& event);
  void OnRS232MessageBox(wxCommandEvent& event);
  int RS232GetTheAnswer(int n_wanted, wxString& answer_1,
                        int *n_received, wxString& error_message1);
  void RS232ReceiveMessage(wxString& message1, int* length1);

  void LoadAtmosphericParam(double temp0, double press0,
                            double hygro0, int shift0,
                            int ra_offset0, int rb_offset0,
                            int ra_sign0, int rb_sign0) {
    temp_value = temp0;
    press_value = press0;
    hygro_value = hygro0;
    UT_shift = shift0;
    ra_offset = ra_offset0;
    rb_offset = rb_offset0;
    ra_sign = ra_sign0;
    rb_sign = rb_sign0;
// Compute ra_null_disp and rb_null_disp:
    Init_RisleyPrisms();
    return;
    };
  void LoadTargetParam(wxString target_name0, int target_type0,
                       double alpha0, double delta0, double equin0) {
    ObjectName = target_name0;
    target_type = target_type0;
    alpha_value = alpha0;
    delta_value = delta0;
    equin_value = equin0;
    if(initialized == 1234) DisplayNewValues();
    return;
    };

protected:

private:

  wxFrame *root_frame;
  int initialized, nwheels, UT_shift;
  int comport_nber1, baudrate1;
  int ra_null_disp, rb_null_disp;
  int ra_position, rb_position, ra_sign, rb_sign, pisco_initialized;
  int ra_offset, rb_offset, ra_computed, rb_computed;
  bool lamp_is_on, auto_risley_on, exposure_is_on;
  JLP_wxLogbook *jlp_logbook1;

// Messages in different languages:
  wxString *m_messg;

// Nber of filters:
  int nfilters;
  wxString FilterParam[40];
  double exposure_start_time, exposure_stop_time;
  double hour_angle, elevation, air_mass, zen_dist;
  double longitude, latitude, alpha_value, delta_value, equin_value;
  double alpha_precessed, delta_precessed;
  double temp_value, hygro_value, press_value;
  int target_type;
  wxString ObjectName, Comments;
  FILE *fp_logbook;

// Wheels:
  char wheel_name[NWHEELS][3];
  int wheel_position[NWHEELS], wheel_is_there[NWHEELS];
  int actual_posi_code[NWHEELS], required_posi_code[NWHEELS];
  JLP_ComboBox PscCmb_Wheel[NWHEELS], PscCmb_AutoRisley, PscCmb_Lamp;
  wxButton *PscBut_UpdateWheels, *PscBut_StartExposure;
  wxButton *PscBut_ValidRA, *PscBut_ValidRB;
  wxTextCtrl *PscCtrl_RA, *PscCtrl_RB;
  wxStaticText *PscStatic_RA, *PscStatic_RB, *PscStatic_equinox;
  wxStaticText *PscStatic_right_ascension, *PscStatic_declination;
  wxStaticText *PscStatic_elevation, *PscStatic_hour_angle;
  wxStaticText *PscStatic_air_mass;
  wxStaticText *PscStatic_ADC_status, *PscStatic_resid_disp;
  wxStaticText *PscStatic_object_name, *PscStatic_exposure_duration;
  wxStaticText *PscStatic_sidereal_time, *PscStatic_universal_time;
  wxTimer *risley_timer, *rs232_timer;
  int risley_timer_msec, rs232_timer_msec;

// RS232
  bool rs232link_is_down;
  wxTextCtrl *PscCtrl_rs232_receivBox, *PscCtrl_rs232_messgBox;
  wxButton *PscBut_rs232_send, *PscBut_rs232_send_cr;

  DECLARE_EVENT_TABLE()
};

#endif // EOF sentry
