// OWLCVT 11/09/94 10:00:19
/**************************************************************
* logbook.cpp
* Set of routines to create and uptdate a logbook
* Called by tav1.cpp  (first written in 1995...)
*
* Language: C++
*
* JLP
* Version of 29-01-2007
***************************************************************/
#pragma hdrfile "logbook_.csm"
#include "owl\compat.h"
#include <stdio.h>

#include <owl\owlall.h>
#include <owl\window.h>
#include <owl\static.h>
#include <owl\button.h>
#include <owl\edit.h>
#include <string.h>     // for strcpy and strcat
#include <owl\opensave.h>    // New TFile Dialog (OWL 2.0)
#include <owl\inputdia.h>   // To use ExecDialog...
#pragma hdrstop

/* MY_EXTERN has to be defined BEFORE including "tav.h" */
#define MY_EXTERN extern
#include "tav.h"      // for CM_xxx and ID_xxxx and MAXxxxLEN definitions
#include "tav2.h"

static TOpenSaveDialog::TData  FileData;      // save/restore info.

#define BUFF_LEN1 580
static char answer1[BUFF_LEN1], error_message1[BUFF_LEN1];

/********************************************************************
* InputComments
*********************************************************************/
int TTestWindow::InputComments()
{
 char title1[80], prompt1[40];
 int start_expo;

 start_expo = 1;

/********************** Ask for comments ****************/
  *Comments = '\0';
#if LANGUAGE == FRENCH
  strcpy(title1,"Nouvelle pose: (débute après RC (Retour Chariot))");
  strcpy(prompt1,"Commentaires:");
#elif LANGUAGE == ITALIAN
  strcpy(title1,"Nuova posa: (inizia doppo CR (INVIO))");
  strcpy(prompt1,"Commenti:");
#else
  strcpy(title1,"New exposure: (start after CR (Carriage Return))");
  strcpy(prompt1,"Comments:");
#endif

   if ( GetApplication()->ExecDialog(new
   TInputDialog(this, title1, prompt1, Comments, sizeof(Comments)-1))
   != IDOK ) {*Comments = '\0'; start_expo = 0;}

return(start_expo);
}


/************************************************************************
* Routines UpdateWheelsFromStatusFile
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
int TTestWindow::UpdateWheelsFromStatusFile(char *error_message)
{
  FILE *fp;
  char buffer[80];
  register int i, k;
  int status, wheel_found, ra_found, rb_found, bonne_found;

status = 0;
wheel_found = 0;
ra_found = 0;
rb_found = 0;
bonne_found = 0;

  if((fp = fopen(TAVSTAT,"r")) == NULL)
    {
     sprintf(error_message," UpdateWheelsFromStatusFile/File %s not found",TAVSTAT);
     return(-1);
    }

sprintf(error_message," UpdateWheelsFromStatusFile/OK, file %s found",TAVSTAT);

/* First initialize wheel_is_there array: */
  for(k = 0; k < NWHEELS; k++) wheel_is_there[k] = 0;

/* Then look for the wheels and their positions: */
  for(k = 0; k < NWHEELS; k++)
   {
    if(fgets(buffer,60,fp) == NULL)
    {
     sprintf(error_message," UpdateWheelsFromStatusFile/Error 2 reading %s",TAVSTAT);
     fclose(fp); return (-1);
    }
      for (i = 0; i < NWHEELS; i++)
          {
          if(!strncmp(wheel_title[i],buffer,2))
                {
                wheel_found++;
                if(sscanf(&buffer[3],"%1d",&wheel_position[k]) == EOF)
                  {
                    sprintf(error_message,
                    " UpdateWheelsFromStatusFile/Error reading position of wheel %s",
                    wheel_title[k]);
                    fclose(fp); return (-1);
                  }
                else wheel_is_there[k] = 1;
                }
          }
    }

/* Continue reading the file: */
  while(fgets(buffer,40,fp) != NULL)
    {
/* ra_position: */
      if(!strncmp("RA",buffer,2))
         {
         ra_found = 1;
         if(sscanf(buffer,"RA %04d",&ra_position) == EOF)
             {
             sprintf(error_message,
              " UpdateWheelsFromStatusFile/Error reading RA position in %s",TAVSTAT);
             fclose(fp); return (-1);
             }
         }
/* rb_position: */
      if(!strncmp("RB",buffer,2))
         {
         rb_found = 1;
         if(sscanf(buffer,"RB %04d",&rb_position) == EOF)
             {
             sprintf(error_message,
              " UpdateWheelsFromStatusFile/Error reading RB position in %s",TAVSTAT);
             fclose(fp); return (-1);
             }
         }
    }

if(wheel_found != NWHEELS || !ra_found || !rb_found || !bonne_found)
  {
   sprintf(error_message,
"UpdateWheelsFromStatusFile/Missing parameter in %s \n nwheels=%d ra=%d rb=%d bonne=%d",
                 TAVSTAT,wheel_found,ra_found,rb_found,bonne_found);
   status = -1;
  }

  fclose(fp);

return(status);
}
/************************************************************************
* SaveStatus
* Write new values to status file
************************************************************************/
int TTestWindow::SaveStatus(char *error_message)
{
  FILE *fp;
  int status, ih0, ih1;
  double h2;
  register int k;

status = 0;

/* Open output data file: */
  if((fp = fopen(TAVSTAT,"w")) == NULL)
    {
     sprintf(error_message," SaveStatus/Error opening output file %s",TAVSTAT);
     return(-1);
    }
/* Wheel positions
* Example: DA 2
*/
  for(k = 0; k < NWHEELS; k++)
   {
    fprintf(fp,"%c%c %1d \n", wheel_title[k][0], wheel_title[k][1],
            wheel_position[k]);
   }

/* Risley prism positions
Example:
  RA 0034
  RB 0453
*/
   fprintf(fp,"RA %04d \n",ra_position);
   fprintf(fp,"RB %04d \n",rb_position);

/* Star name:
     #Name_ HD 2345
*/
   fprintf(fp,"#Name_ %s\n",StarName);

/* Coordinates:
     #Alpha 23 45 45.5
     #Delta -3 45 32
*/
   if(astro_target)
   {
     convert_coord(alpha_value,&ih0,&ih1,&h2,"H");
     fprintf(fp,"#Alpha %d %d %.1f\n",ih0,ih1,h2);
     convert_coord(delta_value,&ih0,&ih1,&h2,"D");
/* Case of -0 30 54: */
     if(ih0 == 0 && delta_value < 0.)
       fprintf(fp,"#Delta -0 %2d %.1f\n",ih1,h2);
     else
       fprintf(fp,"#Delta %d %2d %.1f\n",ih0,ih1,h2);

/* Equinox:
     #Equin 1950.00
*/
     fprintf(fp,"#Equin %.2f\n",equin_value);
   }
   else
   {
     alpha_value = -1.;
     fprintf(fp,"#Alpha %d %d %.1f\n",-1,0,0.);
     delta_value = 0;
     convert_coord(delta_value,&ih0,&ih1,&h2,"D");
/* Case of -0 30 54: */
     if(ih0 == 0 && delta_value < 0.)
       fprintf(fp,"#Delta -0 %2d %.1f\n",ih1,h2);
     else
       fprintf(fp,"#Delta %d %2d %.1f\n",ih0,ih1,h2);
     equin_value = 0.;
     fprintf(fp,"#Equin %.2f\n",equin_value);
   }

/* Parameters of the atmosphere:
     *Press 453.0
     *Tempe -5.0
     *Hygro 65
*/
   fprintf(fp,"*Tempe %.1f\n",temp_value);
   fprintf(fp,"*Press %.1f\n",press_value);
   fprintf(fp,"*Hygro %.1f\n",hygro_value);

/* Close TAVSTAT file:*/
 fclose(fp);
 return(status);
}
/************************************************************************
* FilterIndex
* Look for index in FilterParam corresponding to wheel current configuration
************************************************************************/
int TTestWindow::FilterIndex(void)
{
int i_posi, k_index;
register int i_wheel, k_filter;
char *pc;

k_index = -1;
/* Loop on all the wheels (just in case filters are not at the good place): */
for(i_wheel = 0; i_wheel < NWHEELS; i_wheel++)
 {
 for(k_filter = 0; k_filter < nfilters; k_filter++)
   {
/* Example: "FA 2", first "FA" should be found in both strings: */
    if(!strncmp(wheel_title[i_wheel],FilterParam[k_filter],2))
      {
/* Then 2 should be the current wheel position: */
      pc =  FilterParam[k_filter] + 3;
      sscanf(pc,"%1d",&i_posi);
      if(i_posi == wheel_position[i_wheel]) { k_index = k_filter; break; }
      }
   }
 }

return(k_index);
}
/*******************************************************************
* Interactively select position file and then call LoadPosition2
*******************************************************************/
void TTestWindow::LoadPosition1()
{
char message1[81], title1[80], s[40];

/* Use dialog box "TFileOpenDialog" */
  FileData.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
  FileData.SetFilter("Position files (*.pos)|*.pos|AllFiles (*.*)|*.*|");
#if LANGUAGE == FRENCH
  strcpy(title1,"Fichier des positions des roues");
#elif LANGUAGE == ITALIAN
  strcpy(title1,"File con le posizioni delle ruote");
#else
  strcpy(title1,"Wheel position file");
#endif
  strcpy(s,"c:\\pisco\\");
  FileData.InitialDir = s;
  if (TFileOpenDialog(GetApplication()->MainWindow, FileData, 0, title1).Execute()
      != IDOK) return;
  else
     strcpy(position_file,FileData.FileName);

#if LANGUAGE == FRENCH
  sprintf(message1,"Voulez-vous télécharger les positions du fichier \"%s\" ?",
                         position_file);
  strcpy(title1,"Téléchargement des positions des roues");
#elif LANGUAGE == ITALIAN
  sprintf(message1,"Volete caricare le posizioni del file \"%s\" ?",
                         position_file);
  strcpy(title1,"Caricamento delle posizioni delle ruote");
#else
  sprintf(message1,"Do you want to load wheel positions from file \"%s\" ?",
          position_file);
  strcpy(title1,"Loading wheel positions");
#endif
  if( ::MessageBox(HWindow, message1, title1, MB_YESNO) != IDNO)
    LoadPosition2(position_file);
}
/*******************************************************************
*
* Called by LoadPosition1 or directly by TTestWindow::InitRS232Link
*******************************************************************/
int TTestWindow::LoadPosition2(char *filename)
{
FILE *fp;
char message1[80], buffer[80], command1[20];
int n_wanted, n_received, status;

/* Open position file: */
if((fp = fopen(filename,"r")) == NULL) {

#if LANGUAGE == FRENCH
  sprintf(message1," Erreur lors de l'ouverture du fichier: %s", filename);
  ::MessageBox(HWindow, message1,"Téléchargement", MB_OK);
#elif LANGUAGE == ITALIAN
  sprintf(message1," Errore aprendo il file: %s", filename);
  ::MessageBox(HWindow, message1,"Caricamento", MB_OK);
#else
  sprintf(message1," Error opening file %s",filename);
  ::MessageBox(HWindow, message1,"Loading wheel positions", MB_OK);
#endif
  return(-1);
}
/***************************************************************
Process lines with a star "*":
Example:

EN 1 0065 *
EN 2 0265
EN 3 0265
***********************************************************/
buffer[10] = '\0';
while(fgets(buffer,20,fp) != NULL) {
  if(buffer[10] == '*') {
    buffer[9] = '\0';
/* Command should be like:   "*CH 1 0065\r"
*/
    sprintf(command1,"*%s\r",buffer);
/* JLP95 debug
    ::MessageBox(HWindow, command1,"Telechargement/debug", MB_OK);
*/
/* Send command to Speckle camera: */
    n_wanted = -1;
    status = RS232w1->RS232_Send_Receive(command1,answer1,error_message1,n_wanted,
                                &n_received);
    if(status) {
#if LANGUAGE == FRENCH
      ::MessageBox(HWindow, error_message1,"Téléchargement/erreur", MB_OK);
#elif LANGUAGE == ITALIAN
      ::MessageBox(HWindow, error_message1,"Caricamento/errore", MB_OK);
#else
      ::MessageBox(HWindow, error_message1,"Position loading/error", MB_OK);
#endif
      fclose(fp); return(-1);
/* Wait for a few seconds before sending new order: */
      } else {
      rs232_delay(delay_msec);
      }
  }  // EOF case buffer[10] == *
  buffer[10] = '\0';
/* End of while loop... */
}
  fclose(fp);
return(0);
}
/***************************************************************
*  Handle exposure toggle button
* Update caption
****************************************************************/
void TTestWindow::HandleButton_Exposure()
{
double yyear, local_time1, h2, end_time, local_UT;
int mmonth, dday;
char title[40], buffer[80], time0[40], command1[20];
register int k;
int ih0, ih1, n_wanted, n_received, start_or_stop_is_ok;

start_or_stop_is_ok = 1;

/************* End current exposure: ********/
if(exposure_on)
  {
#if LANGUAGE == FRENCH
  strcpy(title,"Lancer une nouvelle pose");
#elif LANGUAGE == ITALIAN
  strcpy(title,"Lanciare una nuova posa");
#else
  strcpy(title,"Start a new exposure");
#endif
    jlp_utime(time0,&end_time);
    convert_coord(end_time - start_time,&ih0,&ih1,&h2,"H");
    sprintf(buffer,"%2d H %2d m %2d s",ih0,ih1,(int)h2);
/* Update "exposure time" on the display: */
    ExpoTime1->SetText(" ");

/* Update logbook if logboook has been opened: */
  if(fp_logbook != NULL)
    {
#if ((LANGUAGE == FRENCH) || (LANGUAGE == ITALIAN))
    fprintf(fp_logbook,"---> Fin: %s (T.U.) ",time0);
    fprintf(fp_logbook,"  (Temps de pose: %s)\n",buffer);
#else
    fprintf(fp_logbook,"---> End: %s (U.T.) ",time0);
    fprintf(fp_logbook,"  (Exposure time: %s)\n",buffer);
#endif
/* End of logbook case... */
    }
/* End of case: exposure to be terminated */
  }
/************* Start new exposure and initialize start_time: ********/
else
  {
/****************/
/* If logboook has been opened, ask for comments and test if OK: */
/* In case of logbook, wait for the user comments: */
    if(fp_logbook != NULL)
       if(!InputComments()) start_or_stop_is_ok =0;

/* Start exposure if OK: */
    if(start_or_stop_is_ok)
    {
    jlp_utime(time0,&start_time);
    idle_index = 0;
/****************/
/* Update logbook if logboook has been opened: */
    if(fp_logbook != NULL)
     {
     jlp_utime(time0,&start_time);

/* Write to logbook: */
#if ((LANGUAGE == FRENCH) || (LANGUAGE == ITALIAN))
     fprintf(fp_logbook,
             "******************* NOUVELLE POSE *********************\n");
     fprintf(fp_logbook,"Objet = %s    ",StarName);
#else
     fprintf(fp_logbook,
             "******************* NEW EXPOSURE *********************\n");
     fprintf(fp_logbook,"Object = %s\n",StarName);
#endif
/* Write comments on the next line: (after January 2006: longer comments,
*  but without "Comments: " at the beginning of the line) */
     fprintf(fp_logbook,"%s\n",Comments);
     if(astro_target)
       {
/* Alpha, Delta: */
       convert_coord(alpha_value,&ih0,&ih1,&h2,"H");
       fprintf(fp_logbook," Alpha = %2d H %2d m %.1f s ",ih0,ih1,h2);
       convert_coord(delta_value,&ih0,&ih1,&h2,"D");
/* Case of -0 30 54: */
       if(ih0 == 0 && delta_value < 0.)
         fprintf(fp_logbook," Delta =  -0 D %2d' %2d\" ",ih1,(int)h2);
       else
         fprintf(fp_logbook," Delta = %3d D %2d' %2d\" ",ih0,ih1,(int)h2);
       fprintf(fp_logbook," (Equin=%.2f) \n",equin_value);
       }
       jlp_local_time(buffer);
       decode_time(buffer,&yyear,&mmonth,&dday,&local_time1);
/* Handle special cases: UT day starts after Local day*/
       if(start_time < 0.) start_time += 24.;
       if(local_time1 < 0.) local_time1 += 24.;
       local_UT = local_time1 - start_time;
       if(local_UT < 0.) dday--;

#if ((LANGUAGE == FRENCH) || (LANGUAGE == ITALIAN))
       fprintf(fp_logbook,"---> Debut: %s (T.U.) le %02d/%02d/%4d.\n",
               time0,dday,mmonth,(int)yyear);
       fprintf(fp_logbook," Roues: ");
#else
       fprintf(fp_logbook,"---> Start: %s (U.T.) on %02d/%02d/%4d.\n",
               time0,dday,mmonth,(int)yyear);
       fprintf(fp_logbook," Wheels: ");
#endif
/* Wheel positions: */
     for (k = 0; k < NWHEELS; k++)
       {
        fprintf(fp_logbook,"  %.2s %1d",wheel_title[k],wheel_position[k]);
       }
     fprintf(fp_logbook,"\n");

/* Filter index: */
     k = FilterIndex();

/* Hour angle: */
     if(astro_target)
       {
       convert_coord(hour_angle,&ih0,&ih1,&h2,"H");

#if ((LANGUAGE == FRENCH) || (LANGUAGE == ITALIAN))
       fprintf(fp_logbook," Ang. horaire = %2d H %2d m %2d s ",ih0,ih1,(int)h2);
       fprintf(fp_logbook," Hauteur = %.1f (deg) ",elevation);
       fprintf(fp_logbook," M. d'air = %.2f \n",air_mass);
#else
       fprintf(fp_logbook," Hour angle: %2d H %2d m %2d s ",ih0,ih1,(int)h2);
       fprintf(fp_logbook," Elevation: %.1f (deg)",elevation);
       fprintf(fp_logbook," Airmass: %.2f \n",air_mass);
#endif
/* End of astro_target (in logbook case) */
       }
#if ((LANGUAGE == FRENCH) || (LANGUAGE == ITALIAN))
     fprintf(fp_logbook," Filtre: %.15s     ",&FilterParam[k][5]);
#else
     fprintf(fp_logbook," Filter: %.15s     ",&FilterParam[k][5]);
#endif
/* End of logbook case... */
     }
/****************/
/* In case of non astronomical target and automatic correction
  set Risley Prisms to null dispersion position: */
  if(!astro_target)
    {
/* RA prism: */
       ra_computed = RA_NULLDISP;
/* Copy to Command Status: */
       sprintf(command1,"RA %04d",ra_computed);
       CommandStatus->SetText(command1);
/* Send command: */
       sprintf(command1,"RA %04d\r",ra_computed);
       n_wanted = -1;
       RS232w1->RS232_Send_Receive(command1,answer1,error_message1,n_wanted,&n_received);
/* Wait for a few seconds before sending new order: */
       rs232_delay(delay_msec);

/* RB prism: */
       rb_computed = RB_NULLDISP;
/* Copy to Command Status: */
       sprintf(command1,"RB %04d",rb_computed);
       CommandStatus->SetText(command1);
/* Send command: */
       sprintf(command1,"RB %04d\r",rb_computed);
       n_wanted = -1;
       RS232w1->RS232_Send_Receive(command1,answer1,error_message1,n_wanted,&n_received);
    }

/* JLP95: here ra and rb computed, later (when RS232 OK): ra and rb position. */
  if(fp_logbook != NULL)
     fprintf(fp_logbook,"  RA: %04d  RB: %04d\n",ra_computed, rb_computed);

#if LANGUAGE == FRENCH
    strcpy(title,"Arrêter la pose en cours");
#elif LANGUAGE == ITALIAN
    strcpy(title,"Fermare la posa in corso");
#else
    strcpy(title,"Stop current exposure");
#endif

/* End of case: start_or_stop_is_ok */
  }
/* End of case: exposure to be started */
  }

/* Toggle to new value: 1 -> 0    or  0 -> 1 */
 if(start_or_stop_is_ok)
  {
   exposure_on = 1 - exposure_on;
   ExposureButton->SetCaption(title);
  }

return;
}
/********************************************************************
* OpenLogBook
*
*********************************************************************/
void TTestWindow::OpenLogBook()
{

char error_message[80], title1[40], s[40];

  if(fp_logbook != NULL)
        {
#if LANGUAGE == FRENCH
           sprintf(error_message,"Erreur, fichier déjà ouvert (%s)",
                  logbook_name);
          ::MessageBox(HWindow, error_message,
                                         "Ouverture carnet de bord", MB_OK);
#elif LANGUAGE == ITALIAN
           sprintf(error_message,"Errore, il file è già aperto (%s)",
                  logbook_name);
          ::MessageBox(HWindow, error_message,
                                         "Apertura del giornale di bordo", MB_OK);
#else
          sprintf(error_message,"Error, logfile already opened (%s)",
                  logbook_name);
          ::MessageBox(HWindow, error_message,
                                         "Open Logbook", MB_OK);
#endif
        return;
        }

#if LANGUAGE == FRENCH
  strcpy(title1,"Ouverture du carnet de bord");
#elif LANGUAGE == ITALIAN
  strcpy(title1,"Apertura del giornale di bordo");
#else
  strcpy(title1,"Opening the logbook");
#endif
/* Use dialog box "TFileOpenDialog" */
  FileData.Flags = OFN_OVERWRITEPROMPT;
  FileData.SetFilter("Logbook files (*.log)|*.log|AllFiles (*.*)|*.*|");
  strcpy(s,"c:\\logfiles\\");
  FileData.InitialDir = s;
  if (TFileOpenDialog(GetApplication()->MainWindow, FileData, 0, title1).Execute()
      == IDOK)
    {
     strcpy(logbook_name,FileData.FileName);
     if((fp_logbook = fopen(logbook_name,"a+")) == NULL)
         {
#if LANGUAGE == FRENCH
          sprintf(error_message," Erreur lors de l'ouverture du fichier: %s",
                                 logbook_name);
          ::MessageBox(HWindow, error_message,"Carnet de bord", MB_OK);
#elif LANGUAGE == ITALIAN
          sprintf(error_message," Errore aprendo il file: %s",
                                 logbook_name);
          ::MessageBox(HWindow, error_message,"Giornale di bordo", MB_OK);
#else
          sprintf(error_message," Error opening logfile %s",logbook_name);
          ::MessageBox(HWindow, error_message,"Open Logbook", MB_OK);
#endif
         }
      else
         {
// Write atmospheric parameters:
         fprintf(fp_logbook,"*********************************************\n");
         fprintf(fp_logbook,
                 " Temperature: %.1f, pression: %.1f, hygrometrie: %.1f\n",
                 temp_value, press_value, hygro_value);
         }
    }

return;
}
/********************************************************************
* CloseLogBook
*
*********************************************************************/
void TTestWindow::CloseLogBook()
{

 if(fp_logbook == NULL)
        {
#if LANGUAGE == FRENCH
          ::MessageBox(HWindow, "Erreur, aucun fichier n'a été ouvert",
                                         "Fermeture du carnet de bord", MB_OK);
#elif LANGUAGE == ITALIAN
          ::MessageBox(HWindow, "Errore, nessun file già aperto",
                                         "Chiusura del giornale di bordo", MB_OK);
#else
          ::MessageBox(HWindow, "Error, logfile has not been opened yet",
                                         "Close Logbook", MB_OK);
#endif
        }
 else
        {
        fclose(fp_logbook);
        fp_logbook = NULL;
        }

return;
}
/********************************************************************
* CommentsLogBook
* Add comments to logbook
*********************************************************************/
void TTestWindow::CommentsLogBook()
{

char title1[80], prompt1[40];

/* First check that logbook has been opened */
  if(fp_logbook == NULL)
        {
#if LANGUAGE == FRENCH
          ::MessageBox(HWindow, "Erreur, aucun fichier n'a été ouvert",
                       "Ajout de commentaires (carnet de bord)", MB_OK);
#elif LANGUAGE == ITALIAN
          ::MessageBox(HWindow, "Errore, nessun file già aperto",
                       "Aggiunta dei commenti (giornale di bordo)", MB_OK);
#else
          ::MessageBox(HWindow, "Error, logfile has not been opened yet",
                       "Add comments to logbook", MB_OK);
#endif
        }
/* Then add a comment: */
        else
        {
#if LANGUAGE == FRENCH
         sprintf(title1,"Carnet de bord: %s",logbook_name);
         strcpy(prompt1,"Commentaires:");
#elif LANGUAGE == ITALIAN
         sprintf(title1,"Giornale di bordo: %s",logbook_name);
         strcpy(prompt1,"Commenti:");
#else
         sprintf(title1,"Logbook: %s",logbook_name);
         strcpy(prompt1,"Comments:");
#endif
         *Comments = '\0';
         if ( GetApplication()->ExecDialog(new TInputDialog(this, title1,
                prompt1, Comments, sizeof(Comments)-1)) != IDOK ) *Comments = '\0';

/* Write comments if line not empty: */
         if(*Comments) fprintf(fp_logbook," %s %s\n",prompt1,Comments);

        }

return;
}
/********************************************************************
* BonnetteInput
*********************************************************************/
#if 0
void TTestWindow::BonnetteInput()
{

 char InputText[6], title1[80], prompt1[40], error_message[80];
 int code, bonnette_code;

  sprintf(InputText, "%d", bonnette_code);
#if LANGUAGE == FRENCH
  strcpy(title1,"Code bonnette (en cas de rotation de PISCO):");
  strcpy(prompt1,"Entrez la nouvelle valeur:");
#elif LANGUAGE == ITALIAN
  strcpy(title1,"Code della bonnette (in caso di rotazione di PISCO):");
  strcpy(prompt1,"Entrate la nuova valore:");
#else
  strcpy(title1,"Bonnette code (in the case when PISCO can rotate):");
  strcpy(prompt1,"Input new value:");
#endif

  if ( GetApplication()->ExecDialog(new TInputDialog(this, title1,
    prompt1, InputText, sizeof InputText)) == IDOK )
  {
      sscanf(InputText,"%d",&code);
      if ( code > -180 && code < 360)
         {
/* Refresh whole screen: */
         bonnette_code = code;
         RefreshScreen(error_message,1);
         }
  }
return;
}
#endif
/********************************************************************
* Risley prism setup
*********************************************************************/
void TTestWindow::ADC_Setup()
{

 char InputText[6], title1[80], prompt1[40];
 int i1, i2, isign;

  strcpy(title1,"ADC setup");
  sprintf(InputText, "%d %d %d", ra_offset, rb_offset, ra_rb_sign);
  strcpy(prompt1,"ADC: ra_offset, rb_offset, ra_rb_sign:");

  if ( GetApplication()->ExecDialog(new TInputDialog(this, title1,
    prompt1, InputText, sizeof InputText)) == IDOK )
  {
      sscanf(InputText,"%d %d %d", &i1, &i2, &isign);
      if (i1 >= 0 && i2 < 1025 && i2 >= 0 && i2 < 1025
          && (isign == -1 || isign == 1))
         {
          ra_offset = i1;
          rb_offset = i2;
          ra_rb_sign = isign;
         }
  }
return;
}
/********************************************************************
* Display telescope position
*********************************************************************/
void TTestWindow::TelescopePosition()
{
 double yyear, local_time1;
 int mmonth, dday, ih0, ih1;
 double ww_alpha, ww_delta, Delta_years, h2;
 char t_alpha[80], t_delta[80], t_hangle[40], t_height[40];
 char t_airmass[40], buffer[80*6];

// Coordinates:
   if(astro_target)
   {
// Transfer to "corrected values":
     ww_alpha = alpha_value;
     ww_delta = delta_value;
// Compute correction if equinox is not zero:
     if(equin_value != 0.) {
        jlp_local_time(buffer);
        decode_time(buffer,&yyear,&mmonth,&dday,&local_time1);
        Delta_years = yyear + ((double)mmonth)/12. - equin_value;
        precess(&ww_alpha, &ww_delta, Delta_years);
        }
     convert_coord(ww_alpha,&ih0,&ih1,&h2,"H");
#if LANGUAGE == FRENCH
     sprintf(t_alpha,"Alpha %d H %d m %.1f s (corrigé de la précession)\n",ih0,ih1,h2);
#elif LANGUAGE == ITALIAN
     sprintf(t_alpha,"Alpha %d H %d m %.1f s (corretto dalla precessione)\n",ih0,ih1,h2);
#else
     sprintf(t_alpha,"Alpha %d H %d m %.1f s (corrected for precession)\n",ih0,ih1,h2);
#endif
     convert_coord(ww_delta,&ih0,&ih1,&h2,"D");
#if LANGUAGE == FRENCH
/* Case of -0 30 54: */
     if(ih0 == 0 && delta_value < 0.)
       sprintf(t_delta,"Delta -0° %d' %.1f\" (corrigé de la précession)\n",ih1,h2);
     else
       sprintf(t_delta,"Delta %d° %d' %.1f\" (corrigé de la précession)\n",ih0,ih1,h2);
#elif LANGUAGE == ITALIAN
     if(ih0 == 0 && delta_value < 0.)
       sprintf(t_delta,"Delta -0° %d' %.1f\" (corretto dalla precessione)\n",ih1,h2);
     else
       sprintf(t_delta,"Delta %d° %d' %.1f\" (corretto dalla precessione)\n",ih0,ih1,h2);
#else
     if(ih0 == 0 && delta_value < 0.)
       sprintf(t_delta,"Delta -0° %d' %.1f\" (corrected for precession)\n",ih1,h2);
     else
       sprintf(t_delta,"Delta %d° %d' %.1f\" (corrected for precession)\n",ih0,ih1,h2);
#endif

/* Hour angle: */
     convert_coord((double)hour_angle,&ih0,&ih1,&h2,"H");
#if LANGUAGE == FRENCH
     sprintf(t_hangle," Ang. horaire = %2d H %2d m %2d s",ih0,ih1,(int)h2);
     sprintf(t_height," Hauteur = %.1f (deg) ", elevation);
     sprintf(t_airmass," M. d'air = %.2f ",air_mass);
#elif LANGUAGE == ITALIAN
     sprintf(t_hangle," Ang. orario = %2d H %2d m %2d s",ih0,ih1,(int)h2);
     sprintf(t_height," Altezza = %.1f (gradi) ", elevation);
     sprintf(t_airmass," M. d'aria = %.2f ",air_mass);
#else
     sprintf(t_hangle," Hour angle: %2d H %2d m %2d s ",ih0,ih1,(int)h2);
     sprintf(t_height," Elevation: %.1f (deg)",elevation);
     sprintf(t_airmass," Airmass: %.2f ",air_mass);
#endif
     sprintf(buffer,"%s \n %s \n %s \n %s \n %s \n %s \n",
             t_alpha,t_delta,t_hangle,t_height,t_airmass);
/* End of astro_target */
   }
   else
   {
#if LANGUAGE == FRENCH
     sprintf(buffer,"Mode calibration: la position du télescope est inconnue!");
#elif LANGUAGE == ITALIAN
     sprintf(buffer,"Modo di calibrazione: la posizione del telescopio è sconociuta!");
#else
     sprintf(buffer,"Calibration mode: telescope position is unknown!");
#endif
   }

#if LANGUAGE == FRENCH
::MessageBox(HWindow, buffer,"Position actuelle du télescope", MB_OK);
#elif LANGUAGE == ITALIAN
::MessageBox(HWindow, buffer,"Posizione attuale del telescopio", MB_OK);
#else
::MessageBox(HWindow, buffer,"Current position of the telescope", MB_OK);
#endif
return;
}
/********************************************************************
* VersionDisplay
*********************************************************************/
void TTestWindow::VersionDisplay()
{
   strcpy(error_message1,
"Versione 25-09-2006 (per PISCO in Merate)\n \n \
Risley offset (from January 1997):\n \
\n \
Null dispersion:\n \
    RA_NULLDISP=57\n \
    RB_NULLDISP=214\n \
RA and RB aligned to the North with red to North:\n \
    OFFSET_RA=511\n \
    OFFSET_RB=156\n \
Increasing Risley code rotates the dispersion\
 from North to West\n \
Both Risley prisms have the same orientation\n \
\n");
#if LANGUAGE == FRENCH
   ::MessageBox(HWindow, error_message1,
                 "Programme de commande de PISCO (version de Calern)",
                  MB_OK);
#elif LANGUAGE == ITALIAN
   ::MessageBox(HWindow, error_message1,
                 "Programma di controllo di PISCO (versione di Calern)",
                  MB_OK);
#else
   ::MessageBox(HWindow, error_message1,
                  "Control program of the OMP speckle camera (Calern version)",
                  MB_OK);
#endif
}
/********************************************************************
* DelayInput
*********************************************************************/
void TTestWindow::DelayInput()
{

 char InputText[6], title1[80], prompt1[80];
 long int value;

  sprintf(InputText, "%d", delay_msec);
#if LANGUAGE == FRENCH
  strcpy(title1,"Retard en millisecondes (liaison RS232) ");
  strcpy(prompt1,"Entrez la nouvelle valeur (entre 1 et 5000) : ");
#elif LANGUAGE == ITALIAN
  strcpy(title1,"Ritardo in millisecondi (per il collegamento RS232) ");
  strcpy(prompt1,"Entrate la nuova valore (tra 1 e 5000) : ");
#else
  strcpy(title1,"Delay time (milliseconds), for the RS232 link");
  strcpy(prompt1,"Input new value (between 1 and 5000): ");
#endif

  if ( GetApplication()->ExecDialog(new TInputDialog(this, title1,
    prompt1, InputText, sizeof InputText)) == IDOK )
  {
      sscanf(InputText,"%ld",&value);
      if ( value > 0 && value <= 5000)
         delay_msec = value;
  }
}
/********************************************************************
* CalibInput
*********************************************************************/
void TTestWindow::CalibInput()
{

 char InputText[MAXSTARNAMELEN], title1[40], prompt1[40], error_message[80];

  strcpy(InputText, StarName);
#if LANGUAGE == FRENCH
  strcpy(title1,"Nouvel objet:");
  strcpy(prompt1,"Désignation:");
#elif LANGUAGE == ITALIAN
  strcpy(title1,"Nuovo oggetto:");
  strcpy(prompt1,"Designazione:");
#else
  strcpy(title1,"New object:");
  strcpy(prompt1,"Designation:");
#endif

   if ( GetApplication()->ExecDialog(new TInputDialog(this, title1,
    prompt1, InputText, sizeof InputText)) == IDOK )
  {
      strcpy(StarName,InputText);
      astro_target = 0;
/* Refresh screen, with new values of target name and air mass, etc. */
/* JLP95 some obscure problems made the program aborted here (in version 3.1)
   Now it seems to be fixed...
*/
    RefreshScreen(error_message, 1);
  }
}

