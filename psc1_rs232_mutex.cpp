/**************************************************************************
* psc1_rs232_mutex.h
*
* Routines to handle the RS232 mutex
* (dialog between main window and Andor thread)
*
* JLP
* Version 26/11/2015
****************************************************************************/
//
//***************************************************************************
// Mutexes are global objects that can be accessed by other processes
// When two processes open a mutex with the same name, they refer to the
// same mutex. This allows multiple processes to be synchronized.
//***************************************************************************
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include "wx/thread.h" // also for wxMutex

#include <stdlib.h>
#include <stdio.h>
#include "psc1_rs232_mutex.h"

// Prototypes included in psc1_rs232_mutex.h:
/*
int Create_RS232Mutex();
int Delete_RS232Mutex();
int Read_RS232Mutex(BOOL &rs232_is_busy);
int Write_RS232Mutex(BOOL rs232_is_busy);
*/

// Mutex for handling "Stop" button:
static wxMutex *hRS232Mutex;
static bool rs232_is_busy_saved_value = TRUE;
static char sss[120];

/**************************************************************************
* Routine to create the RS232 Mutex:
**************************************************************************/
int Create_RS232Mutex() {
int status = 0;

 hRS232Mutex = new wxMutex();

 if(hRS232Mutex == NULL) {
   wxMessageBox(wxT("Error creating mutex"), wxT("CreateRS232Mutex"),
                wxICON_ERROR);
   status = -1;
   }

return(status);
}
/**************************************************************************
* Routine to delete the RS232 Mutex:
**************************************************************************/
int Delete_RS232Mutex() {
int status = -1;

 if(hRS232Mutex != NULL) {
   delete hRS232Mutex;
   hRS232Mutex = NULL;
   status = 0;
   }

return(status);
}
/**************************************************************************
* Routine to access the RS232 Mutex and read the current status:
**************************************************************************/
int Read_RS232Mutex(bool &rs232_is_busy) {
int status = 0;
wxMutexError mutex_status;

 if(hRS232Mutex == NULL) return(-1);

// timeout set to 1000 msec:
 mutex_status=hRS232Mutex->LockTimeout(1000);
 switch (mutex_status) {
   case wxMUTEX_NO_ERROR:
     status = 0;
     break;
   default:
   case wxMUTEX_DEAD_LOCK:
   case wxMUTEX_TIMEOUT:
     status = -1;
#ifdef DEBUG
     sprintf(sss,"hRS232Mutex->LockTimeout(1000) returned %d",
             (int)mutex_status);
     wxMessageBox(wxString(sss), wxT("Read_RS232Mutex"),
                 wxICON_ERROR);
#endif
     break;
  }

// Now access to rs232_is_busy_value and unlock the mutex:
 if(status == 0) {
    rs232_is_busy = rs232_is_busy_saved_value;
    hRS232Mutex->Unlock();
 } else {
    rs232_is_busy = true;
 }

return(status);
}
/**************************************************************************
* Routine to access the RS232 Mutex and modify the current status:
**************************************************************************/
int Write_RS232Mutex(bool rs232_is_busy) {
int status = 0;
wxMutexError mutex_status;

 if(hRS232Mutex == NULL) return(-1);

// timeout set to 1000 msec:
 mutex_status=hRS232Mutex->LockTimeout(1000);
 switch (mutex_status) {
   case wxMUTEX_NO_ERROR:
     status = 0;
     break;
   default:
   case wxMUTEX_DEAD_LOCK:
   case wxMUTEX_TIMEOUT:
     status = -1;
     sprintf(sss,"hRS232Mutex->LockTimeout(1000) returned %d",
             (int)mutex_status);
     wxMessageBox(wxString(sss), wxT("Write_RS232Mutex"),
                 wxICON_ERROR);
     break;
  }

// Now access to rs232_is_busy_value and unlock the mutex:
 if(status == 0) {
    rs232_is_busy_saved_value = rs232_is_busy;
    hRS232Mutex->Unlock();
 }

return(status);
}
