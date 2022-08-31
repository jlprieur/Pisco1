/****************************************************************************
* Name: psc1_pisco_logbook.cpp
* Psc1_PiscoPanel class
*
* JLP
* Version 14/01/2016
****************************************************************************/
#include "psc1_frame_id.h"
#include "psc1_pisco_panel.h"
#include "tav_utils.h"     // utime()
/*******************************************************************
* Add comments to the logbook
********************************************************************/
void Psc1_PiscoPanel::AddCommentsToLogbook()
{
wxString comments0;

 if(initialized != 1234) return;

 GetCommentsForTheLogbook(comments0);
 if(!comments0.IsEmpty()) {
   comments0.Append(wxT("\n"));
   WriteToLogbook(comments0, true);
   }

return;
}
/*******************************************************************
* Get comments for the logbook
********************************************************************/
int Psc1_PiscoPanel::GetCommentsForTheLogbook(wxString& comments0)
{
int status = -1;
wxString str1;

comments0 = wxT("");

if((initialized != 1234) || ((jlp_logbook1 == NULL) && (fp_logbook == NULL)))
                             return(-2);

// To avoid conflicts with modal dialog, stop the timers:
PauseMyTimers();

// i=59 "Enter your comments"
wxTextEntryDialog dlg0(NULL, m_messg[59]);
if(dlg0.ShowModal() == wxID_OK) {
 comments0 = dlg0.GetValue();
 status = 0;
}

// Restart the timers:
ResumeMyTimers();

return(status);
}
/************************************************************************
* Stop the timers (used to avoid conflicts with modal dialogs)
*************************************************************************/
void Psc1_PiscoPanel::PauseMyTimers()
{
if(initialized == 1234) {
 if(risley_timer != NULL) risley_timer->Stop();
 if(rs232_timer != NULL) rs232_timer->Stop();
 }
return;
}
/************************************************************************
* Restart the timers (used to avoid conflicts with modal dialogs)
*************************************************************************/
void Psc1_PiscoPanel::ResumeMyTimers()
{
if(initialized == 1234) {
 if(risley_timer != NULL) risley_timer->Start(risley_timer_msec);
 if(rs232_timer != NULL) rs232_timer->Start(rs232_timer_msec);
 }
return;
}
/************************************************************************
* Write to logbook
*************************************************************************/
int Psc1_PiscoPanel::WriteToLogbook(wxString str1, bool SaveToFile)
{
int status = -1;

 if(initialized == 1234) {
  if(fp_logbook != NULL) fprintf(fp_logbook, "%s\n", (const char *)str1.mb_str());
  if(jlp_logbook1 != NULL) status = jlp_logbook1->WriteToLogbook(str1, SaveToFile);
 }

return(status);
}
/***************************************************************
* Update logbook when starting an exposure
****************************************************************/
int Psc1_PiscoPanel::Logbook_StartExposure()
{
double yyear, utime1, h2;
int mmonth, dday;
char cbuffer0[80], cbuffer1[80], cbuffer2[80], cbuffer3[80];
int ih0, ih1, k, status;
wxString buffer, comments0;

if(jlp_logbook1 == NULL) return(1);

/* Ask for comments and return if CANCEL was selected: */
  status = GetCommentsForTheLogbook(comments0);
  if(status != 0) return(2);

// fprintf(fp_logbook, "%s",
//         "******************* NEW EXPOSURE *********************\n");
// i=403 "NEW EXPOSURE"
  sprintf(cbuffer1,
          "******************* %s *********************\n",
          (const char *)m_messg[403].mb_str());
  WriteToLogbook(wxString(cbuffer1), true);

// i=404 "Object"
  strcpy(cbuffer1, (const char *)m_messg[404].mb_str());
  strcpy(cbuffer2, (const char *)ObjectName.mb_str());
  sprintf(cbuffer3,"%s = %s\n", cbuffer1, cbuffer2);
  WriteToLogbook(wxString(cbuffer3), true);

// Write comments on the next line:
  if(!comments0.IsEmpty()) {
    sprintf(cbuffer1, "%s\n", (const char *)comments0.mb_str());
    WriteToLogbook(wxString(cbuffer1), true);
    }

// Astronomical target:
  if(target_type == 0) {
// Right ascension:
    convert_coord(alpha_value,&ih0,&ih1,&h2,"H");
    sprintf(cbuffer1, "Alpha = %2d h %2d m %.1f s ", ih0, ih1, h2);
    WriteToLogbook(wxString(cbuffer1), true);

// Declination:
    convert_coord(delta_value,&ih0,&ih1,&h2,"D");
// Case of -0 30 54:
    if(ih0 == 0 && delta_value < 0.)
      sprintf(cbuffer1, "Delta = -00 d %2d' %2d\" ", ih1, (int)h2);
    else
      sprintf(cbuffer1, "Delta = %3d d %2d' %2d\" ", ih0, ih1, (int)h2);
    WriteToLogbook(wxString(cbuffer1), true);

// Equinox:
    sprintf(cbuffer1," (Equin=%.2f) \n",equin_value);
    WriteToLogbook(wxString(cbuffer1), true);
  }


// Start time (U.T.): here I only use the date
  jlp_utime(cbuffer1, &utime1, &yyear, &mmonth, &dday, UT_shift);

// fprintf(fp_logbook,"---> Start: %s (U.T.) on %02d/%02d/%4d.\n",
// i=405 "Start"
// i=401 "U.T."
// i=406 "on"
 convert_coord(exposure_start_time, &ih0, &ih1, &h2, (const char *)("H"));
 strcpy(cbuffer1, (const char *)m_messg[405].mb_str());
 strcpy(cbuffer2, (const char *)m_messg[401].mb_str());
 strcpy(cbuffer3, (const char *)m_messg[406].mb_str());
 sprintf(cbuffer0,"---> %s: %d h %d m %.1f s (%s) %s %02d/%02d/%4d.\n",
         cbuffer1, ih0, ih1, h2, cbuffer2, cbuffer3,
         dday, mmonth, (int)yyear);
 WriteToLogbook(wxString(cbuffer0), true);

// i=407 "Wheels"
 sprintf(cbuffer1, "%s: ", (const char *)m_messg[407].mb_str());
 WriteToLogbook(wxString(cbuffer1), true);

/* Wheel positions: */
  buffer = wxT("");
  for (k = 0; k < nwheels; k++) {
    if(wheel_is_there[k]) {
      sprintf(cbuffer1, "  %.2s %1d", wheel_name[k], wheel_position[k]);
      buffer.Append(wxString(cbuffer1));
    }
  }

  buffer.Append(wxT("\n"));
  WriteToLogbook(buffer, true);

// Write GR position (eyepiece focal lentgh, etc)
  buffer = PscCmb_Wheel[nwheels - 1].combo->GetStringSelection();
  sprintf(cbuffer1, "GR%s ", (const char *)buffer.mb_str());
  WriteToLogbook(wxString(cbuffer1), true);

/* Filter index: */
 k = FilterIndex();
// fprintf(fp_logbook," Filtre: %.15s     ",&FilterParam[k][5]);
// i=408 "Filter"
 strcpy(cbuffer1, FilterParam[k]);
 strcpy(cbuffer2, (const char *)m_messg[408].mb_str());
 sprintf(cbuffer0,"%s %.15s \n", cbuffer2, &cbuffer1[5]);
 WriteToLogbook(wxString(cbuffer0), true);

// Astronomical target:
  if(target_type == 0) {

// fprintf(fp_logbook," Hour angle: %2d h %2d m %2d s ",ih0,ih1,(int)h2);
// i=226 "Hour angle"
    convert_coord(hour_angle,&ih0,&ih1,&h2,"H");
    strcpy(cbuffer1, (const char *)m_messg[226].mb_str());
    sprintf(cbuffer0,"%s %2dh %2dm %2ds ", cbuffer1, ih0,ih1,(int)h2);
    WriteToLogbook(wxString(cbuffer0), true);

// fprintf(fp_logbook," Elevation: %.1f ",elevation);
// i=224 "Elevation"
    strcpy(cbuffer1, (const char *)m_messg[224].mb_str());
    sprintf(cbuffer0,"%s %.1f ", cbuffer1, elevation);
    WriteToLogbook(wxString(cbuffer0), true);

// fprintf(fp_logbook," Airmass: %.2f \n",air_mass);
// i=225 "Air mass"
    strcpy(cbuffer1, (const char *)m_messg[225].mb_str());
    sprintf(cbuffer0,"%s %.2f\n", cbuffer1, air_mass);
    WriteToLogbook(wxString(cbuffer0), true);

// End of astronomical target
    }

return(0);
}
/***************************************************************
* Update logbook when stopping an exposure
****************************************************************/
void Psc1_PiscoPanel::Logbook_StopExposure()
{
int ih0, ih1;
double h2, exposure_time;
char cbuffer0[128], cbuffer1[128], cbuffer2[128];

// fprintf(fp_logbook,"---> End: %s (U.T.) ",time0);
// i=400 "End"
// i=401 "U.T."
 convert_coord(exposure_stop_time, &ih0, &ih1, &h2, (const char *)("H"));
 strcpy(cbuffer1, (const char*)m_messg[400].mb_str());
 strcpy(cbuffer2, (const char*)m_messg[401].mb_str());
 sprintf(cbuffer0,"---> %s: %d h %d m %.1f (%s)", cbuffer1,
         ih0, ih1, h2, cbuffer2);
 WriteToLogbook(wxString(cbuffer0), true);

// fprintf(fp_logbook,"  (Exposure time: %.1f s)\n", exposure_time);
// i=402 "Exposure time"
 exposure_time = exposure_stop_time - exposure_start_time;
 if(exposure_time < 0.) exposure_time += 24.;
 if(exposure_time > 24.) exposure_time -= 24.;
// Conversion to seconds:
 exposure_time *= 3600.;
 strcpy(cbuffer1, (const char*)m_messg[402].mb_str());
 sprintf(cbuffer0,"  (%s: %.1f s)\n", cbuffer1, exposure_time);
 WriteToLogbook(wxString(cbuffer0), true);

return;
}
