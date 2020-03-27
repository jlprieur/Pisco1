/******************************************************************************
* Name:        gdp_frame_logbook.cpp (PscFrame class)
* same as  "pisco/Gdpisco/gp_frame_logbook.cpp" (PscFrame class)
*          but without "int BinariesSaveMeasurements();"
*
* Purpose:     Logbook utilities
* Author:      JLP
* Version:     03/08/2015
******************************************************************************/
#include "psc1_frame.h"
#include "psc1_frame_id.h"  // Menu identifiers

/* Declared in psc1_frame.h
void OnViewLogbook(wxCommandEvent& event)
void OnSaveLogbook(wxCommandEvent& WXUNUSED(event))
void OnOffsetCorrection(wxCommandEvent& event);
void OnFlatFieldCorrection(wxCommandEvent& event);
int  SaveLogbook(wxString save_filename)
void ShowLogbook()
void HideLogbook()
void ClearLogbook()
void CleanLogbook()
int  WriteToLogbook(wxString str1, bool SaveToFile);
int  AddNewPointToLogbook(int xx, int yy)
*/

/************************************************************************
* Showing/hiding logbook panel
************************************************************************/
void PscFrame::OnViewLogbook(wxCommandEvent& event)
{
  switch (event.GetId())
  {
   case ID_LOGBOOK_SHOW:
     ShowLogbook();
     break;
   case ID_LOGBOOK_HIDE:
     HideLogbook();
     break;
   }
}
/************************************************************************
* Save useful content of logbook to file
************************************************************************/
void PscFrame::OnSaveLogbook(wxCommandEvent& WXUNUSED(event))
{
int status;
wxString save_filename;

// Select name for output logbook file:
wxFileDialog saveFileDialog(this, wxT("Save logbook to file"), wxT(""), wxT(""),
               wxT("Logbook files (*.log;*.txt)|*.log;*.txt"),
               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

// To avoid conflicts with modal dialog, stop the timers:
if((initialized == 1234) && (psc1_pisco_panel != NULL))
    psc1_pisco_panel->PauseMyTimers();

status = saveFileDialog.ShowModal();

// Restart the timers:
if((initialized == 1234) && (psc1_pisco_panel != NULL))
    psc1_pisco_panel->ResumeMyTimers();

if (status == wxID_CANCEL) return;

save_filename = saveFileDialog.GetFilename();

SaveLogbook(save_filename);

return;
}
/*******************************************************************
* Add comments to the logbook
********************************************************************/
void PscFrame::OnAddCommentsToLogbook(wxCommandEvent& event)
{
if(initialized != 1234) return;

if(psc1_pisco_panel != NULL) psc1_pisco_panel->AddCommentsToLogbook();

return;
}
/************************************************************************
* Save useful content of logbook to file
* Input:
* save_filename: wxString whose value is set in gdp_frame_menu.cpp
************************************************************************/
int PscFrame::SaveLogbook(wxString save_filename)
{
int status = -2;

 if(initialized == 1234) status = jlp_Logbook->SaveLogbook(save_filename);

return(status);
}
/************************************************************************
* Showing logbook panel
************************************************************************/
void PscFrame::ShowLogbook()
{
 if(initialized != 1234) return;

 m_topsizer->Show(jlp_Logbook);
 m_topsizer->Layout();
}
/************************************************************************
* Hiding logbook panel
************************************************************************/
void PscFrame::HideLogbook()
{
 if(initialized != 1234) return;

 m_topsizer->Hide(jlp_Logbook);
 m_topsizer->Layout();
}
/************************************************************************
* Write to logbook
*************************************************************************/
int  PscFrame::WriteToLogbook(wxString str1, bool SaveToFile)
{
int status = -1;

 if(initialized == 1234) {
  if(fp_logbook != NULL) fprintf(fp_logbook, "%s\n", (const char *)str1.mb_str());
  if(jlp_Logbook != NULL) status = jlp_Logbook->WriteToLogbook(str1, SaveToFile);
 }

return(status);
}
/************************************************************************
* Open logbook file
************************************************************************/
void PscFrame::OnOpenLogbook(wxCommandEvent& WXUNUSED(event))
{
OpenLogbook();
return;
}
/************************************************************************
* Open logbook file
************************************************************************/
int PscFrame::OpenLogbook()
{
wxString filename1;
char filename0[128];
int status;

if(fp_logbook) fclose(fp_logbook);
fp_logbook = NULL;

// Select name for output logbook file:
wxFileDialog openFileDialog(this, wxT("Open logbook file"), wxT(""), wxT(""),
               wxT("Logbook files (*.log;*.txt)|*.log;*.txt"),
               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

// To avoid conflicts with modal dialog, stop the timers:
if((initialized == 1234) && (psc1_pisco_panel != NULL))
    psc1_pisco_panel->PauseMyTimers();

status = openFileDialog.ShowModal();

// Restart the timers:
if((initialized == 1234) && (psc1_pisco_panel != NULL))
    psc1_pisco_panel->ResumeMyTimers();

if (status == wxID_CANCEL) return(-1);

filename1 = openFileDialog.GetFilename();

strcpy(filename0, (const char *)filename1.mb_str());
fp_logbook = fopen(filename0, "w");

return(0);
}
/************************************************************************
* Add a new point to logbook
*************************************************************************/
int PscFrame::AddNewPointToLogbook(double xx, double yy, double value)
{
int status = -1;
wxString str1;

 if(initialized == 1234) {
  str1.Printf("%.2f %.2f %.4g\n", xx, yy, value);
  WriteToLogbook(str1, true);
 }

return(status);
}
