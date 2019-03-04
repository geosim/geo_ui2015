// gs_ui_organizer.cpp : implementation file
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"

#include "gs_error.h" 
#include "gs_utily.h"
#include "gs_resbf.h"
#include "gs_init.h"
#include "gs_class.h"
#include "gs_sec.h"
#include "gs_filtr.h"
#include "GEOListCtrl.h"

#include "gs_ui_utily.h"
#include "gs_ui_WrkSession.h"
#include "gs_ui_attribvalueslistdlg.h"
#include "gs_ui_organizer.h"

#include "d2hMap.h" // doc to help


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CDataModelDlg

IMPLEMENT_DYNCREATE(CDataModelDlg, CFrameWnd)

CDataModelDlg::CDataModelDlg()
{
}

CDataModelDlg::~CDataModelDlg()
{
}


BEGIN_MESSAGE_MAP(CDataModelDlg, CFrameWnd)
	//{{AFX_MSG_MAP(CDataModelDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataModelDlg message handlers

/////////////////////////////////////////////////////////////////////////////
// Selezione singola di un componente (progetto, classe, sottoclasse, secondaria)
// COneGEOComponentSelDlg - INIZIO
COneGEOComponentSelDlg::COneGEOComponentSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COneGEOComponentSelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COneGEOComponentSelDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
   GraphSelectionVis       = FALSE;   // Visibilità bottone per selezione di oggetti grafici
   ConnectionCheckVis      = FALSE;   // Visibilità per scelta controllo connessioni
   ConnectionCheck         = TRUE;    // Controllo connessioni
   UseCurrentPrjOnly       = false;
   FilterOnExtractedClass  = FALSE;   // Nessun filtro
   FilterOnUpdateableClass = FALSE;   // Nessun filtro
   ComponentToSelect       = gsAny;   // qualsiasi

   Prj = Cls = Sub = Sec = 0;

   pToolTip  = NULL;
   HelpStrId = 0;
}

COneGEOComponentSelDlg::~COneGEOComponentSelDlg()
{
   if (hBmpGraphSelection) DeleteObject((HGDIOBJ) hBmpGraphSelection);
   if (pToolTip) delete pToolTip;
}

void COneGEOComponentSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COneGEOComponentSelDlg)
	DDX_Control(pDX, IDC_GRAPH_SELECTION, m_GraphSelection);
	DDX_Control(pDX, IDC_MSG, m_Msg);
	DDX_Control(pDX, IDC_GEOTREECTRL, m_GEOTreeCtrl);
	DDX_Control(pDX, IDC_ENABLE_CONNECTION, m_EnableConnection);
	//}}AFX_DATA_MAP
}


BOOL COneGEOComponentSelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   SetWindowText(Title);
   m_Msg.SetWindowText(Msg);

   if (GraphSelectionVis)
   {
      COLORREF  crFrom;
	   HINSTANCE Instance;

      crFrom   = RGB(255, 0, 0); // rosso
	   // determine location of the bitmap in resource fork
      Instance           = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_SELECT_OBJS), RT_BITMAP);
      hBmpGraphSelection = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SELECT_OBJS));
      gsui_SetBmpColorToDlgBkColor(hBmpGraphSelection, crFrom);
      m_GraphSelection.SetBitmap(hBmpGraphSelection);
      m_GraphSelection.ShowWindow(SW_SHOW);
   }   
   else
      m_GraphSelection.ShowWindow(SW_HIDE);

   if (ConnectionCheckVis)
   {
      m_EnableConnection.ShowWindow(SW_SHOW);
      m_EnableConnection.SetCheck((ConnectionCheck) ? BST_CHECKED : BST_UNCHECKED);
      ChangeSelectionGeotreectrl(); // abilita o meno m_EnableConnection
   }
   else
   {
      CRect rect;
      int   cy;
      
      m_EnableConnection.ShowWindow(SW_HIDE);
      m_EnableConnection.GetWindowRect(&rect);
      cy = rect.top + rect.Height();

      m_GEOTreeCtrl.GetWindowRect(&rect);
      m_GEOTreeCtrl.SetWindowPos(NULL, 0, 0, 
                                 rect.Width(), cy - rect.top,
                                 SWP_NOZORDER | SWP_NOMOVE);
   }   

   FilterOnClassCategoryTypeList.copy(&m_GEOTreeCtrl.FilterOnClassCategoryTypeList);
   FilterOnCodes.copy(m_GEOTreeCtrl.FilterOnCodes);
   m_GEOTreeCtrl.FilterOnExtracted = FilterOnExtractedClass;
   m_GEOTreeCtrl.FilterOnUpdateable = FilterOnUpdateableClass;
   
   if (ComponentToSelect == gsAny || ComponentToSelect & gsSecondaryTable)
      m_GEOTreeCtrl.FinalObjectType = GSSecondaryTab;
   else if (ComponentToSelect & gsSubClass)
      m_GEOTreeCtrl.FinalObjectType = GSSubClass;
   else if (ComponentToSelect & gsClass)
      m_GEOTreeCtrl.FinalObjectType = GSClass;
   else if (ComponentToSelect & gsProject)
      m_GEOTreeCtrl.FinalObjectType = GSProject;

   if (UseCurrentPrjOnly && get_GS_CURRENT_WRK_SESSION())
     m_GEOTreeCtrl.LoadFromDB(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   else
      m_GEOTreeCtrl.LoadFromDB();
   m_GEOTreeCtrl.Refresh();
   m_GEOTreeCtrl.SetFocus();

   if (Prj > 0)
      if (Cls > 0)
         if (Sub > 0)
            if (Sec > 0)
               m_GEOTreeCtrl.SetSelectedSec(Prj, Cls, Sub, Sec);
            else
               m_GEOTreeCtrl.SetSelectedSub(Prj, Cls, Sub);
         else
            m_GEOTreeCtrl.SetSelectedCls(Prj, Cls);
      else
         m_GEOTreeCtrl.SetSelectedPrj(Prj);
   
   pToolTip = new CToolTipCtrl;
   if (pToolTip->Create(this))
   {
	   pToolTip->AddTool(&m_GraphSelection, _T("Seleziona oggetto"));
      pToolTip->Activate(TRUE);
   }

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(COneGEOComponentSelDlg, CDialog)
	//{{AFX_MSG_MAP(COneGEOComponentSelDlg)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDC_GRAPH_SELECTION, OnGraphSelection)
	//}}AFX_MSG_MAP
   ON_BN_CLICKED(IDC_ENABLE_CONNECTION, OnBnClickedEnableConnection)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &COneGEOComponentSelDlg::OnTvnSelchangedGeotreectrl)
   ON_NOTIFY(NM_DBLCLK, IDC_GEOTREECTRL, &COneGEOComponentSelDlg::OnNMDblclkGeotreectrl)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COneGEOComponentSelDlg message handlers

void COneGEOComponentSelDlg::OnOK() 
{
   Component WhatIsChoosen;
   bool      IsSimulation = FALSE;
   C_STRING  Msg;

   Msg = _T("Selezionare");

	// validation
   if (m_GEOTreeCtrl.GetSelected(GSProject) != 0)
      if (m_GEOTreeCtrl.GetSelected(GSClass) != 0)
         if (m_GEOTreeCtrl.GetSelected(GSSubClass) != 0)
            if (m_GEOTreeCtrl.GetSelected(GSSecondaryTab) != 0)
               WhatIsChoosen = gsSecondaryTable;
            else
               WhatIsChoosen = gsSubClass;
         else
            if (m_GEOTreeCtrl.GetSelected(GSSecondaryTab) != 0)
               WhatIsChoosen = gsSecondaryTable;
            else
            {
               C_CLASS *pCls;

               WhatIsChoosen = gsClass;

               // Controllo se si tratta di una classe simulazione
               if ((pCls = gsc_find_class(m_GEOTreeCtrl.GetSelected(GSProject), 
                                          m_GEOTreeCtrl.GetSelected(GSClass))) &&
                   pCls->ptr_id()->category == CAT_EXTERN)
                  // Se si è scelto di selezionare una sottoclasse
                  // non è possibile scegliere la classe madre
                  if (ComponentToSelect & gsSubClass)
                  {
                     Msg += _T(" una sottoclasse.");
                     gsui_alert(Msg);
                     return;
                  }

            }
      else
         WhatIsChoosen = gsProject;
   else
      WhatIsChoosen = gsAny;

   // Se non è stato selezionato quello che volevo
   if (!(WhatIsChoosen & ComponentToSelect))
   {
      if (ComponentToSelect & gsProject) // progetto
         Msg += _T(" un progetto");
      if (ComponentToSelect & gsClass) // classe
      {
         if (ComponentToSelect & gsProject) Msg += _T(',');
         Msg += _T(" una classe");
      }
      if (ComponentToSelect & gsSubClass) // sottoclasse
      {
         if (ComponentToSelect & gsProject ||
             ComponentToSelect & gsClass) Msg += _T(',');
         Msg += _T(" una sottoclasse");
      }
      if (ComponentToSelect & gsSecondaryTable) // secondaria
      {
         if (ComponentToSelect & gsProject ||
             ComponentToSelect & gsClass ||
             ComponentToSelect & gsSubClass) Msg += _T(',');
         Msg += _T(" una tabella secondaria");
      }
      Msg += _T('.');

      gsui_alert(Msg);

      return;
   }

   Prj = m_GEOTreeCtrl.GetSelected(GSProject);
	Cls = m_GEOTreeCtrl.GetSelected(GSClass);
	Sub = m_GEOTreeCtrl.GetSelected(GSSubClass);
	Sec = m_GEOTreeCtrl.GetSelected(GSSecondaryTab);
	
	CDialog::OnOK();
}


void COneGEOComponentSelDlg::OnHelp() 
{
   if (HelpStrId > 0) gsc_help(HelpStrId);
}


BOOL COneGEOComponentSelDlg::PreTranslateMessage(MSG* pMsg) 
{
   if (pToolTip) pToolTip->RelayEvent(pMsg);
	
	return CDialog::PreTranslateMessage(pMsg);
}


void COneGEOComponentSelDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   ChangeSelectionGeotreectrl();

   *pResult = 0;
}


void COneGEOComponentSelDlg::OnNMDblclkGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   OnOK();
   *pResult = 0;
}


void COneGEOComponentSelDlg::OnGraphSelection() 
{
	ads_name  ent;
	ads_point pt;
   bool      Found = FALSE;
   C_CLASS   *pCls;

   if (!get_GS_CURRENT_WRK_SESSION()) return;

   GetParent()->EnableWindow(TRUE);
   ShowWindow(SW_HIDE);
   
   do
   {
      if (acedEntSel(_T(""), ent, pt) == RTNORM)
      {
         if ((pCls = get_GS_CURRENT_WRK_SESSION()->find_class(ent)) != NULL &&
             pCls->is_extracted() == GS_GOOD)
         { 
            Found = TRUE;
            break;
         }
      }
      else
         break;

      acutPrintf(_T("\nOggetto non valido."));
   }
   while (1);

   ShowWindow(SW_SHOW);
   SetFocus();
   GetParent()->EnableWindow(FALSE);
   EnableWindow(TRUE);

   if (Found)
   {
      if (pCls->ptr_id()->sub_code > 0)
         m_GEOTreeCtrl.SetSelectedSub(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), pCls->ptr_id()->code, pCls->ptr_id()->sub_code);
      else
         m_GEOTreeCtrl.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), pCls->ptr_id()->code);

      m_GEOTreeCtrl.Refresh();
      m_GEOTreeCtrl.SetFocus();
   }
}


void COneGEOComponentSelDlg::ChangeSelectionGeotreectrl()
{
   bool Disable = FALSE;
   long _prj, _cls, _sub;

   _prj = m_GEOTreeCtrl.GetSelected(GSProject);
   _cls = m_GEOTreeCtrl.GetSelected(GSClass);
   _sub = m_GEOTreeCtrl.GetSelected(GSSubClass);

   // Se NON è possibile scegliere se usare le connessioni
   if (!ConnectionCheckVis) return;
   
   if (m_GEOTreeCtrl.GetSelected(GSProject) == 0 || m_GEOTreeCtrl.GetSelected(GSClass) == 0) Disable = TRUE;
   else
   // Se la classe selezionata è una sottoclasse
   if (m_GEOTreeCtrl.GetSelected(GSSubClass) > 0) Disable = TRUE;
   else
   {
      C_CLASS *pCls;

      if (!(pCls = (C_CLASS *) gsc_find_class(m_GEOTreeCtrl.GetSelected(GSProject),
                                              m_GEOTreeCtrl.GetSelected(GSClass)))) return;  

      // Se la classe selezionata non ha una lista di connessioni
      if (!pCls->ptr_connect_list()) Disable = TRUE;
      else if (pCls->ptr_connect_list()->get_count() == 0) Disable = TRUE;
   }
   
   m_EnableConnection.EnableWindow((Disable) ? FALSE : TRUE);

   return;
}


void COneGEOComponentSelDlg::OnBnClickedEnableConnection()
{
   ConnectionCheck = (m_EnableConnection.GetCheck() == BST_CHECKED) ? TRUE : FALSE;   
}


// COneGEOComponentSelDlg - FINE


/*************************************************************************/
/*.doc gsui_DisplayDWGExt                                                */
/*+
    Comando per disegnare le estensioni dei disegni di una classe di entità.
-*/
/*************************************************************************/
int gsui_DisplayDWGExt(void)
{
   int      Prj;

   acedRetVoid();

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   COneGEOComponentSelDlg OneGEOComponentSelDlg;

   OneGEOComponentSelDlg.Title = _T("GEOsim - Mostra estensioni disegni");
   OneGEOComponentSelDlg.Msg   = _T("Seleziona una classe:");

   OneGEOComponentSelDlg.ComponentToSelect = COneGEOComponentSelDlg::gsClass; // Selezione di una classe
   if (gsc_getLastUsedPrj(&Prj) == GS_GOOD && Prj > 0)
      OneGEOComponentSelDlg.Prj = Prj;
   // Solo le classi con grafica
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SPAGHETTI, TYPE_SPAGHETTI);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_POLYLINE);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_TEXT);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_NODE);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_SURFACE);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_EXTERN, 0);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_POLYLINE);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_TEXT);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_NODE);
   OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_SURFACE);

   OneGEOComponentSelDlg.FilterOnExtractedClass = FALSE;
   OneGEOComponentSelDlg.FilterOnUpdateableClass = FALSE;
   if (OneGEOComponentSelDlg.DoModal() != IDOK) return GS_GOOD;

   if (OneGEOComponentSelDlg.Prj > 0 && OneGEOComponentSelDlg.Cls > 0)
      gsc_DisplayDWGExt(OneGEOComponentSelDlg.Prj, OneGEOComponentSelDlg.Cls);

   return GS_GOOD;
}


/*************************************************************************/
/*.doc gsui_DelDataModelComponent            solo per prova              */
/*+
    Comando per cancellare un componente dal modello dei dati (progetto,
    classe, tabella secondaria).
-*/
/*************************************************************************/
int gsui_DelDataModelComponent(void)
{
   C_PROJECT     *pPrj;
   C_STRING      Msg, PrjPath, FilePath;
   C_ATTRIB_LIST AttribList;

   acedRetVoid();

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   COneGEOComponentSelDlg OneGEOComponentSelDlg;

   OneGEOComponentSelDlg.Title = _T("GEOsim - Cancellazione modello dati");
   OneGEOComponentSelDlg.Msg   = _T("Seleziona componente da cancellare:");

   OneGEOComponentSelDlg.ComponentToSelect = COneGEOComponentSelDlg::gsAny;
    // Tutti i progetti, classi, sottoclassi, tabelle secondarie
   if (OneGEOComponentSelDlg.DoModal() != IDOK) return GS_GOOD;
   
   pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(OneGEOComponentSelDlg.Prj);
   if (!pPrj) return GS_BAD;

   if (OneGEOComponentSelDlg.Sec > 0)
   {
      C_SECONDARY *pSec;
      
      if (!(pSec = gsc_find_sec(OneGEOComponentSelDlg.Prj,
                                OneGEOComponentSelDlg.Cls,
                                OneGEOComponentSelDlg.Sub,
                                OneGEOComponentSelDlg.Sec)))
         return GS_BAD;

      Msg = _T("Confermare la cancellazione della tabella secondaria <");
      Msg += pSec->name;
      Msg += _T(">");

      // Msg, int Default, bool UsePassword
      if (gsui_confirm(Msg.get_name(), GS_BAD, TRUE) == GS_BAD) return GS_GOOD;
   }
   else
   if (OneGEOComponentSelDlg.Cls > 0)
   {
      C_CLASS *pCls;
      
      if (!(pCls = gsc_find_class(OneGEOComponentSelDlg.Prj,
                                  OneGEOComponentSelDlg.Cls)))
         return GS_BAD;

      Msg = _T("Confermare la cancellazione della classe <");
      Msg += pCls->get_name();
      Msg += _T(">");

      // Msg, int Default, bool UsePassword
      if (gsui_confirm(Msg.get_name(), GS_BAD, TRUE) == GS_BAD) return GS_GOOD;
      pCls->ptr_attrib_list()->copy(&AttribList);
      PrjPath = pCls->get_pPrj()->get_dir();
   }
   else
   if (OneGEOComponentSelDlg.Prj > 0)
   {
      Msg = _T("Confermare la cancellazione del progetto <");
      Msg += pPrj->get_name();
      Msg += _T(">");

      // Msg, int Default, bool UsePassword
      if (gsui_confirm(Msg.get_name(), GS_BAD, TRUE) == GS_BAD) return GS_GOOD;
   }

   // Elimino eventuali file FDF, DEF, TAB, REF
   C_ATTRIB *pAttrib = (C_ATTRIB *) AttribList.get_head();
   while (pAttrib)
   {
      while (gsui_getValuesListFilePath(PrjPath,
                                        OneGEOComponentSelDlg.Prj,
                                        OneGEOComponentSelDlg.Cls,      
                                        OneGEOComponentSelDlg.Sub,
                                        OneGEOComponentSelDlg.Sec,
                                        pAttrib->name, FilePath, GS_GOOD) == GS_GOOD)
         gsc_delfile(FilePath);

      pAttrib = (C_ATTRIB *) AttribList.get_next();
   }

   return GS_GOOD;
}


//////////////////////////////////////////////////////////////////////////
// Funzioni per l'inserimento delle entità - INIZIO
//////////////////////////////////////////////////////////////////////////


/*************************************************************************/
/*.doc gsc_ui_getDirectInsertableClassList                               */
/*+
   Funzione che restituisce una lista contenente i codici delle classi
   e sottoclassi le cui entità sono inseribili direttamente (quelle estratte,
   modificabili e, nel caso di sottoclassi, che abbiano regole di connessione
   topologiche che consentano di essere inserite direttamente).
   Parametri:
   C_FAMILY_LIST &FilterOnCodes;

   Restituisce GS_GOOD in caso di successo altrimenti GS_BAD
-*/
/*************************************************************************/
int gsc_ui_getDirectInsertableClassList(C_FAMILY_LIST &FilterOnCodes)
{
   C_CLS_PUNT_LIST ExtrClsList, ExtrSubList;
   C_CLS_PUNT      *pExtrCls, *pExtrSub;
   C_CLASS         *pCls, *pSub;
   C_FAMILY        *p;

   FilterOnCodes.remove_all();
   FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   p = (C_FAMILY *) FilterOnCodes.get_head();

   // Ottiene la lista delle classi estratte nella sessione corrente
   if (get_GS_CURRENT_WRK_SESSION()->get_pPrj()->extracted_class(ExtrClsList) == GS_BAD)
      return GS_BAD;

   pExtrCls = (C_CLS_PUNT *) ExtrClsList.get_head();
   while (pExtrCls)
   {
      pCls = (C_CLASS *) pExtrCls->cls;

      if (pCls->ptr_id()->abilit != GSUpdateableData) // scarto le classi non modificabili
      {
         pExtrCls = (C_CLS_PUNT *) ExtrClsList.get_next();
         continue;
      }

      if (pCls->ptr_id()->category == CAT_SPAGHETTI) // scarto le classi spaghetti
      {
         pExtrCls = (C_CLS_PUNT *) ExtrClsList.get_next();
         continue;
      }

      if (pCls->ptr_id()->category == CAT_GRID) // scarto le classi griglia
      {
         pExtrCls = (C_CLS_PUNT *) ExtrClsList.get_next();
         continue;
      }

      p->relation.add_tail_int(pCls->ptr_id()->code);

      if (pCls->ptr_id()->category == CAT_EXTERN)
         if (((C_EXTERN *) pCls)->get_DirectInsertableSubs(ExtrSubList) == GS_GOOD)
         {
            pExtrSub = (C_CLS_PUNT *) ExtrSubList.get_head();
            while (pExtrSub)
            {
               pSub = (C_CLASS *) pExtrSub->cls;
               ((C_FAMILY *) p->relation.get_cursor())->relation.add_tail_int(pSub->ptr_id()->sub_code);

               pExtrSub = (C_CLS_PUNT *) ExtrSubList.get_next();
            }
         }

      pExtrCls = (C_CLS_PUNT *) ExtrClsList.get_next();
   }

   return GS_GOOD;
}


/*************************************************************************/
/*.doc gsuiinsdata                                                       */
/*+
    Comando per inserire un'entità di GEOsim.
-*/
/*************************************************************************/
int gsuiinsdata(void)
{
   return gsc_ui_insdata();
}
int gsc_ui_insdata(int cls, int sub, bool ConnectionCheck)
{
   int                    WhyNot;
   COneGEOComponentSelDlg OneGEOComponentSelDlg;
   C_CLASS                *pCls;
   C_SELSET               ss;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   if (get_GS_CURRENT_WRK_SESSION()->isReadyToUpd(&WhyNot) == GS_BAD)
   {
      if (WhyNot == eGSInvSessionStatus)
      {
         gsui_alert(_T("Lo stato della sessione di lavoro non consente questo comando."));
         set_GS_ERR_COD(eGSInvSessionStatus);
      }
      else if (WhyNot == eGSReadOnlySession)
      {
         gsui_alert(_T("Sessione di lavoro non aggiornabile."));
         set_GS_ERR_COD(eGSReadOnlySession);
      }

      return GS_BAD;
   }

   // La scelta della classe va fatta con interfaccia grafica
   if (cls == 0)
   {
      // Ricavo una lista (in formato stringa) delle classi e sottoclassi
      // direttamente inseribili
      if (gsc_ui_getDirectInsertableClassList(OneGEOComponentSelDlg.FilterOnCodes) == GS_BAD)
         return GS_BAD;

      OneGEOComponentSelDlg.Title = _T("GEOsim - Inserimento entità");
      OneGEOComponentSelDlg.Msg   = _T("Seleziona classe da inserire:");

      // gli spaghetti devono essere importati perchè dalla 2015 non si riesce a controllare i comandi autocad
      //OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SPAGHETTI, TYPE_SPAGHETTI); 
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_POLYLINE);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_TEXT);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_NODE);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_SURFACE);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_EXTERN, 0);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_POLYLINE);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_TEXT);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_NODE);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_SURFACE);
      OneGEOComponentSelDlg.FilterOnClassCategoryTypeList.values_add_tail(CAT_GROUP, TYPE_GROUP);

      OneGEOComponentSelDlg.UseCurrentPrjOnly = true; // usa solo il progetto corrente
      OneGEOComponentSelDlg.FilterOnExtractedClass = true; // Solo classi estratte
      OneGEOComponentSelDlg.FilterOnUpdateableClass = true; // Solo classi aggiornabili
      OneGEOComponentSelDlg.ComponentToSelect = COneGEOComponentSelDlg::gsClass | 
                                                COneGEOComponentSelDlg::gsSubClass;
      OneGEOComponentSelDlg.Prj = get_GS_CURRENT_WRK_SESSION()->get_PrjId();

      OneGEOComponentSelDlg.HelpStrId = IDH_Inserimentoentit;

      if (get_LAST_CLS() == 0)
      {  // non è stata inizializzato il codice dell'ultima classe inserita
         C_FAMILY   *pCode;

         if ((pCode = (C_FAMILY *) OneGEOComponentSelDlg.FilterOnCodes.get_head())) // progetto
            if ((pCode = (C_FAMILY *) pCode->relation.get_head())) // prima classe
            {
               OneGEOComponentSelDlg.Cls = pCode->get_key();
               if ((pCode = (C_FAMILY *) pCode->relation.get_head())) // prima sotto-classe
                  OneGEOComponentSelDlg.Sub = pCode->get_key();
            }
      }
      else
      {
         OneGEOComponentSelDlg.Cls = get_LAST_CLS();
         OneGEOComponentSelDlg.Sub = get_LAST_SUB();
      }

      OneGEOComponentSelDlg.GraphSelectionVis  = TRUE; // Bottone per selezione di oggetti grafici
      OneGEOComponentSelDlg.ConnectionCheckVis = TRUE; // Check di scelta uso connessioni

      // Tutti i progetti, classi, sottoclassi, tabelle secondarie
      if (OneGEOComponentSelDlg.DoModal() != IDOK) return GS_GOOD;

      ConnectionCheck = OneGEOComponentSelDlg.ConnectionCheck;
      if ((pCls = (C_CLASS *) get_GS_CURRENT_WRK_SESSION()->find_class(OneGEOComponentSelDlg.Cls,
                                                                       OneGEOComponentSelDlg.Sub)) == NULL)
         return GS_BAD;
   }
   else // la classe è stata passata come parametro
      if ((pCls = (C_CLASS *) get_GS_CURRENT_WRK_SESSION()->find_class(cls, sub)) == NULL)
         return GS_BAD;

   if (pCls->ptr_id()->sub_code > 0) // Si tratta di una sottoclasse
   {
      C_CLASS *pMotherCls;

      pMotherCls = (C_CLASS *) get_GS_CURRENT_WRK_SESSION()->find_class(pCls->ptr_id()->code, 0);
      if ((WhyNot = pMotherCls->InsertEnt((C_SUB *) pCls, &ss)) != GS_GOOD)
         return WhyNot;

   }
   else
      if ((WhyNot = pCls->InsertEnt(NULL, ConnectionCheck, NULL, &ss)) != GS_GOOD)
         return WhyNot;

   if (pCls->ptr_id()->category != CAT_SPAGHETTI)
   {
      ads_name ent;
      ss.get_selection(ent);
      acedSSSetFirst(ent, ent);
      gsc_WrkSessionPanel(TRUE);
   }

   return GS_GOOD;
}


//////////////////////////////////////////////////////////////////////////
// Funzioni per l'inserimento delle entità - FINE
// Funzioni per l'aggiornamento delle entità - INIZIO
//////////////////////////////////////////////////////////////////////////


/*************************************************************************/
/*.doc gsuidbdata                                                        */
/*+
   Comando per interrogare e modificare i dati di database di una o
   più entità di GEOsim.
-*/
/*************************************************************************/
int gsuidbdata(void)
{
   ads_name ss;
   long     qty;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
      if (gsc_ssget(NULL, NULL, NULL, NULL, ss) != RTNORM) return GS_CAN;
      if (acedSSLength(ss, &qty) == RTNORM && qty > 0) break;
   }
   while (1);

   acedSSSetFirst(NULL, NULL);
   acedSSSetFirst(ss, ss);
   acedSSFree(ss);

   gsc_WrkSessionPanel(TRUE);

   return GS_GOOD;
}


/*************************************************************************/
/*.doc gsuidbGriddata                                                    */
/*+
   Comando per interrogare e modificare i dati di database di una o
   più entità di GEOsim di tipo griglia in una finestra.
-*/
/*************************************************************************/
int gsuidbGriddata(void)
{
   C_CLS_PUNT_LIST ExtrGridClsList;
   ads_point       pt1, pt2;
   int             res;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   // verifico se tra le classi estratte vi sono delle classi tipo griglia
   if (get_GS_CURRENT_WRK_SESSION()->get_pPrj()->extracted_class(ExtrGridClsList, CAT_GRID) == GS_BAD ||
       ExtrGridClsList.get_count() == 0)
   {
      gsui_alert(_T("Nessuna classe griglia estratta."));
      return GS_BAD;
   }

   acedInitGet(RSG_OTHER, _T("ssfilter"));
   res = acedGetPoint(NULL, _T("\nSelezionare zona: "), pt1);
   switch (res)
   {
      case RTNORM: // richiesta secondo angolo della finestra
         acedInitGet(RSG_OTHER, NULL);
         res = acedGetCorner(pt1, _T("\nSpecificare secondo punto della zona (<Invio> per nessuno): "), pt2);        
         if (res != RTNORM && res != RTNONE) return GS_CAN;
         if (res == RTNONE) ads_point_set(pt1, pt2);

         acedSSSetFirst(NULL, NULL);

         gsc_WrkSessionPanel(TRUE);
         // Setto modalità griglie
         pDockPaneWrkSession->GridDBQuery(pt1, pt2);
         break;

      case RTKWORD: // risultato ultimo filtro
      {
         C_LINK_SET *pLS = get_GS_LSFILTER();
         C_CLASS    *pCls;
      
         if (!pLS) return GS_CAN;

         if ((pCls = get_GS_CURRENT_WRK_SESSION()->find_class(pLS->cls)) == NULL) return GS_BAD;
         if (pCls->get_category() != CAT_GRID) { set_GS_ERR_COD(eGSInvClassType); return GS_CAN; }

         acedSSSetFirst(NULL, NULL);

         gsc_WrkSessionPanel(TRUE);
         // Setto modalità griglie
         pDockPaneWrkSession->GridDBQuery((C_CGRID *) pCls, *(pLS->ptr_KeyList()));
         break;
      }

      default:
         return GS_CAN;
   }

   return GS_GOOD;
}


/*************************************************************************/
/*.doc gsuidbGriddataOnKeyList                                           */
/*+
   Funzione LISP per interrogare e modificare i dati di database di una o
   più entità di GEOsim di tipo griglia data la lista delle celle.
   Parametri:
   (<cls> (<key1> <key2> ...))
   se la funzione noin riceve parametri
   viene utilizzato il risultato dell'ultimo filtro
-*/
/*************************************************************************/
int gsuidbGriddataOnKeyList(void)
{
   presbuf      arg = acedGetArgs(); // ricavo i valori impostati
   int          cls;
   C_LONG_BTREE KeyList, *pKeys;
   C_CLASS      *pCls;

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   if (arg)
   {
      // codice classe
      if (gsc_rb2Int(arg, &cls) == GS_BAD) return RTERROR;
      // lista dei codici delle celle
      if ((arg = arg->rbnext) == NULL) { set_GS_ERR_COD(eGSInvalidArg); return RTERROR; }
      if (KeyList.from_rb(arg) == GS_BAD) return RTERROR;
      pKeys = &KeyList;
   }
   else // leggo dal risultato del filtro
   {
      C_LINK_SET *pLS = get_GS_LSFILTER();
      
      if (!pLS) return RTERROR;

      cls   = pLS->cls;
      pKeys = pLS->ptr_KeyList();
   }

   if ((pCls = get_GS_CURRENT_WRK_SESSION()->find_class(cls)) == NULL) return RTERROR;
   if (pCls->get_category() != CAT_GRID) { set_GS_ERR_COD(eGSInvClassType); return RTERROR; }

   acedSSSetFirst(NULL, NULL);

   gsc_WrkSessionPanel(TRUE);
   // Setto modalità griglie
   pDockPaneWrkSession->GridDBQuery((C_CGRID *) pCls, *pKeys);

   return GS_GOOD;
}


/////////////////////////////////////////////////////////////////////////////
// Selezione multipla di classi di entità
// CMultiGEOClassSelDlg - INIZIO
IMPLEMENT_DYNAMIC(CMultiGEOClassSelDlg, CDialog)

CMultiGEOClassSelDlg::CMultiGEOClassSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiGEOClassSelDlg::IDD, pParent)
{
   Prj                     = 0;
   GraphSelectionVis       = FALSE;   // Visibilità bottone per selezione di oggetti grafici
   FilterOnExtractedClass  = FALSE;   // Nessun filtro
   FilterOnUpdateableClass = FALSE;   // Nessun filtro
   FilterOnSecondaryTable  = _T("*"); // Tutte le tab. secondarie
   StatusColumnVisibility  = FALSE;   // Senza la colonna di stato
   MultiSelect             = TRUE;    // Multiselezione abilitata
   PrjSelectionEnabled     = TRUE;    // Possibilità di scegliere un progetto
   ColumnAutoSize          = TRUE;    // Le larghezza delle delle colonne calcolata in automatico

   m_GEOClassListCtrl.ObjectType = GSClass;

   pToolTip  = NULL;
   HelpStrId = 0;
}

CMultiGEOClassSelDlg::~CMultiGEOClassSelDlg()
{
   if (hBmpGraphSelection) DeleteObject((HGDIOBJ) hBmpGraphSelection);
   if (pToolTip) delete pToolTip;
}

void CMultiGEOClassSelDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);

   DDX_Control(pDX, IDC_GRAPH_SELECTION, m_GraphSelection);
   DDX_Control(pDX, IDC_MSG, m_Msg);
   DDX_Control(pDX, IDC_GEOCLASSLISTCTRL, m_GEOClassListCtrl);
   DDX_Control(pDX, IDC_PROJECT_COMBO, m_ProjectComboCtrl);
}

void CMultiGEOClassSelDlg::OnInitPrjList()
{
   C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->get_head(), *pSelPrj = NULL;
   while (m_ProjectComboCtrl.DeleteString(0) != CB_ERR); // svuoto la combo

   while (pPrj)
   {
      m_ProjectComboCtrl.AddString(pPrj->get_name());
      if (Prj == pPrj->get_key()) pSelPrj = pPrj;

      pPrj = (C_PROJECT *) get_GS_PROJECT()->get_next(); 
   }

   if (!pSelPrj)
      if (get_GS_CURRENT_WRK_SESSION())
      {
         pSelPrj = get_GS_CURRENT_WRK_SESSION()->get_pPrj();
         Prj = pSelPrj->get_key();
      }
      else
      if (gsc_getLastUsedPrj(&Prj) == GS_GOOD)
         pSelPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj);

   if (!pSelPrj && get_GS_PROJECT()->get_count() == 1)
   {
      pSelPrj = (C_PROJECT *) get_GS_PROJECT()->get_head();
      Prj = pSelPrj->get_key();
   }
   if (pSelPrj)
   {
      m_ProjectComboCtrl.SetWindowText(pSelPrj->get_name());
      m_GEOClassListCtrl.FilterOnPrj = Prj;
      m_GEOClassListCtrl.LoadFromDB();
      m_GEOClassListCtrl.Refresh();
      m_GEOClassListCtrl.SetFocus();
   }
}

BOOL CMultiGEOClassSelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   SetWindowText(Title);
   m_Msg.SetWindowText(Msg);

   if (get_GS_CURRENT_WRK_SESSION() && GraphSelectionVis)
   {
      COLORREF  crFrom;
	   HINSTANCE Instance;

      crFrom   = RGB(255, 0, 0); // rosso
	   // determine location of the bitmap in resource fork
      Instance           = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_SELECT_OBJS), RT_BITMAP);
      hBmpGraphSelection = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SELECT_OBJS));
      gsui_SetBmpColorToDlgBkColor(hBmpGraphSelection, crFrom);
      m_GraphSelection.SetBitmap(hBmpGraphSelection);
      m_GraphSelection.ShowWindow(SW_SHOW);
   }   
   else
      m_GraphSelection.ShowWindow(SW_HIDE);

   FilterOnClassCategoryType.copy(&m_GEOClassListCtrl.FilterOnClassCategoryTypeList);
   m_GEOClassListCtrl.FilterOnExtractedInCurrWrkSession = FilterOnExtractedClass;
   m_GEOClassListCtrl.FilterOnUpdateable = FilterOnUpdateableClass;
   if (FilterOnClassCodeList.get_count() == 0)
      m_GEOClassListCtrl.UseFilterOnCodes = false;
   else
      m_GEOClassListCtrl.UseFilterOnCodes = true;
   FilterOnClassCodeList.copy(&m_GEOClassListCtrl.FilterOnCodes);
   m_GEOClassListCtrl.ColumnClassStatusVisibility = StatusColumnVisibility;
   m_GEOClassListCtrl.MultiSelect    = MultiSelect;
   m_GEOClassListCtrl.ColumnAutosize = ColumnAutoSize;

   // OnInitPrjList viene eseguito dopo aver settato le proprietà di m_GEOClassListCtrl 
   // perchè lancia il metodo "update" di m_GEOClassListCtrl
   OnInitPrjList();
   if (!PrjSelectionEnabled) // se non si può scegliere il progetto
      m_ProjectComboCtrl.EnableWindow(FALSE);

   // "SetSelectedCodes" deve essere lanciato dopo il metodo "update"
   m_GEOClassListCtrl.SetSelectedCodes(ClsCodeList); 

   pToolTip = new CToolTipCtrl;
   if (pToolTip->Create(this))
   {
	   pToolTip->AddTool(&m_GraphSelection, _T("Seleziona oggetto"));
      pToolTip->Activate(TRUE);
   }

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CMultiGEOClassSelDlg, CDialog)
   ON_BN_CLICKED(IDC_GRAPH_SELECTION, &CMultiGEOClassSelDlg::OnBnClickedGraphSelection)
   ON_BN_CLICKED(IDOK, &CMultiGEOClassSelDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CMultiGEOClassSelDlg::OnBnClickedHelp)
   ON_CBN_SELCHANGE(IDC_PROJECT_COMBO, &CMultiGEOClassSelDlg::OnCbnSelchangeProjectCombo)
   ON_NOTIFY(NM_DBLCLK, IDC_GEOCLASSLISTCTRL, &CMultiGEOClassSelDlg::OnNMDblclkGeoclasslistctrl)
END_MESSAGE_MAP()


// CMultiGEOClassSelDlg message handlers

void CMultiGEOClassSelDlg::OnCbnSelchangeProjectCombo()
{
   int       CurSel;
   CString   dummy;
   C_PROJECT *pPrj;

   if ((CurSel = m_ProjectComboCtrl.GetCurSel()) == CB_ERR) return;
   m_ProjectComboCtrl.GetLBText(CurSel, dummy);
   if (!(pPrj = (C_PROJECT *) get_GS_PROJECT()->search_name(dummy))) return;
   Prj = pPrj->get_key();
   m_GEOClassListCtrl.FilterOnPrj = Prj;
   m_GEOClassListCtrl.LoadFromDB();
   m_GEOClassListCtrl.Refresh();
   m_GEOClassListCtrl.SetFocus();
}

void CMultiGEOClassSelDlg::OnBnClickedGraphSelection()
{
   C_SELSET       SelSet;
   C_INT_INT_LIST ClsCodeSubcodeList;
   C_INT_INT      *pClsCodeSubcode;

   if (!get_GS_CURRENT_WRK_SESSION()) return;

   GetParent()->EnableWindow(TRUE);
   ShowWindow(SW_HIDE);
   
   if (gsc_ssget(NULL, NULL, NULL, NULL, SelSet) == RTNORM)
      SelSet.get_ClsCodeList(ClsCodeSubcodeList);

   ShowWindow(SW_SHOW);
   SetFocus();
   GetParent()->EnableWindow(FALSE);
   EnableWindow(TRUE);

   if (SelSet.get_ClsCodeList(ClsCodeSubcodeList) == GS_GOOD && 
       (pClsCodeSubcode = (C_INT_INT *) ClsCodeSubcodeList.get_head()) != NULL)
   {
      C_INT_LIST ClsCodeList;

      while (pClsCodeSubcode)
      {
         ClsCodeList.add_tail_int(pClsCodeSubcode->get_key());
         pClsCodeSubcode = (C_INT_INT *) ClsCodeSubcodeList.get_next();
      }
      m_GEOClassListCtrl.SetSelectedCodes(ClsCodeList);
      m_GEOClassListCtrl.Refresh(true);
      m_GEOClassListCtrl.SetFocus();
   }
}

void CMultiGEOClassSelDlg::OnBnClickedOk()
{
   Prj = m_GEOClassListCtrl.FilterOnPrj;
   m_GEOClassListCtrl.GetSelectedCodes(ClsCodeList);
   gsc_setLastUsedPrj(Prj);
   OnOK();
}

void CMultiGEOClassSelDlg::OnBnClickedHelp()
{
   if (HelpStrId > 0) gsc_help(HelpStrId);
}

BOOL CMultiGEOClassSelDlg::PreTranslateMessage(MSG* pMsg) 
{
   if (pToolTip) pToolTip->RelayEvent(pMsg);
	
	return CDialog::PreTranslateMessage(pMsg);
}


void CMultiGEOClassSelDlg::OnNMDblclkGeoclasslistctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

   OnBnClickedOk();

   *pResult = 0;
}


/*************************************************************************/
/*.doc gsui_SelClass                                                */
/*+
  Funzione LISP per fare una selezione singola o multipla delle classi.
  Parametri:
  (("DLGTITLE"<Title>)("DLGMSG"<Msg>)("PRJ"<prj>) ("CLASSCODELIST" (<cls 1>..<cls n>))
   ("FILTERONCLASSCAT"<FilterOnClassCategory>)(""FILTERONCLASSTYPE"<FilterOnClassType>)
   ("FILTERONEXTRACTEDCLASS"<FilterOnExtractedClass>)
   ("FILTERONUPDATEABLECLASS"<FilterOnUpdateableClass>)
   ("FILTERONCLASSCODELIST"<FilterOnClassCodeList>)
   ("FILTERONSEC"<FilterOnSecondaryTable>)("STATUSCOLUMNVIS"<StatusColumnVisibility>)
   ("MULTISELECT"<MultiSelect>)
   ("GRAPHSELVIS"<GraphSelectionVis>")("PRJSELENABLED<PrjSelectionEnabled>)
   ("HELPID"<HelpStrId>))
  Tutti i parametri sono opzionali
  
  Restituisce una lista di codici di classi selezionate
-*/
/*************************************************************************/
int gsui_SelClass(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   presbuf              arg = acedGetArgs(), p;
   CMultiGEOClassSelDlg MultiGEOClassSelDlg;

   acedRetNil();

   do
   {
      if ((p = gsc_CdrAssoc(_T("DLGTITLE"), arg, GS_BAD)) && p->restype == RTSTR)
         MultiGEOClassSelDlg.Title = p->resval.rstring; // titolo
      if ((p = gsc_CdrAssoc(_T("DLGMSG"), arg, GS_BAD)) && p->restype == RTSTR)
         MultiGEOClassSelDlg.Msg = p->resval.rstring; // messaggio
      if ((p = gsc_CdrAssoc(_T("PRJ"), arg, GS_BAD)))
         gsc_rb2Int(p, &(MultiGEOClassSelDlg.Prj)); // codice progetto
      if ((p = gsc_CdrAssoc(_T("CLASSCODELIST"), arg, GS_BAD)))
         MultiGEOClassSelDlg.ClsCodeList.from_rb(p); // Lista di classi selezionate
      if ((p = gsc_CdrAssoc(_T("FILTERONCLASSCATEGORYTYPE"), arg, GS_BAD)))
         MultiGEOClassSelDlg.FilterOnClassCategoryType.from_rb(p); // lista di coppie categoria-tipo di classe
      if ((p = gsc_CdrAssoc(_T("FILTERONEXTRACTEDCLASS"), arg, GS_BAD)))
         gsc_rb2Bool(p, &(MultiGEOClassSelDlg.FilterOnExtractedClass)); // classi estratte
      if ((p = gsc_CdrAssoc(_T("FILTERONUPDATEABLECLASS"), arg, GS_BAD)))
         gsc_rb2Bool(p, &(MultiGEOClassSelDlg.FilterOnUpdateableClass)); // classi aggiornabili
      if ((p = gsc_CdrAssoc(_T("FILTERONCLASSCODELIST"), arg, GS_BAD)))
         MultiGEOClassSelDlg.FilterOnClassCodeList.from_rb(p); // Lista di classi da filtrare
      if ((p = gsc_CdrAssoc(_T("FILTERONSEC"), arg, GS_BAD)) && p->restype == RTSTR)
         MultiGEOClassSelDlg.FilterOnSecondaryTable = p->resval.rstring; // secondarie
      if ((p = gsc_CdrAssoc(_T("STATUSCOLUMNVIS"), arg, GS_BAD)))
         gsc_rb2Bool(p, &(MultiGEOClassSelDlg.StatusColumnVisibility)); // visibilità colonna di stato
      if ((p = gsc_CdrAssoc(_T("MULTISELECT"), arg, GS_BAD)))
         gsc_rb2Bool(p, &(MultiGEOClassSelDlg.MultiSelect)); // Multiselezione abilitata
      if ((p = gsc_CdrAssoc(_T("GRAPHSELVIS"), arg, GS_BAD)))
         gsc_rb2Bool(p, &(MultiGEOClassSelDlg.GraphSelectionVis)); // selezione grafica
      if ((p = gsc_CdrAssoc(_T("PRJSELENABLED"), arg, GS_BAD)))
         gsc_rb2Bool(p, &(MultiGEOClassSelDlg.PrjSelectionEnabled)); // selezione progetto
      if ((p = gsc_CdrAssoc(_T("HELPID"), arg, GS_BAD)))
         gsc_rb2Int(p, &(MultiGEOClassSelDlg.HelpStrId)); // ID pagina help
      if ((p = gsc_CdrAssoc(_T("COLUMNAUTOSIZE"), arg, GS_BAD)))
         gsc_rb2Bool(p, &(MultiGEOClassSelDlg.ColumnAutoSize)); // larghezza colonne automatiche      
   }
   while (0);

   if (MultiGEOClassSelDlg.DoModal() != IDOK) return RTNORM;

   if (MultiGEOClassSelDlg.Prj > 0 && MultiGEOClassSelDlg.ClsCodeList.get_count() > 0)
   {
      C_RB_LIST res;

      // Restituisco la lista
      res << acutBuildList(RTSHORT, MultiGEOClassSelDlg.Prj, RTLB, 0);
      if ((res += MultiGEOClassSelDlg.ClsCodeList.to_rb()) == NULL) return RTERROR;
      res += acutBuildList(RTLE, 0);
      res.LspRetList();
   }

   return GS_GOOD;
}