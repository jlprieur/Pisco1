/******************************************************************************
* Name:        psc1_frame_menu (PscFrame class)
* Purpose:     handling menu events of PscFrame clas (see Filespeck2.cpp)
* Author:      JLP
* Version:     28/09/2015
******************************************************************************/
#include "psc1_frame.h"
#include "psc1_frame_id.h"  // Menu identifiers

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__) || defined(__WXMGL__) || defined(__WXX11__)
    #define USE_XPM
#endif

BEGIN_EVENT_TABLE(PscFrame, wxFrame)

// Handle closing event (when pressing on th top-right cross...)
   EVT_CLOSE(PscFrame::OnClose)

// Notebook:
   EVT_BOOKCTRL_PAGE_CHANGING(ID_NOTEBOOK, PscFrame::OnPageChanging)

// Menu items:
   EVT_MENU(ID_QUIT,            PscFrame::OnQuit)

// Menu/Logbook
   EVT_MENU(ID_LOGBOOK_SHOW, PscFrame::OnViewLogbook)
   EVT_MENU(ID_LOGBOOK_HIDE, PscFrame::OnViewLogbook)
   EVT_MENU(ID_LOGBOOK_COMMENTS, PscFrame::OnAddCommentsToLogbook)
   EVT_MENU(ID_LOGBOOK_SAVE, PscFrame::OnSaveLogbook)

// Miscellaneous:
   EVT_MENU(ID_CONTEXT_HELP,   PscFrame::OnContextHelp)
   EVT_MENU(ID_ABOUT,          PscFrame::OnAbout)
   EVT_MENU(ID_HELP,           PscFrame::OnHelp)

// For closing:
   EVT_CLOSE(PscFrame::OnClose)

END_EVENT_TABLE()

/********************************************************************
* Setup the menu on top of main frame
********************************************************************/
void PscFrame::Gdp_SetupMenu()
{
SetHelpText( _T("Program to control PISCO") );

  menu_bar = new wxMenuBar;

// ***************** File menu **********************************
  menuFile = new wxMenu;

#if 0
  menuFileOpen = new wxMenu;
  menuFileOpen->Append(ID_LOAD_RESULTS2, _T("Old results without bispectrum"),
                    _T("LongInt, Autoc, Modsq"));
  menuFile->Append(wxID_ANY,_T("Open"), menuFileOpen, _T("Reading data from files"));
  menuFileSave = new wxMenu;
  menuFileSave->Append( ID_SAVE_RESULTS2, _T("Processing results"),
                    _T("Save processing results to FITS files"));
  menuFileSave->Append( ID_SAVE_TO_3DFITS, _T("Elementary frames to 3DFITS"),
                    _T("Save elementary frames to 3D FITS files"));
#endif
  menuFile->AppendSeparator();
  menuFileSave = new wxMenu;
//  menuFile->Append(wxID_ANY,_T("Save"), menuFileSave, _T("Saving to file"));
  menuFile->Append(wxID_ANY, Str0[iLang][4], menuFileSave, Str0[iLang][5]);
  menuFile->AppendSeparator();

//  menuFile->Append(ID_QUIT, _T("E&xit\tAlt-X"), _T("Quit program"));
  menuFile->Append(ID_QUIT, Str0[iLang][9], Str0[iLang][10]);
//  menu_bar->Append(menuFile, _T("&File"));
  menu_bar->Append(menuFile, Str0[iLang][0]);

// ***************** Logbook menu ******************************
  menuLog = new wxMenu;
// i=50 "Show logbook"
// i=51 "Display the logbook window"
  menuLog->Append( ID_LOGBOOK_SHOW, Str0[iLang][50],
                   Str0[iLang][51], wxITEM_RADIO);
// i=52 "Hide logbook"
// i=53 "Hide the logbook window"
  menuLog->Append( ID_LOGBOOK_HIDE, Str0[iLang][52],
                   Str0[iLang][53], wxITEM_RADIO);
// i=54 "Comments"
// i=55 "Add comments to the logbook"
  menuLog->Append( ID_LOGBOOK_COMMENTS, Str0[iLang][54],
                   Str0[iLang][55]);
// i=56 "Save logbook"
// i=57 "Save the logbook content"
  menuLog->Append( ID_LOGBOOK_SAVE, Str0[iLang][56],
                   Str0[iLang][57]);
// i=58 "Logbook"
  menu_bar->Append(menuLog, Str0[iLang][58]);

// ***************** Help menu ******************************
  menuHelp = new wxMenu;
// i=90 "Help"
  menuHelp->Append( ID_HELP, Str0[iLang][90], Str0[iLang][91]);
// i=92 "Context help...\tCtrl-H"
// i=93 "Get context help for a control"
  menuHelp->Append(ID_CONTEXT_HELP, Str0[iLang][92],
                     Str0[iLang][93]);
// i=94 "About..."
  menuHelp->Append( ID_ABOUT, Str0[iLang][94], Str0[iLang][95]);
// i=96 "Help"
  menu_bar->Append(menuHelp, Str0[iLang][96]);

  SetMenuBar(menu_bar);


return;
}
