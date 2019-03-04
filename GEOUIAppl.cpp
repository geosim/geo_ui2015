// GEOUIAppl.cpp: implementation of the GEOUIAppl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>      /*  per wcscat() wcscmp()  */

#include "resource.h"

#include <rxmfcapi.h>         // ACAD MFC stuff

#include "gs_list.h"
#include "gs_def.h"       // definizioni globali
#include "gs_error.h"     // codici errori
#include "gs_resbf.h"     // gestione resbuf
#include "gs_init.h"      // direttori globali
#include "gs_utily.h" 

#include "GEOUIAppl.h"
#include "gs_ui_utily.h"
#include "gs_ui_WrkSession.h"

#include "resource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GEOUIAppl::GEOUIAppl()
{

}

GEOUIAppl::~GEOUIAppl()
{
   terminate();
}

/*********************************************************/
/*.doc GEOUIAppl::init                       <external> */
/*+
  Questa funzione inizializza GEOUI.
                                                       
  Restituisce GS_GOOD in caso di successo altrimenti restituisce GS_BAD.
-*/  
/*********************************************************/
int GEOUIAppl::init(void)
{
   acutPrintf(_T("\nCaricamento interfaccia grafica GEOsim."));
   if (gsc_register_all_controls() != GS_GOOD)
      acutPrintf(_T("Registrazione controlli fallita.\n"));

   // aggiungo reattore contestuale
   cmdAddInputContextReactor();

   return GS_GOOD;     
}


/*********************************************************/
/*.doc GEOUIAppl::terminate                  <external> */
/*+           
  Questa funzione termina GEOUI.
  La procedura:
                                                       
  Restituisce GS_GOOD in caso di successo altrimenti restituisce GS_BAD.
-*/  
/*********************************************************/
int GEOUIAppl::terminate(void)
{                 
   if (pDockPaneWrkSession) 
      { delete pDockPaneWrkSession; pDockPaneWrkSession = NULL; }

   // rimuovo reattore contestuale
   cmdRemoveInputContextReactor();

   return GS_GOOD; 
}

//inizializzazione membri statici
C_GEOUICMDLIST GEOUIAppl::GEOUICMD_LIST;
    
