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
#include "gs_whip.h"

#include "gs_ui_utily.h"
#include "gs_ui_organizer.h"
#include "gs_ui_postgresql.h"

#include "d2hMap.h" // doc to help


const int BITMAP_WIDTH  = 16;
const int BITMAP_HEIGHT = 16;


bool classWithLinkedData(C_CLASS *pCls)
{
   if (pCls->ptr_info())
      if (pCls->ptr_info()->LinkedTable == true)
         return true;

   if (pCls->ptr_GphInfo())
      if (pCls->ptr_GphInfo()->getDataSourceType() == GSDBGphDataSource)
      {
         C_DBGPH_INFO *pGphInfo = (C_DBGPH_INFO *) pCls->ptr_GphInfo();
         
         if (pGphInfo->LinkedTable || pGphInfo->LinkedLblGrpTable || pGphInfo->LinkedLblTable)
            return true;
      }

   return false;
}


bool CheckFactors(CString &min, CString &max, HWND hOwnerWnd = NULL)
{
   CString     _value;
   long        MinFactor = -1, MaxFactor = -1;

   if (min.GetLength() > 0)
      if ((MinFactor = _wtol(min)) < 0)
      {
         gsui_alert(_T("Il fattore di scala minimo non può essere negativo."), hOwnerWnd);
         return false;
      }

   if (max.GetLength() > 0)
      if ((MaxFactor = _wtol(max)) < 0)
      {
         gsui_alert(_T("Il fattore di scala massimo non può essere negativo."), hOwnerWnd);
         return false;
      }

   if (MinFactor >= 0 && MaxFactor >= 0)
      if (MinFactor < MaxFactor) // rapporto di scala 1:x
      {
         gsui_alert(_T("Il fattore di scala minimo non può superare quello massimo."), hOwnerWnd);
         return false;
      }

   return true;
}


// CPGViewClassSelDlg dialog

IMPLEMENT_DYNAMIC(CPGViewClassSelDlg, CDialog)

CPGViewClassSelDlg::CPGViewClassSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPGViewClassSelDlg::IDD, pParent)
{
   Prj = 0;
   Geometry_columns_table_clean = true;
   OpMode = create;
}

CPGViewClassSelDlg::~CPGViewClassSelDlg()
{
}

void CPGViewClassSelDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_PROJECT_COMBO, m_ProjectComboCtrl);
   DDX_Control(pDX, IDC_GEOCLASSLISTCTRL, m_GEOClassListCtrl);
   DDX_Control(pDX, IDC_GEOMETRY_COLUMNS_CHECK, m_Geometry_columns_check);
   DDX_Control(pDX, IDC_PROGRESS, mProgressCtrl);
}

BOOL CPGViewClassSelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   m_GEOClassListCtrl.ObjectType = GSClass;
   m_GEOClassListCtrl.ColumnClassStatusVisibility = false;
   m_GEOClassListCtrl.MultiSelect = true;
   m_GEOClassListCtrl.ColumnAutosize = true;
   m_GEOClassListCtrl.UseFilterOnCodes = true;

   // OnInitPrjList viene eseguito dopo aver settato le proprietà di m_GEOClassListCtrl 
   // perchè lancia il metodo "update" di m_GEOClassListCtrl
   OnInitPrjList();

   m_Geometry_columns_check.SetCheck(BST_CHECKED);
   Geometry_columns_table_clean = true;

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CPGViewClassSelDlg::OnInitPrjList()
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
      else if (gsc_getLastUsedPrj(&Prj) == GS_GOOD)
         pSelPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj);

   if (pSelPrj)
   {
      m_ProjectComboCtrl.SetWindowText(pSelPrj->get_name());
      OnInitClsList();
   }
}


void CPGViewClassSelDlg::OnInitClsList()
{
   C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj);

   if (pPrj)
   {
      C_SINTH_CLASS_LIST SinthClassList;
      C_SINTH_CLASS      *pSinthClass;
      C_CLASS            *pCls;
      int                i = 1;

      if (pPrj->getSinthClassList(SinthClassList) == GS_BAD) return;

      mProgressCtrl.SetRange32(0, SinthClassList.get_count());

      pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_head();
      // Considero solo le classi con dati in PostgreSQL che non siano esterni
      while (pSinthClass)
      {
         mProgressCtrl.SetPos(i);
         i++;
         if ((pCls = pPrj->find_class(pSinthClass->get_key())) != NULL)
            // Le classi gestite sono quelle con db in PostGIS
            if (gsc_is_class_with_pg_data(pCls))
               // Scarto le tabelle collegate
               if (classWithLinkedData(pCls) == false)
                  m_GEOClassListCtrl.FilterOnCodes.add_tail_int(pSinthClass->get_key());

         pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_next();
      }
      mProgressCtrl.SetPos(0);
   }

   m_GEOClassListCtrl.FilterOnPrj = Prj;
   m_GEOClassListCtrl.LoadFromDB();
   m_GEOClassListCtrl.Refresh();
   m_GEOClassListCtrl.SetFocus();
}

BEGIN_MESSAGE_MAP(CPGViewClassSelDlg, CDialog)
   ON_CBN_SELCHANGE(IDC_PROJECT_COMBO, &CPGViewClassSelDlg::OnCbnSelchangeProjectCombo)
   ON_BN_CLICKED(IDOK, &CPGViewClassSelDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CPGViewClassSelDlg::OnBnClickedHelp)
   ON_BN_CLICKED(IDC_GEOMETRY_COLUMNS_CHECK, &CPGViewClassSelDlg::OnBnClickedGeometryColumnsCheck)
   ON_BN_CLICKED(ID_DEL_VIEW, &CPGViewClassSelDlg::OnBnClickedDelView)
END_MESSAGE_MAP()


// CPGViewClassSelDlg message handlers

void CPGViewClassSelDlg::OnCbnSelchangeProjectCombo()
{
   int       CurSel;
   CString   dummy;
   C_PROJECT *pPrj;

   if ((CurSel = m_ProjectComboCtrl.GetCurSel()) == CB_ERR) return;
   m_ProjectComboCtrl.GetLBText(CurSel, dummy);
   if (!(pPrj = (C_PROJECT *) get_GS_PROJECT()->search_name(dummy))) return;
   Prj = pPrj->get_key();
   OnInitClsList();
}

// creazione delle viste
void CPGViewClassSelDlg::OnBnClickedOk()
{
   if (Prj <= 0)
   {
      gsui_alert(_T("Selezionare il progetto di GEOsim."));
      return;
   }

   m_GEOClassListCtrl.GetSelectedCodes(ClsCodeList);
   if (ClsCodeList.get_count() == 0)
   {
      gsui_alert(_T("Selezionare almeno una classe di GEOsim."));
      return;
   }

   OpMode = create;

   OnOK();
}

// cancella zione delle viste
void CPGViewClassSelDlg::OnBnClickedDelView()
{
   if (Prj <= 0)
   {
      gsui_alert(_T("Selezionare il progetto di GEOsim."));
      return;
   }

   m_GEOClassListCtrl.GetSelectedCodes(ClsCodeList);
   if (ClsCodeList.get_count() == 0)
   {
      gsui_alert(_T("Selezionare almeno una classe di GEOsim."));
      return;
   }

   OpMode = erase;

   OnOK();
}

void CPGViewClassSelDlg::OnBnClickedHelp()
{
   gsc_help(IDH_CreazionevistePostgreSQLperaccessoaidatidaapplicazioniesterne);
}

void CPGViewClassSelDlg::OnBnClickedGeometryColumnsCheck()
{
   Geometry_columns_table_clean = (m_Geometry_columns_check.GetCheck() == BST_CHECKED) ? true : false;
}


/*************************************************************************/
/*.doc gsui_pgview                                                       */
/*+
    Comando per ricreare le viste in postgis utili per unire geometria 
    e dati alfanumerici.
-*/
/*************************************************************************/
int gsui_pgview(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   acedRetVoid();

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   CPGViewClassSelDlg PGViewClassSelDlg;

   if (PGViewClassSelDlg.DoModal() != IDOK) return GS_GOOD;

   if (PGViewClassSelDlg.OpMode == CPGViewClassSelDlg::create)
      return gsc_pgview(PGViewClassSelDlg.Prj, PGViewClassSelDlg.ClsCodeList, PGViewClassSelDlg.Geometry_columns_table_clean);
   else 
      return gsc_del_pgview(PGViewClassSelDlg.Prj, PGViewClassSelDlg.ClsCodeList, PGViewClassSelDlg.Geometry_columns_table_clean);
}


// CQGIS_ClassGroupDlg dialog

IMPLEMENT_DYNAMIC(CQGIS_ClassGroupDlg, CDialog)

CQGIS_ClassGroupDlg::CQGIS_ClassGroupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQGIS_ClassGroupDlg::IDD, pParent)
{
   Prev_iItem    = -1;
   Curr_iSubItem = -1;
}

CQGIS_ClassGroupDlg::~CQGIS_ClassGroupDlg()
{
}

BOOL CQGIS_ClassGroupDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_GroupListCtrl.SetExtendedStyle(m_GroupListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

   Refresh();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQGIS_ClassGroupDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_LIST, m_GroupListCtrl);
   DDX_Control(pDX, IDC_GENERIC_EDIT, m_GenericEdit);
}


BEGIN_MESSAGE_MAP(CQGIS_ClassGroupDlg, CDialog)
   ON_BN_CLICKED(IDC_ADD, &CQGIS_ClassGroupDlg::OnBnClickedAdd)
   ON_BN_CLICKED(IDC_REMOVE, &CQGIS_ClassGroupDlg::OnBnClickedRemove)
   ON_BN_CLICKED(IDC_REMOVE_ALL, &CQGIS_ClassGroupDlg::OnBnClickedRemoveAll)
   ON_NOTIFY(NM_CLICK, IDC_LIST, &CQGIS_ClassGroupDlg::OnNMClickList)
   ON_EN_KILLFOCUS(IDC_GENERIC_EDIT, &CQGIS_ClassGroupDlg::OnEnKillfocusGenericEdit)
   ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, &CQGIS_ClassGroupDlg::OnLvnKeydownList)
END_MESSAGE_MAP()


void CQGIS_ClassGroupDlg::SaveToFile(C_PROFILE_SECTION_BTREE &ProfileSections)
{
   C_BPROFILE_SECTION *ProfileSection;

   if (!(ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("QGISClassGroupList"))))
   {
      ProfileSections.add(_T("QGISClassGroupList"));
      ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.get_cursor();
   }

   ProfileSection->set_entryList(GroupList);

   return;
}


void CQGIS_ClassGroupDlg::LoadFromFile(C_PROFILE_SECTION_BTREE &ProfileSections)
{
   C_BPROFILE_SECTION *ProfileSection;
   C_2STR_BTREE       *pProfileEntries;
   C_B2STR            *pProfileEntry;
   
   if (!(ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("QGISClassGroupList"))))
      return;
   GroupList.remove_all();
   pProfileEntries = (C_2STR_BTREE *) ProfileSection->get_ptr_EntryList();
   pProfileEntry = (C_B2STR *) pProfileEntries->go_top();
   while (pProfileEntry)
   {
      GroupList.add_tail_2str(pProfileEntry->get_name(), pProfileEntry->get_name2());
      pProfileEntry = (C_B2STR *) pProfileEntries->go_next();
   }

   Refresh();

   return;
}

void CQGIS_ClassGroupDlg::Refresh(void)
{
	CRect   Rect;
   C_2STR  *pGroup;
	LV_ITEM lvitem;
   int     Width, iRow = 0;

   m_GroupListCtrl.DeleteAllItems();
   while (m_GroupListCtrl.DeleteColumn(0) != 0); // svuoto la lista

	m_GroupListCtrl.GetWindowRect(&Rect);
   Width = Rect.Width() - 4;

   m_GroupListCtrl.InsertColumn(0, _T("Nome gruppo"), LVCFMT_LEFT,
	                             Width * 12/20, 0);
   Width = Width - (Width * 12/20);
   m_GroupListCtrl.InsertColumn(1, _T("Maschera di raggruppamento"), LVCFMT_LEFT,
	                             Width, 0);

   lvitem.mask = LVIF_TEXT;

   pGroup = (C_2STR *) GroupList.get_head();
   while (pGroup)
   {  // Nome gruppo
      lvitem.iItem    = iRow++;
      lvitem.iSubItem = 0;
      lvitem.pszText  = pGroup->get_name();
   	m_GroupListCtrl.InsertItem(&lvitem);

      // Maschera di raggruppamento
      lvitem.iSubItem++;
      lvitem.pszText  = pGroup->get_name2();
      m_GroupListCtrl.SetItem(&lvitem);

      pGroup = (C_2STR *) GroupList.get_next();
   }

   if (m_GroupListCtrl.GetItemCount() > m_GroupListCtrl.GetCountPerPage())
      m_GroupListCtrl.SetColumnWidth(1, Width - 16); // per la barra di scorrimento verticale
}

void CQGIS_ClassGroupDlg::OnBnClickedAdd()
{
	LV_ITEM    lvitem;
   C_STRING   Alias;
   C_STR_LIST AvailableDriveList;
   int        i = 1;
   C_2STR    *pGroup;

   if ((pGroup = new C_2STR(_T(""), _T(""))) == NULL) return;
   if (GroupList.add_tail(pGroup) == GS_BAD) return;

   lvitem.iItem  = m_GroupListCtrl.GetItemCount();

	// Valore
   lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
   lvitem.lParam   = (LPARAM) pGroup;
   lvitem.iSubItem = 0;
   lvitem.pszText  = pGroup->get_name();
   m_GroupListCtrl.InsertItem(&lvitem);

	// Note
 	lvitem.mask     = LVIF_TEXT;
   lvitem.iSubItem++;
   lvitem.pszText  = pGroup->get_name2();
   m_GroupListCtrl.SetItem(&lvitem);

   // Lo seleziono
   m_GroupListCtrl.SetFocus();
   m_GroupListCtrl.SetItemState(lvitem.iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
   m_GroupListCtrl.EnsureVisible(lvitem.iItem, FALSE);
   Prev_iItem = lvitem.iItem;

   CRect rcItem;
   long  OffSetX, OffSetY, Width;

   m_GroupListCtrl.GetWindowRect(rcItem);
	ScreenToClient(rcItem);
   OffSetX = rcItem.left;
   OffSetY = rcItem.top;

   m_GroupListCtrl.GetSubItemRect(lvitem.iItem, 0, LVIR_LABEL, rcItem);
   OffSetX += 3;
   Width   = rcItem.Width() - 2;
   Curr_iSubItem = 0;

   m_GenericEdit.SetWindowText(_T(""));
	m_GenericEdit.SetWindowPos(NULL, rcItem.left + OffSetX, rcItem.top + OffSetY + 2,
   		                     Width, rcItem.Height() + 2, SWP_SHOWWINDOW);
   m_GenericEdit.SetFocus();
   m_GenericEdit.SetSel(0, -1); // Seleziona tutto il testo
}

void CQGIS_ClassGroupDlg::OnLvnKeydownList(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

   if (pLVKeyDow->wVKey == 46) // CANC
      OnBnClickedRemove();

	*pResult = 0;
}

void CQGIS_ClassGroupDlg::OnBnClickedRemove()
{
   POSITION iItem = m_GroupListCtrl.GetFirstSelectedItemPosition();
   
   if (iItem)
   {
      C_STRING Msg(_T("Cancellare il gruppo \""));
      CString  Value;

      Value = m_GroupListCtrl.GetItemText((int) iItem - 1, 0); // Valore
      Msg += Value;
      Msg += _T("\" ?");
      if (gsui_confirm(Msg.get_name(), GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
      {
         if (GroupList.search_name(Value)) GroupList.remove_at();
         m_GroupListCtrl.DeleteItem((int) iItem - 1); // cancella riga corrente
      }
   }		
}

void CQGIS_ClassGroupDlg::OnBnClickedRemoveAll()
{
   if (gsui_confirm(_T("Cancellare tutto il contenuto della lista ?"),
                    GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
   {
      GroupList.remove_all();
      Refresh();
   }
}

void CQGIS_ClassGroupDlg::OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
   CRect            rcItem;
   CString          ActualValue;
   long             OffSetX, OffSetY, Width;

   if (pNMItemActivate->iItem == -1) return; // nessuna selezione

   // se non era già stata selezionata questa casella
   if (Prev_iItem != pNMItemActivate->iItem)
   {
      Prev_iItem = pNMItemActivate->iItem;
      return;
   }

   m_GroupListCtrl.GetWindowRect(rcItem);
	ScreenToClient(rcItem);
   OffSetX = rcItem.left;
   OffSetY = rcItem.top;

   if (pNMItemActivate->iSubItem > 0)
   {
      m_GroupListCtrl.GetSubItemRect(pNMItemActivate->iItem, pNMItemActivate->iSubItem, LVIR_BOUNDS, rcItem);
      OffSetX += 7;
      Width   = rcItem.Width() - 6;
   }
   else
   {
      m_GroupListCtrl.GetSubItemRect(pNMItemActivate->iItem, pNMItemActivate->iSubItem, LVIR_LABEL, rcItem);
      OffSetX += 3;
      Width   = rcItem.Width() - 2;
   }

   ActualValue   = m_GroupListCtrl.GetItemText(pNMItemActivate->iItem, pNMItemActivate->iSubItem);
   Curr_iSubItem = pNMItemActivate->iSubItem;

   m_GenericEdit.SetWindowText(ActualValue);
	m_GenericEdit.SetWindowPos(NULL, rcItem.left + OffSetX, rcItem.top + OffSetY + 2,
   		                     Width, rcItem.Height() + 2, SWP_SHOWWINDOW);
   m_GenericEdit.SetFocus();
   m_GenericEdit.SetSel(0, -1); // Seleziona tutto il testo
	
   *pResult = 0;
}

void CQGIS_ClassGroupDlg::OnEnKillfocusGenericEdit()
{
   m_GenericEdit.ShowWindow(SW_HIDE);	

   C_2STR *pPrevValues;
   CString dummy;

	// Aggiorno valore in memoria
   dummy = m_GroupListCtrl.GetItemText(Prev_iItem, 0); // Valore
   if ((pPrevValues = (C_2STR *) GroupList.search_name(dummy)) == NULL) return;
   m_GenericEdit.GetWindowText(dummy);

   if (Curr_iSubItem == 0) // Valore
      pPrevValues->set_name(dummy);
   else // Note
      pPrevValues->set_name2(dummy);

   // Aggiorno alias in CListCtrl
   C_STRING Value(dummy);
	m_GroupListCtrl.SetItemText(Prev_iItem, Curr_iSubItem, Value.get_name());	
}

bool CQGIS_ClassGroupDlg::CheckValues()
{
   C_2STR *pGroup = (C_2STR *) GroupList.get_head();

   while (pGroup)
   {
      if (gsc_strlen(pGroup->get_name()) == 0)
      {
         gsui_alert(_T("Un raggruppamento di classi è senza nome."));
         return false;
      }
      if (gsc_strlen(pGroup->get_name2()) == 0)
      {
         C_STRING Msg;

         Msg = _T("Il raggruppamento di classi <");
         Msg += pGroup->get_name();
         Msg += _T("> è senza maschera di raggruppamento.");
         gsui_alert(Msg);
         return false;
      }

      pGroup = (C_2STR *) GroupList.get_next();
   }

   return true;
}


// CQGIS_ClassDlg dialog

IMPLEMENT_DYNAMIC(CQGIS_ClassDlg, CDialog)

CQGIS_ClassDlg::CQGIS_ClassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQGIS_ClassDlg::IDD, pParent)
{
   Prj = 0;
}

CQGIS_ClassDlg::~CQGIS_ClassDlg()
{
}

BOOL CQGIS_ClassDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   m_GEOSrcClassListCtrl.ObjectType = GSClass;
   m_GEOSrcClassListCtrl.ColumnClassStatusVisibility = false;
   m_GEOSrcClassListCtrl.MultiSelect = true;
   m_GEOSrcClassListCtrl.ColumnAutosize = true;
   m_GEOSrcClassListCtrl.UseFilterOnCodes = true;

   m_GEODstClassListCtrl.ObjectType = GSClass;
   m_GEODstClassListCtrl.ColumnClassStatusVisibility = false;
   m_GEODstClassListCtrl.MultiSelect = true;
   m_GEODstClassListCtrl.ColumnAutosize = true;
   m_GEODstClassListCtrl.UseFilterOnCodes = true;

   // OnInitPrjList viene eseguito dopo aver settato le proprietà di m_GEOClassListCtrl 
   // perchè lancia il metodo "update" di m_GEOClassListCtrl
   OnInitPrjList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQGIS_ClassDlg::OnInitPrjList()
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
      else if (gsc_getLastUsedPrj(&Prj) == GS_GOOD)
         pSelPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj);

   if (pSelPrj)
   {
      m_ProjectComboCtrl.SetWindowText(pSelPrj->get_name());
      OnInitClsList();
   }
}


void CQGIS_ClassDlg::OnInitClsList()
{
   C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj);

   m_GEOSrcClassListCtrl.FilterOnCodes.remove_all();
   m_GEODstClassListCtrl.FilterOnCodes.remove_all();

   if (pPrj)
   {
      C_SINTH_CLASS_LIST SinthClassList;
      C_SINTH_CLASS      *pSinthClass;
      C_CLASS            *pCls;
      int                i = 1;

      if (pPrj->getSinthClassList(SinthClassList) == GS_BAD) return;

      mProgressCtrl.SetRange32(0, SinthClassList.get_count());

      pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_head();
      // Considero solo le classi con dati in PostgreSQL
      while (pSinthClass)
      {
         mProgressCtrl.SetPos(i);
         i++;
         if ((pCls = pPrj->find_class(pSinthClass->get_key())) != NULL)
            // Le classi gestite sono quelle con db in PostGIS
            if (gsc_is_class_with_pg_data(pCls))
               // se non è già stata selezionata
               if (ClsCodeList.search_key(pSinthClass->get_key()) == NULL)
                  m_GEOSrcClassListCtrl.FilterOnCodes.add_tail_int(pSinthClass->get_key());
               else
                  m_GEODstClassListCtrl.FilterOnCodes.add_tail_int(pSinthClass->get_key());

         pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_next();
      }
      mProgressCtrl.SetPos(0);
   }

   m_GEOSrcClassListCtrl.ObjectType = GSClass;
   m_GEOSrcClassListCtrl.FilterOnPrj = Prj;
   m_GEOSrcClassListCtrl.LoadFromDB();
   m_GEOSrcClassListCtrl.Refresh();

   m_GEODstClassListCtrl.ObjectType = GSClass;
   m_GEODstClassListCtrl.FilterOnPrj = Prj;
   m_GEODstClassListCtrl.LoadFromDB();
   m_GEODstClassListCtrl.Refresh();
}

void CQGIS_ClassDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_PROJECT_COMBO, m_ProjectComboCtrl);
   DDX_Control(pDX, IDC_MIN_FACTOR_EDIT, m_GeomMinScaleFactor_Edit);
   DDX_Control(pDX, IDC_MAX_FACTOR_EDIT, m_GeomMaxScaleFactor_Edit);
   DDX_Control(pDX, IDC_GEOCLASSLISTCTRL, m_GEOSrcClassListCtrl);
   DDX_Control(pDX, IDC_GEOCLASSLIST_TO_EXPORT, m_GEODstClassListCtrl);
   DDX_Control(pDX, IDC_PROGRESS, mProgressCtrl);
}


BEGIN_MESSAGE_MAP(CQGIS_ClassDlg, CDialog)
   ON_BN_CLICKED(IDC_ADD_CLASS_BUTTON, &CQGIS_ClassDlg::OnBnClickedAddClassButton)
   ON_BN_CLICKED(IDC_DEL_CLASS_BUTTON, &CQGIS_ClassDlg::OnBnClickedDelClassButton)
   ON_EN_KILLFOCUS(IDC_MIN_FACTOR_EDIT, &CQGIS_ClassDlg::OnEnKillfocusMinFactorEdit)
   ON_EN_KILLFOCUS(IDC_MAX_FACTOR_EDIT, &CQGIS_ClassDlg::OnEnKillfocusMaxFactorEdit)
   ON_CBN_SELCHANGE(IDC_PROJECT_COMBO, &CQGIS_ClassDlg::OnCbnSelchangeProjectCombo)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_GEOCLASSLIST_TO_EXPORT, OnLvnItemchangedGeoclasslistToExport)
   //ON_NOTIFY(LVN_ITEMCHANGED, IDC_GEOCLASSLIST_TO_EXPORT, &CQGIS_ClassDlg::OnLvnItemchangedGeoclasslistToExport)
END_MESSAGE_MAP()


void CQGIS_ClassDlg::SaveToFile(C_PROFILE_SECTION_BTREE &ProfileSections)
{
   C_BPROFILE_SECTION      *ProfileSection;
   bool        Unicode = false;
   C_STRING    Buffer, Entry;
   C_2LONG_INT *pClsScaleFactors;

   if (!(ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("QGISClasses"))))
   {
      ProfileSections.add(_T("QGISClasses"));
      ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.get_cursor();
   }

   ProfileSection->set_entry(_T("Prj"), Prj);
   Buffer = ClsCodeList.to_str(_T(';'));
   ProfileSection->set_entry(_T("ClassList"), Buffer.get_name());

   pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.get_head();
   while (pClsScaleFactors)
   {
      Entry = _T("GeomMinScaleFactorCls");
      Entry += pClsScaleFactors->get_class_id();
      ProfileSection->set_entry(Entry.get_name(), pClsScaleFactors->get_id());

      Entry = _T("GeomMaxScaleFactorCls");
      Entry += pClsScaleFactors->get_class_id();
      ProfileSection->set_entry(Entry.get_name(), pClsScaleFactors->get_id_2());

      pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.get_next();
   }

   return;
}

void CQGIS_ClassDlg::LoadFromFile(C_PROFILE_SECTION_BTREE &ProfileSections)
{
   C_BPROFILE_SECTION *ProfileSection;
   C_2STR_BTREE       *pProfileEntries;
   C_B2STR            *pProfileEntry;
   C_STRING           EntryValue, Entry;
   C_INT              *pClsCode;
   long               MinFactor, MaxFactor;
   C_2LONG_INT        *pClsScaleFactors;
   int                i = 1;

   if (!(ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("QGISClasses"))))
      return;
   pProfileEntries = (C_2STR_BTREE *) ProfileSection->get_ptr_EntryList();

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("Prj")))) return;
   Prj = _wtoi(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("ClassList")))) return;
   EntryValue = pProfileEntry->get_name2();

   ClsScaleFactorsList.remove_all();
   ClsCodeList.from_str(EntryValue.get_name(), _T(';'));

   mProgressCtrl.SetRange32(0, ClsCodeList.get_count());

   pClsCode = (C_INT *) ClsCodeList.get_head();
   while (pClsCode)
   {
      mProgressCtrl.SetPos(i);
      i++;

      // verifico che la classe esista ancora
      if (gsc_find_class(Prj, pClsCode->get_key()) == NULL)
      {
         ClsCodeList.remove_at(); // cancella e va al successivo
         pClsCode = (C_INT *) ClsCodeList.get_cursor();
         continue;
      }
      Entry = _T("GeomMinScaleFactorCls");
      Entry += pClsCode->get_key();
      if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(Entry.get_name()))) return;
      MinFactor = _wtol(pProfileEntry->get_name2());

      Entry = _T("GeomMaxScaleFactorCls");
      Entry += pClsCode->get_key();
      if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(Entry.get_name()))) return;
      MaxFactor = _wtol(pProfileEntry->get_name2());

      if ((pClsScaleFactors = new C_2LONG_INT(MinFactor, MaxFactor, pClsCode->get_key())) == NULL)
         { set_GS_ERR_COD(eGSOutOfMem); return; } 
      ClsScaleFactorsList.add_tail(pClsScaleFactors);

      pClsCode = (C_INT *) ClsCodeList.get_next();
   }
   mProgressCtrl.SetPos(0);

   OnInitPrjList();
   OnInitClsList();

   return;
}

void CQGIS_ClassDlg::OnCbnSelchangeProjectCombo()
{
   int       CurSel;
   CString   dummy;
   C_PROJECT *pPrj;

   if ((CurSel = m_ProjectComboCtrl.GetCurSel()) == CB_ERR) return;
   m_ProjectComboCtrl.GetLBText(CurSel, dummy);
   if (!(pPrj = (C_PROJECT *) get_GS_PROJECT()->search_name(dummy))) return;
   Prj = pPrj->get_key();
   OnInitClsList();
}

void CQGIS_ClassDlg::OnBnClickedAddClassButton()
{
   C_INT_LIST  dummy;   
   C_INT       *pClsCode;
   C_2LONG_INT *pClsScaleFactors;
   C_CLASS     *pCls;
   long        MinFactor, MaxFactor;

   m_GEOSrcClassListCtrl.GetSelectedCodes(dummy);
   pClsCode = (C_INT *) dummy.get_head();
   while (pClsCode)
   {
      // se non è già stata selezionata
      if (ClsCodeList.search_key(pClsCode->get_key()) == NULL)
      {
         ClsCodeList.add_tail_int(pClsCode->get_key());
         // se non ci sono ancora i fattori di scala
         if ((pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.search_key(pClsCode->get_key())) == NULL)
         {
            if ((pCls = gsc_find_class(Prj, pClsCode->get_key())))
            {
               if (pCls->get_category() == CAT_EXTERN)
               {
                  MinFactor = 10000; // 1:10000
                  MaxFactor = 0;
               }
               else
                  switch (pCls->get_type())
                  {
                     case TYPE_POLYLINE:
                        MinFactor = 10000; // 1:10000
                        MaxFactor = 0;
                        break;
                     case TYPE_TEXT:
                        // con altezza = 1 il minimo fattore di scala = 1:500
                        // con altezza = 2 il minimo fattore di scala = 1:1000
                        MinFactor = (long) ((pCls->ptr_fas()) ? pCls->ptr_fas()->h_text * 500 : 500);
                        MaxFactor = 0;
                        break;
                     case TYPE_SURFACE:
                        MinFactor = 5000;
                        MaxFactor = 0;
                        break;
                     case TYPE_NODE:
                        MinFactor = 2000;
                        MaxFactor = 0;
                        break;
                     default: // spaghetti
                        MinFactor = 5000;
                        MaxFactor = 0;
                        break;
                  }
               
               if ((pClsScaleFactors = new C_2LONG_INT(MinFactor, MaxFactor, pClsCode->get_key())) == NULL)
                  { set_GS_ERR_COD(eGSOutOfMem); return; } 
               ClsScaleFactorsList.add_tail(pClsScaleFactors);
            }
         }
      }

      pClsCode = (C_INT *) pClsCode->get_next();
   }

   if (dummy.get_count() > 0) OnInitClsList();
}

void CQGIS_ClassDlg::OnBnClickedDelClassButton()
{
   C_INT_LIST dummy;   
   C_INT      *pClsCode;

   m_GEODstClassListCtrl.GetSelectedCodes(dummy);
   pClsCode = (C_INT *) dummy.get_head();
   while (pClsCode)
   {
      ClsCodeList.remove_key(pClsCode->get_key());

      pClsCode = (C_INT *) pClsCode->get_next();
   }

   if (dummy.get_count() > 0) OnInitClsList();
}


void CQGIS_ClassDlg::OnLvnItemchangedGeoclasslistToExport(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

   C_STRING    DstClsCodeList;
   C_INT_LIST  dummy;   
   C_INT       *pClsCode;
   C_2LONG_INT *pClsScaleFactors;
   long        PrevMinFactor = -1, MinFactor, MaxFactor, PrevMaxFactor = -1;
   bool        CheckMinFactor = true, CheckMaxFactor = true;

   m_GEODstClassListCtrl.GetSelectedCodes(dummy);
   if ((pClsCode = (C_INT *) dummy.get_head()))
      while (pClsCode && CheckMinFactor && CheckMaxFactor)
      {
         if ((pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.search_key(pClsCode->get_key())) == NULL)
            return;
         MinFactor = pClsScaleFactors->get_id();
         MaxFactor = pClsScaleFactors->get_id_2();

         if (PrevMinFactor == -1) PrevMinFactor = MinFactor;
         if (PrevMaxFactor == -1) PrevMaxFactor = MaxFactor;

         if (PrevMinFactor != MinFactor) CheckMinFactor = false;
         if (PrevMaxFactor != MaxFactor) CheckMaxFactor = false;

         pClsCode = (C_INT *) pClsCode->get_next();
      }
   else
   {
      CheckMinFactor = false;
      CheckMaxFactor = false;
   }

   if (CheckMinFactor) DstClsCodeList = MinFactor;
   else DstClsCodeList = _T("");
   m_GeomMinScaleFactor_Edit.SetWindowText(DstClsCodeList.get_name());
   
   if (CheckMaxFactor) DstClsCodeList = MaxFactor;
   else DstClsCodeList = _T("");
   m_GeomMaxScaleFactor_Edit.SetWindowText(DstClsCodeList.get_name());

   *pResult = 0;
}

void CQGIS_ClassDlg::OnLvnItemchangedGeoclasslistctrl2(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

   C_STRING    DstClsCodeList;
   C_INT_LIST  dummy;   
   C_INT       *pClsCode;
   C_2LONG_INT *pClsScaleFactors;
   long        PrevMinFactor = -1, MinFactor, MaxFactor, PrevMaxFactor = -1;
   bool        CheckMinFactor = true, CheckMaxFactor = true;

   m_GEODstClassListCtrl.GetSelectedCodes(dummy);
   pClsCode = (C_INT *) dummy.get_head();
   while (pClsCode && CheckMinFactor && CheckMaxFactor)
   {
      if ((pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.search_key(pClsCode->get_key())) == NULL)
         return;
      MinFactor = pClsScaleFactors->get_id();
      MaxFactor = pClsScaleFactors->get_id_2();

      if (PrevMinFactor == -1) PrevMinFactor = MinFactor;
      if (PrevMaxFactor == -1) PrevMaxFactor = MaxFactor;

      if (PrevMinFactor != MinFactor) CheckMinFactor = false;
      if (PrevMaxFactor != MaxFactor) CheckMaxFactor = false;

      pClsCode = (C_INT *) pClsCode->get_next();
   }

   if (CheckMinFactor) DstClsCodeList = MinFactor;
   else DstClsCodeList = _T("");
   m_GeomMinScaleFactor_Edit.SetWindowText(DstClsCodeList.get_name());
   
   if (CheckMaxFactor) DstClsCodeList = MaxFactor;
   else DstClsCodeList = _T("");
   m_GeomMaxScaleFactor_Edit.SetWindowText(DstClsCodeList.get_name());

   *pResult = 0;
}


void CQGIS_ClassDlg::OnEnKillfocusMinFactorEdit()
{
   CString     _value_min, _value_max;
   C_INT_LIST  dummy;   
   C_INT       *pClsCode;
   C_2LONG_INT *pClsScaleFactors;
   long        MinFactor;

   m_GeomMinScaleFactor_Edit.GetWindowText(_value_min);
   if (_value_min.GetLength() == 0) return;

   m_GeomMaxScaleFactor_Edit.GetWindowText(_value_max);

   if (CheckFactors(_value_min, _value_max) == false) return;

   MinFactor = _wtol(_value_min);
   m_GEODstClassListCtrl.GetSelectedCodes(dummy);
   pClsCode = (C_INT *) dummy.get_head();
   while (pClsCode)
   {
      if ((pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.search_key(pClsCode->get_key())))
         pClsScaleFactors->set_id(MinFactor);

      pClsCode = (C_INT *) pClsCode->get_next();
   }
}

void CQGIS_ClassDlg::OnEnKillfocusMaxFactorEdit()
{
   CString     _value_min, _value_max;
   C_INT_LIST  dummy;   
   C_INT       *pClsCode;
   C_2LONG_INT *pClsScaleFactors;
   long        MaxFactor;

   m_GeomMinScaleFactor_Edit.GetWindowText(_value_min);

   m_GeomMaxScaleFactor_Edit.GetWindowText(_value_max);
   if (_value_max.GetLength() == 0) return;

   if (CheckFactors(_value_min, _value_max) == false) return;

   MaxFactor = _wtol(_value_max);
   m_GEODstClassListCtrl.GetSelectedCodes(dummy);
   pClsCode = (C_INT *) dummy.get_head();
   while (pClsCode)
   {
      if ((pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.search_key(pClsCode->get_key())))
         pClsScaleFactors->set_id_2(MaxFactor);

      pClsCode = (C_INT *) pClsCode->get_next();
   }
}

bool CQGIS_ClassDlg::CheckValues()
{
   C_2LONG_INT *pClsScaleFactors;
   C_STRING    Msg;
   C_CLASS     *pCls;

   if (Prj <= 0)
   {
      gsui_alert(_T("Selezionare il progetto di GEOsim."));
      return false;
   }

   if (ClsCodeList.get_count() == 0)
   {
      gsui_alert(_T("Selezionare almeno una classe di GEOsim."));
      return false;
   }

   pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.get_head();
   while (pClsScaleFactors)
   {
      if (pClsScaleFactors->get_id() < 0)
      {
         Msg = _T("Il fattore di scala minimo per la geometria della classe <");
         if ((pCls = gsc_find_class(Prj, pClsScaleFactors->get_class_id())))
            Msg += pCls->get_name();
         Msg += _T("> non può essere negativo.");
         gsui_alert(Msg);
         return false;
      }

      if (pClsScaleFactors->get_id() < pClsScaleFactors->get_id_2()) // rapporto di scala 1:x
      {
         Msg = _T("Il fattore di scala minimo per la geometria della classe <");
         if ((pCls = gsc_find_class(Prj, pClsScaleFactors->get_class_id())))
            Msg += pCls->get_name();
         Msg += _T("> non può superare quello massimo.");
         gsui_alert(Msg);
         return false;
      }

      pClsScaleFactors = (C_2LONG_INT *) ClsScaleFactorsList.get_next();
   }

   return true;
}


// CQGISPropertiesDlg dialog

IMPLEMENT_DYNAMIC(CQGISPropertiesDlg, CDialog)

CQGISPropertiesDlg::CQGISPropertiesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQGISPropertiesDlg::IDD, pParent)
{

}

CQGISPropertiesDlg::~CQGISPropertiesDlg()
{
}

BOOL CQGISPropertiesDlg::OnInitDialog() 
{
   CString _value;

	CDialog::OnInitDialog();

   m_Port_Edit.GetWindowText(_value);
   if (_value.GetLength() == 0)
      m_Port_Edit.SetWindowText(_T("5432"));

   m_SRID_Edit.GetWindowText(_value);
   if (_value.GetLength() == 0)
      m_SRID_Edit.SetWindowText(_T("3003"));

   m_LblScaleFactor_Check.SetCheck(BST_CHECKED);
   m_LblMinScaleFactor_Edit.SetWindowText(_T("500")); // 1:500 
   m_LblMaxScaleFactor_Edit.SetWindowText(_T("0"));

   Refresh();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQGISPropertiesDlg::Refresh() 
{
   if (m_LblScaleFactor_Check.GetCheck() == BST_CHECKED)
   {
      m_LblMinScaleFactor_Edit.EnableWindow(TRUE);
      m_LblMaxScaleFactor_Edit.EnableWindow(TRUE);
   }
   else
   {
      m_LblMinScaleFactor_Edit.EnableWindow(FALSE);
      m_LblMaxScaleFactor_Edit.EnableWindow(FALSE);
   }

	return;
}

void CQGISPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_EDIT1, m_QGISPrjFile_Edit);
   DDX_Control(pDX, IDC_HOST_EDIT, m_Host_Edit);
   DDX_Control(pDX, IDC_PORT_EDIT, m_Port_Edit);
   DDX_Control(pDX, IDC_DATABASE_EDIT, m_DB_Edit);
   DDX_Control(pDX, IDC_USER_NAME_EDIT, m_UsrName_Edit);
   DDX_Control(pDX, IDC_PWD_EDIT, m_Pwd_Edit);
   DDX_Control(pDX, IDC_SRID_EDIT, m_SRID_Edit);
   DDX_Control(pDX, IDC_SCALE_FACTOR_CHECK, m_LblScaleFactor_Check);
   DDX_Control(pDX, IDC_MIN_FACTOR_EDIT2, m_LblMinScaleFactor_Edit);
   DDX_Control(pDX, IDC_MAX_FACTOR_EDIT2, m_LblMaxScaleFactor_Edit);
}


BEGIN_MESSAGE_MAP(CQGISPropertiesDlg, CDialog)
   ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CQGISPropertiesDlg::OnBnClickedBrowseButton)
   ON_BN_CLICKED(IDC_SCALE_FACTOR_CHECK, &CQGISPropertiesDlg::OnBnClickedScaleFactorCheck)
END_MESSAGE_MAP()


// CQGISPropertiesDlg message handlers
void CQGISPropertiesDlg::SaveToFile(C_PROFILE_SECTION_BTREE &ProfileSections)
{
   CString dummy;
   C_BPROFILE_SECTION *ProfileSection;

   if (!(ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("QGISProperties"))))
   {
      ProfileSections.add(_T("QGISProperties"));
      ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.get_cursor();
   }

   m_QGISPrjFile_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("PrjFile"), LPCTSTR(dummy));

   m_Host_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("Host"), LPCTSTR(dummy));

   m_Port_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("Port"), LPCTSTR(dummy));

   m_DB_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("DB"), LPCTSTR(dummy));

   m_UsrName_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("UsrName"), LPCTSTR(dummy));

   m_Pwd_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("Pwd"), LPCTSTR(dummy));

   m_SRID_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("SRID"), LPCTSTR(dummy));

   dummy = (m_LblScaleFactor_Check.GetCheck() == BST_CHECKED) ? _T("1") : _T("0");
   ProfileSection->set_entry(_T("LblScaleFactorUse"), LPCTSTR(dummy));

   m_LblMinScaleFactor_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("LblMinScaleFactor"), LPCTSTR(dummy));

   m_LblMaxScaleFactor_Edit.GetWindowText(dummy);
   ProfileSection->set_entry(_T("LblMaxScaleFactor"), LPCTSTR(dummy));

   return;
}

void CQGISPropertiesDlg::LoadFromFile(C_PROFILE_SECTION_BTREE &ProfileSections)
{
   C_BPROFILE_SECTION *ProfileSection;
   C_2STR_BTREE       *pProfileEntries;
   C_B2STR            *pProfileEntry;

   if (!(ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("QGISProperties"))))
      return;
   pProfileEntries = (C_2STR_BTREE *) ProfileSection->get_ptr_EntryList();

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("PrjFile")))) return;
   m_QGISPrjFile_Edit.SetWindowText(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("Host")))) return;
   m_Host_Edit.SetWindowText(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("Port")))) return;
   m_Port_Edit.SetWindowText(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("DB")))) return;
   m_DB_Edit.SetWindowText(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("UsrName")))) return;
   m_UsrName_Edit.SetWindowText(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("Pwd")))) return;
   m_Pwd_Edit.SetWindowText(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("SRID")))) return;
   m_SRID_Edit.SetWindowText(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("LblScaleFactorUse")))) return;
   m_LblScaleFactor_Check.SetCheck((gsc_strcmp(pProfileEntry->get_name2(), _T("1")) == 0) ? BST_CHECKED : BST_UNCHECKED);

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("LblMinScaleFactor")))) return;
   m_LblMinScaleFactor_Edit.SetWindowText(pProfileEntry->get_name2());

   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("LblMaxScaleFactor")))) return;
   m_LblMaxScaleFactor_Edit.SetWindowText(pProfileEntry->get_name2());

   Refresh();

   return;
}

void CQGISPropertiesDlg::OnBnClickedBrowseButton()
{
   C_STRING filename, str;
   CString  dummy;

   m_QGISPrjFile_Edit.GetWindowText(dummy);

   if (dummy.GetLength() == 0)
   {
      str = get_GEODIR();
      str += _T('\\');
      str += GEOSUPPORTDIR;
      str += _T('\\');

      if (gsc_get_tmp_filename(str.get_name(), _T("QGIS_Project"), _T(".qgs"), str) == GS_BAD)
         return;
   }
   else
      str = dummy;

   if (gsc_GetFileD(_T("GEOsim - Salva file di progetto Quantum GIS"),
                    str, _T("qgs"), UI_SAVEFILE_FLAGS, filename) == RTNORM)
      m_QGISPrjFile_Edit.SetWindowText(filename.get_name());
}

void CQGISPropertiesDlg::OnBnClickedScaleFactorCheck()
{
   Refresh();
}

bool CQGISPropertiesDlg::CheckValues()
{
   CString dummy;

   m_QGISPrjFile_Edit.GetWindowText(dummy);
   QGISPathFile = dummy.Trim();
   if (QGISPathFile.len() == 0)
   {
      gsui_alert(_T("Impostare il file di progetto Quantum GIS."));
      return false;
   }

   m_Host_Edit.GetWindowText(dummy);
   Host = dummy.Trim();
   if (Host.len() == 0)
   {
      gsui_alert(_T("Impostare l'host per il DB di PostgreSQL."));
      return false;
   }
      
   m_Port_Edit.GetWindowText(dummy);
   Port = dummy.Trim();
   if (Port.len() == 0)
   {
      gsui_alert(_T("Impostare la porta per il DB di PostgreSQL."));
      return false;
   }

   m_Port_Edit.GetWindowText(dummy);
   Database = dummy.Trim();
   if (Database.len() == 0)
   {
      gsui_alert(_T("Impostare il DataBase di PostgreSQL."));
      return false;
   }

   m_DB_Edit.GetWindowText(dummy);
   Database = dummy.Trim();
   if (Database.len() == 0)
   {
      gsui_alert(_T("Impostare il DataBase di PostgreSQL."));
      return false;
   }

   m_UsrName_Edit.GetWindowText(dummy);
   User = dummy.Trim();
   if (User.len() == 0)
   {
      gsui_alert(_T("Impostare il nome utente per la connessione a PostgreSQL."));
      return false;
   }

   m_Pwd_Edit.GetWindowText(dummy);
   Password = dummy;
   if (Password.len() == 0)
   {
      gsui_alert(_T("Impostare la parola segreta per la connessione a PostgreSQL."));
      return false;
   }

   m_SRID_Edit.GetWindowText(dummy);
   SRID = _wtoi(dummy.Trim());
   if (SRID <= 0)
   {
      gsui_alert(_T("Impostare il SRID del progetto di Quantum GIS."));
      return false;
   }

   UseLblScaleFactors = (m_LblScaleFactor_Check.GetCheck() == BST_CHECKED) ? true : false;
   if (UseLblScaleFactors)
   {
      long n1, n2;

      m_LblMinScaleFactor_Edit.GetWindowText(dummy);
      if (dummy.GetLength() == 0)
      {
         gsui_alert(_T("Impostare il fattore di scala minimo per le etichette."));
         return false;
      }
      n1 = _wtol(dummy.Trim());
      if ((n1 = _wtol(dummy.Trim())) < 0)
      {
         gsui_alert(_T("Il fattore di scala minimo per le etichette non può essere negativo."));
         return false;
      }

      m_LblMaxScaleFactor_Edit.GetWindowText(dummy);
      if (dummy.GetLength() == 0)
      {
         gsui_alert(_T("Impostare il fattore di scala massimo per le etichette."));
         return false;
      }
      n2 = _wtol(dummy.Trim());
      if (n1 < n2)  // rapporto di scala 1:x
      {
         gsui_alert(_T("Il fattore di scala minimo per le etichette non può superare quello massimo."));
         return false;
      }

      LblScaleFactors.set_id(n1);
      LblScaleFactors.set_id_2(n2);
   }

   return true;
}


// CQGIS_Dlg dialog

IMPLEMENT_DYNAMIC(CQGIS_Dlg, CDialog)

CQGIS_Dlg::CQGIS_Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQGIS_Dlg::IDD, pParent)
{
}

CQGIS_Dlg::~CQGIS_Dlg()
{
}

void CQGIS_Dlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_TAB, m_TabCtrl);
}


BEGIN_MESSAGE_MAP(CQGIS_Dlg, CDialog)
   ON_BN_CLICKED(IDHELP, &CQGIS_Dlg::OnBnClickedHelp)
   ON_BN_CLICKED(IDOK, &CQGIS_Dlg::OnBnClickedOk)
   ON_BN_CLICKED(IDSAVE, &CQGIS_Dlg::OnBnClickedSave)
   ON_BN_CLICKED(IDLOAD, &CQGIS_Dlg::OnBnClickedLoad)
   ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CQGIS_Dlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CQGIS_Dlg message handlers
BOOL CQGIS_Dlg::OnInitDialog() 
{
   CRect TabRect;

	CDialog::OnInitDialog();

   m_PropertiesDlg.Create(IDD_QGIS_PROPERTIES, this);
   m_ClassDlg.Create(IDD_QGIS_CLASS, this);
   m_ClassGroupDlg.Create(IDD_QGIS_CLASS_GRP, this);
   
   m_TabCtrl.InsertItem(0, _T("Parametri generali"));
   m_TabCtrl.InsertItem(1, _T("Classi di entità"));
   m_TabCtrl.InsertItem(2, _T("Raggruppamento classi"));

   m_TabCtrl.GetWindowRect(&TabRect);
   ScreenToClient(TabRect);
   int OffSetX = 1, OffSetY = 21;
   TabRect.MoveToXY(TabRect.left + OffSetX, TabRect.top + OffSetY);
   TabRect.bottom -= (OffSetY + 3); TabRect.right -= (OffSetX + 3);
   m_PropertiesDlg.MoveWindow(TabRect);
   m_ClassDlg.MoveWindow(TabRect);
   m_ClassGroupDlg.MoveWindow(TabRect);

   LRESULT Result;
   // Simulate selection of the first item.
   m_TabCtrl.SetCurSel(0);
   OnTcnSelchangeTab(NULL, &Result);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQGIS_Dlg::OnBnClickedHelp()
{
   switch (m_TabCtrl.GetCurSel())
   {
      case 0: // parametri generali
         gsc_help(IDH_SezioneParametrigenerali);		
         break;
      case 1: // selzione classi
         gsc_help(IDH_SezioneClassidientit);		
         break;
      case 2: // raggruppamento classi
         gsc_help(IDH_SezioneRaggruppamentoclassi);		
         break;
   }
}

void CQGIS_Dlg::OnBnClickedOk()
{
   if (m_PropertiesDlg.CheckValues() &&
       m_ClassDlg.CheckValues() &&
       m_ClassGroupDlg.CheckValues())
      OnOK();
}

void CQGIS_Dlg::OnBnClickedSave(void)
{
   C_STRING filename, str;

   str = get_GEODIR();
   str += _T('\\');
   str += GEOSUPPORTDIR;
   str += _T('\\');
   if (gsc_get_tmp_filename(str.get_name(), _T("QGIS_Project"), _T(".gs_x_qgs"), str) == GS_BAD)
      return;

   if (gsc_GetFileD(_T("GEOsim - Salva profilo per progetto Quantum GIS"),
                    str, _T("gs_x_qgs"), UI_SAVEFILE_FLAGS, filename) == RTNORM)
      SaveToFile(filename);
}

void CQGIS_Dlg::OnBnClickedLoad(void)
{
   C_STRING filename, str;

   str = get_GEODIR();
   str += _T('\\');
   str += GEOSUPPORTDIR;
   str += _T('\\');

   if (gsc_GetFileD(_T("GEOsim - Carica profilo per progetto Quantum GIS"),
                    str, _T("gs_x_qgs"), UI_LOADFILE_FLAGS, filename) == RTNORM)
      LoadFromFile(filename);
}

void CQGIS_Dlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
   switch (m_TabCtrl.GetCurSel())
   {
      case 0: // Parametri generali
	      m_PropertiesDlg.EnableWindow(TRUE);
	      m_PropertiesDlg.ShowWindow(SW_SHOW);
	      m_ClassDlg.EnableWindow(FALSE);
	      m_ClassDlg.ShowWindow(SW_HIDE);
	      m_ClassGroupDlg.EnableWindow(FALSE);
	      m_ClassGroupDlg.ShowWindow(SW_HIDE);
         break;
      case 1: // Classi di entità
	      m_PropertiesDlg.EnableWindow(FALSE);
	      m_PropertiesDlg.ShowWindow(SW_HIDE);
	      m_ClassDlg.EnableWindow(TRUE);
	      m_ClassDlg.ShowWindow(SW_SHOW);
	      m_ClassGroupDlg.EnableWindow(FALSE);
	      m_ClassGroupDlg.ShowWindow(SW_HIDE);
         break;
      case 2: // Raggruppamento classi
	      m_PropertiesDlg.EnableWindow(FALSE);
	      m_PropertiesDlg.ShowWindow(SW_HIDE);
	      m_ClassDlg.EnableWindow(FALSE);
	      m_ClassDlg.ShowWindow(SW_HIDE);
	      m_ClassGroupDlg.EnableWindow(TRUE);
	      m_ClassGroupDlg.ShowWindow(SW_SHOW);
         break;
   }
   *pResult = 0;
}

void CQGIS_Dlg::SaveToFile(C_STRING &Path)
{
   C_PROFILE_SECTION_BTREE ProfileSections;

   m_PropertiesDlg.SaveToFile(ProfileSections);
   m_ClassDlg.SaveToFile(ProfileSections);
   m_ClassGroupDlg.SaveToFile(ProfileSections);
   
   gsc_write_profile(Path.get_name(), ProfileSections);
}

void CQGIS_Dlg::LoadFromFile(C_STRING &Path)
{
   C_PROFILE_SECTION_BTREE ProfileSections;

   if (gsc_read_profile(Path, ProfileSections) == GS_BAD) return;

   m_PropertiesDlg.LoadFromFile(ProfileSections);
   m_ClassDlg.LoadFromFile(ProfileSections);
   m_ClassGroupDlg.LoadFromFile(ProfileSections);
}


/*************************************************************************/
/*.doc gsui_qgis                                                         */
/*+
    Comando per creare un progetto di Quantum GIS.
-*/
/*************************************************************************/
int gsui_qgis(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   acedRetVoid();

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   CQGIS_Dlg QGISProjectDlg;

   if (QGISProjectDlg.DoModal() != IDOK) return GS_GOOD;

   if (QGISProjectDlg.m_ClassDlg.Prj > 0 && QGISProjectDlg.m_ClassDlg.ClsCodeList.get_count() > 0)
   {
      C_2LONG *pLblScaleFactors = NULL;

      CQGISPropertiesDlg *pProps = &QGISProjectDlg.m_PropertiesDlg;
      CQGIS_ClassDlg     *pClasses = &QGISProjectDlg.m_ClassDlg;
      CQGIS_ClassGroupDlg *pClassgroups = &QGISProjectDlg.m_ClassGroupDlg;

      if (pProps->UseLblScaleFactors)
         pLblScaleFactors = &pProps->LblScaleFactors;

      return gsc_WriteQGISProjectFile(pProps->QGISPathFile, pClasses->Prj, 
                                      pProps->Host, pProps->Port, 
                                      pProps->Database, pProps->User, pProps->Password,
                                      pProps->SRID, 
                                      pClasses->ClsCodeList,
                                      &pClassgroups->GroupList,
                                      &pClasses->ClsScaleFactorsList,
                                      pLblScaleFactors);
   }

   return GS_GOOD;
}


// CHistoryClassSelDlg dialog


IMPLEMENT_DYNAMIC(CHistoryClassSelDlg, CDialog)

CHistoryClassSelDlg::CHistoryClassSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHistoryClassSelDlg::IDD, pParent)
{
   Prj = 0;
   OpMode = enable_and_createcls;
}

CHistoryClassSelDlg::~CHistoryClassSelDlg()
{
}

void CHistoryClassSelDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_PROJECT_COMBO, m_ProjectComboCtrl);
   DDX_Control(pDX, IDC_GEOCLASSLISTCTRL, m_GEOClassListCtrl);
   DDX_Control(pDX, IDC_PROGRESS, mProgressCtrl);
}


BEGIN_MESSAGE_MAP(CHistoryClassSelDlg, CDialog)
   ON_CBN_SELCHANGE(IDC_PROJECT_COMBO, &CHistoryClassSelDlg::OnCbnSelchangeProjectCombo)
   ON_BN_CLICKED(IDOK, &CHistoryClassSelDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CHistoryClassSelDlg::OnBnClickedHelp)
   ON_BN_CLICKED(ID_DEL_HISTORY, &CHistoryClassSelDlg::OnBnClickedDelHistory)
   ON_BN_CLICKED(ID_ENABLE_HISTORY, &CHistoryClassSelDlg::OnBnClickedEnableHistory)
END_MESSAGE_MAP()


// CHistoryClassSelDlg message handlers


BOOL CHistoryClassSelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   m_GEOClassListCtrl.ObjectType = GSClass;
   m_GEOClassListCtrl.ColumnClassStatusVisibility = false;
   m_GEOClassListCtrl.MultiSelect = true;
   m_GEOClassListCtrl.ColumnAutosize = true;
   m_GEOClassListCtrl.UseFilterOnCodes = true;

   // OnInitPrjList viene eseguito dopo aver settato le proprietà di m_GEOClassListCtrl 
   // perchè lancia il metodo "update" di m_GEOClassListCtrl
   OnInitPrjList();

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CHistoryClassSelDlg::OnInitPrjList()
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
      else if (gsc_getLastUsedPrj(&Prj) == GS_GOOD)
         pSelPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj);

   if (pSelPrj)
   {
      m_ProjectComboCtrl.SetWindowText(pSelPrj->get_name());
      OnInitClsList();
   }
}


void CHistoryClassSelDlg::OnInitClsList()
{
   C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj);

   if (pPrj)
   {
      C_SINTH_CLASS_LIST SinthClassList;
      C_SINTH_CLASS      *pSinthClass;
      C_CLASS            *pCls;
      int                i = 1;

      if (pPrj->getSinthClassList(SinthClassList) == GS_BAD) return;

      mProgressCtrl.SetRange32(0, SinthClassList.get_count());

      pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_head();
      // Considero solo le classi con dati in PostgreSQL
      while (pSinthClass)
      {
         mProgressCtrl.SetPos(i);
         i++;

         if ((pCls = pPrj->find_class(pSinthClass->get_key())) != NULL)
            // Le classi gestite sono quelle con db in PostGIS
            if (gsc_is_class_with_pg_data(pCls))
               // Scarto le tabelle collegate
               if (classWithLinkedData(pCls) == false)
                  m_GEOClassListCtrl.FilterOnCodes.add_tail_int(pSinthClass->get_key());

         pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_next();
      }
      mProgressCtrl.SetPos(0);
   }

   m_GEOClassListCtrl.FilterOnPrj = Prj;
   m_GEOClassListCtrl.LoadFromDB();
   m_GEOClassListCtrl.Refresh();
   m_GEOClassListCtrl.SetFocus();
}


void CHistoryClassSelDlg::OnCbnSelchangeProjectCombo()
{
   int       CurSel;
   CString   dummy;
   C_PROJECT *pPrj;

   if ((CurSel = m_ProjectComboCtrl.GetCurSel()) == CB_ERR) return;
   m_ProjectComboCtrl.GetLBText(CurSel, dummy);
   if (!(pPrj = (C_PROJECT *) get_GS_PROJECT()->search_name(dummy))) return;
   Prj = pPrj->get_key();
   OnInitClsList();
}

bool CHistoryClassSelDlg::checkSelections(void)
{
   if (Prj <= 0)
   {
      gsui_alert(_T("Selezionare il progetto di GEOsim."));
      return false;
   }

   m_GEOClassListCtrl.GetSelectedCodes(ClsCodeList);
   if (ClsCodeList.get_count() == 0)
   {
      gsui_alert(_T("Selezionare almeno una classe di GEOsim."));
      return false;
   }

   return true;
}

void CHistoryClassSelDlg::OnBnClickedOk()
{
   if (checkSelections() == false)
      return;

   OpMode = enable_and_createcls;

   OnOK();
}

void CHistoryClassSelDlg::OnBnClickedDelHistory()
{
   if (checkSelections() == false)
      return;

   OpMode = disable;

   gsui_info(_T("La storicizzazione sarà disabilitata ma le tabelle storiche non saranno cancellate."), m_hWnd);

   OnOK();
}

void CHistoryClassSelDlg::OnBnClickedEnableHistory()
{
   if (checkSelections() == false)
      return;

   OpMode = enable;

   OnOK();
}

void CHistoryClassSelDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Storicizzazionedeidati);
}


/*************************************************************************/
/*.doc gsui_CreateHistorySystem                                          */
/*+
    Comando per creare il sistema di storicizzazione per le classi di GEOsim.
-*/
/*************************************************************************/
int gsui_CreateHistorySystem(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   acedRetVoid();
 
   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   CHistoryClassSelDlg HistoryClassSelDlg;

   if (HistoryClassSelDlg.DoModal() != IDOK) return GS_GOOD;

   switch (HistoryClassSelDlg.OpMode)
   {
      case CHistoryClassSelDlg::enable_and_createcls:
         return gsc_CreateHistorySystem(HistoryClassSelDlg.Prj, true, HistoryClassSelDlg.ClsCodeList);
      case CHistoryClassSelDlg::enable:
         return gsc_EnableHistorySystem(HistoryClassSelDlg.Prj, HistoryClassSelDlg.ClsCodeList);
      case CHistoryClassSelDlg::disable:
         return gsc_DisableHistorySystem(HistoryClassSelDlg.Prj, HistoryClassSelDlg.ClsCodeList);
   }
   
   return GS_GOOD;
}
