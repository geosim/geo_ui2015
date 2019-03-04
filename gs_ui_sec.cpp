/**********************************************************
Name: GS_UI_SEC
                                   
Module description: File funzioni per le tabelle secondarie
                    di GEOUI (GEOsim User Interface)
            
Author: Roberto Poltini

(c) Copyright 2002-2012 by IREN ACQUA GAS  S.p.A

**********************************************************/


/**********************************************************/
/*   INCLUDE                                              */
/**********************************************************/

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"

#include "gs_error.h" 
#include "gs_def.h"       // definizioni globali
#include "gs_class.h" 
#include "gs_sec.h"
#include "gs_area.h" 
#include "gs_init.h"
#include "gs_resbf.h"
#include "gs_utily.h"

#include "gs_ui_utily.h"
#include "gs_ui_sec.h"
#include "ValuesListDlg.h"
#include "gs_ui_query.h"
#include "gs_ui_organizer.h"

#include "d2hMap.h" // doc to help


#define IDOBJSSEL   13


/*************************************************************************/
/*.doc gsuiInsDynSegmentationData                                        */
/*+
   Comando per inserire un scheda secondaria che supporti la segmentazione dinamica.
-*/
/*************************************************************************/
int gsuiInsDynSegmentationData(void)
{
	ads_name             ent;
	ads_point            pt1, pt2;
   double               dist1, dist2;
   C_CLASS              *pCls;
   C_SELSET             SelSet;
   long                 ent_id, sec_id;
   C_SINTH_SEC_TAB_LIST SinthSecList;
   C_SINTH_SEC_TAB      *pSinthSec;
   C_SECONDARY          *pSecondary;
   C_RB_LIST            MotherColValues, ColValues;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   // Selezione di un punto di partenza
   do
   {
      if (acedEntSel(_T("\nSpecificare punto iniziale: "), ent, pt1) != RTNORM) return GS_CAN;
      if (gsc_getClosestPointTo(ent, pt1, pt1) != GS_GOOD) return GS_BAD;

      if ((pCls = get_GS_CURRENT_WRK_SESSION()->find_class(ent)) == NULL ||
          pCls->is_extracted() != GS_GOOD)
      {
         gsui_alert(_T("Entità non valida"));
         continue;
      }

      // Verifico se la classe ha delle tabelle secondarie che supportano la segmentazione dinamica
      if (pCls->get_pPrj()->getSinthClsSecondaryTabList(pCls->ptr_id()->code, pCls->ptr_id()->sub_code, SinthSecList) == GS_BAD)
         return GS_BAD;
      SinthSecList.FilterOnDynSegmentationSupportedOnly();

      if (SinthSecList.get_count() == 0)
      {
         gsui_alert(_T("Entità appartenente a una classe non abilitata alla segmentazione dinamica"));
         continue;
      }

      // Verifico se l'entità GEOsim è modificabile
      if (pCls->get_Key_SelSet(ent, &ent_id, SelSet) == GS_BAD) break;
      if (pCls->is_updateableSS(ent_id, SelSet, NULL, GS_GOOD, GS_GOOD) != GS_GOOD)
      {
         gsui_alert(_T("Entità non modificabile"));
         continue;
      }
      break;
   }
   while (1);

   if (SinthSecList.get_count() > 1)
   {  // Scelta della tabella secondaria
      CValuesListDlg ListDlg;
      C_STRING       Buffer;

      ListDlg.m_Title      = _T("GEOsim - Segmentazione dinamica");
      ListDlg.m_Msg        = _T("Scegli una tabella secondaria abilitata:");
      ListDlg.m_ColsHeader = _T("Tabella secondaria");
      ListDlg.m_SingleSel  = TRUE;
      ListDlg.m_OriginType = CValuesListDlg::RESBUF_LIST;

      SinthSecList.sort_name();
      ListDlg.RbList << acutBuildList(RTLB, 0);
      pSinthSec = (C_SINTH_SEC_TAB *) SinthSecList.get_head();
      while (pSinthSec)
      {
         ListDlg.RbList += acutBuildList(RTLB, RTSTR, pSinthSec->get_name(), RTLE, 0);

         pSinthSec = (C_SINTH_SEC_TAB *) SinthSecList.get_next();
      }
      ListDlg.RbList += acutBuildList(RTLE, 0);

      if (ListDlg.DoModal() != IDOK || ListDlg.m_ValueList.get_count() != 1) return GS_CAN;

      // getptr_at (1 è il primo)
      pSinthSec = (C_SINTH_SEC_TAB *) SinthSecList.getptr_at(ListDlg.m_ValueList.get_head()->get_key() + 1);    
   }
   else
      pSinthSec = (C_SINTH_SEC_TAB *) SinthSecList.get_head();    

   if ((pSecondary = (C_SECONDARY *) pCls->find_sec(pSinthSec->get_key())) == NULL)
      return GS_BAD;

   if ((dist1 = gsc_getDistAtPoint(ent, pt1)) == -1) return GS_BAD;

   if (pSecondary->get_default_values(ColValues) == GS_BAD) return GS_BAD;
   // Se si tratta di eventi lineari devo chiedere il punto finale
   if (pSecondary->DynSegmentationType() == GSLinearDynSegmentation)
   {
      if (acedGetPoint(NULL, _T("\nSpecificare punto finale: "), pt2) != RTNORM)
         return GS_CAN;
      if (gsc_getClosestPointTo(ent, pt2, pt2) != GS_GOOD) return GS_BAD;

      if ((dist2 = gsc_getDistAtPoint(ent, pt2)) == -1) return GS_BAD;

      if (dist1 > dist2) // se dist1 è più grande di dist2
      {  // inverto dist1 e dist2
         double dummy = dist1;
         dist1 = dist2;
         dist2 = dummy;
      }

      ColValues.CdrAssocSubst(pSecondary->info.real_final_distance_attrib.get_name(), dist2);
      if (pSecondary->info.nominal_final_distance_attrib.len() > 0)
         ColValues.CdrAssocSubst(pSecondary->info.nominal_final_distance_attrib.get_name(), dist2);
   }

   ColValues.CdrAssocSubst(pSecondary->info.real_init_distance_attrib.get_name(), dist1);
   if (pSecondary->info.nominal_init_distance_attrib.len() > 0)
      ColValues.CdrAssocSubst(pSecondary->info.nominal_init_distance_attrib.get_name(), dist1);

   if (pSecondary->ins_data(ent_id, ColValues, &sec_id) == GS_BAD)
   {
      gsui_alert(_T("Inserimento fallito."));
      gsc_print_error();
      return GS_BAD;
   }

   if (pCls->query_data(ent_id, MotherColValues) == GS_GOOD)
   {  // apro finestra
      CSecDBValuesListDlg m_SecDBValuesListDlg(NULL, pSecondary, sec_id);

      m_SecDBValuesListDlg.pActualCls = pCls;
      MotherColValues.copy(m_SecDBValuesListDlg.MotherColValues);

      m_SecDBValuesListDlg.DoModal();
   }

   return GS_GOOD;
}


//**********************************************************
// FINE INSERIMENTO SEGMENTAZIONE DINAMICA
// INIZIO CALIBRAZIONE SEGMENTAZIONE DINAMICA
//**********************************************************


/*************************************************************************/
/*.doc gsc_ui_getExtractedUpdateableDynSegmentationClassList             */
/*+
   Funzione che restituisce una lista contenente i codici delle classi
   e sottoclassi che supportano la segmentazione dinamica e 
   che siano estratte e modificabili.
   Parametri:
   C_FAMILY_LIST &ExtractedUpdateableDynSegmentationClassList;

   Restituisce GS_GOOD in caso di successo altrimenti GS_BAD
-*/
/*************************************************************************/
int gsc_ui_getExtractedUpdateableDynSegmentationClassList(C_FAMILY_LIST &ExtractedUpdateableDynSegmentationClassList)
{
   C_CLS_PUNT_LIST ExtrClsList, ExtrSubList;
   C_CLS_PUNT      *pExtrCls, *pExtrSub;
   C_CLASS         *pCls, *pSub;

   ExtractedUpdateableDynSegmentationClassList.remove_all();
   // Ottiene la lista delle classi estratte nella sessione corrente che supportino la segmentazione dinamica
   if (get_GS_CURRENT_WRK_SESSION()->get_pPrj()->extracted_class(ExtrClsList, 0, 0, GS_GOOD) == GS_BAD)
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

      ExtractedUpdateableDynSegmentationClassList.add_tail_int(pCls->ptr_id()->code);

      if (pCls->ptr_id()->category == CAT_EXTERN)
         if (((C_EXTERN *) pCls)->get_DynSegmentationSupportedSubs(ExtrSubList) == GS_GOOD)
         {
            pExtrSub = (C_CLS_PUNT *) ExtrSubList.get_head();
            while (pExtrSub)
            {
               pSub = (C_CLASS *) pExtrSub->cls;
               ((C_FAMILY *) ExtractedUpdateableDynSegmentationClassList.get_cursor())->relation.add_tail_int(pSub->ptr_id()->sub_code);

               pExtrSub = (C_CLS_PUNT *) ExtrSubList.get_next();
            }
         }

      pExtrCls = (C_CLS_PUNT *) ExtrClsList.get_next();
   }

   return GS_GOOD;
}


/*************************************************************************/
/*.doc gsuiCalibrateDynSegmentationData                                  */
/*+
    Comando per calibrare i valori nominali della sgmentazione dinamica.
-*/
/*************************************************************************/
int gsuiCalibrateDynSegmentationData(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride    myResources;
   INT_PTR                      ret;
   CDynSegmentationCalibrateDlg DynSegmentationCalibrateDlg;

   acedRetVoid();

   if (!get_GS_CURRENT_WRK_SESSION())
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   do
   {
   	if ((ret = DynSegmentationCalibrateDlg.DoModal()) == IDOBJSSEL)
      { // Selezione di una finestra
         gsc_ssget(NULL, NULL, NULL, NULL, DynSegmentationCalibrateDlg.SelSet);
         DynSegmentationCalibrateDlg.AutoSel = (DynSegmentationCalibrateDlg.SelSet.length() > 0) ? false : true;
      }
   }
   while (ret == IDOBJSSEL);

   if (ret == IDOK)
   {
      C_CLASS *pDynSegmCls, *pRefCls;

      if (!(pDynSegmCls = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(DynSegmentationCalibrateDlg.DynSegmCls,
                                                                                           DynSegmentationCalibrateDlg.DynSegmSub)))
         return GS_BAD;
      if (!(pRefCls = (C_CGRID *) get_GS_CURRENT_WRK_SESSION()->find_class(DynSegmentationCalibrateDlg.RefCls,
                                                                                       DynSegmentationCalibrateDlg.RefSub)))
         return GS_BAD;

      if (DynSegmentationCalibrateDlg.AutoSel)
      {
         if (pDynSegmCls->CalibrateDynSegmentationData(DynSegmentationCalibrateDlg.AllObjs, pRefCls,
                                                       DynSegmentationCalibrateDlg.DistAttrib,
                                                       DynSegmentationCalibrateDlg.Tolerance, GS_GOOD) == GS_BAD)
            return GS_BAD;
      }
      else
         if (pDynSegmCls->CalibrateDynSegmentationData(DynSegmentationCalibrateDlg.SelSet, pRefCls,
                                                       DynSegmentationCalibrateDlg.DistAttrib,
                                                       DynSegmentationCalibrateDlg.Tolerance, GS_GOOD) == GS_BAD)
            return GS_BAD;
   }

   return GS_GOOD;  
}

// CDynSegmentationCalibrateDlg dialog

IMPLEMENT_DYNAMIC(CDynSegmentationCalibrateDlg, CDialog)

CDynSegmentationCalibrateDlg::CDynSegmentationCalibrateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDynSegmentationCalibrateDlg::IDD, pParent)
{
   hBmpSelObjs = NULL;

   DynSegmCls = DynSegmSub = RefCls = RefSub = 0;
   AutoSel = true;
   gsc_ssget(_T("_X"), NULL, NULL, NULL, AllObjs);
   Tolerance = 0.0;
}

CDynSegmentationCalibrateDlg::~CDynSegmentationCalibrateDlg()
{
   if (hBmpSelObjs) DeleteObject((HGDIOBJ) hBmpSelObjs);
}

void CDynSegmentationCalibrateDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_GEOTREECTRL_REF, m_GeoTree_Ref);
   DDX_Control(pDX, IDC_DIST_ATTRIBUTE, m_DistAttribCombo);
   DDX_Control(pDX, IDC_TOLERANCE_EDIT, m_ToleranceEdit);
   DDX_Control(pDX, IDC_N_SELECTED, mLbl_nSelected);
   DDX_Control(pDX, IDC_BUTTON_OBJSSELECTION, m_ObjsSelection);
   DDX_Control(pDX, IDC_RADIO_SELECTALL, m_AutoSelection);
   DDX_Control(pDX, IDC_RADIO_MANUALSELECTION, m_ManualSelection);
}


BEGIN_MESSAGE_MAP(CDynSegmentationCalibrateDlg, CDialog)
   ON_BN_CLICKED(IDC_RADIO_SELECTALL, &CDynSegmentationCalibrateDlg::OnBnClickedRadioSelectall)
   ON_BN_CLICKED(IDC_RADIO_MANUALSELECTION, &CDynSegmentationCalibrateDlg::OnBnClickedRadioManualselection)
   ON_BN_CLICKED(IDC_BUTTON_OBJSSELECTION, &CDynSegmentationCalibrateDlg::OnBnClickedButtonObjsselection)
   ON_CBN_SELCHANGE(IDC_DIST_ATTRIBUTE, &CDynSegmentationCalibrateDlg::OnCbnSelchangeDistAttribute)
   ON_BN_CLICKED(IDOK, &CDynSegmentationCalibrateDlg::OnBnClickedOk)
   ON_EN_KILLFOCUS(IDC_TOLERANCE_EDIT, &CDynSegmentationCalibrateDlg::OnEnKillfocusToleranceEdit)
   ON_BN_CLICKED(IDHELP, &CDynSegmentationCalibrateDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


BOOL CDynSegmentationCalibrateDlg::OnInitDialog() 
{
   COLORREF      crFrom;
	HINSTANCE     Instance;
   C_STRING      Mask;
   C_FAMILY_LIST ExtractedUpdateableDynSegmentationClassList;
   C_FAMILY_LIST CodeList;

	CDialog::OnInitDialog();
	
   if (!get_GS_CURRENT_WRK_SESSION()) return TRUE;

   crFrom = RGB(255, 0, 0); // rosso
   // determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_SELECT_OBJS), RT_BITMAP);
   hBmpSelObjs = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SELECT_OBJS));
   gsui_SetBmpColorToDlgBkColor(hBmpSelObjs, crFrom);
   m_ObjsSelection.SetBitmap(hBmpSelObjs);

   // Ricavo una lista (in formato stringa) delle classi e sottoclassi
   // estratte nella sessione corrente che supportino la segmentazione dinamica
   gsc_ui_getExtractedUpdateableDynSegmentationClassList(ExtractedUpdateableDynSegmentationClassList);

   // Solo il progetto della sessione corrente
   m_GeoTree.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   // Solo le classi e sottoclassi indicate da ExtractedUpdateableDynSegmentationClassList
   ((C_FAMILY *) m_GeoTree.FilterOnCodes.get_cursor())->relation.paste_tail(ExtractedUpdateableDynSegmentationClassList);
   m_GeoTree.FilterOnExtracted = true; // Solo classi estratte
   m_GeoTree.FinalObjectType = GSSubClass; // Si vuole selezionare una classe o una sottoclasse
   if (DynSegmCls == 0)
   {  // non è stata inizializzato il codice della classe
      C_FAMILY      *pCode;

      pCode = (C_FAMILY *) ExtractedUpdateableDynSegmentationClassList.get_head(); // primo progetto
      if (pCode && pCode->relation.get_count() > 0) // se ci sono classi classi
      {
         pCode = (C_FAMILY *) pCode->relation.get_head(); // prima classe
         DynSegmCls = pCode->get_key();
         if (pCode->relation.get_count() == 0) // se non ci sono sottoclassi
            m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), DynSegmCls);
         else // se ci sono sottoclassi
         {
            pCode = (C_FAMILY *) pCode->relation.get_head(); // prima sottoclasse
            DynSegmSub = pCode->get_key();
            m_GeoTree.SetSelectedSub(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), DynSegmCls, DynSegmSub);
         }
      }
   }
   else
      if (DynSegmSub == 0)
         m_GeoTree.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), DynSegmCls);
      else
         m_GeoTree.SetSelectedSub(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), DynSegmCls, DynSegmSub);

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   m_GeoTree_Ref.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   m_GeoTree_Ref.SetSelectedPrj(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
   m_GeoTree_Ref.FinalObjectType = GSSubClass; // Si vuole selezionare una classe o una sottoclasse
   // tutte le classi puntuali
   m_GeoTree_Ref.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_NODE);
   m_GeoTree_Ref.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_TEXT);
   m_GeoTree_Ref.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_NODE);
   m_GeoTree_Ref.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_TEXT);
   m_GeoTree_Ref.FilterOnExtracted = true; // Solo classi estratte

   if (RefCls == 0) // NON è stato inizializzato il codice della classe di riferimento
   {
      int Cls, Sub;
      C_STRING CategoryFilter, TypeFilter;

      TypeFilter = Mask;
      if (gsui_getFirstExtractedClsSubCode(&Cls, &Sub, CategoryFilter, TypeFilter) == GS_GOOD)
         { RefCls = Cls; RefSub = Sub; }
   }

   if (RefCls > 0) // è stato inizializzato il codice della classe di riferimento
   {
      if (RefSub == 0)
         m_GeoTree_Ref.SetSelectedCls(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), RefCls);
      else
         m_GeoTree_Ref.SetSelectedSub(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), RefCls, RefSub);
   }
   m_GeoTree_Ref.Refresh();

   RefreshSelection();
   RefreshAttribList();

   int Pos = m_SrcNumericAttribNameList.getpos_name(DistAttrib.get_name(), FALSE);
   if (Pos > 0) m_DistAttribCombo.SetCurSel(Pos - 1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDynSegmentationCalibrateDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_CLASS  *pCls;

   m_SrcNumericAttribNameList.remove_all();
   while (m_DistAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo

   if (RefCls == 0) return;
   if (!(pCls = get_GS_CURRENT_WRK_SESSION()->find_class(RefCls, RefSub))) return;
   if (!pCls->ptr_attrib_list()) return;

   pAttrib = (C_ATTRIB *) pCls->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      // Salto l'attributo chiave e quelli non numerici
      if (pAttrib->name.comp(pCls->ptr_info()->key_attrib, false) != 0 &&
          gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD)
      {
         m_DistAttribCombo.AddString(pAttrib->name.get_name());
         m_SrcNumericAttribNameList.add_tail_str(pAttrib->name.get_name());
      }

      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_SrcNumericAttribNameList.sort_name(); // li ordino in modo crescente
}

void CDynSegmentationCalibrateDlg::RefreshSelection()
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

   Msg = Tolerance;
   m_ToleranceEdit.SetWindowText(Msg.get_name());
}

// CDynSegmentationCalibrateDlg message handlers

void CDynSegmentationCalibrateDlg::OnBnClickedRadioSelectall()
{
   AutoSel = true;
   RefreshSelection();
}

void CDynSegmentationCalibrateDlg::OnBnClickedRadioManualselection()
{
   AutoSel = false;
   RefreshSelection();
}

void CDynSegmentationCalibrateDlg::OnBnClickedButtonObjsselection()
{
   EndDialog(IDOBJSSEL); // This value is returned by DoModal!
}

void CDynSegmentationCalibrateDlg::OnCbnSelchangeDistAttribute()
{
   if (m_DistAttribCombo.GetCurSel() == CB_ERR) return;
   DistAttrib = m_SrcNumericAttribNameList.getptr_at(m_DistAttribCombo.GetCurSel() + 1)->get_name();
}
BEGIN_EVENTSINK_MAP(CDynSegmentationCalibrateDlg, CDialog)
   ON_EVENT(CDynSegmentationCalibrateDlg, IDC_GEOTREECTRL, 1, CDynSegmentationCalibrateDlg::ChangeSelectionGeotreectrl, VTS_NONE)
   ON_EVENT(CDynSegmentationCalibrateDlg, IDC_GEOTREECTRL_REF, 1, CDynSegmentationCalibrateDlg::ChangeSelectionGeotreectrlRef, VTS_NONE)
END_EVENTSINK_MAP()

void CDynSegmentationCalibrateDlg::ChangeSelectionGeotreectrl()
{
   DynSegmCls = m_GeoTree.GetSelected(GSClass);
   DynSegmSub = m_GeoTree.GetSelected(GSSubClass);
}

void CDynSegmentationCalibrateDlg::ChangeSelectionGeotreectrlRef()
{
   RefCls = m_GeoTree_Ref.GetSelected(GSClass);
   RefSub = m_GeoTree_Ref.GetSelected(GSSubClass);
   RefreshAttribList();
}

void CDynSegmentationCalibrateDlg::OnBnClickedOk()
{
   bool NoEntities = false;

   if (DynSegmCls == 0)
   {
      gsui_alert(_T("Scegliere una classe da calibrare."));
      return;
   }
   if (AutoSel)
      { if (AllObjs.length() == 0) NoEntities = true; }
   else
      { if (SelSet.length() == 0) NoEntities = true; }
   if (NoEntities)
   {
      gsui_alert(_T("Nessun oggetto selezionato per la calibrazione."));
      return;
   }

   if (RefCls == 0)
   {
      gsui_alert(_T("Scegliere una classe di riferimento distanziometrico."));
      return;
   }
   if (DistAttrib.len() == 0)
   {
      gsui_alert(_T("Scegliere un attributo da cui leggere le distanze."));
      return;
   }
   if (Tolerance < 0)
   {
      gsui_alert(_T("La tolleranza non può essere negativa."));
      return;
   }

   OnOK();
}

void CDynSegmentationCalibrateDlg::OnEnKillfocusToleranceEdit()
{
   CString dummy;
   C_STRING Msg;

   m_ToleranceEdit.GetWindowText(dummy);
   Msg = LPCTSTR(dummy);
   if (Msg.tof() >= 0) 
      Tolerance = Msg.tof();
   else 
   {
      gsui_alert(_T("La tolleranza non può essere negativa."));
      Tolerance = 0.0;
   }

   RefreshSelection();
}

void CDynSegmentationCalibrateDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Calibrazione);
}


//**********************************************************
// FINE CALIBRAZIONE SEGMENTAZIONE DINAMICA
// INIZIO INIZIALIZZAZIONE SEGMENTAZIONE DINAMICA
//**********************************************************


/*************************************************************************/
/*.doc gsui_InsDynSegmentationData                                       */
/*+
   Funzione LISP per inizializzare i campi della tabella secondaria
   usati dalla segmentazione dinamica.
-*/
/*************************************************************************/
int gsui_InitDynSegmentation(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   CDynSegmentationInitDlg   DynSegmentationInitDlg;
   C_RB_LIST                 ret;

   acedRetNil();
   DynSegmentationInitDlg.FromRbList(acedGetArgs());
   if (DynSegmentationInitDlg.DoModal() == IDOK && (ret << DynSegmentationInitDlg.ToRbList()))
      ret.LspRetList();

   return GS_GOOD;
}


// CDynSegmentationInitDlg dialog

IMPLEMENT_DYNAMIC(CDynSegmentationInitDlg, CDialog)

CDynSegmentationInitDlg::CDynSegmentationInitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDynSegmentationInitDlg::IDD, pParent)
{
}

CDynSegmentationInitDlg::~CDynSegmentationInitDlg()
{
}

void CDynSegmentationInitDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_REAL_INIT_DISTANCE_ATTRIB_COMBO, m_RealInitDistanceAttribCombo);
   DDX_Control(pDX, IDC_NOMINAL_INIT_DISTANCE_ATTRIB_COMBO, m_NominalInitDistanceAttribCombo);
   DDX_Control(pDX, IDC_REAL_FINAL_DISTANCE_ATTRIB_COMBO, m_RealFinalDistanceAttribCombo);
   DDX_Control(pDX, IDC_NOMINAL_FINAL_DISTANCE_ATTRIB_COMBO, m_NominalFinalDistanceAttribCombo);
   DDX_Control(pDX, IDC_REAL_OFFSET_ATTRIB_COMBO, m_RealOffsetAttribCombo);
   DDX_Control(pDX, IDC_PUNCTUAL_RADIO, m_PunctualRadio);
   DDX_Control(pDX, IDC_LINEAR_RADIO, m_LinearRadio);
}


BEGIN_MESSAGE_MAP(CDynSegmentationInitDlg, CDialog)
   ON_CBN_SELCHANGE(IDC_REAL_INIT_DISTANCE_ATTRIB_COMBO, &CDynSegmentationInitDlg::OnCbnSelchangeRealInitDistanceAttribCombo)
   ON_CBN_SELCHANGE(IDC_NOMINAL_INIT_DISTANCE_ATTRIB_COMBO, &CDynSegmentationInitDlg::OnCbnSelchangeNominalInitDistanceAttribCombo)
   ON_CBN_SELCHANGE(IDC_REAL_FINAL_DISTANCE_ATTRIB_COMBO, &CDynSegmentationInitDlg::OnCbnSelchangeRealFinalDistanceAttribCombo)
   ON_CBN_SELCHANGE(IDC_NOMINAL_FINAL_DISTANCE_ATTRIB_COMBO, &CDynSegmentationInitDlg::OnCbnSelchangeNominalFinalDistanceAttribCombo)
   ON_CBN_SELCHANGE(IDC_REAL_OFFSET_ATTRIB_COMBO, &CDynSegmentationInitDlg::OnCbnSelchangeRealOffsetAttribCombo)
   ON_BN_CLICKED(IDC_PUNCTUAL_RADIO, &CDynSegmentationInitDlg::OnBnClickedPunctualRadio)
   ON_BN_CLICKED(IDC_LINEAR_RADIO, &CDynSegmentationInitDlg::OnBnClickedLinearRadio)
   ON_BN_CLICKED(IDOK, &CDynSegmentationInitDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CDynSegmentationInitDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


void CDynSegmentationInitDlg::RefreshSelection()
{
   CRect rcItem;
   int   WndHeight, nCmdShow;

   GetWindowRect(rcItem);

   if (DynSegmentationType == GSPunctualDynSegmentation)
   {
      m_PunctualRadio.SetCheck(BST_CHECKED);
      m_LinearRadio.SetCheck(BST_UNCHECKED);

      SetWindowPos(NULL, rcItem.left, rcItem.top, rcItem.Width(), 225, SWP_SHOWWINDOW);
      GetClientRect(rcItem);
      WndHeight = rcItem.Height();

      CWnd *p = GetDlgItem(IDC_REAL_INIT_DISTANCE_ATTRIB_LBL);
      GetDlgItem(IDC_REAL_INIT_DISTANCE_ATTRIB_LBL)->SetWindowText(_T("Distanza:"));
      GetDlgItem(IDC_NOMINAL_INIT_DISTANCE_ATTRIB_LBL)->SetWindowText(_T("Distanza nominale:"));

      nCmdShow = SW_HIDE; // invisibile

      GetDlgItem(IDC_DISTANCE_GROUP)->GetWindowRect(rcItem); ScreenToClient(rcItem);
      GetDlgItem(IDC_DISTANCE_GROUP)->SetWindowPos(NULL, rcItem.left, rcItem.top, rcItem.Width(), 80, SWP_SHOWWINDOW);
   }
   else
   {
      m_PunctualRadio.SetCheck(BST_UNCHECKED);
      m_LinearRadio.SetCheck(BST_CHECKED);

      SetWindowPos(NULL, rcItem.left, rcItem.top, rcItem.Width(), 349, SWP_SHOWWINDOW);
      GetClientRect(rcItem);
      WndHeight = rcItem.Height();

      GetDlgItem(IDC_REAL_INIT_DISTANCE_ATTRIB_LBL)->SetWindowText(_T("Distanza iniziale:"));
      GetDlgItem(IDC_NOMINAL_INIT_DISTANCE_ATTRIB_LBL)->SetWindowText(_T("Distanza nominale iniziale:"));

      nCmdShow = SW_SHOW; // visibile

      GetDlgItem(IDC_DISTANCE_GROUP)->GetWindowRect(rcItem); ScreenToClient(rcItem);
      GetDlgItem(IDC_DISTANCE_GROUP)->SetWindowPos(NULL, rcItem.left, rcItem.top, rcItem.Width(), 203, SWP_SHOWWINDOW);
   }

   GetDlgItem(IDC_REAL_FINAL_DISTANCE_ATTRIB_LBL)->ShowWindow(nCmdShow);
   GetDlgItem(IDC_REAL_FINAL_DISTANCE_ATTRIB_COMBO)->ShowWindow(nCmdShow);
   GetDlgItem(IDC_FINAL_DIST_IMAGE)->ShowWindow(nCmdShow);
   GetDlgItem(IDC_NOMINAL_FINAL_DISTANCE_ATTRIB_LBL)->ShowWindow(nCmdShow);
   GetDlgItem(IDC_NOMINAL_FINAL_DISTANCE_ATTRIB_COMBO)->ShowWindow(nCmdShow);
   GetDlgItem(IDC_REAL_OFFSET_ATTRIB_LBL)->ShowWindow(nCmdShow);
   GetDlgItem(IDC_REAL_OFFSET_ATTRIB_COMBO)->ShowWindow(nCmdShow);
   GetDlgItem(IDC_OFFSET_DIST_IMAGE)->ShowWindow(nCmdShow);

   GetDlgItem(IDOK)->GetWindowRect(rcItem); ScreenToClient(rcItem);
   GetDlgItem(IDOK)->SetWindowPos(NULL, rcItem.left, WndHeight - rcItem.Height() - 10, rcItem.Width(), rcItem.Height(), SWP_SHOWWINDOW);
   GetDlgItem(IDCANCEL)->GetWindowRect(rcItem); ScreenToClient(rcItem);
   GetDlgItem(IDCANCEL)->SetWindowPos(NULL, rcItem.left, WndHeight - rcItem.Height() - 10, rcItem.Width(), rcItem.Height(), SWP_SHOWWINDOW);
   GetDlgItem(IDHELP)->GetWindowRect(rcItem); ScreenToClient(rcItem);
   GetDlgItem(IDHELP)->SetWindowPos(NULL, rcItem.left, WndHeight - rcItem.Height() - 10, rcItem.Width(), rcItem.Height(), SWP_SHOWWINDOW);
}

void CDynSegmentationInitDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_DBCONNECTION *pConn;

   m_NumericAttribNameList.remove_all();
   while (m_RealInitDistanceAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_NominalInitDistanceAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_RealFinalDistanceAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_NominalFinalDistanceAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_RealOffsetAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo

   m_NominalInitDistanceAttribCombo.AddString(_T("")); // Riga vuota perchè valore opzionale
   m_NominalFinalDistanceAttribCombo.AddString(_T("")); // Riga vuota perchè valore opzionale
   m_RealOffsetAttribCombo.AddString(_T("")); // Riga vuota perchè valore opzionale

   // Verifico la connessione OLE-DB
   if (UdlFile.get_name() &&
       (pConn = get_pDBCONNECTION_LIST()->get_Connection(UdlFile.get_name(),
                                                         &UdlProperties,
                                                         false,
                                                         GS_BAD)) != NULL)
   {
      attrib_list.init_ADOType(pConn);
      pAttrib = (C_ATTRIB *) attrib_list.get_head();

      while (pAttrib)
      {
         // Salto gli attributi non numerici
         if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD)
         {
            m_RealInitDistanceAttribCombo.AddString(pAttrib->name.get_name());
            m_NominalInitDistanceAttribCombo.AddString(pAttrib->name.get_name());
            m_RealFinalDistanceAttribCombo.AddString(pAttrib->name.get_name());
            m_NominalFinalDistanceAttribCombo.AddString(pAttrib->name.get_name());
            m_RealOffsetAttribCombo.AddString(pAttrib->name.get_name());

            m_NumericAttribNameList.add_tail_str(pAttrib->name.get_name());
         }

         pAttrib = (C_ATTRIB *) attrib_list.get_next();
      }
   }

   m_NumericAttribNameList.sort_name(); // li ordino in modo crescente
}


// CDynSegmentationInitDlg message handlers

BOOL CDynSegmentationInitDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   if (real_init_distance_attrib.len() > 0 && real_final_distance_attrib.len() > 0)
      DynSegmentationType = GSLinearDynSegmentation;
   else
      DynSegmentationType = GSPunctualDynSegmentation;

   RefreshSelection();
   RefreshAttribList();

   int Pos = m_NumericAttribNameList.getpos_name(real_init_distance_attrib.get_name(), FALSE);
   if (Pos > 0) m_RealInitDistanceAttribCombo.SetCurSel(Pos - 1);

   Pos = m_NumericAttribNameList.getpos_name(nominal_init_distance_attrib.get_name(), FALSE);
   if (Pos > 0) m_NominalInitDistanceAttribCombo.SetCurSel(Pos);

   Pos = m_NumericAttribNameList.getpos_name(real_final_distance_attrib.get_name(), FALSE);
   if (Pos > 0) m_RealFinalDistanceAttribCombo.SetCurSel(Pos - 1);

   Pos = m_NumericAttribNameList.getpos_name(nominal_final_distance_attrib.get_name(), FALSE);
   if (Pos > 0) m_NominalFinalDistanceAttribCombo.SetCurSel(Pos);

   Pos = m_NumericAttribNameList.getpos_name(real_offset_attrib.get_name(), FALSE);
   if (Pos > 0) m_RealOffsetAttribCombo.SetCurSel(Pos);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/****************************************************************/
/*.doc CDynSegmentationInitDlg::FromRbList           <external> */
/*+
  Questa funzione consente di inizializzare la connessione da una lista 
  di resbuf composta come di seguito:
  (<Connessione a DB><struttura tabella><dyn segm info>)
  dove:
  <Connessione a DB> = (<UDL file> ((<UDLPropName><UDLPropVal>)...))
  <dyn segm info> = (("REAL_INIT_DISTANCE_ATTRIB" <value>)
                     ("NOMINAL_INIT_DISTANCE_ATTRIB" <value>)
                     ("REAL_FINAL_DISTANCE_ATTRIB" <value>)
                     ("NOMINAL_FINAL_DISTANCE_ATTRIB" <value>)
                     ("REAL_OFFSET_ATTRIB" <value>))
-*/  
/****************************************************************/
void CDynSegmentationInitDlg::FromRbList(resbuf *RbList)
{
   presbuf pRb, pItem;

   UdlFile.clear();
   UdlProperties.remove_all();
   real_init_distance_attrib.clear();
   real_final_distance_attrib.clear();
   nominal_init_distance_attrib.clear();
   nominal_final_distance_attrib.clear();
   real_offset_attrib.clear();

   attrib_list.remove_all();

   pItem = gsc_nth(0, RbList); // Connessione a DB
   // Nome file UDL
   if ((pRb = gsc_CdrAssoc(_T("UDL_FILE"), pItem, FALSE)) && pRb->restype == RTSTR)
      UdlFile = pRb->resval.rstring;
   // Proprietà connessione
   if ((pRb = gsc_CdrAssoc(_T("UDL_PROP"), pItem, FALSE)))
      if (pRb->restype == RTSTR) // Le proprietà sono in forma di stringa
      {
         // traduco stringa in lista
         if (gsc_PropListFromConnStr(pRb->resval.rstring, UdlProperties) == GS_BAD) return;
      }
      else
      if (pRb->restype == RTLB) // Le proprietà sono in forma di lista
         if (gsc_getUDLProperties(&pRb, UdlProperties) == GS_BAD) return;

   // Conversione path UDLProperties da assoluto in dir relativo
   if (gsc_UDLProperties_nethost2drive(UdlFile.get_name(), UdlProperties) == GS_BAD)
      return;

   pItem = gsc_nth(1, RbList); // struttura della tabella
   attrib_list.from_rb(pItem);

   pItem = gsc_nth(2, RbList); // informazioni per la segmentazione dinamica
   if ((pRb = gsc_CdrAssoc(_T("REAL_INIT_DISTANCE_ATTRIB"), pItem, FALSE)) && pRb->restype == RTSTR)
      real_init_distance_attrib = gsc_alltrim(pRb->resval.rstring);
   if ((pRb = gsc_CdrAssoc(_T("REAL_FINAL_DISTANCE_ATTRIB"), pItem, FALSE)) && pRb->restype == RTSTR)
      real_final_distance_attrib = gsc_alltrim(pRb->resval.rstring);
   if ((pRb = gsc_CdrAssoc(_T("NOMINAL_INIT_DISTANCE_ATTRIB"), pItem, FALSE)) && pRb->restype == RTSTR)
      nominal_init_distance_attrib = gsc_alltrim(pRb->resval.rstring);
   if ((pRb = gsc_CdrAssoc(_T("NOMINAL_FINAL_DISTANCE_ATTRIB"), pItem, FALSE)) && pRb->restype == RTSTR)
      nominal_final_distance_attrib = gsc_alltrim(pRb->resval.rstring);
   if ((pRb = gsc_CdrAssoc(_T("REAL_OFFSET_ATTRIB"), pItem, FALSE)) && pRb->restype == RTSTR)
      real_offset_attrib = gsc_alltrim(pRb->resval.rstring);
}


/****************************************************************/
/*.doc CDynSegmentationInitDlg::ToRbList             <external> */
/*+
  Questa funzione restituisce il risultato della classe in una lista 
  di resbuf composta come di seguito:
  (("UDL_FILE" <Connection>) ("UDL_PROP" <Properties>) ("TABLE_REF" <value>))
  dove
  <Connection> = <file UDL> || <stringa di connessione>
  <Properties> = stringa delle proprietà | ((<prop1><value>)(<prop1><value>)...)
-*/  
/****************************************************************/
resbuf *CDynSegmentationInitDlg::ToRbList(void)
{
   C_RB_LIST RbList;

   RbList << acutBuildList(RTLB, RTSTR, _T("REAL_INIT_DISTANCE_ATTRIB"), 0);
   if ((RbList += gsc_str2rb(real_init_distance_attrib)) == NULL) return NULL;
   RbList += acutBuildList(RTLE, RTLB, RTSTR, _T("REAL_FINAL_DISTANCE_ATTRIB"), 0);
   if ((RbList += gsc_str2rb(real_final_distance_attrib)) == NULL) return NULL;
   RbList += acutBuildList(RTLE, RTLB, RTSTR, _T("NOMINAL_INIT_DISTANCE_ATTRIB"), 0);
   if ((RbList += gsc_str2rb(nominal_init_distance_attrib)) == NULL) return NULL;
   RbList += acutBuildList(RTLE, RTLB, RTSTR, _T("NOMINAL_FINAL_DISTANCE_ATTRIB"), 0);
   if ((RbList += gsc_str2rb(nominal_final_distance_attrib)) == NULL) return NULL;
   RbList += acutBuildList(RTLE, RTLB, RTSTR, _T("REAL_OFFSET_ATTRIB"), 0);
   if ((RbList += gsc_str2rb(real_offset_attrib)) == NULL) return NULL;
   RbList += acutBuildList(RTLE, 0);
   RbList += attrib_list.to_rb();

   RbList.ReleaseAllAtDistruction(GS_BAD);

   return RbList.get_head();
}

void CDynSegmentationInitDlg::OnCbnSelchangeRealInitDistanceAttribCombo()
{
   if (m_RealInitDistanceAttribCombo.GetCurSel() == CB_ERR) return;
   real_init_distance_attrib = m_NumericAttribNameList.getptr_at(m_RealInitDistanceAttribCombo.GetCurSel() + 1)->get_name();
}

void CDynSegmentationInitDlg::OnCbnSelchangeNominalInitDistanceAttribCombo()
{
   if (m_NominalInitDistanceAttribCombo.GetCurSel() == CB_ERR) return;
   // La prima riga è vuota perchè è un valore opzionale
   if (m_NominalInitDistanceAttribCombo.GetCurSel() == 0)
      nominal_init_distance_attrib.clear();
   else
      nominal_init_distance_attrib = m_NumericAttribNameList.getptr_at(m_NominalInitDistanceAttribCombo.GetCurSel())->get_name();
}

void CDynSegmentationInitDlg::OnCbnSelchangeRealFinalDistanceAttribCombo()
{
   if (m_RealFinalDistanceAttribCombo.GetCurSel() == CB_ERR) return;
   real_final_distance_attrib = m_NumericAttribNameList.getptr_at(m_RealFinalDistanceAttribCombo.GetCurSel() + 1)->get_name();
}

void CDynSegmentationInitDlg::OnCbnSelchangeNominalFinalDistanceAttribCombo()
{
   if (m_NominalFinalDistanceAttribCombo.GetCurSel() == CB_ERR) return;
   // La prima riga è vuota perchè è un valore opzionale
   if (m_NominalFinalDistanceAttribCombo.GetCurSel() == 0)
      nominal_final_distance_attrib.clear();
   else
      nominal_final_distance_attrib = m_NumericAttribNameList.getptr_at(m_NominalFinalDistanceAttribCombo.GetCurSel())->get_name();
}

void CDynSegmentationInitDlg::OnCbnSelchangeRealOffsetAttribCombo()
{
   if (m_RealOffsetAttribCombo.GetCurSel() == CB_ERR) return;
   // La prima riga è vuota perchè è un valore opzionale
   if (m_RealOffsetAttribCombo.GetCurSel() == 0)
      real_offset_attrib.clear();
   else
      real_offset_attrib = m_NumericAttribNameList.getptr_at(m_RealOffsetAttribCombo.GetCurSel())->get_name();
}

void CDynSegmentationInitDlg::OnBnClickedPunctualRadio()
{
   DynSegmentationType = GSPunctualDynSegmentation;
   RefreshSelection();
}

void CDynSegmentationInitDlg::OnBnClickedLinearRadio()
{
   DynSegmentationType = GSLinearDynSegmentation;
   RefreshSelection();
}

void CDynSegmentationInitDlg::OnBnClickedOk()
{
   if (DynSegmentationType == GSPunctualDynSegmentation)
   {
      if (real_init_distance_attrib.get_name() == NULL)
      {
         gsui_alert(_T("Scegliere un attributo per la distanza."));
         return;
      }
      real_final_distance_attrib.clear();
      nominal_final_distance_attrib.clear();
      real_offset_attrib.clear();
   }
   else
   {
      if (real_init_distance_attrib.get_name() == NULL)
      {
         gsui_alert(_T("Scegliere un attributo per la distanza iniziale."));
         return;
      }
      if (real_final_distance_attrib.get_name() == NULL)
      {
         gsui_alert(_T("Scegliere un attributo per la distanza finale."));
         return;
      }
   }

   OnOK();
}

void CDynSegmentationInitDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Abilitazioneallasegmentazionedinamica);
}
