/****************************************************************************
* Name: psc1_pisco_panel.cpp
* Psc1_PiscoPanel class
*
* JLP
* Version 11/01/2016
****************************************************************************/
#include "psc1_frame_id.h"
#include "psc1_pisco_panel.h"
#include "psc1_rs232_mutex.h"
#include "jlp_rs232_thread.h"  // JLP_Rs232Thread()
#include "tav_utils.h"         // input_location();
#include "jlp_rs232_in_c.h"    // RS232 link routines

#ifndef PI
#define PI 3.14159
#endif

// #define DEBUG

// ============================================================================
// implementation
// ============================================================================

/*******************************************************************************
* Constructor from wxFrame:
*******************************************************************************/
Psc1_PiscoPanel::Psc1_PiscoPanel(wxFrame *panel_frame, JLP_wxLogbook *jlp_logbook0,
                                 FILE *fp_logbook0, wxString *str_messg,
                                 const int n_messg)
                  : wxPanel( panel_frame )
{
wxString input_filename;
int status, k, iloc;

 initialized = 0;
 fp_logbook = fp_logbook0;
 pisco_initialized = 0;
 nwheels = 9;
 nfilters = 0;
 target_type = 0;
 exposure_start_time = 0.;
 exposure_stop_time = 0.;
 lamp_is_on = false;
 auto_risley_on = true;
 exposure_is_on = false;
 alpha_value = 0.;
 delta_value = 75.;
 equin_value = 0.;
 temp_value = 10.;
 press_value = 1013.;
 hygro_value = 50;
 UT_shift = -2.;
 ObjectName = wxT("Test");
 Comments = wxT("");
 rs232link_is_down = true;

// Risley prisms:
/* OLD VERSION (MERATE)
 ra_offset = 511;
 rb_offset = 156;
 ra_sign = 1;
 rb_sign = 1;
// Null dispersion (57 214:
 ra_position = (ra_offset - 512) + 58;
 rb_position = 156 + 58;
 */
// CALERN: Red to North with RA=960 RB=568 ? NO !
// CALERN: Red to North with RA=448 RB=56 : no (but close)
// CALERN: Red to North with RA=520 RB=130 : seems good
 ra_offset = 520;
 rb_offset = 130;
 ra_sign = 1;
 rb_sign = 1;

// Null dispersion (CALRN: RA=0 RB=120):
// Compute ra_null_disp and rb_null_disp:
 ra_null_disp = 0;
 rb_null_disp = rb_offset - ra_offset - 512;
 if(rb_null_disp < 0) rb_null_disp += 1024;
 if(rb_null_disp > 1023) rb_null_disp -= 1024;

 ra_position = ra_null_disp;
 rb_position = rb_null_disp;

 // Assume all wheels are connected (useful for debug mode)
 for(k = 0; k < nwheels; k++) {
  wheel_is_there[k] = 1;
  wheel_position[k] = 1;
  }
 rs232_timer = NULL;
 risley_timer = NULL;

// Location :
// 8= Calern
 iloc = 8;
 input_location(&latitude, &longitude, iloc);

// Messages (in different languages)
 m_messg = new wxString[n_messg];
 for(k = 0; k < n_messg; k++) m_messg[k] = str_messg[k];

// Transform coma into point for numbers:
 setlocale(LC_NUMERIC, "C");

// Background colour:
 SetBackgroundColour(*wxLIGHT_GREY);

// Smaller fonts (needed for Windows, to have a similar appearance as Linux)
// SetWindowVariant(wxWINDOW_VARIANT_SMALL);
 jlp_logbook1 = jlp_logbook0;

// Create all widgets of PiscoPanel:
 PiscoPanel_Setup();

// Create RS232 Mutex:
 Create_RS232Mutex();
// Set RS232_is_busy = false
 Write_RS232Mutex(false);

 initialized = 1234;

// Prompt the user for RS232 settings
// Create RS232 thread and open connection:
 comport_nber1 = -1;
 status = OpenRS232LinkAndStartPisco();
// Do not call Close() since no window opened at this stage:
 if(status) exit(3);

// Start timers:
 StartRisleyTimer();
 StartRs232Timer();

// Update display (after setting initialized to 1234) :
 DisplayNewValues();

// Check positions from RS232 link:
 CheckPositions();

return;
}
/*******************************************************************************
* Destructor
*******************************************************************************/
Psc1_PiscoPanel::~Psc1_PiscoPanel()
{
}
/*******************************************************************************
* Close
*******************************************************************************/
bool Psc1_PiscoPanel::Close()
{

if(initialized == 1234) MyShutdown();

return(true);
}
/*******************************************************************************
* Shutdown
*******************************************************************************/
void Psc1_PiscoPanel::MyShutdown()
{

if(initialized != 1234) return;

// Stop Risley timer:
 StopRisleyTimer();

// Stop RS232 timer:
 StopRs232Timer();

// Close Port:
 if(comport_nber1 >= 0) RS232_CloseComport(comport_nber1);

// Delete RS232 Mutex:
 Delete_RS232Mutex();

initialized = 0;

return;
}
/*******************************************************************************
* Update display
*******************************************************************************/
void Psc1_PiscoPanel::DisplayNewValues()
{
int ih0, ih1;
double h2;
wxString buffer;
double yyear, utime1, delta_years;
int mmonth, dday;
char cbuffer[80];

  if(initialized != 1234) return;

if(lamp_is_on) PscCmb_Lamp.combo->SetSelection(1);
 else PscCmb_Lamp.combo->SetSelection(0);

if(auto_risley_on) PscCmb_AutoRisley.combo->SetSelection(1);
 else PscCmb_AutoRisley.combo->SetSelection(0);

// Target name:
  PscStatic_object_name->SetLabel(ObjectName);

// Right ascension
  convert_coord(alpha_value,&ih0,&ih1,&h2,"H");
  buffer.Printf(wxT("%02d h %02d m %02d s"), ih0, ih1, (int)h2);
  PscStatic_right_ascension->SetLabel(buffer);

// Declination
  convert_coord(delta_value,&ih0,&ih1,&h2,"D");
// Case of -0 30 54:
  if(ih0 == 0 && delta_value < 0.)
     buffer.Printf(wxT("-00 d %02d ' %02d ''"), ih1, (int)h2);
   else
     buffer.Printf(wxT("%02d d %02d ' %02d ''"), ih0, ih1, (int)h2);
  PscStatic_declination->SetLabel(buffer);

// Equinox
  buffer.Printf(wxT("%.1f"), equin_value);
  PscStatic_equinox->SetLabel(buffer);

// Compute precession correction if needed:
  alpha_precessed = alpha_value;
  delta_precessed = delta_value;
  if(equin_value != 0.) {
// Here I only use the date
      jlp_utime(cbuffer, &utime1, &yyear, &mmonth, &dday, UT_shift);
      delta_years = yyear + ((double)mmonth)/12. - equin_value;
      precess(&alpha_precessed, &delta_precessed, delta_years);
      }

}
/********************************************************************
* Pisco Setup panel
********************************************************************/
int Psc1_PiscoPanel::PiscoPanel_Setup()
{
wxBoxSizer *w_topsizer, *w_hsizer1;
wxBoxSizer *pisco_left_sizer, *pisco_center_sizer, *pisco_right_sizer;

  w_topsizer = new wxBoxSizer(wxVERTICAL);

  w_hsizer1 = new wxBoxSizer( wxHORIZONTAL );

  pisco_left_sizer = new wxBoxSizer(wxVERTICAL);
  PiscoPanel_LeftSetup(pisco_left_sizer);
  w_hsizer1->Add(pisco_left_sizer, 0, wxTOP | wxLEFT, 10);

  pisco_center_sizer = new wxBoxSizer(wxVERTICAL);
  PiscoPanel_CenterSetup(pisco_center_sizer);
  w_hsizer1->Add(pisco_center_sizer, 0, wxTOP | wxLEFT, 10);

  pisco_right_sizer = new wxBoxSizer(wxVERTICAL);
  PiscoPanel_RightSetup(pisco_right_sizer);
  w_hsizer1->Add(pisco_right_sizer, 0, wxTOP | wxLEFT | wxRIGHT, 10);

  w_topsizer->Add(w_hsizer1, 0, wxALIGN_CENTER);

  this->SetSizer(w_topsizer);

  Centre();

return(0);
}
/********************************************************************
* Pisco Setup panel (left side)
********************************************************************/
void Psc1_PiscoPanel::PiscoPanel_LeftSetup(wxBoxSizer *pisco_left_sizer)
{
wxStaticBoxSizer *wheel_sizer;
int k, irows, icols, vgap = 8, hgap = 12, wwidth = 200, hheight = 26;
wxFlexGridSizer *fgs1;

// *************** Wheels: ***************************************
LoadWheelChoices_from_file(TAVCONFI);

// Sizer surrounded with a rectangle, with a title on top:
// i=200 "Wheel positions"
 wheel_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[200]);
 irows = 2 * nwheels + 1;
 icols = 1;
 fgs1 = new wxFlexGridSizer(irows, icols, vgap, hgap);

// Image format:
 for(k = 0; k < nwheels; k++) {
 PscCmb_Wheel[k].combo = new wxComboBox(this, (ID_PISCO_WHEEL_0 + k),
                                        wxT(""), wxDefaultPosition,
                                        wxSize(wwidth, hheight),
                                        PscCmb_Wheel[k].nchoices,
                                        PscCmb_Wheel[k].choices);
 fgs1->Add(new wxStaticText(this, -1, PscCmb_Wheel[k].label));
 fgs1->Add(PscCmb_Wheel[k].combo);
 }

 wheel_sizer->Add(fgs1, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
// pisco_left_sizer->Add(wheel_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 10);
 pisco_left_sizer->Add(wheel_sizer, 0);

return;
}
/********************************************************************
* Pisco Setup panel (central side)
********************************************************************/
void Psc1_PiscoPanel::PiscoPanel_CenterSetup(wxBoxSizer *pisco_center_sizer)
{
wxStaticBoxSizer *risley_sizer;
wxBoxSizer *hsizer0, *hsizer1;
int irows, icols, vgap = 12, hgap = 12;
wxFlexGridSizer *fgs1;


// Sizer surrounded with a rectangle, with a title on top:
// i=204 "Risley prisms"
 risley_sizer = new wxStaticBoxSizer(wxVERTICAL, this,
                                     m_messg[204]);
 irows = 12;
 icols = 3;
 fgs1 = new wxFlexGridSizer(irows, icols, vgap, hgap);

// "RA: 0000"
 PscStatic_RA = new wxStaticText(this, wxID_ANY, wxT("RA: 0000"));
 fgs1->Add(PscStatic_RA, 0, wxALIGN_CENTER_VERTICAL);
 PscCtrl_RA = new wxTextCtrl(this, wxID_ANY, wxT("512"));
 fgs1->Add(PscCtrl_RA, 0, wxALIGN_CENTER_VERTICAL);
// i=205 "Validate"
 PscBut_ValidRA = new wxButton(this, ID_PISCO_RA_VALID, m_messg[205]);
 fgs1->Add(PscBut_ValidRA);

// "RB: 0000"
 PscStatic_RB = new wxStaticText(this, wxID_ANY, wxT("RB: 0000"));
 fgs1->Add(PscStatic_RB, 0, wxALIGN_CENTER_VERTICAL);
 PscCtrl_RB = new wxTextCtrl(this, wxID_ANY, wxT("1012"));
 fgs1->Add(PscCtrl_RB, 0, wxALIGN_CENTER_VERTICAL);
// i=205 "Valid"
 PscBut_ValidRB = new wxButton(this, ID_PISCO_RB_VALID, m_messg[205]);
 fgs1->Add(PscBut_ValidRB);

 risley_sizer->Add(fgs1, 0, wxALL, 10);

// i=206 "Manual correction"
// i=207 "Automatic correction"
 PscCmb_AutoRisley.label = wxT("");
 PscCmb_AutoRisley.nchoices = 2;
 PscCmb_AutoRisley.choices[0] = m_messg[206];
 PscCmb_AutoRisley.choices[1] = m_messg[207];
 PscCmb_AutoRisley.combo = new wxComboBox(this, ID_PISCO_AUTO_RISLEY, wxT(""),
                                   wxDefaultPosition, wxSize(250, 28),
                                   PscCmb_AutoRisley.nchoices, PscCmb_AutoRisley.choices);
 hsizer1 = new wxBoxSizer(wxHORIZONTAL);
 hsizer1->Add(PscCmb_AutoRisley.combo);
 risley_sizer->Add(hsizer1, 0, wxBOTTOM | wxALIGN_CENTER_HORIZONTAL, 10);

 pisco_center_sizer->Add(risley_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 10);

// RS232 window:
 RS232_window_setup(pisco_center_sizer);

// i=202 "Lamp off"
// i=203 "Lamp on"
 PscCmb_Lamp.label = wxT("");
 PscCmb_Lamp.nchoices = 2;
 PscCmb_Lamp.choices[0] = m_messg[202];
 PscCmb_Lamp.choices[1] = m_messg[203];
// lampara (with accents) needs height=30
 PscCmb_Lamp.combo = new wxComboBox(this, ID_PISCO_LAMP_ON, wxT(""),
                                   wxDefaultPosition, wxSize(240, 30),
                                   PscCmb_Lamp.nchoices, PscCmb_Lamp.choices);
 hsizer0 = new wxBoxSizer(wxHORIZONTAL);
 hsizer0->Add(PscCmb_Lamp.combo);
 pisco_center_sizer->Add(hsizer0, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 20);

return;
}
/********************************************************************
* Pisco Setup panel (right side)
********************************************************************/
void Psc1_PiscoPanel::PiscoPanel_RightSetup(wxBoxSizer *pisco_right_sizer)
{
wxStaticBoxSizer *expo_sizer;
wxBoxSizer *hsizer0, *hsizer1;
int irows, icols, vgap, hgap, wwidth = 200;
wxFlexGridSizer *fgs1;

// i=208 "Start Exposure"
// i=209 "Stop Exposure"
 hsizer0 = new wxBoxSizer(wxHORIZONTAL);
 PscBut_StartExposure = new wxButton(this, ID_PISCO_EXPO_START, m_messg[208],
                                     wxDefaultPosition, wxSize(300,28));
 hsizer0->Add(PscBut_StartExposure);
 pisco_right_sizer->Add(hsizer0, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 30);

// Sizer surrounded with a rectangle, with a title on top:
// i=220 "Exposure parameters"
 expo_sizer = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[220]);
 irows = 16;
 icols = 2;
 vgap = 12;
 hgap = 10;
 fgs1 = new wxFlexGridSizer(irows, icols, vgap, hgap);
// i=221 "Name"
 PscStatic_object_name = new wxStaticText(this, wxID_ANY, wxT("ADS 12377"),
// Should be large enough to be able to display ADC messages ... :
                                          wxDefaultPosition, wxSize(250,28));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[221]));
 fgs1->Add(PscStatic_object_name);
// i=229 "Exposure duration"
 PscStatic_exposure_duration  = new wxStaticText(this, wxID_ANY, wxT("2305"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[229]));
 fgs1->Add(PscStatic_exposure_duration);
// i=223 "Right ascension"
 PscStatic_right_ascension = new wxStaticText(this, wxID_ANY, wxT("02 34 55.6"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[223]));
 fgs1->Add(PscStatic_right_ascension);
// i=222 "Declination"
 PscStatic_declination = new wxStaticText(this, wxID_ANY, wxT("-05 45 34"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[222]));
 fgs1->Add(PscStatic_declination);
// i=232 "Equinox"
 PscStatic_equinox = new wxStaticText(this, wxID_ANY, wxT("2000"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[232]));
 fgs1->Add(PscStatic_equinox);
// i=224 "Elevation"
 PscStatic_elevation  = new wxStaticText(this, wxID_ANY, wxT("+30.4"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[224]));
 fgs1->Add(PscStatic_elevation);
// i=226 "Hour angle"
 PscStatic_hour_angle = new wxStaticText(this, wxID_ANY, wxT("01 23 45.5"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[226]));
 fgs1->Add(PscStatic_hour_angle);
// i=225 "Air mass"
 PscStatic_air_mass  = new wxStaticText(this, wxID_ANY, wxT("1.235"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[225]));
 fgs1->Add(PscStatic_air_mass);
// i=227 "Sidereal time"
 PscStatic_sidereal_time = new wxStaticText(this, wxID_ANY, wxT("23 34 55"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[227]));
 fgs1->Add(PscStatic_sidereal_time);
// i=228 "Universal time"
 PscStatic_universal_time = new wxStaticText(this, wxID_ANY, wxT("21 23 43"));
 fgs1->Add(new wxStaticText(this, wxID_ANY, m_messg[228]));
 fgs1->Add(PscStatic_universal_time);

 expo_sizer->Add(fgs1, 0, wxALL, 10);

// ADC_status (for warning messages if object under the horizon, for instance)
 PscStatic_ADC_status = new wxStaticText(this, wxID_ANY, wxT(" "));
 expo_sizer->Add(PscStatic_ADC_status, 0, wxLEFT | wxTOP, 10);

// resid_disp (residual dispersion)
 PscStatic_resid_disp = new wxStaticText(this, wxID_ANY, wxT(" "));
 expo_sizer->Add(PscStatic_resid_disp, 0, wxALL, 10);

 pisco_right_sizer->Add(expo_sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 10);

// Create new button
// i=201 "Update status"
 hsizer1 = new wxBoxSizer(wxHORIZONTAL);
 PscBut_UpdateWheels = new wxButton(this, ID_PISCO_CHECK_POSI, m_messg[201]);
 hsizer1->Add(PscBut_UpdateWheels);
 pisco_right_sizer->Add(hsizer1, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 20);

return;
}
/********************************************************************
* Initialization of combo boxes with config file
* to load the labels of the PISCO wheels
* by reading the file TAVCONF="tavconfi.dat"
* Example:
*
AS Lampes calib.
1: Halogene
2: Filament
3: Argon
4: Mercure
5: Neon
6: Xenon
EN Roue d'entree
1: vide
2: vide
3: risque d'oc.
4: miroir calib
5: risque d'oc.
6: vide
CH Roue de champ
1: Fente
2: vide
3: mire
4: coronographe
5: vide
6: lent. de champ
MA Roue a masques
1: 4 trous
2: trou 7 mm
3: vide
4: vide
5: trous alignes
6: trous en rond
DA Roue a densites
1: vide
2: densite 1
3: densite 2
4: densite 3
5: densite 4
6: vide
DB Roue a densites
1: vide
2: densite 0.2
3: densite 0.4
4: densite 0.6
5: densite 0.8
6: vide
FA Roue a filtres
1: R 650/70
2: Grisme
3: V 550/70
4: vide
FB Roue a filtres
1: vide
2: IR 812/15
3: vide
4: BV 489/15
5: vide
6: R 614/15
GR Roue de grandisst
1: Pupille
2: Spectro
3: 4.5 mm
4: 20 mm
5: 50 mm
6: 10 mm
## Filtres:
#FA 1 650.00 70.0
#FA 3 550.00 70.0
#FB 2 812.00 15.0
#FB 4 489.00 15.0
#FB 6 614.00 15.0
*
********************************************************************/
int Psc1_PiscoPanel::LoadWheelChoices_from_file(char* tavconfig_fname)
{
FILE *fp;
char buffer[80];
register int i, k;
wxString err_msg;

  if((fp = fopen(tavconfig_fname,"r")) == NULL)
    {
     err_msg.Printf(wxT("Error: tavconfig file %s not found"), tavconfig_fname);
     wxMessageBox(err_msg, wxT("LoadWheelChoices"), wxOK | wxICON_ERROR);
     return(-1);
    }

  for(k = 0; k < nwheels; k++)
   {
    if(fgets(buffer,80,fp) == NULL)
    {
     err_msg.Printf(wxT("Error reading tavconfig file %s"), tavconfig_fname);
     wxMessageBox(err_msg, wxT("LoadWheelChoices"), wxOK | wxICON_ERROR);
     fclose(fp); return (-1);
    }
// Wheel name, e.g., EN, DA, GR, etc
    wheel_name[k][0] = buffer[0];
    wheel_name[k][1] = buffer[1];
    wheel_name[k][2] = '\0';
// Wheel label, e.g. "EN Roue d'entree"
    PscCmb_Wheel[k].label = wxString(buffer);
    PscCmb_Wheel[k].label.Trim();
    if(!strncmp(buffer,"FA",2))
      PscCmb_Wheel[k].nchoices = 4;
    else
      PscCmb_Wheel[k].nchoices = 6;
    for (i = 0; i < PscCmb_Wheel[k].nchoices; i++) {
        fgets(buffer,80,fp);
        PscCmb_Wheel[k].choices[i] = wxString(buffer);
        (PscCmb_Wheel[k].choices[i]).Trim();
      }
   }
fclose(fp);
return(0);
}
/**********************************************************************
* Initialize PISCO: boot and load nominal hexadecimal wheel values
**********************************************************************/
int Psc1_PiscoPanel::InitPisco()
{
wxString buffer, answer1, error_message1;
int n_wanted, n_received, status, i;
char *pc, answer[1024], command1[1024];


// Initialization of the microprocessor of the speckle camera:
 status = BootPisco();
 if(status == 0)
    rs232link_is_down = false;
  else
    rs232link_is_down = true;

 if(rs232link_is_down == true) {
// i=260 "The microprocessor is offline. Do you want to exit from this program?");
      buffer = wxT(">") + answer1 + wxT("\n");
      buffer.Append(m_messg[260]);
      buffer.Append(m_messg[261]);
// Do not call Close() since no window opened at this stage:
      if(wxMessageBox(buffer, m_messg[262], wxYES_NO | wxICON_ERROR) == wxYES)
        exit(4);
    }

// Init Risley prism position to null dispersion
// and update their position in Pisco control panel
  Init_RisleyPrisms();

// Filters:
  FiltersFromConfigFile(TAVCONFI);

// Load good wheel encoding positions from file TAVPOSI:
  status = LoadWheelPositionCodes(TAVPOSI);
  if(!status) pisco_initialized = 1;

return(status);
}
/**********************************************************************
*
**********************************************************************/
bool Psc1_PiscoPanel::PiscoIsAlive()
{
wxString buffer, answer1, error_message1;
int n_wanted, n_received, status, i;
char *pc, answer[1024], command1[1024];
bool is_alive = false;

// Check if link is OK: to get a prompt "*", we need to send a blank (doesn't need a CReturn)
  strcpy(command1," ");
// Try at most 3 times:
  for (i = 0; i < 3; i++) {
// set n_wanted to 8 characters, in order to display a few lines (the prompt
// is on the 5th line)
     n_wanted = 8;
     RS232_SendCommand2(command1, n_wanted, answer1, &n_received, error_message1);
/* DEBUG
     buffer.Printf(wxT("n_wanted=%d n_received=%d >%s<"),
                   n_wanted, n_received, (const char *)answer1);
     wxMessageBox(buffer, wxT("Answer from blank"), wxOK);
*/
// DDEBUG: wait for 32 seconds (Risley prisms are rotating)
//     Sleep(32000);

     if(n_received >= 0) {
// Look for "*" in the answer, exit from loop if "*" is found:
// May be also 'G0000'
      strncpy(answer, (const char *)answer1.mb_str(), n_received);
      pc = answer;
      answer[8] = '\0';
      while(*pc && *pc != '*' && *pc != '?' && *pc != 'G' && *pc != 'T') pc++;
// Exit from loop if "*" was received:
        if(*pc == '*' || *pc == '?' || *pc == 'G' || *pc == 'T') {
          is_alive = true;
          break;
          }
      }
  } // EOF loop on i

return(is_alive);
}
/**********************************************************************
*
**********************************************************************/
int Psc1_PiscoPanel::BootPisco()
{
wxString buffer, answer1, error_message1;
int n_wanted, n_received, status = -1, i;
char *pc, answer[1024], command1[1024];

// To get a prompt "*", we need to send a blank (doesn't need a CReturn)
// Without \r
  strcpy(command1, " ");
  for(i = 0; i < 4; i++) {
   RS232_SendCommand(command1);
   }

// Initialization of the microprocessor of the speckle camera:
// WITHOUT \r AT THE END !!!!!
  strcpy(command1, "G0000");
  n_wanted = 512;
  RS232_SendCommand2(command1, n_wanted, answer1, &n_received, error_message1);

// i=263  "PISCO is starting up ('boot')"
// i=264 "Enter OK when the Risley prisms have finished rotating
  buffer = m_messg[263] + wxT("\n") + m_messg[264];
  if(wxMessageBox(buffer, wxT("BootPisco"), wxOK) == wxOK)
     status = 0;
  else
     status = -1;

/* OLD VERSION:
//  RS232_SendCommand(command1);
// Wait for 32000 milliseconds, since Risley prisms are rotating:
// Tested in november 2015: 28 seconds needed for rotation
  Sleep(32000);
// Answer may be also "G 000  TAVELOGRAPHE READY" if PISCO has been previously booted
  if((n_received > 1) || (answer1[0] == 'G') || (answer1[1] == 'G')) {
     status = 0;
  } else {
// Check if PiscoIsAlive by sending " " commands:
   if(PiscoIsAlive()) status = 0;
  }
*/

// NEW VERSION: end a G0000 again to solve problems when PISCO is already running:
// NOT WORKING !!!
//  strcpy(command1, "G0000");
//  RS232_SendCommand(command1);

return(status);
}
/*******************************************************************
* Load wheel positions codes from file and initialize PISCO with
* those positions
*
*******************************************************************/
int Psc1_PiscoPanel::LoadWheelPositionCodes(char *posi_fname)
{
FILE *fp;
wxString error_message1;
char buffer[80], command1[128];
int status;

#ifdef DEBUG
  error_message1.Printf(wxT("Opening file %s"), posi_fname);
  wxMessageBox(error_message1, wxT("Loading wheel position codes"), wxOK);
#endif

// Open position file:
if((fp = fopen(posi_fname,"r")) == NULL) {
  error_message1.Printf(wxT("Error opening file %s"), posi_fname);
  wxMessageBox(error_message1, wxT("Loading wheel position codes"),
               wxOK | wxICON_ERROR);
  return(-1);
  }

/***************************************************************
Process lines with a star "*" at the 11th position:
Example:

EN 1 0065 *
EN 2 0265
EN 3 0265
***********************************************************/
buffer[10] = '\0';
while(fgets(buffer,20,fp) != NULL) {
  if(buffer[10] == '*') {
    buffer[9] = '\0';
// Send command to Speckle camera:
// Command should be like:   "*CH 1 0065\r"
    sprintf(command1, "*%s\r", buffer);
    RS232_SendCommand(command1);
  }  // EOF case buffer[10] == *
  buffer[10] = '\0';
/* End of while loop... */
}
fclose(fp);

return(0);
}
