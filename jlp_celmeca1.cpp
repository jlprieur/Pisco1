/***********************************************************
* cel_meca1.cpp
* Set of routines about celestial mechanics.
* To compute atmospheric absorption
* and position of Risley prisms (first written in 1993)
*
* CONTAINS:
*   input_location(xlat,xlong,iloc)
*   current_local_coord(xlat,xlong,alpha,delta,hour_angle,elev,azim)
*   precess(alpha,delta,Delta_years)
*   julian(aa,mm,idd,time,djul)
*   convert_coord(coord,ial1,ial2,al3,opt)
*
* Except for xlat (degrees) and xlong (hours), all angles are in radians
* (internally)
*
* JLP
* Version of 20/11/2005
***********************************************************/
#include <stdio.h>
#include <math.h>
#include "tav_utils.h"


//#define TTREE
#ifdef TTREE
int input_location(double *xlat, double *xlong, int *iloc)
{ return(0);}
int current_local_coord(double xlat, double xlong, double alpha, double delta,
                        double *hour_angle, double *elev, double *azim)
{ return(0);}
int precess(double *alpha, double *delta, double Delta_years)
{ return(0);}
int julian(double aa, int mm, int idd, double time, double *djul)
{ return(0);}
int convert_coord(double coord, int *ial1, int *ial2, double *al3, char *opt)
{ return(0);}
double jlp_gst0h(double aa, int mm, int idd)
{ return(0);}
#else
/****************************************************************
* Subroutine INPUT_LOCATION
* Prompt for the latitude and longitude of the observatory
*
* Output:
* XLAT (degrees) : latitude
* XLONG (hours) : longitude
*****************************************************************/
int input_location(double *xlat, double *xlong, int iloc)
{

/* Location :
 1= Pic du Midi
 2= Mount Stromlo
 3= CFHT (Hawaii)
 4= La Silla (ESO)
 5= O.H.P.
 6= CAV
 7= Merate
 8= Calern
 9= Other
*/

 switch (iloc)
    {
/* Pic du Midi: LW=+00 DEG 08' 42" lLAT=42 DEG  56' 12"*/
      case 1:
        {
          *xlong = 0.0096667;
          *xlat = 42.9366667;
          break;
        }
/* Mount Stromlo: LW=-149 DEG 00' 30" LAT=-35 DEG 19.2' */
      case 2:
        {
          *xlong = -9.9338889;
          *xlat = -35.320;
          break;
        }
/* CFHT, Hawaii:  LW=+155 DEG 28' 18" LAT=+19 DEG 49.6' */
      case 3:
        {
          *xlong = 10.364778;
          *xlat = 19.826667;
          break;
        }
/* La Silla (ESO): LW=+70 DEG 43' 48"  LAT=-29 DEG 15.4' */
      case 4:
        {
          *xlong = 4.7153333;
          *xlat = -29.256667;
          break;
        }

/* OHP (Haute Provence): LW=-5 DEG 42' 48"   LAT=43 DEG 56*/
      case 5:
        {
          *xlong = -.3808889;
          *xlat = 43.926667;
          break;
        }

/* (Obs. Jolimont: LW=1 DEG 28'  LAT=+43 DEG 37'
              cf. Annuaire du Bureau des Longitudes)
Villemur : LW= 1 deg 40  LAT=+43 deg 50 ???? */
      case 6:
        {
          *xlong = -0.11;
          *xlat = 43.8;
          break;
        }

/* Merate : LW=-9 DEG 25' 09"   LAT=45 DEG 40 35 */
      case 7:
        {
          *xlong = -.62794447;
          *xlat = 45.676389;
          break;
        }

/* Calern : LW=-6 DEG 55' 06"   LAT=43 DEG 45 13 */
      case 8:
        {
          *xlong = -0.46153703;
          *xlat = 43.75361111;
          break;
        }

/************* Should prompt for the coordinates of the observatory: */
     default:
        {
/* Latitude (in hours):  */
         *xlat = 0.;
/* Longitude (in degrees): */
         *xlong = 0.;
         break;
        }

/* End of switch: */
   }

return(0);
}
/******************************************************************
* Subroutine CURRENT_LOCAL_COORD to compute the azimuth and elevation
* of a star, at the time of the call
*
* Input:
* XLAT (degrees), XLONG (hours): coordinates of the observatory
* ALPHA (hours), DELTA (degrees): coordinates of the star
*
* Other parameters:
* DJUL : Julian day
*
* Output:
* HOUR_ANGLE (hours): hour angle
* ELEV(degrees): elevation
* AZIM(degrees): azimuth
***************************************************************/
int current_local_coord(double xlat, double xlong, double alpha, double delta,
                        double *hour_angle, double *elev, double *azim,
                        int UT_shift)
{
double sidereal_time;
char buffer[80];
int status;

/* Compute sidereal time (in hours) at the time indicated by the clock
of the PC:*/
   jlp_lsidtime(buffer, &sidereal_time, xlong, UT_shift);

 status = local_coord(sidereal_time, xlat, alpha, delta,
                      hour_angle, elev, azim);
return(status);
}
/******************************************************************
* Subroutine LOCAL_COORD to compute the azimuth and elevation
* of a star, using the sidereal time as input
*
* Input:
* XLAT (degrees), XLONG (hours): coordinates of the observatory
* ALPHA (hours), DELTA (degrees): coordinates of the star
*
* Other parameters:
* DJUL : Julian day
*
* Output:
* HOUR_ANGLE (hours): hour angle
* ELEV(degrees): elevation
* AZIM(degrees): azimuth
***************************************************************/
int local_coord(double sidereal_time, double xlat,
                double alpha, double delta,
                double *hour_angle, double *elev, double *azim)
{
double sin_elev, sin_azim, cos_azim, xlat_rad;
double delta_rad, hour_angle_rad;

/* Calculation of the hour angle (in hours) */
    *hour_angle = sidereal_time - alpha;
    *hour_angle = *hour_angle - 24. * (double)((int)(*hour_angle /24.));

/* Calculation of the elevation ELEV
*/
    hour_angle_rad = *hour_angle * PI / 12.;
    xlat_rad = xlat * DEGTORAD;
    delta_rad = delta * DEGTORAD;
    sin_elev =  sin(delta_rad) * sin(xlat_rad)
                + cos(delta_rad) * cos(xlat_rad) * cos(hour_angle_rad);
    if(sin_elev <= 1.0) *elev = asin(sin_elev);
       else *elev = 0.;

/* Calculation of the azimuth: */
   if(cos(*elev) == 0.) {
      *azim = 0.;
   } else {
      sin_azim = sin(hour_angle_rad) * cos(delta_rad) / cos(*elev);
      cos_azim = ( cos(hour_angle_rad) * sin(xlat_rad) * cos(delta_rad)
                  - sin(delta_rad) * cos(xlat_rad)) / cos(*elev);
      *azim = acos(cos_azim);
      if(sin_azim < 0) *azim *= -1.;
     }

// Conversion to degrees:
  *elev /= DEGTORAD;
  *azim /= DEGTORAD;

return(0);
}
/************************************************************
*       SUBROUTINE PRECESS
* From "Numerical Ephemerides" ?
* Input/Output : ALPHA in hours and DELTA in degrees
* Input: Delta_years : (year of obsevation - year of the catalogue)
* New version of 20/11/2005
*************************************************************/
int precess(double *alpha, double *delta, double Delta_years)
{
double alpha_rad, delta_rad, palpha, pdelta;

alpha_rad = *alpha * PI / 12.;
delta_rad = *delta * DEGTORAD;

/* PALPHA and PDELTA : correction per year palpha hours and pdelta degrees */
  palpha = (3.075 + 1.336 * sin(alpha_rad) * tan(delta_rad) ) / 3600.;
  pdelta = (20.04 * cos(alpha_rad)) / 3600.;

  palpha *= Delta_years;
  pdelta *= Delta_years;

  *alpha += palpha;
  *delta += pdelta;

return(0);
}
/*********************************************************************
* Subroutine JULIAN to compute the Julian day of an observation:
*
* The Julian day begins at Greenwich mean noon (at 12 U.T.)
*
* Here also the Gregorian calendar reform is taken into account.
* Thus the day following 1582 October 4 is 1582 October 15.
*
* The B.C. years are counted astronomically. Thus the year
* before the year +1 is the year 0.
*
* Input:
* AA, MM, IDD, TIME : year,month, day, time of the observation
* DJUL : Julian day
**********************************************************************/
int julian(double aa, int mm, int idd, double time, double *djul)
{
double day1, year1, date_obs, date_reform;
long int month1, ia1, ib1;

  day1 = time/24. + (double)idd;
/* First the year after the 1st March ... */
  if(mm > 2)
    {
     year1 = aa;
     month1 = mm;
    }
   else
    {
     year1 = aa - 1;
     month1 = mm + 12;
    }

/* Then check if after the Gregorian reform: */
/* BE CAREFUL WITH PC's AND LONG INTEGERS.... */
    date_obs = aa + ((long int)(275 * mm / 9)
               - 2. * (long int) ((mm + 9) / 12) + idd - 30 ) / 365.;
    date_reform = 1582. + 289./365.;
    if(date_obs >= date_reform)
       {
         ia1 = (long int) (year1 / 100.);
         ib1 = 2 - ia1 + (long int) (((float)ia1)/4.);
       }
    else
         ib1 = 0;

/* Now final formula: */
      *djul = (double)((long int)(365.25 * year1)
              + (long int)(30.6001 * (month1 + 1))
              + day1 + 1720994.5 + ib1);

return(0);
}
/**************************************************************
* Subroutine CONVERT_COORD
* Input: COORD in degrees or in hours
* Output: IAL1 degrees IAL2 ' AL3 " (when O='D')
* Output: IAL1 hour IAL2 mn AL3 sec (when O='H')
***************************************************************/
int convert_coord(double coord, int *ial1, int *ial2, double *al3, char *opt)
{
double coord1, al2;
int isign;

coord1 = coord;

/* Translating the values between 0H and 24H or 0.deg and 360.deg */
  isign = 1;
  if(*opt == 'H')
    {
     coord1 = coord1 - 24. * (float)((int)(coord1/24.));
     if(coord1 < 0) coord1 += 24.;
    }
   else
    {
      if(coord1 < 0)
         {
          isign = -1;
          coord1 = -coord1;
         }
    }

/*  Converting the values in mn, sec or ' " */
    *ial1 = (int)coord1;
    al2 = (coord1 - (double)(*ial1)) * 60.;
    *ial2 = (int)al2;
    *al3 = (al2 - (double)(*ial2)) * 60.;

/* Case when al3 = 60.0: */
   if(*al3 >= 59.99) { *al3 = 0.; (*ial2)++; }

/* When negative ' or negative ", we add 60 : */
    if(*opt == 'D')
     {
       (*ial1) *= isign;
       if(*al3 < 0.)
         { (*al3) += 60.;
           (*ial2)--;
         }

       if((*ial2) < 0.)
         { (*ial2) += 60;
           (*ial1)--;
         }
      }

return(0);
}
/***************************************************************
* jlp_gst0h
* Output: GST0H in hours
****************************************************************/
double jlp_gst0h(double aa, int mm, int idd)
{
double tt1, gst0h, djul;

/* Julian day of the observation (at 0H UT): */
    julian(aa,mm,idd,0.,&djul);

/* Greenwich Sidereal time at 0:00:00 (U.T.) */
    tt1 = (djul - 2415020.0) / 36525.;
    gst0h = 24. * (0.276919398 + 100.0021359 * tt1 + 0.000001075*tt1*tt1);

return(gst0h);
}
#endif // ifndef TTREE
