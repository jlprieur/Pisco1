/****************************************************************************
* Name: psc1_frame_id.h  (PscFrame class for Pisco1)
* Purpose: display and process FITS images obtained with PISCO and PISCO2
*
* list of ID used by PscFrame class
*
* JLP
* Version 07/01/2015
****************************************************************************/
#ifndef _psc1_frame_id_h_
#define _psc1_frame_id_h_

//---------------------------------------------------------------------
enum
{
  ID_QUIT        = 1000,
  ID_CLOSE,

// File:
  ID_SAVE_TO_3DFITS,
  ID_SAVE_RESULTS2,

  // Toolbar buttons:
  ID_START_PROCESS,
  ID_STOP_PROCESS,
  ID_START_STREAM,
  ID_STOP_STREAM,
  ID_SAVE_RESULTS,
  ID_STATISTICS,
  ID_BISP_RESTORE,
  ID_ERASE_ALL,

// Menu/Notebook:
  ID_NOTEBOOK,
  ID_PARAM_PANEL,
  ID_TARGET_PANEL,
  ID_PISCO_PANEL,

// Context menu
  ID_CONTEXT_HELP,

// Display menu
  ID_DISPMENU_DEFAULT,
  ID_DISPMENU_OFFSET,
  ID_DISPMENU_FFIELD,
  ID_DISPMENU_UNRES_AUTOC,
  ID_DISPMENU_UNRES_MODSQ,
  ID_DISPMENU_OPT4,
  ID_DISPMENU_OPT5,
  ID_DISPMENU_OPT6,
  ID_DISPMENU_OPT7,
  ID_DISPMENU_OPT8,
  ID_DISPMENU_OPT9,
  ID_DISPMENU_OPT10,

// Logbook menu
  ID_LOGBOOK_SHOW,
  ID_LOGBOOK_HIDE,
  ID_LOGBOOK_COMMENTS,
  ID_LOGBOOK_SAVE,

// Wheels: combo boxes
  ID_PISCO_WHEEL_0,
  ID_PISCO_WHEEL_1,
  ID_PISCO_WHEEL_2,
  ID_PISCO_WHEEL_3,
  ID_PISCO_WHEEL_4,
  ID_PISCO_WHEEL_5,
  ID_PISCO_WHEEL_6,
  ID_PISCO_WHEEL_7,
  ID_PISCO_WHEEL_8,

// Wheels, Risley prisms, etc
  ID_PISCO_CHECK_POSI,
  ID_PISCO_LAMP_ON,
  ID_PISCO_EXPO_START,
  ID_PISCO_RA_VALID,
  ID_PISCO_RB_VALID,
  ID_PISCO_AUTO_RISLEY,
  ID_PISCO_RISLEY_TIMER,
  ID_PISCO_RS232_TIMER,

// RS232 link:
  ID_RS232_SEND,
  ID_RS232_SEND_WOCR,
  ID_RS232_RECBOX,

// Param Panel:
  ID_PARAM_TEMPERATURE,
  ID_PARAM_PRESSURE,
  ID_PARAM_HYGROMETRY,
  ID_PARAM_RA_OFFSET,
  ID_PARAM_RB_OFFSET,
  ID_PARAM_RA_SIGN,
  ID_PARAM_RB_SIGN,
  ID_PARAM_UTSHIFT,
  ID_PARAM_VALIDATE,
  ID_PARAM_CANCEL,

// Target Panel:
  ID_TARGET_TYPE,
  ID_TARGET_STAR_NAME,
  ID_TARGET_ALPHA,
  ID_TARGET_DELTA,
  ID_TARGET_EQUINOX,
  ID_TARGET_OPENCAT1,
  ID_TARGET_OPEN_ADSWDS,
  ID_TARGET_FIND_OBJECT,
  ID_TARGET_ALPHA0,
  ID_TARGET_ALPHARANG,
  ID_TARGET_DELTAMIN,
  ID_TARGET_DELTAMAX,
  ID_TARGET_MAGMIN,
  ID_TARGET_MAGMAX,
  ID_TARGET_RHOMIN,
  ID_TARGET_RHOMAX,
  ID_TARGET_NEXT,
  ID_TARGET_PREV,
  ID_TARGET_SELECT,
  ID_TARGET_FIND_ALL,
  ID_TARGET_VALIDATE,
  ID_TARGET_CANCEL,

// Help:
  ID_ABOUT,
  ID_HELP,
};

#endif
