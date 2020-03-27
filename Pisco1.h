/****************************************************************************
* Pisco1.h
* Application project for Pisco1.cpp (PscFrame class)
* Purpose: display and process FITS images obtained with PISCO and PISCO2
*
* JLP
* Version 05/09/2015
****************************************************************************/
#ifndef _andor_speck2_h_
#define _andor_speck2_h_

// For compilers that support precompilation, includes "wx/wx.h".
// #include "wx/wxprec.h"

#include "wx/wx.h"
#include <wx/app.h> // To declare wxGetApp()

//----------------------------------------------------------------------
// Psc1_App : the application object
//----------------------------------------------------------------------

class Psc1_App: public wxApp
{
public:
   Psc1_App();
   virtual ~Psc1_App(){};
   virtual bool OnInit();

};

#endif
