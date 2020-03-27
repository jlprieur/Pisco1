/****************************************************************************
* Name: PscFrame.cpp
* PscFrame class
* Purpose: display and process 3D-FITS or Andor images
*
* JLP
* Version 16/09/2015
****************************************************************************/
#include "time.h"


#include "wx/progdlg.h"

/*
#if !wxUSE_TOGGLEBTN
    #define wxToggleButton wxCheckBox
    #define EVT_TOGGLEBUTTON EVT_CHECKBOX
#endif
*/

// JLP routines:
#include "Pisco1.h"            // MyAppl()
#include "psc1_pisco_panel.h"  // Psc1_PiscoPanel class
#include "psc1_frame.h"
#include "psc1_frame_id.h"     // Definition of identifiers
#include "jlp_language_dlg.h"  // JLP_Language_Dlg class

/**********************************************************************
* PscFrame constructor
*
* INPUT:
*   GdpWidth,GdpHeight : size of created window
*
***********************************************************************/
PscFrame::PscFrame(const wxChar *title, int GdpWidth, int GdpHeight)
       : wxFrame(NULL, wxID_ANY, title, wxPoint(-1, -1),
         wxSize(GdpWidth, GdpHeight))
{
  int status;

// Transform coma into point for numbers:
 setlocale(LC_NUMERIC, "C");
 initialized = 0;

// Smaller fonts (needed for Windows, to have a similar appearance as Linux)
// SetWindowVariant(wxWINDOW_VARIANT_SMALL);

// iLang: 0=English 1=French 2=Italian 3=Spanish 4=German
// Language as default:
// French
  iLang = 1;

// Prompt the user for a new value of iLang:
  status = SelectLanguageSetup();
// Exit from here (no window to be closed since nothing opened yet)
  if(status) exit(1);

// Load menu messages to Str0:
  status = LoadMenuMessages();
// Exit from here (no window to be closed since nothing opened yet)
  if(status) exit(2);

// Initialization of private variables
  Main_Init();

// Status bar:
// Create a status bar with two fields at the bottom:
  m_StatusBar = CreateStatusBar(2);
// First field has a variable length, second has a fixed length:
  int widths[2];
  widths[0] = -1;
  widths[1] = 200;
  SetStatusWidths( 2, widths );

// Create notebook and its pages
  NotebookSetup(GdpWidth, GdpHeight);

// Create a menu on top of the window:
  Gdp_SetupMenu();

  initialized = 1234;

return;
}
/**********************************************************************
* PscFrame destructor
*
***********************************************************************/
PscFrame::~PscFrame()
{
}
/**********************************************************************
* Close main window
***********************************************************************/
void PscFrame::OnClose(wxCloseEvent& event)
{
  MyClose();
  event.Skip();
// wxMessageBox(wxT("OK6"), wxT(" Frame/OnClose"), wxOK);
}
/**********************************************************************
* Close main window
***********************************************************************/
bool PscFrame::MyClose()
{
if(initialized == 1234) {
// Pisco panel shutdown: close serial port, stop timer, etc.
 psc1_pisco_panel->MyShutdown();

// Close logbook if previously opened:
 if(fp_logbook) fclose(fp_logbook);

}
initialized = 0;
return(true);
}
/**********************************************************************
* Initialization of private variables
*
***********************************************************************/
void PscFrame::Main_Init()
{

// Text panel as default in notebook:
  i_NotebookPage = 0;
  fp_logbook = NULL;

// Set delay value to update Risley prisms once very 3 seconds:
  risley_delay_msec = 3000;

return;
}
/**********************************************************************
* Notebook setup
*
***********************************************************************/
void PscFrame::NotebookSetup(int width0, int height0)
{
wxString str1;
int width1, height1;

// Create topsizer to locate panels and log window
  m_topsizer = new wxBoxSizer( wxVERTICAL );
// Create book control (multi-panels):
//  m_notebook = new wxBookCtrl(this, ID_BOOK_CTRL);
  m_notebook = new wxNotebook(this, ID_NOTEBOOK);

// Create Logbook panel first:
  str1 = wxString("");
  width1 = width0;
  height1 = (int)((double)height0 / 8.);
  jlp_Logbook = new JLP_wxLogbook(this, str1, width1, height1);

  wxLog::SetActiveTarget(new wxLogTextCtrl(jlp_Logbook));

// Create Parameter panel:
  m_ParamPanel = new wxPanel(m_notebook, ID_PARAM_PANEL);
// i=100 "Parameters"
  m_notebook->AddPage(m_ParamPanel, Str0[iLang][100]);
  ParamPanelSetup();

// Create Pisco panel:
  m_PiscoPanel = new wxPanel(m_notebook, ID_PISCO_PANEL);
// i=102 "Pisco control"
  m_notebook->AddPage(m_PiscoPanel, Str0[iLang][102]);
  PiscoPanelSetup();

// Create Target panel:
// SHOULD BE CALLED AFTER CREATING Psc1_Pisco_Panel since needs its pointer to stop its timers
  m_TargetPanel = new wxPanel(m_notebook, ID_TARGET_PANEL);
// i=101 "Target"
  m_notebook->AddPage(m_TargetPanel, Str0[iLang][101]);
  TargetPanelSetup();

// Proportion set to 7, i.e., graphic panel will be 7/8 of the window
  m_topsizer->Add(m_notebook, 7, wxEXPAND | wxALL);

// Proportion set to 1, i.e., log window will be 1/8 of the window
  m_topsizer->Add(jlp_Logbook, 1, wxEXPAND);

// Sizer implementation on the panel:
  SetSizerAndFit(m_topsizer);

}
/*****************************************************************
* Quit (menu item)
*****************************************************************/
void PscFrame::OnQuit (wxCommandEvent& event)
{
// Close PISCO Panel:
  MyClose();
// Try this:
 wxWindow::Destroy();
}
/*****************************************************************
* Help (menu item)
*****************************************************************/
void PscFrame::OnHelp( wxCommandEvent& WXUNUSED(event) )
{
// i=97 ("Sorry: \"Help\" is not implemented yet\n"
// i=98 ("Current version: October 2015"),
 (void)wxMessageBox(Str0[iLang][97] + wxT("\n") + Str0[iLang][98],
                    _T("Pisco1"), wxICON_INFORMATION | wxOK );
}
/*****************************************************************
* About (menu item)
*****************************************************************/
void PscFrame::OnAbout( wxCommandEvent& WXUNUSED(event) )
{
wxString buffer;

// i=99 ("Created with wxWidgets"),
 buffer = wxT("Pisco1\nJean-Louis Prieur (c) 2015\n")
          + Str0[iLang][99];
 (void)wxMessageBox( buffer, _T("Pisco1"),
                     wxICON_INFORMATION | wxOK );
}
/*****************************************************************
* Context help
*****************************************************************/
void PscFrame::OnContextHelp(wxCommandEvent& WXUNUSED(event))
{
    // starts a local event loop
    wxContextHelp chelp(this);
}
/************************************************************************
* Display text in status bar
*************************************************************************/
void PscFrame::SetText_to_StatusBar(wxString str1, const int icol)
{
// Update the first field (since 2nd argument is 0 here) of the status bar:
  if(m_StatusBar != NULL) m_StatusBar->SetStatusText(str1, icol);
}
/**********************************************************************
* ParamPanel setup
* Create Parameter Panel with Psc1_ParamPanel class (widget panel)
*
***********************************************************************/
void PscFrame::ParamPanelSetup()
//int width0, int height0)
{
wxBoxSizer *vbox_sizer;

//int width1, height1;
//int ix, iy, i, j, k;
//width1 = (width0 - 40)/3;
//height1 = (height0 - 40)/2;

 vbox_sizer = new wxBoxSizer( wxVERTICAL );

// Parameter Panel
 psc1_param_panel = new Psc1_ParamPanel((wxFrame *)m_ParamPanel,
                                        &Str0[iLang][0], NMAX_MESSAGES);

// 1 : vertically strechable
// wxEXPAND : horizontally strechable
 vbox_sizer->Add((wxFrame *)psc1_param_panel, 1, wxEXPAND);

 m_ParamPanel->SetSizerAndFit(vbox_sizer);

return;
}
/**********************************************************************
* TargetPanel setup
* Create Target Panel with Psc1_TargetPanel class (widget panel)
*
* SHOULD BE CALLED AFTER CREATING Psc1_Pisco_Panel since needs its pointer to stop its timers
*
***********************************************************************/
void PscFrame::TargetPanelSetup()
//int width0, int height0)
{
wxBoxSizer *vbox_sizer;

//int width1, height1;
//int ix, iy, i, j, k;
//width1 = (width0 - 40)/3;
//height1 = (height0 - 40)/2;

 vbox_sizer = new wxBoxSizer( wxVERTICAL );

// Target Panel
 psc1_target_panel = new Psc1_TargetPanel((wxFrame *)m_TargetPanel,
                                          psc1_pisco_panel,
                                          &Str0[iLang][0], NMAX_MESSAGES);

// 1 : vertically strechable
// wxEXPAND : horizontally strechable
 vbox_sizer->Add((wxFrame *)psc1_target_panel, 1, wxEXPAND);

 m_TargetPanel->SetSizerAndFit(vbox_sizer);

return;
}
/**********************************************************************
* PiscoPanel setup
* Create PiscoPanel with Psc1_PiscoPanel class (widget panel)
*
***********************************************************************/
void PscFrame::PiscoPanelSetup()
//int width0, int height0)
{
wxBoxSizer *vbox_sizer;

//int width1, height1;
//int ix, iy, i, j, k;
//width1 = (width0 - 40)/3;
//height1 = (height0 - 40)/2;

 vbox_sizer = new wxBoxSizer( wxVERTICAL );

// Open Logbook (BEFORE creating the pisco panel !)
 fp_logbook = NULL;
 OpenLogbook();

// Pisco Panel
 psc1_pisco_panel = new Psc1_PiscoPanel((wxFrame *)m_PiscoPanel, jlp_Logbook,
                                        fp_logbook, &Str0[iLang][0], NMAX_MESSAGES);

// 1 : vertically strechable
// wxEXPAND : horizontally strechable
 vbox_sizer->Add((wxFrame *)psc1_pisco_panel, 1, wxEXPAND);

 m_PiscoPanel->SetSizerAndFit(vbox_sizer);

return;
}
/**************************************************************************
*
***************************************************************************/
int PscFrame::SelectLanguageSetup()
{
JLP_Language_Dlg *LanguageDlg;
int status, i_lang;

  LanguageDlg = new JLP_Language_Dlg(this, wxT("Language Selection"));

// To avoid conflicts with modal dialog, stop the timers:
  status = LanguageDlg->ShowModal();

// Retrieve the object/orbit parameters:
  LanguageDlg->RetrieveData(&i_lang);

// Set private variable iLang if status is OK:
  if(status == 0 && i_lang >= 0 && i_lang < NLANG) {
   iLang = i_lang;
   }

delete LanguageDlg;

return(status);
}
/**********************************************************************
* Notebook page is changing
*
* 0 = Andor Panel
* 1 = Processing Panel
* 2 = Display Panel
*
***********************************************************************/
void PscFrame::OnPageChanging( wxBookCtrlEvent &event )
{
int status = 0;
wxString buffer, target_name0;
int selOld, selNew;
double temp0, press0, hygro0;
double alpha0, delta0, equin0;
int shift0, ra_offset0, rb_offset0, ra_sign0, rb_sign0, target_type0;

if(initialized != 1234) return;

selOld = event.GetOldSelection();
selNew = event.GetSelection();

 buffer = _T("");

/* DEBUG
 if(selOld == 0) {
    buffer.Printf(wxT("old sel=%d \n"), selOld);
 if(psc1_param_panel->ChangesDone() == true) buffer.Append(wxT("ChangesDone"));
 else buffer.Append(wxT("ChangesNotDone"));
 if(psc1_param_panel->ValidatedChanges() == true) buffer.Append(wxT("ValidatedChanges"));
 else buffer.Append(wxT("NonValidatedChanges"));
   wxMessageBox(buffer, _T("Pisco1"), wxOK);
 }
*/

// Check is changes have been done and not validated:
 if((selOld == 0) &&
    (psc1_param_panel->ChangesDone() == true) &&
     (psc1_param_panel->ValidatedChanges() == false)) {
   buffer = _T("Exit from Parameter selection: changes not validated.\n");
   status = -1;
 } else if((selOld == 2) &&
    (psc1_target_panel->ChangesDone() == true) &&
     (psc1_target_panel->ValidatedChanges() == false)) {
   buffer = _T("Exit from Target selection: changes not validated\n");
   status = -2;
 }


// Prompt the user if changes have not been validated (nor cancelled):
 if(status) {
   buffer.Append(_T("Warning: changes have not been validated, nor cancelled yet!\n"));
   buffer.Append(_T("\n Do you want to validate those changes ?\n"));
   if(wxMessageBox(buffer, _T("Pisco1"),
                 wxICON_QUESTION | wxYES_NO, this) == wxYES ) {
     if(selOld == 0) psc1_param_panel->ValidateChanges();
     else if (selOld == 2) psc1_target_panel->ValidateChanges();
   } else {
     if(selOld == 0) psc1_param_panel->CancelNonValidatedChanges();
     else if (selOld == 2) psc1_target_panel->CancelNonValidatedChanges();
   }
 }

   if(selOld == 0) {
     psc1_param_panel->GetAtmosphericParam(&temp0, &press0, &hygro0, &shift0,
                                           &ra_offset0, &rb_offset0, &ra_sign0,
                                           &rb_sign0);
     psc1_pisco_panel->LoadAtmosphericParam(temp0, press0, hygro0, shift0,
                                            ra_offset0, rb_offset0, ra_sign0,
                                            rb_sign0);
   } else if(selOld == 2) {
     psc1_target_panel->GetTargetParam(target_name0, &target_type0, &alpha0, &delta0, &equin0);
     psc1_pisco_panel->LoadTargetParam(target_name0, target_type0, alpha0, delta0, equin0);
   }

// Block page change
//   event.Veto();

// Allow page change:
   event.Skip();

return;
}
