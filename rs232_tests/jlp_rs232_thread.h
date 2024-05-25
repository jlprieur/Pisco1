/*****************************************************************************
* psc1_rs232_thread.h
*
* JLP_Rs232Thread
*
* JLP version 01/12/2015
*****************************************************************************/
#ifndef _psc1_rs232_thread_h_
#define _psc1_rs232_thread_h_

// ---------------------------------------------------------------------------
// Headers
// ---------------------------------------------------------------------------

#include "wx/wx.h"
#include "wx/thread.h"
#include "wx/dynarray.h"
#include "wx/numdlg.h"
#include "wx/progdlg.h"

class PscFrame;
class Psc1_PiscoPanel;

// ----------------------------------------------------------------------------
// Decode thread
// ----------------------------------------------------------------------------

class JLP_Rs232Thread : public wxThread
{
public:
  JLP_Rs232Thread(Psc1_PiscoPanel *pisco_panel,
                  int comport_nber0, int baudrate0);

// Destructor
  virtual ~JLP_Rs232Thread();

// Thread execution starts here
  virtual wxThread::ExitCode Entry();

#ifdef TEST_VERSION
  int OpenSerialPort();
  void SendCommand(wxString command0, int n_wanted0);
  void SendingTheCommand();
#endif
  void AskForAnswer(int n_wanted0);
  void ReceivingTheAnswer();
  void ListeningToThePort();
  void StopAnswerReception();
  int GetTheAnswer(wxString &answer0, int *n_received0,
                 wxString &error_message0);

// Called when the thread exits - whether it terminates normally or is
// stopped with Delete() (but not when it is Kill()ed!)
  virtual void OnExit();

  bool ShouldTerminate();

private:
  PscFrame *m_pscframe;
  Psc1_PiscoPanel *m_pisco_panel;
  wxThreadEvent *AnswerIsReceivedEvent;
  int comport_nber1, baudrate1;
  int n_wanted, n_received;
  bool command_to_be_sent, answer_to_be_read;
  wxString command1, answer1, OldBuffer;

};

#endif // EOF sentry
