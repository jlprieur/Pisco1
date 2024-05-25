/************************************************************************
* "jlp_rs232_display.h"
* JLP_Rs232Display class
*
* JLP
* Version 07/09/2015
*************************************************************************/
#ifndef jlp_rs232_display_h_
#define jlp_rs232_display_h_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/tglbtn.h"
#include "wx/bookctrl.h"
#include "wx/imaglist.h"
#include "wx/cshelp.h"
#include "wx/thread.h"
#include "wx/event.h"  // wxThreadEvent

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif

#include "jlp_wx_ipanel.h"     // JLP_wxImagePanel
#include "jlp_igdev_wxwid.h"   // JLP_iGDev_wxWID
#include "psc1_frame.h"    // JLP_AndorCam1

class JLP_Rs232Display : public wxFrame
{

public:

// In "jlp_rs232_display.cpp":

  JLP_Rs232Display(JLP_AndorCam1 *jlp_cam0,
                   const int width0, const int height0);
  ~JLP_Rs232Display(){
   m_cooler_timer->Stop();
   return;
   };

  void LoadNewSettings(ANDOR_SETTINGS Aset const int nx0_ROI_0,
                       const int ny0_ROI_0);
  void LoadImage(double *dble_image0, int nx0, int ny0);
  void LoadDummyImage();
  void OnStartProcessing(wxCommandEvent& event){};
  void DisplayROI();

// Timer:
  void OnCoolerTimer(wxTimerEvent& event);

protected:

  void DisplayPanelSetup(const int width0, const int height0);

private:
  wxStatusBar *m_StatusBar;
  JLP_wxImagePanel *pChildVisu1;
  JLP_AndorCam1 *jlp_cam1;
  ANDOR_SETTINGS Aset1;
  int initialized, nx0_ROI, ny0_ROI;
  wxTimer *m_cooler_timer;
  wxStaticText *Static_Temperature;

  DECLARE_EVENT_TABLE()
};

#endif // EOF sentry
