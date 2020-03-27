/**************************************************************
* tav1.cpp
* Program to control the speckle camera of OMP
* Written from OWLCVT 11/09/94 09:58:43
*
* JLP 2005: SHOULD BE RUN WITH saxcomm.vbx IN THE DIRECTORY OF THE
* EXECUTABLE FILE (or copy this file to c:\windows\system) !!!!!!
*
* Project file (Borland C++ 5.2) SHOULD be created with
*  - VBX, OWL
*  - CodeGuard
*  - shareable libraries
*
*  (NOT with multithreads !)
* No supplementary definitions are then needed, and no inclusion of libraries
* (_RTLDLL;_BIDSDLL;_OWLDLL;STRICT;_OWLPCH;)
*
* Language: C++
*
* JLP
* Version of 09/12/2005
***************************************************************/
#include <stdio.h>
#include <math.h>
#include <string.h>     // for strcpy and strcat
#include <stdlib.h>     // for atoi
#include <ctype.h>      // for isdigit and isalpha

#include <tav.h>
#include "tav1.h"
#include "tav2.h"
#include "tav1.rh"     // TAV_MENU


/***************************************************************
* Main program
****************************************************************/
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  TBIVbxLibrary vbxLib;     // constructing this loads & inits the library
                            // Needed for RS232 communications...
  TTav1 Tav1("Main window", hInstance, hPrevInstance, lpCmdLine, nCmdShow);
  Tav1.Run();
return Tav1.Status;
}

/*--------------------------------------------------
* Idle Action: when the application is "on rest"
*--------------------------------------------------*/
bool TTav1::IdleAction(long IdleCount)
{
char error_message[100];

if(initialized != 1234) return FALSE;

/********* Update every 3 seconds time and Risley correction: */
  if(TTwin->Exposure_on())
         {
/* Delay: */
          Sleep(1000);
/* Display new values on the screen: */
              TTwin->RefreshScreen(error_message, 0);
// impossible: ... ::MessageBox(HWindow, error_message, "Refresh_Screen (Idle):",
//                              MB_OK);
         }

return TRUE;
}
/********************************************************************
Construct the validate application's MainWindow of type TTestWindow
*********************************************************************/
void TTav1::InitMainWindow()
{
char title[80];

/* Titre de l'icone */
#if LANGUAGE == FRENCH
  strcpy(title,"Commande à distance de PISCO");
#elif LANGUAGE == ITALIAN
  strcpy(title,"Controllo di PISCO");
#else
  strcpy(title,"PISCO remote control");
#endif
  TTwin = new TTestWindow(NULL,title);
/* Icon: */
  TTwin->SetIcon(this,IDI_SAXAPPLICATION);
// Menu:
  TTwin->AssignMenu(TAV_MENU);

 initialized = 1234;

// Define it as the main window:
 SetMainWindow(TTwin);

return;
}

