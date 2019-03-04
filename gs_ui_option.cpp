/**********************************************************
Name: GS_UI_OPTION
                                   
Module description: File funzioni per gestire l'interfaccia grafica per
                    le opzioni di nodi rete e le impostazioni generali di GEOsim
            
Author: Roberto Poltini

(c) Copyright 2002-2012 by IREN ACQUA GAS  S.p.A

**********************************************************/

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include "gs_utily.h"
#include "gs_init.h"
#include "gs_ui_option.h"
#include "gs_ui_utily.h"

#include "d2hMap.h" // doc to help


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int BITMAP_WIDTH  = 16;
const int BITMAP_HEIGHT = 16;
const int HDR_BITMAP_WIDTH  = 8;
const int HDR_BITMAP_HEIGHT = 8;


/////////////////////////////////////////////////////////////////////////////
// COptionNet property page

IMPLEMENT_DYNCREATE(COptionNet, CPropertyPage)

COptionNet::COptionNet() : CPropertyPage(COptionNet::IDD)
{
   isChanged     = FALSE;
   Prev_iItem    = -1;
   Prev_iHeader  = -1;
   Curr_iSubItem = -1;

	//{{AFX_DATA_INIT(COptionNet)
	//}}AFX_DATA_INIT
}

COptionNet::~COptionNet()
{
}


static int CALLBACK CompareStr(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   COptionNet *pListCtrl = (COptionNet*) lParamSort;
   C_ALIAS    *pAlias1 = (C_ALIAS *) lParam1, *pAlias2 = (C_ALIAS *) lParam2;
   C_STRING   strItem1, strItem2;

   switch (pListCtrl->Prev_iHeader)
   {
      case 0: // Unità
         strItem1 = pAlias1->get_path();
         strItem2 = pAlias2->get_path();
         break;
      case 1: // Alias Host
         strItem1 = pAlias1->get_host();
         strItem2 = pAlias2->get_host();
         break;
      case 2: // Sistema operativo
         strItem1 = gsc_OSCode2OSName(pAlias1->get_op_sys());
         strItem2 = gsc_OSCode2OSName(pAlias2->get_op_sys());
         break;
      default:
         return 0;
   }

   if (pListCtrl->AscendingOrder) return strItem1.comp(strItem2, FALSE); // ordine crescente
   else return strItem2.comp(strItem1, FALSE); // ordine rescente
}

void COptionNet::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(COptionNet)
   DDX_Control(pDX, IDC_GENERIC_COMBO, m_GenericCombo);
   DDX_Control(pDX, IDC_ALIAS_LIST, m_AliasList);
   //}}AFX_DATA_MAP
   DDX_Control(pDX, IDC_ALIAS_COMBO, m_HostCombo);
}


BEGIN_MESSAGE_MAP(COptionNet, CPropertyPage)
	//{{AFX_MSG_MAP(COptionNet)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_ALIAS_LIST, OnColumnclickAliasList)
	ON_NOTIFY(NM_CLICK, IDC_ALIAS_LIST, OnClickAliasList)
	ON_CBN_KILLFOCUS(IDC_GENERIC_COMBO, OnKillfocusGenericCombo)
	ON_NOTIFY(LVN_KEYDOWN, IDC_ALIAS_LIST, OnKeydownAliasList)
	ON_BN_CLICKED(IDC_NEW_ALIAS, OnNewAlias)
	ON_CBN_SELCHANGE(IDC_GENERIC_COMBO, OnSelchangeGenericCombo)
	ON_BN_CLICKED(IDC_ERASE_ALIAS, OnEraseAlias)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDC_NEW_ALIAS, OnNewAlias)
	ON_COMMAND(IDC_ERASE_ALIAS, OnEraseAlias)  
	//}}AFX_MSG_MAP
   ON_CBN_KILLFOCUS(IDC_ALIAS_COMBO, &COptionNet::OnCbnKillfocusAliasCombo)
   ON_CBN_SELCHANGE(IDC_ALIAS_COMBO, &COptionNet::OnCbnSelchangeAliasCombo)
   ON_CBN_EDITCHANGE(IDC_ALIAS_COMBO, &COptionNet::OnCbnEditchangeAliasCombo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionNet message handlers


BOOL COptionNet::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

   InitAliasList();

   LoadFromGS();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionNet::OnColumnclickAliasList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;
	HD_ITEM      curItem;

	// retrieve embedded header control
	CHeaderCtrl* pHdrCtrl = NULL;
	pHdrCtrl = m_AliasList.GetHeaderCtrl();
	pHdrCtrl->SetImageList(&m_ImageHdrList);

   // alloco buffer
   curItem.pszText    = (TCHAR *) calloc(sizeof(TCHAR), 100);
   curItem.cchTextMax = 100 - 1;

   // se non era già stata selezionata questa casella
   if (Prev_iHeader != pNMListView->iSubItem)
   {
      if (Prev_iHeader != -1)
      {
         curItem.mask = HDI_TEXT;
	      pHdrCtrl->GetItem(Prev_iHeader, &curItem);
	      curItem.mask = HDI_FORMAT; // Solo Testo
      	curItem.fmt= HDF_LEFT | HDF_STRING;
	      pHdrCtrl->SetItem(Prev_iHeader, &curItem);
      }
      AscendingOrder = TRUE;
      Prev_iHeader = pNMListView->iSubItem;
   }
   else
      AscendingOrder = !AscendingOrder;

	// add bitmaps to header item
   curItem.mask   = HDI_TEXT;
	pHdrCtrl->GetItem(pNMListView->iSubItem, &curItem);
	curItem.mask   =  HDI_IMAGE | HDI_FORMAT; //  Testo e immagine
   curItem.iImage = (AscendingOrder) ? 0 : 1;
	curItem.fmt    = HDF_STRING | HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
	pHdrCtrl->SetItem(pNMListView->iSubItem, &curItem);

   m_AliasList.SortItems(CompareStr, (LPARAM) this);

   free(curItem.pszText);

   *pResult = 0;
}

void COptionNet::OnClickAliasList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
   CRect        rcItem;
   CString      ActualValue;

   if (pNMListView->iItem == -1) return; // nessuna selezione

   // se non era già stata selezionata questa casella
   if (Prev_iItem != pNMListView->iItem)
   {
      Prev_iItem = pNMListView->iItem;
      return;
   }

   if (pNMListView->iSubItem > 0) // la prima colonna contiene un'immagine
 	   m_AliasList.GetSubItemRect(pNMListView->iItem, pNMListView->iSubItem, LVIR_BOUNDS, rcItem);
   else
      m_AliasList.GetSubItemRect(pNMListView->iItem, pNMListView->iSubItem, LVIR_LABEL, rcItem);

   ActualValue   = m_AliasList.GetItemText(pNMListView->iItem, pNMListView->iSubItem);
   Curr_iSubItem = pNMListView->iSubItem;

   C_STR_LIST CStrList;
   C_STR      *pCStr;
   CComboBox  *pComboCtrl;

   switch (pNMListView->iSubItem)
   {
      case 0: // Unità
         rcItem.right = m_AliasList.GetColumnWidth(0);
         getAvailableDriveList(CStrList, ActualValue);
         pComboCtrl = &m_GenericCombo;
         break;
      case 1: // Alias
         getAvailableHostList(CStrList, ActualValue);
         pComboCtrl = &m_HostCombo;
         break;
      case 2: // Sistema operativo
         getOpSysList(CStrList);
         pComboCtrl = &m_GenericCombo;
         break;
   }

   while (pComboCtrl->DeleteString(0) != CB_ERR); // svuoto la combo

   pCStr = (C_STR *) CStrList.get_head();
   while (pCStr)
   {
      pComboCtrl->AddString(pCStr->get_name());
      pCStr = (C_STR *) CStrList.get_next();
   }

   pComboCtrl->SetWindowPos(NULL, rcItem.left, rcItem.top - 2,
		                      rcItem.Width(), rcItem.Height() - 2, SWP_SHOWWINDOW);
   pComboCtrl->SetFocus();

   if (pComboCtrl->GetCount() > 0)
   {
      pComboCtrl->SelectString(-1, ActualValue);
      pComboCtrl->ShowDropDown();
   }
   else
      pComboCtrl->SetWindowText(ActualValue);

   *pResult = 0;
}

void COptionNet::OnKeydownAliasList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

   if (pLVKeyDow->wVKey == 46) // CANC
   {
      POSITION iItem = m_AliasList.GetFirstSelectedItemPosition();
      
      if (iItem)
      {
         C_STRING Msg(_T("Cancellare il nodo di rete \""));
         CString  Alias;

         Alias = m_AliasList.GetItemText((int) iItem - 1, 1); // Alias
         Msg += Alias;
         Msg += _T("\" ?");
         if (gsui_confirm(Msg.get_name(), GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
         {
            if (AliasList.search_host(Alias)) AliasList.remove_at();
            m_AliasList.DeleteItem((int) iItem - 1); // cancella riga corrente
	         SetModified(); // enable Apply Now button
            isChanged = TRUE;
         }
      }
   }

	*pResult = 0;
}

void COptionNet::OnNewAlias() 
{
	LV_ITEM    lvitem;
   C_STRING   Alias;
   C_STR_LIST AvailableDriveList;
   int        i = 1;
   C_ALIAS    *pAlias;

   // Cerco una unità non ancora usata
   getAvailableDriveList(AvailableDriveList);
   if (AvailableDriveList.get_count() == 0)
   {
      gsui_alert(_T("Non ci sono unità disponibili."), m_hWnd);
      return;
   }

   // Cerco un alias non ancora usato
   do
   {
      Alias = _T("ALIAS");
      Alias += i++;
   }
   while (AliasList.search_host(Alias.get_name()) != NULL);

   if ((pAlias = new C_ALIAS) == NULL) return;
   pAlias->set_path(AvailableDriveList.get_head()->get_name());
   pAlias->set_host(Alias.get_name());
   pAlias->set_op_sys(GS_OS_WINDOWS);
   if (AliasList.add_to_mem(pAlias) == GS_BAD) return;

   lvitem.iItem  = m_AliasList.GetItemCount();
   lvitem.iImage = 0;
   lvitem.lParam = (LPARAM) pAlias;
	// Unità
   lvitem.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
   lvitem.iSubItem = 0;
   lvitem.pszText  = AvailableDriveList.get_head()->get_name();
   m_AliasList.InsertItem(&lvitem);
	// Alias
 	lvitem.mask = LVIF_TEXT;
   lvitem.iSubItem++;
   lvitem.pszText  = Alias.get_name();
   m_AliasList.SetItem(&lvitem);
	// Sistema operativo
   lvitem.iSubItem++;
   lvitem.pszText = _T("WINDOWS");
   m_AliasList.SetItem(&lvitem);

   // Lo seleziono
   m_AliasList.SetFocus();
   m_AliasList.SetItemState(lvitem.iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
   m_AliasList.EnsureVisible(lvitem.iItem, FALSE);
   Prev_iItem = lvitem.iItem;

	SetModified(); // enable Apply Now button
   isChanged = TRUE;
}

void COptionNet::OnEraseAlias() 
{
   POSITION iItem = m_AliasList.GetFirstSelectedItemPosition();
   
   if (iItem)
   {
      C_STRING Msg(_T("Cancellare il nodo di rete \""));
      CString  Alias;

      Alias = m_AliasList.GetItemText((int) iItem - 1, 1); // Alias Host
      Msg += Alias;
      Msg += _T("\" ?");
      if (gsui_confirm(Msg.get_name(), GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
      {
         if (AliasList.search_host(Alias)) AliasList.remove_at();
         m_AliasList.DeleteItem((int) iItem - 1); // cancella riga corrente
	      SetModified(); // enable Apply Now button
         isChanged = TRUE;
      }
   }	
}


void COptionNet::OnSelchangeGenericCombo() 
{
   if (m_GenericCombo.IsWindowVisible())
   {
	   SetModified(); // enable Apply Now button
      isChanged = TRUE;
   }
}

void COptionNet::OnKillfocusGenericCombo() 
{
   m_GenericCombo.ShowWindow(SW_HIDE);

   if (!isChanged) return;

   C_ALIAS *pPrevAlias, NewAlias;
   CString dummy;

	// Aggiorno alias in memoria
   if ((pPrevAlias = (C_ALIAS *) m_AliasList.GetItemData(Prev_iItem)) == NULL) return;  

   m_GenericCombo.GetWindowText(dummy);

   switch (Curr_iSubItem)
   {
      case 0: // Unità
         NewAlias.set_path(dummy);
         NewAlias.set_op_sys(pPrevAlias->get_op_sys());
         break;
      case 2: // Sistema operativo
         NewAlias.set_op_sys(gsc_OSName2OSCode(dummy));
         NewAlias.set_path(pPrevAlias->get_path());
         break;
   }
   NewAlias.set_host(pPrevAlias->get_host());

   if (AliasList.upd_to_mem(pPrevAlias, &NewAlias) == GS_BAD) return;

   // Aggiorno alias in CListCtrl
   C_STRING Value(dummy);
	m_AliasList.SetItemText(Prev_iItem, Curr_iSubItem, Value.get_name());
}

void COptionNet::OnCbnEditchangeAliasCombo()
{
   if (m_HostCombo.IsWindowVisible())
   {
	   SetModified(); // enable Apply Now button
      isChanged = TRUE;
   }
}

void COptionNet::OnCbnSelchangeAliasCombo()
{
   if (m_HostCombo.IsWindowVisible())
   {
	   SetModified(); // enable Apply Now button
      isChanged = TRUE;
   }
}

void COptionNet::OnCbnKillfocusAliasCombo()
{
   m_HostCombo.ShowWindow(SW_HIDE);

   if (!isChanged) return;

   C_ALIAS *pPrevAlias, NewAlias, *pHost;
   CString dummy;

	// Aggiorno alias in memoria
   if ((pPrevAlias = (C_ALIAS *) m_AliasList.GetItemData(Prev_iItem)) == NULL) return;  

   m_HostCombo.GetWindowText(dummy);

   NewAlias.set_host(dummy);
   NewAlias.set_path(pPrevAlias->get_path());
   // Cerco il valore del sistema operativo dalla lista degli host lato server 
   if ((pHost = SrvSideHostList.search_host(dummy)))
      NewAlias.set_op_sys(pHost->get_op_sys());
   else
      NewAlias.set_op_sys(pPrevAlias->get_op_sys());

   if (AliasList.upd_to_mem(pPrevAlias, &NewAlias) == GS_BAD) return;

   // Aggiorno alias in CListCtrl
   C_STRING Value(dummy);
	m_AliasList.SetItemText(Prev_iItem, Curr_iSubItem, Value.get_name());

   // Aggiorno sistema operativo in CListCtrl
   if (pHost)
   {  
      Value = gsc_OSCode2OSName(NewAlias.get_op_sys());
   	m_AliasList.SetItemText(Prev_iItem, 2, Value.get_name());
   }
}

void COptionNet::getAvailableDriveList(C_STR_LIST &AvailableDriveList, 
                                       const TCHAR *ActualDrive)
{  // ristorna la lista dei drive scartando quelli già usati e quello corrente
   C_STR *pDrive;

   if (DriveList.get_count() == 0) // Inizializzo la lista dei drive collegati
      gsui_getDriveList(DriveList);

   AvailableDriveList.remove_all();
   pDrive = (C_STR *) DriveList.get_head();
   while (pDrive)
   {
      // se non è già usato o se si tratta di quello attuale
      if (AliasList.search_path(pDrive->get_name(), FALSE) == NULL ||
          (ActualDrive && pDrive->comp(ActualDrive, FALSE) == 0))
         if ((pDrive = new C_STR(pDrive->get_name())) != NULL)
            AvailableDriveList.add_tail(pDrive);

      pDrive = (C_STR *) DriveList.get_next();
   }
}

void COptionNet::getOpSysList(C_STR_LIST &OpSysList)
{
   C_STR *pOpSys;

   OpSysList.remove_all();
   if ((pOpSys = new C_STR(_T("DOS"))) != NULL)
      OpSysList.add_tail(pOpSys);
   if ((pOpSys = new C_STR(_T("WINDOWS"))) != NULL)
      OpSysList.add_tail(pOpSys);
   if ((pOpSys = new C_STR(_T("UNIX"))) != NULL)
      OpSysList.add_tail(pOpSys);
}

void COptionNet::getAvailableHostList(C_STR_LIST &AvailableHostNameList, 
                                      const TCHAR *ActualHost)
{  // ritorna la lista degli host lato server scartando quelli già usati e quello corrente
   C_ALIAS *pHost;
   C_STR   *pHostName;

   if (SrvSideHostList.get_count() == 0) // Inizializzo la lista dei drive collegati
      SrvSideHostList.loadHostsFromSrv();

   AvailableHostNameList.remove_all();
   pHost = (C_ALIAS *) SrvSideHostList.get_head();
   while (pHost)
   {
      // se non è già usato o se si tratta di quello attuale
      if (AliasList.search_host(pHost->get_host()) == NULL ||
          (ActualHost && gsc_strcmp(pHost->get_host(), ActualHost, FALSE) == 0))
         if ((pHostName = new C_STR(pHost->get_host())) != NULL)
            AvailableHostNameList.add_tail(pHostName);

      pHost = (C_ALIAS *) SrvSideHostList.get_next();
   }
}

void COptionNet::InitAliasList(void)
{
   CBitmap CBMP;
	CRect   rect;

   // creazione lista vuota
   m_ImageHdrList.Create(HDR_BITMAP_WIDTH, HDR_BITMAP_HEIGHT,
                         ILC_MASK | ILC_COLORDDB, 
                         2, // n. bitmap
                         0);
   // aggiungo gli elementi di base della lista
   // bitmap simbolo "ordine crescente"
   if (CBMP.LoadBitmap(IDB_ASCENDING_ORDER) == 0) return;
   m_ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente
   
   // bitmap simbolo "ordine decrescente"
   if (CBMP.LoadBitmap(IDB_DESCENDING_ORDER) == 0) return;
   m_ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente

   // creazione lista vuota
   m_ImageList.Create(BITMAP_WIDTH, BITMAP_HEIGHT,
                      ILC_MASK | ILC_COLORDDB, 
                      1, // n. bitmap
                      0);
   // aggiungo gli elementi di base della lista
   // bitmap simbolo dei nodi di rete
   CBMP.LoadBitmap(MAKEINTRESOURCE(IDB_NET_NODE));
   m_ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente

   m_AliasList.SetImageList(&m_ImageList, LVSIL_SMALL);

   // insert 3 columns (REPORT mode) and modify the new header items
	m_AliasList.GetWindowRect(&rect);

	m_AliasList.InsertColumn(0, _T("Unità"), LVCFMT_LEFT,
		                      rect.Width() * 4/20, 0);
	m_AliasList.InsertColumn(1, _T("Alias"), LVCFMT_LEFT,
		                      rect.Width() * 9/20, 1);
	m_AliasList.InsertColumn(2, _T("Sistema operativo"), LVCFMT_LEFT,
		                      rect.Width() * 7/20 - 8, 2);

	m_AliasList.SetExtendedStyle(m_AliasList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

   m_GenericCombo.SetParent(&m_AliasList);
   m_HostCombo.SetParent(&m_AliasList);
}

void COptionNet::LoadFromGS()
{
	int     iRow = 0;
	LV_ITEM lvitem;
   C_ALIAS *pAlias;
   C_STRING dummy;

   if (AliasList.load() == GS_BAD) return;

   lvitem.iImage = 0;
   pAlias = (C_ALIAS *) AliasList.get_head();
   while (pAlias)
   {
		lvitem.iItem  = iRow++;
      lvitem.lParam = (LPARAM) pAlias;

      // Unità
   	lvitem.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
      lvitem.iSubItem = 0;
      lvitem.pszText  = pAlias->get_path();
   	m_AliasList.InsertItem(&lvitem);

   	lvitem.mask = LVIF_TEXT;
      // Alias Host
      lvitem.iSubItem++;
      lvitem.pszText = pAlias->get_host();
   	m_AliasList.SetItem(&lvitem);
		// Sistema operativo
      lvitem.iSubItem++;
      dummy = gsc_OSCode2OSName(pAlias->get_op_sys());
      lvitem.pszText = dummy.get_name();
   	m_AliasList.SetItem(&lvitem);

      pAlias = (C_ALIAS *) AliasList.get_next();
   }
}

void COptionNet::SaveToGS()
{
   if (!isChanged) return;

   // Se si esce con OK durante la digitazione di un alias
   if (m_HostCombo.IsWindowVisible()) OnCbnKillfocusAliasCombo();

   if (get_GS_CURRENT_WRK_SESSION())
      gsui_alert(_T("Le impostazioni degli alias non sono modificabili durante una sessione di lavoro."), m_hWnd);

   // Scrittura modifiche su file
   if (AliasList.save() == GS_BAD) return;
   // Ricarico la lista degli host dal server
   SrvSideHostList.loadHostsFromSrv();

   // inizializza le strutture usate da GeoSim che dipendono dall'utente corrente.
   // questa funzione rilascia tutti i progetti e li ricarica
   gsc_initByUser();
   isChanged = FALSE;
}

void COptionNet::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CMenu  Menu;
   CMenu  *SubMenu;

   Menu.LoadMenu(IDR_MENU_OPTION_NET);
   SubMenu = Menu.GetSubMenu(0);
   SubMenu->TrackPopupMenu(0, point.x, point.y, this, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CGSOptions

IMPLEMENT_DYNAMIC(CGSOptions, CPropertySheet)

CGSOptions::CGSOptions(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddControlPages();
}

CGSOptions::CGSOptions(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddControlPages();
}

CGSOptions::~CGSOptions()
{
}


BEGIN_MESSAGE_MAP(CGSOptions, CPropertySheet)
	//{{AFX_MSG_MAP(CGSOptions)
	ON_COMMAND(ID_APPLY_NOW, OnApplyNow)
	ON_COMMAND(IDOK, OnOk)
	ON_COMMAND(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGSOptions message handlers

void CGSOptions::AddControlPages()
{
	AddPage(&m_OptionNet);
	AddPage(&m_OptionEnv);
}

void CGSOptions::InitializeImageList()
{
   // lista di base delle immagini
   CBitmap CBMP;
  
   // creazione lista vuota
   m_ImageList.Create(BITMAP_WIDTH, BITMAP_HEIGHT,
                      ILC_COLORDDB | ILC_MASK, 
                      2, // n. bitmap
                      0);

   // aggiungo gli elementi di base della lista

   // bitmap simbolo dei nodi di rete
   if (CBMP.LoadBitmap(IDB_NET_NODE) == 0) return;
   m_ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente

   // bitmap simbolo dei variabili d'ambiente
   if (CBMP.LoadBitmap(IDB_ENV_VARIABLE) == 0) return;
   m_ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente

   CTabCtrl *pTab = GetTabControl();
   ASSERT(pTab);
   pTab->SetImageList(&m_ImageList);

	TC_ITEM TabCtrlItem;

   TabCtrlItem.mask = TCIF_TEXT | TCIF_IMAGE;
	
   TabCtrlItem.pszText = _T("Nodi di rete");
   TabCtrlItem.iImage = 0;
	pTab->SetItem( 0, &TabCtrlItem);

   TabCtrlItem.pszText = _T("Variabili d'ambiente");
   TabCtrlItem.iImage = 1;
	pTab->SetItem( 1, &TabCtrlItem);

   return;
}

BOOL CGSOptions::OnInitDialog() 
{
   BOOL bResult = CPropertySheet::OnInitDialog();
	
   InitializeImageList();

	return bResult;
}

void CGSOptions::OnApplyNow()
{
   SaveToGS();
	Default();
}

void CGSOptions::OnOk()
{
   SaveToGS();
	Default();
}

void CGSOptions::OnHelp()
{
   CTabCtrl *pTab = GetTabControl();

   switch (pTab->GetCurSel())
   {
      case 0: // nodi di rete
         gsc_help(IDH_Gestionedeinodidirete);		
         break;
      case 1: // variabili di ambiente
         gsc_help(IDH_PERSONALIZZAZIONEDELSISTEMA);		
         break;
   }
}

void CGSOptions::SaveToGS()
{
   m_OptionNet.SaveToGS();
   m_OptionEnv.SaveToGS();
}

/////////////////////////////////////////////////////////////////////////////
// COptionEnv property page

IMPLEMENT_DYNCREATE(COptionEnv, CPropertyPage)

COptionEnv::COptionEnv() : CPropertyPage(COptionEnv::IDD)
{
   isChanged = FALSE;
	//{{AFX_DATA_INIT(COptionEnv)
	//}}AFX_DATA_INIT
}

COptionEnv::~COptionEnv()
{
}

void COptionEnv::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(COptionEnv)
   DDX_Control(pDX, IDC_ADD_ASE_LINK, m_AddASELink);
   DDX_Control(pDX, IDC_AUTO_SYNCRO, m_AutoSyncronize);
   DDX_Control(pDX, IDC_DYNAMIC_EXTRACTION, m_DynamicExtraction);
   DDX_Control(pDX, IDC_UPD_GRAPH_ON_EXTR_SIM, m_UpdGraphOnExtractSim);
   DDX_Control(pDX, IDC_UPD_GRAPH_ON_EXTR, m_UpdGraphOnExtract);
   DDX_Control(pDX, IDC_WAIT_TIME, m_WaitTime);
   DDX_Control(pDX, IDC_NUM_TEST, m_NumTest);
   DDX_Control(pDX, IDC_LOG_FILE, m_LogFile);
   DDX_Control(pDX, IDC_INS_Y_SCALE, m_InsYScale);
   DDX_Control(pDX, IDC_INS_X_SCALE, m_InsXScale);
   DDX_Control(pDX, IDC_INS_ROT, m_InsRot);
   DDX_Control(pDX, IDC_INS_POS, m_InsPos);
   DDX_Control(pDX, IDC_INS_H_TEXT, m_InsHText);
   DDX_Control(pDX, IDC_ALIGNHIGHLIGHTEDFASONSAVE, m_AlignHighlightedFasOnSave);
   DDX_Control(pDX, IDC_ADD_ENTITY_TO_SAVESET, m_AddEntityToSaveSet);
   DDX_Control(pDX, IDC_AUTO_ZOOM_MIN_X_DIM, m_AutoZoomMinXDim);
   DDX_Control(pDX, IDC_AUTO_ZOOM_MIN_Y_DIM, m_AutoZoomMinYDim);
   DDX_Control(pDX, IDC_SEL_PREVIOUS_EXTRACTED_CLASSES, m_SelPreviousExtractedClasses);
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionEnv, CPropertyPage)
	//{{AFX_MSG_MAP(COptionEnv)
	ON_BN_CLICKED(IDC_ADD_ENTITY_TO_SAVESET, OnAddEntityToSaveset)
	ON_BN_CLICKED(IDC_ALIGNHIGHLIGHTEDFASONSAVE, OnAlignhighlightedfasonsave)
	ON_BN_CLICKED(IDC_INS_H_TEXT, OnInsHText)
	ON_BN_CLICKED(IDC_INS_X_SCALE, OnInsXScale)
	ON_BN_CLICKED(IDC_INS_Y_SCALE, OnInsYScale)
	ON_BN_CLICKED(IDC_INS_POS, OnInsPos)
	ON_EN_CHANGE(IDC_NUM_TEST, OnChangeNumTest)
	ON_EN_KILLFOCUS(IDC_NUM_TEST, OnKillfocusNumTest)
	ON_EN_CHANGE(IDC_WAIT_TIME, OnChangeWaitTime)
	ON_BN_CLICKED(IDC_LOG_FILE, OnLogFile)
	ON_BN_CLICKED(IDC_INS_ROT, OnInsRot)
	ON_BN_CLICKED(IDC_RADIO_DATA_1, OnRadioData1)
	ON_BN_CLICKED(IDC_RADIO_DATA_2, OnRadioData2)
	ON_BN_CLICKED(IDC_RADIO_LOGIC_1, OnRadioLogic1)
	ON_BN_CLICKED(IDC_RADIO_LOGIC_2, OnRadioLogic2)
	ON_BN_CLICKED(IDC_RADIO_LOGIC_3, OnRadioLogic3)
	ON_BN_CLICKED(IDC_RADIO_LOGIC_4, OnRadioLogic4)
	ON_BN_CLICKED(IDC_UPD_GRAPH_ON_EXTR, OnUpdGraphOnExtr)
	ON_BN_CLICKED(IDC_UPD_GRAPH_ON_EXTR_SIM, OnUpdGraphOnExtrSim)
	ON_EN_KILLFOCUS(IDC_WAIT_TIME, OnKillfocusWaitTime)
	ON_BN_CLICKED(IDC_DYNAMIC_EXTRACTION, OnDynamicExtraction)
	ON_BN_CLICKED(IDC_AUTO_SYNCRO, OnAutoSyncro)
	ON_BN_CLICKED(IDC_ADD_ASE_LINK, OnAddAseLink)
	ON_BN_CLICKED(IDC_DIMASSOC_0, OnDimAssoc0)
	ON_BN_CLICKED(IDC_DIMASSOC_1, OnDimAssoc1)
	ON_BN_CLICKED(IDC_DIMASSOC_2, OnDimAssoc2)
	ON_EN_CHANGE(IDC_AUTO_ZOOM_MIN_X_DIM, OnChangeAutoZoomMinXDim)
	ON_EN_KILLFOCUS(IDC_AUTO_ZOOM_MIN_X_DIM, OnKillfocusAutoZoomMinXDim)
	ON_EN_CHANGE(IDC_AUTO_ZOOM_MIN_Y_DIM, OnChangeAutoZoomMinYDim)
	ON_EN_KILLFOCUS(IDC_AUTO_ZOOM_MIN_Y_DIM, OnKillfocusAutoZoomMinYDim)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionEnv message handlers

void COptionEnv::OnAddEntityToSaveset() 
{
	SetModified(); // enable Apply Now button
   isChanged = TRUE;
}

void COptionEnv::OnAlignhighlightedfasonsave() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnInsHText() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnInsXScale() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnInsYScale() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnInsPos() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnChangeNumTest() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnChangeAutoZoomMinXDim() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}
void COptionEnv::OnKillfocusAutoZoomMinXDim() 
{
   CString dummy;
   C_STRING Conv;

   m_AutoZoomMinXDim.GetWindowText(dummy);
   Conv = dummy;
   if (Conv.isDigit() == GS_BAD || Conv.tof() < 0)
   {
      gsui_alert(_T("Il valore non deve essere negativo."), m_hWnd);	
      m_AutoZoomMinXDim.SetFocus();
   }
}

void COptionEnv::OnChangeAutoZoomMinYDim() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}
void COptionEnv::OnKillfocusAutoZoomMinYDim() 
{
   CString dummy;
   C_STRING Conv;

   m_AutoZoomMinYDim.GetWindowText(dummy);
   Conv = dummy;
   if (Conv.isDigit() == GS_BAD || Conv.tof() < 0)
   {
      gsui_alert(_T("Il valore non deve essere negativo."), m_hWnd);	
      m_AutoZoomMinYDim.SetFocus();
   }
}

void COptionEnv::OnChangeWaitTime() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnLogFile() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnInsRot() 
{
	SetModified(); // enable Apply Now button		
   isChanged = TRUE;
}

void COptionEnv::OnRadioData1() 
{
   m_ShortDataFmt = 1;
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnRadioData2() 
{
   m_ShortDataFmt = 0;	
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnRadioLogic1() 
{
	m_LogicFmt = 0;  // si/no
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnRadioLogic2() 
{
	m_LogicFmt = 1;  // Vero/Falso
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnRadioLogic3() 
{
	m_LogicFmt = 2;  // on/off
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnRadioLogic4() 
{
	m_LogicFmt = 3;  // 1/0
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnDimAssoc0() 
{
	m_DimAssoc = 0;  // quotatuta esplosa
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnDimAssoc1() 
{
	m_DimAssoc = 1;  // quotatura non associativa
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnDimAssoc2() 
{
	m_DimAssoc = 2;  // quotatura associativa
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::LoadFromGS()
{
   C_INIT   *pGS_GLOBALVAR = get_GS_GLOBALVAR();
   C_STRING dummy;
	CButton  *pBtn = NULL;

	// Formato booleano
   static UINT LogicButtons [] = { IDC_RADIO_LOGIC_1,
                                   IDC_RADIO_LOGIC_2,
                                   IDC_RADIO_LOGIC_3,
                                   IDC_RADIO_LOGIC_4 };
   m_LogicFmt = pGS_GLOBALVAR->get_BoolFmt();
   for (int i = 0; i < ELEMENTS(LogicButtons); i++)
	{
      pBtn = (CButton *) GetDlgItem(LogicButtons[i]);
		if (!pBtn) continue;
      pBtn->SetCheck((i == m_LogicFmt) ? BST_CHECKED: BST_UNCHECKED);
	}

   // Modalità di quotatura
   static UINT DimAssocButtons [] = { IDC_DIMASSOC_0,
                                      IDC_DIMASSOC_1,
                                      IDC_DIMASSOC_2 };
   m_DimAssoc = pGS_GLOBALVAR->get_DimAssoc();
   for (int i = 0; i < ELEMENTS(DimAssocButtons); i++)
	{
      pBtn = (CButton *) GetDlgItem(DimAssocButtons[i]);
		if (!pBtn) continue;
      pBtn->SetCheck((i == m_DimAssoc) ? BST_CHECKED: BST_UNCHECKED);
	}

   // Formato data
   m_ShortDataFmt = pGS_GLOBALVAR->get_ShortDate();
   if (m_ShortDataFmt == GS_GOOD)
   {
		if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_DATA_1))) pBtn->SetCheck(BST_CHECKED);
		if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_DATA_2))) pBtn->SetCheck(BST_UNCHECKED);
   }
   else
   {
		if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_DATA_1))) pBtn->SetCheck(BST_UNCHECKED);
		if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_DATA_2))) pBtn->SetCheck(BST_CHECKED);
   }

   // Tempo di attesa
   dummy = pGS_GLOBALVAR->get_WaitTime();
   m_WaitTime.SetWindowText(dummy.get_name());

   // N. tentativi
   dummy = pGS_GLOBALVAR->get_NumTest();
   m_NumTest.SetWindowText(dummy.get_name());

   // File LOG
   m_LogFile.SetCheck((pGS_GLOBALVAR->get_LogFile() == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);

   // Scala X
   m_InsXScale.SetCheck((pGS_GLOBALVAR->get_InsXScale() == MANUAL) ? BST_UNCHECKED : BST_CHECKED);

   // Scala Y
   m_InsYScale.SetCheck((pGS_GLOBALVAR->get_InsYScale() == MANUAL) ? BST_UNCHECKED : BST_CHECKED);

   // Rotazione
   m_InsRot.SetCheck((pGS_GLOBALVAR->get_InsRotaz() == MANUAL) ? BST_UNCHECKED : BST_CHECKED);

   // Posizione attributi
   m_InsPos.SetCheck((pGS_GLOBALVAR->get_InsPos() == MANUAL) ? BST_UNCHECKED : BST_CHECKED);

   // Altezza testo
   m_InsHText.SetCheck((pGS_GLOBALVAR->get_InsHText() == MANUAL) ? BST_UNCHECKED : BST_CHECKED);
   
   // Allineamento oggetti evidenziati
   m_AlignHighlightedFasOnSave.SetCheck((pGS_GLOBALVAR->get_AlignHighlightedFASOnSave() == GS_GOOD) ? BST_CHECKED: BST_UNCHECKED);
   
   // Aggiungi oggetti in salvataggio
   m_AddEntityToSaveSet.SetCheck((pGS_GLOBALVAR->get_AddEntityToSaveSet() == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);

   // Aggiorna grafica in estrazione
   m_UpdGraphOnExtract.SetCheck((pGS_GLOBALVAR->get_UpdGraphOnExtract() == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);

   // Aggiorna grafica simulazioni in estrazione
   m_UpdGraphOnExtractSim.SetCheck((pGS_GLOBALVAR->get_UpdGraphOnExtractSim() == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);

   if (m_UpdGraphOnExtract.GetCheck() == BST_CHECKED) m_UpdGraphOnExtractSim.EnableWindow(FALSE);
   else m_UpdGraphOnExtractSim.EnableWindow(TRUE);

   // Estrazione dinamica
   m_DynamicExtraction.SetCheck((pGS_GLOBALVAR->get_DynamicExtraction() == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);

   // Sincronizzazione automatica
   m_AutoSyncronize.SetCheck((pGS_GLOBALVAR->get_AutoSyncro() == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);

   // Aggiunta collegamenti ASE
   m_AddASELink.SetCheck((pGS_GLOBALVAR->get_AddLPTOnExtract() == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);

   // Dimensione minima finestra Autozoom
   dummy = pGS_GLOBALVAR->get_AutoZoomMinXDim();
   m_AutoZoomMinXDim.SetWindowText(dummy.get_name());
   dummy = pGS_GLOBALVAR->get_AutoZoomMinYDim();
   m_AutoZoomMinYDim.SetWindowText(dummy.get_name());

   // Selezione classe estratte nella sessione di lavoro precedente
   m_SelPreviousExtractedClasses.SetCheck((pGS_GLOBALVAR->get_SelectPreviousExtractedClasses() == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);
}

void COptionEnv::SaveToGS()
{
   C_INIT  *pGS_GLOBALVAR = get_GS_GLOBALVAR();
   CString dummy;
	CButton *pBtn = NULL;

   if (!isChanged) return;

	// Formato booleano
   static UINT LogicButtons [] = { IDC_RADIO_LOGIC_1,
                                   IDC_RADIO_LOGIC_2,
                                   IDC_RADIO_LOGIC_3,
                                   IDC_RADIO_LOGIC_4 };
   for (int i = 0; i < ELEMENTS(LogicButtons); i++)
	{
      pBtn = (CButton *) GetDlgItem(LogicButtons[i]);
		if (!pBtn) continue;

      if (pBtn->GetCheck() == 1) pGS_GLOBALVAR->set_BoolFmt(i);
	}

   // Modalità di quotatura
   static UINT DimAssocButtons [] = { IDC_DIMASSOC_0,
                                      IDC_DIMASSOC_1,
                                      IDC_DIMASSOC_2 };
   for (int i = 0; i < ELEMENTS(DimAssocButtons); i++)
	{
      pBtn = (CButton *) GetDlgItem(DimAssocButtons[i]);
		if (!pBtn) continue;

      if (pBtn->GetCheck() == 1) pGS_GLOBALVAR->set_DimAssoc(i);
	}

   // Formato data
	if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_DATA_1)) && pBtn->GetCheck() == 1)
      pGS_GLOBALVAR->set_ShortDate(GS_GOOD);
   else
      pGS_GLOBALVAR->set_ShortDate(GS_BAD);

   // Tempo di attesa
   m_WaitTime.GetWindowText(dummy);
   pGS_GLOBALVAR->set_WaitTime(_wtoi(dummy));

   // N. tentativi
   m_NumTest.GetWindowText(dummy);
   pGS_GLOBALVAR->set_NumTest(_wtoi(dummy));

   // File LOG
   pGS_GLOBALVAR->set_LogFile((m_LogFile.GetCheck() == 1) ? GS_GOOD : GS_BAD);

   // Scala X
   pGS_GLOBALVAR->set_InsXScale((m_InsXScale.GetCheck() == 1) ? AUTO : MANUAL);

   // Scala Y
   pGS_GLOBALVAR->set_InsYScale((m_InsYScale.GetCheck() == 1) ? AUTO : MANUAL);

   // Rotazione
   pGS_GLOBALVAR->set_InsRotaz((m_InsRot.GetCheck() == 1) ? AUTO : MANUAL);

   // Posizione attributi
   pGS_GLOBALVAR->set_InsPos((m_InsPos.GetCheck() == 1) ? AUTO : MANUAL);

   // Altezza testo
   pGS_GLOBALVAR->set_InsHText((m_InsHText.GetCheck() == 1) ? AUTO : MANUAL);
   
   // Allineamento oggetti evidenziati
   pGS_GLOBALVAR->set_AlignHighlightedFASOnSave((m_AlignHighlightedFasOnSave.GetCheck() == 1) ? GS_GOOD : GS_BAD);
   
   // Aggiungi oggetti in salvataggio
   pGS_GLOBALVAR->set_AddEntityToSaveSet((m_AddEntityToSaveSet.GetCheck() == 1) ? GS_GOOD : GS_BAD);

   // Aggiorna grafica in estrazione
   pGS_GLOBALVAR->set_UpdGraphOnExtract((m_UpdGraphOnExtract.GetCheck() == 1) ? GS_GOOD : GS_BAD);

   // Aggiorna grafica simulazioni in estrazione
   pGS_GLOBALVAR->set_UpdGraphOnExtractSim((m_UpdGraphOnExtractSim.GetCheck() == 1) ? GS_GOOD : GS_BAD);

   // Estrazione dinamica
   pGS_GLOBALVAR->set_DynamicExtraction((m_DynamicExtraction.GetCheck() == 1) ? GS_GOOD : GS_BAD);

   // Sincronizzazione automatica
   pGS_GLOBALVAR->set_AutoSyncro((m_AutoSyncronize.GetCheck() == 1) ? GS_GOOD : GS_BAD);

   // Aggiunta collegamenti ASE
   pGS_GLOBALVAR->set_AddLPTOnExtract((m_AddASELink.GetCheck() == 1) ? GS_GOOD : GS_BAD);

   // Dimensione minima finestra Autozoom
   m_AutoZoomMinXDim.GetWindowText(dummy);
   pGS_GLOBALVAR->set_AutoZoomMinXDim(_wtof(dummy));
   m_AutoZoomMinYDim.GetWindowText(dummy);
   pGS_GLOBALVAR->set_AutoZoomMinYDim(_wtof(dummy));

   // Selezione classe estratte nella sessione di lavoro precedente
   pGS_GLOBALVAR->set_SelectPreviousExtractedClasses((m_SelPreviousExtractedClasses.GetCheck() == 1) ? GS_GOOD : GS_BAD);

   pGS_GLOBALVAR->Save();

   isChanged = FALSE;
}

BOOL COptionEnv::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
   LoadFromGS();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionEnv::OnUpdGraphOnExtr() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;

   if (m_UpdGraphOnExtract.GetCheck() == 1) m_UpdGraphOnExtractSim.EnableWindow(FALSE);
   else m_UpdGraphOnExtractSim.EnableWindow(TRUE);
}

void COptionEnv::OnUpdGraphOnExtrSim() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnDynamicExtraction() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnAutoSyncro() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnAddAseLink() 
{
	SetModified(); // enable Apply Now button	
   isChanged = TRUE;
}

void COptionEnv::OnKillfocusNumTest() 
{
   CString dummy;
   C_STRING Conv;

   m_NumTest.GetWindowText(dummy);
   Conv = dummy;
   if (Conv.isDigit() == GS_BAD || Conv.toi() < 1 || Conv.toi() > 99)
   {
      gsui_alert(_T("Il valore deve essere compreso tra 1 e 99."), m_hWnd);
      m_NumTest.SetFocus();
   }
}

void COptionEnv::OnKillfocusWaitTime() 
{
   CString dummy;
   C_STRING Conv;

   m_WaitTime.GetWindowText(dummy);
   Conv = dummy;
   if (Conv.isDigit() == GS_BAD || Conv.toi() < 1 || Conv.toi() > 99)
   {
      gsui_alert(_T("Il valore deve essere compreso tra 0 e 99."), m_hWnd);	
      m_WaitTime.SetFocus();
   }
}


/*************************************************************************/
/*.doc gsui_options                                                       */
/*+
  Comando per impostare le opzioni di GEOsim (variabili di ambiente di
  GEOsim, nodi di rete)

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_options(void)
{
   CGSOptions Options(_T("GEOsim - Opzioni"));

   acedRetVoid();
   Options.DoModal();

   return GS_GOOD;  
}
