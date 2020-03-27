/*****************************************************************************
* psc1_rs232_thread.h
*
* JLP_Rs232Thread
*
* JLP
* Version 03/12/2015
*
*****************************************************************************/
#include "psc1_frame.h"
#include "psc1_frame_id.h"         // ID_RSTHREAD_DONE
#include "jlp_rs232_in_c.h"             // RS232_PollPort()...
#include "jlp_rs232_thread.h"      // JLP_Rs232Thread()...
#include "psc1_rs232_mutex.h"      // Mutex()...

/*
#define DEBUG
*/

/****************************************************************************
* JLP_Rs232Thread
* Constructor
****************************************************************************/
JLP_Rs232Thread::JLP_Rs232Thread(Psc1_PiscoPanel *pisco_panel,
                                 int comport_nber0, int baudrate0)
                                 : wxThread(wxTHREAD_DETACHED)
{
  comport_nber1 = comport_nber0;
  baudrate1 = baudrate0;
  m_pisco_panel = pisco_panel;
  answer_to_be_read = false;
  command_to_be_sent = false;
  Write_RS232Mutex(command_to_be_sent, answer_to_be_read);
  command1 = wxT("");
  answer1 = wxT("");
  OldBuffer = wxT("");
  AnswerIsReceivedEvent = NULL;
  n_wanted = 0;
  n_received = 0;

#ifdef TEST_VERSION
  OpenSerialPort();
#endif
}
#ifdef TEST_VERSION
/****************************************************************************
* JLP_Rs232Thread
* Open port
****************************************************************************/
int JLP_Rs232Thread::OpenSerialPort()
{
wxString buffer;
// 8bits No parity 1 stop bit 0=eof string
char mode1[] = {'8', 'N', '1', 0};
int status;

// Open connection:
    if(!RS232_OpenComport(comport_nber1, baudrate1, mode1)) {
    buffer.Printf(wxT("Serial COM%d port successfuly opened\n\
 (%d bauds, 8 bits, no parity, 1 stop-bit)"),
                    comport_nber1 + 1, baudrate1);
    wxMessageBox(buffer, wxT("JLP_RS232Thread"), wxOK);
    status = 0;
    } else {
    buffer.Printf(wxT("Error opening serial COM%d port\n\
 (%d bauds, 8 bits, no parity, 1 stop-bit)"),
                    comport_nber1 + 1, baudrate1);
    wxMessageBox(buffer, wxT("JLP_RS232Thread"), wxOK | wxICON_ERROR);
    status = -1;
    }

return(status);
}
#endif
/****************************************************************************
* JLP_Rs232Thread
* Destructor
****************************************************************************/
JLP_Rs232Thread::~JLP_Rs232Thread()
{
// Closing the port before quitting the application
 RS232_CloseComport(comport_nber1);

return;
}
#ifdef TEST_VERSION
/****************************************************************************
* Setup before sending a command
*
****************************************************************************/
void JLP_Rs232Thread::SendCommand(wxString command0, int n_wanted0)
{
wxString buffer;

  command1 = command0;
  n_wanted = n_wanted0;
  n_received = 0;

  if(n_wanted > 0) answer_to_be_read = true;
   else answer_to_be_read = false;

  answer1 = wxT("");
  command_to_be_sent = true;
  OldBuffer = wxT("");

  Write_RS232Mutex(command_to_be_sent, answer_to_be_read);

return;
}
/****************************************************************************
* Setup before starting new processing
*
****************************************************************************/
void JLP_Rs232Thread::SendingTheCommand()
{
wxString buffer;
char sended[512];

// Update display:
// Update Screen
//**************************************************************************
// inform the GUI toolkit that we're going to use GUI functions
// from a secondary thread:
  wxMutexGuiEnter();
// Multiline text: add a carriage return
// *PscCtrl_rs232_receivBox << wxT(">") + command1 + wxT("\n");
  buffer = wxT(">") + command1 + wxT("\n");
  m_pisco_panel->UpdateReceiveBox(buffer);
 wxMutexGuiLeave();

// Send Command: (\r is added in calling routines when needed)
  sprintf(sended, "%s\n", (const char *)command1.c_str());
  if(comport_nber1 >= 0) RS232_cputs(comport_nber1, sended);

  command_to_be_sent = false;
  if(n_wanted > 0)
    answer_to_be_read = true;
  else
    answer_to_be_read = false;

  Write_RS232Mutex(command_to_be_sent, answer_to_be_read);

wxMessageBox(command1, wxT("SendingTheCommand"), wxOK);
return;
}
#endif
/****************************************************************************
* Setup before sending a command
*
****************************************************************************/
void JLP_Rs232Thread::AskForAnswer(int n_wanted0)
{
  n_wanted = n_wanted0;
  n_received = 0;

  if(n_wanted > 0) answer_to_be_read = true;
   else answer_to_be_read = false;

  answer1 = wxT("");

Write_RS232Mutex(command_to_be_sent, answer_to_be_read);
return;
}
/****************************************************************************
* Called when the thread exits - whether it terminates normally or is
* stopped with Delete() (but not when it is Kill()ed!)
*
****************************************************************************/
void JLP_Rs232Thread::OnExit()
{
return;
}
/****************************************************************************
* Thread execution starts here
*
****************************************************************************/
wxThread::ExitCode JLP_Rs232Thread::Entry()
{
 while( !TestDestroy()) {
// Sleep or work...
#ifdef TEST_VERSION
  Read_RS232Mutex(command_to_be_sent, answer_to_be_read);
  if(command_to_be_sent) {
    SendingTheCommand();
    } else if(answer_to_be_read) {
    ReceivingTheAnswer();
    } else {
    ListeningToThePort();
    }
#else
    Sleep(100);
    ListeningToThePort();
#endif
 }

OldBuffer = wxT("");
return NULL;  // Success
}
/****************************************************************************
* Receiving any answer from port
* Save the answer to buffer until n_wanted has been received
****************************************************************************/
void JLP_Rs232Thread::ListeningToThePort()
{
wxString buffer;
char recBuff[4096], recbuf[32];
char cbuffer[4096], ch1;
int nreaded, i, j, k, status;

buffer = wxT("");

// Poll the serie's buffer
  recBuff[0] = 0;
  k = 0;
  for(i = 0; i < 32; i++) {
// Check if there is something to be read:
    recbuf[0] = '\0';
    nreaded = 0;
    status = RS232_PollComport(comport_nber1, recbuf, 8, &nreaded);
//    if(status == 0 && nreaded > 0) {
    if(nreaded > 0) {
     for(j = 0; j < nreaded; j++) {
       recBuff[k] =recbuf[j];
       k++;
       }
     } else {
     break;
     }
  }

// If nothing could be read, will wait for next thread iteration
  nreaded = k;
  if(nreaded == 0) return;

// Copy readable characters
    k = 0;
    for(i = 0; i < nreaded; i++) {
      ch1 = (char)recBuff[i];
      if(isprint(ch1) || ch1 == '\n' || ch1 == '\r'
         || ch1 == '\0') {
        cbuffer[k] = ch1;
        k++;
        }
      }
  if(k == 0) return;

// Finalize the char buffer
  cbuffer[k] = '\0';

//   buffer = wxString(cbuffer);
  buffer = wxString(cbuffer);

// Update Screen
  if(!buffer.IsEmpty()) {
// Test to avoid anomalous repetitions
// Warning: this test is in two steps (2nd test is valid only if buffer is not empty)
    if (buffer.Cmp(OldBuffer) != 0) {
     OldBuffer = buffer;
//**************************************************************************
// Inform the GUI toolkit that we're going to use GUI functions
// from a secondary thread:
     wxMutexGuiEnter();
// Show the message
    m_pisco_panel->UpdateReceiveBox(buffer);
// If we don't release the GUI mutex the MyImageDialog won't be able to refresh
     wxMutexGuiLeave();

// Complete the answer if required:
  if(answer_to_be_read) {
    n_received += k;
    answer1.Append(buffer);
    if(n_wanted >= n_received) {
      n_received = n_wanted;
      StopAnswerReception();
     }
   }
  }
}

return;
}
/****************************************************************************
* Receive the answer from port until n_wanted has been received
****************************************************************************/
void JLP_Rs232Thread::StopAnswerReception()
{
wxString buffer;

// Create new event: (can be used only once, since it is deleted by wxQueueEvent
  AnswerIsReceivedEvent = new wxThreadEvent(wxEVT_THREAD, ID_RS232_THREAD_DONE);

// Send event to PscFrame:
  wxQueueEvent( (wxFrame*)m_pisco_panel, AnswerIsReceivedEvent);

answer_to_be_read = false;
Write_RS232Mutex(command_to_be_sent, answer_to_be_read);
}
/****************************************************************************
* Retrieve results
*
****************************************************************************/
int JLP_Rs232Thread::GetTheAnswer(wxString &answer0, int *n_received0,
                               wxString &error_message0)
{
int status;
wxString buffer;

answer0 = answer1;
*n_received0 = n_received;

if(answer_to_be_read) {
  error_message0 = wxT("Error can't get complete answer yet: please wait and try again !\n");
  status = -1;
  } else {
  status = 0;
  }

answer_to_be_read = false;
Write_RS232Mutex(command_to_be_sent, answer_to_be_read);
OldBuffer = wxT("");

return(status);
}
