/**************************************************************
* tav2.cpp
* Program to control PISCO, the speckle camera of OMP
* Written from OWLCVT 11/09/94
*
* Language: C++
*
* JLP
* Version of 08/09/2011
***************************************************************/
#include "owl\compat.h"
#include <owl\owlall.h>
#include <owl\window.h>
#include <owl\static.h>
#include <owl\button.h>
#include <owl\listbox.h>
#include <owl\combobox.h>
#include <owl\dialog.h>
#include <owl\radiobut.h>
#include <owl\groupbox.h>
#include <owl\edit.h>

#include <stdio.h>
#include <math.h>
#include <string.h>     // for strcpy and strcat
#include <stdlib.h>     // for atoi
#include <ctype.h>      // for isdigit and isalpha

#include "tav.h"
#include "tav2.h"
#include "tav1.rh"

/* New parameter, ZOOM1, (November 1995) to adjust the screen size:
 You can set it to 1.0 to go back to previous settings...
*/
// Windows 98/800x600: ZOOM1 = !.0 HEIGHT1=20
// Windows XP/1024x860: ZOOM1 = !.5 HEIGHT1=25
#define ZOOM1 1.5
#define HEIGHT1 25
#define WIDTH1 200

#define XSTART1 20
#define YSTART1 20

#define RISLEY_TOLER 4
#define WHEEL_TOLER 15

/* Size of buffer for current status (from RS232: "?") */
#define BUFF_LEN1 580
static char error_message1[BUFF_LEN1], answer1[BUFF_LEN1];
static char rs232_wheel_names[240], rs232_wheel_is_there[240];
static int Wheel_To_Update;

static char debug_string[200];

/*--------------------------------------------------
* TTestWindow implementations:
*--------------------------------------------------*/

TTestWindow::TTestWindow(TWindow * AParent, LPSTR ATitle) :
  TFrameWindow(AParent, ATitle)
{
register int k;

  bad_initialization = 0;
  exposure_on = 0;
  strcpy(ADC_text1, "");

/* To know if RS232 window has been created or not... */
  pisco_initialized = 0;
  rs232win_not_created = 1;
  rs232link_is_down = 1;
  no_rs232link_wanted = 0;

/* Initialize the number of the wheel to be updated */
  Wheel_To_Update = -1;

/* Initialize wheel status: */
   for(k = 0; k < NWHEELS; k++) wheel_is_there[k] = 0;
   ra_is_there = 0;
   rb_is_there = 0;

/* Initializes idle index (i.e. index for clock output and parameters update)*/
   idle_index = 0;

/* Initializes logbook file pointer to NULL: */
   fp_logbook = NULL;

/* Automatic Risley correction: */
   auto_risley_on = 1;

/* Draw ComboBox and buttons: */
     SetupTavWindow();
}
/**************************************************************************
* Initialization, first setup of Combo menus
*
***************************************************************************/
void TTestWindow::SetupWindow()
{
char message1[100];

int status;
  TFrameWindow::SetupWindow();
    status = InitRS232Link();
// Return status = -4 if user wants to exit
// But cannot exit at this stage since TTestWindow setup is not finished yet!
  if(status == -4) {
    bad_initialization = 1;
    }
  else {
     InitTavParameters();
     InitPISCO();
     }

return;
}
/*******************************************************************
* Main window setup with ComboBox and Buttons
********************************************************************/
int TTestWindow::SetupTavWindow()
{
  char buffer[80], message1[80];
  int x0, y0, x1, y1, xoffset, yoffset, id_combo;
  int x_width, y_width;
  double utime1;
  register int k;
  TDC *MyTDC;

/*****************************************************************/
/* First column: wheel status display: */
/*****************************************************************/

/* Width and height of dialog subwindows: */
  yoffset = 20 + 2 * HEIGHT1;

  id_combo = ID_COMBO_WHEEL;

  x1 = XSTART1;
  y1 = YSTART1 + HEIGHT1 * 0.2;

  for(k = 0; k < NWHEELS; k++)
    {
/* Open new subwindow */
      Combo_wheel[k] = new TComboBox(this, id_combo, x1, y1,
                         WIDTH1, 20*HEIGHT1,
                         CBS_DROPDOWNLIST, 0);
/* New settings: */
      y1 += yoffset;
      id_combo++;
    }

/*******************************************************************
* Write labels on top of subwindows:
* (Read labels in tavconfig.dat file)
*/
  if(LoadLabels1(message1))
    {
    ::MessageBox(HWindow, message1, "LoadLabels1 fatal error", MB_OK);
/* Exit: */
    return(-1);
    }

  x1 = XSTART1;
  y1 = YSTART1 - 20;
  for(k = 0; k < NWHEELS; k++)
   {
/* Write new label on top: */
     new TStatic(this, -1, wheel_title[k], x1, y1, WIDTH1, HEIGHT1, 0);
     y1 += yoffset;
    }

/* Button to get new values of the wheel positions: */
  x1 -= 5 * ZOOM1;

#if LANGUAGE == FRENCH
  new TButton(this, ID_UPDATE_WHEELS, "Vérif. des positions",
                              x1, y1, WIDTH1, HEIGHT1, FALSE);
#elif LANGUAGE == ITALIAN
  new TButton(this, ID_UPDATE_WHEELS, "Verif. delle posizioni",
                              x1, y1, WIDTH1, HEIGHT1, FALSE);
#else
  new TButton(this, ID_UPDATE_WHEELS, "Position checking",
                              x1, y1, WIDTH1, HEIGHT1, FALSE);
#endif

/*********************************************************************/
/* Second column: Lamp and Risley prisms */
/*********************************************************************/
  x1 =  XSTART1 + 50 * ZOOM1 + WIDTH1;
  y1 = YSTART1;

/******************** Lamp box: ON/OFF: */
  lamp_is_on = 0;
  x_width = WIDTH1 * 0.8;
#if LANGUAGE == FRENCH
  LampButton = new TButton(this, ID_BUTTON_LAMP,
             "Lampe éteinte",
              x1, y1, x_width, HEIGHT1, TRUE);
#elif LANGUAGE == ITALIAN
  LampButton = new TButton(this, ID_BUTTON_LAMP,
             "Lampada spenta",
              x1, y1, x_width, HEIGHT1, TRUE);
#else
  LampButton = new TButton(this, ID_BUTTON_LAMP,
             "Lamp is OFF",
              x1, y1, x_width, HEIGHT1, TRUE);
#endif

// Handle RA and RB values
  x1 =  XSTART1 + 50 * ZOOM1 + WIDTH1;
  y1 += (30 + 15 * ZOOM1);
  xoffset = 30 * ZOOM1;
  yoffset = HEIGHT1 + 20;
  ra_position = 0;
  x_width = WIDTH1 * 0.8;
  sprintf(buffer,"Risley RA: %04d",ra_position);
  RAStatus = new TStatic(this, ID_RA, buffer, x1 + 15, y1, x_width, HEIGHT1, 18);
  y1 += yoffset * 0.8;
  x1+= xoffset;
  ra_computed = RA_NULLDISP;
  sprintf(buffer,"%04d",ra_computed);
  EditRA = new TEdit(this, ID_RISLEY_RA, buffer, x1, y1, 40 * ZOOM1, HEIGHT1,
                     MAXRISLEN, FALSE);
  y1 += yoffset * 0.8;
  x1-= xoffset;
#if LANGUAGE == FRENCH
  new TButton(this, ID_BUTTON_RA, "Validez RA", x1, y1, x_width, HEIGHT1, FALSE);
#elif LANGUAGE == ITALIAN
  new TButton(this, ID_BUTTON_RA, "Convalidare RA", x1, y1, x_width, HEIGHT1, FALSE);
#else
  new TButton(this, ID_BUTTON_RA, "Valid RA", x1, y1, x_width, HEIGHT1, FALSE);
#endif
  y1 += yoffset;
  rb_position = 0;
  sprintf(buffer,"Risley RB: %04d",rb_position);
  RBStatus = new TStatic(this, ID_RB, buffer, x1 + 15, y1, x_width, HEIGHT1, 18);
  y1 += yoffset * 0.8;
  x1 += xoffset;
  rb_computed = RB_NULLDISP;
  sprintf(buffer,"%04d",rb_computed);
  EditRB = new TEdit(this, ID_RISLEY_RB, buffer, x1, y1, 40 * ZOOM1, HEIGHT1,
                     MAXRISLEN, FALSE);
  y1 += yoffset;
  x1 -= xoffset;
#if LANGUAGE == FRENCH
  new TButton(this, ID_BUTTON_RB, "Validez RB", x1, y1, x_width, HEIGHT1, FALSE);
#elif LANGUAGE == ITALIAN
  new TButton(this, ID_BUTTON_RB, "Convalidare RB", x1, y1, x_width, HEIGHT1, FALSE);
#else
  new TButton(this, ID_BUTTON_RB, "Valid RB", x1, y1, x_width, HEIGHT1, FALSE);
#endif
  EnableKBHandler();

/**************************** Automatic correction ON/OFF */
  x1 =  XSTART1 + 50 * ZOOM1 + WIDTH1 * 0.8 ;
  y1 += yoffset;
  x_width = 1.2 * WIDTH1; y_width = 6 * HEIGHT1;
#if LANGUAGE == FRENCH
  GroupBox_Risley = new TGroupBox(this, ID_GROUPBOX_RISLEY,
                       "Correction de disper.", x1, y1, x_width, y_width);
#elif LANGUAGE == ITALIAN
  GroupBox_Risley = new TGroupBox(this, ID_GROUPBOX_RISLEY,
                       "Correzione della disper.", x1, y1, x_width, y_width);
#else
  GroupBox_Risley = new TGroupBox(this, ID_GROUPBOX_RISLEY,
                       "Disper. correction:", x1, y1, x_width, y_width);
#endif
  x1 += 0.2 * WIDTH1;
  y1 += yoffset * 0.8;
  x_width = WIDTH1 * 0.8;
  y_width = HEIGHT1 * 0.8;
#if LANGUAGE == FRENCH
  RTButtonON = new TRadioButton(this, ID_RBUTTON_ON, "Automatique",
    x1, y1, x_width, y_width, GroupBox_Risley);
#elif LANGUAGE == ITALIAN
  RTButtonON = new TRadioButton(this, ID_RBUTTON_ON, "Automatica",
    x1, y1, x_width, y_width, GroupBox_Risley);
#else
  RTButtonON = new TRadioButton(this, ID_RBUTTON_ON, "Automatic",
    x1, y1, x_width, y_width, GroupBox_Risley);
#endif
  y1 += yoffset * 0.8;

#if LANGUAGE == FRENCH
  RTButtonOFF = new TRadioButton(this, ID_RBUTTON_OFF, "Interactive",
    x1, y1, x_width, y_width, GroupBox_Risley);
#elif LANGUAGE == ITALIAN
  RTButtonOFF = new TRadioButton(this, ID_RBUTTON_OFF, "Interattiva",
    x1, y1, x_width, y_width, GroupBox_Risley);
#else
  RTButtonOFF = new TRadioButton(this, ID_RBUTTON_OFF, "Interactive",
    x1, y1, x_width, y_width, GroupBox_Risley);
#endif

/* Button to get new values of RA and RB for dispersion correction: */
 x_width = WIDTH1;
 x1 -= 0.1 * WIDTH1;
 y1 += yoffset * 0.8;

#if LANGUAGE == FRENCH
  new TButton(this, ID_UPDATE_RISLEY, "Nouvelles valeurs",
                              x1, y1, x_width, HEIGHT1, FALSE);
#elif LANGUAGE == ITALIAN
  new TButton(this, ID_UPDATE_RISLEY, "Nuovi valori",
                              x1, y1, x_width, HEIGHT1, FALSE);
#else
  new TButton(this, ID_UPDATE_RISLEY, "New values",
                              x1, y1, x_width, HEIGHT1, FALSE);
#endif

/*********************************************************************/
/* Third column: Status display */
/*********************************************************************/
 x0 = XSTART1 + 80 * ZOOM1 + 2 * WIDTH1;
 y0 = YSTART1 - 10;

/* Command Status: */
 x_width = WIDTH1 * 1.2;
 y_width = 2. * HEIGHT1;
#if LANGUAGE == FRENCH
  new TGroupBox(this, -1, "Commande RS232:", x0, y0, x_width, y_width);
#elif LANGUAGE == ITALIAN
  new TGroupBox(this, -1, "Comando RS232:", x0, y0, x_width, y_width);
#else
  new TGroupBox(this, -1, "RS232 command:", x0, y0, x_width, y_width):
#endif
  x1 = x0 + WIDTH1 * 0.2;
  y1 = y0 + HEIGHT1;
  CommandStatus = new TStatic(this, ID_COMMAND, " ",
                    x1, y1, WIDTH1 * 0.8, HEIGHT1 * 0.8, MAXCOMMANDLEN);

/******************** Exposure box: Start/End: */
  y1 += 35;
  exposure_on = 0;
  x_width = WIDTH1 * 1.2; y_width = HEIGHT1 * 1.5;
#if LANGUAGE == FRENCH
  ExposureButton = new TButton(this, ID_BUTTON_EXPOSURE,
                                 "Lancer une nouvelle pose",
              x0, y1, x_width, y_width, TRUE);
#elif LANGUAGE == ITALIAN
  ExposureButton = new TButton(this, ID_BUTTON_EXPOSURE,
                                 "Lanciare una nuova posa",
              x0, y1, x_width, y_width, TRUE);
#else
  ExposureButton = new TButton(this, ID_BUTTON_EXPOSURE,
             "Start a new exposure",
              x0, y1, x_width, y_width, TRUE);
#endif

/**************** General status: */
  y1 += HEIGHT1 * 2;
  x_width = WIDTH1 * 2.0;
  y_width = 14 * HEIGHT1;
  xoffset = 5;
  yoffset = 3 + HEIGHT1;
#if LANGUAGE == FRENCH
  GroupBox_Status = new TGroupBox(this, -1, "Tableau de bord",
                                  x0, y1, x_width, y_width);
#elif LANGUAGE == ITALIAN
  GroupBox_Status = new TGroupBox(this, -1, "Quadro informativo",
                                  x0, y1, x_width, y_width);
#else
  GroupBox_Status = new TGroupBox(this, -1, "General status",
                                  x0, y1, x_width, y_width);
#endif
// Inefficient: ....  GroupBox_Status->Attr.Style |= SS_CENTER;

/* Object: */
  y1 += yoffset + 2;
  x1 = x0 + 20;
// Fixed part:
  x_width = WIDTH1 * 0.6;
#if LANGUAGE == FRENCH
  new TStatic(this, -1, "Objet :", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  new TStatic(this, -1, "Oggetto :", x1, y1, x_width, HEIGHT1, 20);
#else
  new TStatic(this, -1, "Target: ", x1, y1, x_width, HEIGHT1, 20);
#endif
// Part that will change according to the target name:
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.2;
  Target1 = new TStatic(this, ID_TARGET, " ", x1, y1, x_width, HEIGHT1, 25);
//  Target1->Attr.Style |= SS_CENTER;
  Target1->Attr.Style |= SS_LEFT;

/* Right Ascension: */
  y1 += yoffset + 2;
  x1 = x0 + 20;
// Fixed part:
  x_width = WIDTH1 * 0.4;
  new TStatic(this, -1, "R.A.  :", x1, y1, x_width, HEIGHT1, 20);
// Part that will change according to R.A. value:
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.4;
  RightAsc1 = new TStatic(this, ID_RIGHTASCENSION, " ", x1, y1,
                          x_width, HEIGHT1, 25);
  RightAsc1->Attr.Style |= SS_RIGHT;

/* Declination: */
  y1 += yoffset + 2;
  x1 = x0 + 20;
// Fixed part:
  x_width = WIDTH1 * 0.4;
  new TStatic(this, -1, "Delta :", x1, y1, x_width, HEIGHT1, 20);
// Part that will change according to Delta value:
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.4;
  Delta1 = new TStatic(this, ID_DECLINATION, " ", x1, y1, x_width, HEIGHT1, 25);
  Delta1->Attr.Style |= SS_RIGHT;

/* Universal time: */
  y1 += yoffset + 2;
  x1 = x0 + 20;
  x_width = WIDTH1 * 0.8;
#if LANGUAGE == FRENCH
  new TStatic(this, -1, "Heure (T.U.) :", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  new TStatic(this, -1, "Ora (T.U.) :", x1, y1, x_width, HEIGHT1, 20);
#else
  new TStatic(this, -1, "Time (U.T.):", x1, y1, x_width, HEIGHT1, 20);
#endif
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.0;
  if(jlp_utime(buffer,&utime1)) strcpy(buffer,"UT/Error.");
  Clock1 = new TStatic(this, ID_CLOCK, buffer, x1, y1, x_width, HEIGHT1, 14);
  Clock1->Attr.Style |= SS_RIGHT;

/* Local sidereal time: */
  y1 += yoffset;
  x1 = x0 + 20;
  x_width = WIDTH1 * 0.8;
#if LANGUAGE == FRENCH
  new TStatic(this, -1, "Heure sidér. locale:", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  new TStatic(this, -1, "Ora sider. locale:", x1, y1, x_width, HEIGHT1, 20);
#else
  new TStatic(this, -1, "Local sider. time:", x1, y1, x_width, HEIGHT1, 20);
#endif
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.0;
  SiderTime1 = new TStatic(this, ID_SIDERTIME, " ", x1, y1, x_width, HEIGHT1, 14);
  SiderTime1->Attr.Style |= SS_RIGHT;

/* Hour angle: */
  y1 += yoffset;
  x1 = x0 + 20;
  x_width = WIDTH1 * 0.8;
#if LANGUAGE == FRENCH
  new TStatic(this, -1, "Angle horaire:", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  new TStatic(this, -1, "Angolo orario:", x1, y1, x_width, HEIGHT1, 20);
#else
  new TStatic(this, -1, "Hour angle:", x1, y1, x_width, HEIGHT1, 20);
#endif
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.0;
  HourAngle1 = new TStatic(this, ID_HOURANGLE, " ",
                            x1, y1, x_width, HEIGHT1, 20);
  HourAngle1->Attr.Style |= SS_RIGHT;

/* Elevation: */
  y1 += yoffset;
  x1 = x0 + 20;
  x_width = WIDTH1 * 0.8;
#if LANGUAGE == FRENCH
  new TStatic(this, -1, "Hauteur (deg):", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  new TStatic(this, -1, "Altezza (gradi):", x1, y1, x_width, HEIGHT1, 20);
#else
  new TStatic(this, -1, "Elevation (deg): ", x1, y1, x_width, HEIGHT1, 20);
#endif
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.0;
  Elevation1 = new TStatic(this, ID_ELEVATION, "-1", x1, y1, x_width, HEIGHT1, 20);
  Elevation1->Attr.Style |= SS_RIGHT;

/* Airmass: */
  y1 += yoffset;
  x1 = x0 + 20;
  x_width = WIDTH1 * 0.8;
#if LANGUAGE == FRENCH
  new TStatic(this, -1, "Masse d'air :", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  new TStatic(this, -1, "Massa d'aria :", x1, y1, x_width, HEIGHT1, 20);
#else
  new TStatic(this, -1, "Airmass: ", x1, y1, x_width, HEIGHT1, 20);
#endif
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.0;
  Airmass1 = new TStatic(this, ID_AIRMASS, "-1", x1, y1, x_width, HEIGHT1, 20);
  Airmass1->Attr.Style |= SS_RIGHT;

/* Exposure time: */
  y1 += yoffset; x1 = x0 + 20;
  x_width = WIDTH1 * 0.8;
#if LANGUAGE == FRENCH
  new TStatic(this, -1, "Temps de pose:", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  new TStatic(this, -1, "Tempo di posa:", x1, y1, x_width, HEIGHT1, 20);
#else
  new TStatic(this, -1, "Expo. time:", x1, y1, x_width, HEIGHT1, 20);
#endif
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.0;
  ExpoTime1 = new TStatic(this, ID_EXPOTIME, " ", x1, y1, x_width, HEIGHT1, 20);
  ExpoTime1->Attr.Style |= SS_RIGHT;

/* Filter: */
  y1 += yoffset; x1 = x0 + 20;
  x_width = WIDTH1 * 0.8;
#if LANGUAGE == FRENCH
  new TStatic(this, -1, "Filtre :", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  new TStatic(this, -1, "Filtro :", x1, y1, x_width, HEIGHT1, 20);
#else
  new TStatic(this, -1, "Filter:", x1, y1, x_width, HEIGHT1, 20);
#endif
  x1 += x_width + xoffset;
  x_width = WIDTH1 * 1.0;
#if LANGUAGE == FRENCH
  Filter1 = new TStatic(this, ID_FILTER, "Aucun filtre", x1, y1, x_width, HEIGHT1, 20);
#elif LANGUAGE == ITALIAN
  Filter1 = new TStatic(this, ID_FILTER, "Alcun filtro", x1, y1, x_width, HEIGHT1, 20);
#else
  Filter1 = new TStatic(this, ID_FILTER, "No filter", x1, y1, x_width, HEIGHT1, 20);
#endif
  Filter1->Attr.Style |= SS_RIGHT;

/* ADC text: */
  y1 += yoffset; x1 = x0 + 20;
  x_width = WIDTH1 * 2.0;
  ADC_Text1 = new TStatic(this, ID_ADC_TEXT, " ", x1, y1, x_width, HEIGHT1, 20);
  ADC_Text1->Attr.Style |= SS_RIGHT;

//////////////////////////////////////////////////////////////////
// September 2007
// Marco asked for a special flag when white light is used:
/******************** White light: */
  x1 = XSTART1 + 130 * ZOOM1 + 3 * WIDTH1;
  y1 = YSTART1;
  x_width = WIDTH1;
  y_width = 2 * HEIGHT1;
#if LANGUAGE == FRENCH
  strcpy(WhiteLightText, "ATTENTION: LUMIERE BLANCHE!");
#elif LANGUAGE == ITALIAN
  strcpy(WhiteLightText, "ATTENZIONE: LUCE BIANCA!");
#else
  strcpy(WhiteLightText, "WARNING: WHITE LIGHT!");
#endif

  WhiteLight = new TStatic(this, ID_WHITE_LIGHT, " ", x1, y1, x_width, y_width, 30);
return 0;
}

/**************************************************************************
* Closing routine
*
***************************************************************************/
bool TTestWindow::CanClose()
{
  char message1[81], title1[41];

/* End exposure if exposure running... */
  if(exposure_on) HandleButton_Exposure();

/* Close logbook if necessary: */
  if(fp_logbook != NULL) fclose(fp_logbook);

// Close RS232 window if necessary:
  Close_RS232window();

  if(bad_initialization) return TRUE;

#if LANGUAGE == FRENCH
  strcpy(message1,"Voulez-vous sauvegarder la configuration actuelle?");
  strcpy(title1,"Sortie du programme");
#elif LANGUAGE == ITALIAN
  strcpy(message1,"Volete salvare la configurazione attuale?");
  strcpy(title1,"Uscita dal programma");
#else
  strcpy(message1,"Do you want to save current configuration?");
  strcpy(title1,"Exit from this program");
#endif
  if( ::MessageBox(HWindow, message1, title1, MB_YESNO) != IDNO)
    {
/*
    ::MessageBox(HWindow, "OK: we save current configuration",
                        "Exit from this program", MB_OK);
*/
    if(SaveStatus(message1))
          ::MessageBox(HWindow, message1, "SaveStatus error", MB_OK);
     }

return TRUE;
}
/**********************************************************************
* Open RS232 window and initialize RS232 link:
*
**********************************************************************/
int TTestWindow::InitRS232Link()
{
int status;

/* Default delay parameter: 1000  (check in rs232win.cpp the routine "rs232_delay")*/
// JLP 2011: changed from 1000 to 100 for new computer and Sleep()
  delay_msec = 100;

// Open RS232 window, RS232 link and boot PISCO
  status = Open_RS232window();

// Return status = -4 if user wants to exit
// But cannot exit at this stage since TTestWindow setup is not finished yet!
return(status);
}
/**********************************************************************
*
**********************************************************************/
int TTestWindow::InitPISCO()
{
int status;
char buffer[80];

/* Wait some time before sending new order: */
   rs232_delay(delay_msec);

/* Set Risley prisms to zero dispersion: */
   sprintf(buffer,"%04d",RA_NULLDISP);
   EditRA->SetText(buffer);
// Send order to PISCO:
   HandleButton_RA();

/* Wait some time before sending new order: */
   rs232_delay(delay_msec);

/* JLP94/Warning: for some unknown reason, EditRB call should be just before
* HandleButton_RB() call ... */
   sprintf(buffer,"%04d",RB_NULLDISP);
   EditRB->SetText(buffer);
   HandleButton_RB();

/* JLP95: assume Risley prisms work properly... */
   ra_position = RA_NULLDISP;
   rb_position = RB_NULLDISP;

/* Position of the risley prisms with red dispersion towards the North:
* increasing code: dispersion rotates to west at pic du midi
*/
/* Risley prisms: */
ra_offset = 511;
rb_offset = 156;
ra_rb_sign = 1;



/* Wait some time before sending new order: */
   rs232_delay(delay_msec);

/* Load good wheel encoding positions from file TAVPOSI: */
   status = LoadPosition2(TAVPOSI);
   if(!status) pisco_initialized = 1;

return(status);
}
/**********************************************************************
*
**********************************************************************/
int TTestWindow::InitTavParameters()
{
char message1[81];

/* Exposure button (default is off): */
//  ExposureButton->SetCheck(BF_UNCHECKED);

/* Switch automatic correction off: */
  if(auto_risley_on)
    {
    RTButtonON->Check(); RTButtonOFF->Uncheck();
    }
  else
    {
    RTButtonOFF->Check(); RTButtonON->Uncheck();
    }


/* Get star parameters from status file TAVSTAT: */
  if(UpdateStarFromStatusFile(TAVSTAT, message1))
    {
    ::MessageBox(HWindow, message1, "UpdateStarFromStatusFile error", MB_OK);
    }

/* Get atmosphere parameters from status file TAVSTAT: */
  if(UpdateAtmFromStatusFile(TAVSTAT, message1))
    {
    ::MessageBox(HWindow, message1, "UpdateAtmFromStatusFile error", MB_OK);
    }

/* Update wheel configuration from file TAVCONF: */
  if(UpdateConfig(message1))
    {
    ::MessageBox(HWindow, message1, "UpdateConfig fatal error", MB_OK);
    return(-1);
    }

/* Ask for atmospheric parameters: */
   AtmInput();

/* Open logbook: */
   OpenLogBook();

/* Setup initial wheel positions from TAVSTAT file */
  if(UpdateWheelsFromStatusFile(message1))
    {
    ::MessageBox(HWindow, message1,
                 "UpdateWheelFromStatusFile error", MB_OK);
    }

/* Update current positions from RS232 link: */
  UpdateWheelsFromRS232Link();

/* Display new values on the screen: */
  if(RefreshScreen(message1, 1))
    {
#if LANGUAGE == FRENCH
    ::MessageBox(HWindow, message1,
              "Attention: correction insuffisante", MB_OK);
#elif LANGUAGE == ITALIAN
    ::MessageBox(HWindow, message1,
              "Attenzione: correzzione insufficiente", MB_OK);
#else
    ::MessageBox(HWindow, message1,
              "Warning: limited correction", MB_OK);
#endif
    }

return(0);
}
/***************************************************************
*   RefreshScreen
* To display new status on the screen
*
* SHOULD NOT OUTPUT any message (since it is called by IdleAction) !!!!
*
* INPUT:
*  whole_screen if 1 whole configuration, if 0 only time and risley correction
* OUTPUT:
*  error_message is filled in ComputeCorrection
****************************************************************/
int TTestWindow::RefreshScreen(char *error_message, int whole_screen)
{
char buffer[80];
register int k;
int ih0, ih1, status, Refresh_status;
double cross_angle, utime1;
double h2, ww_long, ww_sidtime;

// Return immediately if initialization of Main Window has failured:
 if(bad_initialization)return(0);

 Refresh_status = 0;

/* New time value: */
  if(jlp_utime(buffer,&utime1))
    Clock1->SetText("U.T: error");
  else
    Clock1->SetText(buffer);

  if(exposure_on)
    {
     convert_coord(utime1 - start_time,&ih0,&ih1,&h2,"H");
     sprintf(buffer,"%2d H %2d m %2d s",ih0,ih1,(int)h2);
     ExpoTime1->SetText(buffer);
    }
  else
     ExpoTime1->SetText(" ");

/* Compute hour angle, air mass, and Risley prism positions */
 if(astro_target)
 {
  status = ComputeCorrection(error_message, &cross_angle);
/* First error case: object under the horizon: */
  if(status < 0)
         {
#if LANGUAGE == FRENCH
          sprintf(error_message,"L'objet est sous l'horizon");
          HourAngle1->SetText(" Non calculable");
#elif LANGUAGE == ITALIAN
          sprintf(error_message,"L'oggetto è sotto l'orizzonte");
          HourAngle1->SetText(" Non calcolabile");
#else
          sprintf(error_message,"The target is under the horizon");
          HourAngle1->SetText(" Undefined ");
#endif
          Refresh_status = -1;
         }
  else
         {
/* In case of minor error, still display hour angle, elevation and air mass: */
         if(status || cross_angle == 0.) Refresh_status = 1;
/* Hour angle: */
          convert_coord(hour_angle,&ih0,&ih1,&h2,"H");
          sprintf(buffer,"%2d H %2d m %2d s",ih0,ih1,(int)h2);
          HourAngle1->SetText(buffer);
/* Elevation: */
          sprintf(buffer,"%.1f",elevation);
          Elevation1->SetText(buffer);
/* Air mass: */
          sprintf(buffer,"%.2f",air_mass);
          Airmass1->SetText(buffer);
         }

/* If limit has been reached, display warning message (in calling routine): */
  if(cross_angle == 0.) Refresh_status = 1;

/* Sidereal time (after ComputeCorrection to be sure that ww_long is good) */
  ww_long = longitude;

  if(jlp_lsidtime(buffer, &ww_sidtime, ww_long))
          SiderTime1->SetText("LST/Error.");
  else
          SiderTime1->SetText(buffer);

/* Fill editing boxes: */
  sprintf(buffer,"%04d",ra_computed);
  EditRA->SetText(buffer);
  sprintf(buffer,"%04d",rb_computed);
  EditRB->SetText(buffer);

/* If automatic correction, send a command every time the difference
between current status and computed position is more than RISLEY_TOLER encoding steps: */
  if(auto_risley_on && exposure_on) Automatic_Correction(error_message);
/* End of case of astro target: */
  }
/* Case of calibration: */
  else
  {
    sprintf(buffer,"%04d",RA_NULLDISP);
    EditRA->SetText(buffer);
    sprintf(buffer,"%04d",RB_NULLDISP);
    EditRB->SetText(buffer);
    HourAngle1->SetText(" ");
    Airmass1->SetText(" ");
    Elevation1->SetText(" ");
    RightAsc1->SetText(" ");
    Delta1->SetText(" ");
  }

/* RA and RB positions first read in UpdateWheelFromStatusFile
  and then in Update_From_RS232... */
  sprintf(buffer,"Risley RA: %04d",ra_position);
  RAStatus->SetText(buffer);
  sprintf(buffer,"Risley RB: %04d",rb_position);
  RBStatus->SetText(buffer);


/******************* Whole screen updating (not every second...) *****/
 if(whole_screen)
  {
/**************** Lamp status: */
if(lamp_is_on)
  {
#if LANGUAGE == FRENCH
  strcpy(buffer,"Lampe ALLUMEE");
#elif LANGUAGE == ITALIAN
  strcpy(buffer,"Lampada ACCESA");
#else
  strcpy(buffer,"Lamp ON");
#endif
  }
else
  {
#if LANGUAGE == FRENCH
  strcpy(buffer,"Lampe éteinte");
#elif LANGUAGE == ITALIAN
  strcpy(buffer,"Lampada spenta");
#else
  strcpy(buffer,"Lamp OFF");
#endif
  }
LampButton->SetCaption(buffer);

/* Look for current filter */
  k = FilterIndex();
/* And display relevant text (after "FB 2 " for instance):
Example:
#FA 2 550.00 72.0, but only "FA 2 550.00 72.0" in FilterParam...
*/
  if(k != -1)
     {
     strncpy(buffer,&FilterParam[k][5],MAXFILTERLEN);
     buffer[MAXFILTERLEN-1] = '\0';
     Filter1->SetText(buffer);
     }
  else
#if LANGUAGE == FRENCH
     Filter1->SetText("Aucun filtre");
#elif LANGUAGE == ITALIAN
     Filter1->SetText("Alcun filtro");
#else
     Filter1->SetText("No filter");
#endif

/* Wheel positions: */
    for (k = 0; k < NWHEELS; k++)
      {
        if(wheel_is_there[k])
            {
            sprintf(buffer,"%1d",wheel_position[k]);
            Combo_wheel[k]->SetText(buffer);
            }
        else
            {
#if LANGUAGE == FRENCH
            strcpy(buffer,"0: Pas branchée !");
#elif LANGUAGE == ITALIAN
            strcpy(buffer,"0: Non collegato !");
#else
            strcpy(buffer,"0: Not connected!");
#endif
            Combo_wheel[k]->SetText(buffer);
/* In case 0 was not there, create new entry: */
            answer1[0]='\0';
            Combo_wheel[k]->GetText(answer1,20);
            if(strncmp(answer1,buffer,17))
                 {
                 Combo_wheel[k]->AddString(buffer);
                 Combo_wheel[k]->SetText(buffer);
                 }
            }
      }

/* Update atmospheric dispersion correction text: */
  ADC_Text1->SetText(ADC_text1);

/* Target name: */
  Target1->SetText(StarName);

   if(astro_target) {
/* Coordinates: */
      convert_coord(alpha_value,&ih0,&ih1,&h2,"H");
      sprintf(buffer,"%2d H %2d m %4.1f s",ih0,ih1,h2);
      RightAsc1->SetText(buffer);
      convert_coord(delta_value,&ih0,&ih1,&h2,"D");
      sprintf(buffer,"%2d° %2d' %.1f\"",ih0,ih1,h2);
/* Case of -0 30 54: */
      if(ih0 == 0 && delta_value < 0.) sprintf(buffer,"-0° %2d' %.1f\"",ih1,h2);
      Delta1->SetText(buffer);
      }
    else  {
      RightAsc1->SetText(" ");
      Delta1->SetText(" ");
      }

/* End of "whole_screen" case... */
 }

  return(Refresh_status);
}
//**********************************************************************
// Automatic correction
/* If automatic correction, send a command every time the difference
between current status and computed position is more than RISLEY_TOLER encoding steps: */
//**********************************************************************
int TTestWindow::Automatic_Correction(char *error_message)
{
char buffer[80];
int n_wanted, n_received, status, iget, nval;
  if((ABS(ra_computed - ra_position) > RISLEY_TOLER)
     || (ABS(rb_computed - rb_position) > RISLEY_TOLER))
           {
/* Copy to Command Status: */
           sprintf(buffer,"RA %04d",ra_computed);
           CommandStatus->SetText(buffer);
/* Send command: */
           sprintf(buffer,"RA %04d\r",ra_computed);
// Wait for the answer, like "RA 0123"
           n_wanted = 7;
           status = RS232w1->RS232_Send_Receive(buffer,answer1,error_message1,
                                       n_wanted, &n_received);
           if(status || (n_received < n_wanted)
              || answer1[0] != 'R' || answer1[1] != 'A')
              {
#if LANGUAGE == FRENCH
              strcpy(error_message1, "RA/Liaison RS232: erreur");
#elif LANGUAGE == ITALIAN
              strcpy(error_message1, "RA/Collegamento RS232: errore");
#else
              strcpy(error_message1, "RA/RS232 link: error");
#endif
// Previously:  in case of a problem, update position with "?"
//              rs232_delay(delay_msec); Update_From_RS232();
/* JLP95: In case of problems, assume Risley prisms work properly... */
/* (since "?" cause interruption which may be harmfull...) */
              ra_position = ra_computed;
              }
           else {
/* RA position: */
              ra_position = ra_computed;
              if(!strncmp(answer1, "RA ", 3)){
                nval = sscanf(answer1,"RA %04d",&iget);
                if(nval == 1) ra_position = iget;
                }
              }
// In all cases update Risley position:
          sprintf(buffer,"Risley RA: %04d",ra_position);
          RAStatus->SetText(buffer);

/* Wait some time before sending new order: */
           rs232_delay(delay_msec);
/* Copy to Command Status: */
           sprintf(buffer,"RB %04d",rb_computed);
           CommandStatus->SetText(buffer);
/* Send command: */
           sprintf(buffer,"RB %04d\r",rb_computed);
// Wait for the answer, like "RB 0123"
           n_wanted = 7;
           status = RS232w1->RS232_Send_Receive(buffer,answer1,error_message1,
                                       n_wanted, &n_received);
           if(status || (n_received < n_wanted) ||
               answer1[0] != 'R' || answer1[1] != 'B')
              {
#if LANGUAGE == FRENCH
              strcpy(error_message1, "RB/Liaison RS232: erreur");
#elif LANGUAGE == ITALIAN
              strcpy(error_message1, "RB/Collegamento RS232: errore");
#else
               strcpy(error_message1, "RB/RS232 link: error");
#endif
// Previously:  in case of a problem, update position with "?"
//              rs232_delay(delay_msec); Update_From_RS232();
/* JLP95: In case of problems, assume Risley prisms work properly... */
/* (since "?" cause interruption which may be harmfull...) */
              rb_position = rb_computed;
              }
          else
             {
/* RB position: */
              rb_position = rb_computed;
              if(!strncmp(answer1, "RB ", 3)){
                nval = sscanf(answer1,"RB %04d",&iget);
                if(nval == 1) rb_position = iget;
                }
             }
          sprintf(buffer,"Risley RB: %04d",rb_position);
          RBStatus->SetText(buffer);
/* End of if (ra_computed - ra_position) > RISLEY_TOLER */
        }
return(status);
}
/***************************************************************
* To remove \n character...
*
****************************************************************/
void clean_buffer(char *buffer, int size)
{
register int i;
char *pc;
 pc = buffer;
 for(i = 0; i < size - 1 && *pc != '\0' && *pc != '\n'; i++) pc++;
 *pc = '\0';
}

/***************************************************************
*  Edition of RA position
****************************************************************/
void TTestWindow::HandleButton_RA()
{

  char command1[MAXCOMMANDLEN], TheText[MAXRISLEN], buffer[80];
  int isend, iget, n_wanted, n_received, status, nval;
  UINT StartPos, EndPos;

/* Get text from editing box:
*/
  EditRA->GetSelection(StartPos, EndPos);
  if ( StartPos == EndPos )
    EditRA->GetText(TheText, sizeof(TheText));
  else EditRA->GetSubText(TheText, StartPos, EndPos);
/* Send command to speckle camera: */
  if(!isdigit(TheText[0]) || !isdigit(TheText[1]) ||
     !isdigit(TheText[2]) || !isdigit(TheText[3]))
    {
#if LANGUAGE == FRENCH
    sprintf(buffer," Mauvaise syntaxe: >%s< start=%d,end=%d",
           TheText,StartPos,EndPos);
         ::MessageBox(HWindow, buffer, " Erreur d'entrée pour RA", MB_OK);
#elif LANGUAGE == ITALIAN
    sprintf(buffer," Cattiva sintassi: >%s< start=%d,end=%d",
           TheText,StartPos,EndPos);
         ::MessageBox(HWindow, buffer, " Errore d'entrata per RA", MB_OK);
#else
    sprintf(buffer," Bad syntax: >%s< start=%d,end=%d",
           TheText,StartPos,EndPos);
    ::MessageBox(HWindow, buffer, " RA input error ", MB_OK);
#endif
    }
  else
    {
    sscanf(TheText,"%04d",&isend);
    if(isend >= 0 && isend < 1024)
      {
/* Copy to Command Status: */
       sprintf(command1,"RA %s",TheText);
       CommandStatus->SetText(command1);
/* Send command to Speckle camera: */
       sprintf(command1,"RA %s\r",TheText);
// Wait for the answer, like "RA 0123"
       n_wanted = 7;
       status = RS232w1->RS232_Send_Receive(command1,answer1,error_message1,n_wanted,
                                   &n_received);
/* JLP95: assume that Risley prisms work properly, despite problems
   with RS232 link: */
       ra_position = isend;
       if(status || (n_received < n_wanted)){
#if LANGUAGE == FRENCH
         ::MessageBox(HWindow, error_message1, "RA/Liaison RS232: erreur",
                      MB_OK);
#elif LANGUAGE == ITALIAN
         ::MessageBox(HWindow, error_message1, "RA/Collegamento RS232: errore",
                      MB_OK);
#else
         ::MessageBox(HWindow, error_message1, "RA/RS232 link: error",
                      MB_OK);
#endif
         }
        else {
/* RA position: */
           if(!strncmp(answer1, "RA ", 3)){
              nval = sscanf(answer1,"RA %04d",&iget);
              if(nval == 1) ra_position = iget;
              }
          }
/* Copy RA and RB positions to Editing box
*  (via ra_computed which is written to that box
*  everytime the user press the "Refresh" button:) */
        sprintf(buffer,"Risley RA: %04d",ra_position);
        RAStatus->SetText(buffer);
      }
    else
      {
#if LANGUAGE == FRENCH
      sprintf(buffer," Mauvaise valeur: >%s<", TheText);
                ::MessageBox(HWindow, buffer, " Erreur d'entrée pour RA", MB_OK);
#elif LANGUAGE == ITALIAN
    sprintf(buffer," Cattiva sintassi: >%s<",TheText);
         ::MessageBox(HWindow, buffer, " Errore d'entrata per RA", MB_OK);
#else
      sprintf(buffer," Bad value: >%s<", TheText);
      ::MessageBox(HWindow, buffer, " RA input error ", MB_OK);
#endif
      }
    }
  EditRA->SetSelection(0, 0);
return;
}
/***************************************************************
*  Edition of RB position
****************************************************************/
void TTestWindow::HandleButton_RB()
{
  char command1[MAXCOMMANDLEN], TheText[MAXRISLEN], buffer[80];
  int isend, iget, n_wanted, n_received, status, nval;
  UINT StartPos, EndPos;

/* Get text from editing box:
*/
  EditRB->GetSelection(StartPos, EndPos);
  if ( StartPos == EndPos )
         EditRB->GetText(TheText, sizeof(TheText));
  else EditRB->GetSubText(TheText, StartPos, EndPos);

/* Send a command to change rb_position: */
  if(!isdigit(TheText[0]) || !isdigit(TheText[1]) ||
          !isdigit(TheText[2]) || !isdigit(TheText[3]))
       {
#if LANGUAGE == FRENCH
       sprintf(buffer," Mauvaise syntaxe: >%s< start=%d,end=%d",
              TheText,StartPos,EndPos);
       ::MessageBox(HWindow, buffer, " Erreur d'entrée pour RB", MB_OK);
#elif LANGUAGE == ITALIAN
       sprintf(buffer," Cattiva sintassi: >%s< start=%d,end=%d",
               TheText,StartPos,EndPos);
       ::MessageBox(HWindow, buffer, " Errore d'entrata per RB", MB_OK);
#else
       sprintf(buffer," Bad syntax: >%s< start=%d,end=%d",
               TheText,StartPos,EndPos);
       ::MessageBox(HWindow, buffer, " RB input error ", MB_OK);
#endif
       return;
       }

// Syntax is good, so I go ahead:
 sscanf(TheText,"%04d",&isend);
 if(isend < 0 || isend > 1024) {
#if LANGUAGE == FRENCH
         sprintf(buffer," Mauvaise valeur: >%s<", TheText);
         ::MessageBox(HWindow, buffer, " Erreur d'entrée pour RB", MB_OK);
#elif LANGUAGE == ITALIAN
         sprintf(buffer," Cattiva sintassi: >%s<",TheText);
         ::MessageBox(HWindow, buffer, " Errore con RB", MB_OK);
#else
         sprintf(buffer," Bad value: >%s<", TheText);
         ::MessageBox(HWindow, buffer, " Error: bad value for RB", MB_OK);
#endif
         return;
         }

/* Copy to Command Status: */
  sprintf(command1,"RB %s",TheText);
  CommandStatus->SetText(command1);

/* Send command to Speckle camera: */
  sprintf(command1,"RB %s\r",TheText);
// JLP95
// Wait for the answer, like "RB 0123"
  n_wanted = 7;
  status = RS232w1->RS232_Send_Receive(command1,answer1,error_message1,
                              n_wanted, &n_received);
/* JLP95: assume that Risley prisms work properly, despite problems
   with RS232 link: */
   rb_position = isend;
   if(status || (n_received < n_wanted) || strncmp(answer1,"RB ",3)
       || !isdigit(answer1[3]) || !isdigit(answer1[4])
       || !isdigit(answer1[5]) || !isdigit(answer1[6]) ){
#if LANGUAGE == FRENCH
             ::MessageBox(HWindow, error_message1, "RB/Liaison RS232: erreur",
                           MB_OK);
#elif LANGUAGE == ITALIAN
             ::MessageBox(HWindow, error_message1, "RB/Collegamento RS232: errore",
                          MB_OK);
#else
             ::MessageBox(HWindow, error_message1, "RB/RS232 link: error",
                           MB_OK);
#endif
             }
           else {
/* RB position: */
              if(!strncmp(answer1, "RB ", 3)){
                nval = sscanf(answer1,"RB %04d",&iget);
                if(nval == 1) rb_position = iget;
                }
             }
/* JLP95: assume that Risley prisms work properly, despite problems
   with RS232 link: */
  sprintf(buffer,"Risley RB: %04d",rb_position);
  RBStatus->SetText(buffer);
  EditRB->SetSelection(0, 0);
return;
}
/***************************************************************
*  Update screen values and propose new values for RA and RB:
****************************************************************/
void TTestWindow::HandleButton_UpdateRisley()
{

  char error_message[80];

/* Compute hour angle, air mass, and Risley prism positions */
  if(RefreshScreen(error_message, 0))
         {
#if LANGUAGE == FRENCH
         ::MessageBox(HWindow, error_message,
                       "Attention: correction insuffisante", MB_OK);
#elif LANGUAGE == ITALIAN
         ::MessageBox(HWindow, error_message,
                       "Attenzione: correzzione insufficiente", MB_OK);
#else
         ::MessageBox(HWindow, error_message,
                       "Warning: limited correction", MB_OK);
#endif
         }
return;
}
/***************************************************************
*  ComputeCorrection
* Return:
* -1 if fatal error
* 1 or 2 if minor error
****************************************************************/
int TTestWindow::ComputeCorrection(char *error_message, double *cross_angle)
{
  double yyear, local_time1;
  int mmonth, dday;
  int iloc, Compute_status, status, code_ra, code_rb, k;
  double beta, resid_disp, zen_dist, lambdac, dlambda;
  double ww_alph, ww_delt, ww_hour, ww_elev;
  double delta_years, ww_zen_dist_rad;
  char buffer[80], ADC_text0[80];
  FILE *fp1;

  Compute_status = 0;
  strcpy(error_message," ");

/* Pic du Midi: iloc = 1, Merate: iloc=7 Calern: iloc=8*/
  iloc = 8;
  input_location(&latitude, &longitude, &iloc);

// Coordinates corrected for precession:
  ww_alph = alpha_value;
  ww_delt = delta_value;

/* Compute precession correction if needed: */
  if(equin_value != 0.) {
      jlp_local_time(buffer);
      decode_time(buffer,&yyear,&mmonth,&dday,&local_time1);
      delta_years = yyear + ((double)mmonth)/12. - equin_value;
      precess(&ww_alph, &ww_delt, delta_years);
      }

/* Then hour angle: */
  status = jlp_hangle(ww_alph, ww_delt, longitude, latitude, &ww_hour,
                      &ww_elev);
  if(!status)
     {
     elevation = ww_elev;
       if(elevation <= 90. && elevation > 5.)
           {
/* zen_dist is in degrees, ww_zen_dist_rad is in radians */
             zen_dist = 90. - elevation;
             ww_zen_dist_rad = zen_dist * DEGTORAD;
             air_mass = 1 / cos(ww_zen_dist_rad);
           }
       else
           air_mass = -1.;
     hour_angle = ww_hour;
     }
  else
    {
#if LANGUAGE == FRENCH
    strcpy(error_message,"Erreur dans ComputeCorrection/jlp_hangle");
#elif LANGUAGE == ITALIAN
    strcpy(error_message,"Errore in ComputeCorrection/jlp_hangle");
#else
    strcpy(error_message,"Error in ComputeCorrection/jlp_hangle");
#endif
    return(-1);
    }

/************** RA and RB positions: *************************/
/* If object under the horizon, move the prisms
   to the null dispersion position: */
if(air_mass < 0.)
  {
#if LANGUAGE == FRENCH
        strcpy(ADC_text1," Objet sous l'horizon: aucune correction!");
#elif LANGUAGE == ITALIAN
        strcpy(ADC_text1," Oggetto sotto l'orizzonte: nessuna correzzione!");
#else
        strcpy(ADC_text1," Object under the horizon: no correction!");
#endif
    ra_computed = RA_NULLDISP;  rb_computed = RB_NULLDISP;
  }
/* Case air_mass > 0, hence ra and rb positions can be computed */
  else
  {
/* Filter parameters:
Example:
#FA 2 550.00 72.0, but only "FA 2 550.00 72.0" in FilterParam...
*/
  k = FilterIndex();
  if(k != -1)
    {
    sscanf(&FilterParam[k][5],"%lf %lf",&lambdac,&dlambda);

    if((fp1 = fopen(LOGFILE,"w")) == NULL)
      {
#if LANGUAGE == FRENCH
        sprintf(error_message,
               " ComputeCorrection/Erreur lors de l'ouverture du fichier %s",
               LOGFILE);
#elif LANGUAGE == ITALIAN
        sprintf(error_message,
               " ComputeCorrection/Errore aprendo il file %s",
               LOGFILE);
#else
        sprintf(error_message," ComputeCorrection/Error opening file %s",
               LOGFILE);
#endif
      }
/* Debug: */
     else {
        fprintf(fp1," Latitude %f longitude %f \n",latitude,longitude);
        fprintf(fp1,
            " zd = %.2f deg, lambda_cent = %.2f nm, delta_lambda = %.2f nm\n",
            zen_dist, lambdac, dlambda);
        fprintf(fp1," Hour_angle %f zd %f",hour_angle,zen_dist);
       }

      Compute_status = risley_posi(fp1, latitude, hour_angle, zen_dist,
           lambdac, dlambda, &beta, cross_angle, &resid_disp,
           temp_value, hygro_value, press_value, ra_offset, rb_offset,
           ra_rb_sign, &code_ra, &code_rb, ADC_text0);
    if(Compute_status == 0) {
      sprintf(error_message,
      "beta=%.2f, cross_angle=%.1f, ra=%d, rb=%d resid_disp=%.4f\n",
      beta, *cross_angle, code_ra, code_rb, resid_disp);
     } else {
      strcpy(ADC_text1, ADC_text0);
     }
    if(fp1 != NULL) fclose(fp1);
    }
/* Case when no filter has been selected */
  else
    {
#if LANGUAGE == FRENCH
    strcpy(error_message,"Pas de filtre, donc pas de correction...\n");
#elif LANGUAGE == ITALIAN
    strcpy(error_message,"Nessuno filtro, dunque nessuna correzzione...\n");
#else
    strcpy(error_message,"No filter, therefore no correction...\n");
#endif
    Compute_status = 1;
    }

/* If satisfactory, move the prisms to the computed position: */
  if(!Compute_status && code_ra > 0)
    {
    ra_computed = code_ra;  rb_computed = code_rb;
    }
/* Else move the prisms to the null dispersion position: */
  else
    {
    ra_computed = RA_NULLDISP;  rb_computed = RB_NULLDISP;
    }

  } /* end of case air_mass > 0 */

return(Compute_status);
}
/***************************************************************
*  Send a command to speckle camera and update wheel positions
****************************************************************/
int TTestWindow::UpdateWheelsFromRS232Link()
{
  register int k;
  char *pc;
  int status, i;

  debug_string[0] = '\0';
/* Get current status and update parameters: */
  rs232_delay(delay_msec);
  status = Update_From_RS232();
  for(i = 0; i < 3 && status; i++)
   {
/* DEBUG:
   ::MessageBox(HWindow, debug_string, "UpdateWheelsFromRS232Link", MB_OK);
*/
   rs232_delay(delay_msec);
   status = Update_From_RS232();
   }

/* Answer from speckle camera is in "error_message1": */
  if(!status)
    {
/* DEBUG
#if LANGUAGE == FRENCH
    ::MessageBox(HWindow, error_message1, "Collegamento RS232: risposta di PISCO", MB_OK);
#elif LANGUAGE == ITALIAN
    ::MessageBox(HWindow, error_message1, "Liaison RS232: réponse de PISCO", MB_OK);
#else
    ::MessageBox(HWindow, error_message1, "RS232 link: answer from PISCO", MB_OK);
#endif
*/
    }
  else
    {
     pc = error_message1;
     for(k = 0; k < NWHEELS; k++)
       {
       sprintf(pc,"k=%2d %2d %.2s\n   ",k,wheel_is_there[k],wheel_title[k]);
       pc+=12;
       }
     *pc = '\0';
    ::MessageBox(HWindow, error_message1,
                 "Wheel status: 1=present 0=absent", MB_OK);

    return -1;
    }

#ifdef DEBUG
    for(k=0; k< NWHEELS; k++)
       sprintf(&error_message1[k*14],"k=%1d %04X %04X\n",
              k,required_posi[k],actual_posi[k]);
    ::MessageBox(HWindow, error_message1, "Positions", MB_OK);
#endif
/* Check if wheels are between two positions */
    for(k=0; k< NWHEELS; k++)
       {if((required_posi[k]-actual_posi[k]) > WHEEL_TOLER
           || (required_posi[k]-actual_posi[k]) < -WHEEL_TOLER)
           {
#if LANGUAGE == FRENCH
           sprintf(error_message1,
             " Roue %s\n Position=%04X, ordre=%04X",
                   wheel_title[k],actual_posi[k],required_posi[k]);
           ::MessageBox(HWindow, error_message1,
                        "Attention, roue mal positionnée !", MB_OK);
#elif LANGUAGE == ITALIAN
           sprintf(error_message1,
             " Ruota %s\n Posizione=%04X, ordine=%04X",
                   wheel_title[k],actual_posi[k],required_posi[k]);
           ::MessageBox(HWindow, error_message1,
                        "Attenzione, ruota mal posizionata !", MB_OK);
#else
           sprintf(error_message1,
                   " Wheel %s \n Actual_position=%04X, required_position=%04X",
                   wheel_title[k],actual_posi[k],required_posi[k]);
           ::MessageBox(HWindow, error_message1,
                        "Warning, bad position!", MB_OK);
#endif
           }
       }

// JLP September 2007: new modification asked by Marco:
// I display a message if (FA 1 and FB 6) are selected,
// which correponds to "White light" (=  Luce Bianca).
// (NB: wheel number of FA is 6, that of FB is 7)
 if(((wheel_position[6] == 1) || (wheel_position[6] == 3))
    && (wheel_position[7] == 6)) {
   WhiteLight->SetText(WhiteLightText);
 } else {
   WhiteLight->Clear();
 }
return(0);
}

/***************************************************************
*  Send a command to speckle camera and update wheel positions
****************************************************************/
void TTestWindow::HandleButton_UpdateWheels()
{
char error_message[80];

if(UpdateWheelsFromRS232Link() == 0) {
/* Display current state on the screen */
  if(RefreshScreen(error_message, 1))
    {

#if LANGUAGE == FRENCH
    ::MessageBox(HWindow, error_message,
                 "Attention: correction de Risley insuffisante", MB_OK);
#elif LANGUAGE == ITALIAN
    ::MessageBox(HWindow, error_message,
                 "Attenzione: la correzzione di Risley è insufficiente", MB_OK);
#else
    ::MessageBox(HWindow, error_message,
                 "Warning: unsufficient Risley correction", MB_OK);
#endif
    }
  }
return;
}
/***************************************************************
*  Send a command to speckle camera and update wheel positions
****************************************************************/
int TTestWindow::Update_From_RS232()
{
  char command1[25], *pc;
/* Warning: error_message and buffer should be large enough... */
  char c1[BUFF_LEN1];
  register int i, j;
  int n_wanted, n_received, status, istart, iend, c1_len;
  int ra_is_found, rb_is_found, nwheels_found;

  if(no_rs232link_wanted || rs232link_is_down) return(-1);

/* Initialization: */
rs232_wheel_names[0] = '\0';
rs232_wheel_is_there[0] = '\0';
ra_is_found = 0;
rb_is_found = 0;
nwheels_found = 0;

/* Copy ", ?" at the end of Command Status string: */
    CommandStatus->GetText(command1,20);
    pc = command1; command1[20]='\0'; while(*pc) pc++;
    strcpy(pc,", ?");
    CommandStatus->SetText(command1);
/* Send command to Speckle camera: */
    strcpy(command1,"?\r");
    n_wanted = 500;
    status = RS232w1->RS232_Send_Receive(command1,answer1,error_message1,
                                n_wanted, &n_received);
    if(status) return(-1);
/* If answer is too short return(-2): */
    if(n_received <= 100) return(-2);

    rs232_delay(delay_msec);
/*********************************************************
* Read answer1 and update positions
*********************************************************/
/* Example:
       "AS EN CH MA RA RB DB FA FT GR FB FC FD \r\n"
       "-  -  -  -  -  -  -  -  X  -  -  X  X  \r\n"
       "R   No  ORDRE  ETAT \r\n"
       "AS  2  07B5   07DC \r\n"
       "EN  2  07B5   07DC \r\n"
       "CH  5  07B5   07DC \r\n"
       "MA  2  042B   0417 \r\n"
       "RA     0856   0856 \r\n"
       "RB     0257   0256 \r\n"
       "DA  2  07B5   07DC \r\n"
       "DB  2  07B5   07DC \r\n"
       "FA  2  07B5   07DC \r\n"
       "FT  2  07B5   07DC \r\n"
       "GR  2  07B5   07DC \r\n"
       "FB  2  07B5   07DC \r\n"
       "FC  2  0000   0000 \r\n"
       "FD  2  0000   0000 \r\n"
       "LA LAMPE EST OFF";
*/
/* Analyse string from begining to end, looking to \r \n or \f character: */
  answer1[BUFF_LEN1 - 2] = '\0';
  istart = 0; i = 0;
  while(answer1[i] && i < BUFF_LEN1 - 1)
    {
    istart = i;
/* Increase i as long as answer1[i] is not a control character: */
    while(!iscntrl(answer1[i]) && i < BUFF_LEN1 - 1) i++;
    iend = i;
/* Transfer from answer1 to c1 array: */
      for(j = istart; j < iend; j++) c1[j-istart] = answer1[j];
/* c1 length: */
      c1_len = iend - istart + 1;
      c1[c1_len] = '\0';
/* Update parameter: */
      status = Update_From_RS232_Decode(c1,c1_len, &ra_is_found,
                                        &rb_is_found, &nwheels_found);
      if(status) return(-1);
/* Jump 1 step because of "\r" "\f" or "\n": */
    istart = iend + 1;
/* If Control character and not at the end, jump as many steps
as needed: */
    while(iscntrl(answer1[istart]) && istart < BUFF_LEN1 - 1) istart++;
    i = istart;
    }

/* Determine which wheels are present from the first two lines: */
   status = Wheel_Status();

/* The most important is that the positions of the 9 wheels
* are correctly decoded: */
if(ra_is_found && rb_is_found && nwheels_found == 9) status = 0;

  sprintf(debug_string," ra_is_found=%d rb=%d nwheels=%d",
          ra_is_found, rb_is_found, nwheels_found);

return(status);
}
/***************************************************************
*  Look for parameter in c1 string and update it
****************************************************************/
int TTestWindow::Update_From_RS232_Decode(char *c1, int c1_len, int *ra_is_found,
                                    int *rb_is_found, int *nwheels_found)
{
char buffer[80];
register int k;

/* First line (AS EN ...): store it to "rs232_wheel_names" buffer: */
  if(!strncmp(c1,"AS EN",5))
      {
      strcpy(rs232_wheel_names,c1);
      }
/* Lamp status:  LA LAMPE EST ON or LA LAMPE EST OFF*/
  else if(!strncmp(c1,"LA LAMPE EST ",13))
    {
      if( c1[14] == 'N')
         lamp_is_on = 1;
      else
         lamp_is_on = 0;
    }
/* Wheel position status: searching for X or -.
*  Store it to "rs232_wheel_is_there" buffer. */
  else if(!strncmp(c1,"- ",2) || !strncmp(c1,"X ",2))
    {
       strcpy(rs232_wheel_is_there,c1);
    }
/* Risley prisms: */
  else if(!strncmp(c1,"RA",2))
    {
      sscanf(&c1[3],"%04d",&ra_position);
      *ra_is_found = 1;
    }
  else if(!strncmp(c1,"RB",2))
    {
      sscanf(&c1[3],"%04d",&rb_position);
      *rb_is_found = 1;
    }
/* Loop on all the wheels: */
  else
    {
    for (k = 0; k < NWHEELS; k++)
      {
      if(!strncmp(c1,wheel_title[k],2))
         {
// JLP96: doesn't work well         sscanf(&c1[4],"%1d",&wheel_position[k]);
// so I go back to previous settings:
//         sscanf(&c1[3],"%1d",&wheel_position[k]);
// I change it again, since pb sometimes:
         sscanf(&c1[3],"%d",&wheel_position[k]);
/* JLP96: Check if status code is correct: */
//         sscanf(&c1[7],"%04X",&actual_posi[k]);
//         sscanf(&c1[14],"%04X",&required_posi[k]);
         sscanf(&c1[6],"%04X",&actual_posi[k]);
         sscanf(&c1[13],"%04X",&required_posi[k]);
// Wheel has been found, so I exit:
         *nwheels_found = *nwheels_found + 1; break;
         }
      }
/* JLP95: Even if found = 0, should not return status=-1,
* since it can be something else, like G000, etc....
*/
    }

return(0);
}
/***************************************************************
*  Determines which wheels are present from the first two lines
* INPUT:
* char rs232_wheel_names[] : from speckle camera "?" with wheel names
* char rs232_wheel_is_there[] : from speckle camera "?" with crosses if
*                         a wheel is not there.
****************************************************************/
int TTestWindow::Wheel_Status()
{
char *pc1, *pc2;
int status, nwheels_found;
register int k;

/* Initialization: */
nwheels_found = 0;
for (k = 0; k < NWHEELS; k++)
 {
  wheel_is_there[k] = 0;
 }
ra_is_there = 0;
rb_is_there = 0;

/* Order is
AS.EN.CH.MA.RA.RB.DA.DB.FA.FT.GR.FB.FC.FD
 1  2  3  4  5  6  7  8  9  0  1  2  3  4
*/
/* Load pc1 and pc2 with two lines already input from "status buffer": */
  pc1 = rs232_wheel_names;
  pc2 = rs232_wheel_is_there;
/* Use "rs232_wheel_names" as a reference
*  to increase "rs232_wheel_names" index */
  while (*pc1 && *pc2)
     {
/* If blank, go to next character: */
/* A bit more complex than really needed for historical reasons... */
     if(*pc1 == ' ')
        {pc1++; pc2++;}
/* Decode 2 characters: */
/* Check if risley prisms are there: */
      else if(!strncmp(pc1,"RA",2))
         {
          ra_is_there = 1;
/* Skip 2 characters, since 2 have been decoded: */
          pc1++; pc2++;
          if(*pc1 && *pc2) {pc1++; pc2++;}
         }
      else if(!strncmp(pc1,"RB",2))
         {
          rb_is_there = 1;
/* Skip 2 characters, since 2 have been decoded: */
          pc1++; pc2++;
          if(*pc1 && *pc2) {pc1++; pc2++;}
         }
/* Loop on all the wheels: */
      else
        {
/*
wheel title:  AS EN CH MA DA DB FA FB GR
k index    :   0  1  2  3  4  5  6  7  8
*/
         for (k = 0; k < NWHEELS; k++)
           {
           if(!strncmp(pc1,wheel_title[k],2))
             {
/* Update wheel_is_there array: */
             if(!strncmp(pc2,"- ",2))
               {wheel_is_there[k] = 1; nwheels_found++;}
             }
/* End of "for k ..." loop */
           }
/* Skip 2 characters, since 2 have been decoded: */
         pc1++; pc2++;
         if(*pc1 && *pc2) {pc1++; pc2++;}
/* End of case : *pc1 != ' ' */
        }
/* EOF while loop: */
      }

// Current PISCO configuration has 9 wheels:
if(!ra_is_there || !rb_is_there || nwheels_found != 9)
 {
 status = 1;
 }
else status = 0;

return (status);
}
/***************************************************************
*  Answer to Combo_wheel selection
****************************************************************/
void TTestWindow::HandleBoxMsg_wheel()
{
 int n_wanted, n_received, old_position, new_position, status;
 char command1[MAXCOMMANDLEN], select1[40], buffer[80];

/* If no wheel selected, return: */
   if(Wheel_To_Update < 0) return;

/* If wheel not connected, error message and return: */
   if(!wheel_is_there[Wheel_To_Update])
      {

#if LANGUAGE == FRENCH
         ::MessageBox(HWindow, "Roue non branchée !", "Erreur", MB_OK);
#elif LANGUAGE == ITALIAN
         ::MessageBox(HWindow, "Ruota non collegata !", "Errore", MB_OK);
#else
         ::MessageBox(HWindow, "Wheel not connected!", "Error", MB_OK);
#endif
/* Cancel position chosen by the user: */
      Combo_wheel[Wheel_To_Update]->SetText("0");
      return;
      }

/* Transfer input text to "select1" */
   Combo_wheel[Wheel_To_Update]->GetText(select1,30);

/* Check if position has been actually changed from previous one: */
   old_position = wheel_position[Wheel_To_Update];
   sscanf(&select1[0],"%1d",&new_position);
   if(new_position != old_position)
        {
        sprintf(command1,"%c%c %1d",wheel_title[Wheel_To_Update][0],
                wheel_title[Wheel_To_Update][1],new_position);
/* Updates new position: */
        wheel_position[Wheel_To_Update] = new_position;
        RefreshScreen(buffer, 1);
        sprintf(buffer," %s, command= >%s<", select1, command1);
//        ::MessageBox(HWindow, buffer, wheel_title[Wheel_To_Update], MB_OK);
         CommandStatus->SetText(command1);
/* Send command to Speckle camera: */
        sprintf(command1,"%c%c %1d\r",wheel_title[Wheel_To_Update][0],
                wheel_title[Wheel_To_Update][1],new_position);
        n_wanted = -1;
        status = RS232w1->RS232_Send_Receive(command1,buffer,error_message1,n_wanted,
                                    &n_received);
         if(status)
         {
#if LANGUAGE == FRENCH
         ::MessageBox(HWindow, error_message1, "Liaison RS232: erreur", MB_OK);
#elif LANGUAGE == ITALIAN
         ::MessageBox(HWindow, error_message1, "Collegamento RS232: errore", MB_OK);
#else
         ::MessageBox(HWindow, error_message1, "RS232 link: error", MB_OK);
#endif
         }
         else
          rs232_delay(delay_msec);
       }

// JLP September 2007: new modification asked by Marco:
// I display a message if (FA 1 and FB 6) are selected,
// which correponds to "White light" (=  Luce Bianca).
// (NB: wheel number of FA is 6, that of FB is 7)
 if(((wheel_position[6] == 1) || (wheel_position[6] == 3))
    && (wheel_position[7] == 6))
     {
// No filter = White light
     WhiteLight->SetText(WhiteLightText);
     } else {
// Presence of filter = No white light
     WhiteLight->Clear();
     }
return;
}
/***************************************************************
*
****************************************************************/
void TTestWindow::HandleBoxMsg_wheel0()
{
 Wheel_To_Update = 0;
 TTestWindow::HandleBoxMsg_wheel();}

void TTestWindow::HandleBoxMsg_wheel1()
{
 Wheel_To_Update = 1;
 TTestWindow::HandleBoxMsg_wheel();}

void TTestWindow::HandleBoxMsg_wheel2()
{
 Wheel_To_Update = 2;
 TTestWindow::HandleBoxMsg_wheel();}

void TTestWindow::HandleBoxMsg_wheel3()
{
 Wheel_To_Update = 3;
 TTestWindow::HandleBoxMsg_wheel();}

void TTestWindow::HandleBoxMsg_wheel4()
{
 Wheel_To_Update = 4;
 TTestWindow::HandleBoxMsg_wheel();}

void TTestWindow::HandleBoxMsg_wheel5()
{
 Wheel_To_Update = 5;
 TTestWindow::HandleBoxMsg_wheel();}

void TTestWindow::HandleBoxMsg_wheel6()
{
 Wheel_To_Update = 6;
 TTestWindow::HandleBoxMsg_wheel();}

void TTestWindow::HandleBoxMsg_wheel7()
{
 Wheel_To_Update = 7;
 TTestWindow::HandleBoxMsg_wheel();}

void TTestWindow::HandleBoxMsg_wheel8()
{
 Wheel_To_Update = 8;
 TTestWindow::HandleBoxMsg_wheel();}

/*********************************************************
* Enable/disable automatic Risley correction:
***********************************************************/
void TTestWindow::HandleRButton_AutoRisley()
{
  auto_risley_on = 1;
// Debug:
//  ::MessageBox(HWindow, "Automatic", "Risley correction", MB_OK);
}
void TTestWindow::HandleRButton_InteracRisley()
{
  auto_risley_on = 0;
// Debug:
//  ::MessageBox(HWindow, "Interactive", "Risley correction", MB_OK);
}
/********************************************************************
*
********************************************************************/
void TTestWindow::HandleButton_Lamp()
{
char buffer[80], command1[10], command2[10];
int n_wanted, n_received, status;

/****************** Toggle lamp status: */
  lamp_is_on = 1 - lamp_is_on;

/****************** Update lamp field: */
if(lamp_is_on)
  {
  strcpy(command1,"ON\r");
  strcpy(command2,"ON");
#if LANGUAGE == FRENCH
  strcpy(buffer,"Lampe ALLUMEE");
#elif LANGUAGE == ITALIAN
  strcpy(buffer,"Lampada ACCESA");
#else
  strcpy(buffer,"Lamp ON");
#endif
  }
else
  {
  strcpy(command1,"OFF\r");
  strcpy(command2,"OFF");
#if LANGUAGE == FRENCH
#elif LANGUAGE == ITALIAN
  strcpy(buffer,"Lampe spenta");
#else
  strcpy(buffer,"Lamp OFF");
#endif
  }
LampButton->SetCaption(buffer);

/* Update command status: */
   CommandStatus->SetText(command2);

/* Send command to Speckle camera: */
   n_wanted = -1;
   status = RS232w1->RS232_Send_Receive(command1,answer1,error_message1,n_wanted,
                               &n_received);
   if(!status) rs232_delay(delay_msec);
}
/************************************************************************
* Routine UpdateConfig
* to update the whole configuration of the speckle camera
* by reading the file TAVCONF="tavconfi.dat"
*
* Example:

AS Lampes calib.
1: Halogene
2: Filament
3: Argon
4: Mercure
5: Neon
6: Xenon
EN Roue d'entree
1: vide
2: vide
3: risque d'oc.
4: miroir calib
5: risque d'oc.
6: vide
CH Roue de champ
1: Fente
2: vide
3: mire
4: coronographe
5: vide
6: lent. de champ
MA Roue a masques
1: 4 trous
2: trou 7 mm
3: vide
4: vide
5: trous alignes
6: trous en rond
DA Roue a densites
1: vide
2: densite 1
3: densite 2
4: densite 3
5: densite 4
6: vide
DB Roue a densites
1: vide
2: densite 0.2
3: densite 0.4
4: densite 0.6
5: densite 0.8
6: vide
FA Roue a filtres
1: R 650/70
2: Grisme
3: V 550/70
4: vide
FB Roue a filtres
1: vide
2: IR 812/15
3: vide
4: BV 489/15
5: vide
6: R 614/15
GR Roue de grandisst
1: Pupille
2: Spectro
3: 4.5 mm
4: 20 mm
5: 50 mm
6: 10 mm
## Filtres:
#FA 1 650.00 70.0
#FA 3 550.00 70.0
#FB 2 812.00 15.0
#FB 4 489.00 15.0
#FB 6 614.00 15.0
*
************************************************************************/
int TTestWindow::UpdateConfig(char *error_message)
{
/* Read information from a file */
  FILE *fp;
  char buffer[81], *pc;
  register int i, k;
  int status;

status = 0;

  if((fp = fopen(TAVCONF,"r")) == NULL)
    {
#if LANGUAGE == FRENCH
     sprintf(error_message," UpdateConfig/Fichier %s introuvable",TAVCONF);
#elif LANGUAGE == ITALIAN
     sprintf(error_message," UpdateConfig/File %s non trovato",TAVCONF);
#else
     sprintf(error_message," UpdateConfig/File %s not found",TAVCONF);
#endif
     return(-1);
    }

sprintf(error_message," UpdateConfig/OK, file %s found",TAVCONF);

/* Loading wheel settings: */
 for (k = 0; k < NWHEELS; k++)
  {
    if(fgets(wheel_title[k],60,fp) == NULL)
    {
     sprintf(error_message," UpdateConfig/Error 1 reading %s",TAVCONF);
     fclose(fp); return (-1);
    }

// Load 4 values since 4 positions for wheel FA
  if(!strncmp(wheel_title[k],"FA",2))
      for (i = 0; i < 4; i++)
        {
         fgets(buffer,60,fp);
         clean_buffer(buffer,60);
         Combo_wheel[k]->AddString(buffer);
        }
// Load 6 values since 6 positions for all other wheels:
  else
      for (i = 0; i < 6; i++)
        {
         fgets(buffer,60,fp);
         clean_buffer(buffer,60);
         Combo_wheel[k]->AddString(buffer);
        }
  }

/* Now the filters:
Example:
  #FB 1 650.00 67.0
*/
nfilters = 0;
  while(fgets(buffer,40,fp) != NULL)
    {
      if(buffer[0] == '#' && buffer[1] != '#')
         {
         strncpy(FilterParam[nfilters], &buffer[1], MAXFILTERLEN);
/* Look for end of string: */
         FilterParam[nfilters][MAXFILTERLEN-1] = '\0';
         pc = FilterParam[nfilters];
         while(*pc && *pc != '\n') pc++;
         *pc = '\0';
         nfilters++;
         }
    }

/* Error message if no filter parameters, since Risley correction
will be impossible... */
if(nfilters == 0)
   {
#if LANGUAGE == FRENCH
      sprintf(error_message,
       " UpdateConfig/Erreur: les paramètres des filtres sont absents dans %s",
                TAVCONF);
#elif LANGUAGE == ITALIAN
      sprintf(error_message,
       " UpdateConfig/Errore: i parametri dei filtri sono assenti in %s",
                TAVCONF);
#else
      sprintf(error_message,
              " UpdateConfig/Error: no filter parameters in %s",TAVCONF);
#endif
      status =-1;
   }

fclose(fp);

return(status);
}
/************************************************************************
* Routine LoadLabels1
* to load the labels of the speckle camera wheels
* by reading the file TAVCONF="tavconfi.dat"
*
************************************************************************/
int TTestWindow::LoadLabels1(char *error_message)
{
  FILE *fp;
  char buffer[80];
  register int i, k;

  if((fp = fopen(TAVCONF,"r")) == NULL)
    {
     sprintf(error_message," LoadLabels1/File %s not found",TAVCONF);
     return(-1);
    }

  for(k = 0; k < NWHEELS; k++)
   {
    if(fgets(wheel_title[k],60,fp) == NULL)
    {
     sprintf(error_message," LoadLabels1/Error 1 reading %s",TAVCONF);
     fclose(fp); return (-1);
    }
    clean_buffer(wheel_title[k],60);
    if(!strncmp(wheel_title[k],"FA",2))
      for (i = 0; i < 4; i++) fgets(buffer,60,fp);
    else
      for (i = 0; i < 6; i++) fgets(buffer,60,fp);
    }

  fclose(fp);
 return(0);
 }
/********************************************************************
* Exit
*********************************************************************/
void TTestWindow::CloseSelection()
{
// Don't need to call CanClose, since automatically called by CloseWindow()
// TWindow::CanClose();
TWindow::CloseWindow();
// exit(-1);
}

/********************************************************************/
/*** Response table with all
     routines that can be called directly from menu or buttons
     (logbook.cpp, atmdlg.cpp, stardlg.cpp)
****/
DEFINE_RESPONSE_TABLE1( TTestWindow, TFrameWindow )
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL, HandleBoxMsg_wheel0 ),
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL + 1, HandleBoxMsg_wheel1 ),
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL + 2, HandleBoxMsg_wheel2 ),
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL + 3, HandleBoxMsg_wheel3 ),
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL + 4, HandleBoxMsg_wheel4 ),
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL + 5, HandleBoxMsg_wheel5 ),
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL + 6, HandleBoxMsg_wheel6 ),
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL + 7, HandleBoxMsg_wheel7 ),
    EV_CBN_SELCHANGE( ID_COMBO_WHEEL + 8, HandleBoxMsg_wheel8 ),
    EV_COMMAND( ID_UPDATE_RISLEY, HandleButton_UpdateRisley ),
    EV_COMMAND( ID_UPDATE_WHEELS, HandleButton_UpdateWheels ),
    EV_COMMAND( ID_BUTTON_RA, HandleButton_RA ),
    EV_COMMAND( ID_BUTTON_RB, HandleButton_RB ),
    EV_COMMAND( ID_RBUTTON_ON, HandleRButton_AutoRisley ),
    EV_COMMAND( ID_RBUTTON_OFF, HandleRButton_InteracRisley ),
    EV_COMMAND( ID_BUTTON_LAMP, HandleButton_Lamp ),
    EV_COMMAND( CM_CLOSE, CloseSelection ),
// From logbook.cpp ...
    EV_COMMAND( CM_CALIBR, CalibInput ),
    EV_COMMAND( CM_LOADPOSI, LoadPosition1 ),
    EV_COMMAND( CM_OPEN_LOGBOOK, OpenLogBook ),
    EV_COMMAND( CM_CLOSE_LOGBOOK, CloseLogBook ),
    EV_COMMAND( CM_COMMENTS_LOGBOOK, CommentsLogBook ),
    EV_COMMAND( CM_ADC_SETUP, ADC_Setup ),
    EV_COMMAND( CM_TELPOSI, TelescopePosition ),
    EV_COMMAND( CM_VERSION, VersionDisplay ),
    EV_COMMAND( CM_RS232DELAY, DelayInput ),
    EV_COMMAND( ID_BUTTON_EXPOSURE, HandleButton_Exposure ),
// From stardlg.cpp ...
    EV_COMMAND( CM_STARINPUT, StarInput ),
    EV_COMMAND( CM_STARCATALOG, CatalogInput ),
// From RS232win.cpp ...
//    EV_COMMAND( CM_RS232WIN, Open_RS232window ),
    EV_COMMAND( CM_RS232DIAL, RS232_Dialog ),
// From atmdlg.cpp ...
    EV_COMMAND( CM_ATMINPUT, AtmInput ),
    EV_WM_SIZE,
END_RESPONSE_TABLE;
