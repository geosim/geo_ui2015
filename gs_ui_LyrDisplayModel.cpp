// gs_ui_LyrDisplayModel.cpp : file di implementazione
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"		// simboli principali
#include "gs_ui_utily.h"

#include "gs_thm.h"
#include "gs_init.h"
#include "gs_utily.h"
#include "gs_ui_LyrDisplayModel.h"

#include "d2hMap.h" // doc to help


// finestra di dialogo CLyrDisplayModelDialog
void CLyrDisplayModelDialog::LoadFromGS(void)
{
   C_LAYER_DISPLAY_MODEL *pCurrLyrDispModel = get_LAYER_DISPLAY_MODEL();

   if (pCurrLyrDispModel->get_count() > 0)
   {
      C_STRING Buffer;

      Buffer = pCurrLyrDispModel->get_descr();
      Buffer += _T(" (");
      Buffer += pCurrLyrDispModel->get_path();
      Buffer += _T(')');

      m_CurrLyrDispModel.SetWindowText(Buffer.get_name());
   }
   else
      m_CurrLyrDispModel.SetWindowText(_T("nessuno")); // msg
}

void CLyrDisplayModelDialog::RefreshIntervalsList(void)
{
	int              iRow = 0;
	LV_ITEM          lvitem;
	CRect            rect;
   C_LAYER_SETTINGS *pValues;

	m_IntervalsList.SetExtendedStyle(m_IntervalsList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

   // svuoto la lista
   m_IntervalsList.DeleteAllItems();
   while (m_IntervalsList.DeleteColumn(0) != 0);

   // insert 2 columns (REPORT mode) and modify the new header items
	m_IntervalsList.GetWindowRect(&rect);

	m_IntervalsList.InsertColumn(0, _T("Ingrandimento min."), LVCFMT_LEFT, // msg
		                          rect.Width() * 5/20, 0);
	m_IntervalsList.InsertColumn(1, _T("File di configurazione piani"), LVCFMT_LEFT, // msg
		                          rect.Width() * 15/20 - 3, 1);

   m_LyrDispModel.sort_nameByNum();
   pValues = (C_LAYER_SETTINGS *) m_LyrDispModel.get_head();
   while (pValues)
   {  // ingrandimento minimo
      lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
		lvitem.iItem    = iRow++;
      lvitem.iSubItem = 0;
      if (iRow == 0 && pValues->get_min_scale() != 0) // prima riga
         lvitem.pszText  = _T("Considerato 0"); // msg
      else
         lvitem.pszText  = pValues->get_name();
      lvitem.lParam   = (LPARAM) pValues;
      m_IntervalsList.InsertItem(&lvitem);

      // Path di configurazione dei layer
      lvitem.mask     = LVIF_TEXT;
      lvitem.iSubItem++;
      lvitem.pszText  = pValues->get_path();
   	m_IntervalsList.SetItem(&lvitem);

      pValues = (C_LAYER_SETTINGS *) m_LyrDispModel.get_next();
   }
   
   if (m_IntervalsList.GetItemCount() > m_IntervalsList.GetCountPerPage())
      m_IntervalsList.SetColumnWidth(1, m_IntervalsList.GetColumnWidth(1) - 16);
}

IMPLEMENT_DYNAMIC(CLyrDisplayModelDialog, CDialog)
CLyrDisplayModelDialog::CLyrDisplayModelDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLyrDisplayModelDialog::IDD, pParent)
{
}

CLyrDisplayModelDialog::~CLyrDisplayModelDialog()
{
}

void CLyrDisplayModelDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_LIST1, m_IntervalsList);
   DDX_Control(pDX, IDC_CURRENT_LST_DISP_MODEL, m_CurrLyrDispModel);
   DDX_Control(pDX, IDC_EDIT, m_GenericEdit);
   DDX_Control(pDX, IDC_BROWSE_FILE, m_BrowseButton);
   DDX_Control(pDX, IDC_EDIT1, m_ModelDescriptionEdit);
   DDX_Control(pDX, IDSAVE, m_SaveButton);
   DDX_Control(pDX, IDLOAD, m_LoadButton);
}


BEGIN_MESSAGE_MAP(CLyrDisplayModelDialog, CDialog)
   ON_BN_CLICKED(IDNEW, OnBnClickedNew)
   ON_BN_CLICKED(IDREMOVE, OnBnClickedRemove)
   ON_BN_CLICKED(IDSAVE, OnBnClickedSave)
   ON_BN_CLICKED(IDLOAD, OnBnClickedLoad)
   ON_BN_CLICKED(IDCLOSE, OnBnClickedClose)
   ON_BN_CLICKED(IDHELP, OnBnClickedHelp)
   ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNMClickList1)
   ON_EN_KILLFOCUS(IDC_EDIT, OnEnKillfocusEdit)
   ON_NOTIFY(LVN_KEYDOWN, IDC_LIST1, OnLvnKeydownList1)
   ON_BN_CLICKED(IDC_BROWSE_FILE, OnBnClickedBrowseFile)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDNEW, OnBnClickedNew)
	ON_COMMAND(IDREMOVE, OnBnClickedRemove)  
   ON_BN_CLICKED(IDACTIVATE, OnBnClickedActivate)
   ON_BN_CLICKED(IDDEACTIVATE, OnBnClickedDeactivate)
END_MESSAGE_MAP()


// gestori di messaggi CLyrDisplayModelDialog
BOOL CLyrDisplayModelDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

   m_GenericEdit.ModifyStyle(0L, WS_BORDER); // con bordino

   RefreshIntervalsList();

   LoadFromGS();

   if (gsc_superuser() == GS_BAD) // Comandi disponibili solo a superutente
      m_SaveButton.EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CLyrDisplayModelDialog::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if ((pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONDBLCLK)
       && pMsg->wParam == MK_LBUTTON && pMsg->hwnd == m_BrowseButton.m_hWnd)
   {
      OnBnClickedBrowseFile();
      return 1;
   }

   return CDialog::PreTranslateMessage(pMsg);
}

void CLyrDisplayModelDialog::OnBnClickedNew()
{
	LV_ITEM          lvitem;
   C_STRING         Path = _T("");
   int              i = 1;
   C_LAYER_SETTINGS *pValues;
   double           ZoomFact = 0.0;

   if (m_LyrDispModel.get_count() == 1)
   {
      pValues = (C_LAYER_SETTINGS *) m_LyrDispModel.get_tail();
      ZoomFact = pValues->get_min_scale() + 1;
   }
   else if (m_LyrDispModel.get_count() > 1)
   {      
      m_LyrDispModel.sort_nameByNum();
      // leggo differenza tra gli ultimi 2 intervalli
      pValues = (C_LAYER_SETTINGS *) m_LyrDispModel.get_tail();
      ZoomFact = pValues->get_min_scale();
      pValues = (C_LAYER_SETTINGS *) m_LyrDispModel.get_prev();
      ZoomFact = ZoomFact + (ZoomFact - pValues->get_min_scale());
   }

   if (get_GS_CURRENT_WRK_SESSION())
   {
      Path = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      Path += _T('\\');
      Path += GEOSUPPORTDIR;
      Path += _T('\\');
   }

   if ((pValues = new C_LAYER_SETTINGS()) == NULL) return;
   if (m_LyrDispModel.add_tail(pValues) == GS_BAD) return;
   pValues->set_min_scale(ZoomFact);
   pValues->set_path(Path.get_name());

   lvitem.iItem  = m_IntervalsList.GetItemCount();

	// ingrandimento minimo
   lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
   lvitem.lParam   = (LPARAM) pValues;
   lvitem.iSubItem = 0;
   lvitem.pszText  = pValues->get_name(); // min scale
   m_IntervalsList.InsertItem(&lvitem);

	// Path del file di configurazione piani
 	lvitem.mask     = LVIF_TEXT;
   lvitem.iSubItem++;
   lvitem.pszText  = pValues->get_path();
   m_IntervalsList.SetItem(&lvitem);

   // Lo seleziono
   m_IntervalsList.SetFocus();
   m_IntervalsList.SetItemState(lvitem.iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
   m_IntervalsList.EnsureVisible(lvitem.iItem, FALSE);
   Prev_iItem = lvitem.iItem;

   CRect rcItem;
   long  OffSetX, OffSetY, Width;

   m_IntervalsList.GetWindowRect(rcItem);
	ScreenToClient(rcItem);
   OffSetX = rcItem.left;
   OffSetY = rcItem.top;

   m_IntervalsList.GetSubItemRect(lvitem.iItem, 0, LVIR_LABEL, rcItem);
   OffSetX += 3;
   Width   = rcItem.Width() - 2;
   Curr_iSubItem = 0;

   m_GenericEdit.SetWindowText(pValues->get_name()); // fattore di ingrandimento
	m_GenericEdit.SetWindowPos(NULL, rcItem.left + OffSetX, rcItem.top + OffSetY + 2,
   		                     Width, rcItem.Height(), SWP_SHOWWINDOW);
   m_GenericEdit.SetFocus();
   m_GenericEdit.SetSel(0, -1); // Seleziona tutto il testo
}

void CLyrDisplayModelDialog::OnBnClickedRemove()
{
   POSITION iItem = m_IntervalsList.GetFirstSelectedItemPosition();
   
   if (iItem)
   {
      C_STRING Msg(_T("Cancellare la configurazione dei piani (il file non verrà rimosso) ?")); // msg
      CString  Value;

      Value = m_IntervalsList.GetItemText((int) iItem - 1, 0); // Valore
      if (gsui_confirm(Msg.get_name(), GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
      {
         if (m_LyrDispModel.search_name(Value)) m_LyrDispModel.remove_at();
         m_IntervalsList.DeleteItem((int) iItem - 1); // cancella riga corrente
      }
   }		
}

void CLyrDisplayModelDialog::OnBnClickedSave()
{
   C_STRING Dir, filename;

   if (gsc_getPathInfoFromINI(_T("LastUsedLyrDisplayModelFile"), Dir) == GS_BAD)
   {
      if (get_GS_CURRENT_WRK_SESSION())
         Dir = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      else
         Dir = get_GEODIR();

      Dir += _T('\\');
      Dir += GEOSUPPORTDIR;

      if (gsc_get_tmp_filename(Dir.get_name(), _T("LayerDisplayModel"), _T(".ldm"), Dir) == GS_BAD)
         return;
   }

   // "Salva modello di visualizzazione piani"
   if (gsc_GetFileD(_T("GEOsim - Salva modello di visualizzazione piani"),
                    Dir, _T("ldm"), UI_SAVEFILE_FLAGS, filename) == RTNORM)
   {
      CString  dummy;

      m_ModelDescriptionEdit.GetWindowText(dummy);
      m_LyrDispModel.set_descr(dummy);
      m_LyrDispModel.save(filename);

      // Scrivo nel file GEOSIM.INI
      gsc_setPathInfoToINI(_T("LastUsedLyrDisplayModelFile"), filename);
   }
}

void CLyrDisplayModelDialog::OnBnClickedLoad()
{
   C_STRING filename, str, GSIniFile;

   if (gsc_getPathInfoFromINI(_T("LastUsedLyrDisplayModelFile"), str) == GS_BAD)
   {
      if (get_GS_CURRENT_WRK_SESSION())
         str = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      else
         str = get_GEODIR();

      str += _T('\\');
      str += GEOSUPPORTDIR;
      str += _T('\\');
   }

   // "Carica modello di visualizzazione piani"
   if (gsc_GetFileD(_T("GEOsim - Carica modello di visualizzazione piani"),
                    str, _T("ldm"), UI_LOADFILE_FLAGS, filename) == RTNORM)
   {
      m_LyrDispModel.load(filename);
      RefreshIntervalsList();

      m_ModelDescriptionEdit.SetWindowText((m_LyrDispModel.get_descr()) ? m_LyrDispModel.get_descr() : _T(""));

      gsc_setPathInfoToINI(_T("LastUsedLyrDisplayModelFile"), filename);
   }
}

void CLyrDisplayModelDialog::OnBnClickedClose()
{
	CDialog::OnOK();
}

void CLyrDisplayModelDialog::OnBnClickedHelp()
{
   gsc_help(IDH_Gestionemodellidivisualizzazionedeipiani);
}

void CLyrDisplayModelDialog::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
   CRect        rcItem;
   CString      ActualValue;
   long         OffSetX, OffSetY, Width;

   if (pNMListView->iItem == -1) return; // nessuna selezione

   // se non era già stata selezionata questa casella
   if (Prev_iItem != pNMListView->iItem)
   {
      Prev_iItem = pNMListView->iItem;
      return;
   }

   m_IntervalsList.GetWindowRect(rcItem);
	ScreenToClient(rcItem);
   OffSetX = rcItem.left;
   OffSetY = rcItem.top; OffSetY += 2;

   if (pNMListView->iSubItem > 0)
   {
   	CRect rect;

      m_IntervalsList.GetSubItemRect(pNMListView->iItem, pNMListView->iSubItem, LVIR_BOUNDS, rcItem);
      OffSetX += 2;
      Width   = rcItem.Width() - 6;

      m_BrowseButton.GetWindowRect(&rect);
      Width -= rect.Width();
	   m_BrowseButton.SetWindowPos(NULL, rcItem.left + OffSetX + Width, rcItem.top + OffSetY,
   		                         rect.Width(), rcItem.Height(), SWP_SHOWWINDOW);
   }
   else
   {
      m_IntervalsList.GetSubItemRect(pNMListView->iItem, pNMListView->iSubItem, LVIR_LABEL, rcItem);
      OffSetX += 2;
      Width   = rcItem.Width() - 2;
   }

   ActualValue   = m_IntervalsList.GetItemText(pNMListView->iItem, pNMListView->iSubItem);
   Curr_iSubItem = pNMListView->iSubItem;

   m_GenericEdit.SetWindowText(ActualValue);
	m_GenericEdit.SetWindowPos(NULL, rcItem.left + OffSetX, rcItem.top + OffSetY,
   		                     Width, rcItem.Height(), SWP_SHOWWINDOW);
   m_GenericEdit.SetFocus();
   m_GenericEdit.SetSel(0, -1); // Seleziona tutto il testo
	
	*pResult = 0;
}

void CLyrDisplayModelDialog::OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

   if (pLVKeyDow->wVKey == 46) // CANC
      OnBnClickedRemove();

	*pResult = 0;
}

void CLyrDisplayModelDialog::OnEnKillfocusEdit()
{
   C_LAYER_SETTINGS *pValues, *pValues1;
   CString          dummy;

   pValues = (C_LAYER_SETTINGS *) m_IntervalsList.GetItemData(Prev_iItem);
   m_GenericEdit.GetWindowText(dummy);

   if (Curr_iSubItem == 0) // si stava editando il fattore di ingrandimento
   {
      if (gsc_is_numeric(dummy) == GS_BAD)
      {
         gsui_alert(_T("Il fattore di ingrandimento deve essere un numero >= 0.")); // msg
         return;
      }
   
      // verifico che non ci sia già un fattore uguale
      if ((pValues1 = (C_LAYER_SETTINGS *) m_LyrDispModel.search_name(dummy)) && 
          pValues1 != pValues)
      {
         gsui_alert(_T("Il fattore di ingrandimento deve essere univoco.")); // msg
         return;
      }
      
      if (pValues->set_min_scale(_wtof(dummy)) == GS_BAD)
      {
         gsui_alert(_T("Fattore di ingrandimento non valido.")); // msg
         return;
      }

      m_GenericEdit.ShowWindow(SW_HIDE);
      RefreshIntervalsList();
      m_IntervalsList.SetFocus();
      // riposiziono la riga
      Prev_iItem = m_LyrDispModel.getpos_name(dummy) - 1;
      m_IntervalsList.SetItemState(Prev_iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
      m_IntervalsList.EnsureVisible(Prev_iItem, FALSE);
   }
   else // si stava editando la path del file di configurazione piani
   {
      m_GenericEdit.ShowWindow(SW_HIDE);
      m_BrowseButton.ShowWindow(SW_HIDE);
      pValues->set_path(dummy);
      // Aggiorno path in CListCtrl
	   m_IntervalsList.SetItemText(Prev_iItem, Curr_iSubItem, dummy);	
   }
}

void CLyrDisplayModelDialog::OnBnClickedBrowseFile()
{
   C_STRING filename, str;
   CString  dummy;

   m_GenericEdit.GetWindowText(dummy);

   if (dummy.GetLength() == 0)
   {
      if (get_GS_CURRENT_WRK_SESSION())
         str = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      else
         str = get_GEODIR();

      str += _T('\\');
      str += GEOSUPPORTDIR;
      str += _T('\\');
   }
   else
      str = dummy;

   if (gsc_GetFileD(_T("GEOsim - Selezionare file configurazione piani"),
                    str, _T("lyr"), 4, filename) == RTNORM)
   {
      C_LAYER_SETTINGS *pValues;

      if ((pValues = (C_LAYER_SETTINGS *) m_IntervalsList.GetItemData(Prev_iItem)))
      {
         pValues->set_path(filename.get_name());
         // Aggiorno path in CListCtrl
	      m_IntervalsList.SetItemText(Prev_iItem, Curr_iSubItem, filename.get_name());	
      }
   }
}

void CLyrDisplayModelDialog::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CMenu  Menu;
   CMenu  *SubMenu;

   Menu.LoadMenu(IDR_MENU_LYR_DISP_MODEL);
   SubMenu = Menu.GetSubMenu(0);
   SubMenu->TrackPopupMenu(0, point.x, point.y, this, NULL);
}

void CLyrDisplayModelDialog::OnBnClickedActivate()
{
   C_LAYER_DISPLAY_MODEL *pLyrDispModel = get_LAYER_DISPLAY_MODEL();
   C_STRING Buffer;

   m_LyrDispModel.copy(*pLyrDispModel);
   
   Buffer = pLyrDispModel->get_descr();
   Buffer += _T(" (");
   Buffer += pLyrDispModel->get_path();
   Buffer += _T(')');
   m_CurrLyrDispModel.SetWindowText(Buffer.get_name()); // msg
}

void CLyrDisplayModelDialog::OnBnClickedDeactivate()
{
   C_LAYER_DISPLAY_MODEL *pLyrDispModel = get_LAYER_DISPLAY_MODEL();

   pLyrDispModel->clear();
   m_CurrLyrDispModel.SetWindowText(_T("nessuno")); // msg
}


/*************************************************************************/
/*.doc gsui_lyrdispmodel                                                 */
/*+
  Comando per gestire i modelli di visualizzazione dei layer di GEOsim.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_lyrdispmodel(void)
{
   CLyrDisplayModelDialog LyrDisplayModelDialog;

   acedRetVoid();


   LyrDisplayModelDialog.DoModal();

   return GS_GOOD;  
}