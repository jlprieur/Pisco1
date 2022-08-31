/****************************************************************************
* Name: psc1_target_catalogs.cpp
* Psc1_TargetPanel class
*
* JLP
* Version 20/11/2015
****************************************************************************/
#include "psc1_frame_id.h"
#include "psc1_target_panel.h"
#include "psc1_pisco_panel.h"
#include "tav_utils.h"   // convert_coord()

/****************************************************************************
* Open WDS double star catalog
****************************************************************************/
void Psc1_TargetPanel::OnOpenWdsCatalog(wxCommandEvent& event)
{
int status;
// Select name for output logbook file:
wxFileDialog openFileDialog(this, wxT("Open double star PISCO or WDS catalog"),
                            wxT(""), wxT(""),
                            wxT("Catalog files (*.txt)|*.txt"));

WdsCatalog_fname = wxT("");

// To avoid conflicts with modal dialog, stop the timers:
if((initialized == 1234) && (m_psc1_pisco_panel != NULL))
    m_psc1_pisco_panel->PauseMyTimers();

status = openFileDialog.ShowModal();

// Restart the timers:
if((initialized == 1234) && (m_psc1_pisco_panel != NULL))
    m_psc1_pisco_panel->ResumeMyTimers();

if (status == wxID_CANCEL) return;

WdsCatalog_fname = openFileDialog.GetFilename();

// Update catalog name:
 PscStatic_WdsCatalog->SetLabel(WdsCatalog_fname);

// Get first object according to selection:
 FindNextObjectInWdsCatalog();

return;
}
