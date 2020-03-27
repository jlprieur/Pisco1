// OWLCVT 11/09/94 10:01:47
/**************************************************************
* stardlg.cpp
* Set of routines dealing with star dialog to input star parameters
* Called by tav1.cpp   (first written in 1994...)
*
* Language: C++
*
* JLP
* Version of 20-11-2005
***************************************************************/
#include <stdio.h>
#include <string.h>     // for strcpy and strcat
#include <stdlib.h>     // for atoi
#include <ctype.h>      // for isdigit and isalpha

#include "stardlg.h"
#include "tav2.h"       // TTestWindow
#include "tav1.rh"

/*--------------------------------------------------
* TStarDlg implementations:
*--------------------------------------------------*/

/***********************************************************************
*
************************************************************************/
TStarDlg::TStarDlg(TWindow * AParent, LPSTR title_name, char *star_name0,
                   double alpha_value0, double delta_value0,
                   double equin_value0, TStruct_StarDlg *struct_StarDlg0)
                   :TDialog(AParent, title_name)
{
char title1[60];
int ih0, ih1;
double h2;

   Struct_StarDlg1 = struct_StarDlg0;
   astro_target = 0;

/* First initialization: */
   strcpy(StarName, star_name0);
   sprintf(Struct_StarDlg1->StarNameEdit,"%s",StarName);

/* Write current value to StarAlpha string: */
   alpha_value = alpha_value0;
   convert_coord(alpha_value,&ih0,&ih1,&h2,"H");
   sprintf(StarAlpha,"%2d %2d %.1f",ih0,ih1,h2);
   sprintf(Struct_StarDlg1->AlphaEdit,"%s",StarAlpha);

/* Write current value to StarDelta string: */
   delta_value = delta_value0;
   convert_coord(delta_value,&ih0,&ih1,&h2,"D");
   sprintf(StarDelta,"%3d %2d %.1f",ih0,ih1,h2);
/* Case of -0 30 54: */
   if(ih0 == 0 && delta_value < 0.) sprintf(StarDelta,"-0 %2d %.1f",ih1,h2);
   sprintf(Struct_StarDlg1->DeltaEdit,"%s",StarDelta);

/* Write current value to StarEquin string: */
   equin_value = equin_value0;
   sprintf(StarEquin,"%7.2f",equin_value);
   sprintf(Struct_StarDlg1->EquinEdit,"%s",StarEquin);
   TransferBuffer = (void far*)&Struct_StarDlg1;

/* Then allow interactive editing: */
  new TEdit(this, ID_STARNAMEEDIT,
      sizeof(Struct_StarDlg1->StarNameEdit));
  new TEdit(this, ID_ALPHAEDIT,
      sizeof(Struct_StarDlg1->AlphaEdit));
  new TEdit(this, ID_DELTAEDIT,
      sizeof(Struct_StarDlg1->DeltaEdit));
  new TEdit(this, ID_EQUINEDIT,
      sizeof(Struct_StarDlg1->EquinEdit));
  TransferBuffer = (void far*) Struct_StarDlg1;
  if(catalog_input)
     sprintf(title1,"Catalog: %.35s",catalog_name);
  else
#if LANGUAGE == FRENCH
     strcpy(title1,"Entrée directe des paramètres:");
#elif LANGUAGE == ITALIAN
     strcpy(title1,"Entrata diretta dei parametri:");
#else
     strcpy(title1,"Direct input of parameters:");
#endif

  catalog_input = 0;
  Catalog_Title = new TStatic(this, ID_CATALOG, title1, 15, 10, 250, 40, 60);
}

/*--------------------------------------------------
* When OK button is pressed and CanClose called,
* respond by validating the star data which
* was input.  The dialog is not ended until valid
* entries have been made in all fields.
*--------------------------------------------------*/
bool TStarDlg::CanClose()
{
  char error_message[80];
  BOOL status;

/******************************* Catalog Input ************************/
  if(catalog_input)
  {
   ::GetDlgItemText(HWindow, ID_STARNAMEEDIT, StarName, MAXSTARNAMELEN);
   status = Read_Catalog1(catalog_name,StarName,StarAlpha,StarDelta,StarEquin,
                   error_message);
   if(status != TRUE)
     {
#if LANGUAGE == FRENCH
       ::MessageBox(HWindow,error_message,"Read_Catalog1: erreur", MB_OK);
#elif LANGUAGE == ITALIAN
       ::MessageBox(HWindow,error_message,"Read_Catalog1: errore", MB_OK);
#else
       ::MessageBox(HWindow,error_message,"Read_Catalog1: error", MB_OK);
#endif
     return FALSE;
     }
/* JLP96:
   else
    {
    sprintf(error_message,">%s<",StarEquin);
    ::MessageBox(HWindow,error_message,"Read_Catalog1: OK", MB_OK);
    }
JLP96 */
  }
  else
  {
/******************************* Direct Input ************************/
/* Transfer to buffers: */
   ::GetDlgItemText(HWindow, ID_STARNAMEEDIT, StarName, MAXSTARNAMELEN);
   ::GetDlgItemText(HWindow, ID_ALPHAEDIT, StarAlpha, MAXALPHALEN);
   ::GetDlgItemText(HWindow, ID_DELTAEDIT, StarDelta, MAXDELTALEN);
   ::GetDlgItemText(HWindow, ID_EQUINEDIT, StarEquin, MAXEQUINLEN);

/* Make sure all input fields (except the name) have been filled in. */
  if (!*StarAlpha || !*StarDelta || !*StarEquin)
    {
#if LANGUAGE == FRENCH
    ::MessageBox(HWindow,
                 "Remplissez tous les champs svp","Erreur d'entrée", MB_OK);
#elif LANGUAGE == ITALIAN
    ::MessageBox(HWindow,
                 "Compilate tutti i campi per favore","Errore d'entrata", MB_OK);
#else
    ::MessageBox(HWindow,
                 "All fields must be filled in","Input Error", MB_OK);
#endif
     return FALSE;
    }
  }

/*************** Now since fields are filled,
**************** Make sure all input fields have valid data */
  if ( ValidAlpha() && ValidDelta())
    {
// ValidEquin to update equin_value
     ValidEquin();
/* Refresh screen, with new values of target name and Risley correction */
/* JLP95 some obscure problems made the program aborted here (in version 3.1)
   Now it seems to be fixed...
*/
#if 0
     if(TTwin->RefreshScreen(error_message, 1))
       {
#if LANGUAGE == FRENCH
       ::MessageBox(HWindow,error_message,"Correction incomplète", MB_OK);
#elif LANGUAGE == ITALIAN
       ::MessageBox(HWindow,error_message,"Correzione incompleta", MB_OK);
#else
       ::MessageBox(HWindow,error_message,"Limited correction", MB_OK);
#endif
       }
//      else ::MessageBox(HWindow,error_message,"Correction complète", MB_OK);
#endif

/* End of ValidAlpha... */
     }
return TRUE;
}
/*--------------------------------------------------
* Validate star right ascension.
*
* INPUT:
* StarAlpha: string with alpha_value
*
* OUTPUT:
* alpha_value: value of alpha in hours
*--------------------------------------------------*/
BOOL TStarDlg::ValidAlpha()
{
  char *pc, error_message[80];
  int ih0, ih1, good_syntax = 1;
  double h2;
  BOOL valid;

// Following is not selective at all...
//   if(sscanf(StarAlpha,"%d %d %lf",&ih0,&ih1,&h2) == EOF)
// So I do it myself:
  StarAlpha[MAXALPHALEN-1] = '\0';
  pc = StarAlpha;
  while(*pc)
    {
    if(!isdigit(*pc) && *pc != ' ' && *pc != '.')
       {good_syntax = 0; break;}
    pc++;
    }

/* Set the value of "valid": */
  valid = FALSE;
  if(good_syntax)
     {
     sscanf(StarAlpha,"%d %d %lf",&ih0,&ih1,&h2);
     valid = TRUE;
     if(ih0 < 0 || ih0 > 24 || ih1 < 0 ||  ih1 > 60
             || h2 < 0. || h2 > 60.) valid = FALSE;
     }

/* Transfer new value if everything is OK */
// alpha_value in hours
  if(valid == TRUE)
     alpha_value = (double)ih0 + ((double)ih1)/60. + h2/3600.;
  else
    {
#if LANGUAGE == FRENCH
     sprintf(error_message,
     "Alpha incorrect (%.25s), exemples: 23 2 46  ou  3 42 5.3",StarAlpha);
     ::MessageBox(HWindow, error_message, "Erreur d'entrée", MB_OK);
#elif LANGUAGE == ITALIAN
     sprintf(error_message,
     "Alpha cattivo (%.25s), per essempio: 23 2 46  o  3 42 5.3",StarAlpha);
     ::MessageBox(HWindow, error_message, "Errore d'entrata", MB_OK);
#else
     sprintf(error_message,
        "Wrong alpha (%.25s), should be like  23 2 46  or  3 42 5.3",StarAlpha);
     ::MessageBox(HWindow, error_message, "Input Error", MB_OK);
#endif
   }

  return valid;
}
/*--------------------------------------------------
* Validate star declination.
*
* INPUT:
* StarDelta: string with delta_value
*
* OUTPUT:
* delta_value: value of delta in degrees
*--------------------------------------------------*/
BOOL TStarDlg::ValidDelta()
{
  char *pc, error_message[80];
  int ih0, ih1, good_syntax = 1;
  double h2;
  BOOL valid;

// Following is not selective at all...
//   if(sscanf(StarDelta,"%d %d %lf",&ih0,&ih1,&h2) == EOF)
// So I do it myself:
  StarDelta[MAXDELTALEN-1] = '\0';
  pc = StarDelta;
  while(*pc)
    {
    if(!isdigit(*pc) && *pc != ' ' && *pc != '-' && *pc != '.' )
       {good_syntax = 0; break;}
    pc++;
    }

  valid = FALSE;
  if(good_syntax)
     {
       sscanf(StarDelta,"%d %d %lf",&ih0,&ih1,&h2);
       valid = TRUE;
       if(ih0 < -90 || ih0 > 90 || ih1 < 0 ||  ih1 > 60
             || h2 < 0. || h2 > 60.) valid = FALSE;
     }

/* Transfer new value if everything is OK */
if(valid == TRUE)
   {
/* Look for "-" sign: (should process correctly -0 30 10...) */
   pc = StarDelta;
/* Stop at "-" or at the first figure encountered: */
   while(*pc && *pc !='-' && isdigit(*pc)) pc++;
/* Note that if "-" is present, ih0 is negative: */
    if(*pc == '-')
       delta_value = (double)ih0 - ((double)ih1)/60. - h2/3600.;
    else
       delta_value = (double)ih0 + ((double)ih1)/60. + h2/3600.;
   }
  else
   {
#if LANGUAGE == FRENCH
     sprintf(error_message,
          "Delta incorrect (%.25s), exemples: 23 2 46 ou -3 42 5.3",StarDelta);
          ::MessageBox(HWindow, error_message, "Erreur d'entrée", MB_OK);
#elif LANGUAGE == ITALIAN
     sprintf(error_message,
          "Delta cattivo (%.25s), per essempio: 23 2 46 o -3 42 5.3",StarDelta);
          ::MessageBox(HWindow, error_message, "Errore d'entrata", MB_OK);
#else
     sprintf(error_message,
       "Wrong Delta (%.25s), should be like  23 2 46  or  -3 42 5.3",StarDelta);
     ::MessageBox(HWindow, error_message, "Input Error", MB_OK);
#endif
   }

  return valid;
}

/*--------------------------------------------------
* Validate star Equin.
*
* INPUT:
* StarEquin: string with equin_value
*
* OUTPUT:
* equin_value: float value of equinox in years
*--------------------------------------------------*/
BOOL TStarDlg::ValidEquin()
{
  char *pc, error_message[90];
  int good_syntax;
  double w1;
  BOOL valid;

  StarEquin[MAXEQUINLEN-1] = '\0';
  pc = StarEquin;
  good_syntax = 1;
/* JLP96: allow for CR at the end of the line: */
  while(*pc && *pc != '\r' && *pc != '\f' && *pc != '\n')
    {
    if(!isdigit(*pc) && *pc != '.' && *pc != ' ')
       {good_syntax = 0; break;}
    pc++;
    }

  valid = FALSE;

  if(good_syntax)
     {
     valid = TRUE;
     sscanf(StarEquin,"%lf",&w1);
     if(w1 != 0. && (w1 < 1600. || w1 > 2100.) ) valid = FALSE;
     }

/* Transfer new value if everything is OK */
if(valid == TRUE)
     equin_value = w1;
 else
  {
#if LANGUAGE == FRENCH
     sprintf(error_message,"StarEquin=>%s< %s",StarEquin,
             "Equinoxe incorrect! Devrait etre: 1950, 2000.0 ou 0...");
     ::MessageBox(HWindow, error_message, "Erreur d'entrée", MB_OK);
#elif LANGUAGE == ITALIAN
     sprintf(error_message,"StarEquin=>%s< %s",StarEquin,
             "Equinozio cattivo! Dovrebbe essere: 1950, 2000.0 o 0...");
     ::MessageBox(HWindow, error_message, "Errore d'entrata", MB_OK);
#else
     sprintf(error_message,"StarEquin=>%s< %s",StarEquin,
             "Wrong equinox value! Should be like 1950, 2000.0 or 0");
     ::MessageBox(HWindow, error_message, "Input Error", MB_OK);
#endif
     equin_value = 0.;
  }

  return(valid);
}
/***************************************************************
*
****************************************************************/
void TTestWindow::StarInput()
{
// #define DEBUGStar
#ifdef DEBUGStar
char StarInfo[MAXSTARNAMELEN + MAXALPHALEN + MAXDELTALEN + MAXEQUINLEN + 5] ;
#endif

  if ( GetModule()->ExecDialog(
               new TStarDlg(this,"StarINFO", StarName, alpha_value,
               delta_value, equin_value, &Struct_StarDlg1)) == IDOK )
  {
// DEBUG:
#ifdef DEBUGStar
/* Display info... */
    strcpy(StarInfo, Struct_StarDlg1.StarNameEdit);
    strcat(StarInfo, "\n");
    strcat(StarInfo, Struct_StarDlg1.AlphaEdit);
    strcat(StarInfo, "\n");
    strcat(StarInfo, Struct_StarDlg1.DeltaEdit);
    strcat(StarInfo, "\n");
    strcat(StarInfo, Struct_StarDlg1.EquinEdit);
    sprintf(StarInfo," alph=%f delt=%f equi=%f ",
                      alpha_value,delta_value,equin_value);
    ::MessageBox(HWindow, StarInfo,"Information stored", MB_OK);
#endif
/* Update astro_target: */
     astro_target = 1;
  }

return;
}
/************************************************************************
* Routines UpdateAtmStatus, UpdateStarStatus, ...
* to update the whole configuration of the speckle camera
* by reading the file TAVSTAT="tavstat.dat"
* Example:
EN 1
CH 5
MA 3
DA 2
DB 4
FA 2
FB 5
GR 3
RA 0034
RB 0453
#Name_ HD 2345
#Alpha 23 45 45.5
#Delta -3 45 32
#Equin 1950.0
*Press 453.0
*Tempe -5.0
*Hygro 65
&Bonne 123
************************************************************************/
/************************************************************************
* UpdateStarFromStatusFile
* Example, for syntax:
  #Name_ HD 2345
  #Alpha 23 45 45.5
  #Delta -3 45 32
  #Equin 1950.0
************************************************************************/
int TTestWindow::UpdateStarFromStatusFile(char *TAVSTAT_fname,
                                          char *error_message)
{
  FILE *fp;
  char buffer[80], *pc;
  char star_alpha[MAXALPHALEN], star_delta[MAXDELTALEN];
  char star_equin[MAXEQUINLEN];
  int status, name_found, alpha_found, delta_found, equin_found;
  int ih0, ih1;
  double h2;

status = 0;

/* Open data file : */
  if((fp = fopen(TAVSTAT_fname,"r")) == NULL)
    {
     sprintf(error_message," UpdateStarFromStatusFile/File %s not found",
             TAVSTAT_fname);
     return(-1);
    }

sprintf(error_message," UpdateStarFromStatusFile/OK, file %s found",
   TAVSTAT_fname);

name_found = 0;
alpha_found = 0;
delta_found = 0;
equin_found = 0;

  while(fgets(buffer,40,fp) != NULL)
    {
/* Name: */
      if(!strncmp("#Name_",buffer,6))
         {
         name_found = 1;
         pc = buffer + 7;
         strncpy(StarName,pc,MAXSTARNAMELEN);
         clean_buffer(StarName,MAXSTARNAMELEN);
         }
/* Alpha: */
      if(!strncmp("#Alpha",buffer,6))
         {
         alpha_found = 1;
         pc = buffer + 7;
         strncpy(star_alpha,pc,MAXALPHALEN);
         clean_buffer(star_alpha,MAXALPHALEN);
         if(sscanf(star_alpha,"%d %d %lf",&ih0,&ih1,&h2) == EOF)
            {
            sprintf(error_message,
                  " UpdateStarFromStatusFile/Error reading alpha in %s\n",TAVSTAT);
            return(-1);
            }
/* astro_target set to one if alpha != -1 */
           if(ih0 == -1)
             {
             astro_target = 0;
             alpha_value = 0.;
             }
           else
             {
             astro_target = 1;
             alpha_value = (double)ih0 + ((double)ih1)/60. + h2/3600.;
             }
         }
/* Delta: */
      if(!strncmp("#Delta",buffer,6))
         {
         delta_found = 1;
         pc = buffer + 7;
         strncpy(star_delta,pc,MAXDELTALEN);
         clean_buffer(star_delta,MAXDELTALEN);
         if(sscanf(star_delta,"%d %d %lf",&ih0,&ih1,&h2) == EOF)
            {
            sprintf(error_message,
                  " UpdateStarFromStatusFile/Error reading delta in %s\n",TAVSTAT);
            return(-1);
            }
/* Be careful, if negative values... */
          if((ih0 < 0) || (ih0 == 0 && (star_delta[0] == '-' || star_delta[1] == '-')))
            delta_value = (double)ih0 - ((double)ih1)/60. - h2/3600.;
          else
            delta_value = (double)ih0 + ((double)ih1)/60. + h2/3600.;
          }
/* Equin: */
      if(!strncmp("#Equin",buffer,6))
         {
         equin_found = 1;
         pc = buffer + 7;
         strncpy(star_equin,pc,MAXEQUINLEN);
         clean_buffer(star_equin,MAXEQUINLEN);
         if(sscanf(star_equin,"%lf",&equin_value) == EOF)
            {
            sprintf(error_message,
                  " UpdateStarFromStatusFile/Error reading equinox in %s\n",TAVSTAT);
            return(-1);
            }
         }
   }

if(!name_found || !alpha_found || !delta_found || !equin_found)
  {
   sprintf(error_message," Missing parameter in %s %d %d %d %d",TAVSTAT,
           name_found, alpha_found, delta_found, equin_found);
   status = -1;
  }

 fclose(fp);

 return(status);
}
/************************************************************************
*
************************************************************************/
void TTestWindow::CatalogInput()
{
TOpenSaveDialog::TData FileData;

  char title1[40], s[40];
#if LANGUAGE == FRENCH
  strcpy(title1,"Nom du catalogue astronomique");
#elif LANGUAGE == ITALIAN
  strcpy(title1,"Nome del catalogo astronomico");
#else
  strcpy(title1,"Input astronomical catalogue");
#endif

  catalog_input = 1;
/* Use dialog box "TFileOpenDialog" */
  FileData.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
  FileData.SetFilter("Catalog files (*.cat)|*.cat|AllFiles (*.*)|*.*|");
  strcpy(s,"c:\\pisco\\");
  FileData.InitialDir = s;
  if (TFileOpenDialog(GetApplication()->MainWindow, FileData, 0, title1).Execute()
      != IDOK)
     {
     *catalog_name = '\0';
     catalog_input = 0;
     }
  else
     strcpy(catalog_name,FileData.FileName);
}
/********************************************************************
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
int Read_Catalog1(char *catalog_name, char *StarName,
                   char *StarAlpha, char *StarDelta, char *StarEquin,
                   char *error_message)
{
int found, name_length;
char buffer[80], *pc_buff, *pc_name, *pc1, buff1[80];
FILE *fp;

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
#if LANGUAGE == FRENCH
      strcpy(error_message,"Champ vide pour le nom de l'objet");
#elif LANGUAGE == ITALIAN
      strcpy(error_message,"Campo vuoto per il nome dell'oggetto");
#else
      strcpy(error_message,"Empty field for target name");
#endif
      return FALSE;
     }

/* Open catalog : */
  if((fp = fopen(catalog_name,"r")) == NULL)
    {
#if LANGUAGE == FRENCH
     sprintf(error_message,"Erreur en ouvrant le catalogue: %.30s",
               catalog_name);
#elif LANGUAGE == ITALIAN
     sprintf(error_message,"Errore aprendo il catalogo: %.30s",
               catalog_name);
#else
     sprintf(error_message,"Error opening the catalog: %.30s",catalog_name);
#endif
     return FALSE;
    }

  found = 0;
/* Loop on all the lines: */
  while(fgets(buffer,79,fp) != NULL)
    {
      pc_buff = buffer;
      buffer[79] = '\0';
/* Allow for one or more "&" at the beginning: */
      while(*pc_buff == '&') pc_buff++;
/* Remove blanks at the beginning: */
      while(*pc_buff == ' ') pc_buff++;
/* Check if \cr has been found, or if end of line: */
      if(strncmp(pc_buff,"\cr",3) && *pc_buff)
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
           strncpy(StarName,buff1,MAXSTARNAMELEN);
           StarName[MAXSTARNAMELEN-1] ='\0';
           break;
          }
        }
    }
 fclose(fp);

/* Error message if needed: */
if(!found)
 {
#if LANGUAGE == FRENCH
   sprintf(error_message,"Cet objet (%.20s) n'est pas dans le catalogue",
           StarName);
#elif LANGUAGE == ITALIAN
   sprintf(error_message,"Quest'oggetto (%.20s) non è nello catalogo",
           StarName);
#else
   sprintf(error_message,"This object (%.20s) is not in the catalog",
           StarName);
#endif
 return FALSE;
 }

/****************** Entry has been found  **********************/
/* Now decode coordinates from the buffer: */
    pc_buff += name_length;
/* Update error message in case of a problem: */
#if LANGUAGE == FRENCH
    strcpy(error_message,"Erreur en lisant les coordonnées");
#elif LANGUAGE == ITALIAN
    strcpy(error_message,"Errore leggendo le coordinate");
#else
    strcpy(error_message,"Error reading coordinates");
#endif

/* First & (between name and alpha): */
      while(*pc_buff != '&' && *pc_buff) pc_buff++;
      if(!*pc_buff) return FALSE;
/* Allow for one "&" or more, and some blanks: */
      while(*pc_buff == '&' || *pc_buff == ' ') pc_buff++;
      pc1 = pc_buff;
/* Second & (between alpha and delta): */
      while(*pc_buff != '&' && *pc_buff) pc_buff++;
      if(!*pc_buff) return FALSE;
/* Write 0 at the end of Alpha string: */
      *pc_buff = '\0'; pc_buff++;
/* Save StarAlpha value: */
      strncpy(StarAlpha,pc1,MAXALPHALEN);
      StarAlpha[MAXALPHALEN-1] = '\0';
/* Allow for one "&" or more, and some blanks: */
      while(*pc_buff == '&' || *pc_buff == ' ') pc_buff++;
      pc1 = pc_buff;
/* Third & (between delta and equinox): */
      while(*pc_buff != '&' && *pc_buff) pc_buff++;
      if(!*pc_buff) return FALSE;
/* Write 0 at the end of Delta string: */
      *pc_buff = '\0'; pc_buff++;
/* Save StarDelta value: */
      strncpy(StarDelta,pc1,MAXDELTALEN);
      StarDelta[MAXDELTALEN-1] = '\0';
/* Allow for one "&" or more, and some blanks: */
      while(*pc_buff == '&' || *pc_buff == ' ') pc_buff++;
      pc1 = pc_buff;
/* Fourth &, '\r'=CR, ' ', or \cr (after equinox): */
      while(*pc_buff != '&' && *pc_buff != '\r' && *pc_buff != ' '
            && *pc_buff != '\\' && *pc_buff) pc_buff++;
/* Write 0 at the end of Equinox string: */
      *pc_buff = '\0';
/* Save StarEquin value: */
      strncpy(StarEquin,pc1,MAXEQUINLEN);
      StarEquin[MAXEQUINLEN-1] = '\0';

return TRUE;
}

