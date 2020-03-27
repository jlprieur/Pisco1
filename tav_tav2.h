/*********************************************************
* Include file needed for tav2.cpp
* JLP
* Version 25-09-2007
********************************************************/
#ifndef tav2_included__
#define tav2_included__

#include "tav.h"


/* Declare TTestWindow, a TFrameWindow descendant */
class TTestWindow : public TFrameWindow {
public:
    TGroupBox *GroupBox_Risley, *GroupBox_Status;
    TRadioButton *RTButtonON, *RTButtonOFF;

/* Next routine is called first (initialization function) */
    TTestWindow(TWindow * AParent, LPSTR ATitle);

    int SetupTavWindow();
    int InitRS232Link();
    int InitPISCO();
    int InitTavParameters();
    void CalibInput();
    int InputComments();
    int RefreshScreen(char *error_message, int whole_screen);

    int Automatic_Correction(char *error_message);
    void StarInput();
    void CatalogInput();
    void AtmInput();
    void LoadPosition1();
    int LoadPosition2(char *Filename);
// "logbook.cpp":
    void OpenLogBook();
    void CloseLogBook();
    void CommentsLogBook();

// "rs232win.cpp":
    int Open_RS232window();
    void Close_RS232window();


/* ADC setup: */
    void ADC_Setup();

/* Telescope position: */
    void TelescopePosition();

/* Help: */
    void VersionDisplay();

/* Delay parameter: */
    void DelayInput();

/* Exit: */
    void CloseSelection();

    virtual void SetupWindow();
    void HandleButton_Lamp();

/* Risley positions: */
    void HandleButton_RA();
    void HandleButton_RB();
/* Update positions: */
    void HandleButton_UpdateRisley();
    int UpdateWheelsFromRS232Link();
    void HandleButton_UpdateWheels();
/* Risley interactive/automatic correction: */
    void HandleRButton_AutoRisley();
    void HandleRButton_InteracRisley();
/* Start/End exposure: */
    void HandleButton_Exposure();
/* Combo boxes: */
    void HandleBoxMsg_wheel0();
    void HandleBoxMsg_wheel1();
    void HandleBoxMsg_wheel2();
    void HandleBoxMsg_wheel3();
    void HandleBoxMsg_wheel4();
    void HandleBoxMsg_wheel5();
    void HandleBoxMsg_wheel6();
    void HandleBoxMsg_wheel7();
    void HandleBoxMsg_wheel8();
    virtual void HandleBoxMsg_wheel();
// RS232 Window:
    void RS232_Dialog();

/* To allow for resizing the main window: */
    void EvSize( UINT sizeType, TSize& size)
      {Invalidate(); TFrameWindow::EvSize(sizeType, size);}

/* To check if main window can be closed: */
    virtual bool CanClose();
    int bad_initialization;

// Formely outside TTestWindow:
    int ComputeCorrection(char *error_message, double *cross_angle);
    int UpdateConfig(char *error_message);
    int Update_From_RS232();
    int Update_From_RS232_Decode(char *c1, int c1_len, int *ra_is_found,
                                 int *rb_is_found, int *nwheels_found);
    int Wheel_Status();
    int LoadLabels1(char *error_message);
    int UpdateStarFromStatusFile(char *TAVSTAT_fname, char *error_message);
    int UpdateAtmFromStatusFile(char *TAVSTAT_fname, char *error_message);

// logbook.cpp:
    int UpdateWheelsFromStatusFile(char *error_message);
    int SaveStatus(char *error_message);
    int FilterIndex(void);

// Accessors:
    int Exposure_on(){return(exposure_on);};

private:
 TComboBox *Combo_wheel[NWHEELS];
 TStatic *RAStatus, *RBStatus, *WhiteLight;
 TStatic *Clock1, *ADC_Text1, *Target1, *Airmass1, *HourAngle1;
 TStatic *RightAsc1, *Delta1, *Filter1, *Elevation1, *SiderTime1;
 TEdit *EditRA, *EditRB;
 TButton *LampButton;
 TStatic *ExpoTime1;
 TButton *ExposureButton;

 int ra_offset, rb_offset, ra_rb_sign;
 char ADC_text1[80];

 TStruct_StarDlg Struct_StarDlg1;
 TStruct_AtmDlg Struct_AtmDlg1;

 /* Wheels: */
  char wheel_title[NWHEELS][80];
  int wheel_position[NWHEELS];
  int actual_posi[NWHEELS], required_posi[NWHEELS];
  char position_file[80], WhiteLightText[80];
  int wheel_is_there[NWHEELS], ra_is_there, rb_is_there;

/* Flag set to one if astronomical target: */
  int astro_target;
/* Location parameters: longitude in hours, latitude in degrees */
  double longitude, latitude;
/* Atmosphere parameters: */
  double press_value, temp_value, hygro_value;
/* Risley prisms: */
  int ra_position, rb_position;
  int ra_computed, rb_computed;
/* Lamp status: */
  int lamp_is_on;
/* Comments: */
  char Comments[COMMENTSLEN+1];
/* Miscellaneous: */
  double hour_angle, air_mass, elevation;

/* Star parameters: */
  char StarName[MAXSTARNAMELEN+1];
// alpha_value in hours, delta_value in degrees
  double alpha_value, delta_value, equin_value;
  char catalog_name[124];
  int target_is_astro, catalog_input;

/* Filter parameters: */
  int nfilters;
  char FilterParam[NFILTER_MAX][MAXFILTERLEN];

/* Automatic Risley correction: */
  int auto_risley_on;
/* Current status of exposure: */
  int exposure_on;
/* Exposure starting time: */
  double start_time;
/* Internal index for clock output and update of parameters... */
  long idle_index;
/* Logbook parameters: */
  char logbook_name[80];
  FILE *fp_logbook;
/* Command status (RS232 commands) */
  TStatic *CommandStatus;
  int delay_msec;
/* To know if terminal window has been created or not... */
  int pisco_initialized, rs232win_not_created;
  int rs232link_is_down, no_rs232link_wanted;
  TRS232ChildWindow  *RS232w1;
DECLARE_RESPONSE_TABLE( TTestWindow );
};

// Main window:
  TTestWindow *TTwin;
#endif // EOF sentry
