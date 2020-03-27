/******************************************************************************
* jlp_rs232_dlg.h
* To select the RS232 serial port and settings
*
* Author:  JLP
* Version: 03/11/2015
******************************************************************************/
#ifndef jlp_rs232_dlg_h    // sentry
#define jlp_rs232_dlg_h

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/filename.h"

#include "psc1_defs.h"  // JLP_Combobox

// RS232 COM1, COM2, etc:
#define NPORTS_MAXI 4

/********************************************************************
* Class JLP_RS232_Dlg
*********************************************************************/

class JLP_RS232_Dlg: public wxDialog
{
public:

// Constructor:
   JLP_RS232_Dlg(wxFrame *parent, const wxString &title,
                 wxString *str_messg, const int n_messg);
// Destructor:
   ~JLP_RS232_Dlg(){};

   void PanelSetup();

// Handling events:
   void OnOKButton(wxCommandEvent &event);
   void OnCancelButton(wxCommandEvent &event);
   void OnRS232ComSelection(wxCommandEvent &event);
   void OnRS232BaudrateSelection(wxCommandEvent &event);

// Accessors:
   int RetrieveData(int *comport_nber0, int *baudrate0) {
       *comport_nber0 = comport_nber1;
       *baudrate0 = baudrate1;
       return(0);
       }

protected:
   bool DataIsOK() {
       if(comport_nber1 < 0 || comport_nber1 > NPORTS_MAXI) return(false);
       return(true);
       }

private:
   wxComboBox *PscCombo_rs232_comSel;
   JLP_ComboBox PscCmb_rs232_baudrate;
   int iports[NPORTS_MAXI];
   int comport_nber1, baudrate1, initialized;
   wxString *m_messg;

   DECLARE_EVENT_TABLE()
};

#endif               // EOF sentry
