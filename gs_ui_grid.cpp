/**********************************************************
Name: GS_UI_GRID
                                   
Module description: File funzioni per le griglie
                    in modalità GEOUI (GEOsim User Interface)
            
Author: Roberto Poltini

(c) Copyright 2005-2012 by IREN ACQUA GAS  S.p.A

**********************************************************/


#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include "gs_ui_utily.h"
#include "gs_ui_sql.h"
#include "gs_ui_grid.h"

#include "gs_error.h" 
#include "gs_utily.h"
#include "gs_init.h"
#include "gs_resbf.h"
#include "gs_prjct.h"
#include "gs_filtr.h"   // per gsc_define_window
#include "gs_evid.h"
#include "gs_grid.h"

#include "d2hMap.h" // doc to help

#define IDWINDOWSEL 12
#define IDOBJSSEL   13
#define IDRAYSEL    14
#define IDPOINTSEL  15


//**********************************************************
// INIZIO FUNZIONI DI VARIA UTILITA
//**********************************************************


//**********************************************************
// FINE FUNZIONI DI VARIA UTILITA
// INIZIO CALCOLO DIREZIONE FLUSSO ACQUA
//**********************************************************


/*************************************************************************/
/*.doc gsui_getFirstExtractedGridClsCode                                 */
/*+
  Funzione che restituisce il codice della prima classe griglia estratta
  (in ordine alfabetico).

  La funzione restituisce il codice della classe in caso di successo altrimenti 0.  
-*/
/*************************************************************************/
int gsui_getFirstExtractedGridClsCode(void)
{
   int Cls, Sub;
   C_STRING CategoryFilter, TypeFilter;

   CategoryFilter = CAT_GRID;
   if (gsui_getFirstExtractedClsSubCode(&Cls, &Sub, CategoryFilter, TypeFilter) == GS_BAD) return 0;

   return Cls;
}


/*************************************************************************/
/*.doc gsui_GridSetFlowDirection                                         */
/*+
  Comando per calcolare, per ciascuna cella della zona definita dall'utente,
  la cella a valle in cui l'acqua scola.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridSetFlowDirection(void)
{
   INT_PTR      ret;
   CGridFlowDlg GridFlowDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	if ((ret = GridFlowDlg.DoModal()) == IDWINDOWSEL)
      { // Selezione di una finestra
         C_RB_LIST CoordList;
         
         // Evidenzio la zona
         if (gsc_define_window(CoordList) == GS_GOOD)
         {
            gsc_rb2Pt(CoordList.getptr_at(1), GridFlowDlg.pt1);
            gsc_rb2Pt(CoordList.getptr_at(2), GridFlowDlg.pt2);
         }
      }
   }
   while (ret == IDWINDOWSEL);

   if (ret == IDOK)
   {
      C_CGRID *pGrid;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(GridFlowDlg.Cls)))
         return GS_BAD;
      if (pGrid->SetHydrologyFlow(GridFlowDlg.ZAttrib, GridFlowDlg.pt1, GridFlowDlg.pt2,
                                  GridFlowDlg.FlowAttrib) == GS_BAD)
         return GS_BAD;
   }

   return GS_GOOD;  
}


///////////////////////////////////////////////////////////////////////////////
// Inizio finestra di dialogo CGridFlowDlg
///////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC(CGridFlowDlg, CDialog)
CGridFlowDlg::CGridFlowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridFlowDlg::IDD, pParent)
{
   Cls = 0;
   pt1[X] = pt1[Y] = pt1[Z] = 0.0;
   ads_point_set(pt1, pt2);
}

CGridFlowDlg::~CGridFlowDlg()
{
}

void CGridFlowDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_GRID_GRIDAREATYPE, m_SelAreaType);
   DDX_Control(pDX, IDC_Z_ATTRIBUTE, m_ZAttrib);
   DDX_Control(pDX, IDC_DIRECTION_ATTRIBUTE, m_FlowAttrib);
}


BEGIN_MESSAGE_MAP(CGridFlowDlg, CDialog)
   ON_BN_CLICKED(IDHELP, OnBnClickedHelp)
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
   ON_CBN_SELCHANGE(IDC_GRID_GRIDAREATYPE, OnCbnSelchangeGridGridareatype)
   ON_CBN_SELCHANGE(IDC_Z_ATTRIBUTE, OnCbnSelchangeZAttribute)
   ON_CBN_SELCHANGE(IDC_DIRECTION_ATTRIBUTE, OnCbnSelchangeDirectionAttribute)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGridFlowDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()


BOOL CGridFlowDlg::OnInitDialog() 
{
   C_STRING Mask;

	CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte
   m_GeoTree.FilterOnUpdateable = true; // solo quelle modificabili

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   m_SelAreaType.AddString(_T("Estensioni"));
   m_SelAreaType.AddString(_T("Finestra"));
   if (ads_point_equal(pt1, pt2))
      m_SelAreaType.SetCurSel(Extension);
   else
      m_SelAreaType.SetCurSel(Window);

   RefreshAttribList();

   int Pos = m_ZAttribNameList.getpos_name(ZAttrib.get_name(), FALSE);
   if (Pos > 0) m_ZAttrib.SetCurSel(Pos - 1);

   Pos = m_FlowAttribNameList.getpos_name(FlowAttrib.get_name(), FALSE);
   if (Pos > 0) m_FlowAttrib.SetCurSel(Pos - 1);
   
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// gestori di messaggi CGridFlowDlg

void CGridFlowDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Calcolodelladirezionedelflusso);
}

void CGridFlowDlg::OnBnClickedOk()
{
   if (Cls == 0)
   {
      gsui_alert(_T("Scegliere una classe."));
      return;
   }

   if (ZAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo da cui leggere le quote."));
      return;
   }

   if (FlowAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo in cui memorizzare la direzione del flusso."));
      return;
   }

   OnOK();
}

void CGridFlowDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;

   m_ZAttribNameList.remove_all();
   m_FlowAttribNameList.remove_all();
   while (m_ZAttrib.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_FlowAttrib.DeleteString(0) != CB_ERR); // svuoto la combo

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto gli attributi non numerici
      // Salto l'attributo chiave
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD &&
          pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
      {
         // Salto gli attributi calcolati per assegnare il flusso
         if (pAttrib->is_calculated() == GS_BAD)
         {
            m_FlowAttrib.AddString(pAttrib->name.get_name());
            m_FlowAttribNameList.add_tail_str(pAttrib->name.get_name());
         }

         m_ZAttrib.AddString(pAttrib->name.get_name());
         m_ZAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_ZAttribNameList.sort_name(); // li ordino in modo crescente
   m_FlowAttribNameList.sort_name(); // li ordino in modo crescente
}

void CGridFlowDlg::OnCbnSelchangeGridGridareatype()
{
   if (m_SelAreaType.GetCurSel() == Window)
      EndDialog(IDWINDOWSEL); // This value is returned by DoModal!
}


void CGridFlowDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls   = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();

   *pResult = 0;
}

void CGridFlowDlg::OnCbnSelchangeZAttribute()
{
   if (m_ZAttrib.GetCurSel() == CB_ERR) return;
   ZAttrib = m_ZAttribNameList.getptr_at(m_ZAttrib.GetCurSel() + 1)->get_name();
}

void CGridFlowDlg::OnCbnSelchangeDirectionAttribute()
{
   if (m_FlowAttrib.GetCurSel() == CB_ERR) return;
   FlowAttrib = m_FlowAttribNameList.getptr_at(m_FlowAttrib.GetCurSel() + 1)->get_name();
}


//**********************************************************
// FINE CALCOLO DIREZIONE FLUSSO ACQUA
// INIZIO GRAPE
//**********************************************************


/*************************************************************************/
/*.doc gsui_GridDrape                                                    */
/*+
  Comando che sposta degli oggetti "adagiandoli" sulla griglia.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridDrape(void)
{
   INT_PTR       ret;
   CGridDrapeDlg GridDrapeDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	if ((ret = GridDrapeDlg.DoModal()) == IDOBJSSEL)
      { // Selezione di una finestra
         gsc_ssget(NULL, NULL, NULL, NULL, GridDrapeDlg.SelSet);
         GridDrapeDlg.AutoSel = (GridDrapeDlg.SelSet.length() > 0) ? false : true;
      }
   }
   while (ret == IDOBJSSEL);

   if (ret == IDOK)
   {
      C_CGRID *pGrid;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(GridDrapeDlg.Cls)))
         return GS_BAD;

      if (GridDrapeDlg.AutoSel)
      {
         if (pGrid->ObjectOnGrid(GridDrapeDlg.AllObjs, GridDrapeDlg.ZAttrib) == GS_BAD)
            return GS_BAD;
      }
      else
         if (pGrid->ObjectOnGrid(GridDrapeDlg.SelSet, GridDrapeDlg.ZAttrib) == GS_BAD)
            return GS_BAD;
   }

   return GS_GOOD;  
}


IMPLEMENT_DYNAMIC(CGridDrapeDlg, CDialog)
CGridDrapeDlg::CGridDrapeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridDrapeDlg::IDD, pParent)
{
   hBmpSelObjs = NULL;

   Cls     = 0;
   AutoSel = true;
   gsc_ssget(_T("_X"), NULL, NULL, NULL, AllObjs);
}

CGridDrapeDlg::~CGridDrapeDlg()
{
   if (hBmpSelObjs) DeleteObject((HGDIOBJ) hBmpSelObjs);
}

void CGridDrapeDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_Z_ATTRIBUTE, m_ZAttrib);
   DDX_Control(pDX, IDC_BUTTON_OBJSSELECTION, m_ObjsSelection);
   DDX_Control(pDX, IDC_N_SELECTED, mLbl_nSelected);
   DDX_Control(pDX, IDC_RADIO_SELECTALL, m_AutoSelection);
   DDX_Control(pDX, IDC_RADIO_MANUALSELECTION, m_ManualSelection);
}


BEGIN_MESSAGE_MAP(CGridDrapeDlg, CDialog)
   ON_BN_CLICKED(IDC_RADIO_SELECTALL, OnBnClickedRadioSelectall)
   ON_BN_CLICKED(IDC_RADIO_MANUALSELECTION, OnBnClickedRadioManualselection)
   ON_BN_CLICKED(IDC_BUTTON_OBJSSELECTION, OnBnClickedButtonObjsselection)
   ON_CBN_SELCHANGE(IDC_Z_ATTRIBUTE, OnCbnSelchangeZAttribute)
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, OnBnClickedHelp)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGridDrapeDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()


BOOL CGridDrapeDlg::OnInitDialog() 
{
   COLORREF  crFrom;
	HINSTANCE Instance;
   C_STRING  Mask;

   CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   crFrom = RGB(255, 0, 0); // rosso
   // determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_SELECT_OBJS), RT_BITMAP);
   hBmpSelObjs = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SELECT_OBJS));
   gsui_SetBmpColorToDlgBkColor(hBmpSelObjs, crFrom);
   m_ObjsSelection.SetBitmap(hBmpSelObjs);

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   RefreshSelection();
   RefreshAttribList();

   int Pos = m_ZAttribNameList.getpos_name(ZAttrib.get_name(), FALSE);
   if (Pos > 0) m_ZAttrib.SetCurSel(Pos - 1); 

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// gestori di messaggi CGridDrapeDlg

void CGridDrapeDlg::OnBnClickedRadioSelectall()
{
   AutoSel = true;
   RefreshSelection();
}

void CGridDrapeDlg::OnBnClickedRadioManualselection()
{
   AutoSel = false;
   RefreshSelection();
}

void CGridDrapeDlg::OnBnClickedButtonObjsselection()
{
   EndDialog(IDOBJSSEL); // This value is returned by DoModal!
}

void CGridDrapeDlg::OnBnClickedOk()
{
   if (Cls == 0)
   {
      gsui_alert(_T("Scegliere una classe."));
      return;
   }

   if (ZAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo da cui leggere le quote."));
      return;
   }

   OnOK();
}

void CGridDrapeDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Drape);
}

void CGridDrapeDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;

   m_ZAttribNameList.remove_all();
   while (m_ZAttrib.DeleteString(0) != CB_ERR); // svuoto la combo

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto gli attributi non numerici
      // Salto l'attributo chiave
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD &&
          pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
      {
         m_ZAttrib.AddString(pAttrib->name.get_name());
         m_ZAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_ZAttribNameList.sort_name(); // li ordino in modo crescente
}

void CGridDrapeDlg::RefreshSelection()
{
   long     n;
   C_STRING Msg; 

   if (AutoSel == true)
   {
      n = AllObjs.length();
      m_AutoSelection.SetCheck(BST_CHECKED);
      m_ManualSelection.SetCheck(BST_UNCHECKED);
   }
   else
   {
      n = SelSet.length();
      m_AutoSelection.SetCheck(BST_UNCHECKED);
      m_ManualSelection.SetCheck(BST_CHECKED);
   }

   Msg = n;
   Msg += _T(" oggetti selezionati");
   mLbl_nSelected.SetWindowText(Msg.get_name());
}

void CGridDrapeDlg::OnCbnSelchangeZAttribute()
{
   if (m_ZAttrib.GetCurSel() == CB_ERR) return;
   ZAttrib = m_ZAttribNameList.getptr_at(m_ZAttrib.GetCurSel() + 1)->get_name();
}


void CGridDrapeDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();

   *pResult = 0;
}


//**********************************************************
// FINE GRAPE
// INIZIO INTERPOLAZIONE IDW
//**********************************************************


/*************************************************************************/
/*.doc gsui_GridSpatialInterpolationIDW                                  */
/*+
  Comando per calcolare, per ciascuna cella della zona definita dall'utente,
  il valore di un attributo non ancora valorizzato in base all'algoritmo
  IDW di interpolazione spaziale.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridSpatialInterpolationIDW(void)
{
   INT_PTR ret;
   CGriDSpatialInterpolationIDWDlg SpatialInterpolationIDWDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	ret = SpatialInterpolationIDWDlg.DoModal();
   	if (ret == IDWINDOWSEL)
      { // Selezione di una finestra
         C_RB_LIST CoordList;
         
         // Evidenzio la zona
         if (gsc_define_window(CoordList) == GS_GOOD)
         {
            gsc_rb2Pt(CoordList.getptr_at(1), SpatialInterpolationIDWDlg.pt1);
            gsc_rb2Pt(CoordList.getptr_at(2), SpatialInterpolationIDWDlg.pt2);
         }
      }
      else
         if (ret == IDRAYSEL)
         { // Selezione di un raggio
            C_STRING Msg;
            double   Dist;

            Msg = _T("\nLunghezza raggio <");
            Msg += SpatialInterpolationIDWDlg.Ray;
            Msg += _T(">: ");
            acedInitGet(RSG_OTHER, NULL);
            if (acedGetDist(NULL, Msg.get_name(), &Dist) == RTNORM)
               SpatialInterpolationIDWDlg.Ray = Dist;
         }
   }
   while (ret == IDWINDOWSEL || ret == IDRAYSEL);

   if (ret == IDOK)
   {
      C_CGRID  *pGrid;
      C_STRING *pBarrierAttrib = NULL;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(SpatialInterpolationIDWDlg.Cls)))
         return GS_BAD;
      
      if (SpatialInterpolationIDWDlg.BarrierAttrib.len() > 0) 
         pBarrierAttrib = &(SpatialInterpolationIDWDlg.BarrierAttrib);

      if (gsc_point_equal(SpatialInterpolationIDWDlg.pt1, SpatialInterpolationIDWDlg.pt2))
      {
         if (pGrid->SpatialInterpolationIDW(SpatialInterpolationIDWDlg.Attrib, 
                                            SpatialInterpolationIDWDlg.Ray,
                                            SpatialInterpolationIDWDlg.MinKnownPts,
                                            SpatialInterpolationIDWDlg.Quadratic,
                                            NULL, NULL,
                                            SpatialInterpolationIDWDlg.Recursive,
                                            pBarrierAttrib) == GS_BAD)
         return GS_BAD;
      }
      else
         if (pGrid->SpatialInterpolationIDW(SpatialInterpolationIDWDlg.Attrib, 
                                            SpatialInterpolationIDWDlg.Ray,
                                            SpatialInterpolationIDWDlg.MinKnownPts,
                                            SpatialInterpolationIDWDlg.Quadratic,
                                            SpatialInterpolationIDWDlg.pt1,
                                            SpatialInterpolationIDWDlg.pt2,
                                            SpatialInterpolationIDWDlg.Recursive,
                                            pBarrierAttrib) == GS_BAD)
         return GS_BAD;
   }

   return GS_GOOD;  
}


// CGriDSpatialInterpolationIDWDlg dialog

IMPLEMENT_DYNAMIC(CGriDSpatialInterpolationIDWDlg, CDialog)

CGriDSpatialInterpolationIDWDlg::CGriDSpatialInterpolationIDWDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGriDSpatialInterpolationIDWDlg::IDD, pParent)
{
   hBmpSelRay = NULL;

   Cls = 0;
   pt1[X] = pt1[Y] = pt1[Z] = 0.0;
   ads_point_set(pt1, pt2);
   m_insideRayToggle = false;
   Ray = 0.0;
   MinKnownPts = 3;
   Quadratic = false;
   Recursive = true;
}

CGriDSpatialInterpolationIDWDlg::~CGriDSpatialInterpolationIDWDlg()
{
   if (hBmpSelRay) DeleteObject((HGDIOBJ) hBmpSelRay);
}

void CGriDSpatialInterpolationIDWDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_GRID_GRIDAREATYPE, m_SelAreaType);
   DDX_Control(pDX, IDC_ATTRIBUTE, m_Attrib);
   DDX_Control(pDX, IDC_BARRIER_ATTRIBUTE, m_BarrierAttrib);
   DDX_Control(pDX, IDC_INSIDE_RAY, m_InsideRay);
   DDX_Control(pDX, IDC_RAY, m_Ray);
   DDX_Control(pDX, IDC_RAY_BUTTON, m_SelRayButton);
   DDX_Control(pDX, IDC_MIN_PTS, m_MinKnownPts);
   DDX_Control(pDX, IDC_INVERSE_DIST_RADIO, m_InverseDistance);
   DDX_Control(pDX, IDC_QUADRATIC_INVERSE_DIST_RADIO, m_QuadraticInverseDistance);
   DDX_Control(pDX, IDC_RECURSIVE, m_Recursive);
}


BEGIN_MESSAGE_MAP(CGriDSpatialInterpolationIDWDlg, CDialog)
   ON_BN_CLICKED(IDOK, &CGriDSpatialInterpolationIDWDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGriDSpatialInterpolationIDWDlg::OnBnClickedHelp)
   ON_CBN_SELCHANGE(IDC_GRID_GRIDAREATYPE, &CGriDSpatialInterpolationIDWDlg::OnCbnSelchangeGridGridareatype)
   ON_BN_CLICKED(IDC_INSIDE_RAY, &CGriDSpatialInterpolationIDWDlg::OnBnClickedInsideRay)
   ON_BN_CLICKED(IDC_INVERSE_DIST_RADIO, &CGriDSpatialInterpolationIDWDlg::OnBnClickedInverseDistRadio)
   ON_BN_CLICKED(IDC_QUADRATIC_INVERSE_DIST_RADIO, &CGriDSpatialInterpolationIDWDlg::OnBnClickedQuadraticInverseDistRadio)
   ON_BN_CLICKED(IDC_RAY_BUTTON, &CGriDSpatialInterpolationIDWDlg::OnBnClickedRayButton)
   ON_EN_KILLFOCUS(IDC_RAY, &CGriDSpatialInterpolationIDWDlg::OnEnKillfocusRay)
   ON_EN_KILLFOCUS(IDC_MIN_PTS, &CGriDSpatialInterpolationIDWDlg::OnEnKillfocusMinPts)
   ON_BN_CLICKED(IDC_RECURSIVE, &CGriDSpatialInterpolationIDWDlg::OnBnClickedRecursive)
   ON_CBN_SELCHANGE(IDC_ATTRIBUTE, &CGriDSpatialInterpolationIDWDlg::OnCbnSelchangeAttribute)
   ON_CBN_SELCHANGE(IDC_BARRIER_ATTRIBUTE, &CGriDSpatialInterpolationIDWDlg::OnCbnSelchangeBarrierAttribute)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGriDSpatialInterpolationIDWDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()

BOOL CGriDSpatialInterpolationIDWDlg::OnInitDialog() 
{
   C_STRING  Mask;
   COLORREF  crFrom;
	HINSTANCE Instance;

	CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   crFrom = RGB(255, 0, 0); // rosso
   // determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_RAY), RT_BITMAP);
   hBmpSelRay = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_RAY));
   gsui_SetBmpColorToDlgBkColor(hBmpSelRay, crFrom);
   m_SelRayButton.SetBitmap(hBmpSelRay);

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte
   m_GeoTree.FilterOnUpdateable = true; // solo quelle modificabili

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   m_SelAreaType.AddString(_T("Estensioni"));
   m_SelAreaType.AddString(_T("Finestra"));
   if (ads_point_equal(pt1, pt2))
      m_SelAreaType.SetCurSel(Extension);
   else
      m_SelAreaType.SetCurSel(Window);

   RefreshAttribList();

   int Pos = m_AttribNameList.getpos_name(Attrib.get_name(), FALSE);
   if (Pos > 0) m_Attrib.SetCurSel(Pos - 1);

   Pos = m_BarrierAttribNameList.getpos_name(BarrierAttrib.get_name(), FALSE);
   if (Pos > 0) m_BarrierAttrib.SetCurSel(Pos - 1);
   
   RefreshSelection();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGriDSpatialInterpolationIDWDlg::RefreshSelection()
{
   C_STRING Msg; 

   if (m_insideRayToggle)
   {
      m_InsideRay.SetCheck(BST_CHECKED);
      m_Ray.EnableWindow(TRUE);
      m_SelRayButton.EnableWindow(TRUE);
   }
   else
   {
      m_InsideRay.SetCheck(BST_UNCHECKED);
      m_Ray.EnableWindow(FALSE);
      m_SelRayButton.EnableWindow(FALSE);
   }
   Msg = Ray;
   m_Ray.SetWindowTextW(Msg.get_name());

   Msg = MinKnownPts;
   m_MinKnownPts.SetWindowTextW(Msg.get_name());
      
   if (Quadratic == false)
   {
      m_InverseDistance.SetCheck(BST_CHECKED);
      m_QuadraticInverseDistance.SetCheck(BST_UNCHECKED);
   }
   else
   {
      m_InverseDistance.SetCheck(BST_UNCHECKED);
      m_QuadraticInverseDistance.SetCheck(BST_CHECKED);
   }
   
   if (Recursive) m_Recursive.SetCheck(BST_CHECKED);
   else m_Recursive.SetCheck(BST_UNCHECKED);
}

// CGriDSpatialInterpolationIDWDlg message handlers

void CGriDSpatialInterpolationIDWDlg::OnBnClickedOk()
{
   if (Cls == 0)
   {
      gsui_alert(_T("Scegliere una classe griglia."));
      return;
   }

   if (Attrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo da calcolare."));
      return;
   }

   if (Ray < 0)
   {
      gsui_alert(_T("Il raggio non può essere negativo."));
      return;
   }
   else
      if (Ray == 0 && MinKnownPts == 0)
      {
         gsui_alert(_T("Se non si è impostato un raggio prefissato è necessario indicare un numero minimo di celle valide."));
         return;
      }


   OnOK();
}

void CGriDSpatialInterpolationIDWDlg::OnBnClickedHelp()
{
   gsc_help(IDH_InterpolazionespazialeIDWInverseDistanceWeighted);
}

void CGriDSpatialInterpolationIDWDlg::OnCbnSelchangeGridGridareatype()
{
   if (m_SelAreaType.GetCurSel() == Window)
      EndDialog(IDWINDOWSEL); // This value is returned by DoModal!
}

void CGriDSpatialInterpolationIDWDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;

   m_AttribNameList.remove_all();
   m_BarrierAttribNameList.remove_all();
   while (m_Attrib.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_BarrierAttrib.DeleteString(0) != CB_ERR); // svuoto la combo

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   m_BarrierAttribNameList.add_tail_str(_T("")); // prima riga vuota
   m_BarrierAttrib.AddString(_T(""));

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto gli attributi non numerici
      // Salto l'attributo chiave
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD &&
          pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
      {
         // Salto gli attributi calcolati per assegnare il valore
         if (pAttrib->is_calculated() == GS_BAD)
         {
            m_Attrib.AddString(pAttrib->name.get_name());
            m_AttribNameList.add_tail_str(pAttrib->name.get_name());
         }

         m_BarrierAttrib.AddString(pAttrib->name.get_name());
         m_BarrierAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_AttribNameList.sort_name(); // li ordino in modo crescente
   m_BarrierAttribNameList.sort_name(); // li ordino in modo crescente
}

void CGriDSpatialInterpolationIDWDlg::OnBnClickedInsideRay()
{
   m_insideRayToggle = (m_InsideRay.GetCheck() == BST_CHECKED) ? true : false;
   RefreshSelection();
}

void CGriDSpatialInterpolationIDWDlg::OnBnClickedInverseDistRadio()
{
   Quadratic = (m_InverseDistance.GetCheck() == BST_CHECKED) ? false : true;
   RefreshSelection();
}

void CGriDSpatialInterpolationIDWDlg::OnBnClickedQuadraticInverseDistRadio()
{
   Quadratic = (m_QuadraticInverseDistance.GetCheck() == BST_CHECKED) ? true : false;
   RefreshSelection();
}

void CGriDSpatialInterpolationIDWDlg::OnBnClickedRayButton()
{
   EndDialog(IDRAYSEL); // This value is returned by DoModal!
}

void CGriDSpatialInterpolationIDWDlg::OnEnKillfocusRay()
{
   CString dummy;
   C_STRING Msg;

   m_Ray.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   if (Msg.tof() > 0) Ray = Msg.tof();
   else Ray = 0.0;

   RefreshSelection();
}

void CGriDSpatialInterpolationIDWDlg::OnEnKillfocusMinPts()
{
   CString dummy;
   C_STRING Msg;

   m_MinKnownPts.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   if (Msg.tof() > 0) MinKnownPts = Msg.toi();
   else MinKnownPts = 0;
   RefreshSelection();
}

void CGriDSpatialInterpolationIDWDlg::OnBnClickedRecursive()
{
   Recursive = (m_Recursive.GetCheck() == BST_CHECKED) ? true : false;
   RefreshSelection();
}

void CGriDSpatialInterpolationIDWDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls   = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();

   *pResult = 0;
}

void CGriDSpatialInterpolationIDWDlg::OnCbnSelchangeAttribute()
{
   if (m_Attrib.GetCurSel() == CB_ERR) return;
   Attrib = m_AttribNameList.getptr_at(m_Attrib.GetCurSel() + 1)->get_name();
}

void CGriDSpatialInterpolationIDWDlg::OnCbnSelchangeBarrierAttribute()
{
   if (m_BarrierAttrib.GetCurSel() == CB_ERR) return;
   BarrierAttrib = m_BarrierAttribNameList.getptr_at(m_BarrierAttrib.GetCurSel() + 1)->get_name();
}


//**********************************************************
// FINE INTERPOLAZIONE IDW
// INIZIO CONTEGGIO CELLE A MONTE
//**********************************************************


/*************************************************************************/
/*.doc gsui_GridHydrologyCountUpstreamCells                                  */
/*+
  Comando per calcolare, per ciascuna cella della zona definita dall'utente,
  quante sono le celle a monte.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridHydrologyCountUpstreamCells(void)
{
   INT_PTR ret;
   CGriDHydrologyCountUpstreamCellsDlg HydrologyCountUpstreamCellsDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	ret = HydrologyCountUpstreamCellsDlg.DoModal();
   	if (ret == IDWINDOWSEL)
      { // Selezione di una finestra
         C_RB_LIST CoordList;
         
         // Evidenzio la zona
         if (gsc_define_window(CoordList) == GS_GOOD)
         {
            gsc_rb2Pt(CoordList.getptr_at(1), HydrologyCountUpstreamCellsDlg.pt1);
            gsc_rb2Pt(CoordList.getptr_at(2), HydrologyCountUpstreamCellsDlg.pt2);
         }
      }
   }
   while (ret == IDWINDOWSEL);

   if (ret == IDOK)
   {
      C_CGRID  *pGrid;
      C_STRING *pBarrierAttrib = NULL;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(HydrologyCountUpstreamCellsDlg.Cls)))
         return GS_BAD;
      
      if (gsc_point_equal(HydrologyCountUpstreamCellsDlg.pt1, HydrologyCountUpstreamCellsDlg.pt2))
      {
         if (pGrid->SetHydrologyCountUpstreamCells(HydrologyCountUpstreamCellsDlg.FlowAttrib,
                                                   NULL, NULL,
                                                   HydrologyCountUpstreamCellsDlg.OutputAttrib) == GS_BAD)
            return GS_BAD;
      }
      else
         if (pGrid->SetHydrologyCountUpstreamCells(HydrologyCountUpstreamCellsDlg.FlowAttrib,
                                                   HydrologyCountUpstreamCellsDlg.pt1,
                                                   HydrologyCountUpstreamCellsDlg.pt2,
                                                   HydrologyCountUpstreamCellsDlg.OutputAttrib) == GS_BAD)
            return GS_BAD;
   }

   return GS_GOOD;  
}

// CGriDHydrologyCountUpstreamCellsDlg dialog

IMPLEMENT_DYNAMIC(CGriDHydrologyCountUpstreamCellsDlg, CDialog)

CGriDHydrologyCountUpstreamCellsDlg::CGriDHydrologyCountUpstreamCellsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGriDHydrologyCountUpstreamCellsDlg::IDD, pParent)
{
   Cls = 0;
   pt1[X] = pt1[Y] = pt1[Z] = 0.0;
   ads_point_set(pt1, pt2);
}

CGriDHydrologyCountUpstreamCellsDlg::~CGriDHydrologyCountUpstreamCellsDlg()
{
}

void CGriDHydrologyCountUpstreamCellsDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_GRID_GRIDAREATYPE, m_SelAreaType);
   DDX_Control(pDX, IDC_DIRECTION_ATTRIBUTE, m_FlowAttrib);
   DDX_Control(pDX, IDC_OUTPUT_ATTRIBUTE, m_OutputAttrib);
}


BEGIN_MESSAGE_MAP(CGriDHydrologyCountUpstreamCellsDlg, CDialog)
   ON_CBN_SELCHANGE(IDC_GRID_GRIDAREATYPE, &CGriDHydrologyCountUpstreamCellsDlg::OnCbnSelchangeGridGridareatype)
   ON_CBN_SELCHANGE(IDC_DIRECTION_ATTRIBUTE, &CGriDHydrologyCountUpstreamCellsDlg::OnCbnSelchangeAttribute)
   ON_CBN_SELCHANGE(IDC_OUTPUT_ATTRIBUTE, &CGriDHydrologyCountUpstreamCellsDlg::OnCbnSelchangeOutputAttribute)
   ON_BN_CLICKED(IDOK, &CGriDHydrologyCountUpstreamCellsDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGriDHydrologyCountUpstreamCellsDlg::OnBnClickedHelp)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGriDHydrologyCountUpstreamCellsDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()


void CGriDHydrologyCountUpstreamCellsDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls   = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();

   *pResult = 0;
}

BOOL CGriDHydrologyCountUpstreamCellsDlg::OnInitDialog() 
{
   C_STRING Mask;

	CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte
   m_GeoTree.FilterOnUpdateable = true; // solo quelle modificabili

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   m_SelAreaType.AddString(_T("Estensioni"));
   m_SelAreaType.AddString(_T("Finestra"));
   if (ads_point_equal(pt1, pt2))
      m_SelAreaType.SetCurSel(Extension);
   else
      m_SelAreaType.SetCurSel(Window);

   RefreshAttribList();

   int Pos = m_FlowAttribNameList.getpos_name(FlowAttrib.get_name(), FALSE);
   if (Pos > 0) m_FlowAttrib.SetCurSel(Pos - 1);

   Pos = m_OutputAttribNameList.getpos_name(OutputAttrib.get_name(), FALSE);
   if (Pos > 0) m_OutputAttrib.SetCurSel(Pos - 1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGriDHydrologyCountUpstreamCellsDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;

   m_FlowAttribNameList.remove_all();
   m_OutputAttribNameList.remove_all();
   while (m_FlowAttrib.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_OutputAttrib.DeleteString(0) != CB_ERR); // svuoto la combo

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto gli attributi non numerici
      // Salto l'attributo chiave
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD &&
          pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
      {
         // Salto gli attributi calcolati per assegnare il flusso
         if (pAttrib->is_calculated() == GS_BAD)
         {
            m_OutputAttrib.AddString(pAttrib->name.get_name());
            m_OutputAttribNameList.add_tail_str(pAttrib->name.get_name());
         }

         m_FlowAttrib.AddString(pAttrib->name.get_name());
         m_FlowAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_FlowAttribNameList.sort_name();   // li ordino in modo crescente
   m_OutputAttribNameList.sort_name(); // li ordino in modo crescente
}

void CGriDHydrologyCountUpstreamCellsDlg::OnCbnSelchangeGridGridareatype()
{
   if (m_SelAreaType.GetCurSel() == Window)
      EndDialog(IDWINDOWSEL); // This value is returned by DoModal!
}

void CGriDHydrologyCountUpstreamCellsDlg::OnCbnSelchangeAttribute()
{
   if (m_FlowAttrib.GetCurSel() == CB_ERR) return;
   FlowAttrib = m_FlowAttribNameList.getptr_at(m_FlowAttrib.GetCurSel() + 1)->get_name();
}

void CGriDHydrologyCountUpstreamCellsDlg::OnCbnSelchangeOutputAttribute()
{
   if (m_OutputAttrib.GetCurSel() == CB_ERR) return;
   OutputAttrib = m_OutputAttribNameList.getptr_at(m_OutputAttrib.GetCurSel() + 1)->get_name();
}

void CGriDHydrologyCountUpstreamCellsDlg::OnBnClickedOk()
{
   if (Cls == 0)
   {
      gsui_alert(_T("Scegliere una classe griglia."));
      return;
   }

   if (FlowAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere l'attributo contenente la direzione del flusso."));
      return;
   }

   if (OutputAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo per memorizzare il conteggio."));
      return;
   }

   OnOK();
}

void CGriDHydrologyCountUpstreamCellsDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Conteggiodellecelleamonte);
}


//**********************************************************
// FINE CONTEGGIO CELLE A MONTE
// INIZIO RICERCA BACINO A MONTE O A VALLE
//**********************************************************


/*************************************************************************/
/*.doc gsui_GridGetCatchmentAreaCells                                    */
/*+
  Comando per calcolare, da una cella nota il bacino delle celle a monte
  o a valle.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridGetCatchmentAreaCells(void)
{
   INT_PTR ret;
   CGridGetCatchmentAreaCellsDlg GetCatchmentAreaCellsDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	ret = GetCatchmentAreaCellsDlg.DoModal();
   	if (ret == IDPOINTSEL)
      {  // Selezione di una punto
         ads_point dummyPt;
     
         if (acedGetPoint(NULL, _T("\nSelezionare un punto: "), dummyPt) == RTNORM)
            ads_point_set(dummyPt, GetCatchmentAreaCellsDlg.pt);
      }
   }
   while (ret == IDPOINTSEL);

   if (ret == IDOK)
   {
      C_CGRID    *pGrid;
      long       key;
      C_LINK_SET ResultLS;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(GetCatchmentAreaCellsDlg.Cls)))
         return GS_BAD;

      pGrid->ptr_grid()->pt2key(GetCatchmentAreaCellsDlg.pt, &key);
      if (GetCatchmentAreaCellsDlg.DirectionDown) // verso valle
      {
         if (pGrid->GetFlowDownCells(key, GetCatchmentAreaCellsDlg.FlowAttrib, ResultLS) == GS_BAD)
            return GS_BAD;
      }
      else // verso monte
         if (pGrid->GetCatchmentAreaCells(key, GetCatchmentAreaCellsDlg.FlowAttrib, ResultLS) == GS_BAD)
            return GS_BAD;

      set_GS_LSFILTER(&(pGrid->ptr_id()->code), &(pGrid->ptr_id()->sub_code),
                      ResultLS.ptr_SelSet(), ResultLS.ptr_KeyList());
   }

   return GS_GOOD;  
}


// CGridGetCatchmentAreaCellsDlg dialog

IMPLEMENT_DYNAMIC(CGridGetCatchmentAreaCellsDlg, CDialog)

CGridGetCatchmentAreaCellsDlg::CGridGetCatchmentAreaCellsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridGetCatchmentAreaCellsDlg::IDD, pParent)
{
   hBmpSelPt = NULL;

   Cls = 0;
   pt[X] = pt[Y] = pt[Z] = 0.0;
   DirectionDown = false; // ricerca verso monte
}

CGridGetCatchmentAreaCellsDlg::~CGridGetCatchmentAreaCellsDlg()
{
   if (hBmpSelPt) DeleteObject((HGDIOBJ) hBmpSelPt);
}

void CGridGetCatchmentAreaCellsDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_POINT_SELECTION, m_PtSelection);
   DDX_Control(pDX, IDC_COORDX_EDIT, m_X);
   DDX_Control(pDX, IDC_COORDY_EDIT, m_Y);
   DDX_Control(pDX, IDC_DIRECTION_ATTRIBUTE, m_FlowAttrib);
   DDX_Control(pDX, IDC_DIRECTION_DOWN_RADIO, m_DirectionDown);
   DDX_Control(pDX, IDC_DIRECTION_UP_RADIO, m_DirectionUp);
}


BEGIN_MESSAGE_MAP(CGridGetCatchmentAreaCellsDlg, CDialog)
   ON_BN_CLICKED(IDC_POINT_SELECTION, &CGridGetCatchmentAreaCellsDlg::OnBnClickedPointSelection)
   ON_CBN_SELCHANGE(IDC_DIRECTION_ATTRIBUTE, &CGridGetCatchmentAreaCellsDlg::OnCbnSelchangeDirectionAttribute)
   ON_BN_CLICKED(IDOK, &CGridGetCatchmentAreaCellsDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGridGetCatchmentAreaCellsDlg::OnBnClickedHelp)
   ON_EN_KILLFOCUS(IDC_COORDX_EDIT, &CGridGetCatchmentAreaCellsDlg::OnEnKillfocusCoordxEdit)
   ON_EN_KILLFOCUS(IDC_COORDY_EDIT, &CGridGetCatchmentAreaCellsDlg::OnEnKillfocusCoordyEdit)
   ON_BN_CLICKED(IDC_DIRECTION_DOWN_RADIO, &CGridGetCatchmentAreaCellsDlg::OnBnClickedDirectionDownRadio)
   ON_BN_CLICKED(IDC_DIRECTION_UP_RADIO, &CGridGetCatchmentAreaCellsDlg::OnBnClickedDirectionUpRadio)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGridGetCatchmentAreaCellsDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()


BOOL CGridGetCatchmentAreaCellsDlg::OnInitDialog() 
{
   COLORREF  crFrom;
	HINSTANCE Instance;
   C_STRING  Mask;

	CDialog::OnInitDialog();

   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   crFrom = RGB(255, 0, 0); // rosso
   // determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_SELECT_OBJS), RT_BITMAP);
   hBmpSelPt = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SELECT_OBJS));
   gsui_SetBmpColorToDlgBkColor(hBmpSelPt, crFrom);
   m_PtSelection.SetBitmap(hBmpSelPt);

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   RefreshSelection();
   RefreshAttribList();

   int Pos = m_FlowAttribNameList.getpos_name(FlowAttrib.get_name(), FALSE);
   if (Pos > 0) m_FlowAttrib.SetCurSel(Pos - 1); 

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGridGetCatchmentAreaCellsDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;

   m_FlowAttribNameList.remove_all();
   while (m_FlowAttrib.DeleteString(0) != CB_ERR); // svuoto la combo

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto gli attributi non numerici
      // Salto l'attributo chiave
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD &&
          pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
      {
         m_FlowAttrib.AddString(pAttrib->name.get_name());
         m_FlowAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_FlowAttribNameList.sort_name(); // li ordino in modo crescente
}

void CGridGetCatchmentAreaCellsDlg::RefreshSelection()
{
   C_STRING Msg; 

   Msg = pt[X];
   m_X.SetWindowText(Msg.get_name());
   Msg = pt[Y];
   m_Y.SetWindowText(Msg.get_name());

   if (DirectionDown) // verso valle
   {
      m_DirectionDown.SetCheck(BST_CHECKED);
      m_DirectionUp.SetCheck(BST_UNCHECKED);
   }
   else // verso monte
   {
      m_DirectionDown.SetCheck(BST_UNCHECKED);
      m_DirectionUp.SetCheck(BST_CHECKED);
   }
}


void CGridGetCatchmentAreaCellsDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();

   *pResult = 0;
}


void CGridGetCatchmentAreaCellsDlg::OnBnClickedPointSelection()
{
   EndDialog(IDPOINTSEL); // This value is returned by DoModal!
}

void CGridGetCatchmentAreaCellsDlg::OnCbnSelchangeDirectionAttribute()
{
   if (m_FlowAttrib.GetCurSel() == CB_ERR) return;
   FlowAttrib = m_FlowAttribNameList.getptr_at(m_FlowAttrib.GetCurSel() + 1)->get_name();
}

void CGridGetCatchmentAreaCellsDlg::OnBnClickedOk()
{
   C_CGRID *pGrid;
   long    key;

   if (Cls == 0 ||
       !get_GS_CURRENT_WRK_SESSION() ||
       !(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(Cls)))
   {
      gsui_alert(_T("Scegliere una classe."));
      return;
   }

   if (FlowAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere l'attributo contenente la direzione del flusso."));
      return;
   }

   if (pGrid->ptr_grid()->pt2key(pt, &key) == GS_BAD)
   {
      gsui_alert(_T("Il punto indicato non appartiene a nessuna cella della griglia."));
      return;
   }

   OnOK();
}

void CGridGetCatchmentAreaCellsDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Ricercadelbacino);
}

void CGridGetCatchmentAreaCellsDlg::OnEnKillfocusCoordxEdit()
{
   CString  dummy;
   C_STRING Msg;

   m_X.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   pt[X] = Msg.tof();
   RefreshSelection();
}

void CGridGetCatchmentAreaCellsDlg::OnEnKillfocusCoordyEdit()
{
   CString  dummy;
   C_STRING Msg;

   m_X.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   pt[X] = Msg.tof();
   RefreshSelection();
}

void CGridGetCatchmentAreaCellsDlg::OnBnClickedDirectionDownRadio()
{
   DirectionDown = true;
   RefreshSelection();
}

void CGridGetCatchmentAreaCellsDlg::OnBnClickedDirectionUpRadio()
{
   DirectionDown = false;
   RefreshSelection();
}


//**********************************************************
// FINE RICERCA BACINO A MONTE O A VALLE
// INIZIO RICERCA VALLI O CRINALI
//**********************************************************


/*************************************************************************/
/*.doc gsui_GridDisplayValleyOrRidge                                     */
/*+
  Comando per visualizzare le valli (i fiumi) o i crinali della griglia.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridDisplayValleyOrRidge(void)
{
   INT_PTR ret;
   CGridDisplayValleyOrRidgeDlg DisplayValleyOrRidgeDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	if ((ret = DisplayValleyOrRidgeDlg.DoModal()) == IDWINDOWSEL)
      { // Selezione di una finestra
         C_RB_LIST CoordList;
         
         // Evidenzio la zona
         if (gsc_define_window(CoordList) == GS_GOOD)
         {
            gsc_rb2Pt(CoordList.getptr_at(1), DisplayValleyOrRidgeDlg.pt1);
            gsc_rb2Pt(CoordList.getptr_at(2), DisplayValleyOrRidgeDlg.pt2);
         }
      }
   }
   while (ret == IDWINDOWSEL);

   if (ret == IDOK)
   {
      C_CGRID *pGrid;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(DisplayValleyOrRidgeDlg.Cls)))
         return GS_BAD;
      if (pGrid->DisplayValleyOrRidge(DisplayValleyOrRidgeDlg.ZAttrib, DisplayValleyOrRidgeDlg.FAS,
                                      DisplayValleyOrRidgeDlg.pt1, DisplayValleyOrRidgeDlg.pt2,
                                      DisplayValleyOrRidgeDlg.Valley,
                                      DisplayValleyOrRidgeDlg.DisplayMode) == GS_BAD)
         return GS_BAD;
   }

   return GS_GOOD;  
}


// CGridDisplayValleyOrRidgeDlg dialog

IMPLEMENT_DYNAMIC(CGridDisplayValleyOrRidgeDlg, CDialog)

CGridDisplayValleyOrRidgeDlg::CGridDisplayValleyOrRidgeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridDisplayValleyOrRidgeDlg::IDD, pParent)
{
   Cls = 0;
   pt1[X] = pt1[Y] = pt1[Z] = 0.0;
   ads_point_set(pt1, pt2);
   Valley      = true;
   DisplayMode = EXTRACTION;

   // Dimensione min. finestra di zoom
   MinZoomWindow = max(get_GS_GLOBALVAR()->get_AutoZoomMinXDim(), 
                       get_GS_GLOBALVAR()->get_AutoZoomMinYDim());
}

CGridDisplayValleyOrRidgeDlg::~CGridDisplayValleyOrRidgeDlg()
{
}

void CGridDisplayValleyOrRidgeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_GRID_GRIDAREATYPE, m_SelAreaType);
   DDX_Control(pDX, IDC_Z_ATTRIBUTE, m_ZAttrib);
   DDX_Control(pDX, IDC_VALLEY_RADIO, m_Valley);
   DDX_Control(pDX, IDC_RIDGE_RADIO, m_Ridge);
   DDX_Control(pDX, IDC_PREVIEW_CHECK, m_Preview);
   DDX_Control(pDX, IDC_FAS_BUTTON, m_FAS);
}


BEGIN_MESSAGE_MAP(CGridDisplayValleyOrRidgeDlg, CDialog)
   ON_BN_CLICKED(IDOK, &CGridDisplayValleyOrRidgeDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGridDisplayValleyOrRidgeDlg::OnBnClickedHelp)
   ON_BN_CLICKED(IDC_VALLEY_RADIO, &CGridDisplayValleyOrRidgeDlg::OnBnClickedValleyRadio)
   ON_BN_CLICKED(IDC_RIDGE_RADIO, &CGridDisplayValleyOrRidgeDlg::OnBnClickedRidgeRadio)
   ON_CBN_SELCHANGE(IDC_Z_ATTRIBUTE, &CGridDisplayValleyOrRidgeDlg::OnCbnSelchangeZAttribute)
   ON_BN_CLICKED(IDC_PREVIEW_CHECK, &CGridDisplayValleyOrRidgeDlg::OnBnClickedPreviewCheck)
   ON_BN_CLICKED(IDC_FAS_BUTTON, &CGridDisplayValleyOrRidgeDlg::OnBnClickedFasButton)
   ON_CBN_SELCHANGE(IDC_GRID_GRIDAREATYPE, &CGridDisplayValleyOrRidgeDlg::OnCbnSelchangeGridGridareatype)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGridDisplayValleyOrRidgeDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()



BOOL CGridDisplayValleyOrRidgeDlg::OnInitDialog() 
{
   C_STRING Mask;

	CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   m_SelAreaType.AddString(_T("Estensioni"));
   m_SelAreaType.AddString(_T("Finestra"));
   if (ads_point_equal(pt1, pt2))
      m_SelAreaType.SetCurSel(Extension);
   else
      m_SelAreaType.SetCurSel(Window);

   RefreshAttribList();
   RefreshSelection();

   int Pos = m_ZAttribNameList.getpos_name(ZAttrib.get_name(), FALSE);
   if (Pos > 0) m_ZAttrib.SetCurSel(Pos - 1);
   
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// CGridDisplayValleyOrRidgeDlg message handlers

void CGridDisplayValleyOrRidgeDlg::OnBnClickedOk()
{
   if (Cls == 0)
   {
      gsui_alert(_T("Scegliere una classe."));
      return;
   }

   if (ZAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo da cui leggere le quote."));
      return;
   }

   OnOK();
}

void CGridDisplayValleyOrRidgeDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Visualizzazionedivalliecrinali);
}

void CGridDisplayValleyOrRidgeDlg::OnBnClickedValleyRadio()
{
   Valley = true;
   RefreshSelection();
}

void CGridDisplayValleyOrRidgeDlg::OnBnClickedRidgeRadio()
{
   Valley = false;
   RefreshSelection();
}

void CGridDisplayValleyOrRidgeDlg::OnCbnSelchangeZAttribute()
{
   if (m_ZAttrib.GetCurSel() == CB_ERR) return;
   ZAttrib = m_ZAttribNameList.getptr_at(m_ZAttrib.GetCurSel() + 1)->get_name();
}

void CGridDisplayValleyOrRidgeDlg::OnBnClickedPreviewCheck()
{
   DisplayMode = (m_Preview.GetCheck() == BST_CHECKED) ? PREVIEW : EXTRACTION;
   RefreshSelection();
}

void CGridDisplayValleyOrRidgeDlg::OnBnClickedFasButton()
{  // caratteristiche settabili
   long EnabledFas = GSLayerSetting + GSLineTypeSetting + GSWidthSetting + 
                     GSColorSetting + GSThicknessSetting + GSLineTypeScaleSetting + 
                     GSHighlightSetting + GSZoomSetting + GSBlinkingSetting;
   int What = GRAPHICAL;

   gsc_ddChooseGraphSettings(EnabledFas, true, &flag_set, FAS, 
                             &MinZoomWindow, &What);
}


void CGridDisplayValleyOrRidgeDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls   = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();

   *pResult = 0;
}


void CGridDisplayValleyOrRidgeDlg::OnCbnSelchangeGridGridareatype()
{
   if (m_SelAreaType.GetCurSel() == Window)
      EndDialog(IDWINDOWSEL); // This value is returned by DoModal!
}

void CGridDisplayValleyOrRidgeDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;

   m_ZAttribNameList.remove_all();
   while (m_ZAttrib.DeleteString(0) != CB_ERR); // svuoto la combo

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto gli attributi non numerici
      // Salto l'attributo chiave
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD &&
          pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
      {
         m_ZAttrib.AddString(pAttrib->name.get_name());
         m_ZAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_ZAttribNameList.sort_name(); // li ordino in modo crescente
}

void CGridDisplayValleyOrRidgeDlg::RefreshSelection()
{
   if (Valley == true)
   {
      m_Valley.SetCheck(BST_CHECKED);
      m_Ridge.SetCheck(BST_UNCHECKED);
   }
   else
   {
      m_Valley.SetCheck(BST_UNCHECKED);
      m_Ridge.SetCheck(BST_CHECKED);
   }

   m_Preview.SetCheck((DisplayMode == PREVIEW) ? BST_CHECKED : BST_UNCHECKED);
}


//**********************************************************
// FINE RICERCA VALLI O CRINALI
// INIZIO AGGIORNAMENTO QUOTA DA GRAFICA
//**********************************************************


/*************************************************************************/
/*.doc gsui_GridUpdZFromGraph                                            */
/*+
  Comando che setta il valore di un campo della griglia 
  con la quota letta dagli oggetti grafici.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridUpdZFromGraph(void)
{
   INT_PTR ret;
   CGridUpdZFromGraphDlg GridUpdZFromGraphDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	if ((ret = GridUpdZFromGraphDlg.DoModal()) == IDOBJSSEL)
      { // Selezione di una finestra
         gsc_ssget(NULL, NULL, NULL, NULL, GridUpdZFromGraphDlg.SelSet);
         GridUpdZFromGraphDlg.AutoSel = (GridUpdZFromGraphDlg.SelSet.length() > 0) ? false : true;
      }
   }
   while (ret == IDOBJSSEL);

   if (ret == IDOK)
   {
      C_CGRID *pGrid;
      C_STRING *pSrcAttrib = NULL;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(GridUpdZFromGraphDlg.Cls)))
         return GS_BAD;

      if (GridUpdZFromGraphDlg.ZFromGraph == false)
         pSrcAttrib = &(GridUpdZFromGraphDlg.srcAttribName);

      if (GridUpdZFromGraphDlg.AutoSel)
      {
         if (pGrid->upd_data_fromGraph(GridUpdZFromGraphDlg.AllObjs, GridUpdZFromGraphDlg.ZAttrib, pSrcAttrib) == GS_BAD)
            return GS_BAD;
      }
      else
         if (pGrid->upd_data_fromGraph(GridUpdZFromGraphDlg.SelSet, GridUpdZFromGraphDlg.ZAttrib, pSrcAttrib) == GS_BAD)
            return GS_BAD;
   }

   return GS_GOOD;  
}


// CGridUpdZFromGraphDlg dialog

IMPLEMENT_DYNAMIC(CGridUpdZFromGraphDlg, CDialog)

CGridUpdZFromGraphDlg::CGridUpdZFromGraphDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridUpdZFromGraphDlg::IDD, pParent)
{
   hBmpSelObjs = NULL;

   Cls     = 0;
   AutoSel = true;
   gsc_ssget(_T("_X"), NULL, NULL, NULL, AllObjs);
   ZFromGraph = true;
}

CGridUpdZFromGraphDlg::~CGridUpdZFromGraphDlg()
{
   if (hBmpSelObjs) DeleteObject((HGDIOBJ) hBmpSelObjs);
}

void CGridUpdZFromGraphDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_Z_ATTRIBUTE, m_ZAttrib);
   DDX_Control(pDX, IDC_BUTTON_OBJSSELECTION, m_ObjsSelection);
   DDX_Control(pDX, IDC_N_SELECTED, mLbl_nSelected);
   DDX_Control(pDX, IDC_RADIO_SELECTALL, m_AutoSelection);
   DDX_Control(pDX, IDC_RADIO_MANUALSELECTION, m_ManualSelection);
   DDX_Control(pDX, IDC_RADIO_ELEVATION, m_SrcElevOption);
   DDX_Control(pDX, IDC_RADIO_ATTRIB, m_SrcAttribOption);
   DDX_Control(pDX, IDC_SRC_ATTRIB, mSrcAttribName);
}


BEGIN_MESSAGE_MAP(CGridUpdZFromGraphDlg, CDialog)
   ON_BN_CLICKED(IDC_RADIO_SELECTALL, &CGridUpdZFromGraphDlg::OnBnClickedRadioSelectall)
   ON_BN_CLICKED(IDC_RADIO_MANUALSELECTION, &CGridUpdZFromGraphDlg::OnBnClickedRadioManualselection)
   ON_BN_CLICKED(IDC_BUTTON_OBJSSELECTION, &CGridUpdZFromGraphDlg::OnBnClickedButtonObjsselection)
   ON_CBN_SELCHANGE(IDC_Z_ATTRIBUTE, &CGridUpdZFromGraphDlg::OnCbnSelchangeZAttribute)
   ON_BN_CLICKED(IDOK, &CGridUpdZFromGraphDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGridUpdZFromGraphDlg::OnBnClickedHelp)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGridUpdZFromGraphDlg::OnTvnSelchangedGeotreectrl)
   ON_BN_CLICKED(IDC_RADIO_ELEVATION, &CGridUpdZFromGraphDlg::OnBnClickedRadioElevation)
   ON_BN_CLICKED(IDC_RADIO_ATTRIB, &CGridUpdZFromGraphDlg::OnBnClickedRadioAttrib)
END_MESSAGE_MAP()


BOOL CGridUpdZFromGraphDlg::OnInitDialog() 
{
   COLORREF  crFrom;
	HINSTANCE Instance;
   C_STRING  Mask;

   CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   crFrom = RGB(255, 0, 0); // rosso
   // determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_SELECT_OBJS), RT_BITMAP);
   hBmpSelObjs = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SELECT_OBJS));
   gsui_SetBmpColorToDlgBkColor(hBmpSelObjs, crFrom);
   m_ObjsSelection.SetBitmap(hBmpSelObjs);

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte
   m_GeoTree.FilterOnUpdateable = true; // solo quelle modificabili

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   RefreshSelection();
   RefreshAttribList();

   int Pos = m_ZAttribNameList.getpos_name(ZAttrib.get_name(), FALSE);
   if (Pos > 0) m_ZAttrib.SetCurSel(Pos - 1); 

   if (srcAttribName.get_name())
      mSrcAttribName.SetWindowText(srcAttribName.get_name());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// CGridUpdZFromGraphDlg message handlers

void CGridUpdZFromGraphDlg::OnBnClickedRadioSelectall()
{
   AutoSel = true;
   RefreshSelection();
}

void CGridUpdZFromGraphDlg::OnBnClickedRadioManualselection()
{
   AutoSel = false;
   RefreshSelection();
}

void CGridUpdZFromGraphDlg::OnBnClickedButtonObjsselection()
{
   EndDialog(IDOBJSSEL); // This value is returned by DoModal!
}

void CGridUpdZFromGraphDlg::OnCbnSelchangeZAttribute()
{
   if (m_ZAttrib.GetCurSel() == CB_ERR) return;
   ZAttrib = m_ZAttribNameList.getptr_at(m_ZAttrib.GetCurSel() + 1)->get_name();
}


void CGridUpdZFromGraphDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();

   *pResult = 0;
}

void CGridUpdZFromGraphDlg::OnBnClickedRadioElevation()
{
   ZFromGraph = true;
   RefreshSelection();
}


void CGridUpdZFromGraphDlg::OnBnClickedRadioAttrib()
{
   ZFromGraph = false;
   RefreshSelection();
   mSrcAttribName.SetFocus();
}

void CGridUpdZFromGraphDlg::OnBnClickedOk()
{
   if (Cls == 0)
   {
      gsui_alert(_T("Scegliere una classe."));
      return;
   }

   if (ZAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo dove memorizzare le quote."));
      return;
   }

   if (ZFromGraph == false)
   {
      CString dummy;

      mSrcAttribName.GetWindowText(dummy);
      srcAttribName = dummy;
      srcAttribName.alltrim();
      if (srcAttribName.len() == 0)
      {
         gsui_alert(_T("Scegliere un attributo da cui leggere le quote."));
         return;
      }
   }

   OnOK();
}

void CGridUpdZFromGraphDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Aggiornamentocelledaelevazioneoggettiautocad);
}

void CGridUpdZFromGraphDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;

   m_ZAttribNameList.remove_all();
   while (m_ZAttrib.DeleteString(0) != CB_ERR); // svuoto la combo

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto gli attributi non numerici
      // Salto l'attributo chiave
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD &&
          pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
      {
         m_ZAttrib.AddString(pAttrib->name.get_name());
         m_ZAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_ZAttribNameList.sort_name(); // li ordino in modo crescente
}

void CGridUpdZFromGraphDlg::RefreshSelection()
{
   long     n;
   C_STRING Msg; 

   if (AutoSel == true)
   {
      n = AllObjs.length();
      m_AutoSelection.SetCheck(BST_CHECKED);
      m_ManualSelection.SetCheck(BST_UNCHECKED);
   }
   else
   {
      n = SelSet.length();
      m_AutoSelection.SetCheck(BST_UNCHECKED);
      m_ManualSelection.SetCheck(BST_CHECKED);
   }

   Msg = n;
   Msg += _T(" oggetti selezionati");
   mLbl_nSelected.SetWindowText(Msg.get_name());

   if (ZFromGraph == true)
   {
      m_SrcElevOption.SetCheck(BST_CHECKED);
      m_SrcAttribOption.SetCheck(BST_UNCHECKED);
      mSrcAttribName.EnableWindow(FALSE);
   }
   else
   {
      m_SrcElevOption.SetCheck(BST_UNCHECKED);
      m_SrcAttribOption.SetCheck(BST_CHECKED);
      mSrcAttribName.EnableWindow(TRUE);
   }
}


//**********************************************************
// FINE AGGIORNAMENTO QUOTA DA GRAFICA
// INIZIO AGGIORNAMENTO GRIGLIA DA DATABASE
//**********************************************************


/*************************************************************************/
/*.doc gsui_GridUpdFromDB                                            */
/*+
  Comando che aggiorna un campo (o tutti) della griglia con i dati di un DB esterno.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridUpdFromDB(void)
{
   CGridUpdFromDBDlg GridUpdFromDBDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

 	if (GridUpdFromDBDlg.DoModal() == IDOK)
   {
      C_CGRID        *pGrid;
      C_DBCONNECTION *pConn;
      C_STRING       *pAttribSrc = NULL, *pAttribDst = NULL;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(GridUpdFromDBDlg.Cls)))
         return GS_BAD;

      if ((pConn = get_pDBCONNECTION_LIST()->get_Connection(GridUpdFromDBDlg.UdlFile.get_name(),
                                                            &(GridUpdFromDBDlg.UdlProperties),
                                                            false,
                                                            GS_BAD)) == NULL)
         return GS_BAD;

      if (GridUpdFromDBDlg.DstAttrib.comp(_T("*")) != 0) // diverso da "tutti gli attributi"
      {
         pAttribSrc = &GridUpdFromDBDlg.SrcAttrib;
         pAttribDst = &GridUpdFromDBDlg.DstAttrib;
      }

      if (pGrid->upd_data_fromDB(pConn, GridUpdFromDBDlg.FullRefTable,
                                 GridUpdFromDBDlg.SrcXAttrib, GridUpdFromDBDlg.SrcYAttrib,
                                 pAttribSrc, pAttribDst) == GS_BAD)
         return GS_BAD;
   }

   return GS_GOOD;  
}


// CGridUpdFromDBDlg dialog

IMPLEMENT_DYNAMIC(CGridUpdFromDBDlg, CDialog)

CGridUpdFromDBDlg::CGridUpdFromDBDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridUpdFromDBDlg::IDD, pParent)
{
   Cls = 0;
}

CGridUpdFromDBDlg::~CGridUpdFromDBDlg()
{
}

void CGridUpdFromDBDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_UDL_EDIT, m_UdlEdit);
   DDX_Control(pDX, IDC_FULLREFTABLE_EDIT, m_FullReTableEdit);
   DDX_Control(pDX, IDC_SRCXATTRIB_COMBO, m_SrcXAttribCombo);
   DDX_Control(pDX, IDC_SRCYATTRIB_COMBO, m_SrcYAttribCombo);
   DDX_Control(pDX, IDC_SRCATTRIB_COMBO, m_SrcAttribCombo);
   DDX_Control(pDX, IDC_DSTATTRIB_COMBO, m_DstAttribCombo);
}


BEGIN_MESSAGE_MAP(CGridUpdFromDBDlg, CDialog)
   ON_BN_CLICKED(IDOK, &CGridUpdFromDBDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGridUpdFromDBDlg::OnBnClickedHelp)
   ON_CBN_SELCHANGE(IDC_SRCXATTRIB_COMBO, &CGridUpdFromDBDlg::OnCbnSelchangeSrcxattribCombo)
   ON_CBN_SELCHANGE(IDC_SRCYATTRIB_COMBO, &CGridUpdFromDBDlg::OnCbnSelchangeSrcyattribCombo)
   ON_CBN_SELCHANGE(IDC_SRCATTRIB_COMBO, &CGridUpdFromDBDlg::OnCbnSelchangeSrcattribCombo)
   ON_CBN_SELCHANGE(IDC_DSTATTRIB_COMBO, &CGridUpdFromDBDlg::OnCbnSelchangeDstattributeCombo)
   ON_BN_CLICKED(IDC_BUTTON1, &CGridUpdFromDBDlg::OnBnClickedButton1)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGridUpdFromDBDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()

BOOL CGridUpdFromDBDlg::OnInitDialog() 
{
   C_STRING Mask;

	CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte
   m_GeoTree.FilterOnUpdateable = true; // solo quelle modificabili

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   RefreshAttribList();

   int Pos = m_SrcNumericAttribNameList.getpos_name(SrcXAttrib.get_name(), FALSE);
   if (Pos > 0) m_SrcXAttribCombo.SetCurSel(Pos - 1);

   Pos = m_SrcNumericAttribNameList.getpos_name(SrcYAttrib.get_name(), FALSE);
   if (Pos > 0) m_SrcYAttribCombo.SetCurSel(Pos - 1);

   Pos = m_SrcAttribNameList.getpos_name(SrcAttrib.get_name(), FALSE);
   if (Pos > 0) m_SrcAttribCombo.SetCurSel(Pos - 1);

   Pos = m_DstAttribNameList.getpos_name(DstAttrib.get_name(), FALSE);
   if (Pos > 0) m_DstAttribCombo.SetCurSel(Pos - 1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// CGridUpdFromDBDlg message handlers

void CGridUpdFromDBDlg::OnBnClickedOk()
{
   if (Cls == 0)
   {
      gsui_alert(_T("Scegliere una classe."));
      return;
   }

   if (SrcXAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un campo da cui leggere la coordinata X."));
      return;
   }
   if (SrcYAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un campo da cui leggere la coordinata Y."));
      return;
   }

   if (SrcAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un campo sorgente."));
      return;
   }
   if (DstAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo di destinazione."));
      return;
   }

   if (DstAttrib.comp(_T("*")) == 0 && SrcAttrib.comp(_T("*")) != 0)
   {
      gsui_alert(_T("Se si vuole copiare in tutti gli attributi della griglia, devono essere selezionati tutti i campi della tabella sorgente (*)."));
      return;
   }
   if (DstAttrib.comp(_T("*")) != 0 && SrcAttrib.comp(_T("*")) == 0)
   {
      gsui_alert(_T("Se si vuole copiare tutti i campi della tabella sorgente, devono essere selezionati tutti gli attributi della griglia (*)."));
      return;
   }

   OnOK();
}

void CGridUpdFromDBDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Aggiornamentodadatabaseesterno);
}

void CGridUpdFromDBDlg::OnCbnSelchangeSrcxattribCombo()
{
   if (m_SrcXAttribCombo.GetCurSel() == CB_ERR) return;
   SrcXAttrib = m_SrcNumericAttribNameList.getptr_at(m_SrcXAttribCombo.GetCurSel() + 1)->get_name();
}

void CGridUpdFromDBDlg::OnCbnSelchangeSrcyattribCombo()
{
   if (m_SrcYAttribCombo.GetCurSel() == CB_ERR) return;
   SrcYAttrib = m_SrcNumericAttribNameList.getptr_at(m_SrcYAttribCombo.GetCurSel() + 1)->get_name();
}

void CGridUpdFromDBDlg::OnCbnSelchangeSrcattribCombo()
{
   if (m_SrcAttribCombo.GetCurSel() == CB_ERR) return;
   SrcAttrib = m_SrcAttribNameList.getptr_at(m_SrcAttribCombo.GetCurSel() + 1)->get_name();

   if (SrcAttrib.comp(_T("*")) == 0) // "tutti gli attributi"
   {
      int pos = m_DstAttribNameList.getpos_name(_T("*")); // 1-indexed
      if (pos > 0)
      {
         m_DstAttribCombo.SetCurSel(pos - 1); // 0-indexed
         DstAttrib = _T("*");
      }
   }
}

void CGridUpdFromDBDlg::OnCbnSelchangeDstattributeCombo()
{
   if (m_DstAttribCombo.GetCurSel() == CB_ERR) return;
   DstAttrib = m_DstAttribNameList.getptr_at(m_DstAttribCombo.GetCurSel() + 1)->get_name();

   if (DstAttrib.comp(_T("*")) == 0) //  "tutti gli attributi"
   {
      int pos = m_SrcAttribNameList.getpos_name(_T("*")); // 1-indexed
      if (pos > 0)
      {
         m_SrcAttribCombo.SetCurSel(pos - 1); // 0-indexed
         SrcAttrib = _T("*");
      }
   }
}

void CGridUpdFromDBDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;
   C_DBCONNECTION *pConn;
   C_ATTRIB_LIST SrcStru;

   m_UdlEdit.SetWindowText(UdlFile.get_name());
   m_FullReTableEdit.SetWindowText(FullRefTable.get_name());

   m_SrcNumericAttribNameList.remove_all();
   m_SrcAttribNameList.remove_all();
   m_DstAttribNameList.remove_all();
   while (m_SrcXAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_SrcYAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_SrcAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_DstAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo

   m_SrcAttribCombo.AddString(_T("*")); // "tutti gli attributi"
   m_SrcAttribNameList.add_tail_str(_T("*"));
   m_DstAttribCombo.AddString(_T("*")); // "tutti gli attributi"
   m_DstAttribNameList.add_tail_str(_T("*"));

   // Verifico la connessione OLE-DB
   if (UdlFile.get_name() && FullRefTable.get_name() &&
       (pConn = get_pDBCONNECTION_LIST()->get_Connection(UdlFile.get_name(),
                                                         &UdlProperties,
                                                         false,
                                                         GS_BAD)) != NULL)
      if (SrcStru.from_DB(pConn, pConn, FullRefTable.get_name()) == GS_GOOD)
      {
         pAttrib = (C_ATTRIB *) SrcStru.get_head();

         while (pAttrib)
         {
            // Salto gli attributi non numerici
            if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD)
            {
               m_SrcXAttribCombo.AddString(pAttrib->name.get_name());
               m_SrcYAttribCombo.AddString(pAttrib->name.get_name());
               m_SrcNumericAttribNameList.add_tail_str(pAttrib->name.get_name());
            }
            m_SrcAttribCombo.AddString(pAttrib->name.get_name());
            m_SrcAttribNameList.add_tail_str(pAttrib->name.get_name());

            pAttrib = (C_ATTRIB *) SrcStru.get_next();
         }
      }

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto l'attributo chiave
      if (pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
         // Salto gli attributi calcolati
         if (pAttrib->is_calculated() == GS_BAD)
         {
            m_DstAttribCombo.AddString(pAttrib->name.get_name());
            m_DstAttribNameList.add_tail_str(pAttrib->name.get_name());
         }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_SrcNumericAttribNameList.sort_name(); // li ordino in modo crescente
   m_SrcAttribNameList.sort_name(); // li ordino in modo crescente
   m_DstAttribNameList.sort_name(); // li ordino in modo crescente
}


void CGridUpdFromDBDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls   = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();

   *pResult = 0;
}


void CGridUpdFromDBDlg::OnBnClickedButton1()
{
   CDBConnDlg MyDlg;

   MyDlg.UdlFile = UdlFile;
   UdlProperties.copy(MyDlg.UdlProperties);
   MyDlg.FullRefTable = FullRefTable;
   MyDlg.Flags = CONN_DLG_CHOICE_ON_TABLE | CONN_DLG_CHOICE_ON_VIEW;
   if (MyDlg.DoModal() == IDOK)
   {
      UdlFile = MyDlg.UdlFile;
      MyDlg.UdlProperties.copy(UdlProperties);
      FullRefTable = MyDlg.FullRefTable;
      RefreshAttribList();
   }
}


//**********************************************************
// FINE RICERCA VALLI O CRINALI
// INIZIO AUTOCOMPOSIZIONE CURVE DI LIVELLO
//**********************************************************



// CGridAutoContoursDlg dialog

IMPLEMENT_DYNAMIC(CGridAutoContoursDlg, CDialog)

CGridAutoContoursDlg::CGridAutoContoursDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridAutoContoursDlg::IDD, pParent)
{
   m_AutoCompositionType       = PerNIntervals;
   m_AutoCompositionNSteps     = 5;
   m_AutoCompositionStep       = 0;
   m_AutoCompositionStartColor.setAutoCADColorIndex(1); // rosso
   m_AutoCompositionEndColor.setAutoCADColorIndex(3);   // verde
   m_MinValue                  = 0;
   m_MaxValue                  = 0;
}

CGridAutoContoursDlg::~CGridAutoContoursDlg()
{
}

void CGridAutoContoursDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_AUTO_PER_N_INTERVALS, m_AutoPerIntervalsRadio);
   DDX_Control(pDX, IDC_AUTO_PER_STEPS, m_AutoPerStepsRadio);
   DDX_Control(pDX, IDC_AUTO_N_INTERVALS, m_AutoIntervalsEdit);
   DDX_Control(pDX, IDC_AUTO_STEP, m_AutoStepEdit);
   DDX_Control(pDX, IDC_MIN_VALUE, m_MinValueEdit);
   DDX_Control(pDX, IDC_MAX_VALUE, m_MaxValueEdit);
}

BEGIN_MESSAGE_MAP(CGridAutoContoursDlg, CDialog)
   ON_BN_CLICKED(IDC_AUTO_PER_N_INTERVALS, &CGridAutoContoursDlg::OnBnClickedAutoPerNIntervals)
   ON_BN_CLICKED(IDC_AUTO_PER_STEPS, &CGridAutoContoursDlg::OnBnClickedAutoPerSteps)
   ON_WM_CTLCOLOR()
   ON_BN_CLICKED(IDC_START_COLOR, &CGridAutoContoursDlg::OnBnClickedStartColor)
   ON_BN_CLICKED(IDC_FINAL_COLOR, &CGridAutoContoursDlg::OnBnClickedFinalColor)
   ON_BN_CLICKED(IDOK, &CGridAutoContoursDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGridAutoContoursDlg::OnBnClickedHelp)
END_MESSAGE_MAP()

BOOL CGridAutoContoursDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
   // Create bold font
   CFont* pFont = GetFont();
   LOGFONT lf;
   pFont->GetLogFont(&lf);
   lf.lfWeight = FW_BOLD;
   m_BoldFont.CreateFontIndirect(&lf);

   RefreshSelection();
   
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// CGridAutoContoursDlg message handlers

void CGridAutoContoursDlg::OnBnClickedAutoPerNIntervals()
{
   m_AutoCompositionType = PerNIntervals;
   RefreshSelection();
}

void CGridAutoContoursDlg::OnBnClickedAutoPerSteps()
{
   m_AutoCompositionType = PerSteps;
   RefreshSelection();
}

void CGridAutoContoursDlg::RefreshSelection()
{
   C_STRING Msg;

   Msg = m_AutoCompositionNSteps;
   m_AutoIntervalsEdit.SetWindowText(Msg.get_name());
   Msg = m_AutoCompositionStep;
   m_AutoStepEdit.SetWindowText(Msg.get_name());

   switch (m_AutoCompositionType)
   {
      case PerNIntervals:
         m_AutoPerIntervalsRadio.SetCheck(BST_CHECKED);
         m_AutoPerStepsRadio.SetCheck(BST_UNCHECKED);
         m_AutoIntervalsEdit.EnableWindow(TRUE);
         m_AutoStepEdit.EnableWindow(FALSE);
         break;
      case PerSteps:
         m_AutoPerIntervalsRadio.SetCheck(BST_UNCHECKED);
         m_AutoPerStepsRadio.SetCheck(BST_CHECKED);
         m_AutoIntervalsEdit.EnableWindow(FALSE);
         m_AutoStepEdit.EnableWindow(TRUE);
         break;
   }

   Msg = m_MinValue;
   m_MinValueEdit.SetWindowText(Msg.get_name());
   Msg = m_MaxValue;
   m_MaxValueEdit.SetWindowText(Msg.get_name());

   AutoCompositionColorsList.remove_all();
   if (m_AutoCompositionStartColor.getColorMethod() != C_COLOR::None &&
       m_AutoCompositionEndColor.getColorMethod() != C_COLOR::None)
      // Ricavo una sfumatura di 10 colori
      AutoCompositionColorsList.getColorsFromTo(m_AutoCompositionStartColor, 
                                                m_AutoCompositionEndColor,
                                                10);

   if (AutoCompositionColorsList.get_count() > 0)
   {
      GetDlgItem(IDC_ARROW1)->RedrawWindow();
      GetDlgItem(IDC_ARROW2)->RedrawWindow();
      GetDlgItem(IDC_ARROW3)->RedrawWindow();
      GetDlgItem(IDC_ARROW4)->RedrawWindow();
      GetDlgItem(IDC_ARROW5)->RedrawWindow();
      GetDlgItem(IDC_ARROW6)->RedrawWindow();
      GetDlgItem(IDC_ARROW7)->RedrawWindow();
      GetDlgItem(IDC_ARROW8)->RedrawWindow();
      GetDlgItem(IDC_ARROW9)->RedrawWindow();
      GetDlgItem(IDC_ARROW10)->RedrawWindow();
   }
}

HBRUSH CGridAutoContoursDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   C_COLOR *p = NULL;

   // Call original functions
   HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

   switch (pWnd->GetDlgCtrlID())
   {
      case IDC_ARROW1:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(1);
         break;
      case IDC_ARROW2:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(2);
         break;
      case IDC_ARROW3:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(3);
         break;
      case IDC_ARROW4:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(4);
         break;
      case IDC_ARROW5:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(5);
         break;
      case IDC_ARROW6:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(6);
         break;
      case IDC_ARROW7:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(7);
         break;
      case IDC_ARROW8:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(8);
         break;
      case IDC_ARROW9:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(9);
         break;
      case IDC_ARROW10:
         p = (C_COLOR *) AutoCompositionColorsList.getptr_at(10);
         break;
   }
   
   if (p)
   {
      COLORREF textColor;

      // Set text color
      p->getRGB(&textColor);
      pDC->SetTextColor(textColor);
      // Set bold font
      pDC->SelectObject(&m_BoldFont);
   }

   // Return result of original function
   return hbr;
}

void CGridAutoContoursDlg::OnBnClickedStartColor()
{
   C_COLOR CurLayerColor;

   CurLayerColor.setForeground();
   if (gsc_SetColorDialog(m_AutoCompositionStartColor, true, CurLayerColor) == GS_GOOD)
      RefreshSelection();
}

void CGridAutoContoursDlg::OnBnClickedFinalColor()
{
   C_COLOR CurLayerColor;

   CurLayerColor.setForeground();
   if (gsc_SetColorDialog(m_AutoCompositionEndColor, true, CurLayerColor) == GS_GOOD)
      RefreshSelection();
}


void CGridAutoContoursDlg::OnBnClickedOk()
{
   CString  dummy;
   C_STRING Msg;

   switch (m_AutoCompositionType)
   {
      case PerNIntervals:
         m_AutoIntervalsEdit.GetWindowText(dummy);
         Msg = LPCTSTR(dummy);
         if (Msg.toi() > 0) m_AutoCompositionNSteps = Msg.toi();
         else
         {
            gsui_alert(_T("Il numero di intervalli deve essere un numero intero positivo."));
            return;
         }
         break;

      case PerSteps:
         m_AutoStepEdit.GetWindowText(dummy);
         Msg = LPCTSTR(dummy);
         if (Msg.tof() > 0) m_AutoCompositionStep = Msg.tof();
         else
         {
            gsui_alert(_T("Il passo deve essere un numero positivo."));
            return;
         }
         break;
   }
   
   double _min, _max;

   m_MinValueEdit.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   _min = Msg.tof();
   m_MaxValueEdit.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   _max = Msg.tof();
   if (_min >= _max)
   {
      gsui_alert(_T("Il valore minimo deve essere maggiore di quello massimo."));
      return;
   }
   m_MinValue = _min;
   m_MaxValue = _max;

   OnOK();
}

void CGridAutoContoursDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Visualizzazionecurvedilivello);
}


//**********************************************************
// FINE AUTOCOMPOSIZIONE CURVE DI LIVELLO
// INIZIO VISUALIZZAZIONE CURVE DI LIVELLO
//**********************************************************


/*************************************************************************/
/*.doc gsui_GridDisplayContours                                          */
/*+
  Comando per visualizzare le curve di livello della griglia.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_GridDisplayContours(void)
{
   INT_PTR                 ret;
   CGridDisplayContoursDlg DisplayContoursDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	if ((ret = DisplayContoursDlg.DoModal()) == IDWINDOWSEL)
      { // Selezione di una finestra
         C_RB_LIST CoordList;
         
         // Evidenzio la zona
         if (gsc_define_window(CoordList) == GS_GOOD)
         {
            gsc_rb2Pt(CoordList.getptr_at(1), DisplayContoursDlg.pt1);
            gsc_rb2Pt(CoordList.getptr_at(2), DisplayContoursDlg.pt2);
         }
      }
   }
   while (ret == IDWINDOWSEL);

   if (ret == IDOK)
   {
      C_CGRID *pGrid;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(DisplayContoursDlg.Cls)))
         return GS_BAD;
      if (pGrid->DisplayContours(DisplayContoursDlg.ZAttrib, DisplayContoursDlg.FAS_list,
                                 DisplayContoursDlg.pt1, DisplayContoursDlg.pt2,
                                 DisplayContoursDlg.DisplayMode,
                                 DisplayContoursDlg.Join,
                                 DisplayContoursDlg.Fit) == GS_BAD)
         return GS_BAD;
   }

   return GS_GOOD;  
}


// CGridDisplayContoursDlg dialog

IMPLEMENT_DYNAMIC(CGridDisplayContoursDlg, CDialog)

CGridDisplayContoursDlg::CGridDisplayContoursDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridDisplayContoursDlg::IDD, pParent)
{
   Cls = 0;
   pt1[X] = pt1[Y] = pt1[Z] = 0.0;
   ads_point_set(pt1, pt2);
   DisplayMode = EXTRACTION;
   Join        = true;
   Fit         = true;
}

CGridDisplayContoursDlg::~CGridDisplayContoursDlg()
{
}

void CGridDisplayContoursDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_GRID_GRIDAREATYPE, m_SelAreaType);
   DDX_Control(pDX, IDC_Z_ATTRIBUTE, m_ZAttrib);
   DDX_Control(pDX, IDC_PREVIEW_CHECK, m_Preview);
   DDX_Control(pDX, IDC_JOIN_CHECK, m_Join);
   DDX_Control(pDX, IDC_CURVE_CHECK, m_Fit);
   DDX_Control(pDX, IDC_AUTOCOMPOSITION, m_AutocompositionButton);
   DDX_Control(pDX, IDC_CONTOUR_LIST, m_ContourList);
   DDX_Control(pDX, IDC_SELECT_INTERVAL_EDIT, m_SelectIntervalEdit);
}


BEGIN_MESSAGE_MAP(CGridDisplayContoursDlg, CDialog)
   ON_BN_CLICKED(IDOK, &CGridDisplayContoursDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGridDisplayContoursDlg::OnBnClickedHelp)
   ON_CBN_SELCHANGE(IDC_GRID_GRIDAREATYPE, &CGridDisplayContoursDlg::OnCbnSelchangeGridGridareatype)
   ON_CBN_SELCHANGE(IDC_Z_ATTRIBUTE, &CGridDisplayContoursDlg::OnCbnSelchangeZAttribute)
   ON_BN_CLICKED(IDC_PREVIEW_CHECK, &CGridDisplayContoursDlg::OnBnClickedPreviewCheck)
   ON_BN_CLICKED(IDC_JOIN_CHECK, &CGridDisplayContoursDlg::OnBnClickedJoinCheck)
   ON_BN_CLICKED(IDC_CURVE_CHECK, &CGridDisplayContoursDlg::OnBnClickedCurveCheck)
   ON_BN_CLICKED(IDC_AUTOCOMPOSITION, &CGridDisplayContoursDlg::OnBnClickedAutocomposition)
   ON_BN_CLICKED(IDNEW, &CGridDisplayContoursDlg::OnBnClickedNew)
   ON_BN_CLICKED(IDMODIFY, &CGridDisplayContoursDlg::OnBnClickedModify)
   ON_BN_CLICKED(IDERASE, &CGridDisplayContoursDlg::OnBnClickedErase)
   ON_NOTIFY(NM_CUSTOMDRAW, IDC_CONTOUR_LIST, &CGridDisplayContoursDlg::OnNMCustomdrawContourList)
   ON_BN_CLICKED(IDSELECT_BUTTON, &CGridDisplayContoursDlg::OnBnClickedSelectIntervals)
   ON_NOTIFY(NM_DBLCLK, IDC_CONTOUR_LIST, &CGridDisplayContoursDlg::OnNMDblclkContourList)
   ON_BN_CLICKED(IDSAVE, &CGridDisplayContoursDlg::OnBnClickedSave)
   ON_BN_CLICKED(IDLOAD, &CGridDisplayContoursDlg::OnBnClickedLoad)
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CGridDisplayContoursDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()


BOOL CGridDisplayContoursDlg::OnInitDialog() 
{
   C_STRING Mask;
	CRect    rect;

	CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // tutte le classi griglia
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnExtracted = true; // solo classi estratte

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Cls == 0)
      Cls = gsui_getFirstExtractedGridClsCode(); // seleziono la prima classe griglia

   if (Cls > 0)
      m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), Cls);
   else
      m_GeoTree.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());

   m_SelAreaType.AddString(_T("Estensioni"));
   m_SelAreaType.AddString(_T("Finestra"));
   if (ads_point_equal(pt1, pt2))
      m_SelAreaType.SetCurSel(Extension);
   else
      m_SelAreaType.SetCurSel(Window);

   RefreshAttribList();
   RefreshSelection();

   int Pos = m_ZAttribNameList.getpos_name(ZAttrib.get_name(), FALSE);
   if (Pos > 0) m_ZAttrib.SetCurSel(Pos - 1);

 	m_ContourList.SetExtendedStyle(m_ContourList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_ContourList.GetWindowRect(&rect);
   int VirtualCtrlWidth = rect.Width() - 4;

   m_ContourList.InsertColumn(0, _T("Valore"), LVCFMT_LEFT,
	                           VirtualCtrlWidth * 4/20, 0);
   m_ContourList.InsertColumn(1, _T("Colore"), LVCFMT_LEFT,
                              VirtualCtrlWidth * 2/20, 1);
   m_ContourList.InsertColumn(2, _T("Descrizione"), LVCFMT_LEFT,
                              VirtualCtrlWidth * 4/20, 2);
   m_ContourList.InsertColumn(3, _T("Tipolinea"), LVCFMT_LEFT,
                              VirtualCtrlWidth * 5/20, 3);
   m_ContourList.InsertColumn(4, _T("Piano"), LVCFMT_LEFT,
                              VirtualCtrlWidth * 5/20, 4);

   RefreshContourList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CGridDisplayContoursDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   Cls   = m_GeoTree.GetSelected(GSClass);
   RefreshAttribList();
   RefreshSelection();

   *pResult = 0;
}


void CGridDisplayContoursDlg::OnBnClickedOk()
{
   if (Cls == 0)
   {
      gsui_alert(_T("Scegliere una classe."));
      return;
   }

   if (ZAttrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un attributo da cui leggere le quote."));
      return;
   }

   OnOK();
}

void CGridDisplayContoursDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Visualizzazionecurvedilivello);
}

void CGridDisplayContoursDlg::OnCbnSelchangeGridGridareatype()
{
   if (m_SelAreaType.GetCurSel() == Window)
      EndDialog(IDWINDOWSEL); // This value is returned by DoModal!
}

void CGridDisplayContoursDlg::OnCbnSelchangeZAttribute()
{
   if (m_ZAttrib.GetCurSel() == CB_ERR) return;
   if (ZAttrib.comp(m_ZAttribNameList.getptr_at(m_ZAttrib.GetCurSel() + 1)->get_name()) != 0)
   {  // se era diverso dal preceente valore
      ZAttrib = m_ZAttribNameList.getptr_at(m_ZAttrib.GetCurSel() + 1)->get_name();
      AutoContoursDlg.m_MinValue = 0;
      AutoContoursDlg.m_MaxValue = 0;
   }
   RefreshSelection();
}

void CGridDisplayContoursDlg::OnBnClickedPreviewCheck()
{
   DisplayMode = (m_Preview.GetCheck() == BST_CHECKED) ? PREVIEW : EXTRACTION;
   RefreshSelection();
}

void CGridDisplayContoursDlg::OnBnClickedJoinCheck()
{
   Join = (m_Join.GetCheck() == BST_CHECKED) ? true : false;
   RefreshSelection();
}

void CGridDisplayContoursDlg::OnBnClickedCurveCheck()
{
   Fit = (m_Fit.GetCheck() == BST_CHECKED) ? true : false;
   RefreshSelection();
}

void CGridDisplayContoursDlg::OnBnClickedAutocomposition()
{
   // se AutoContoursDlg.m_MinValue e AutoContoursDlg.m_MaxValue
   // non sono stati inizializzati
   if (AutoContoursDlg.m_MinValue == 0 && AutoContoursDlg.m_MaxValue == 0)
   {
      C_CGRID        *pGrid;
      C_STRING       TempTableRef;
      C_DBCONNECTION *pConn;

      if (!(pGrid = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(Cls)))
         return;
      // ricavo connessione OLE-DB per tabella TEMP
      if ((pConn = pGrid->ptr_info()->getDBConnection(TEMP)) == NULL) return;
      // senza creare la tabella
      if (pGrid->getTempTableRef(TempTableRef, GS_BAD) == GS_BAD) return;
      // ricavo il valore minore
      if (pConn->GetNumAggrValue(TempTableRef.get_name(), ZAttrib.get_name(),
                                 _T("MIN"), &AutoContoursDlg.m_MinValue) != GS_GOOD) return;
      // ricavo il valore maggiore
      if (pConn->GetNumAggrValue(TempTableRef.get_name(), ZAttrib.get_name(),
                                 _T("MAX"), &AutoContoursDlg.m_MaxValue) != GS_GOOD) return;
   }

	if (AutoContoursDlg.DoModal() != IDOK) return;

   if (AutoContoursDlg.m_MinValue == AutoContoursDlg.m_MaxValue) return;

   switch (AutoContoursDlg.m_AutoCompositionType)
   {
      case CGridAutoContoursDlg::PerNIntervals:
         if (gsc_getFASList4ContoursGrid(AutoContoursDlg.m_AutoCompositionNSteps,
                                         AutoContoursDlg.m_MinValue, AutoContoursDlg.m_MaxValue,
                                         AutoContoursDlg.m_AutoCompositionStartColor, AutoContoursDlg.m_AutoCompositionEndColor,
                                          _T("ContourSet"), FAS_list) == GS_BAD)
            return;
         break;
      case CGridAutoContoursDlg::PerSteps:
         if (gsc_getFASList4ContoursGrid(AutoContoursDlg.m_AutoCompositionStep,
                                         AutoContoursDlg.m_MinValue, AutoContoursDlg.m_MaxValue,
                                         AutoContoursDlg.m_AutoCompositionStartColor, AutoContoursDlg.m_AutoCompositionEndColor,
                                         _T("ContourSet"), FAS_list) == GS_BAD)
            return;
         break;
   }

   RefreshContourList();
}

void CGridDisplayContoursDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pGrid;

   m_ZAttribNameList.remove_all();
   while (m_ZAttrib.DeleteString(0) != CB_ERR); // svuoto la combo

   if (Cls == 0) return;
   if (!(pGrid = get_GS_CURRENT_WRK_SESSION()->find_class(Cls))) return;

   pAttrib = (C_ATTRIB *) pGrid->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto gli attributi non numerici
      // Salto l'attributo chiave
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD &&
          pAttrib->name.comp(pGrid->ptr_info()->key_attrib, false) != 0)
      {
         m_ZAttrib.AddString(pAttrib->name.get_name());
         m_ZAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_ZAttribNameList.sort_name(); // li ordino in modo crescente
}

void CGridDisplayContoursDlg::RefreshSelection()
{
   m_Join.SetCheck((Join) ? BST_CHECKED : BST_UNCHECKED);
   m_Fit.SetCheck((Fit) ? BST_CHECKED : BST_UNCHECKED);

   if (DisplayMode == PREVIEW)
   {
      m_Preview.SetCheck(BST_CHECKED);
      m_Join.EnableWindow(FALSE);
      m_Fit.EnableWindow(FALSE);
   }
   else
   {
      m_Preview.SetCheck(BST_UNCHECKED);
      m_Join.EnableWindow(TRUE);
      if (m_Join.GetCheck() == BST_CHECKED)
         m_Fit.EnableWindow(TRUE); // disponibile se join è attivo
      else
         m_Fit.EnableWindow(FALSE);
   }

   if (Cls > 0 && ZAttrib.len() > 0)
      m_AutocompositionButton.EnableWindow(TRUE);
   else
      m_AutocompositionButton.EnableWindow(FALSE);
}

void CGridDisplayContoursDlg::OnNMCustomdrawContourList(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLVCUSTOMDRAW lpLVCustomDraw = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
   C_FAS            *pFAS = NULL;

   switch(lpLVCustomDraw->nmcd.dwDrawStage)
   {
      case CDDS_ITEMPREPAINT:
      case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
         if (1 == lpLVCustomDraw->iSubItem) // colonna del colore
            pFAS = (C_FAS *) FAS_list.goto_pos((int) lpLVCustomDraw->nmcd.dwItemSpec + 1); // 1-index

         if (pFAS)
         {
            //Remove standard highlighting of selected (sub)item.
            lpLVCustomDraw->nmcd.uItemState = CDIS_DEFAULT;
            lpLVCustomDraw->clrText = RGB(255,255,255); // white text
            //lpLVCustomDraw->clrTextBk = RGB(0,0,0); // black background
            pFAS->color.getRGB(&lpLVCustomDraw->clrTextBk); // background color
         }
         else 
         {
            lpLVCustomDraw->clrText = CLR_DEFAULT;
            lpLVCustomDraw->clrTextBk = CLR_DEFAULT;
         }
         break;

    default: break;    
  }

  *pResult = 0;
  *pResult |= CDRF_NOTIFYPOSTPAINT;
  *pResult |= CDRF_NOTIFYITEMDRAW;
  *pResult |= CDRF_NOTIFYSUBITEMDRAW;
}

void CGridDisplayContoursDlg::RefreshContourList()
{
	int      iRow = 0;
	LV_ITEM  lvitem;
   C_STRING dummyStr;
   C_FAS    *pFAS = (C_FAS *) FAS_list.get_head();

   m_ContourList.DeleteAllItems();

   while (pFAS)
   {  // Valore
      gsc_conv_Number(pFAS->elevation, dummyStr);
      lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
      lvitem.iItem    = iRow++;
      lvitem.iSubItem = 0;
      lvitem.pszText  = dummyStr.get_name();
      lvitem.lParam   = (LPARAM) pFAS;
   	m_ContourList.InsertItem(&lvitem);

      // Colore
      lvitem.mask    = LVIF_TEXT;
      lvitem.iSubItem++;
      lvitem.pszText = _T("");
      m_ContourList.SetItem(&lvitem);

      // Descrizione
      // viene usato lo stile di quotatura per memorizzare la descrizione della curva di livello
      lvitem.iSubItem++;
      lvitem.pszText = pFAS->dimension_style.get_name();
      m_ContourList.SetItem(&lvitem);

      // Tipolinea
      lvitem.iSubItem++;
      lvitem.pszText = pFAS->line;
      m_ContourList.SetItem(&lvitem);

      // Piano
      lvitem.iSubItem++;
      lvitem.pszText = pFAS->layer;
      m_ContourList.SetItem(&lvitem);

      pFAS = (C_FAS *) FAS_list.get_next();
   }
}

void CGridDisplayContoursDlg::OnBnClickedNew()
{
   CGridPropContourDlg PropContourDlg;
   C_FAS               *pFAS, *pNewFAS;

	if (PropContourDlg.DoModal() != IDOK) return;

   if ((pNewFAS = new C_FAS) == NULL) return;
   pNewFAS->elevation = PropContourDlg.m_Value;
   pNewFAS->color = PropContourDlg.m_Color;
   // viene usato lo stile di quotatura per memorizzare la descrizione della curva di livello
   pNewFAS->dimension_style = PropContourDlg.m_Description;
   gsc_strcpy(pNewFAS->layer, PropContourDlg.m_Layer.get_name(), MAX_LEN_LAYERNAME);
   gsc_strcpy(pNewFAS->line, PropContourDlg.m_LineType.get_name(), MAX_LEN_LINETYPENAME);

   // Inserimento ordinato
   pFAS = (C_FAS *) FAS_list.get_head();
   while (pFAS)
   {
      if (pFAS->elevation > PropContourDlg.m_Value) break;
      pFAS = (C_FAS *) FAS_list.get_next();
   }

   if (pFAS) 
      FAS_list.insert_before(pNewFAS);
   else
      FAS_list.add_tail(pNewFAS);

   FAS_list.sortByNum(GSElevationSetting); // ordino in modo crescente
   RefreshContourList();
}

void CGridDisplayContoursDlg::OnBnClickedModify()
{
   bool                first = true;
   C_FAS               *pFAS;
   CGridPropContourDlg PropContourDlg;
   int                 i = 0, n_selected = 0;

   for (i = 0; i < m_ContourList.GetItemCount(); i++)
      if (m_ContourList.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
      {
         n_selected++;
         pFAS = (C_FAS *) m_ContourList.GetItemData(i);

         if (first)
         {
            first = false;
            PropContourDlg.m_Value = pFAS->elevation;
            PropContourDlg.m_Color = pFAS->color;
            // viene usato lo stile di quotatura per memorizzare la descrizione della curva di livello
            PropContourDlg.m_Description = pFAS->dimension_style;
            PropContourDlg.m_LineType = pFAS->line;
            PropContourDlg.m_Layer = pFAS->layer;
         }
         else
         {
            if (PropContourDlg.m_Value != pFAS->elevation)
               PropContourDlg.m_Value = (std::numeric_limits<double>::min)(); // valore non valido che significa non settato
            if (PropContourDlg.m_Color != pFAS->color)
               PropContourDlg.m_Color.setNone(); // valore non valido che significa non settato
            // viene usato lo stile di quotatura per memorizzare la descrizione della curva di livello
            if (PropContourDlg.m_Description.comp(pFAS->dimension_style) != 0)
               PropContourDlg.m_Description.clear();
            if (PropContourDlg.m_LineType.comp(pFAS->line) != 0)
               PropContourDlg.m_LineType.clear();
            if (PropContourDlg.m_Layer.comp(pFAS->layer) != 0)
               PropContourDlg.m_Layer.clear();
         }
      }

   if (n_selected == 0) return;
   if (n_selected > 1) PropContourDlg.m_Multi = true;
	if (PropContourDlg.DoModal() != IDOK) return;

   for (i = 0; i < m_ContourList.GetItemCount(); i++)
      if (m_ContourList.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
      {
         pFAS = (C_FAS *) m_ContourList.GetItemData(i);

         if (PropContourDlg.m_Value != (std::numeric_limits<double>::min)()) // valore non valido che significa non settato
            pFAS->elevation = PropContourDlg.m_Value;
         if (PropContourDlg.m_Color.getColorMethod() != C_COLOR::None) // valore non valido che significa non settato
            pFAS->color = PropContourDlg.m_Color;
         // viene usato lo stile di quotatura per memorizzare la descrizione della curva di livello
         if (PropContourDlg.m_Description.len() > 0)
            pFAS->dimension_style = PropContourDlg.m_Description;
         if (PropContourDlg.m_LineType.len() > 0)
            gsc_strcpy(pFAS->line, PropContourDlg.m_LineType.get_name(), MAX_LEN_LINETYPENAME);
         if (PropContourDlg.m_Layer.len() > 0)
             gsc_strcpy(pFAS->layer, PropContourDlg.m_Layer.get_name(), MAX_LEN_LAYERNAME);
      }

   FAS_list.sortByNum(GSElevationSetting); // ordino in modo crescente
   RefreshContourList();
}

void CGridDisplayContoursDlg::OnBnClickedErase()
{
   bool ask = true;

   for (int i = 0; i < m_ContourList.GetItemCount(); i++)
      if (m_ContourList.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
      {
         if (ask)
         {
            ask = false;
            if (gsui_confirm(_T("Cancellare i valori selezioni ?"), GS_GOOD, FALSE, FALSE, m_hWnd) != GS_GOOD)
               return;
         }

         FAS_list.remove((C_NODE *) m_ContourList.GetItemData(i));
      }

   RefreshContourList();
}

void CGridDisplayContoursDlg::OnBnClickedSelectIntervals()
{
   CString dummy;
   C_STRING Msg;
   double   _Step;
   int      i;
   C_FAS    *pFAS;

   m_SelectIntervalEdit.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   if ((_Step = Msg.tof()) <= 0)
   {
      gsui_alert(_T("Il passo di selezione deve essere un numero positivo."));
      return;
   }

   // Deseleziono tutte le righe
   for (i = 0; i < m_ContourList.GetItemCount(); i++)
      m_ContourList.SetItemState(i, 0, LVIS_SELECTED);

   i = 0;
   while ((pFAS = (C_FAS *) m_ContourList.GetItemData(i)))
   {
      if (fmod(pFAS->elevation, _Step) == 0) // ogni _Value
         m_ContourList.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
      i++;
   }
}

void CGridDisplayContoursDlg::OnNMDblclkContourList(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

   OnBnClickedModify();

   *pResult = 0;
}

void CGridDisplayContoursDlg::OnBnClickedSave()
{
   C_STRING filename, DefaultFileName, LastContoursFile, Dir;

   // Se non esiste alcun file precedente
   if (gsc_getPathInfoFromINI(_T("LastContoursFile"), LastContoursFile) == GS_BAD ||
       gsc_dir_exist(LastContoursFile) == GS_BAD)
   {
      // imposto il direttorio di ricerca del file
      Dir = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      Dir += _T("\\");
      Dir += GEOQRYDIR;
   }
   else
      if (gsc_dir_from_path(LastContoursFile, Dir) == GS_BAD) return;

   // "Posizione"
   if (gsc_get_tmp_filename(Dir.get_name(), _T("Curve_di_livello"), _T(".contour"), DefaultFileName) == GS_BAD)
      return;

   // "Seleziona file di query"
   if (gsc_GetFileD(_T("Seleziona file delle curve di livello"),
                    DefaultFileName, _T("contour"), UI_SAVEFILE_FLAGS, filename) == RTNORM)
      if (Save(filename.get_name()) == GS_GOOD)
      {
         // memorizzo la scelta in GEOSIM.INI per riproporla la prossima volta
         gsc_setPathInfoToINI(_T("LastContoursFile"), filename);
      }
      else
         gsui_alert(_T("File non salvato."));
}

int CGridDisplayContoursDlg::Save(const TCHAR *Path)
{
   C_STRING EntryValue;
   FILE     *f;
   bool     Unicode = false;

   if ((f = gsc_open_profile(Path, UPDATEABLE, MORETESTS, &Unicode)) == NULL) return GS_BAD;

   // Join
   EntryValue = (Join) ? _T("1") : _T("0");
   if (gsc_set_profile(f, _T("Contours"), _T("ContoursJoin"), EntryValue.get_name(), 0, Unicode) == GS_BAD)
      { gsc_close_profile(f); return GS_BAD; }

   // Curve
   EntryValue = (Fit) ? _T("1") : _T("0");
   if (gsc_set_profile(f, _T("Contours"), _T("ContoursFit"), EntryValue.get_name(), 0, Unicode) == GS_BAD)
      { gsc_close_profile(f); return GS_BAD; }
   gsc_close_profile(f);

   EntryValue = Path;
   if (FAS_list.Save(EntryValue, _T("Contours")) == GS_BAD) return GS_BAD;
 
   return GS_GOOD;
}

void CGridDisplayContoursDlg::OnBnClickedLoad()
{
   C_STRING filename, DefaultFileName, LastContoursFile, Dir;

   // Se non esiste alcun file precedente
   if (gsc_getPathInfoFromINI(_T("LastContoursFile"), LastContoursFile) == GS_BAD ||
       gsc_dir_exist(LastContoursFile) == GS_BAD)
   {
      // imposto il direttorio di ricerca del file
      Dir = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      Dir += _T("\\");
      Dir += GEOQRYDIR;
   }
   else
      if (gsc_dir_from_path(LastContoursFile, Dir) == GS_BAD) return;

   if (gsc_GetFileD(_T("Seleziona file delle curve di livello"), 
                    Dir, _T("contour"), UI_LOADFILE_FLAGS, filename) == RTNORM)
      if (Load(filename.get_name()) == GS_GOOD)
      {
         // memorizzo la scelta in GEOSIM.INI per riproporla la prossima volta
         gsc_setPathInfoToINI(_T("LastContoursFile"), filename);

         RefreshSelection();
         RefreshContourList();
      }
      else
         gsui_alert(_T("File non caricato."));
}

int CGridDisplayContoursDlg::Load(const TCHAR *Path)
{
   C_PROFILE_SECTION_BTREE ProfileSections;
   C_BPROFILE_SECTION      *ProfileSection;
   C_2STR_BTREE            *pProfileEntries;
   C_B2STR                 *pProfileEntry;
   C_STRING                EntryValue;

   if (gsc_read_profile(Path, ProfileSections) == GS_BAD) return GS_BAD;
   if ((ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("Contours"))) == NULL)
      return GS_BAD;
   pProfileEntries = (C_2STR_BTREE *) ProfileSection->get_ptr_EntryList();

   // Join
   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("ContoursJoin")))) return GS_BAD;
   Join = (_wtoi(pProfileEntry->get_name2()) == 0) ? false : true;

   // Curve
   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("ContoursFit")))) return GS_BAD;
   Fit = (_wtoi(pProfileEntry->get_name2()) == 0) ? false : true;

   if (FAS_list.Load(ProfileSections, _T("Contours")) == GS_BAD) return GS_BAD;

   return GS_GOOD;
}


//**********************************************************
// INIZIO PROPRIETA' CURVA DI LIVELLO
//**********************************************************


// CGridPropContourDlg dialog

IMPLEMENT_DYNAMIC(CGridPropContourDlg, CDialog)

CGridPropContourDlg::CGridPropContourDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridPropContourDlg::IDD, pParent)
{
   m_Multi = false;
   m_Value = (std::numeric_limits<double>::min)(); // valore non valido che significa non settato
   m_Color.setNone();                              // valore non valido che significa non settato
}

CGridPropContourDlg::~CGridPropContourDlg()
{
}

void CGridPropContourDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_VALUE, m_ValueEdit);
   DDX_Control(pDX, IDC_COLOR, m_ColorEdit);
   DDX_Control(pDX, IDC_DESCRIPTION, m_DescriptionEdit);
   DDX_Control(pDX, IDC_LINETYPE, m_LineTypeEdit);
   DDX_Control(pDX, IDC_LAYER, m_LayerEdit);
}


BEGIN_MESSAGE_MAP(CGridPropContourDlg, CDialog)
   ON_BN_CLICKED(IDC_COLOR_BUTTON, &CGridPropContourDlg::OnBnClickedColorButton)
   ON_BN_CLICKED(IDC_LINETYPE_BUTTON, &CGridPropContourDlg::OnBnClickedLinetypeButton)
   ON_BN_CLICKED(IDC_LAYER_BUTTON, &CGridPropContourDlg::OnBnClickedLayerButton)
   ON_BN_CLICKED(IDOK, &CGridPropContourDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGridPropContourDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


BOOL CGridPropContourDlg::OnInitDialog() 
{
   C_STRING Msg;

	CDialog::OnInitDialog();

   // se valore non valido che significa non settato
   if (m_Value == (std::numeric_limits<double>::min)()) Msg = _T(""); 
   else Msg = m_Value;
   m_ValueEdit.SetWindowText(Msg.get_name());

   if (m_Color.getColorMethod() == C_COLOR::None) Msg = _T("");  // valore non valido che significa non settato
   else m_Color.getString(Msg);
   m_ColorEdit.SetWindowText(Msg.get_name());

   m_DescriptionEdit.SetWindowText(m_Description.get_name());
   m_LineTypeEdit.SetWindowText(m_LineType.get_name());
   m_LayerEdit.SetWindowText(m_Layer.get_name());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// CGridPropContourDlg message handlers

void CGridPropContourDlg::OnBnClickedColorButton()
{
   CString  dummy;
   C_STRING Msg;
   C_COLOR  _color, CurLayerColor;

   CurLayerColor.setForeground();
   m_ColorEdit.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   if (Msg.len() > 0)
      if (_color.setString(Msg) == GS_BAD) return;

   if (gsc_SetColorDialog(_color, true, CurLayerColor) != GS_GOOD) return;
   _color.getString(Msg);
   m_ColorEdit.SetWindowText(Msg.get_name());
}

void CGridPropContourDlg::OnBnClickedLinetypeButton()
{
   CString  dummy;
   C_STRING Msg;
   presbuf  pRb;

   m_LineTypeEdit.GetWindowText(dummy);
   Msg = dummy;
   if ((pRb = gsc_ddsellinetype(Msg.get_name())) != NULL)
      m_LineTypeEdit.SetWindowText(pRb->resval.rstring);
}

void CGridPropContourDlg::OnBnClickedLayerButton()
{
   CString  dummy;
   C_STRING Msg;
   presbuf  pRb;

   m_LayerEdit.GetWindowText(dummy);
   Msg = dummy;
   if ((pRb = gsc_ddsellayers(Msg.get_name())) != NULL)
      m_LayerEdit.SetWindowText(pRb->resval.rstring);
}

void CGridPropContourDlg::OnBnClickedOk()
{
   CString  dummy;
   C_STRING Msg;

   m_ValueEdit.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   Msg.alltrim();
   if (Msg.len() > 0)
   {
      if (gsc_conv_Number(Msg.get_name(), &m_Value) != GS_GOOD)
      {       
         gsui_alert(_T("Il valore deve essere un numero reale."));
         return;
      }
   }
   else
      if (m_Multi == false)
      {
         gsui_alert(_T("Il valore deve essere un numero reale."));
         return;
      }
      else
         m_Value = (std::numeric_limits<double>::min)(); // valore non valido che significa non settato

   m_ColorEdit.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   if (Msg.len() > 0)
   {
      if (m_Color.setString(LPCTSTR(dummy)) == GS_BAD)
      {       
         gsui_alert(_T("Il colore non è valido."));
         return;
      }
   }
   else
      if (m_Multi == false)
         m_Color.setByLayer();
      else
         m_Color.setNone(); // valore non valido che significa non settato

   m_DescriptionEdit.GetWindowText(dummy);
   m_Description = LPCTSTR(dummy);

   m_LineTypeEdit.GetWindowText(dummy);
   m_LineType = LPCTSTR(dummy);
   m_LineType.alltrim();
   if (m_Multi == false)
      if (m_LineType.len() == 0) m_LineType = DEFAULT_LINETYPE;

   m_LayerEdit.GetWindowText(dummy);
   m_Layer = LPCTSTR(dummy);
   m_Layer.alltrim();
   if (m_Multi == false)
      if (m_Layer.len() == 0) m_Layer = DEFAULT_LAYER;

   OnOK();
}

void CGridPropContourDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Visualizzazionecurvedilivello);
}
