/*********************************************************
* Prototypes of routines used by Pisco1.cpp
*
* JLP
* Version 09-12-2015
********************************************************/
#ifndef tav_utils_h_
#define tav_utils_h_

#define PI 3.14159265
#define DEGTORAD   (PI/180.00)

/* Prototypes: */

// stardlg.cpp
int Read_Catalog1(char *catalog_name, char *StarName,
                   char *StarAlpha, char *StarDelta, char *StarEquin,
                   char *error_message);

// "tav2.cpp":
void clean_buffer(char *buffer, int size);
int SaveStatus(char *error_message);

// "jlp_celmeca1.cpp":
int input_location(double *xlat, double *xlong, int iloc);
int current_local_coord(double xlat, double xlong, double alpha,
                        double delta, double *hour_angle, double *elev,
                        double *azim, int UT_shift);
int local_coord(double sidereal_time, double xlat, double alpha,
                double delta, double *hour_angle, double *elev, double *azim);
int precess(double *alpha, double *delta, double Delta_years);
int julian(double aa, int mm, int idd, double time, double *djul);
int convert_coord(double coord, int *ial1, int *ial2, double *al3, char *opt);
double jlp_gst0h(double aa, int mm, int idd);

// "jlp_risley.cpp" :
int GET_CROSSANGLE(double lambda_cent, double dlambda, double *cross_angle,
                   double *resid_disp, double *beta0, double tt, double ff,
                   double pp, double zd, int ra_offset, int rb_offset,
                   int ra_sign, int rb_sign, int *icode1, int *icode2, FILE *fp1,
                   char *ADC_text0, int *italk);
int GET_PW_SAT(double *pw_sat, double tt);

// "jlp_utime.cpp":
int jlp_utime(char *UT_Time, double *utime1,
              double *year, int* month, int *day, int UT_shift);
int jlp_local_time(char *buffer);
int decode_time(char *buffer, double *aa, int *mm, int *idd,
                       double *time1);
int jlp_lsidtime(char *LSTime, double *sidereal_time, double xlong,
                 int UT_shift);
int risley_posi(FILE *fp1, double xlat, double hour_angle,
                double zen_dist, double lambdac, double dlambda,
                double *beta, double *cross_angle, double *resid_disp,
                double temp_value, double hygro_value, double press_value,
                int ra_offset, int rb_offset, int ra_sign, int rb_sign,
                int *code_ra, int *code_rb, char *AC_text0);

#endif
