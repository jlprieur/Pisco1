/*****************************************************************
* Set of routines to get Universal Time, Hour Angle, Elevation, ...
*
* Version 20-11-2005
* JLP
******************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "psc1_typedef.h" // ABS
#include "tav_utils.h"

/*
#define DEBUG
*/
#define PI 3.14159265
#define DEGTORAD   (PI/180.00)

/*****************************************************************
* Routine to get the universal time from the local time
*
* INPUT:
* UT_shift:
* Example of output:
*     12 H 05 m 34 s
******************************************************************/
int jlp_utime(char *UT_Time, double *utime1,
              double *year, int* month, int *day, int UT_shift)
{
char buffer[81];
double local_time, h2;
int ih0, ih1;

jlp_local_time(buffer);
#ifdef DEBUG
printf(" OK: local time is >%s< \n",buffer);
#endif
 decode_time(buffer, year, month, day, &local_time);

 *utime1 = local_time + (double)UT_shift;

// Handle special case:
 if(*utime1 < 0.) {
    *utime1 += 24.;
    (*day)--;
  }

  convert_coord(*utime1, &ih0, &ih1, &h2, "H");
  sprintf(UT_Time,"%02d h %02d m %02d s", ih0, ih1, (int)h2);

return(0);
}
/*****************************************************************
* Routine to get local sidereal time
*
* INPUT:
* xlong is in hours (not in radians...)
*
* OUTPUT:
* sidereal time in hours
*
* Example of output:
*     12 H 05 m 34 s
******************************************************************/
int jlp_lsidtime(char *LSTime, double *sidereal_time, double xlong,
                 int UT_shift)
{
char buffer[81];
double UT_time, h2, gst0h, year;
int month, day, ih0, ih1;

/* Universal Time (winter/summer time): */
  jlp_utime(buffer, &UT_time, &year, &month, &day, UT_shift);
/* gst0h is hours ... */
  gst0h = jlp_gst0h(year, month, day);
/* xlong is in hours ... */
  *sidereal_time = gst0h + UT_time * 1.002737908 - xlong;
/* sidereal time is in hours: */
  convert_coord(*sidereal_time, &ih0, &ih1, &h2, "H");
  sprintf(LSTime, "%02d h %02d m %02d s", ih0, ih1, (int)h2);

return(0);
}

/*****************************************************************
* Routine to decode local time
*
* Example of input:
*     "Wed Jun 19 12:05:34 1994                            "
* Output:
* aa = 1994.0
* mm = 6
* idd = 19
* time1 = 12.xxxx
******************************************************************/
int decode_time(char *buffer, double *aa, int *mm, int *idd,
                       double *time1)
{
int status, i1, i2, i3;
char day[81], month[81], year[81], time2[81];

status = 0;

/********************* First extract character strings */

sscanf(&buffer[4],"%s",month);
month[3]='\0';

sscanf(&buffer[8],"%s",day);
day[2]='\0';

sscanf(&buffer[11],"%s",time2);
time2[8]='\0';

sscanf(&buffer[20],"%s",year);
year[4]='\0';

#ifdef DEBUG
  printf(" Month >%s<  Day >%s< Year >%s< Time >%s<\n",month,day,year,time2);
#endif

/********************* Then decode these character strings */

/* Jan Feb Mar Apr May Jun Jul Aug Sep Nov Dec */
switch (month[0])
{
/* Jan Jun Jul */
  case 'J':
    *mm = 1;
    if(month[1] == 'u') *mm = 6;
    if(month[1] == 'u' && month[2] == 'l') *mm = 7;
    break;
/* Feb */
  case 'F':
      *mm = 2;
      break;
/* Mar May */
  case 'M':
      *mm = 3;
      if(month[2] == 'y') *mm = 5;
      break;
/* Apr Aug */
  case 'A':
      *mm = 4;
      if(month[1] == 'u') *mm = 8;
      break;
/* Sep */
  case 'S':
      *mm = 9;
      break;
/* Oct */
  case 'O':
      *mm = 10;
      break;
/* Nov */
  case 'N':
      *mm = 11;
      break;
/* Dec */
  case 'D':
      *mm = 12;
      break;
  default:
     printf(" decode_time/Fatal error decoding month\n");
     status = -1;
     break;
}

sscanf(day,"%02d",idd);

sscanf(year,"%d",&i1);
*aa = (double)i1;

#ifdef DEBUG
 printf(" mm %d  idd %d aa %lf \n",*mm,*idd,*aa);
#endif

sscanf(time2,"%02d:%02d:%02d",&i1,&i2,&i3);
*time1 = (double)i1 + ((double)i2)/60. + ((double)i3)/3600.;
#ifdef DEBUG
 printf(" %02d:%02d:%02d time1 %lf \n",i1,i2,i3,*time1);
#endif

return(status);
}
/*****************************************************************
* Routine to compute risley prism positions
*
* INPUT:
*  xlat : degrees
*  hour_angle : hours
*
* OUTPUT:
*  beta0: angle between the lines linking the star to the pole
*         and the star to the zenith.
******************************************************************/
int risley_posi(FILE *fp1, double xlat, double hour_angle,
                double zen_dist, double lambdac, double dlambda,
                double *beta, double *cross_angle, double *resid_disp,
                double temp_value, double hygro_value, double press_value,
                int ra_offset, int rb_offset, int ra_sign, int rb_sign,
                int *code_ra, int *code_rb, char *ADC_text0)
{
double ff, xlat_rad, hour_rad, zdist_rad, ww_beta, sin_beta, pw_sat;
double ww_cross, ww_resid;
int italk, status;

/* Conversion to radians: */
  xlat_rad = xlat * DEGTORAD;
  hour_rad = hour_angle * PI/12.0;

/*
 Get Beta angle, i.e., angle between the sides linking the star to the pole
 and the star to the zenith (usefull for correction of atmospheric dispersion)
*/
   zdist_rad = zen_dist * DEGTORAD;
   sin_beta = 1.;
   if(zdist_rad != 0.) {
    sin_beta = cos(xlat_rad) * sin(hour_rad) / sin(zdist_rad);
    }
   if(ABS(sin_beta) <= 1.00) ww_beta = asin(sin_beta);
    else ww_beta = PI/2.;

/* Conversion to degrees: */
   ww_beta /= DEGTORAD;
   *beta = ww_beta;
   if(fp1 != NULL)
     fprintf(fp1,
            " Beta angle (for atm. disp.) : %.3f (deg) (North=0, East=90)\n",
            *beta);

/* Saturation water pressure: */
 GET_PW_SAT(&pw_sat,temp_value);
 ff = hygro_value * pw_sat /100.;

/* WARNING: beta, zen_dist should be in degrees for GET_CROSSANGLE:
*/
/* If italk=1 write information to file fp1: */
   italk = 1;
   status = GET_CROSSANGLE(lambdac,dlambda,&ww_cross,&ww_resid,
                 &ww_beta,temp_value,ff,press_value,zen_dist, ra_offset,
                 rb_offset, ra_sign, rb_sign, code_ra,code_rb,fp1,
                 ADC_text0, &italk);

*cross_angle = ww_cross;
*resid_disp = ww_resid;

return(status);
}
