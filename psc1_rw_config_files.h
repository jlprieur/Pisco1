/***************************************************************************
* psc1_rw_config_files
*
* JLP
* Version 17/08/2015
***************************************************************************/
#ifndef psc1_rw_config_files_
#define psc1_rw_config_files_

#include "psc1_typedef.h"

/* Routines accessible to all modules */

 int Init_PscSet(PISCO_SETTINGS& PscSet2);
 int Copy_PscSet(const PISCO_SETTINGS PscSet1, PISCO_SETTINGS& PscSet2);

 int ReadBoolFromConfigFile(char *filename, char *keyword, bool *bvalue);
 int ReadIntFromConfigFile(char *filename, char *keyword, int *ivalue);
 int ReadDoubleFromConfigFile(char *filename, char *keyword, double *dvalue);
 int ReadStringFromConfigFile(char *filename, char *keyword, char *cvalue);

#endif
