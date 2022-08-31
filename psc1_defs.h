/**************************************************************************
* psc1_defs.h
* Definitions for Pisco1
*
* JLP
* Version 24/10/2015
**************************************************************************/
#ifndef psc1_defs_h_
#define psc1_defs_h_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// 5 languages: English, French, Italian, Spanish, German
#define NLANG 5
// Tranlated messages in file:
#define NMAX_MESSAGES 1024

// Number of wheels:
#define NWHEELS 9

#define RISLEY_TOLER 4
#define WHEEL_TOLER 15
/*
#define RA_NULLDISP 57
#define RB_NULLDISP 214
*/
#define TAVCONFI "TAVCONFI.DAT"
#define TAVPOSI "tavposi.pos"
#define LOGFILE "tav.log"

#define NCHOICES 32

typedef struct {
wxComboBox *combo;
wxString choices[NCHOICES];
wxString label;
int nchoices;
} JLP_ComboBox;


typedef struct {
double alpha_min;
double alpha_range;
double delta_min;
double delta_max;
double rho_min;
double rho_max;
double mag_min;
double mag_max;
} Binary_Profile;

typedef struct {
wxString Star_name;
wxString WDS_name;
wxString WDS2_name;
wxString Discover_name;
wxString Companion_name;
wxString Spectral_type;
double alpha;
double delta;
double rho;
double theta;
double mag;
double dmag;
double epoch;
} Binary_Parameters;

#endif
