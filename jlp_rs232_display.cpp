/************************************************************************
* "jlp_rs232_display.cpp"
* JLP_Rs232Display class
*
* JLP
* Version 07/09/2015
*************************************************************************/
#include "jlp_rs232_display.h"

BEGIN_EVENT_TABLE(JLP_Rs232Display, wxFrame)

// Buttons:
// EVT_BUTTON(ID_AD_START_PROCESSING, JLP_Rs232Display::OnStartProcessing)
 EVT_TIMER(ID_AD_COOLER_TIMER, JLP_Rs232Display::OnCoolerTimer)

END_EVENT_TABLE()

/*******************************************************************************
* Constructor from PscFrame:
*******************************************************************************/
JLP_Rs232Display::JLP_Rs232Display(JLP_AndorCam1 *jlp_cam0, ANDOR_SETTINGS Aset0,
                                   const int width0, const int height0)
                                   : wxFrame(NULL, wxID_ANY, wxT("Andor display"),
                                   wxPoint(-1, -1), wxSize(width0, height0),
  wxCAPTION | wxMAXIMIZE_BOX |wxMINIMIZE_BOX | wxSTAY_ON_TOP | wxRESIZE_BORDER
  | wxFRAME_TOOL_WINDOW)
{
 int timer_ms_delay;


// Transform coma into point for numbers:
 setlocale(LC_NUMERIC, "C");

// Smaller fonts (needed for Windows, to have a similar appearance as Linux)
// SetWindowVariant(wxWINDOW_VARIANT_SMALL);

 initialized = 0;

 jlp_cam1 = jlp_cam0;
 Copy_AndorSettings(Aset0, Aset1);

// nx0_ROI, ny0_ROI will be used for ROI : here they are initialized with dummy values
 nx0_ROI = Aset0.nx0;
 ny0_ROI = Aset0.ny0;

// Start the timer (delay = 2 seconds):
 timer_ms_delay = 2000;
 m_cooler_timer = new wxTimer(this, ID_AD_COOLER_TIMER);

 m_cooler_timer->Start(timer_ms_delay, wxTIMER_CONTINUOUS);

// To direct timer events to this class:
 m_cooler_timer->SetOwner(this, ID_AD_COOLER_TIMER);

// Setup panel
 DisplayPanelSetup(width0, height0);

 initialized = 1234;

// Load Dummy image to update nx0, ny0, etc
 LoadDummyImage();

return;
}
/**********************************************************************
* DisplayPanel setup
* Create DisplayPanel with image panels:
*
***********************************************************************/
void JLP_Rs232Display::DisplayPanelSetup(const int width0, const int height0)
{
wxBoxSizer *main_vtop_sizer, *hsizer0;
wxBoxSizer *hbox_sizer1, *vbox_sizer1;
int width1, height1;
int widths[2];

// Status bar:
// Create a status bar with two fields at the bottom:
 m_StatusBar = CreateStatusBar(2);
// First field has a variable length, second has a fixed length:
 widths[0] = -1;
 widths[1] = 200;
 SetStatusWidths( 2, widths );

 main_vtop_sizer = new wxBoxSizer( wxVERTICAL );
 hsizer0 = new wxBoxSizer( wxHORIZONTAL );

// Create static text
 Static_Temperature = new wxStaticText(this, wxID_ANY,
                               wxT("En cours..."));
 hsizer0->Add(Static_Temperature);

 main_vtop_sizer->Add(hsizer0, 0, wxALL, 10);

// Size of subpanel:
 width1 = (width0 - 40);
 height1 = (height0 - 80);

vbox_sizer1 = new wxBoxSizer( wxVERTICAL );
hbox_sizer1 = new wxBoxSizer( wxHORIZONTAL );

// New scrollwindow for image display (and NULL for JLP_wxLogbook):
    pChildVisu1 = new JLP_wxImagePanel(this, NULL, m_StatusBar,
                                         20, 20, width1, height1);
// wxEXPAND | xALL: horizontally strechable with borders all around
    vbox_sizer1->Add(pChildVisu1, 1, wxEXPAND | wxALL);
// 1: vertically strechable
    hbox_sizer1->Add(vbox_sizer1, 1, wxEXPAND);
//
  main_vtop_sizer->Add(hbox_sizer1, 1, wxEXPAND);

this->SetSizerAndFit(main_vtop_sizer);

return;
}
/*********************************************************************
* Display a dummy image to the screen
*
*********************************************************************/
void JLP_Rs232Display::LoadDummyImage()
{
int i, nx0, ny0;
double *dble_image0;

nx0 = Aset1.nx0 / Aset1.xbin;
ny0 = Aset1.ny0 / Aset1.ybin;
dble_image0 = new double[nx0 * ny0];
for(i = 0; i < nx0 * ny0; i++) dble_image0[i] = 0.;

LoadImage(dble_image0, nx0, ny0);

return;
}
/*********************************************************************
* Display an image to the screen
*
*********************************************************************/
void JLP_Rs232Display::LoadImage(double *dble_image0, int nx0, int ny0)
{
int index;

 pChildVisu1->LoadImage(dble_image0, nx0, ny0);

// Show ROI on screen:
 DisplayROI();

return;
}
/*********************************************************************
* Load New settings (mainly used to update ROI display)
*
*********************************************************************/
void JLP_Rs232Display::LoadNewSettings(ANDOR_SETTINGS Aset0, const int nx0_ROI_0,
                                       const int ny0_ROI_0)
{

// Copy current settings: Aset0) to Aset1:
 Copy_AndorSettings(Aset0, Aset1);

 nx0_ROI = nx0_ROI_0;
 ny0_ROI = ny0_ROI_0;

// Show ROI on screen:
 DisplayROI();

return;
}
/****************************************************************
* Handle timer events, for checking cooler temperature
* (called every 5000 milliseconds )
*****************************************************************/
void JLP_Rs232Display::OnCoolerTimer(wxTimerEvent &WXUNUSED(event))
{
int current_temp;
wxString buffer;

if(initialized != 1234) return;

 jlp_cam1->JLP_Andor_GetTemperature(&current_temp, buffer);

// Remove trailing blanks:
 buffer.Trim();
 Static_Temperature->SetLabel(buffer);

// Write to status bar:
/*
 wxFont font1;
 font1 = m_StatusBar->GetFont();
 font1.SetPointSize(8);
 font1.SetWeight(wxFONTWEIGHT_BOLD);
 m_StatusBar->SetFont(font1);
 m_StatusBar->SetStatusText(buffer, 0);
*/

// Show ROI on screen (useful to update center of image after streaming):
 DisplayROI();

return;
}
/****************************************************************
* Show ROI on screen:
*
*****************************************************************/
void JLP_Rs232Display::DisplayROI()
{
int status;
double xstart, ystart, xend, yend;

// Only valid if full frame:
if((Aset1.nx0 != Aset1.gblXPixels)
  || (Aset1.nx0 != Aset1.gblXPixels)) return;

 xstart = Aset1.xc0 - (nx0_ROI / 2);
 ystart = Aset1.yc0 - (ny0_ROI / 2);
 xend = xstart + nx0_ROI;
 yend = ystart + ny0_ROI;

 status = pChildVisu1->DrawRectangle(xstart, ystart, xend, yend);

/* DEBUG:
{
wxString buffer;
buffer.Printf(wxT("DisplayROI: xc0,yc0=%d %d x1,y1= %.1f,%.1f, x2,y2= %.1f,%.1f status = %d"),
              Aset1.xc0, Aset1.yc0, xstart, ystart, xend, yend, status);
wxLogError(buffer);
}
*/
return;
}
