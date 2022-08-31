/****************************************************************************
* Name: psc1_frame.h (PscFrame class for Pisco1)
* Purpose: display and process FITS images obtained with PISCO and PISCO2
*
* JLP
* Version 07/01/2015
****************************************************************************/
#ifndef _psc1_frame_
#define _psc1_frame_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/tglbtn.h"
#include "wx/bookctrl.h"
#include "wx/imaglist.h"
#include "wx/cshelp.h"

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif

#if USE_XPM_BITMAPS
    #include "bitmaps/new.xpm"
    #include "bitmaps/open.xpm"
    #include "bitmaps/save.xpm"
    #include "bitmaps/copy.xpm"
    #include "bitmaps/cut.xpm"
    #include "bitmaps/preview.xpm"  // paste XPM
    #include "bitmaps/print.xpm"
    #include "bitmaps/help.xpm"
#endif // USE_XPM_BITMAPS


#include "jlp_gdev_wxwid.h"     // JLP_iGDev_wxWID
#include "jlp_wxlogbook.h"       // JLP_wxLogbook class
#include "psc1_param_panel.h"    // Psc1_ParamPanel class
#include "psc1_target_panel.h"   // Psc1_TargetPanel class
#include "psc1_pisco_panel.h"    // Psc1_PiscoPanel class

#include "psc1_defs.h"           // NWHEELS,...

//----------------------------------------------------------------------
// class definitions
//----------------------------------------------------------------------

class PscFrame: public wxFrame
{
public:

// In "psc1_main.cpp":
    PscFrame(const wxChar *title, int width0, int height0);
    ~PscFrame();
    bool MyClose();

    void Main_Init();
    void NotebookSetup(int width0, int height0);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
    void SetText_to_StatusBar(wxString text, const int icol);
    int LoadNewPointFromCursor(double xx, double yy, double value);
    void ParamPanelSetup();
    void TargetPanelSetup();
    void PiscoPanelSetup();
    void OnPageChanging(wxBookCtrlEvent &event);
    int SelectLanguageSetup();
    void CDisplayClockOnStatusBar();

#if wxUSE_TOOLTIPS
    void OnToggleTooltips(wxCommandEvent& event);
#endif // wxUSE_TOOLTIPS

    void OnContextHelp(wxCommandEvent& event);

// OnClose
    void OnClose(wxCloseEvent &event);

// In "psc1_frame_menu.cpp":
    void Gdp_SetupMenu();

// in "psc1_frame_logbook.cpp":
    void OnOpenLogbook(wxCommandEvent& event);
    int OpenLogbook();
    void OnViewLogbook(wxCommandEvent& event);
    void OnSaveLogbook(wxCommandEvent& event);
    void OnAddCommentsToLogbook(wxCommandEvent& event);
    int SaveLogbook(wxString save_filename);
    void ShowLogbook();
    void HideLogbook();
    int WriteToLogbook(wxString str1, bool SaveToFile);
    int AddNewPointToLogbook(double xx, double yy, double value);

// In psc1_language.cpp
    int LoadMenuMessages();

protected:

// Clock on StatusBar:
    void CDisplay_OnUpdateClock(wxTimerEvent&) {
        CDisplay_UpdateClock();
    }

    void CDisplay_UpdateClock() {
      wxDateTime wxdt;
      wxString wxbuff;
      if(initialized == 1234) {
        UT_shift = psc1_param_panel->GetUTShift();
        wxdt = wxDateTime::Now().Subtract(wxTimeSpan(-UT_shift));
        wxbuff = wxT("UT ") + wxdt.FormatTime();
        SetText_to_StatusBar(wxbuff, 2);
        }
    }

private:
  wxStatusBar *m_StatusBar;
  int initialized;

// Clock on Menubar:
  wxTimer m_clockTimer;
  int UT_shift;

// Messages in 5 languages:
  int iLang;
  wxString Str0[NLANG][NMAX_MESSAGES];

// Timers
  int risley_delay_msec;
  wxTimer *risley_timer;

// Toolbar:
  wxToolBar *m_toolbar;
  wxStaticText *DispStatic1;

// Menus:
  wxMenuBar *menu_bar;
  wxMenu *menuFile, *menuFileSave, *menuLog, *menuHelp;
  wxBoxSizer  *m_topsizer;

// Notebook:
  wxBookCtrl *m_notebook;
  int i_NotebookPage;

// ParamPanel
  wxPanel *m_ParamPanel;
  Psc1_ParamPanel *psc1_param_panel;

// TargetPanel
  wxPanel *m_TargetPanel;
  Psc1_TargetPanel *psc1_target_panel;

// PiscoPanel
  wxPanel *m_PiscoPanel;
  Psc1_PiscoPanel *psc1_pisco_panel;

// Logbook:
  wxString    m_Logbook;
  FILE *fp_logbook;
  JLP_wxLogbook *jlp_Logbook;

  DECLARE_EVENT_TABLE()
};

#endif
