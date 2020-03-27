/****************************************************************************
* Pisco1.cpp
* Application project for Pisco1.cpp (PscFrame class)
* Purpose: display and process FITS images obtained with PISCO and PISCO2
*
* JLP
* Version 05/09/2015
****************************************************************************/

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__) || defined(__WXMGL__) || defined(__WXX11__)
    #define USE_XPM
#endif

#ifdef USE_XPM
    #include "mondrian.xpm"
#endif

#include "Pisco1.h"
#include "psc1_frame.h"    // PscFrame class

// Create a new application object: (to be able to use wxGetApp())
IMPLEMENT_APP(Psc1_App)

/****************************************************************************
* Constructor:
****************************************************************************/
Psc1_App::Psc1_App()
{
}
/****************************************************************************
* Setup (called when opening application)
****************************************************************************/
bool Psc1_App::OnInit()
{
// Transform coma into point for numbers:
setlocale(LC_NUMERIC, "C");

// use standard command line handling:
    if ( !wxApp::OnInit() )
        return false;

// parse the cmd line
    int GdpWidth = 400, GdpHeight = 400;

    if ( argc == 3 )
    {
        wxSscanf(wxString(argv[1]), wxT("%d"), &GdpWidth);
        wxSscanf(wxString(argv[2]), wxT("%d"), &GdpHeight);
    }


#if wxUSE_HELP
    wxHelpProvider::Set( new wxSimpleHelpProvider );
#endif // wxUSE_HELP

// Create the main frame window
    PscFrame *frame = new PscFrame(_T("Pisco1"), GdpWidth, GdpHeight);

// Give it an icon
// The wxICON() macros loads an icon from a resource under Windows
// and uses an #included XPM image under GTK+ and Motif
#ifdef USE_XPM
    frame->SetIcon( wxICON(mondrian) );
#endif

    frame->Show(true);

    return true;
}
