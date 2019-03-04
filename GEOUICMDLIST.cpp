// GEOUICMDLIST.cpp: implementation of the C_GEOUICMDLIST class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "GEOUICMDLIST.h"
#include "gs_ui_utily.h"

#include <rxmfcapi.h>         // ACAD MFC stuff

#include "gs_ui_user.h"
#include "gs_ui_option.h"
#include "gs_ui_WrkSession.h"
#include "gs_ui_organizer.h"
#include "gs_ui_attribvalueslistdlg.h"
#include "gs_ui_LyrDisplayModel.h"
#include "gs_ui_grid.h"
#include "gs_ui_sql.h"
#include "gs_ui_Topology.h"
#include "gs_ui_dblink.h"
#include "gs_ui_sec.h"
#include "gs_ui_postgresql.h"
#include "gs_ui_class_set_dlg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/* Costruttore */
C_GEOUICMDLIST::C_GEOUICMDLIST()
{
   const int ItalianName = 0;
   const int BritishName = 1;
   const int SpanishName = 2;
   const int FrenchName  = 3;
   const int GermanName  = 4;
   int      lan = 0, i = 0, index = 0, offset = 0;
   C_STR   *punt;

   typedef struct 
   {
      TCHAR *name;
      int   (*fptr)();
      int   flag;          // controllo 1° Bit operativita su spazio carta no-si
                           // controllo 2° Bit comando disponibile con o senza login
   } ftblent;

   typedef struct 
   {
      TCHAR *MultiName[2];
      int   (*fptr)();
      int   flag;          // controllo 1° Bit operativita su spazio carta no-si
                           // controllo 2° Bit comando disponibile con o senza login
   } MultiLanFunTbl;

   // Il seguente vettore contiene i nomi dei comandi GEOUI nelle varie lingue supportate
   MultiLanFunTbl VectorGEOUICmds[] =
   //   Italiano                    Inglese                    Nome interno     Flag di funzionamento
   //                                                                           1° Bit = 1 operativita su spazio carta
   //                                                                           se 2° Bit = 1 comando disponibile senza login
   {  
      {{_T("C:gsui_prova"),         _T("C:gsui_prova")},       gsui_prova,       1},
      // opzioni GEOsim
      {{_T("C:gsuioptions"),        _T("C:gsuioptions")},      gsui_options,     2},
      {{_T("C:gsuiDisplayDWGExt"),  _T("C:gsuiDisplayDWGExt")}, gsui_DisplayDWGExt, 0},
      {{_T("C:gsuiDelDataModelComponent"), _T("C:gsuiDelDataModelComponent")}, gsui_DelDataModelComponent, 0},
      // PostgreSQL - QGIS - History
      {{_T("C:gsuipgview"),         _T("C:gsuipgview")},       gsui_pgview,      2},
      {{_T("C:gsuiqgis"),           _T("C:gsuiqgis")},         gsui_qgis,        2},
      {{_T("C:gsuicreatehistorysystem"), _T("C:gsuicreatehistorysystem")}, gsui_CreateHistorySystem, 0},
      // Strumenti di editing
      {{_T("C:gsuiinsdata"),        _T("C:gsuiinsdata")},      gsuiinsdata,      0},
      {{_T("C:gsuidbdata"),         _T("C:gsuidbdata")},       gsuidbdata,       0},
      {{_T("C:gsuidbgriddata"),     _T("C:gsuidbgriddata")},   gsuidbGriddata,   0},
      {{_T("C:gsuiInsDynSegmentationData"), _T("C:gsuiInsDynSegmentationData")}, gsuiInsDynSegmentationData, 0},
      {{_T("C:gsuiCalibrateDynSegmentationData"), _T("C:gsuiCalibrateDynSegmentationData")}, gsuiCalibrateDynSegmentationData, 0},
      // utenti
      {{_T("C:gsuilogin"),          _T("C:gsuilogin")},        gsui_login,       3},
      {{_T("C:gsui_createusr"),     _T("C:gsui_createusr")},   gsui_createusr,   2},
      {{_T("C:gsui_modipwdcurrusr"), _T("C:gsui_modipwdcurrusr")}, gsui_modipwdcurrusr, 2},
      {{_T("C:gsui_delusr"),        _T("C:gsui_delusr")},      gsui_delusr,      2},
      {{_T("C:gsui_modusr"),        _T("C:gsui_modusr")},      gsui_modusr,      2},
      // lista di scelta per attributi
      {{_T("C:gsuiAttribValuesList"), _T("C:gsuiAttribValuesList")}, gsui_AttribValuesList, 3},
      // Comandi per gestire il modello dati
      {{_T("C:gsuiClassSet"),      _T("C:gsuiClassSet")},      gsui_ClassSet,    2},
      // Comandi per gestire le griglie
      {{_T("C:gsuiGridSetFlowDirection"), _T("C:gsuiGridSetFlowDirection")}, gsui_GridSetFlowDirection, 3},
      {{_T("C:gsuiGridDrape"),     _T("C:gsuiGridDrape")},     gsui_GridDrape,   3},
      {{_T("C:gsuiGridSpatialInterpolationIDW"), _T("C:gsuiGridSpatialInterpolationIDW")}, gsui_GridSpatialInterpolationIDW, 3},
      {{_T("C:gsuiGridHydrologyCountUpstreamCells"), _T("C:gsuiGridHydrologyCountUpstreamCells")}, gsui_GridHydrologyCountUpstreamCells, 3},
      {{_T("C:gsuiGridGetCatchmentAreaCells"), _T("C:gsuiGridGetCatchmentAreaCells")}, gsui_GridGetCatchmentAreaCells, 3},
      {{_T("C:gsuiGridDisplayValleyOrRidge"), _T("C:gsuiGridDisplayValleyOrRidge")}, gsui_GridDisplayValleyOrRidge, 3},
      {{_T("C:gsuiGridUpdZFromGraph"), _T("C:gsuiGridUpdZFromGraph")}, gsui_GridUpdZFromGraph, 3},
      {{_T("C:gsuiGridUpdFromDB"), _T("C:gsuiGridUpdFromDB")}, gsui_GridUpdFromDB, 3},
      {{_T("C:gsuiGridDisplayContours"), _T("C:gsuiGridDisplayContours")}, gsui_GridDisplayContours, 3},
      // ricerche topologiche
      {{_T("C:gsuiTopoPropagation"), _T("C:gsuiTopoPropagation")}, gsui_topo_propagation, 3},
      {{_T("C:gsuiTopoShortestPath"), _T("C:gsuiTopoShortestPath")}, gsui_topo_shortestpath, 3},
      // gestione dei modelli di visualizzazione dei layer
      {{_T("C:gsuilyrdispmodel"),  _T("C:gsuilyrdispmodel")},  gsui_lyrdispmodel, 3}
   };

   // Il seguente vettore contiene i nomi dei comandi di Acad ridefiniti da Geosim
   MultiLanFunTbl VectorAcadCmds[] =
   {
   //   Italiano                 Inglese            Nome interno  Flag di funzionamento
   //                                                             1° Bit = operativita su spazio carta no-si
   //                                                             2° Bit = comando disponibile con (1) o senza login (0)
      {{_T("C:comando1"),          _T("C:command1")        },   comandoInterno1,     1}//,	
   //   {{"C:comando2",          "C:comando2"   	   },   comando2,      1},
   };


   // Il seguente vettore contiene i nomi delle funzioni LISP
   ftblent VectorLispFunction[] =
   {
   //  Inglese                         Nome interno               Flag di funzionamento
   //                                                             1° Bit = operativita su spazio carta no-si
   //                                                             2° Bit = comando disponibile con (1) o senza login (0)
      // Funzioni di utilità
      {_T("gslui_alert"),                   gslui_alert,              3},
      {_T("gslui_info"),                    gslui_info,               3},
      {_T("gslui_confirm"),                 gslui_confirm,            3},
      {_T("gs_register_all_controls"),      gs_register_all_controls, 3},
      // scelta di classi di entità
      {_T("gsui_SelClass"),                 gsui_SelClass,            3},
      // Pannello sessione di lavoro
      {_T("gsuiWrkSessionPanel"),           gsui_WrkSessionPanel,     3},
      // Connessioni OLE-DB
      {_T("gsui_DBConn"),                   gsui_DBConn,              3},
      {_T("gsui_ExistingTabConn"),          gsui_ExistingTabConn,     3},
      {_T("gsui_NewTabConn"),               gsui_NewTabConn,          3},
      {_T("gsui_NewPrefixTabConn"),         gsui_NewPrefixTabConn,    3},
      {_T("gsui_TabContainer"),             gsui_TabContainer,        3},
      // Gestione classi con tabelle dati esterne
      {_T("gsui_getAlfaNumDBLink"),         gsui_getAlfaNumDBLink,    3},
      {_T("gsui_getGeomDBLink"),            gsui_getGeomDBLink,       3},
      // Segmentazione dinamica
      {_T("gsui_InitDynSegmentation"),      gsui_InitDynSegmentation, 3},
      // Griglie
      {_T("gsui_dbgriddataOnKeyList"),      gsuidbGriddataOnKeyList,  3},
      // SRID
      {_T("gsui_getSRID"),                  gsui_getSRID,             3}
   };


   // Inizializzo exfun
   exfun    = NULL;
   LenExfun = 0;

   // costruisco il vettore comando-funzione
   if (gsc_GetAcadLanguage(&lan) == GS_BAD) return;

   switch (lan)
   {
      case LAN_ITALIAN:
         index = 0;
         break;
      case LAN_BRITISH:
         index = 1;
         break;
      case LAN_SPANISH:
         index = 2;
         break;   
      case LAN_FRENCH:
         index = 3;
         break;
      case LAN_GERMAN:
         index = 4;
         break;
      default: 
         return;
   }
   
   LenExfun = ELEMENTS(VectorGEOUICmds)+ELEMENTS(VectorAcadCmds)+ELEMENTS(VectorLispFunction);
   
   // alloco exfun
   if ((exfun = (FunTable *) calloc(LenExfun, sizeof(FunTable))) == NULL)
      return; 

   // caricamento vettore comandi GEOsim proprietari
   for (i = 0; i < ELEMENTS(VectorGEOUICmds); i++)
   {
      exfun[i].name = VectorGEOUICmds[i].MultiName[index];
      exfun[i].fptr = VectorGEOUICmds[i].fptr;
      exfun[i].flag = VectorGEOUICmds[i].flag;
   }
   offset = i;
   // caricamento vettore comandi AutoCAD rifatti
   for (i = 0; i < ELEMENTS(VectorAcadCmds); i++)
   {
      exfun[i + offset].name = VectorAcadCmds[i].MultiName[index];
      exfun[i + offset].fptr = VectorAcadCmds[i].fptr;
      exfun[i + offset].flag = VectorAcadCmds[i].flag;
   }
   offset += i;
   // caricamento vettore comandi AutoCAD rifatti
   for (i = 0; i < ELEMENTS(VectorLispFunction); i++)
   {
      exfun[i + offset].name = VectorLispFunction[i].name;
      exfun[i + offset].fptr = VectorLispFunction[i].fptr;
      exfun[i + offset].flag = VectorLispFunction[i].flag;
   }
   
   // alloco la lista dei nomi dei comandi AutoCAD rifatti
   for (i = 0; i < ELEMENTS(VectorAcadCmds); i++)
   {
      if ((punt = new(C_STR)) == NULL) return;
      if ((punt->set_name(VectorAcadCmds[i].MultiName[index])) == GS_BAD) return; 
      AcadCmds.add_tail(punt);
   }
}

// Distruttore
C_GEOUICMDLIST::~C_GEOUICMDLIST()
   { free(exfun); }


/*************************************************************************/
/*.doc C_GEOUICMDLIST::funcLoad()                                             */
/*
    This function is called to define all function names in the ADS
    function table.  Each named function will be callable from lisp or
    invokable from another ADS application.
*/
/*************************************************************************/
int C_GEOUICMDLIST::funcLoad()
{
   int i;

   for (i = 0; i < LenExfun; i++) 
      if (!ads_defun(exfun[i].name.get_name(), i)) return RTERROR;
    
   return RTNORM;
}


/*************************************************************************/
/*.doc C_GEOUICMDLIST::funcUnload()                                      */
/*  
    This function is called to undefine all function names in the ADS
    function table.  Each named function will be removed from the
    AutoLISP hash table.
*/
/*************************************************************************/
int C_GEOUICMDLIST::funcUnload()
{
   int i;

   // Undefine each function we defined
   for (i = 0; i < LenExfun; i++) 
      ads_undef(exfun[i].name.get_name(), i);
    
   return RTNORM;
}


/*************************************************************************/
/*.doc C_GEOUICMDLIST::get_ptr_fun(int val)                                   */
/*  
    Questa funzione resrtituisce il puntatore alla funzione val-esima.
    Parametri: 
    int val;   Posizione nel vettore delle funzioni.
*/
/*************************************************************************/
FunTable* C_GEOUICMDLIST::get_ptr_fun(int val)
{
   int    rc = GS_BAD;
   resbuf rb;

   ads_retnil();

   if (val >= LenExfun) return NULL;

   // se ci si trova nello spazio carta
   if (ads_getvar(_T("CVPORT"), &rb) != RTNORM || rb.restype != RTSHORT)
      { set_GS_ERR_COD(eGSVarNotDef); return NULL; }
   if (rb.resval.rint == 1)
      if (!(exfun[val].flag & 1))         // comando non abilitato allo spazio carta.
         { set_GS_ERR_COD(eGSPaperSpaceFound); return NULL; }
                         
   if (gsc_whoami(NULL) == GS_BAD)
      if (!(exfun[val].flag & 2))         // comando non abilitato senza login.
         return NULL;
	
   return &(exfun[val]);
}


/*************************************************************************/
/*.doc C_GEOUICMDLIST::dofun                                             */
/*
    This function is called to invoke the function which has the
    registerd function code that is obtained from  ads_getfuncode.  The
    function will return RTERROR if the function code is invalid, or
    RSERR if the invoked function fails to return RTNORM.  The value
    RSRSLT will be returned if the function code is valid and the
    invoked subroutine returns RTNORM.
*/
/*************************************************************************/
int C_GEOUICMDLIST::dofun()
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   int      val, rc = GS_BAD, old_reactor_abilit;
   FunTable *pfun;

   ads_retnil();

   if ((val = ads_getfuncode()) < 0) return RTERROR;

   do
   {
      
      if ((pfun = get_ptr_fun(val)) == NULL) break;

      // disattivo il controllo sul reattore
      old_reactor_abilit = gsc_disable_reactors();

		set_GS_ERR_COD(eGSNoError);

      rc = (*(pfun->fptr))();          // richiamo la funzione

      // se non è una chiamata a funzione per attivare/disattivare i reattori
      if (!(pfun->name.comp(_T("gs_disable_reactors")) == 0 ||
            pfun->name.comp(_T("gs_enable_reactors")) == 0))
         // ripristino il controllo sul reattore come in precedenza
         if (old_reactor_abilit == GS_GOOD) gsc_enable_reactors();
   }
   while (0);

   if (rc == RTERROR || rc == GS_BAD)
      gsc_print_error();
   else
      if (rc == RTCAN || rc == GS_CAN)
         acutPrintf(gsc_msg(220)); // "\n*Annulla*\n"

   return ((rc == RTNORM) ? RSRSLT : RSERR);
}
