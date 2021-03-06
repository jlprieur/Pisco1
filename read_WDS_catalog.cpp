/************************************************************************
* "raed_WDS_catalog.c"
* To retrieve data from the WDS catalog
*
* JLP
* Version 20/11/2009
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>                /* exit() */
#include <string.h>
#include <ctype.h>                 /* isprint... */
#include <math.h>
#include <time.h>                  /* date */
#include "psc1_defs.h"               // Binary_Parameters ...
#include "jlp_string.h"            // jlp_compact_string,...

#include "read_WDS_catalog.h"     /* Prototypes of the routines defined here */

#define ABS(a) ((a) < 0.0  ? (-(a)) : (a))
#ifndef PI
#define PI 3.14159265
#endif
#define DEGTORAD   (PI/180.00)


/*
#define DEBUG
#define DEBUG_1
*/

/* Defined here:
int search_discov_name_in_WDS_catalog(char *WDS_catalog, char *discov_name,
                                      char *WDS_name, int *found);
int get_data_from_WDS_catalog(char *WDS_catalog, char *discov_name,
                              char *WDS_name, double *WY, double *WR,
                              double *WT, int *found);
int read_coordinates_from_WDS_catalog(char *WDS_name, char *WDS_catalog,
                                      float *alpha, float *delta,
                                      float *equinox, int *found);
*/
static int WDS_name_to_coordinates(char *WDS_name0, double *alpha0,
                                   double *delta0, char *err_message);
static int read_WDS_precise_coordinates(char *cat_line0, double *alpha0,
                                        double *delta0, char *err_message);

/*************************************************************************
* Format of WDS catalog (version of 2009-2012):
*
  COLUMN     Format                     DATA
  --------   ------         ----------------------------
  1  -  10   A10             2000 Coordinates
  11 -  17   A7              Discoverer & Number
  18 -  22   A5              Components
  24 -  27   I4              Date (first)
  29 -  32   I4              Date (last)
  34 -  37   I4              Number of Observations (up to 9999)
  39 -  41   I3              Position Angle (first - XXX)
  43 -  45   I3              Position Angle (last  - XXX)
  47 -  51   F5.1            Separation (first)
  53 -  57   F5.1            Separation (last)
  59 -  63   F5.2            Magnitude of First Component
  65 -  69   F5.2            Magnitude of Second Component
  71 -  79   A9              Spectral Type (Primary/Secondary)
  81 -  84   I4              Primary Proper Motion (RA)
  85 -  88   I4              Primary Proper Motion (Dec)
  90 -  93   I4              Secondary Proper Motion (RA)
  94 -  97   I4              Secondary Proper Motion (Dec)
  99 - 106   A8              Durchmusterung Number
 108 - 111   A4              Notes
 113 - 130   A18             2000 arcsecond coordinates
*
* INPUT:
*  WDS_catalog: name of the WDS catalog
*  discov_name: discoverer's name of the object to be searched for
*
* OUTPUT:
*  WDS_name: WDS name corresponding to discov_name
*  found: 1 is object was found, 0 otherwise
*************************************************************************/
int search_discov_name_in_WDS_catalog(char *WDS_catalog, char *discov_name2,
                                      char *comp_name2, char *WDS_name2,
                                      double *alpha2, double *delta2,
                                      int *found, char *err_message)
{
FILE *fp_WDS_cat;
char cat_line0[256], discov_name0[20], comp_name0[20], WDS_name0[20];
char discov_name1[20], comp_name1[20], buff1[128];
int iline, status;

WDS_name2[0] = '\0';
*alpha2 = 0.;
*delta2 = 0.;

// Removes all the blanks
strncpy(discov_name1, discov_name2, 20);
jlp_compact_string(discov_name1, 20);
strncpy(comp_name1, comp_name2, 20);
jlp_compact_string(comp_name1, 20);
if(comp_name1[0] == '\0') strcpy(comp_name1, "AB");

/* Open input file containing the WDS catalog */
if((fp_WDS_cat = fopen(WDS_catalog, "r")) == NULL) {
   sprintf(err_message, "search_discov_name_in_WDS_catalog/Fatal error opening WDS catalog: %s\n",
           WDS_catalog);
   return(-1);
  }

/* Look for the data concerning this object: */
*found = 0;
iline = 0;
while(!feof(fp_WDS_cat)) {
 if(fgets(cat_line0, 256, fp_WDS_cat)) {
   iline++;
   if(cat_line0[0] != '%') {
/*
  1  -  10   A10             2000 Coordinates
  11 -  17   A7              Discoverer & Number
  18 -  22   A5              Components
*/
   strncpy(WDS_name0, &cat_line0[0], 10);
   WDS_name0[10] = '\0';
/* 7 characters for WDS, 8 characters for Marco's file */
   strncpy(discov_name0, &cat_line0[10], 7);
   discov_name0[7] = '\0';
   jlp_compact_string(discov_name0, 20);

   strncpy(comp_name0, &cat_line0[17], 5);
   comp_name0[5] = '\0';
   jlp_compact_string(comp_name0, 20);
   if(comp_name0[0] == '\0') strcpy(comp_name0, "AB");

/* DEBUG
{ wxString buff;
buff.Printf(wxT("iline=%d name=%s (%s) comp=%s (%s)"), iline,
            discov_name1, discov_name0, comp_name1, comp_name0);
wxMessageBox(buff, wxT("ZZZFIND"), wxOK);
if(iline >= 8)return(-1);
}
*/

// Check first if discoverer is the same:
   if(!strcmp(discov_name1, discov_name0)) {
// Then check if companion is the same:
       if(!strcmp(comp_name0, comp_name1)) {
#ifdef DEBUG_1
     printf("Object found in WDS catalog (discov_name =%s)\n", discov_name);
     printf(" WDS=%s discov=%s comp=%s\n", WDS_name0, discov_name0, comp_name0);
#endif
       strcpy(WDS_name2, WDS_name0);
       *found = 1;

       status = read_WDS_precise_coordinates(cat_line0, alpha2, delta2,
                                             buff1);
       if(status) WDS_name_to_coordinates(WDS_name0, alpha2, delta2, buff1);
       break;
      } /// comp_name is OK
     } // discov_name is OK
   } /* EOF cat_line[0] != '%' */
  } /* EOF fgets... */
} /* EOF while */

fclose(fp_WDS_cat);
return(0);
}
/*************************************************************************
* Search in the WDS catalog
*
* Format of WDS catalog (version of 2009-2012):
*
  COLUMN     Format                     DATA
  --------   ------         ----------------------------
  1  -  10   A10             2000 Coordinates
  11 -  17   A7              Discoverer & Number
  18 -  22   A5              Components
  24 -  27   I4              Date (first)
  29 -  32   I4              Date (last)
  34 -  37   I4              Number of Observations (up to 9999)
  39 -  41   I3              Position Angle (first - XXX)
  43 -  45   I3              Position Angle (last  - XXX)
  47 -  51   F5.1            Separation (first)
  53 -  57   F5.1            Separation (last)
  59 -  63   F5.2            Magnitude of First Component
  65 -  69   F5.2            Magnitude of Second Component
  71 -  79   A9              Spectral Type (Primary/Secondary)
  81 -  84   I4              Primary Proper Motion (RA)
  85 -  88   I4              Primary Proper Motion (Dec)
  90 -  93   I4              Secondary Proper Motion (RA)
  94 -  97   I4              Secondary Proper Motion (Dec)
  99 - 106   A8              Durchmusterung Number
 108 - 111   A4              Notes
 113 - 130   A18             2000 arcsecond coordinates
*
*
* INPUT:
*  WDS_catalog: name of the WDS catalog
*  bpr1: containing alpha_min, alpha_max, delta_min, delta_max, etc
*
* OUTPUT:
*  WDS_name: WDS name corresponding to discov_name
*  bpa1: containing parameters of the object found:
*      discov_name: discoverer's name
*      WY, WR, WT: year, rho, theta of the last observation reported in the WDS
*  found: 1 is an object was found, 0 otherwise
***********************************************************************/
int search_in_WDS_catalog(char *WDS_catalog, Binary_Profile bpr1,
                          Binary_Parameters *bpa1, int *found,
                          const bool search_all_and_save, char *save_fname,
                          char *err_message)
{
FILE *fp_WDS_cat, *fp_save;
char cat_line0[256], discov_name0[20], comp_name0[20], WDS_name0[20];
char cvalue[64], buff1[256];
int iline, ivalue;
double alpha0, alpha_min, alpha_max, delta0, delta_min, delta_max;
double dvalue;
int status;

bpa1->WDS_name = wxT("");
bpa1->Companion_name = wxT("");
bpa1->Discover_name = wxT("");
bpa1->epoch = 0.;
bpa1->rho = 0.;
bpa1->theta = 0.;
bpa1->alpha = 0.;
bpa1->delta = 0.;
bpa1->mag = -1.;
bpa1->dmag = -1.;

// Open input file containing the WDS catalog
if((fp_WDS_cat = fopen(WDS_catalog, "r")) == NULL) {
   fprintf(stderr, "get_data_from_WDS_catalog/Fatal error opening WDS catalog: %s\n",
           WDS_catalog);
   return(-1);
  }

// Open output file with the found selection
if(search_all_and_save) {
  if((fp_save = fopen(save_fname, "w")) == NULL) {
   fprintf(stderr, "get_data_from_WDS_catalog/Fatal error opening output file: %s\n",
           save_fname);
   return(-1);
  }
}

// Look for the data concerning this object:
*found = 0;
iline = 0;
while(!feof(fp_WDS_cat)) {
 if(fgets(cat_line0, 256, fp_WDS_cat)) {
   iline++;
   if(cat_line0[0] != '%') {
/*
  1  -  10   A10             2000 Coordinates
  11 -  17   A7              Discoverer & Number
  18 -  22   A5              Components
Columns   1- 10:    The  hours, minutes, and tenths of minutes of Right
                    Ascension for 2000, followed by the degrees and minutes of
                    Declination for 2000, with + and - indicating north and
                    south declinations. The positions given represent our best
                    estimates of these values. Where possible, these are based
                    on the ACRS and PPM data, with proper motion incorporated.

0123456789
hhmmn+ddmm
12345+1234
*/
   strncpy(WDS_name0, &cat_line0[0], 10);
   WDS_name0[10] = '\0';

// WDS name:
   bpa1->WDS_name = wxString(WDS_name0);

   status = WDS_name_to_coordinates(WDS_name0, &alpha0, &delta0, err_message);
// Save coordinates (temporarily, since they may be improved later...)
   bpa1->alpha = alpha0;
   bpa1->delta = delta0;

   alpha_min = bpr1.alpha_min;
   alpha_max = alpha_min + bpr1.alpha_range;

   delta_min = bpr1.delta_min;
   delta_max = bpr1.delta_max;

// DEBUG:
/*
{ wxString buffer;
   buffer.Printf(wxT("iline=%d WDS=%s alpha0=%.1f alpha_min=%.1f alpha_max=%.1f\n\
delta0=%.1f delta_min=%.1f delta_max=%.1f\n\
rho=%.1f rho_min=%.1f rho_max=%.1f mag=%.1f mag_min=%.1f mag_max=%.1f"),
                 iline, WDS_name0, alpha0, alpha_min, alpha_max, delta0,
                 delta_min, delta_max, bpa1->rho, bpr1.rho_min, bpr1.rho_max,
                 bpa1->mag, bpr1.mag_min, bpr1.mag_max);
   wxMessageBox(buffer, bpa1->WDS_name, wxOK);
   if(iline >= 7) return(-1);
}
*/

// First selection with the raw coordinates:
   if((alpha0 > alpha_min) && (alpha0 <= alpha_max)
       && (delta0 > delta_min) && (delta0 <= delta_max)) {

// Discoverer's name:
   strncpy(discov_name0, &cat_line0[10], 7);
   discov_name0[7] = '\0';
   bpa1->Discover_name = wxString(discov_name0);

// Companion name
   strncpy(comp_name0, &cat_line0[17], 5);
   comp_name0[5] = '\0';
   bpa1->Companion_name = wxString(comp_name0);

/* Read WY, WR, WT:
  24 -  27   I4              Date (first)
  29 -  32   I4              Date (last)
  34 -  37   I4              Number of Observations (up to 9999)
  39 -  41   I3              Position Angle (first - XXX)
  43 -  45   I3              Position Angle (last  - XXX)
  47 -  51   F5.1            Separation (first)
  53 -  57   F5.1            Separation (last)
*/
// (last) year
     strncpy(cvalue, &cat_line0[28], 4);
     cvalue[4] = '\0';
     if(sscanf(cvalue, "%d", &ivalue) == 1) bpa1->epoch = (double)ivalue;

// (last) theta
     strncpy(cvalue, &cat_line0[42], 3);
     cvalue[3] = '\0';
     if(sscanf(cvalue, "%d", &ivalue) == 1) bpa1->theta = (double)ivalue;

// (last) rho
     strncpy(cvalue, &cat_line0[52], 5);
     cvalue[5] = '\0';
     if(sscanf(cvalue, "%lf", &dvalue) == 1) bpa1->rho = (double)dvalue;

/*
  59 -  63   F5.2            Magnitude of First Component
  65 -  69   F5.2            Magnitude of Second Component
  71 -  79   A9              Spectral Type (Primary/Secondary)
*/
     strncpy(cvalue, &cat_line0[58], 5);
     cvalue[5] = '\0';
     if(sscanf(cvalue, "%lf", &dvalue) == 1) bpa1->mag = (double)dvalue;
       else wxMessageBox(wxString(cvalue), wxT("WDS catalog/Error reading magnitude"), wxOK);
     strncpy(cvalue, &cat_line0[64], 5);
     cvalue[5] = '\0';
     if(sscanf(cvalue, "%lf", &dvalue) == 1) bpa1->dmag = (double)dvalue - bpa1->mag;

// Object is found when the last (following) criteria are verified:
     if((bpa1->rho >= bpr1.rho_min) && (bpa1->rho <= bpr1.rho_max)
         && (bpa1->mag >= bpr1.mag_min) && (bpa1->mag <= bpr1.mag_max)) {

       status = read_WDS_precise_coordinates(cat_line0, &alpha0, &delta0,
                                             buff1);
       if(status) {
       sprintf(err_message, " %s of WDS%s in line%d\n",
               buff1, WDS_name0, iline);
       } else {
        bpa1->alpha = alpha0;
        bpa1->delta = delta0;
       }

// Second criterium (to allow NEXT to work):
// Last selection with the precise coordinates:
     if((alpha0 > alpha_min) && (alpha0 <= alpha_max)
       && (delta0 > delta_min) && (delta0 <= delta_max)) {
//       wxMessageBox(wxT("OK: found!"), bpa1->WDS_name, wxOK);
       *found = 1;
       if(search_all_and_save){
          fprintf(fp_save, "%s", cat_line0);
          } else {
            break;
          }
       }
     } // EOF  mag_min <= mag <= mag_max
    } // EOF  alpha_min <= alpha <= alpha_max
   } // EOF not a commented line starting with '%'
  } // EOF fgets...
} // EOF while

fclose(fp_WDS_cat);
if(search_all_and_save) fclose(fp_save);
return(0);
}
/***********************************************************************
* Look for raw coordinates in WDS catalog
*
Columns   1- 10:    The  hours, minutes, and tenths of minutes of Right
                    Ascension for 2000, followed by the degrees and minutes of
                    Declination for 2000, with + and - indicating north and
                    south declinations. The positions given represent our best
                    estimates of these values. Where possible, these are based
                    on the ACRS and PPM data, with proper motion incorporated.
* Example:
06019+6052         in 1-10
*********************************************************************************/
static int WDS_name_to_coordinates(char *WDS_name0, double *alpha0,
                                   double *delta0, char *err_message)
{
char sign[1];
int status = -1, hh, htm, dd, dm;

 if(sscanf(WDS_name0, "%02d%03d%c%02d%02d", &hh, &htm, sign, &dd, &dm) == 5) {
    *alpha0 = (double)hh + ((double)htm)/600.;
    *delta0 = (double)dd + ((double)dm)/60.;
    if(sign[0] == '-') *delta0 *= -1.;
    status = 0;
    } else {
      sprintf(err_message,"read_WDS_precise_coordinates/Error reading >%s<",
              WDS_name0);
    }

return(status);
}
/***********************************************************************
* Look for accurate coordinates in WDS catalog
*
Columns   1- 10:    The  hours, minutes, and tenths of minutes of Right
                    Ascension for 2000, followed by the degrees and minutes of
                    Declination for 2000, with + and - indicating north and
                    south declinations. The positions given represent our best
                    estimates of these values. Where possible, these are based
                    on the ACRS and PPM data, with proper motion incorporated.

Columns 113-130:    The hours, minutes, seconds and tenths of seconds (when
                    known) of Right Ascension for equinox=2000, followed by the degrees,
                    minutes, and seconds of Declination for 2000, with + and -
                    indicating north and south declinations. The positions given
                    represent our best estimates of these values. Where
                    possible, these are based on the Hipparcos and Tycho data,
                    with proper motion incorporated. While the arcminute
                    coordinate (columns 1-10) refer to the primary of a multiple
                    system, the arcsecond coordinate (columns 113-130) refer to
                    the primary of the subsystem. For example, while the BC pair
                    of an A-BC multiple will have the same 10 digit WDS
                    coordinate, the arcsecond coordinate of the BC pair will be
                    at the "B" position.
* Example:
06019+6052         in 1-10
060156.93+605244.8 in 113-130
*********************************************************************************/
static int read_WDS_precise_coordinates(char *cat_line0, double *alpha0,
                                        double *delta0, char *err_message)
{
char cvalue[64], sign[1];
int status = -1, hh, hm, hs, hss, dd, dm, ds, dss;

/*
  113 - 130   A18            2000 precise coordinates
Example:
060156.93+605244.8 in 113-130
*/
 strncpy(cvalue, &cat_line0[112], 18);
 cvalue[18] = '\0';
 if(sscanf(cvalue, "%02d%02d%02d.%02d%c%02d%02d%02d.%d",
    &hh, &hm, &hs, &hss, sign, &dd, &dm, &ds, &dss) == 9) {
    *alpha0 = (double)hh + ((double)hm)/60.
              + ((double)hs + (double)hss/10.)/3600.;
    *delta0 = (double)dd + ((double)dm)/60.
              + ((double)ds + (double)dss/10.)/3600.;
    if(sign[0] == '-') *delta0 *= -1.;
    status = 0;
    } else {
      sprintf(err_message,"read_WDS_precise_coordinates/Error reading >%s<",
              cvalue);
    }

return(status);
}
/************************************************************************
* Conversion of WDS name to ADS (if possible)
*
* INPUT:
* WDS_name1, Discov_name1: WDS name
* ADS_WDS_cross: name of the ADS/WDS cross-reference file
*
* OUTPUT:
* ADS_name1: ADS name
**************************************************************************/
int  WDS_to_ADS_name(char *WDS_name2, char *Discov_name2, char *ADS_name2,
                     char *ADS_WDS_cross, int *found, char *err_message)
{
char in_line[80], ADS_name0[20], WDS_name0[20], discov_name0[20];
char WDS_name1[20], Discov_name1[20];
int iline, status = -1;
FILE *fp_ADS_WDS_cross;
register int i;

strcpy(Discov_name1, Discov_name2);
jlp_compact_string(Discov_name1, 20);
strcpy(WDS_name1, WDS_name2);
jlp_compact_string(WDS_name1, 20);

strcpy(ADS_name2, "");
*found = 0;

/* Open input file containing the ADS_WDS cross-references */
if((fp_ADS_WDS_cross = fopen(ADS_WDS_cross, "r")) == NULL) {
   sprintf(err_message, "WDS_to_ADS_name/Error opening ADS-WDS cross-ref.: %s\n",
           ADS_WDS_cross);
   return(-1);
  }


/* Scan the cross-reference file: */
/* Syntax:
BDS         ADS     Discovr Comp         WDS       Omit?

13663     17158     A  1248           00000+7530
12704         1     STF3053  AB       00026+6606
13664     17180     BGH   1  AB-C     00024+1047

ADS numbers in fields 11 to 15
Discov names in fields 21 to 27
Components in fields 30 to 36
WDS numbers in fields 39 to 48
(NB: C arrays start at 0, hence should remove one from field numbers)
*/
iline = 0;
while(!feof(fp_ADS_WDS_cross)) {
  if(fgets(in_line, 120, fp_ADS_WDS_cross)) {
    iline++;
    if(in_line[0] != '%') {

// ADS name:
      for(i = 0; i < 6; i++) ADS_name0[i] = in_line[10+i];
      ADS_name0[6] = '\0';
      jlp_trim_string(ADS_name0, 20);

// Discoverer:
      for(i = 0; i < 7; i++) discov_name0[i] = in_line[20+i];
      discov_name0[7] = '\0';
      jlp_compact_string(discov_name0, 20);

// WDS name:
      for(i = 0; i < 10; i++) WDS_name0[i] = in_line[38+i];
      WDS_name0[10] = '\0';

// DEBUG:
/*
{
  wxString buff;
  buff.Printf(wxT("ADS=%s WDS=%s (%s) Disc=%s (%s)"),
              ADS_name0, WDS_name1, WDS_name0,
              Discov_name1, discov_name0);
  if(iline > 7 && iline < 11) wxMessageBox(buff, wxT("ZZZ"), wxOK);
}
*/
// Break from loop when found:
      if(!strcmp(WDS_name1, WDS_name0)
         && !strcmp(Discov_name1, discov_name0)) {
        strcpy(ADS_name2, ADS_name0);
        *found = 1;
        break;
        }
      } /* EOF inline[0] == '%" */
    } /* EOF fgets */
} /* EOF while */

/* Close cross-reference file */
fclose(fp_ADS_WDS_cross);
return(0);
}
