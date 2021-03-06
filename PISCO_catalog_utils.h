/************************************************************************
* "PISCO_catalog_utils.h"
* Set of routines used to read PISCO catalog
*
* JLP 
* Version 21/06/2015
*************************************************************************/
#ifndef _PISCO_catalog_utils_h /* BOF sentry */
#define _PISCO_catalog_utils_h

#include <stdio.h>
#include <stdlib.h>                  /* exit() */
#include <string.h>
#include <ctype.h>                   /* isprint... */
#include <math.h>
#include <time.h>                    /* date */

#ifndef ABS
#define ABS(a) ((a) < 0.0  ? (-(a)) : (a))
#endif
#ifndef PI
#define PI 3.14159265
#endif
#define DEGTORAD   (PI/180.00)

/* Declaring linkage specification to have "correct names"
* that can be linked with C programs */

#ifdef __cplusplus
extern "C" {
#endif

/* PISCO catalog zeiss_doppie.cat with the list of objects used by TAV1.EXE */
int get_coordinates_from_PISCO_catalog(char *PISCO_catalog_name, 
                                       char *NameInPiscoCatalog,
                                       float *alpha, float *delta, 
                                       float *coord_equinox, int *has_an_orbit);
int get_data_from_PISCO_catalog(char *PISCO_catalog_name, 
                                char *NameInPiscoCatalog,
                                float *magV, float *B_V, 
                                float *paral, float *err_paral, float *magV_A, 
                                float *magV_B,
                                char *spectral_type, char *discov_name,
                                char *ads_name, char *WDS_name,
                                int *has_an_orbit);
int read_coordinates_from_PISCO_catalog(char *line_buffer, float *alpha, 
                                        float *delta, float *coord_equinox);
int PISCO_catalog_read_line1(char *in_line, char *object_name, char *comp_name,
                             char *ADS_name, float *alpha_Pcat, 
                             float *delta_Pcat, float *equinox_Pcat);
int PISCO_catalog_read_line2(char *in_line, float *magV_A, float *magV_B, 
                             char *spectral_type, int *has_an_orbit);
int PISCO_catalog_read_line2_discov(char *in_line, char *discov_name);
int PISCO_catalog_read_line2_WDS(char *in_line, char *WDS_name);
int PISCO_catalog_read_line2_Hip(char *in_line, float *magV, float *B_V,     
                                 float *paral, float *err_paral);

#ifdef __cplusplus
}
#endif


#endif
