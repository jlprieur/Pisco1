/************************************************************************
* "WDS_catalog_utils.h"
* To retrieve data from the WDS catalog
*
* JLP 
* Version 12/02/2012
*************************************************************************/
#ifndef __WDS_catalog_utils   /* BOF sentry */
#define __WDS_catalog_utils   

int search_discov_name_in_WDS_catalog(char *WDS_catalog, char *discov_name,
                                      char *WDS_name, int *found);
int get_data_from_WDS_catalog(char *WDS_catalog, char *discov_name,
                              char *WDS_name, double *WY, double *WR,
                              double *WT, int *found);
int read_coordinates_from_WDS_catalog(char *WDS_name, char *WDS_catalog, 
                                      float *alpha, float *delta,
                                      float *equinox, int *found);

#endif   /* EOF sentry */
