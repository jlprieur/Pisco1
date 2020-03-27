/****************************************************************************
* Name: psc1_pisco_filters.cpp
* PscFrame class
*
* JLP
* Version 14/09/2015
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "psc1_frame.h"
#include "psc1_frame_id.h" // Definition of identifiers

/************************************************************************
* Routine FiltersFromConfigFile
* to update the whole configuration of the speckle camera
* by reading the file TAVCONF="tavconfi.dat"
*
* Example:

AS Lampes calib.
1: Halogene
2: Filament
3: Argon
4: Mercure
5: Neon
6: Xenon
EN Roue d'entree
1: vide
2: vide
3: risque d'oc.
4: miroir calib
5: risque d'oc.
6: vide
CH Roue de champ
1: Fente
2: vide
3: mire
4: coronographe
5: vide
6: lent. de champ
MA Roue a masques
1: 4 trous
2: trou 7 mm
3: vide
4: vide
5: trous alignes
6: trous en rond
DA Roue a densites
1: vide
2: densite 1
3: densite 2
4: densite 3
5: densite 4
6: vide
DB Roue a densites
1: vide
2: densite 0.2
3: densite 0.4
4: densite 0.6
5: densite 0.8
6: vide
FA Roue a filtres
1: R 650/70
2: Grisme
3: V 550/70
4: vide
FB Roue a filtres
1: vide
2: IR 812/15
3: vide
4: BV 489/15
5: vide
6: R 614/15
GR Roue de grandisst
1: Pupille
2: Spectro
3: 4.5 mm
4: 20 mm
5: 50 mm
6: 10 mm
## Filtres:
#FA 1 650.00 70.0
#FA 3 550.00 70.0
#FB 2 812.00 15.0
#FB 4 489.00 15.0
#FB 6 614.00 15.0
*
************************************************************************/
int Psc1_PiscoPanel::FiltersFromConfigFile(char *tavconfig_fname)
{
/* Read information from a file */
FILE *fp;
char buffer[81];
int status;
wxString err_msg;

status = 0;

  if((fp = fopen(tavconfig_fname,"r")) == NULL)
    {
#if LANGUAGE == FRENCH
     err_msg.Printf(wxT(" FiltersFromConfigFile/Fichier %s introuvable"),tavconfig_fname);
#elif LANGUAGE == ITALIAN
     err_msg.Printf(wxT(" FiltersFromConfigFile/File %s non trovato"),tavconfig_fname);
#else
     err_msg.Printf(wxT(" FiltersFromConfigFile/File %s not found"),tavconfig_fname);
#endif
     wxMessageBox(err_msg, wxT("FiltersFromConfigFile"), wxOK | wxICON_ERROR);
     return(-1);
    }
;

/* Filters:
Example:
  #FB 1 650.00 67.0
*/
nfilters = 0;
  while(fgets(buffer,80,fp) != NULL)
    {
      if(buffer[0] == '#' && buffer[1] != '#')
         {
         FilterParam[nfilters] = wxString(&buffer[1]);
         FilterParam[nfilters].Trim();
         nfilters++;
         }
    }

/* Error message if no filter parameters, since Risley correction
will be impossible... */
if(nfilters == 0)
   {
#if LANGUAGE == FRENCH
    err_msg.Printf(wxT("FiltersFromConfigFile/Erreur: les param\u00c9tres des filtres sont absents dans %s"),
                tavconfig_fname);
#elif LANGUAGE == ITALIAN
    err_msg.Printf(wxT("FiltersFromConfigFile/Errore: i parametri dei filtri sono assenti in %s"),
                tavconfig_fname);
#else
    err_msg.Printf(wxT("FiltersFromConfigFile/Error: no filter parameters in %s"),
                      tavconfig_fname);
#endif
     wxMessageBox(err_msg, wxT("FiltersFromConfigFile"), wxOK | wxICON_ERROR);
     status =-1;
   }

fclose(fp);
return(status);
}
/************************************************************************
* FilterIndex
* Look for index in FilterParam corresponding to wheel current configuration
************************************************************************/
int Psc1_PiscoPanel::FilterIndex()
{
int i_posi, k_index;
int i_wheel, k_filter;
char cbuffer[80];

if(initialized != 1234) return(-1);

k_index = -1;
/* Loop on all the wheels (just in case filters are not at the good place): */
for(i_wheel = 0; i_wheel < nwheels; i_wheel++)
 {
 for(k_filter = 0; k_filter < nfilters; k_filter++)
   {
/* Example: "FA 2", first "FA" should be found in both strings: */
    strcpy(cbuffer, (const char *)FilterParam[k_filter].mb_str());
    if((wheel_name[i_wheel][0] == cbuffer[0]) &&
      (wheel_name[i_wheel][1] == cbuffer[1]))
      {
/* Then 2 should be the current wheel position: */
      sscanf(&cbuffer[3], "%1d",&i_posi);
      if(i_posi == wheel_position[i_wheel]) { k_index = k_filter; break; }
      }
   }
 }

return(k_index);
}
