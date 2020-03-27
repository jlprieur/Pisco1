/***************************************************************
* To compute Risley prisms correction for Calern
*
* Set of routines.
* (or main program in debug mode)
*
* Contains:
* double ad_simon(wave,tt,ff,pp,zd)
* double ind_air_owens (wave,tt,ff,pp)
* double ind_f4 (wave)
* double ind_sk10 (wave)
* int output_curves(npts,lambda_cent,tt,ff,pp,zd,cross_angle,dlambda,beta0,fp1)
* int GET_CROSSANGLE(lambda_cent,dlambda,cross_angle,
*                    resid_disp,beta0,tt,ff,pp,zd,icode1,icode2,fp1,italk)
* double ad_owens(wave,tt,ff,pp,zd)
* int GET_PW_SAT(pw_sat,tt)
*
* In the whole program, "index" is used instead of "(index - 1)"
*
* JLP
* Version of 05-08-2015
***************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "tav_utils.h"

static double ind_air_owens(double wave, double tt, double ff, double pp);
static double ind_sk10 (double wave);
static double ind_f4 (double wave);
static double ATM_DISP(double lambda_high,double tt, double ff,
                       double pp, double zd);
static double ad_owens(double wave, double tt, double ff, double pp, double zd);

/*
#define DEBUG
*/

/* Interactive mode: use printf, whereas non interactive use fprintf
#define INTERAC
*/

#define IDIM 200
/* The magnification factor of the speckle camera at the TBL is 250
   i.e. ratio of focal length of the TBL (50 00 cm) over focal of
   field lens (20 cm) */
// #define MAGNIF 250
// Merate: focal length=15 00 cm
// #define MAGNIF 75
// Calern: focal length=13 00 cm
#define MAGNIF 65
#ifndef PI
#define PI 3.14159265358979323846
#endif
/* To select the dispersion formula to be used:
*/
#define OWENS

#ifdef OWENS
#define ATM_DISP ad_owens
#else
#define ATM_DISP ad_simon
#endif

static double beta1=10.0, beta2=9.92;

/*
#define MAIN_PROG
*/
#ifdef MAIN_PROG
main()
{
double hygro, pw_sat, dlambda, cross_angle, beta0;
double lambda_cent, resid_disp, tt, pp, ff, zd;
double bonnette_code = 0.;
int npts, italk, icode1, icode2, status;
int ra_offset, rb_offset, ra_sign, rb_sign;
char buffer[91];
FILE *fp1;
char logfile[]="risley.log";
#ifndef INTERAC
  FILE *fp2;
  char infile[]="risley.dat";
#endif

/* Open logfile: */
  if((fp1 = fopen(logfile,"w")) == NULL)
    {
    printf("!Fatal error opening >%s< output logfile",logfile);
    return(-1);
    }

#ifdef INTERAC
  printf("Program RISLEY: Risley prisms correlation of the OMP speckle camera\n");
  printf(" Version 26-11-2003 \n");
#else
  if((fp2 = fopen(infile,"r")) == NULL)
    {
     fprintf(fp1,"!Fatal error opening >%s< input file",infile);
     fclose(fp1);
     return(-1);
    }
#endif

#ifdef INTERAC
/*
printf("Enter P (mm Hg, atm. pressure), hygrometric degree (0-100) ");
printf("and T (degree C, temperature):\n");
gets(buffer); sscanf(buffer,"%lf,%lf,%lf",&pp,&hygro,&tt);
*/
/* Mean atmospheric pressure at TBL is 550 mm Hg */
pp=561.;hygro=60.;tt=6.;
#else
/* Read two lines of comments and then the input parameters: */
  fgets(buffer,80,fp2);  fgets(buffer,80,fp2);
  fgets(buffer,80,fp2);
  sscanf(buffer,"%lf %lf %lf",&pp,&hygro,&tt);
#endif

GET_PW_SAT(&pw_sat,tt);
ff = hygro * pw_sat / 100.;

#ifdef INTERAC
  printf(" Saturation water pressure is %f mmHg \n",pw_sat);
  printf(" and water partial pressure is %f mmHg \n",ff);
  printf(" Enter zenith angle and beta0 (relative to North) (in degrees) :\n");
  gets(buffer); sscanf(buffer,"%lf,%lf",&zd,&beta0);
#else
/* Read one line of comments and then the input parameters: */
  fgets(buffer,80,fp2);  fgets(buffer,80,fp2);
  sscanf(buffer,"%lf %lf",&zd,&beta0);
#endif

/*
printf(" Enter roof angle of F4 and SK10 prisms: (degrees)\n");
gets(buffer); sscanf(buffer,"%lf,%lf",&beta1,&beta2);
*/

#ifdef INTERAC
  printf(" Enter central lambda and delta lambda (nm): \n");
  gets(buffer); sscanf(buffer,"%lf,%lf",&lambda_cent,&dlambda);
#else
/* Read one line of comments and then the input parameters: */
  fgets(buffer,80,fp2);  fgets(buffer,80,fp2);
  sscanf(buffer,"%lf %lf",&lambda_cent,&dlambda);
#endif

/*********************************************************************
* Display current status:
*********************************************************************/
#ifdef INTERAC
  printf("\n F4_roof_angle = %.2f deg, SK10_roof_angle = %.2f deg \n",
          beta1,beta2);
  printf("\n P = %.2f mmHg, Pwater = %.2f mmHg, T = %.2f degC \n",pp,ff,tt);
  printf(" zd = %.2f deg, lambda_cent = %.2f nm, delta_lambda = %.2f nm \n\n",
      zd, lambda_cent, dlambda);
  printf(" Beta0 = %.2f \n",beta0);

#ifdef OWENS
  printf(" Computation using Owens' formula \n");
#else
  printf(" Computation using Simon's formula \n");
#endif

#endif

/* Fills logfile: */
  fprintf(fp1," F4_roof_angle = %.2f deg, SK10_roof_angle = %.2f deg \n",
          beta1,beta2);
  fprintf(fp1," P = %.2f mmHg, Pwater = %.2f mmHg, T = %.2f degC \n",pp,ff,tt);
  fprintf(fp1," zd = %.2f deg, lambda_cent = %.2f nm, delta_lambda = %.2f nm\n",
      zd, lambda_cent, dlambda);
  fprintf(fp1," Saturation water pressure = %.2f mmHg,",pw_sat);
  fprintf(fp1," water partial pressure = %.2f mmHg \n",ff);
  fprintf(fp1," Beta0 = %.2f \n",beta0);
#ifdef OWENS
  fprintf(fp1," ********  Computation using Owens' formula *******\n");
#else
  fprintf(fp1," ********  Computation using Simon's formula ******\n");
#endif

/* Deriving cross_angle between prisms
   and computing residuals at lambda_cent */
italk = 1;
status = GET_CROSSANGLE(lambda_cent,dlambda,&cross_angle,&resid_disp,
               &beta0,tt,ff,pp,zd, ra_offset, rb_offset, ra_sign, rb_sign,
               &icode1,&icode2,fp1,&italk);

/* Output of data files to be drawn elsewhere.
   With 66 points, the step is 10 nm: */
#ifdef DEBUG
  npts=11;
#else
  npts=66;
#endif
/*
output_curves(npts,lambda_cent,tt,ff,pp,zd,cross_angle,dlambda,beta0,fp1);
*/

return(0);
}
#endif
/* End of ifdef MAIN_PROG */
/***************************************************************
* ad_simon(wave,tt,ff,pp)
* Routine to compute atmospheric refraction versus wavelength
* From Simon, W. 1966, A.J. 71,190
* ad is in arcseconds
***************************************************************/
static double ad_simon(double wave, double tt, double ff, double pp, double zd)
{
double ad, zd_rad, cc[15];
double zz,z2,z3,z4,ss,sq,qq,yy,y2;

cc[0] = 3.45020e-03;
cc[1] = -3.34591e-04;
cc[2] = -1.60149e-05;
cc[3] = -1.54316;
cc[4] = 2.27095e-01;
cc[5] = 3.14759e-03;
cc[6] = 2.87409e-03;
cc[7] = -2.92730e-04;
cc[8] = -1.56673e-05;
cc[9] = 1.41299e-03;
cc[10] = -2.22307e-04;
cc[11] = -1.66610e-06;
cc[12] = -3.34814e-05;
cc[13] = 5.33352e-06;
cc[14] = 3.52107e-08;

qq = 0.740568;

#ifdef DEBUG
printf(" Computing atmospheric refraction \n");
printf(" From Simon, W. 1966, A.J. 71,190, h0=2811m, p0=547mm, T0=6degC \n");
printf(" pp,ff,tt,zd %f %f %f %f \n",pp,ff,tt,zd);
#endif

zd_rad = zd*PI/180.0;
zz=tan(zd_rad);
z2=zz*zz;
z3=z2*zz;
z4=z2*z2;

ss=pp-0.148238*ff+(1.049-0.0157*tt)*pp*pp*1.e-06;
ss=ss/(720.883*(1.+0.003661*tt));
sq=ss/qq;

/* Formula with lambda in microns... */
  yy=1.E06/(wave*wave);
  y2=yy*yy;

/* Atmospheric dispersion in arcsec
 (From Simon, W. 1966, A.J. 71,190) */

  ad=cc[0] + cc[1]*yy + cc[2]*y2 + cc[3]*zz + cc[4]*zz*yy
     + cc[5]*zz*y2 + cc[6]*z2 + cc[7]*z2*yy + cc[8]*z2*y2
     + cc[9]*z3 + cc[10]*z3*yy + cc[11]*z3*y2 + cc[12]*z4
     + cc[13]*z4*yy + cc[14]*z4*y2;
  ad=ad*sq;

#ifdef DEBUG
 printf(" Atm. disp. is %f arcsec at %.2f nm (outside the telescope, with 0 at 400 nm)\n",
        ad,wave);
#endif

return(ad);
}
/*********************************************************************
*   ind_air_owens
*
*   wave is wavelength in nm.
*   returns n(wave)-1 for atmosphere at TEMP, PRESS (air), WPRESS (water vapor)
*   using formulae 29-31 in Owens, J.C., Appl. Opt. v.6, p.51 (67)
*
*   PRESS, WPRESS in hectopascals (mbars), TEMP in degrees C
*   these values are defined in site.h .
*
*********************************************************************/
static double ind_air_owens(double wave, double tt, double ff, double pp)
{
        double tk, ds, dw, w, wm2, r;
        double TEMP, PRESS, WPRESS;
        TEMP = tt;
        PRESS = pp * 1013. / 760.;
        WPRESS = ff * 1013. /760.;

        tk = TEMP + 273.16;

        ds = PRESS / tk *
            (1. + PRESS * (57.9E-8 - 9.325E-4 / tk + .25844 / (tk*tk)));
        dw = (-2.37321E-3 + (2.23366 / tk) - 710.792
                    / (tk*tk) + 7.75141E-4 / (tk*tk*tk));
        dw = WPRESS / tk * (1. + WPRESS * (1. + WPRESS * 3.7E-4) * dw);

        w = wave * 1.0E-3;                      /* # convert to microns */
        wm2 = 1./(w*w);

        r = ds * (2371.34 + 683939.7 / (130.-wm2) + 4547.3 / (38.9 - wm2));
        r += dw * (6847.31 + 58.058 * wm2
                 - .7115 * wm2*wm2 + .08851 * wm2*wm2*wm2);
        r *= 1.e-8;
#ifdef DEBUG
        printf (" (air index - 1) is %7.5g at %6.1f nm\n", r, wave);
#endif
        return (r);
}

/*********************************************************************
* ind_f4
*  returns n (wave) - 1 for f4 glass
*  using power series from Schott catalog
* wave is wavelength in nm
*********************************************************************/
static double ind_f4 (double wave)
{
        double f4[6], w, w2, wm2, wm4, wm6, wm8, ensq;
        double ind;

        f4[0] = 2.54469;
        f4[1] = -8.5925665E-3;
        f4[2] = 2.2583116E-2;
        f4[3] = 7.378991E-4;
        f4[4] = -9.5060668E-6;
/* error here in original program: bad value was f4[5] = 3.8257676E-6; */
        f4[5] = 3.82577E-8;

/*  convert to microns */
        w = (double)wave * 1.E-3;
        w2 = w*w;
        wm2 = 1./w2;
        wm4 = wm2*wm2;
        wm6 = wm4*wm2;
        wm8 = wm4*wm4;
        ensq = f4[0] + f4[1] * w2 + f4[2] * wm2 + f4[3] * wm4
               + f4[4] * wm6 + f4[5] * wm8;
        ensq = sqrt (ensq);
        ind = ensq - 1.0;

#ifdef DEBUG
        printf (" f4 glass index -1 is %f at %5.1f nm\n",
                   ind, wave);
#endif
        return (ind);
}

/*********************************************************************
* ind_sk10
*  returns n(wave)-1 for sk10 glass
*  using power series from Schott catalog
* wave is wavelength in Angstroms
*********************************************************************/
static double ind_sk10 (double wave)
{
        double sk10[6], w, w2, wm2, wm4, wm6, wm8, ensq;
        double ind;

        sk10[0] = 2.588171;
        sk10[1] = -9.3042171E-3;
        sk10[2] = 1.6075769E-2;
        sk10[3] = 2.2083748E-4;
        sk10[4] = 3.5467529E-6;
        sk10[5] = 2.6143582E-7;

/* convert to microns */
        w = (double)wave * 1.E-3;
        w2 = w*w;
        wm2 = 1./w2;
        wm4 = wm2*wm2;
        wm6 = wm4*wm2;
        wm8 = wm4*wm4;
        ensq = sk10[0] + sk10[1] * w2 + sk10[2] * wm2
                + sk10[3] * wm4 + sk10[4] * wm6 + sk10[5] * wm8;
        ensq = sqrt (ensq);
        ind = ensq - 1.0;
#ifdef DEBUG
        printf (" sk10 glass index -1 is %f at %5.1f nm\n",
                   ind, wave);
#endif
        return (ind);
}
/**********************************************************************
* GET_CROSSANGLE
*
* INPUT:
*  zd: zenith distance in degrees
*  lambda_cent, dlambda
*  tt, ff, pp: temperature, humidity, pression
*  bonnette_code: position of the bonnette (for Pic du Midi, where the
*                bonnette can rotate and make PISCO rotate too...
* OUTPUT:
*
* INPUT/OUTPUT:
*  beta0: angle between the lines linking the star to the pole
*         and the star to the zenith.
**********************************************************************/
int GET_CROSSANGLE(double lambda_cent, double dlambda, double *cross_angle,
                   double *resid_disp, double *beta0, double tt, double ff,
                   double pp, double zd, int ra_offset, int rb_offset,
                   int ra_sign, int rb_sign, int *icode1, int *icode2, FILE *fp1,
                   char *ADC_text0, int *italk)
{
double lambda_low, lambda_high, airdisp;
double sin1, sin2, cross_coef, work, cangle;
double prism_disp, mean_dev, radtosec, bonnette_code;
int status;

if(fp1 != NULL)
{
fprintf(fp1," ************* GET_CROSSANGLE ************ \n");
fprintf(fp1," Central wavelength: %f Bandwidth: %f \n", lambda_cent, dlambda);
fprintf(fp1," Beta: %.1f (deg) zd = %.1f (deg) \n", *beta0, zd);
}

/* First check that object is above the horizon (zd is in degrees...): */
if(zd > 90.0)
  {
  *cross_angle = 0.;
  *icode1 = -1; *icode2 = -1;
  sprintf(ADC_text0, " Sorry the object is under the horizon...");
  if(fp1 != NULL) {
  fprintf(fp1," Sorry the object is under the horizon... \n");
  fprintf(fp1,"> %d %d \n",*icode1,*icode2);
  }
  return(-1);
  }

lambda_low = lambda_cent - dlambda/2.;
lambda_high = lambda_cent + dlambda/2.;

/* Conversion to arcsec: */
radtosec = 180.*3600./PI;

/* Computing air dispersion between lambda_low and lambda_high
*/
airdisp = ATM_DISP(lambda_low, tt, ff, pp, zd) -
          ATM_DISP(lambda_high, tt, ff, pp, zd);
if(airdisp < 0) airdisp = -airdisp;

/* Computing prism dispersion (using beta&, beta2, roof angles): */
  sin1 = sin((double)(beta1*PI/180.));
  sin2 = sin((double)(beta2*PI/180.));

prism_disp = (ind_f4(lambda_low) - ind_f4(lambda_high)) * sin1
             - (ind_sk10(lambda_low) - ind_sk10(lambda_high)) * sin2 ;

if(*italk)
  {
printf("ok2 \n");
printf(" dispersion of individual prism = %f (arcsec in parallel beam) \n",
        prism_disp * radtosec);
printf(" dispersion of individual prism =  \n");
printf(" air dispersion is = %f (arcsec in parallel beam) \n",
        airdisp * MAGNIF);
  }

/* Taking the magnification factor into account: */
  work = (airdisp * MAGNIF / radtosec) / (2. * prism_disp) ;
  if(work > -1. && work < 1.)
     {
     work = 2. * acos(work);
/* Conversion to degrees: */
     *cross_angle = work * 180./PI;
     status = 0;
     }
  else
    {
/* Even if correction is not sufficient, it is better to try
* and correct part of the dispersion: */
#ifdef INTERAC
    printf("\n get_crossangle/Too big dispersion: cannot be corrected \n");
    printf("    Ratio is %f for lambda = %f nm\n", work, lambda_cent);
#endif
    if(fp1 != NULL)
    {
    fprintf(fp1,"\n get_crossangle/Too big dispersion: cannot be corrected \n");
    fprintf(fp1,"    Ratio is %f for lambda = %f nm\n", work, lambda_cent);
    }
    *cross_angle = 0.;
    status = 1;
    }


if(*italk)
  {
#ifdef INTERAC
   printf(" Air dispersion to be corrected: %.4f (arcsec in focal plane) \n",
      airdisp);
   printf(" Optimum cross_angle is %.2f (deg) \n",*cross_angle);
#endif
   if(fp1 != NULL)
   {
   fprintf(fp1," Air dispersion to be corrected: %.4f (arcsec in focal plane)\n",
      airdisp);
   fprintf(fp1," Optimum cross_angle is %.2f (deg) \n",*cross_angle);
   }
  }

/* July 1995:
  The instrument rotates from North to West when bonnette
  rotates positively:
  August 2015: not available at Calern, so I setit to zero:
*/
bonnette_code = 0.;
*beta0 -= bonnette_code;
/* Theoretical code positions assuming 0 offset and clockwise (Sept 94)
(see 10 lines further down for full conversion...)
*/
*icode1 = (int)(1024. * (- *beta0 - (*cross_angle)/2.0) /360.);
*icode2 = *icode1 + (int)(1024. * (*cross_angle) /360.);
 if(fp1 != NULL)
 {
 fprintf(fp1,"DEBUG/ icode1=%d icode2=%d cross_angle=%.1f \n",
         *icode1, *icode2, *cross_angle);
 }
*cross_angle = (float)(*icode2 - *icode1) * 360. / 1024.;

/* offset in X and Y for the prisms; */
/* Position of April 94 to compensate for the  dispersion RB-RA=181,
RA and RB aligned to North: 40, 371
Bonnette at 90 degrees (code 166 , 76)
CP40, channel 4, X axis to South, Y axis (downwards) to West.
OFFSET_RA 371
OFFSET_RB 40
*/
/* Position of September 94 to compensate for the  dispersion RB-RA=181,
RA and RB aligned to North with Red to North:  569, 247
Bonnette at 0 degree (code 0 , 0)
CAR, X axis to East, Y axis (downwards) to South.
From september 1994 until december 1995:
OFFSET_RA 569
OFFSET_RB 247
*/
/* February 1996: we get a better correction with an
offset of (-50,-44) (equiv to -17 degrees for the bonnette):
#define OFFSET_RA 519
#define OFFSET_RB 203
*/
/* January 1997: null dispersion with 57,214,
and alignment to North with 511,156 (measured by E. Aristidi) */
/*
#define OFFSET_RA 511
#define OFFSET_RB 156
*/
/* December 2015: null dispersion with 00,120,
and alignment to North with 520,130 ? */
/*
#define OFFSET_RA 520
#define OFFSET_RB 130
*/

/* CAR September 1994: increasing code number to the West (North on top) */
/* And both prisms have the same orientation ra_sign = rb_sign = 1 */
*icode1 = ra_offset + *icode1 * ra_sign;
*icode2 = rb_offset + *icode2 * ra_sign;

if(*icode1 < 0) *icode1 += 1024;
if(*icode1 > 1023) *icode1 -= 1024;
if(*icode2 < 0) *icode2 += 1024;
if(*icode2 > 1023) *icode2 -= 1024;

if(*italk) {
#ifdef INTERAC
 printf(" Code positions: RA=%d, RB=%d (1024 steps), cross_angle: %.2f (deg) \n",
        *icode1,*icode2,*cross_angle);
#endif
 if(fp1 != NULL) {
 fprintf(fp1," ra_offset=%d rb_offset=%d ra_sign=%d rb_sign=%d \n",
         ra_offset, rb_offset, ra_sign, rb_sign);
 fprintf(fp1," Code positions: RA=%d, RB=%d (1024 steps), cross_angle: %.2f (deg) \n",
        *icode1,*icode2,*cross_angle);
 fprintf(fp1,"> %d %d \n",*icode1,*icode2);
 }
  }

cangle = (*cross_angle/2.) * PI/180.;
cross_coef = 2. * cos(cangle) * radtosec / MAGNIF;

/* Computing mean deviation */
mean_dev = ind_f4(lambda_cent) * sin1 - ind_sk10(lambda_cent) * sin2;
mean_dev = mean_dev * cross_coef;

/* Residual dispersion: */
*resid_disp = (ind_f4(lambda_low) - ind_f4(lambda_high)) * sin1
             - (ind_sk10(lambda_low) - ind_sk10(lambda_high)) * sin2;
*resid_disp = ATM_DISP(lambda_low, tt, ff, pp, zd)
              - ATM_DISP(lambda_high, tt, ff, pp, zd)
              - *resid_disp * cross_coef;

if(*italk)
  {
#ifdef INTERAC
  printf(" Mean deviation at central wavelength = %.4f arcsec in focal plane\n",
      mean_dev);
  printf(" Residual dispersion (between top and bottom) = %.4f arcsec in focal plane\n",
      *resid_disp);
#endif
  if(fp1 != NULL)
  {
  fprintf(fp1," Mean deviation at central wavelength = %.4f arcsec in focal plane\n",
      mean_dev);
  fprintf(fp1," Residual dispersion (between top and bottom) = %.4f arcsec in focal plane\n",
      *resid_disp);
  }
  }

return(status);
}
/***************************************************************
* ad_owens(wave,tt,ff,pp,zd)
* Routine to compute atmospheric refraction versus wavelength
* From index formulae from Owens, J.C., Appl. Opt. v.6, p.51 (67)
* ad is in arcseconds
***************************************************************/
static double ad_owens(double wave, double tt, double ff, double pp, double zd)
{
double ad, work, zd_rad;

/* Conversion to radians: */
   zd_rad = zd*PI/180.0;

#ifdef DEBUG
printf(" Computing atmospheric refraction with Owens'formulae\n");
printf(" pp,ff,tt,zd %f %f %f %f \n",pp,ff,tt,zd);
#endif

/* Basic equation is n1 sin(i1) = n2 sin(i2)   with n1 = 1. (void).
   Small variations of n2 induce small variations of i2:
   0 = n2 cos(i2) di2 + dn2 sin(i2), and thus:
   di2 = - tan(i2) dn2 / n2
   Since n2 - 1 is very small:
   di2 = - dn2 * tan(i2)
*/

/* Reference wave is 400 nm,
   and minus sign is used to be compatible with Simon's formula */
   work = ind_air_owens(400.,tt,ff,pp) - ind_air_owens(wave,tt,ff,pp);
   work = -work * tan(zd_rad);

/* Conversion to arcseconds,*/
   ad = work * 180. * 3600. / PI;

#ifdef DEBUG
 printf(" Atm. disp. is %f arcsec at %.2f nm (outside the telescope, with 0 at 400 nm)\n",
        ad,wave);
#endif

return(ad);
}
/***************************************************************
* GET_PW_SAT
* Routine to compute saturation water pressure
* INPUT:
* tt: temperature in degrees C
***************************************************************/
int GET_PW_SAT(double *pw_sat, double tt)
{
double work, work1, work2, tt1;

tt1 = (tt + 273.16) / 273.16;

work1 = -8.29692 * (tt1 - 1.);
work2 = 4.76955 * ( 1. - 1./tt1);
work = 10.79586 * ( 1. - 1./tt1) - 5.02808 * log(tt1)
   + 1.50474E-04 * (1. - pow(10.,work1))
   + 0.42873E-03 * (pow(10.,work2) - 1.) - 2.2195983 ;

*pw_sat = 760. * pow(10.,work) ;
#ifdef DEBUG
printf(" Saturation water pressure is %f mmHg \n",*pw_sat);
#endif

return(0);
}

