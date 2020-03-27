/************************************************************************
* "WDS_catalog_utils.c"
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
#include "jlp_catalog_utils.h"     /* Routines used to read catalogs:
                                    compact_string,... */

#include "WDS_catalog_utils.h"     /* Prototypes of the routines defined here */ 

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
int search_discov_name_in_WDS_catalog(char *WDS_catalog, char *discov_name,
                                      char *WDS_name, int *found)
{
FILE *fp_WDS_cat;
char cat_line0[256], discov_name0[20], comp_name0[20], WDS_name0[20];
int iline;

/* Removes all the blanks since 7 characters for WDS, and 8 characters 
* for Marco's file */
compact_string(discov_name, 20);

/* Open input file containing the WDS catalog */
if((fp_WDS_cat = fopen(WDS_catalog, "r")) == NULL) {
   fprintf(stderr, "search_discov_name_in_WDS_catalog/Fatal error opening WDS catalog: %s\n",
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
   strncpy(comp_name0, &cat_line0[17], 5);
   comp_name0[5] = '\0';

/* Removes all the blanks since 7 characters for WDS, and 8 characters 
* for Marco's file */
   compact_string(discov_name0, 20);
   if(!strcmp(discov_name, discov_name0)) {
#ifdef DEBUG_1
     printf("Object found in WDS catalog (discov_name =%s)\n", discov_name);
     printf(" WDS=%s discov=%s comp=%s\n", WDS_name0, discov_name0, comp_name0);
#endif
     strcpy(WDS_name, WDS_name0);
     *found = 1;
     break;
     }
   } /* EOF cat_line[0] != '%' */
  } /* EOF fgets... */
} /* EOF while */

fclose(fp_WDS_cat);
return(0);
}
/***********************************************************************
* Get miscellaneous data from the WDS catalog
*
* INPUT:
*  WDS_catalog: name of the WDS catalog
*  discov_name: discoverer's name of the object to be searched for
*
* OUTPUT:
*  WDS_name: WDS name corresponding to discov_name
*  WY, WR, WT: year, rho, theta of the last observation reported in the WDS
*  found: 1 is object was found, 0 otherwise
***********************************************************************/
int get_data_from_WDS_catalog(char *WDS_catalog, char *discov_name,
                              char *WDS_name, double *WY, double *WR,
                              double *WT, int *found)
{
FILE *fp_WDS_cat;
char cat_line0[256], discov_name0[20], comp_name0[20], WDS_name0[20];
char cvalue[64];
double dvalue;
int iline, ivalue;

*WY = 0.;
*WR = 0.;
*WT = 0.;

/* Removes all the blanks since 7 characters for WDS, and 8 characters 
* for Marco's file */
compact_string(discov_name, 20);

/* Open input file containing the WDS catalog */
if((fp_WDS_cat = fopen(WDS_catalog, "r")) == NULL) {
   fprintf(stderr, "get_data_from_WDS_catalog/Fatal error opening WDS catalog: %s\n",
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
   strncpy(comp_name0, &cat_line0[17], 5);
   comp_name0[5] = '\0';

/* Removes all the blanks since 7 characters for WDS, and 8 characters 
* for Marco's file */
   compact_string(discov_name0, 20);
   if(!strcmp(discov_name, discov_name0)) {
#ifdef DEBUG_1
     printf("Object found in WDS catalog (discov_name =%s)\n", discov_name);
     printf(" WDS=%s discov=%s comp=%s\n", WDS_name0, discov_name0, comp_name0);
#endif
     strcpy(WDS_name, WDS_name0);
/* Read WY, WR, WT: 
  24 -  27   I4              Date (first)
  29 -  32   I4              Date (last)
  34 -  37   I4              Number of Observations (up to 9999)
  39 -  41   I3              Position Angle (first - XXX)
  43 -  45   I3              Position Angle (last  - XXX)
  47 -  51   F5.1            Separation (first)
  53 -  57   F5.1            Separation (last)
*/
/* (last) year */
     strncpy(cvalue, &cat_line0[28], 4);
     cvalue[4] = '\0';
     if(sscanf(cvalue, "%d", &ivalue) == 1) *WY = ivalue;

/* (last) theta */
     strncpy(cvalue, &cat_line0[42], 3);
     cvalue[3] = '\0';
     if(sscanf(cvalue, "%d", &ivalue) == 1) *WT = ivalue;

/* (last) rho */
     strncpy(cvalue, &cat_line0[52], 5);
     cvalue[5] = '\0';
     if(sscanf(cvalue, "%lf", &dvalue) == 1) *WR = dvalue;

     *found = 1;
     break;
     }
   } /* EOF cat_line[0] != '%' */
  } /* EOF fgets... */
} /* EOF while */

fclose(fp_WDS_cat);
return(0);
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
                    known) of Right Ascension for 2000, followed by the degrees,
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
*
* INPUT:
*  WDS_name: WDS name of the object 
*  WDS_catalog: name of the WDS catalog
*
* OUTPUT:
* alpha, delta, equinox: coordinates of the object
*            (alpha in hours and delta in degrees)
************************************************************************/
int read_coordinates_from_WDS_catalog(char *WDS_name, char *WDS_catalog, 
                                      float *alpha, float *delta,
                                      float *equinox, int *found)
{
FILE *fp_WDS_cat;
char cat_line0[256], WDS_name0[40], cvalue[64], sign[1];
int iline, hh, hm, hs, hss, dd, dm, ds, dss;

*alpha = 0.;
*delta = 0.;
*equinox = 2000.;

/* Removes all the blanks since 10 characters for WDS */ 
compact_string(WDS_name, 40);

/* Open input file containing the WDS catalog */
if((fp_WDS_cat = fopen(WDS_catalog, "r")) == NULL) {
   fprintf(stderr, "read_coordinates_in_WDS_catalog/Fatal error opening WDS catalog: %s\n",
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
  1  -  10   A10             2000 Coordinates (= WDS name)
  11 -  17   A7              Discoverer & Number
  18 -  22   A5              Components
*/
   strncpy(WDS_name0, &cat_line0[0], 10);
   WDS_name0[10] = '\0';

   if(!strcmp(WDS_name, WDS_name0)) {
     *found = 1;
#ifdef DEBUG
     printf("Object found in WDS catalog (WDS_name =%s)\n", WDS_name);
#endif

/* Read WY, WR, WT: 
  113 - 130   A18            2000 precise coordinates
Example:
060156.93+605244.8 in 113-130
*/
/* (last) year */
     strncpy(cvalue, &cat_line0[112], 18);
     cvalue[18] = '\0';
     if(sscanf(cvalue, "%02d%02d%02d.%02d%c%02d%02d%02d.%d", 
        &hh, &hm, &hs, &hss, sign, &dd, &dm, &ds, &dss) == 9) {
        *alpha = (float)hh + ((float)hm)/60. 
                + ((float)hs + (float)hss/10.)/3600.;
        *delta = (float)dd + ((float)dm)/60. 
                + ((float)ds + (float)dss/10.)/3600.;
        if(sign[0] == '-') *delta *= -1.;
        else if(sign[0] != '+') {
           fprintf(stderr,"read_coordinates/Fatal error: sign=%s\n", sign);
           exit(-1);
           }
#ifdef DEBUG_1
     printf("WDS%s : %02d%02d%02d.%02d%s%02d%02d%02d.%d\n",
            WDS_name, hh, hm, hs, hss, sign, dd, dm, ds, dss); 
     printf("WDS%s : alpha=%f delta=%f \n", WDS_name, *alpha, *delta);
#endif
     } else {
       fprintf(stderr,"read_coordinates_from_WDS_catalog/Error in catalog\n");
       fprintf(stderr,"Warning: error reading coordinates: >%s< of WDS%s in line%d\n",
               cvalue, WDS_name, iline);
     } 

     *found = 1;
     break;
     }
   } /* EOF cat_line[0] != '%' */
  } /* EOF fgets... */
} /* EOF while */

fclose(fp_WDS_cat);
return(0);
}
 
