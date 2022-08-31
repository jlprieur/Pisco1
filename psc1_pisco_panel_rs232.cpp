/***************************************************************
 * Name:      psc1_pisco_panel_rs232.cpp
 * Author:    JLP
 * Version:   29/10/2015
 **************************************************************/
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>  // sleep routine 
#endif

#include "psc1_typedef.h"          // MAXI()
#include "psc1_frame_id.h"
#include "psc1_pisco_panel.h"
#include "psc1_rs232_mutex.h"
#include "jlp_rs232_dlg.h"      // JLP_RS232_Dlg class
#include "jlp_rs232_in_c.h"
#include "jlp_rs232_thread.h"

// define DEBUG

/****************************************************************
* Start_RS232 timer
****************************************************************/
void Psc1_PiscoPanel::StartRs232Timer()
{

// Set delay value to update RS232 messages 10 times/second:
 rs232_timer_msec = 100;

// Timer for updating the Risley position:
 rs232_timer = new wxTimer(this, ID_PISCO_RS232_TIMER);

// Start timer with time interval in milliseconds
 rs232_timer->Start(rs232_timer_msec);

 return;
}
/****************************************************************
* Stop_RS232 timer
****************************************************************/
void Psc1_PiscoPanel::StopRs232Timer()
{
 if(initialized != 1234) return;

 if(rs232_timer) rs232_timer->Stop();

 return;
}
/****************************************************************
* Open RS232 link and start PISCO (boot)
****************************************************************/
int Psc1_PiscoPanel::OpenRS232LinkAndStartPisco()
{
JLP_RS232_Dlg *RS232Dlg;
int status, baudrate0, comport_nber0;
bool RS232_is_busy;

// Initialize baudrate, and port number:
 baudrate0 = 9600;

// comport_nber1=0 corresponds to COM1:
 comport_nber1 = -1;

// Block the link:
 RS232_is_busy = true;
 Write_RS232Mutex(RS232_is_busy);

// Open popup window for selecting RS232 link connection settings:
// i=150 "RS232 link"
 RS232Dlg = new JLP_RS232_Dlg(NULL, m_messg[150], m_messg, NMAX_MESSAGES);
 status = RS232Dlg->ShowModal();

// Retrieve the RS232 port parameters:
 RS232Dlg->RetrieveData(&comport_nber0, &baudrate0);

 delete RS232Dlg;

// Set private variables comport_nber1 and baudrate1 if status is OK:
 if(status == 0 && comport_nber0 >= 0 && comport_nber0 < NPORTS_MAXI) {
    comport_nber1 = comport_nber0;
    baudrate1 = baudrate0;
   } else {
    return(-1);
   }

// Free the link:
 RS232_is_busy = false;
 Write_RS232Mutex(RS232_is_busy);

// Initialize PISCO (need RS232_is_busy := false):
  InitPisco();

 return(status);
}
/**************************************************************************************
* Setup RS232 window:
*
***************************************************************************************/
void Psc1_PiscoPanel::RS232_window_setup(wxBoxSizer *vsizer1)
{
wxStaticBoxSizer *BoxSizer1;
wxBoxSizer *hsizer0, *hsizer1, *hsizer2;

//  BoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Serial link"));
  BoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, m_messg[210]);

// Box to send messages to RS232 link (with/without CR):
  hsizer0 = new wxBoxSizer(wxHORIZONTAL);
// 211: "Message:"
  hsizer0->Add(new wxStaticText(this, wxID_ANY, m_messg[211]),
                                0, wxLEFT | wxALIGN_CENTER_VERTICAL, 10);
  PscCtrl_rs232_messgBox = new wxTextCtrl(this, wxID_ANY, wxT(""),
                                           wxDefaultPosition, wxSize(100,28));
  hsizer0->Add(PscCtrl_rs232_messgBox, 0,
               wxLEFT | wxALIGN_CENTER_VERTICAL, 10);
// 212: "Send"
  PscBut_rs232_send = new wxButton(this, ID_RS232_SEND, m_messg[212]);
  hsizer0->Add(PscBut_rs232_send, 0,
               wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);
// 215: "Send without CR"
  PscBut_rs232_send_cr = new wxButton(this, ID_RS232_SEND_WOCR, m_messg[215]);
  hsizer0->Add(PscBut_rs232_send_cr, 0,
               wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 10);
  BoxSizer1->Add(hsizer0);

// Display for received messages:
  hsizer1 = new wxBoxSizer(wxHORIZONTAL);
  PscCtrl_rs232_receivBox = new wxTextCtrl(this, ID_RS232_RECBOX, wxEmptyString,
                                           wxDefaultPosition, wxSize(350,260),
                                           wxTE_READONLY | wxTE_MULTILINE);
  hsizer1->Add(PscCtrl_rs232_receivBox, 1, wxEXPAND);
  BoxSizer1->Add(hsizer1, 0, wxALL, 10);

  hsizer2 = new wxBoxSizer(wxHORIZONTAL);
  hsizer2->Add(BoxSizer1);
//  vsizer1->Add(hsizer2, 1, wxEXPAND);
  vsizer1->Add(hsizer2, 0, wxEXPAND | wxLEFT, 10);

return;
}
/****************************************************************
* Open RS232 link and start PISCO (boot)
****************************************************************/
void Psc1_PiscoPanel::UpdateReceiveBox(wxString buffer)
{
 if(!buffer.IsEmpty())
    *PscCtrl_rs232_receivBox << buffer;
return;
}
/****************************************************************
* Send a message directly to PISCO
*****************************************************************/
void Psc1_PiscoPanel::OnRS232Send(wxCommandEvent& event)
{
char command1[128];
int n_wanted1, n_received1;
wxString buffer, answer1, err_message1;

if(initialized != 1234) return;

buffer = PscCtrl_rs232_messgBox->GetValue();
// Without CR:
 if(event.GetId() == ID_RS232_SEND_WOCR) {
    sprintf(command1, "%s", (const char*)buffer.mb_str());
// With CR:
  } else {
    sprintf(command1, "%s\r", (const char*)buffer.mb_str());
  }

// Assume that 32 bits will be wanted for the answer:
n_wanted1 = 32;
RS232_SendCommand2(command1, n_wanted1, answer1, &n_received1, err_message1);

return;
}
/****************************************************************
* Send a command to PISCO
*****************************************************************/
void Psc1_PiscoPanel::RS232_SendCommand2(char *command1,
                           int n_wanted, wxString& answer_1,
                           int *n_received, wxString& error_message1)
{

wxString buffer;
int status;
double delay_seconds;
/* NOT BETTER
int ch1_size, i;
unsigned char ch1[32];
*/
bool RS232_is_busy;
int rs232_emission_delay_msec;

 *n_received = 0;

// Set delay value to slow down RS232 emission
// (minimum time between two commands):
 rs232_emission_delay_msec = 100;

// Inquire the status of the link:
// to avoid conflicts in SEND/RECEIVE commands to serial port:
 status = Read_RS232Mutex(RS232_is_busy);
 if(status || RS232_is_busy) return;

// Wait for some time to prevent overloading the port:
//  Sleep(rs232_emission_delay_msec);
  delay_seconds = (double)rs232_emission_delay_msec / 1000.;
  sleep(delay_seconds);

// Update display:
// Multiline text: add a EOF line
// *PscCtrl_rs232_receivBox << wxT(">") + command1 + wxT("\n");
  buffer.Printf(wxT(">%s\n"), command1);
  UpdateReceiveBox(buffer);

// Block the link:
  RS232_is_busy = true;
  Write_RS232Mutex(RS232_is_busy);

// JLP2015: purge the port: does not improve anything !
//  RS232_PurgeComport(comport_nber1);

// Send Command: (\r is added in calling routines when needed)
  RS232_cputs(comport_nber1, command1);
/* TRY something else: NOT BETTER !
  ch1_size = 1;
  for(i = 0; i < 20; i++) {
   ch1[0] = (unsigned char)command1[i];
   if(ch1[0] == 0) break;
   ch1[1] = 0;
   RS232_SendBuf(comport_nber1, ch1, ch1_size);
//   Sleep(1);
//  Sleep(rs232_emission_delay_msec);
   delay_seconds = 1. / 1000.;
   sleep(delay_seconds);
   }
*/


if(n_wanted > 0) {
    RS232GetTheAnswer(n_wanted, answer_1, n_received, error_message1);
   }

// Free the link:
  RS232_is_busy = false;
  Write_RS232Mutex(RS232_is_busy);

#ifdef DEBUG
if(n_wanted > 0) {
 buffer.Printf(wxT("command=>%s< ichar=%d n_wanted=%d n_received=%d, status=%d \n answer=>%s<\n"),
               command1, (int)command1[1], n_wanted, *n_received, status,
               (const char*)answer_1.mb_str());
 wxMessageBox(buffer, wxT("End of SendCommand\n"), wxOK);
}
#endif
/*
 } else {
 buffer.Printf(wxT("command=>%s< \n"), (const char*)command1.mb_str());
 }
*/

return;
}
/****************************************************************
* Send a command to PISCO
*****************************************************************/
void Psc1_PiscoPanel::RS232_SendCommand(char *command1)
{
wxString answer_1, error_message1;
int n_wanted = 0, n_received;

RS232_SendCommand2(command1, n_wanted, answer_1, &n_received,
                   error_message1);
return;
}
/****************************************************************
* Receiver timer: display received messages from RS232 port
*****************************************************************/
void Psc1_PiscoPanel::OnRS232ReceiverTimer(wxTimerEvent& event)
{
wxString message1;
bool RS232_is_busy = true;
int length1, status;

// Inquire the status of the link:
status = Read_RS232Mutex(RS232_is_busy);
// Return since is may be in conflict with GetTheAnswer:
if(status || RS232_is_busy) return;

// Block the link:
RS232_is_busy = true;
Write_RS232Mutex(RS232_is_busy);

RS232ReceiveMessage(message1, &length1);

// Show the message:
 if(length1 > 0) {
   *PscCtrl_rs232_receivBox << message1;
// Multiline text: add a carriage return
// *PscCtrl_rs232_receivBox << wxT("\r");
 }

// Free the link:
RS232_is_busy = false;
Write_RS232Mutex(RS232_is_busy);

return;
}
/****************************************************************
* Get answer from the received messages from the RS232 port
* The Mutex is kept as busy by the calling routine
*****************************************************************/
int Psc1_PiscoPanel::RS232GetTheAnswer(int n_wanted, wxString& answer1,
                                  int *n_received, wxString& error_message1)
{
wxString message1;
int i, length1, waiting_time_msec, status = -1;
double delay_seconds;

*n_received = 0;
answer1 = wxT("");

// Try 4 times at most
// JLP2016: 4 times are needed to get at least 400 characters for wheel status
waiting_time_msec = 10;
for(i = 0; i < 4; i++) {

// NB: 700 characters are needed for CheckPositions()
// Sleep(waiting_time_msec);
  delay_seconds = (double)waiting_time_msec / 1000.;
  sleep(delay_seconds);

 RS232ReceiveMessage(message1, &length1);
// Show the message:
 if(length1 > 0) {
   *PscCtrl_rs232_receivBox << message1;
   *n_received += length1;
   answer1.Append(message1);
   if(*n_received >= n_wanted) {
     status = 0;
     break;
     }
   }
}
return(status);
}
/****************************************************************
* Receive message from the RS232 port
*****************************************************************/
void Psc1_PiscoPanel::RS232ReceiveMessage(wxString& message1, int* length1)
{
char recBuff[512], cbuffer[512], ch1;
int nreaded, i, k;

*length1 = 0;
message1 = wxT("");

if(initialized != 1234) return;

// Poll the serie's buffer
recBuff[0] = 0;
nreaded = 0;
RS232_PollComport(comport_nber1, recBuff, 510, &nreaded);

  if(nreaded != 0) {

// Copy readable characters
    k = 0;
    for(i = 0; i < nreaded; i++) {
      ch1 = (char)recBuff[i];
//      if(isprint(ch1) || ch1 == '\n' || ch1 == '\r') {
      if(isprint(ch1) || ch1 == '\r') {
        cbuffer[k] = ch1;
        k++;
        }
      }
// Finalize the char buffer
    cbuffer[k] = '\0';
    *length1 = k;
    message1 = wxString(cbuffer);
  }

return;
}
