/*********************************************************
* Include file needed for tav2.cpp
* JLP
* Version 20-11-2005
********************************************************/
#ifndef tav_included__
#define tav_included__

#define ABS(x) (((x) < 0) ? (-(x)) : (x))

/* April 1994: RB-RA = 180 to obtain null dispersion: */
/* April 1994: RA=371 RB=40  to obtain maximum dispersion
   with red to the north (bonnette at 90 degrees) */
/* September 1994: RB-RA = 190 ? to obtain null dispersion: */
/* September 1994: RA=569 RB=247  to obtain maximum dispersion
*   with ? red ? to the north (bonnette at 0 degrees)
* So:
*       #define RA_NULLDISP 57
*       #define RB_NULLDISP 247
*/
/* January 1997, null dispersion with 57,214
(Seen directly with the calibration target and halogen lamp)
*/
#define RA_NULLDISP 57
#define RB_NULLDISP 214

/* As it is used for declarations add 1 to sizes to allow for \0 ... */
#define MAXSTARNAMELEN 25
#define MAXALPHALEN 18
#define MAXDELTALEN 18
#define MAXEQUINLEN 9
#define MAXPRESLEN 6
#define MAXTEMPLEN 6
#define MAXHYGROLEN 5
#define MAXRISLEN 5
#define MAXCOMMANDLEN 20
#define MAXFILTERLEN 20
#define COMMENTSLEN 80
#define NFILTER_MAX 8

// For French messages, set FRENCH to 1 and ITALIAN to 0
// For Italian messages, set FRENCH to 0 and ITALIAN to 1
// For English messages, set FRENCH to 0 and ITALIAN to 0
#define LANGUAGE 1
#define FRENCH   0
#define ITALIAN  1

/* Number of wheels: */
#define NWHEELS 9

typedef struct {
 char StarNameEdit[MAXSTARNAMELEN];
 char AlphaEdit[MAXALPHALEN];
 char DeltaEdit[MAXDELTALEN];
 char EquinEdit[MAXEQUINLEN];
} TStruct_StarDlg;

typedef struct {
 char PresEdit[MAXPRESLEN];
 char TempEdit[MAXTEMPLEN];
 char HygroEdit[MAXHYGROLEN];
} TStruct_AtmDlg;

/* Configuration and status files: */
/* JLP2011:I remove c: to avoid the problem when c: does not exist! */
#define TAVCONF "\\pisco\\tavconfi.dat"
#define TAVSTAT "\\pisco\\tavstat.dat"
#define TAVPOSI "\\pisco\\tavposi.pos"
#define LOGFILE "\\pisco\\tav1.log"

#define PI 3.14159265
#define DEGTORAD   (PI/180.00)

/*** Sax Comm BVSP : */
// #include <saxcomm1.h>
// #include <settings.h>
// #include <termchld.h>

#include "rs232win.h"

#include "tav_proto.h"

#endif // EOF sentry
