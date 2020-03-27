/*******************************************************************
* Project Pisco1
*    O.M.P.
*    Copyright © 2015. All Rights Reserved.
*    AUTHOR:       Jean-Louis Prieur
*    VERSION:      21/01/2011
*    SUBSYSTEM:    Pisco1.exe Application
*    FILE:         psc1_typedef.h
*
* ========
* OVERVIEW
* ========
*    Definition of types used by this project
*
********************************************************************/
#ifndef __psc1_typedef_h
// Sentry, use file only if it's not already included.
#define __psc1_typedef_h
#include <stdint.h>

#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : (-(a)))
#endif
#ifndef MINI
#define MINI(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAXI
#define MAXI(a,b) ((a) < (b) ? (b) : (a))
#endif

#ifndef BYTE
typedef     unsigned char   BYTE;
#endif
typedef     char            INT1;
typedef     unsigned char   UINT1;
typedef     int16_t         INT2;
typedef     uint16_t        UINT2;
typedef     int32_t         INT4;
typedef     uint32_t        UINT4;
typedef     float           REAL4;
typedef     double          REAL8;

// For Andor:
typedef     unsigned short  AWORD;

typedef union {                             /* union for conversion  */
                UINT1           c[2];       /* bytes                 */
                INT2               i;       /* 2 byte integer        */
              } VI2;
typedef union {                             /* union for conversion  */
                UINT1            c[2];       /* bytes                 */
                UINT2              i;       /* 2 byte signed integer */
              } UI2;
typedef union {                             /* union for conversion  */
                UINT1           c[4];       /* bytes                 */
                INT4               i;       /* 4 byte integer        */
              } VI4;


/* DESCRIPTOR structure
*/
typedef struct {
char cvalue[80];
char keyword[8];
} DESCRIP;

typedef struct {
DESCRIP descr[10];
int ndescr;
} DESCRIPTORS;

/* Structure used for frame selection:
* Stats is a character strings with final statistics information
*/
typedef struct {
double LowCut, HighCut;
double MinVal, MaxVal;
double Sum, Sumsq;
double Nval;
char Stats[128];
} MY_SELECT;

// Processing parameters selected by the user in dialog windows:
typedef struct {
double slangle, spangle;      // Used for spectroscopic mode
int ProcessingMode;           // For long integ, autocc, bispectrum, etc
bool OnlyVisuMode;            // For displaying only the frames
bool Selection_fwhm, Selection_maxval, Statistics_is_wanted; // For frame selection
bool SubtractUnresolvedAutoc;  // To display autocc - model
                               // (to reduce the contrast near the center)
int nx, ny, nz_cube;          // size of images and cube used for processing
int in3DFITS_nz_total;        // dimension of input 3D FITS file
int out3DFITS_nz_max;         // maximum nz used for saving elementary frames
int nx_num, ny_num;           // Used for spectroscopic mode
int KK_CrossCorr;              // Delay used to compute cross-correlation
int ir_bisp, nmax_bisp;       // Used for computing the bispectrum
int OffsetThreshold;          // Offset to be removed to all frames
int SaveFramesToFitsFile;     // To save frames 0=no, 2=2D 3=3D FITS files
bool DirectVector;            // To compute "Direct Vector" (cf. Bagnuolo)
bool LuckyImaging;            // To compute "Lucky image"
bool SaveSelectionOnlyToFitsFile; // To save selected frames to 2D/3D FITS file
bool SlowProcessing;          // Option used to have a more stable behaviour
bool OffsetFrameCorrection, FlatFieldCorrection; // For pre-processing
MY_SELECT FwhmSelect, MaxValSelect;   // Used for "good frame" selection
// Filenames:
char TavConfigFileName[128], UnresolvedAutocName[128], UnresolvedModsqName[128];
char FlatFieldFileName[128], PhotModsqFileName[128], InputFileName[128];
char FFTW_directory[80];
char out3DFITS_directory[80], outResults_directory[80];
}
PISCO_SETTINGS;

#endif                                      // __psc1_typedef_h sentry.
