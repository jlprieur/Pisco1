/****************************************************************************
* Name: psc1_pisco_panel_onclick.cpp
* Psc1_PiscoPanel class
*
* JLP
* Version 01/12/2015
****************************************************************************/
#include "psc1_frame_id.h"
#include "psc1_pisco_panel.h"
#include "jlp_rs232_thread.h"
#include "tav_utils.h"     // utime()

BEGIN_EVENT_TABLE(Psc1_PiscoPanel, wxPanel)

// Combo boxes:
EVT_COMBOBOX(ID_PISCO_WHEEL_0, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_WHEEL_1, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_WHEEL_2, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_WHEEL_3, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_WHEEL_4, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_WHEEL_5, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_WHEEL_6, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_WHEEL_7, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_WHEEL_8, Psc1_PiscoPanel::OnSelectWheelPosition)
EVT_COMBOBOX(ID_PISCO_AUTO_RISLEY, Psc1_PiscoPanel::OnAutoRisleySelect)
EVT_COMBOBOX(ID_PISCO_LAMP_ON, Psc1_PiscoPanel::OnLampSelect)

// Buttons:
EVT_BUTTON(ID_PISCO_CHECK_POSI, Psc1_PiscoPanel::OnCheckPositions)
EVT_BUTTON(ID_PISCO_EXPO_START, Psc1_PiscoPanel::OnStartStopExposure)
EVT_BUTTON(ID_PISCO_RA_VALID, Psc1_PiscoPanel::OnValidateRisley)
EVT_BUTTON(ID_PISCO_RB_VALID, Psc1_PiscoPanel::OnValidateRisley)

// Text controls:

// In "psc1_pisco_panel_rs232.cpp":
EVT_BUTTON(ID_RS232_SEND, Psc1_PiscoPanel::OnRS232Send)
EVT_BUTTON(ID_RS232_SEND_WOCR, Psc1_PiscoPanel::OnRS232Send)

// Timer:
EVT_TIMER(ID_PISCO_RISLEY_TIMER, Psc1_PiscoPanel::OnRisleyTimer)
EVT_TIMER(ID_PISCO_RS232_TIMER, Psc1_PiscoPanel::OnRS232ReceiverTimer)

// Thread:
// EVT_THREAD(ID_RS232_THREAD_DONE, Psc1_PiscoPanel::OnRS232AnswerIsReady)

// Check boxes:
// EVT_CHECKBOX(ID_PROC_DIRECT_VECTOR, JLP_SpeckProcessPanel::OnDirectVectorCheckBoxClick)

END_EVENT_TABLE()

//---------------------------------------------------------------------------
// "Lucky imaging" Check Box
//---------------------------------------------------------------------------
/*
void JLP_SpeckProcessPanel::OnLuckyCheckBoxClick(wxCommandEvent& event)
{

if(initialized != 1234) return;

 Pset1.LuckyImaging = ProcCheck_Lucky->IsChecked();
 ChangesDone1 = true;
}
*/
/****************************************************************************
* Select wheel position
*
****************************************************************************/
void Psc1_PiscoPanel::OnSelectWheelPosition(wxCommandEvent& event)
{
int k_wheel;
wxString answer1, error_message1;
char command1[128];

if(initialized != 1234) return;

// Get wheel index:
k_wheel = event.GetId() - ID_PISCO_WHEEL_0;

if(!wheel_is_there[k_wheel]) {
// i=240 Wheel not connected
  wxMessageBox(m_messg[240], wxT("OnSelectWheelPosition"), wxOK | wxICON_ERROR);
  return;
}

// Get selected position
wheel_position[k_wheel] = PscCmb_Wheel[k_wheel].combo->GetSelection() + 1;
sprintf(command1, "%s %d\r", wheel_name[k_wheel], wheel_position[k_wheel]);

// Send command to PISCO:
   RS232_SendCommand(command1);

return;
}
/********************************************************************
* Handle Combobox event, for switching lamp on and off
********************************************************************/
void Psc1_PiscoPanel::OnLampSelect(wxCommandEvent& WXUNUSED(event))
{
wxString answer1, error_message1;
int iselect;
char command1[128];

if(initialized != 1234) return;

// Get selected position
iselect = PscCmb_Lamp.combo->GetSelection();

lamp_is_on = (iselect == 1);

// Update lamp field:
if(lamp_is_on)
  strcpy(command1, "ON\r");
else
  strcpy(command1, "OFF\r");

// Send command to PISCO:
   RS232_SendCommand(command1);
}
/********************************************************************
* Handle Combobox event,
* for switching automatic Risley correction on and off
********************************************************************/
void Psc1_PiscoPanel::OnAutoRisleySelect(wxCommandEvent& WXUNUSED(event))
{
int iselect;

if(initialized != 1234) return;

// Get selected position
iselect = PscCmb_AutoRisley.combo->GetSelection();

auto_risley_on = (iselect == 1);

}
/********************************************************************
* Start exposition button
* for switching eposition on and off
********************************************************************/
void Psc1_PiscoPanel::OnStartStopExposure(wxCommandEvent& WXUNUSED(event))
{
char cbuffer[80];
double utime1, year;
int month, day;

if(initialized != 1234) return;

jlp_utime(cbuffer, &utime1, &year, &month, &day, UT_shift);

if(!exposure_is_on) {
  exposure_start_time = utime1;
// Write start_time to logbook:
  Logbook_StartExposure();
 } else {
  exposure_stop_time = utime1;
// Write stop_time to logbook:
  Logbook_StopExposure();
 }

// Display alpha, delta, ...:
DisplayNewValues();

// Toggle button label:
exposure_is_on = !exposure_is_on;
if(exposure_is_on) {
// i=209 "Stop Exposure"
  PscBut_StartExposure->SetLabel(m_messg[209]);
 } else {
// i=208 "Start Exposure"
  PscBut_StartExposure->SetLabel(m_messg[208]);
 }

return;
}
/***************************************************************
*  Edition of RA position
****************************************************************/
void Psc1_PiscoPanel::OnValidateRisley(wxCommandEvent& event)
{
wxString buffer, answer1, error_message1;
long ivalue;
char risley_str[8], command1[128];

if(initialized != 1234) return;

if(event.GetId() == ID_PISCO_RA_VALID) {
    strcpy(risley_str, "RA");
// Get text from editing box:
    buffer = PscCtrl_RA->GetValue();
  } else {
    strcpy(risley_str, "RB");
    buffer = PscCtrl_RB->GetValue();
  }

if(!buffer.ToLong(&ivalue)) {
// i=213 "Error: bad syntax"
  wxMessageBox(m_messg[213], wxT("ValidateRisley"),
               wxOK | wxICON_ERROR);
  return;
 }

if(ivalue >= 0 && ivalue < 1024) {
// Send command to PISCO:
// ("RA 0048" or "RB 0543" for example)
  sprintf(command1, "%s %04d\r", risley_str, (int)ivalue);
  RS232_SendCommand(command1);

  if(event.GetId() == ID_PISCO_RA_VALID) {
    ra_position = ivalue;
    buffer.Printf(wxT("RA : %d"), ivalue);
    PscStatic_RA->SetLabel(buffer);
   } else {
    rb_position = ivalue;
    buffer.Printf(wxT("RB : %d"), ivalue);
    PscStatic_RB->SetLabel(buffer);
   }
} else {
// i=214 "Error: bad value"
  wxMessageBox(m_messg[214], wxT("ValidateRisley"),
               wxOK | wxICON_ERROR);
 }

return;
}
/****************************************************************************
* Check and update positions with RS232 link (by sending a "?" to PISCO)
*
****************************************************************************/
void Psc1_PiscoPanel::OnCheckPositions(wxCommandEvent& WXUNUSED(event))
{
if(initialized != 1234) return;

CheckPositions();

}
/****************************************************************************
* Check and update positions with RS232 link (by sending a "?" to PISCO)
*
****************************************************************************/
void Psc1_PiscoPanel::CheckPositions()
{
int k, status;
wxString buffer;

if(initialized != 1234) return;

// Get current status of wheels, lamp and risley prism positions
 status = CurrentStatusFromRS232();
 if(status) return;

// Update positions with found values
for(k = 0; k < nwheels; k++) {
 if(wheel_is_there[k])
   PscCmb_Wheel[k].combo->SetSelection(wheel_position[k] - 1);
 else
   PscCmb_Wheel[k].combo->SetSelection(0);
}

// Risley positions:
buffer.Printf(wxT("%d"), ra_position);
PscCtrl_RA->SetValue(buffer);
buffer.Printf(wxT("%d"), rb_position);
PscCtrl_RB->SetValue(buffer);

// Lamp:
PscCmb_Lamp.combo->SetSelection(lamp_is_on);

return;
}
/****************************************************************************
* Update wheel position by sending a "?" to PISCO
*
****************************************************************************/
/* Warning: answer1, c1, should be large enough... */
#define BUFF_LEN1 1024
int Psc1_PiscoPanel::CurrentStatusFromRS232()
{
int n_wanted, n_received, status;
wxString answer_1, error_message1, buffer;
char command1[128], c1[BUFF_LEN1], answer1[BUFF_LEN1];
char rs232_wheel_names[256], rs232_wheel_is_there[256];
register int i, j;
int istart, iend, c1_len;
int ra_is_found, rb_is_found, nwheels_found;

// Debug:
//  wxMessageBox(wxT("OK1"), wxT("CurrentStatusFromRS232"), wxOK);

// Send command "?" to PISCO with \r
 n_wanted = 500;
 sprintf(command1, "?\r");
 RS232_SendCommand2(command1, n_wanted, answer_1, &n_received, error_message1);

// Debug:
/*
  buffer.Printf(wxT("command=>%s< n_received=%d\n"), command1, n_received);
  buffer.Append(answer_1);
  wxMessageBox(buffer, wxT("DDEBUG/CurrentStatusFromRS232"), wxOK);
*/

// If answer is too short return(-2):
 if(n_received <= 100) return(-2);

// Initialization:
rs232_wheel_names[0] = '\0';
rs232_wheel_is_there[0] = '\0';
ra_is_found = 0;
rb_is_found = 0;
nwheels_found = 0;
strcpy(answer1, (const char*)answer_1);

/*********************************************************
* Read answer1 and update positions
*********************************************************/
/* Example:
       "AS EN CH MA RA RB DB FA FT GR FB FC FD \r\n"
       "-  -  -  -  -  -  -  -  X  -  -  X  X  \r\n"
       "R   No  ORDRE  ETAT \r\n"
       "AS  2  07B5   07DC \r\n"
       "EN  2  07B5   07DC \r\n"
       "CH  5  07B5   07DC \r\n"
       "MA  2  042B   0417 \r\n"
       "RA     0856   0856 \r\n"
       "RB     0257   0256 \r\n"
       "DA  2  07B5   07DC \r\n"
       "DB  2  07B5   07DC \r\n"
       "FA  2  07B5   07DC \r\n"
       "FT  2  07B5   07DC \r\n"
       "GR  2  07B5   07DC \r\n"
       "FB  2  07B5   07DC \r\n"
       "FC  2  0000   0000 \r\n"
       "FD  2  0000   0000 \r\n"
       "LA LAMPE EST OFF";
*/
/* Analyse string from begining to end, looking to \r \n or \f character: */
  answer1[BUFF_LEN1 - 2] = '\0';
  istart = 0; i = 0;
  while(answer1[i] && (i < BUFF_LEN1 - 1))
    {
    istart = i;
/* Increase i as long as answer1[i] is not a control character: */
    while(!iscntrl(answer1[i]) && (i < BUFF_LEN1 - 1)) i++;
    iend = i;
/* Transfer from answer1 to c1 array: */
      for(j = istart; j < iend; j++) c1[j-istart] = answer1[j];
/* c1 length: */
      c1_len = iend - istart + 1;
      c1[c1_len] = '\0';
/* Update parameter: */
      status = RS232Status_Decode_Nextlines(c1,c1_len, &ra_is_found,
                                        &rb_is_found, &nwheels_found,
                                        rs232_wheel_names,
                                        rs232_wheel_is_there);
      if(status) return(-1);
/* Jump 1 step because of "\r" "\f" or "\n": */
    istart = iend + 1;
/* If Control character and not at the end, jump as many steps
as needed: */
    while(iscntrl(answer1[istart]) && (answer1[istart] != '\0')
           && (istart < BUFF_LEN1 - 1)) istart++;
    i = istart;
    }

// Debug:
  buffer.Printf(wxT("DDEBUG:Decode_NextLines/ ra_is_found=%d rb_is_found=%d nwheels=%d"),
          ra_is_found, rb_is_found, nwheels_found);
  wxMessageBox(buffer, wxT("CurrentStatusFromRS232"), wxOK);

/* Determine which wheels are present from the first two lines: */
// Return status=0 if all wheels have been found
   status = RS232Status_Decode_First_TwoLines(rs232_wheel_names,
                                              rs232_wheel_is_there);

return(status);
}
/***************************************************************
*  Look for parameter in c1 string and update it
****************************************************************/
int Psc1_PiscoPanel::RS232Status_Decode_Nextlines(char *c1, int c1_len,
                                int *ra_is_found, int *rb_is_found,
                                int *nwheels_found,
                                char *rs232_wheel_names,
                                char *rs232_wheel_is_there)
{
register int k;

/* First line (AS EN ...): store it to "rs232_wheel_names" buffer: */
  if(!strncmp(c1,"AS EN",5)) {
      strcpy(rs232_wheel_names,c1);
      }
/* Lamp status:  LA LAMPE EST ON or LA LAMPE EST OFF*/
  else if(!strncmp(c1,"LA LAMPE EST ",13))
    {
      if( c1[14] == 'N')
         lamp_is_on = 1;
      else
         lamp_is_on = 0;
    }
/* Wheel position status: searching for X or -.
*  Store it to "rs232_wheel_is_there" buffer. */
  else if(!strncmp(c1,"- ",2) || !strncmp(c1,"X ",2))
    {
       strcpy(rs232_wheel_is_there,c1);
/* Risley prisms: */
  } else if(!strncmp(c1,"RA",2)) {
      sscanf(&c1[3],"%04d",&ra_position);
      *ra_is_found = 1;
  } else if(!strncmp(c1,"RB",2)) {
      sscanf(&c1[3],"%04d",&rb_position);
      *rb_is_found = 1;
/* Loop on all the wheels: */
  } else {
    for (k = 0; k < nwheels; k++) {
      if(!strncmp(c1, wheel_name[k],2))
         {
           sscanf(&c1[3],"%d",&wheel_position[k]);
/* JLP96: Check if status code is correct: */
         sscanf(&c1[6],"%04X",&actual_posi_code[k]);
         sscanf(&c1[13],"%04X",&required_posi_code[k]);
// Debug:
/*
wxString strr1;
strr1.Printf(wxT("Wheel #%d (nwheels=%d): %s %04X %04X (w_name[7,8]=%s %s)"),
             k, nwheels, wheel_name[k],
             actual_posi_code[k], required_posi_code[k],
             wheel_name[7], wheel_name[8]);
wxMessageBox(strr1, wxT("ZZZ/DecodeNextLines"), wxOK);
*/
// One wheel has been found, so I exit from this routine here:
         *nwheels_found = *nwheels_found + 1; break;
         }
      }
/* JLP95: Even if found = 0, should not return status=-1,
* since it can be something else, like G000, etc....
*/
    }

return(0);
}
/***************************************************************
*  Determines which wheels are present from the first two lines
* INPUT:
* char rs232_wheel_names[] : from speckle camera "?" with wheel names
* char rs232_wheel_is_there[] : from speckle camera "?" with crosses if
*                         a wheel is not there.
****************************************************************/
int Psc1_PiscoPanel::RS232Status_Decode_First_TwoLines(
                    char *rs232_wheel_names, char *rs232_wheel_is_there)
{
char *pc1, *pc2;
int status, nwh_found, ra_is_there, rb_is_there;
register int k;

// Initialization:
nwh_found = 0;
for (k = 0; k < nwheels; k++) {
  wheel_is_there[k] = 0;
 }
ra_is_there = 0;
rb_is_there = 0;

/* Order is
AS.EN.CH.MA.RA.RB.DA.DB.FA.FT.GR.FB.FC.FD
 1  2  3  4  5  6  7  8  9  0  1  2  3  4
*/
// Load pc1 and pc2 with two lines already input from "status buffer":
  pc1 = rs232_wheel_names;
  pc2 = rs232_wheel_is_there;
/* Use "rs232_wheel_names" as a reference
*  to increase "rs232_wheel_names" index */
  while (*pc1 && *pc2) {
/* If blank, go to next character:
* A bit more complex than really needed for historical reasons... */
     if(*pc1 == ' ') {
        pc1++; pc2++;
/* Decode 2 characters:
* Check if risley prisms are there: */
      } else if(!strncmp(pc1,"RA",2)) {
          ra_is_there = 1;
// Skip 2 characters, since 2 have been decoded:
          pc1++; pc2++;
          if(*pc1 && *pc2) {pc1++; pc2++;}
      } else if(!strncmp(pc1,"RB",2)) {
          rb_is_there = 1;
// Skip 2 characters, since 2 have been decoded:
          pc1++; pc2++;
          if(*pc1 && *pc2) {pc1++; pc2++;}
// Loop on all the wheels:
      } else {
/*
nwheels = 9
wheel title:  AS EN CH MA DA DB FA FB GR
k index    :   0  1  2  3  4  5  6  7  8
*/
         for (k = 0; k < nwheels; k++) {
           if(!strncmp(pc1, wheel_name[k],2)) {
// Update wheel_is_there array:
             if(!strncmp(pc2,"- ",2))
               {wheel_is_there[k] = 1; nwh_found++;}
             }
// End of "for k ..." loop
           }
// Skip 2 characters, since 2 have been decoded:
         pc1++; pc2++;
         if(*pc1 && *pc2){ pc1++; pc2++; }
// End of case : *pc1 != ' '
        }
// EOF while loop:
      }

// Current PISCO configuration has 9 wheels:
if(!ra_is_there || !rb_is_there || nwh_found != 9)
 status = 1;
else
 status = 0;

return (status);
}
