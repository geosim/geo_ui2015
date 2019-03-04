// gs_ui_sql.cpp : implementation file
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include "gs_ui_sql.h"

#include "gs_init.h"
#include "gs_resbf.h"
#include "gs_utily.h"


// CDBConnDlg dialog

IMPLEMENT_DYNAMIC(CDBConnDlg, CDialog)

CDBConnDlg::CDBConnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDBConnDlg::IDD, pParent)
{
   pConn    = NULL;
   pToolTip = NULL;
   Flags    = CONN_DLG_CREATE;
   Prev_iItem = -1;
}

CDBConnDlg::~CDBConnDlg()
{
   if (pToolTip) delete pToolTip;
}

void CDBConnDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_UDL_LIST_COMBO, m_UdlComboCtrl);
   DDX_Control(pDX, IDC_TABLE_EDIT, m_TableEditCtrl);
   DDX_Control(pDX, IDC_TABLE_COMBO, m_TableComboCtrl);
   DDX_Control(pDX, IDC_PROP_BUTTON, m_PropButtonCtrl);
   DDX_Control(pDX, IDC_PROP_EDIT, m_PropEditCtrl);
   DDX_Control(pDX, IDC_UDL_PROPERTIES_LIST, m_PropListCtrl);
   DDX_Control(pDX, IDC_TABLE_NAME_LBL, m_TableNameLbl);
   DDX_Control(pDX, IDOK, m_OkButton);
   DDX_Control(pDX, IDCANCEL, m_CancelButton);
   DDX_Control(pDX, IDHELP, m_HelpButton);
   DDX_Control(pDX, IDC_LBL_CONNECTION_STATE, m_ConnectionState);
   DDX_Control(pDX, IDC_SCHEMA_EDIT, m_SchemaEditCtrl);
   DDX_Control(pDX, IDC_SCHEMA_NAME_LBL, m_SchemaNameLbl);
}


BEGIN_MESSAGE_MAP(CDBConnDlg, CDialog)
   ON_CBN_SELCHANGE(IDC_UDL_LIST_COMBO, &CDBConnDlg::OnCbnSelchangeUdlListCombo)
   ON_NOTIFY(NM_CLICK, IDC_UDL_PROPERTIES_LIST, &CDBConnDlg::OnNMClickUdlPropertiesList)
   ON_EN_KILLFOCUS(IDC_PROP_EDIT, &CDBConnDlg::OnEnKillfocusPropEdit)
   ON_BN_CLICKED(IDC_PROP_BUTTON, &CDBConnDlg::OnBnClickedPropButton)
   ON_EN_CHANGE(IDC_TABLE_EDIT, &CDBConnDlg::OnEnChangeTableEdit)
   ON_EN_CHANGE(IDC_SCHEMA_EDIT, &CDBConnDlg::OnEnChangeSchemaEdit)
   ON_BN_CLICKED(IDOK, &CDBConnDlg::OnBnClickedOk)
   ON_CBN_SELCHANGE(IDC_TABLE_COMBO, &CDBConnDlg::OnCbnSelchangeTableCombo)
   ON_CBN_EDITCHANGE(IDC_TABLE_COMBO, &CDBConnDlg::OnCbnEditchangeTableCombo)
   ON_BN_CLICKED(IDC_CONNECT, &CDBConnDlg::OnBnClickedConnect)
END_MESSAGE_MAP()


// CDBConnDlg message handlers

BOOL CDBConnDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   InitCtrls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CDBConnDlg::InitUDLFileList(void)
{
   C_STR_LIST UdlNameList;
   C_STR      *pUdlName;

   while (m_UdlComboCtrl.DeleteString(0) != CB_ERR); // svuoto la combo

   if (gsc_getUDLList(UdlNameList) != GS_GOOD) return;
   
   m_UdlComboCtrl.AddString(_T(""));
   pUdlName = (C_STR *) UdlNameList.get_head();
   while (pUdlName)
   {
      m_UdlComboCtrl.AddString(pUdlName->get_name());
      pUdlName = (C_STR *) UdlNameList.get_next();
   }
}


void CDBConnDlg::InitCtrls(void)
{
	CRect Rect;

   // Sorgente dati
   InitUDLFileList();
   m_UdlComboCtrl.SelectString(-1, UdlFile.get_name());

   // Proprietà UDL
	m_PropListCtrl.GetWindowRect(&Rect);
	m_PropListCtrl.InsertColumn(0, _T("Nome"), LVCFMT_LEFT,
		                         Rect.Width() * 3/10, 0);
	m_PropListCtrl.InsertColumn(1, _T("Valore"), LVCFMT_LEFT,
		                         Rect.Width() * 7/10 - 3, 1);
	m_PropListCtrl.SetExtendedStyle(m_PropListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

   m_PropButtonCtrl.SetParent(&m_PropListCtrl);

   InitPropListCtrl();
   InitTooltip();
   m_ConnectionState.SetWindowText(_T("Stato della connessione: Non valida."));

   // Se si vuole solo configurare una connessione OLE-DB
   if (Flags & CONN_DLG_DB_CONN_ONLY)
   {
      m_TableNameLbl.ShowWindow(SW_HIDE);
      m_TableEditCtrl.ShowWindow(SW_HIDE);
      m_TableComboCtrl.ShowWindow(SW_HIDE);
	   m_TableEditCtrl.GetWindowRect(&Rect);
      int OffSetY = Rect.Height();

      m_OkButton.GetWindowRect(&Rect);
      ScreenToClient(Rect);
      Rect.top -= OffSetY; Rect.bottom -= OffSetY;
      m_OkButton.MoveWindow(&Rect);

      m_CancelButton.GetWindowRect(&Rect);
      ScreenToClient(Rect);
      Rect.top -= OffSetY; Rect.bottom -= OffSetY;
      m_CancelButton.MoveWindow(&Rect);

      m_HelpButton.GetWindowRect(&Rect);
      ScreenToClient(Rect);
      Rect.top -= OffSetY; Rect.bottom -= OffSetY;
      m_HelpButton.MoveWindow(&Rect);

      GetWindowRect(&Rect);
      Rect.bottom -= OffSetY;
      MoveWindow(&Rect);

      TestConnection(false, GS_BAD);
      EnableOKBotton();

      return;
   }

   // Se si vuole solo configurare una connessione a un contenitore di tabelle
   // (connessione a DB, catalogo e schema) senza il nome della tabella
   if (Flags & CONN_DLG_CHOICE_CONTAINER_ONLY)
   {
      C_STRING Catalog, Schema, Table;

      m_TableNameLbl.SetWindowText(_T("Nome catalogo:"));
      m_TableNameLbl.GetWindowRect(&Rect);
      int OffSetY = Rect.Height() + 8;
      Rect.top += OffSetY; Rect.bottom += OffSetY;
      ScreenToClient(Rect);
      m_SchemaNameLbl.ShowWindow(SW_SHOW);
      m_SchemaNameLbl.MoveWindow(&Rect);
	   
      m_TableEditCtrl.GetWindowRect(&Rect);
      OffSetY = Rect.Height() + 8;
      Rect.top += OffSetY; Rect.bottom += OffSetY;
      ScreenToClient(Rect);
      m_SchemaEditCtrl.ShowWindow(SW_SHOW);
      m_SchemaEditCtrl.MoveWindow(&Rect);

      // Nome completo tabella
      if (FullRefTable.get_name() && pConn && 
          pConn->split_FullRefTable(FullRefTable.get_name(), Catalog, Schema, Table) == GS_GOOD)
      {
         m_TableEditCtrl.SetWindowText(Catalog.get_name()); // m_TableEditCtrl contiene lo schema 
         m_SchemaEditCtrl.SetWindowText(Schema.get_name());
      }

      m_OkButton.GetWindowRect(&Rect);
      ScreenToClient(Rect);
      Rect.top += OffSetY; Rect.bottom += OffSetY;
      m_OkButton.MoveWindow(&Rect);

      m_CancelButton.GetWindowRect(&Rect);
      ScreenToClient(Rect);
      Rect.top += OffSetY; Rect.bottom += OffSetY;
      m_CancelButton.MoveWindow(&Rect);

      m_HelpButton.GetWindowRect(&Rect);
      ScreenToClient(Rect);
      Rect.top += OffSetY; Rect.bottom += OffSetY;
      m_HelpButton.MoveWindow(&Rect);

      GetWindowRect(&Rect);
      Rect.bottom += OffSetY;
      MoveWindow(&Rect);

      TestConnection(false, GS_BAD);
      EnableOKBotton();

      return;
   }

   // Si vuole creare una nuova tabella
   if (Flags & CONN_DLG_CREATE)
   {
      m_TableEditCtrl.ShowWindow(SW_SHOW);
      m_TableComboCtrl.ShowWindow(SW_HIDE);
      m_TableEditCtrl.SetWindowText((FullRefTable.get_name()) ? FullRefTable.get_name() : _T(""));

      // Si vuole creare una nuova tabella chiedendone solo un prefisso
      if (Flags & CONN_DLG_CREATE_INPUT_ONLY_PREFIX)
         m_TableNameLbl.SetWindowText(_T("Prefisso nome tabella"));

      TestConnection(false, GS_BAD);
   }
   else // Si vuole scegliere una tabella o vista esistente
   {
      m_TableEditCtrl.ShowWindow(SW_HIDE);
      m_TableEditCtrl.GetWindowRect(&Rect);
      m_TableComboCtrl.ShowWindow(SW_SHOW);
      ScreenToClient(Rect);
      m_TableComboCtrl.MoveWindow(&Rect);

      TestConnection(false, GS_BAD);

      // Leggo le tabelle e le viste dal db
      InitTableViewList();
   }

   EnableOKBotton();
}

void CDBConnDlg::InitPropListCtrl(void)
{   
   presbuf   pUDLPropertiesDescr, pRb;
   C_2STR    *pUdlProperty;
   int       i = 0;
 	LV_ITEM   lvitem;
   
   lvitem.mask = LVIF_TEXT;

   m_PropListCtrl.DeleteAllItems();

   // ((<Name><Descr.><Resource Type>[CATALOG || SCHEMA]) ...)
   UDLPropertiesDescrList << gsc_getUDLPropertiesDescrFromFile(UdlFile.get_name());

   while ((pUDLPropertiesDescr = UDLPropertiesDescrList.nth(i)))
   {
      pRb = gsc_nth(0, pUDLPropertiesDescr);
   
		lvitem.iItem  = i;

      // Nome
      lvitem.iSubItem = 0;
      lvitem.pszText  = pRb->resval.rstring;
   	m_PropListCtrl.InsertItem(&lvitem);

      // Valore
      lvitem.iSubItem = 1;
      if (!(pUdlProperty = (C_2STR *) UdlProperties.search_name(pRb->resval.rstring)))
      {
         pUdlProperty = new C_2STR();
         pUdlProperty->set_name(pRb->resval.rstring);
         UdlProperties.add_tail(pUdlProperty);
      }
      lvitem.pszText = pUdlProperty->get_name2();
   	m_PropListCtrl.SetItem(&lvitem);
      
      i++;
   }
   m_PropButtonCtrl.ShowWindow(SW_HIDE);
}


void CDBConnDlg::InitTableViewList(void)
{
   C_STR_LIST ItemList, PartialList;
   C_STR      *pItem;

   // se si deve scegliere solo una connessione a db 
   // o si deve creare una tabella esistente o
   // si deve scegliere un contenitore di tabelle (connessione a DB, catalogo e schema senza il nome della tabella)
   if ((Flags & CONN_DLG_DB_CONN_ONLY) || (Flags & CONN_DLG_CREATE) || 
      (Flags & CONN_DLG_CHOICE_CONTAINER_ONLY))
      return;

   while (m_TableComboCtrl.DeleteString(0) != CB_ERR); // svuoto la combo

   if (pConn == NULL) return;

   // Leggo la lista delle tabelle e delle viste
   //                                       Cata, Schema, Item name
   if (pConn->getTableAndViewList(ItemList, NULL, NULL, NULL,
                                  (Flags & CONN_DLG_CHOICE_ON_TABLE) ? true : false,
                                  _T("TABLE"),
                                  (Flags & CONN_DLG_CHOICE_ON_VIEW) ? true : false) != GS_GOOD) return;
   // Leggo la lista delle tabelle collegate (vedi ACCESS)
   //                                       Cata, Schema, Item name
   if (pConn->getTableAndViewList(PartialList, NULL, NULL, NULL,
                                  (Flags & CONN_DLG_CHOICE_ON_TABLE) ? true : false,
                                  _T("LINK"),
                                  false) != GS_GOOD) return;
   ItemList.paste_tail(PartialList); // aggiungo alla lista
   // Leggo la lista delle query (vedi ACCESS)
   //                                       Cata, Schema, Item name
   if (pConn->getTableAndViewList(PartialList, NULL, NULL, NULL,
                                  (Flags & CONN_DLG_CHOICE_ON_TABLE) ? true : false,
                                  _T("VIEW"),
                                  false) != GS_GOOD) return;
   ItemList.paste_tail(PartialList); // aggiungo alla lista

   pItem = (C_STR *) ItemList.get_head();
   while (pItem)
   {
      m_TableComboCtrl.AddString(pItem->get_name());
      pItem = (C_STR *) ItemList.get_next();
   }
   m_TableComboCtrl.SelectString(-1, FullRefTable.get_name());
}

void CDBConnDlg::OnCbnSelchangeUdlListCombo()
{
   CString dummy;

   m_UdlComboCtrl.GetWindowText(dummy);
   if (UdlFile.get_name() == NULL || dummy.CompareNoCase(UdlFile.get_name()) != 0)
   {
      UdlFile = dummy.Trim();
      UdlProperties.remove_all();
      InitPropListCtrl();
      // verifico la connessione senza richiedere login e password all'utente
      // e senza stampare messaggi di errore in caso di esito negativo
      TestConnection(false, GS_BAD);
      // Se si vuole selezionare una tabella esistente
      if (!(Flags & CONN_DLG_DB_CONN_ONLY) && !(Flags & CONN_DLG_CREATE) && 
          !(Flags & CONN_DLG_CHOICE_CONTAINER_ONLY))
      {
         FullRefTable.clear();
         m_TableEditCtrl.SetWindowText(_T(""));
         m_SchemaEditCtrl.SetWindowText(_T(""));
         // Leggo le tabelle e le viste dal db
         InitTableViewList();
      }
   }
   EnableOKBotton();
}


///////////////////////////////////////////////////////////////////////////////
// FUNZIONI PER TOOLTIP - INIZIO
///////////////////////////////////////////////////////////////////////////////


// Le coordinate espresse da pt sono "Screen"
BOOL CDBConnDlg::DisplayTooltip(MSG* pMsg)
{
   CRect   Rect, ScreenRect;
   int     iItem, StartItem, EndItem;
   CString ItemText, PrevItemText;

   StartItem = m_PropListCtrl.GetTopIndex();
   EndItem   = StartItem + m_PropListCtrl.GetCountPerPage();

   for (iItem = StartItem; iItem < EndItem; iItem++)
      if (m_PropListCtrl.GetSubItemRect(iItem, 0, LVIR_BOUNDS, Rect) == TRUE)
      {
         ScreenRect = Rect;
         m_PropListCtrl.ClientToScreen(ScreenRect);
         if (ScreenRect.PtInRect(pMsg->pt))
            break;
         else
         if (m_PropListCtrl.GetSubItemRect(iItem, 1, LVIR_BOUNDS, Rect) == TRUE)
         {
            ScreenRect = Rect;
            m_PropListCtrl.ClientToScreen(ScreenRect);
            if (ScreenRect.PtInRect(pMsg->pt)) break;
         }
      }

   // Se non era su una riga del controllo
   if (iItem == EndItem) return TRUE;

   presbuf pUDLPropertiesDescr = UDLPropertiesDescrList.nth(iItem);

   if (pUDLPropertiesDescr && 
       (pUDLPropertiesDescr = gsc_nth(1, pUDLPropertiesDescr)) &&
       pUDLPropertiesDescr->restype == RTSTR)
      ItemText = pUDLPropertiesDescr->resval.rstring;
   else
      ItemText = _T("");

   pToolTip->GetText(PrevItemText, &m_PropListCtrl);
   if (PrevItemText.Compare(ItemText) != 0)
      pToolTip->UpdateTipText(ItemText, &m_PropListCtrl);
   pToolTip->RelayEvent(pMsg);

   return TRUE;
}


BOOL CDBConnDlg::InitTooltip(void)
{
   if (pToolTip) delete pToolTip;
   pToolTip = new CToolTipCtrl;
   if (!pToolTip->Create(this, TTS_ALWAYSTIP)) return FALSE;
	pToolTip->AddTool(&m_PropListCtrl, _T(""));

   pToolTip->SetDelayTime(TTDT_INITIAL, 0);
   pToolTip->Activate(TRUE);
   
   return TRUE;
}


BOOL CDBConnDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONDBLCLK)
       && pMsg->wParam == MK_LBUTTON && pMsg->hwnd == m_PropButtonCtrl.m_hWnd)
   {
      OnBnClickedPropButton();
      return 1;
   }

   DisplayTooltip(pMsg);

   return CDialog::PreTranslateMessage(pMsg);
}


///////////////////////////////////////////////////////////////////////////////
// FUNZIONI PER TOOLTIP - FINE
///////////////////////////////////////////////////////////////////////////////


void CDBConnDlg::OnNMClickUdlPropertiesList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
   CRect        rcItem, rcListCtrl;
   CString      ActualValue;
   int          BottonWidth = 20;

   if (pNMListView->iItem == -1) return; // nessuna selezione

   // se non era già stata selezionata questa casella
   if (Prev_iItem != pNMListView->iItem)
   {
      Prev_iItem = pNMListView->iItem;
      return;
   }

   m_PropListCtrl.GetSubItemRect(pNMListView->iItem, 1, LVIR_BOUNDS, rcItem);
   m_PropListCtrl.GetClientRect(rcListCtrl);
   if (rcListCtrl.right < rcItem.right) rcItem.right = rcListCtrl.right;
   rcItem.top -= 1; rcItem.bottom += 2;
   ActualValue   = m_PropListCtrl.GetItemText(pNMListView->iItem, 1);

   ResourceTypeEnum PropType;
   getCurrPropertyType(&PropType);
   switch (PropType)
   {
      case DirectoryRes: case FileRes:
         m_PropButtonCtrl.SetWindowPos(NULL, rcItem.right - BottonWidth, rcItem.top,
   		                              BottonWidth, rcItem.Height(), SWP_SHOWWINDOW);
         break;
      default:
         BottonWidth = 0;
   }

   m_PropEditCtrl.SetWindowText(ActualValue);
   m_PropListCtrl.ClientToScreen(&rcItem);
   ScreenToClient(rcItem);
   m_PropEditCtrl.SetWindowPos(NULL, rcItem.left, rcItem.top,
   		                      rcItem.Width() - BottonWidth, rcItem.Height(), SWP_SHOWWINDOW);
   m_PropEditCtrl.SetFocus();
   m_PropEditCtrl.SetSel(0, -1); // Seleziona tutto il testo

   *pResult = 0;
}

void CDBConnDlg::OnEnKillfocusPropEdit()
{
   CString Value, Name;

   m_PropEditCtrl.ShowWindow(SW_HIDE);	
   m_PropEditCtrl.GetWindowText(Value);

   SetCurrPropValue(Value);
}

void CDBConnDlg::OnBnClickedPropButton()
{
   presbuf          pUDLPropertiesDescr = UDLPropertiesDescrList.nth(Prev_iItem);
   CString          DefExt, PropValue, Filter;
   ResourceTypeEnum PropType;
   C_2STR           *pItem = (C_2STR *) UdlProperties.getptr_at(Prev_iItem + 1);

   m_PropButtonCtrl.SetFocus();
   getCurrPropertyType(&PropType, &DefExt);
   switch (PropType)
   {
      case DirectoryRes:
      {
         C_STRING OutDir;

         if (gsc_dd_get_existingDir(_T("Selezionare una cartella."), 
                                    (pItem) ? pItem->get_name2() : NULL,
                                    OutDir) == GS_GOOD)
         {
            PropValue = OutDir.get_name();
            SetCurrPropValue(PropValue);
         }
         break;
      }
      case FileRes:
      {
         Filter = _T("*.");
         Filter += DefExt;
         Filter += _T("|*.");
         Filter += DefExt;
         Filter += _T("||");
         CFileDialog Dlg(TRUE, DefExt, (pItem) ? pItem->get_name2() : NULL, 
                         OFN_HIDEREADONLY, Filter);
         if (Dlg.DoModal() == IDOK)
         {
            PropValue = Dlg.GetPathName();
            SetCurrPropValue(PropValue);
         }
         break;
      }
   }
   m_PropButtonCtrl.ShowWindow(SW_HIDE);
}

bool CDBConnDlg::getCurrPropertyType(ResourceTypeEnum *PropType, CString *DefExt)
{
   presbuf pUDLPropertiesDescr = UDLPropertiesDescrList.nth(Prev_iItem);
   TCHAR   *p;

   if (PropType) *PropType = EmptyRes;
   if (DefExt) DefExt->Empty();

   if (pUDLPropertiesDescr && 
       (pUDLPropertiesDescr = gsc_nth(2, pUDLPropertiesDescr)) &&
       pUDLPropertiesDescr->restype == RTSTR && (p = pUDLPropertiesDescr->resval.rstring))
   {
      if (gsc_strcmp(p, _T("DIR"), FALSE) == 0)
      {
         if (PropType) *PropType = DirectoryRes;
         return true;
      }
      else
      if (gsc_strstr(p, _T("FILE"), FALSE) == p) // incomincia per "FILE"
      {
         if (DefExt)
         {
            CString dummy(p);
            int init = dummy.ReverseFind(_T('('));
            int final = dummy.ReverseFind(_T(')'));
            if (init >= 0 && final > init)
               (*DefExt) = dummy.Mid(init + 1, final - init - 1);
         }

         if (PropType) *PropType = FileRes;

         return true; 
      }
      else
      if (gsc_strcmp(p, _T("SERVER"), FALSE) == 0)
      {
         if (PropType) *PropType = ServerRes;

         return true;
      }
   }

   return false;
}
bool CDBConnDlg::EnableOKBotton(void)
{
   bool Result = false;

   // Verifico la connessione OLE-DB
   if (pConn)
      // Se non si deve gestire solo la connessione OLE-DB
      if (!(Flags & CONN_DLG_DB_CONN_ONLY))
      {
         FullRefTable.alltrim();
         // Se si vuole solo configurare una connessione a un contenitore di tabelle
         // (connessione a DB, catalogo e schema) senza il nome della tabella
         if (Flags & CONN_DLG_CHOICE_CONTAINER_ONLY)
            Result = true;
         else
            if (FullRefTable.len() > 0) 
               Result = true;
      }
      else
         Result = true;

   m_OkButton.EnableWindow(Result);

   return Result;
}

void CDBConnDlg::OnEnChangeTableEdit()
{
   // TODO:  If this is a RICHEDIT control, the control will not
   // send this notification unless you override the CDialog::OnInitDialog()
   // function and call CRichEditCtrl().SetEventMask()
   // with the ENM_CHANGE flag ORed into the mask.
   CString dummy;

   m_TableEditCtrl.GetWindowText(dummy);

   // Se si vuole solo configurare una connessione a un contenitore di tabelle
   // (connessione a DB, catalogo e schema) senza il nome della tabella
   if (Flags & CONN_DLG_CHOICE_CONTAINER_ONLY)
   {
      // Verifico la connessione OLE-DB
      if (pConn == NULL) return;

      C_STRING Catalog, Schema, Table(_T("nessuna"));

      // Se supporta l'uso del catalogo
      if (pConn->get_CatalogUsage() != 0)
         Catalog = dummy.Trim();
      // Se supporta l'uso dello schema
      if (pConn->get_SchemaUsage() != 0)
      {
         m_SchemaEditCtrl.GetWindowText(dummy);
         Schema = dummy.Trim();
      }

      if (FullRefTable.paste(pConn->get_FullRefTable(Catalog, Schema, Table)) == NULL)
         return;
   }
   else
      FullRefTable = dummy.Trim();

   EnableOKBotton();
}


void CDBConnDlg::OnEnChangeSchemaEdit()
{
   // TODO:  If this is a RICHEDIT control, the control will not
   // send this notification unless you override the CDialog::OnInitDialog()
   // function and call CRichEditCtrl().SetEventMask()
   // with the ENM_CHANGE flag ORed into the mask.
   CString dummy;

   m_TableEditCtrl.GetWindowText(dummy);

   // Se si vuole solo configurare una connessione a un contenitore di tabelle
   // (connessione a DB, catalogo e schema) senza il nome della tabella
   if (Flags & CONN_DLG_CHOICE_CONTAINER_ONLY)
   {
      // Verifico la connessione OLE-DB
      if (pConn == NULL) return;

      C_STRING Catalog, Schema, Table(_T("nessuna"));

      // Se supporta l'uso del catalogo
      if (pConn->get_CatalogUsage() != 0)
         Catalog = dummy.Trim();
      // Se supporta l'uso dello schema
      if (pConn->get_SchemaUsage() != 0)
      {
         m_SchemaEditCtrl.GetWindowText(dummy);
         Schema = dummy.Trim();
      }

      if (FullRefTable.paste(pConn->get_FullRefTable(Catalog, Schema, Table)) == NULL)
         return;
   }
   else
      FullRefTable = dummy.Trim();

   EnableOKBotton();
}

void CDBConnDlg::OnCbnSelchangeTableCombo()
{
   int     CurSel;
   CString dummy;

   if ((CurSel = m_TableComboCtrl.GetCurSel()) == CB_ERR) return;
   m_TableComboCtrl.GetLBText(CurSel, dummy);
   FullRefTable = dummy.Trim();

   EnableOKBotton();
}
void CDBConnDlg::OnCbnEditchangeTableCombo()
{
   CString dummy;

   //OnCbnSelchangeTableCombo();
   m_TableComboCtrl.GetWindowText(dummy);
   FullRefTable = dummy.Trim();

   EnableOKBotton();
}

void CDBConnDlg::OnBnClickedOk()
{
   // Se si esce con OK durante la digitazione di una proprietà
   if (m_PropEditCtrl.IsWindowVisible()) OnEnKillfocusPropEdit();

   if (EnableOKBotton()) // se non c'è connessione non abilita OK
   {
      // Se si vuole gestire anche la tabella
      if (!(Flags & CONN_DLG_DB_CONN_ONLY) && !(Flags & CONN_DLG_CHOICE_CONTAINER_ONLY))
      {
         int      Exist = pConn->ExistTable(FullRefTable.get_name());
         C_STRING Msg;

         Msg = _T("Tabella ");
         Msg += FullRefTable;

         if (Flags & CONN_DLG_CREATE) // Se si vuole creare una nuova tabella
         {
            // Si vuole creare una nuova tabella senza chiederne solo un prefisso
            if (!(Flags & CONN_DLG_CREATE_INPUT_ONLY_PREFIX))
               if (Exist == GS_GOOD)
               {
                  Msg += _T(" già esistente.");
                  gsc_ddalert(Msg.get_name(), m_hWnd);
                  return;
               }  
         }
         else // Se si vuole selezionare una tabella esistente
            if (Exist != GS_GOOD)
            {
               Msg += _T(" non esistente.");
               gsc_ddalert(Msg.get_name(), m_hWnd);
               return;
            }  

      }

      // Salvo il tipo di connessione
      C_STRING Buffer;

      gsc_setInfoToINI(_T("LAST_UDL"), UdlFile);
      Buffer.paste(gsc_PropListToConnStr(UdlProperties));
      gsc_setInfoToINI(_T("LAST_UDL_PROPERTIES"), Buffer);

      OnOK();
   }
}


bool CDBConnDlg::SetCurrPropValue(CString &NewValue)
{
   presbuf          pUDLPropertiesDescr = UDLPropertiesDescrList.nth(Prev_iItem);
   ResourceTypeEnum PropType;
   C_2STR           *pItem = (C_2STR *) UdlProperties.getptr_at(Prev_iItem + 1);

   NewValue.Trim();
   if (pItem->get_name2() == NULL && NewValue.GetLength() == 0) return true;
   if (gsc_strcmp(pItem->get_name2(), NewValue) == 0) return true;

   if (NewValue.GetLength() > 0)
   {
      getCurrPropertyType(&PropType);
      switch (PropType)
      {
         case DirectoryRes:
         {
            // se si deve specificare una cartella esistente
            if (!(Flags & CONN_DLG_CREATE))
               if (gsc_path_exist(NewValue) == GS_BAD)
                  { gsc_ddalert(_T("Cartella non esistente."), m_hWnd); return GS_BAD; }
            
            break;
         }
         case FileRes:
         {
            // se si deve specificare un file esistente
            if (!(Flags & CONN_DLG_CREATE))
               if (gsc_path_exist(NewValue) == GS_BAD)
                  { gsc_ddalert(_T("File non esistente."), m_hWnd); return GS_BAD; }
            
            break;
         }
      }
   }

   // Aggiorno valore in UdlProperties
   CString Name = m_PropListCtrl.GetItemText(Prev_iItem, 0); // nome proprietà
   ((C_2STR *) UdlProperties.search_name(Name))->set_name2(NewValue);

   // verifico la connessione senza richiedere login e password all'utente
   // e senza stampare messaggi di errore in caso di esito negativo
   TestConnection(false, GS_BAD);

   // Aggiorno valore in m_PropListCtrl
   m_PropListCtrl.SetItemText(Prev_iItem, 1, NewValue);
   InitTableViewList();
   EnableOKBotton();

   return GS_GOOD;
}

bool CDBConnDlg::TestConnection(bool Prompt, int PrintError)
{
   CWaitCursor Csr;

   // Verifico la connessione OLE-DB
   if ((pConn = get_pDBCONNECTION_LIST()->get_Connection(UdlFile.get_name(),
                                                         &UdlProperties,
                                                         Prompt,
                                                         PrintError)) == NULL)
   {
      if (m_ConnectionState.m_hWnd)
         m_ConnectionState.SetWindowText(_T("Stato della connessione: Non valida."));

      if (PrintError == GS_GOOD) gsc_ddalert(_T("Connessione fallita."), m_hWnd);
      return false;
   }
   else
   {
      if (m_ConnectionState.m_hWnd)
         m_ConnectionState.SetWindowText(_T("Stato della connessione: Valida."));

      if (m_TableEditCtrl.m_hWnd)
         // se NON si deve scegliere una connessione a db e 
         // si deve creare una nuova tabella oppure
         // si vuole solo configurare una connessione a un contenitore di tabelle
         // (connessione a DB, catalogo e schema) senza il nome della tabella
         if (!(Flags & CONN_DLG_DB_CONN_ONLY) && 
             ((Flags & CONN_DLG_CREATE) || (Flags & CONN_DLG_CHOICE_CONTAINER_ONLY)))
         {
            CString txt;

            m_TableEditCtrl.GetWindowText(txt);

            switch (pConn->get_StrCaseFullTableRef())
            {
               case Upper:
                  m_TableEditCtrl.ModifyStyle(ES_LOWERCASE, ES_UPPERCASE);
                  txt.MakeUpper();
                  m_TableEditCtrl.SetWindowText(txt);
                  break;
               case Lower:
                  m_TableEditCtrl.ModifyStyle(ES_UPPERCASE, ES_LOWERCASE);
                  txt.MakeLower();
                  m_TableEditCtrl.SetWindowText(txt);
                  break;
               case NoSensitive:
                  m_TableEditCtrl.ModifyStyle(ES_LOWERCASE | ES_UPPERCASE, 0);
                  break;
            }

            if (Flags & CONN_DLG_CHOICE_CONTAINER_ONLY)
               // Se supporta l'uso dello catalogo
               if (pConn->get_CatalogUsage() != 0)
                  m_TableEditCtrl.EnableWindow(TRUE);
               else
                  m_TableEditCtrl.EnableWindow(FALSE);
         }

      if (m_SchemaEditCtrl.m_hWnd)
         // Se si vuole solo configurare una connessione a un contenitore di tabelle
         // (connessione a DB, catalogo e schema) senza il nome della tabella
         if (Flags & CONN_DLG_CHOICE_CONTAINER_ONLY)
         {
            CString txt;

            m_SchemaEditCtrl.GetWindowText(txt);

            switch (pConn->get_StrCaseFullTableRef())
            {
               case Upper:
                  m_SchemaEditCtrl.ModifyStyle(ES_LOWERCASE, ES_UPPERCASE);
                  txt.MakeUpper();
                  m_SchemaEditCtrl.SetWindowText(txt);
                  break;
               case Lower:
                  m_SchemaEditCtrl.ModifyStyle(ES_UPPERCASE, ES_LOWERCASE);
                  txt.MakeLower();
                  m_SchemaEditCtrl.SetWindowText(txt);
                  break;
               case NoSensitive:
                  m_SchemaEditCtrl.ModifyStyle(ES_LOWERCASE | ES_UPPERCASE, 0);
                  break;
            }

            // Se supporta l'uso dello schema
            if (pConn->get_SchemaUsage() != 0)
               m_SchemaEditCtrl.EnableWindow(TRUE);
            else
               m_SchemaEditCtrl.EnableWindow(FALSE);
         }

      return true;
   }
}


/****************************************************************/
/*.doc CDBConnDlg::FromRbList                    <external> */
/*+
  Questa funzione consente di inizializzare la connessione da una lista 
  di resbuf composta come di seguito:
  (<UDL file> ((<UDLPropName><UDLPropVal>)...))
-*/  
/****************************************************************/
void CDBConnDlg::FromRbList(resbuf *RbList)
{
   resbuf *pRb;

   UdlFile.clear();
   UdlProperties.remove_all();
   FullRefTable.clear();

   if (!RbList || RbList->restype == RTNIL)
   {  // se non specificato prova a caricare l'ultima connessione usata
      C_STRING Buffer;

      gsc_getInfoFromINI(_T("LAST_UDL"), UdlFile);
      if (gsc_getInfoFromINI(_T("LAST_UDL_PROPERTIES"), Buffer) == GS_GOOD)
          gsc_PropListFromConnStr(Buffer.get_name(), UdlProperties);
   }
   else
   {
      // Nome file UDL
      if ((pRb = gsc_CdrAssoc(_T("UDL_FILE"), RbList, FALSE)) && pRb->restype == RTSTR)
         UdlFile = pRb->resval.rstring;
      // Proprietà connessione
      if ((pRb = gsc_CdrAssoc(_T("UDL_PROP"), RbList, FALSE)))
         if (pRb->restype == RTSTR) // Le proprietà sono in forma di stringa
         {
            // traduco stringa in lista
            if (gsc_PropListFromConnStr(pRb->resval.rstring, UdlProperties) == GS_BAD) return;
         }
         else
         if (pRb->restype == RTLB) // Le proprietà sono in forma di lista
         {
            if (gsc_getUDLProperties(&pRb, UdlProperties) == GS_BAD) return;
         }
         else
            UdlProperties.remove_all();
   }

   // Conversione path UDLProperties da assoluto in dir relativo
   if (gsc_UDLProperties_nethost2drive(UdlFile.get_name(), UdlProperties) == GS_BAD)
      return;

   // verifico la connessione senza richiedere login e password all'utente
   // e senza stampare messaggi di errore in caso di esito negativo
   if (TestConnection(false, GS_BAD) &&
       (RbList = gsc_CdrAssoc(_T("TABLE_REF"), RbList, FALSE))) // Nome completo tabella
      if (RbList->restype == RTSTR)
         FullRefTable = RbList->resval.rstring;
      else
      if (RbList->restype == RTLB)
      {
         C_STRING Catalog, Schema, Table;

         if ((pRb = gsc_nth(0, RbList)) && pRb->restype == RTSTR) // Catalogo
            Catalog = pRb->resval.rstring;
         if ((pRb = gsc_nth(1, RbList)) && pRb->restype == RTSTR) // Schema
            Schema = pRb->resval.rstring;
         if ((pRb = gsc_nth(2, RbList)) && pRb->restype == RTSTR) // Tabella
            Table = pRb->resval.rstring;

         FullRefTable.paste(pConn->get_FullRefTable(Catalog.get_name(), 
                                                    Schema.get_name(), 
                                                    Table.get_name()));
      }
}
/****************************************************************/
/*.doc CDBConnDlg::ToRbList                    <external> */
/*+
  Questa funzione restituisce il risultato della classe in una lista 
  di resbuf composta come di seguito:
  (("UDL_FILE" <Connection>) ("UDL_PROP" <Properties>) ("TABLE_REF" <value>))
  dove
  <Connection> = <file UDL> || <stringa di connessione>
  <Properties> = stringa delle proprietà | ((<prop1><value>)(<prop1><value>)...)
-*/  
/****************************************************************/
resbuf *CDBConnDlg::ToRbList(void)
{
   C_RB_LIST RbList;
   C_STRING Catalog, Schema, Table;

   if (!pConn) return NULL;

   // Nome file UDL
   RbList << acutBuildList(RTLB, RTSTR, _T("UDL_FILE"), RTSTR, UdlFile.get_name(), RTLE, 
                           RTLB, RTSTR, _T("UDL_PROP"), RTLB, 0);
   // Proprietà connessione
   RbList += UdlProperties.to_rb();
   RbList += acutBuildList(RTLE, RTLE, 0);
   // Nome completo tabella
   if (pConn->split_FullRefTable(FullRefTable.get_name(), Catalog, Schema, Table) == GS_GOOD)
   {
      RbList += acutBuildList(RTLB, RTSTR, _T("TABLE_REF"), RTLB, 0);
      RbList += acutBuildList(RTSTR, (Catalog.get_name()) ? Catalog.get_name() : _T(""), 0);
      RbList += acutBuildList(RTSTR, (Schema.get_name()) ? Schema.get_name() : _T(""), 0);
      RbList += acutBuildList(RTSTR, (Table.get_name()) ? Table.get_name() : _T(""), RTLE, RTLE, 0);
   }

   RbList.ReleaseAllAtDistruction(GS_BAD);

   return RbList.get_head();
}

void CDBConnDlg::OnBnClickedConnect()
{
   // prova di connessione con eventuale richiesta all'utente di login e password
   // e stampa di eventual messaggi di errore
   TestConnection(true, GS_GOOD);
}


/****************************************************************/
/*.doc gsui_DBConn                                   <external> */
/*+
  Questa funzione LISP consente di impostare il collegamento a una
  sorgente dati.
  Parametri: 
  (("UDL_FILE" <Connection>) ("UDL_PROP" <Properties>) ("TABLE_REF" <TableRef>))
  dove
  <Connection> = <file UDL> | <stringa di connessione>
  <Properties> = stringa delle proprietà | ((<prop1><value>)(<prop1><value>)...)
  <TableRef>   = riferimento completo tabella | (<cat><schema><tabella>)

  Restituisce la lista composta come sopra in caso di successo 
  altrimenti restituisce nil.
-*/  
/****************************************************************/
int gsui_DBConn(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   CDBConnDlg MyDlg;
   C_RB_LIST  ret;

   acedRetNil();
   MyDlg.FromRbList(acedGetArgs());
   MyDlg.Flags = CONN_DLG_DB_CONN_ONLY | CONN_DLG_CREATE;
   if (MyDlg.DoModal() == IDOK && (ret << MyDlg.ToRbList()))
      ret.LspRetList();

   return GS_GOOD;
}


/****************************************************************/
/*.doc gsui_ExistingTabConn                          <external> */
/*+
  Questa funzione LISP consente di impostare il collegamento a una
  tabella esistente.
  Parametri: 
  (("UDL_FILE" <Connection>) ("UDL_PROP" <Properties>) ("TABLE_REF" <TableRef>))
  dove
  <Connection> = <file UDL> | <stringa di connessione>
  <Properties> = stringa delle proprietà | ((<prop1><value>)(<prop1><value>)...)
  <TableRef>   = riferimento completo tabella | (<cat><schema><tabella>)

  Restituisce la lista composta come sopra in caso di successo 
  altrimenti restituisce nil.
-*/  
/****************************************************************/
int gsui_ExistingTabConn(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   CDBConnDlg MyDlg;
   C_RB_LIST  ret;

   acedRetNil();
   MyDlg.FromRbList(acedGetArgs());
   MyDlg.Flags = CONN_DLG_CHOICE_ON_TABLE | CONN_DLG_CHOICE_ON_VIEW;
   if (MyDlg.DoModal() == IDOK && (ret << MyDlg.ToRbList()))
      ret.LspRetList();

   return GS_GOOD;
}


/****************************************************************/
/*.doc gsui_NewTabConn                               <external> */
/*+
  Questa funzione LISP consente di impostare il collegamento a una
  tabella nuova da creare.
  Parametri: 
  (("UDL_FILE" <Connection>) ("UDL_PROP" <Properties>) ("TABLE_REF" <TableRef>))
  dove
  <Connection> = <file UDL> | <stringa di connessione>
  <Properties> = stringa delle proprietà | ((<prop1><value>)(<prop1><value>)...)
  <TableRef>   = riferimento completo tabella | (<cat><schema><tabella>)

  Restituisce la lista composta come sopra in caso di successo 
  altrimenti restituisce nil.
-*/  
/****************************************************************/
int gsui_NewTabConn(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   CDBConnDlg MyDlg;
   C_RB_LIST  ret;

   acedRetNil();
   MyDlg.FromRbList(acedGetArgs());
   MyDlg.Flags = CONN_DLG_CREATE;
   if (MyDlg.DoModal() == IDOK && (ret << MyDlg.ToRbList()))
      ret.LspRetList();

   return GS_GOOD;
}

/****************************************************************/
/*.doc gsui_NewPrefixTabConn                         <external> */
/*+
  Questa funzione LISP consente di impostare il collegamento a una
  tabella nuova da creare chiedendone solo il prefisso.
  Parametri: 
  (("UDL_FILE" <Connection>) ("UDL_PROP" <Properties>) ("TABLE_REF" <TableRef>))
  dove
  <Connection> = <file UDL> | <stringa di connessione>
  <Properties> = stringa delle proprietà | ((<prop1><value>)(<prop1><value>)...)
  <TableRef>   = riferimento completo tabella | (<cat><schema><tabella>)

  Restituisce la lista composta come sopra in caso di successo 
  altrimenti restituisce nil.
-*/  
/****************************************************************/
int gsui_NewPrefixTabConn(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   CDBConnDlg MyDlg;
   C_RB_LIST  ret;

   acedRetNil();
   MyDlg.FromRbList(acedGetArgs());
   MyDlg.Flags = CONN_DLG_CREATE | CONN_DLG_CREATE_INPUT_ONLY_PREFIX;
   if (MyDlg.DoModal() == IDOK && (ret << MyDlg.ToRbList()))
      ret.LspRetList();

   return GS_GOOD;
}


/****************************************************************/
/*.doc gsui_TabContainer                             <external> */
/*+
  Questa funzione LISP consente di impostare il collegamento a un contenitore di tabelle
  (connessione a DB, catalogo e schema) senza il nome della tabella.
  Parametri: 
  (("UDL_FILE" <Connection>) ("UDL_PROP" <Properties>) ("TABLE_REF" <TableRef>))
  dove
  <Connection> = <file UDL> | <stringa di connessione>
  <Properties> = stringa delle proprietà | ((<prop1><value>)(<prop1><value>)...)
  <TableRef>   = riferimento completo tabella | (<cat><schema><tabella>)

  Restituisce la lista composta come sopra in caso di successo 
  altrimenti restituisce nil.
-*/  
/****************************************************************/
int gsui_TabContainer(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   CDBConnDlg MyDlg;
   C_RB_LIST  ret;

   acedRetNil();
   MyDlg.FromRbList(acedGetArgs());
   MyDlg.Flags = CONN_DLG_CHOICE_CONTAINER_ONLY;
   if (MyDlg.DoModal() == IDOK && (ret << MyDlg.ToRbList()))
      ret.LspRetList();

   return GS_GOOD;
}
