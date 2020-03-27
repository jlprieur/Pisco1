/***************************************************************************
* psc1_rw_config_files
*
* JLP
* Version 17/08/2015
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psc1_typedef.h"
#include "psc1_rw_config_files.h"

//---------------------------------------------------------------------------
// Initialize structure PISCO_SETTINGS
//    (the prototype is declared in "psc1_typedef.h")
//---------------------------------------------------------------------------
int Init_PscSet(PISCO_SETTINGS& PscSet2) {

    PscSet2.TavConfigFileName[0] = '\0';

return 0;
}
//---------------------------------------------------------------------------
// Copy structure PISCO_SETTINGS
//    (the prototype is declared in "psc1_typedef.h")
// INPUT:
//   PscSet1
//
// OUTPUT:
//   PscSet2
//---------------------------------------------------------------------------
int Copy_PscSet(const PISCO_SETTINGS PscSet1, PISCO_SETTINGS& PscSet2)
{
    strcpy(PscSet2.FwhmSelect.Stats, PscSet1.FwhmSelect.Stats);
    PscSet2.FwhmSelect.LowCut = PscSet1.FwhmSelect.LowCut;
    strcpy(PscSet2.TavConfigFileName, PscSet1.TavConfigFileName);

return 0;
}
/***************************************************************************
*
***************************************************************************/
int ReadBoolFromConfigFile(char *filename, char *keyword, bool *bvalue)
{
int status, ivalue = 0;
char buffer[80];

 status = ReadStringFromConfigFile(filename, keyword, buffer);
 if(!status) sscanf(buffer, "%d", &ivalue);

 if(ivalue)
  *bvalue = true;
 else
  *bvalue = false;

return(status);
}
/***************************************************************************
*
***************************************************************************/
int ReadIntFromConfigFile(char *filename, char *keyword, int *ivalue)
{
int status;
char buffer[80];
*ivalue = 0;
 status = ReadStringFromConfigFile(filename, keyword, buffer);
 if(!status) sscanf(buffer, "%d", ivalue);

return(status);
}
/***************************************************************************
*
***************************************************************************/
int ReadDoubleFromConfigFile(char *filename, char *keyword, double *dvalue)
{
int status;
char buffer[80];
*dvalue = 0.;
 status = ReadStringFromConfigFile(filename, keyword, buffer);
 if(!status) sscanf(buffer, "%lf", dvalue);

return(status);
}
/***************************************************************************
*
***************************************************************************/
int ReadStringFromConfigFile(char *filename, char *keyword, char *cvalue)
{
int status = -1, found = 0;
FILE *fp_in;
char buffer[128], *pc;

strcpy(cvalue, "");

if((fp_in = fopen(filename,"r"))== NULL) {
  fprintf(stderr, "ReadStringFromConfigFile/error opening input file >%s< \n",
          filename);
  return(-1);
  }

while(!found && !feof(fp_in)) {
  fgets(buffer, 80, fp_in);
  pc = buffer;
  while(*pc && (*pc != '=') && strncmp(pc, keyword, strlen(keyword))) pc++;
  if(!strncmp(pc, keyword, strlen(keyword))) {
    found = 1;
    while(*pc && (*pc != '=')) pc++;
     if(*pc == '=') {
        pc++;
        strcpy(cvalue, pc);
        status = 0;
       }
    }

 } // EOF while(!found)

fclose(fp_in);

// Cleanup cvalue:
strcpy(buffer, cvalue);
// Remove blanks at the beginning:
pc = cvalue;
  while(*pc && (*pc == ' ')) pc++;
strcpy(cvalue, pc);

// Remove cr or line feed
pc = cvalue;
  while(*pc && (*pc != '\r') && (*pc != '\n')) pc++;
  *pc = '\0';

return(status);
}
