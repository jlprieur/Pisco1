/****************************************************************************
* Name: psc1_pisco_risley.cpp
* Psc1_PiscoPanel class
*
* JLP
* Version 01/12/2015
****************************************************************************/
#include "psc1_frame_id.h"  // ID_PISCO_RISLEY_TIMER
#include "psc1_typedef.h"   // ABS
#include "psc1_pisco_panel.h"
#include "jlp_rs232_thread.h"
#include "tav_utils.h"

/****************************************************************
* Start_Risley timer
****************************************************************/
void Psc1_PiscoPanel::StartRisleyTimer()
{

// Set delay value to update Risley position 1 time/second:
 risley_timer_msec = 1000;

// Timer for updating the Risley position:
 risley_timer = new wxTimer(this, ID_PISCO_RISLEY_TIMER);

// Start timer with time interval in milliseconds
 risley_timer->Start(risley_timer_msec);

 return;
}
/****************************************************************
* Stop_Risley timer
****************************************************************/
void Psc1_PiscoPanel::StopRisleyTimer()
{
if(initialized != 1234) return;

if(risley_timer) risley_timer->Stop();

 return;
}
/****************************************************************
* Init Risley prism position to null dispersion
* and update their position in Pisco control panel
*
*****************************************************************/
void Psc1_PiscoPanel::Init_RisleyPrisms()
{
wxString buffer, answer1, error_message1;
char command1[128];
int n_wanted, n_received;

// Compute ra_null_disp and rb_null_disp:
// Null dispersion (CALERN: RA=0 RB=120):
 ra_null_disp = 0;
 rb_null_disp = rb_offset - ra_offset - 512;
 if(rb_null_disp < 0) rb_null_disp += 1024;
 if(rb_null_disp > 1023) rb_null_disp -= 1024;

// Set Risley prisms RA and RB to zero dispersion:
 buffer.Printf(wxT("%04d"), ra_null_disp);
 sprintf(command1, "RA %s\r", (const char *)buffer.mb_str());
 PscCtrl_RA->SetValue(buffer);

// Send command to PISCO:
// ("RA 0048" or "RB 0543" for example)
 n_wanted = 7;
 RS232_SendCommand2(command1, n_wanted, answer1, &n_received, error_message1);

// Set Risley prisms RB:
 buffer.Printf(wxT("%04d"), rb_null_disp);
 sprintf(command1, "RB %s\r", (const char *)buffer.mb_str());
 PscCtrl_RB->SetValue(buffer);

// Send command to PISCO:
// ("RA 0048" or "RB 0543" for example)
 n_wanted = 7;
 RS232_SendCommand2(command1, n_wanted, answer1, &n_received, error_message1);

// Temporary: assume Risley prisms work properly...
 ra_position = ra_null_disp;
 rb_position = rb_null_disp;

return;
}
/****************************************************************
* Update Risley nominal position
* and check if their position needs to be modified
*****************************************************************/
void Psc1_PiscoPanel::OnRisleyTimer(wxTimerEvent& event)
{
wxString buffer, error_message;
char cbuffer[80];
int ih0, ih1, status;
int month, day;
double cross_angle, utime1, azimuth;
double h2, sider_time, year;
double ww_zen_dist_rad;

if(initialized != 1234) return;

// New value of universal time :
  if(jlp_utime(cbuffer, &utime1, &year, &month, &day, UT_shift))
    buffer = wxT("U.T: error");
  else
    buffer = wxString(cbuffer);
  PscStatic_universal_time->SetLabel(buffer);

// Sidereal time
  if(jlp_lsidtime(cbuffer, &sider_time, longitude, UT_shift))
    buffer = wxT("LST/Error.");
  else
    buffer = wxString(cbuffer);
  PscStatic_sidereal_time->SetLabel(buffer);

  if(exposure_is_on) {
   utime1 -= exposure_start_time;
// If exposure was stopped, display last time
  } else {
   utime1 = exposure_stop_time - exposure_start_time;
  }
   convert_coord(utime1, &ih0, &ih1, &h2, (const char *)"H");
   buffer.Printf(wxT("%2d h %2d m %2d s"), ih0, ih1, (int)h2);
   PscStatic_exposure_duration->SetLabel(buffer);

/* Compute hour angle, air mass, and Risley prism positions */
// Astronomical target:
  if(target_type == 0) {
// Local coordinates:
    local_coord(sider_time, latitude, alpha_value,
                delta_value, &hour_angle, &elevation, &azimuth);
    if(elevation <= 90. && elevation > 5.) {
// zen_dist is in degrees, ww_zen_dist_rad is in radians
             zen_dist = 90. - elevation;
             ww_zen_dist_rad = zen_dist * DEGTORAD;
             air_mass = 1 / cos(ww_zen_dist_rad);
       } else {
           zen_dist = 90.;
           air_mass = -1.;
       }

// First error case: object under the horizon:
  if(air_mass < 0.) {
// i=230 "The target is under the horizon"
    PscStatic_ADC_status->SetLabel(m_messg[230]);
// Elevation:
    buffer.Printf(wxT("%.1f"), elevation);
    PscStatic_elevation->SetLabel(buffer);
// i=231 " Undefined "
    PscStatic_air_mass->SetLabel(m_messg[231]);
// Hour angle:
    buffer = wxT(" ");
    PscStatic_hour_angle->SetLabel(buffer);
  } else {
// Hour angle:
   convert_coord(hour_angle,&ih0,&ih1,&h2,"H");
   buffer.Printf(wxT("%2d h %2d m %2d s"), ih0, ih1, (int)h2);
   PscStatic_hour_angle->SetLabel(buffer);
// Elevation:
   buffer.Printf(wxT("%.1f"), elevation);
   PscStatic_elevation->SetLabel(buffer);
// Air mass:
   buffer.Printf(wxT("%.2f"), air_mass);
   PscStatic_air_mass->SetLabel(buffer);
   }

  status = ComputeCorrection(&cross_angle);
  if(status == 1) {
// status = 1
// i=236 No filter, therefore no correction...
    PscStatic_ADC_status->SetLabel(m_messg[236]);
    PscStatic_resid_disp->SetLabel(wxT(""));
    }

// If limit has been reached, display warning message (in calling routine):
//  if(cross_angle == 0.) Refresh_status = 1;


/****** End of case of astro target.
****** Now start case of calibration: */
  } else {
  buffer.Printf(wxT("%04d"), ra_null_disp);
  PscCtrl_RA->SetValue(buffer);
  buffer.Printf(wxT("%04d"), rb_null_disp);
  PscCtrl_RB->SetValue(buffer);
  ra_computed = ra_null_disp;
  rb_computed = rb_null_disp;

  buffer = wxT(" ");
  PscStatic_hour_angle->SetLabel(buffer);
  PscStatic_air_mass->SetLabel(buffer);
  PscStatic_elevation->SetLabel(buffer);
  PscStatic_right_ascension->SetLabel(buffer);
  PscStatic_declination->SetLabel(buffer);
  PscStatic_equinox->SetLabel(buffer);
  PscStatic_ADC_status->SetLabel(buffer);
  PscStatic_resid_disp->SetLabel(buffer);
  }

// Fill editing boxes:
    buffer.Printf(wxT("%04d"), ra_computed);
    PscCtrl_RA->SetValue(buffer);
    buffer.Printf(wxT("%04d"), rb_computed);
    PscCtrl_RB->SetValue(buffer);

/** If automatic correction, send a command every time the difference
between current status and computed position is more than RISLEY_TOLER encoding steps:
**/
  if(auto_risley_on && exposure_is_on) ApplyCorrection(error_message);

return;
}
/***************************************************************
* ApplyCorrection
* apply correction when computed values are
* significantly different from previous ones
****************************************************************/
void Psc1_PiscoPanel::ApplyCorrection(wxString &error_message)
{
wxString buffer;
char command1[128];

// Send command to PISCO:
// ("RA 0048" or "RB 0543" for example)
if(ABS(ra_computed - ra_position) > 4) {
 sprintf(command1, "RA %04d\r", ra_computed);
 RS232_SendCommand(command1);
 ra_position = ra_computed;
 buffer.Printf(wxT("RA : %d"), ra_position);
 PscStatic_RA->SetLabel(buffer);
 }

if(ABS(rb_computed - rb_position) > 4) {
 sprintf(command1, "RB %04d\r", rb_computed);
 RS232_SendCommand(command1);
 rb_position = rb_computed;
 buffer.Printf(wxT("RB : %d"), rb_position);
 PscStatic_RB->SetLabel(buffer);
 }

return;
}
/***************************************************************
*  ComputeCorrection
* Return:
* -1 if fatal error
* 1 or 2 if minor error
****************************************************************/
int Psc1_PiscoPanel::ComputeCorrection(double *cross_angle)
{
int Compute_status, code_ra, code_rb, k;
double beta, resid_disp, lambdac, dlambda;
char cbuffer[80], ADC_text0[128];
wxString error_message, buffer;
FILE *fp1;

  Compute_status = 0;
  error_message = wxT(" ");
  ra_computed = ra_null_disp;
  rb_computed = rb_null_disp;

/************** RA and RB positions: *************************/
/* If object under the horizon, move the prisms
   to the null dispersion position: */
if(air_mass < 0.)
  {
// Object under the horizon: no correction!
    PscStatic_ADC_status->SetLabel(m_messg[234]);
/* Case air_mass > 0, hence ra and rb positions can be computed */
  } else {
/* Filter parameters:
Example:
#FA 2 550.00 72.0, but only "FA 2 550.00 72.0" in FilterParam...
*/
  k = FilterIndex();
  if(k != -1) {
    strcpy(cbuffer, (const char *)FilterParam[k].mb_str());
    sscanf(&cbuffer[5], "%lf %lf", &lambdac, &dlambda);

    if((fp1 = fopen(LOGFILE,"w")) == NULL) {
// i=235 "Error opening file"
      buffer.Printf(wxT("%s"), LOGFILE);
      error_message = m_messg[235] + buffer;
      wxMessageBox(error_message, wxT("ComputeCorrection"), wxOK | wxICON_ERROR);
      return(2);
     } else {
        fprintf(fp1," Latitude=%f longitude=%f \n",latitude,longitude);
        fprintf(fp1,
            " zd=%.2f deg, lambda_cent=%.2f nm, delta_lambda=%.2f nm\n",
            zen_dist, lambdac, dlambda);
        fprintf(fp1," Hour_angle=%f zd=%f \n", hour_angle, zen_dist);
       }

//  beta: angle between the lines linking the star to the pole
//         and the star to the zenith.
      Compute_status = risley_posi(fp1, latitude, hour_angle, zen_dist,
           lambdac, dlambda, &beta, cross_angle, &resid_disp,
           temp_value, hygro_value, press_value, ra_offset, rb_offset,
           ra_sign, rb_sign, &code_ra, &code_rb, ADC_text0);
    if(Compute_status == 0) {
      error_message.Printf(wxT("beta=%.2f cross_angle=%.1f ra=%d rb=%d"),
      beta, *cross_angle, code_ra, code_rb);
      PscStatic_ADC_status->SetLabel(error_message);
      buffer.Printf(wxT("resid_disp=%.4f"), resid_disp);
      PscStatic_resid_disp->SetLabel(buffer);
     } else {
      error_message = wxString(ADC_text0);
      PscStatic_ADC_status->SetLabel(error_message);
     }

    if(fp1 != NULL) fclose(fp1);
    }
/* Case when no filter has been selected */
  else
    {
// status = 1
// i=236 No filter, therefore no correction...
    Compute_status = 1;
    }

// If satisfactory, move the prisms to the computed position:
  if(!Compute_status && code_ra > 0) {
    ra_computed = code_ra;
    rb_computed = code_rb;
// Else move the prisms to the null dispersion position:
  } else {
    ra_computed = ra_null_disp;
    rb_computed = rb_null_disp;
  }

} /* end of case air_mass > 0 */

return(Compute_status);
}
