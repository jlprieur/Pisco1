/****************************************************************************
* Name: psc1_target_catalogs.cpp
* Psc1_TargetPanel class
*
* JLP
* Version 20/11/2015
****************************************************************************/
#include "psc1_frame_id.h"
#include "psc1_target_panel.h"
#include "psc1_pisco_panel.h"
#include "tav_utils.h"   // convert_coord()

static int FormatOfCatalog1(const char* fname, int *cat_type);

/****************************************************************************
* Open JLP-formatted or WDS double star catalog
****************************************************************************/
void Psc1_TargetPanel::OnOpenCatalog1(wxCommandEvent& event)
{
int status;
// Select name for output logbook file:
wxFileDialog openFileDialog(this, wxT("Open double star PISCO or WDS catalog"),
                            wxT(""), wxT(""),
                            wxT("Catalog files (*.txt)|*.txt"));

catalog1_fname = wxT("");
// Catalog type: 1=PISCO 2=WDS
catalog_type = 0;

// To avoid conflicts with modal dialog, stop the timers:
if((initialized == 1234) && (m_psc1_pisco_panel != NULL))
    m_psc1_pisco_panel->PauseMyTimers();

status = openFileDialog.ShowModal();

// Restart the timers:
if((initialized == 1234) && (m_psc1_pisco_panel != NULL))
    m_psc1_pisco_panel->ResumeMyTimers();

if (status == wxID_CANCEL) return;

catalog1_fname = openFileDialog.GetFilename();

// Update catalog name:
 PscStatic_Catalog1->SetLabel(catalog1_fname);

// Find format of catalog:
 FormatOfCatalog1((const char *)catalog1_fname, &catalog_type);

return;
}
/****************************************************************************
* Open ADS-WDS cross-reference catalog
****************************************************************************/
void Psc1_TargetPanel::OnOpenADSWDSCrossRefCatalog(wxCommandEvent& event)
{
int status;
// Select name for output logbook file:
wxFileDialog openFileDialog(this, wxT("Open ADS-WDS cross-reference catalog"),
                            wxT(""), wxT(""),
                            wxT("Catalog files (*.txt)|*.txt"));

ADS_WDS_cross_fname = wxT("");

// To avoid conflicts with modal dialog, stop the timers:
if((initialized == 1234) && (m_psc1_pisco_panel != NULL))
    m_psc1_pisco_panel->PauseMyTimers();

status = openFileDialog.ShowModal();

// Restart the timers:
if((initialized == 1234) && (m_psc1_pisco_panel != NULL))
    m_psc1_pisco_panel->ResumeMyTimers();

if (status == wxID_CANCEL) return;

ADS_WDS_cross_fname = openFileDialog.GetFilename();

// Update catalog name:
 PscStatic_ADSWDS_crossref->SetLabel(ADS_WDS_cross_fname);

return;
}
/********************************************************************
* Look for an object in JLP's double star catalog.
*
* INPUT:
* StarName
* OUTPUT:
* StarAlpha, StarDelta, StarEquin
*
* Example: (LateX format)

HD 234    & 2 32 45.0 & 25 50 13 & 1950 \cr
NGC 2343  & 6 24 42.0 & 15 50 23 & 2000 \cr
SAO 123456  & 12 14 51.0 & -5 50 3 & 0 \cr
23 CYG   & 18 44 23.5 & -15 10 33 & 0 \cr

(should also be possible with TeX format:
& HD 234    && 2 32 45.0 && 25 50 13 && 1950 &\cr)
********************************************************************/
int Psc1_TargetPanel::FindObjectInPiscoCatalog(wxString& error_messg)
{
int found, name_length, ih0, ih1;
double h2;
char cbuffer[80], *pc_buff, *pc_name, *pc1, buff1[80];
FILE *fp;
char StarName[80], StarAlpha[80], StarDelta[80], StarEquin[80];

  strcpy(StarName, (const char*)target_name);
/* Remove blanks at the end: */
   pc1 = StarName;
   while(*pc1) pc1++;
   pc1--;
   while(*pc1 == ' ') pc1--;
   pc1++;
   *pc1 = '\0';
/* Remove blanks in the beginning: */
   pc_name = StarName;
   while(*pc_name == ' ') pc_name++;
/* Look for name length: */
   name_length = 0;
   pc1 = pc_name;
   while(*pc1) {name_length++; pc1++;}
   if(name_length == 0)
     {
// i=304 "Empty field for target name");
      error_messg = m_messg[304];
      return(-1);
     }

/* Open catalog : */
  strcpy(cbuffer, (const char *)catalog1_fname.mb_str());
  if((fp = fopen(cbuffer,"r")) == NULL)
    {
// i=305 "Error opening the catalog:"
     error_messg = m_messg[305]; // + catalog1_fname;
     return(-1);
    }

  found = 0;
/* Loop on all the lines: */
  while(fgets(cbuffer,79,fp) != NULL)
    {
      pc_buff = cbuffer;
      cbuffer[79] = '\0';
/* Allow for one or more "&" at the beginning: */
      while(*pc_buff == '&') pc_buff++;
/* Remove blanks at the beginning: */
      while(*pc_buff == ' ') pc_buff++;
/* Check if \r has been found, or if end of line: */
      if(strncmp(pc_buff,"\r",3) && *pc_buff)
        {
/* Name: */
        if(!strncmp(pc_name,pc_buff,name_length))
           {
           found = 1;
/* Look for end of string name: */
           strcpy(buff1,pc_buff);
           pc1 = buff1;
           while(*pc1 != '&' && *pc1) pc1++;
           *pc1 = '\0';
/* Copy new name to old one, since it can be more precise: */
           strcpy(StarName,buff1);
           break;
          }
        }
    }
 fclose(fp);

/* Error message if needed: */
if(!found)
 {
// i=306 "This object is not in the catalog",
      error_messg = target_name + m_messg[306];
 return(-1);
 }

/****************** Entry has been found  **********************/
/* Now decode coordinates from the buffer: */
    pc_buff += name_length;
/* Update error message in case of a problem further below: */
// i=307 "Error reading coordinates"
      error_messg = m_messg[307];

/* First & (between name and alpha): */
      while(*pc_buff != '&' && *pc_buff) pc_buff++;
      if(!*pc_buff) return(-1);
/* Allow for one "&" or more, and some blanks: */
      while(*pc_buff == '&' || *pc_buff == ' ') pc_buff++;
      pc1 = pc_buff;
/* Second & (between alpha and delta): */
      while(*pc_buff != '&' && *pc_buff) pc_buff++;
      if(!*pc_buff) return(-1);
/* Write 0 at the end of Alpha string: */
      *pc_buff = '\0'; pc_buff++;
/* Save StarAlpha value: */
      strcpy(StarAlpha,pc1);
/* Allow for one "&" or more, and some blanks: */
      while(*pc_buff == '&' || *pc_buff == ' ') pc_buff++;
      pc1 = pc_buff;
/* Third & (between delta and equinox): */
      while(*pc_buff != '&' && *pc_buff) pc_buff++;
      if(!*pc_buff) return(-1);
/* Write 0 at the end of Delta string: */
      *pc_buff = '\0'; pc_buff++;
/* Save StarDelta value: */
      strcpy(StarDelta,pc1);
/* Allow for one "&" or more, and some blanks: */
      while(*pc_buff == '&' || *pc_buff == ' ') pc_buff++;
      pc1 = pc_buff;
/* Fourth &, '\r'=CR, ' ', or \cr (after equinox): */
      while(*pc_buff != '&' && *pc_buff != '\r' && *pc_buff != ' '
            && *pc_buff != '\\' && *pc_buff) pc_buff++;
/* Write 0 at the end of Equinox string: */
      *pc_buff = '\0';
/* Save StarEquin value: */
      strcpy(StarEquin,pc1);

target_name = wxString(StarName);
sscanf(StarAlpha, "%d %d %lf", &ih0, &ih1, &h2);
alpha_value = (double)ih0 + (double)ih1 / 60. + h2 / 3600.;
sscanf(StarDelta, "%d %d %lf", &ih0, &ih1, &h2);
if(ih0 >= 0) {
  delta_value = (double)ih0 + (double)ih1 / 60. + h2 / 3600.;
  } else {
  delta_value = (double)ih0 - (double)ih1 / 60. - h2 / 3600.;
  }
sscanf(StarEquin, "%lf", &h2);
equin_value = h2;

return(0);
}
/**************************************************************
*
* OUTPUT:
*  cat_type : 0 if unknown, 1 if JLP-PISCO format, 2 if WDS format
*
***************************************************************/
static int FormatOfCatalog1(const char* fname, int *cat_type)
{
FILE *fp;
char cbuffer[80];

*cat_type = 0;
if((fp = fopen(fname, "r")) == NULL ) {
   return(-1);
   }

fgets(cbuffer, 79, fp);
 if(cbuffer[0] == '%') {
   *cat_type = 1;
 } else if(cbuffer[0] == '0') {
   *cat_type = 2;
 }

fclose(fp);

return(0);
}

