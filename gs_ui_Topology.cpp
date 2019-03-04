// gs_ui_Topology.cpp : implementation file
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"		// simboli principali

#include "gs_ui_utily.h"

#include "gs_error.h" 
#include "gs_thm.h"
#include "gs_init.h"
#include "gs_utily.h"
#include "gs_topo.h"

#include "gs_ui_Topology.h"

#include "d2hMap.h" // doc to help

static C_LINK_SET GS_LSTOPO;     // resbuf permanente


// CTopoResistanceDlg dialog

IMPLEMENT_DYNAMIC(CTopoResistanceDlg, CDialog)

CTopoResistanceDlg::CTopoResistanceDlg(CWnd* pParent /*=NULL*/, 
                                       C_CLASS *pCls /*=NULL*/,
                                       C_2STR_INT_LIST *pCostSQLList /*=NULL*/)
	: CDialog(CTopoResistanceDlg::IDD, pParent)
{
   set_cls(pCls);
   if (pCostSQLList) pCostSQLList->copy(&m_CostSQLList);
   isChanged     = FALSE;
   Prev_iItem    = -1;
   Curr_iSubItem = -1;
}

CTopoResistanceDlg::~CTopoResistanceDlg()
{
}

int CTopoResistanceDlg::set_cls(C_CLASS *pCls)
{
   if (!pCls) { set_GS_ERR_COD(eGSInvalidArg); return GS_BAD; }
   if (m_pCls != pCls)
   {
      m_pCls = pCls;
      m_CostSQLList.remove_all();
   }

   return GS_GOOD;
}

int CTopoResistanceDlg::set_CostSQLList(C_2STR_INT_LIST &CostSQLList)
{
   // prima deve essere settato m_pCls
   if (!m_pCls) { set_GS_ERR_COD(eGSInvalidArg); return GS_BAD; }
   CostSQLList.copy(&m_CostSQLList);

   return GS_GOOD;
}

void CTopoResistanceDlg::get_CostSQLList(C_2STR_INT_LIST &CostSQLList)
{
   m_CostSQLList.copy(&CostSQLList);
}

void CTopoResistanceDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_RESISTANCE_LIST, m_ResistanceList);
   DDX_Control(pDX, IDC_COMBO1, m_Attrib_Combo);
   DDX_Control(pDX, IDC_LBL, m_LblStaticCtrl);
}


/*********************************************************/
/*.doc CTopoResistanceDlg::RefreshList        <internal> /*
/*+
  Questa funzione ridisegna la lista.
  Parametri:
-*/  
/*********************************************************/
void CTopoResistanceDlg::RefreshList(void)
{
	int        iRow = 0, ListWidth;
	LV_ITEM    lvitem;
	CRect      rect;
   C_2STR_INT *pValues;
   C_SUB      *pSub;
   CString    Msg;

   m_Attrib_Combo.ShowWindow(SW_HIDE);
	m_ResistanceList.SetExtendedStyle(m_ResistanceList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

   // svuoto la lista
   m_ResistanceList.DeleteAllItems();
   while (m_ResistanceList.DeleteColumn(0) != 0);

   // insert 5 columns (REPORT mode) and modify the new header items
	m_ResistanceList.GetWindowRect(&rect);
   ListWidth = rect.Width() - 3;

	m_ResistanceList.InsertColumn(0, _T("Sottoclasse"), LVCFMT_LEFT, // msg
		                           ListWidth * 14/100, 0);
	m_ResistanceList.InsertColumn(1, _T("Resistenza diretta"), LVCFMT_LEFT, // msg
		                           ListWidth * 15/100, 1);
	m_ResistanceList.InsertColumn(2, _T("Condizione di passaggio diretto"), LVCFMT_LEFT, // msg
		                           ListWidth * 28/100, 2);
	m_ResistanceList.InsertColumn(3, _T("Resistenza inversa"), LVCFMT_LEFT, // msg
		                           ListWidth * 15/100, 3);
	m_ResistanceList.InsertColumn(4, _T("Condizione di passaggio inverso"), LVCFMT_LEFT, // msg
		                           ListWidth * 28/100, 4);

   if (!m_pCls) return;

   pSub = (C_SUB *) m_pCls->ptr_sub_list()->get_head();
   while (pSub)
   {
		lvitem.iItem = iRow++;

      // nome sottoclasse
      lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
      lvitem.iSubItem = 0;
      lvitem.pszText  = pSub->get_name();
      m_ResistanceList.InsertItem(&lvitem);

      if ((pValues = (C_2STR_INT *) m_CostSQLList.search_type(pSub->get_key())) != NULL)
      {
         // Resistenza diretta
         lvitem.mask    = LVIF_TEXT;
         lvitem.iSubItem++;
         lvitem.pszText = pValues->get_name();
	      m_ResistanceList.SetItem(&lvitem);

         // Condizione di passaggio diretto
         lvitem.mask    = LVIF_TEXT;
         lvitem.iSubItem++;
         lvitem.pszText = pValues->get_name2();
	      m_ResistanceList.SetItem(&lvitem);

         // se sottoclasse lineare esiste anche la direzione inversa nell'elemento successivo
         if (pSub->ptr_id()->type == TYPE_POLYLINE)
         {
            if ((pValues = (C_2STR_INT *) m_CostSQLList.get_next()) != NULL)
            {
               // Resistenza inversa
               lvitem.mask    = LVIF_TEXT;
               lvitem.iSubItem++;
               lvitem.pszText = pValues->get_name();
	            m_ResistanceList.SetItem(&lvitem);

               // Condizione di passaggio inverso
               lvitem.mask    = LVIF_TEXT;
               lvitem.iSubItem++;
               lvitem.pszText = pValues->get_name2();
	            m_ResistanceList.SetItem(&lvitem);
            }
         }
         else if (pSub->ptr_id()->type == TYPE_NODE)
         {
            // Resistenza inversa
            lvitem.mask    = LVIF_TEXT;
            lvitem.iSubItem++;
            lvitem.pszText = pValues->get_name();
            m_ResistanceList.SetItem(&lvitem);

            // Condizione di passaggio inverso
            lvitem.mask    = LVIF_TEXT;
            lvitem.iSubItem++;
            lvitem.pszText = pValues->get_name2();
            m_ResistanceList.SetItem(&lvitem);
         }
      }

      pSub = (C_SUB *) pSub->get_next();
   }
   
   if (m_ResistanceList.GetItemCount() > m_ResistanceList.GetCountPerPage())
      m_ResistanceList.SetColumnWidth(2, m_ResistanceList.GetColumnWidth(2) - 16);

   Msg = "Specificare i valori da utilizzare per determinare la resistenza al passaggio diretto e inverso ";
   Msg += "per le entità lineari e puntuali (per le quali esse coincidono) della classe ";
   Msg += m_pCls->get_name();
   Msg += ". Opzionalmente specificare anche le condizioni SQL che ne permettano il passaggio.";
   m_LblStaticCtrl.SetWindowText(Msg); // msg
}


BEGIN_MESSAGE_MAP(CTopoResistanceDlg, CDialog)
   ON_BN_CLICKED(IDOK, &CTopoResistanceDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDCANCEL, &CTopoResistanceDlg::OnBnClickedCancel)
   ON_NOTIFY(NM_CLICK, IDC_RESISTANCE_LIST, &CTopoResistanceDlg::OnNMClickResistanceList)
   ON_CBN_SELCHANGE(IDC_COMBO1, &CTopoResistanceDlg::OnCbnSelchangeCombo1)
   ON_CBN_KILLFOCUS(IDC_COMBO1, &CTopoResistanceDlg::OnCbnKillfocusCombo1)
   ON_BN_CLICKED(IDC_LOAD_BUTTON, &CTopoResistanceDlg::OnBnClickedLoadButton)
   ON_BN_CLICKED(IDC_SAVE_BUTTON, &CTopoResistanceDlg::OnBnClickedSaveButton)
   ON_CBN_EDITCHANGE(IDC_COMBO1, &CTopoResistanceDlg::OnCbnEditchangeCombo1)
   ON_BN_CLICKED(IDC_BUTTON3, &CTopoResistanceDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


BOOL CTopoResistanceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
   m_Attrib_Combo.SetParent(&m_ResistanceList);
   m_Attrib_Combo.LimitText(255);

   RefreshList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// CTopoResistanceDlg message handlers

void CTopoResistanceDlg::OnBnClickedOk()
{
   if (CComboBoxToC_2STR_INT_LIST(m_CostSQLList) == GS_BAD) return;

   OnOK();
}

int CTopoResistanceDlg::CComboBoxToC_2STR_INT_LIST(C_2STR_INT_LIST &CostSQLList)
{
   CString         SubName, DirectCost, InverseCost, Cond;
   int             iRow = 0;
   C_2STR_INT      *pCostSQL;
   C_SUB           *pSub;
   C_STRING        dummy;

   CostSQLList.remove_all();
   SubName = m_ResistanceList.GetItemText(iRow, 0);
   while (SubName.GetLength() > 0)
   {
      // se è stata impostata una resistenza diretta o inversa
      DirectCost  = m_ResistanceList.GetItemText(iRow, 1);
      InverseCost = m_ResistanceList.GetItemText(iRow, 3);
      if (DirectCost.GetLength() > 0 || InverseCost.GetLength() > 0)
      {
         if ((pSub = (C_SUB *) m_pCls->ptr_sub_list()->search_name(SubName)) == NULL) return GS_BAD;

         if (DirectCost.GetLength() == 0)  DirectCost  = InverseCost;
         if (InverseCost.GetLength() == 0) InverseCost = DirectCost;

         Cond     = m_ResistanceList.GetItemText(iRow, 2);
         pCostSQL = new C_2STR_INT(DirectCost, Cond, pSub->get_key());
         CostSQLList.add_tail(pCostSQL);
         // se sottoclasse lineare esiste anche la direzione inversa nell'elemento successivo
         if (pSub->ptr_id()->type == TYPE_POLYLINE)
         {
            Cond     = m_ResistanceList.GetItemText(iRow, 4);
            pCostSQL = new C_2STR_INT(DirectCost, Cond, pSub->get_key());
            CostSQLList.add_tail(pCostSQL);
         }
      }

      SubName = m_ResistanceList.GetItemText(++iRow, 0);
   }

   return GS_GOOD;
}


void CTopoResistanceDlg::OnBnClickedCancel()
{
   OnCancel();
}

void CTopoResistanceDlg::OnNMClickResistanceList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
   CRect        rcItem;
   CString      SubName, ActualValue;

   if (pNMListView->iItem == -1) return; // nessuna selezione

   // se non era già stata selezionata questa casella
   if (Prev_iItem != pNMListView->iItem)
   {
      Prev_iItem = pNMListView->iItem;
      return;
   }
   if (pNMListView->iSubItem < 1) return; // la prima colonna non può essere modificata

   m_ResistanceList.GetWindowRect(rcItem);

   SubName = m_ResistanceList.GetItemText(pNMListView->iItem, 0);
   m_ResistanceList.GetSubItemRect(pNMListView->iItem, pNMListView->iSubItem, LVIR_BOUNDS, rcItem);

   ActualValue   = m_ResistanceList.GetItemText(pNMListView->iItem, pNMListView->iSubItem);
   Curr_iSubItem = pNMListView->iSubItem;

   C_SUB    *pSub;
   C_ATTRIB *pAttrib;
   bool     Found = false;

   pSub = (C_SUB *) m_pCls->ptr_sub_list()->search_name(SubName);

   while (m_Attrib_Combo.DeleteString(0) != CB_ERR); // svuoto la combo

   pAttrib = (C_ATTRIB *) pSub->ptr_attrib_list()->get_head();
   while (pAttrib)
   {
      if (ActualValue.CompareNoCase(pAttrib->get_name()) == 0)
         Found = true;
      m_Attrib_Combo.AddString(pAttrib->get_name());
      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_Attrib_Combo.SetWindowPos(NULL, rcItem.left, rcItem.top,
		                         rcItem.Width(), rcItem.Height() - 2, SWP_SHOWWINDOW);
   m_Attrib_Combo.SetFocus();

   if (m_Attrib_Combo.GetCount() > 0 && Found)
   {
      m_Attrib_Combo.SelectString(-1, ActualValue);
      m_Attrib_Combo.ShowDropDown();
   }
   else
      m_Attrib_Combo.SetWindowText(ActualValue);

   *pResult = 0;
}

void CTopoResistanceDlg::OnCbnSelchangeCombo1()
{
   if (m_Attrib_Combo.IsWindowVisible())
      isChanged = TRUE;
}


void CTopoResistanceDlg::OnCbnEditchangeCombo1()
{
   if (m_Attrib_Combo.IsWindowVisible())
      isChanged = TRUE;
}

void CTopoResistanceDlg::OnCbnKillfocusCombo1()
{
   CString dummy, SubName;
   C_SUB   *pSub;

   m_Attrib_Combo.ShowWindow(SW_HIDE);

   if (!isChanged) return;

   m_Attrib_Combo.GetWindowText(dummy);
	m_ResistanceList.SetItemText(Prev_iItem, Curr_iSubItem, dummy);

   SubName = m_ResistanceList.GetItemText(Prev_iItem, 0);
   pSub = (C_SUB *) m_pCls->ptr_sub_list()->search_name(SubName);
   if (pSub->ptr_id()->type == TYPE_NODE) // diretta e inversa devono essere uguali
      switch (Curr_iSubItem)
      {
         case 1:
            m_ResistanceList.SetItemText(Prev_iItem, 3, dummy); break;
         case 2:
            m_ResistanceList.SetItemText(Prev_iItem, 4, dummy); break;
         case 3:
            m_ResistanceList.SetItemText(Prev_iItem, 1, dummy); break;
         case 4:
            m_ResistanceList.SetItemText(Prev_iItem, 2, dummy); break;
      }
}

void CTopoResistanceDlg::OnBnClickedLoadButton()
{
   C_STRING filename, str, GSIniFile;

   if (!m_pCls) return;

   if (gsc_getPathInfoFromINI(_T("LastUsedTopoResistanceFile"), str) == GS_BAD)
   {
      if (get_GS_CURRENT_WRK_SESSION())
         str = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      else
         str = get_GEODIR();

      str += _T('\\');
      str += GEOSUPPORTDIR;
      str += _T('\\');
   }

   if (gsc_GetFileD(_T("GEOsim - Carica il file delle regole per la visita topologica"),
                    str, _T("rst"), 4, filename) == RTNORM)
   {
      C_STRING Sez;

      Sez = _T("PRJ");
      Sez += m_pCls->get_PrjId();
      Sez += _T("CLS");
      Sez += m_pCls->get_key();
      m_CostSQLList.load(filename, _T(','), Sez.get_name());
      RefreshList();

      gsc_setPathInfoToINI(_T("LastUsedTopoResistanceFile"), filename);
   }
}

void CTopoResistanceDlg::OnBnClickedSaveButton()
{
   C_STRING        filename, Dir;
   C_2STR_INT_LIST CostSQLList;

   if (!m_pCls) return;

   if (CComboBoxToC_2STR_INT_LIST(CostSQLList) == GS_BAD) return;

   if (gsc_getPathInfoFromINI(_T("LastUsedTopoResistanceFile"), Dir) == GS_BAD)
   {
      if (get_GS_CURRENT_WRK_SESSION())
         Dir = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      else
         Dir = get_GEODIR();

      Dir += _T('\\');
      Dir += GEOSUPPORTDIR;

      if (gsc_get_tmp_filename(Dir.get_name(), _T("TopoResistance"), _T(".rst"), Dir) == GS_BAD)
         return;
   }

   if (gsc_GetFileD(_T("GEOsim - Salva il file delle regole per la visita topologica"),
                    Dir, _T("rst"), UI_SAVEFILE_FLAGS, filename) == RTNORM)
   {
      C_STRING Sez;

      Sez = _T("PRJ");
      Sez += m_pCls->get_PrjId();
      Sez += _T("CLS");
      Sez += m_pCls->get_key();
      CostSQLList.save(filename, _T(','), Sez.get_name());

      // Scrivo nel file GEOSIM.INI
      gsc_setPathInfoToINI(_T("LastUsedTopoResistanceFile"), filename);
   }
}


/*********************************************************/
// classe  CTopoResistanceDlg - fine
// funzioni generiche         - inizio
/*********************************************************/


/*********************************************************/
/*.doc gsc_PutSym_sstopo                      <external> */
/*+                                                                       
  Funzione che esporta in ambiente LISP (creandone una copia)
  il gruppo di selezione del filtro chiamdolo "sstopo".
  Parametri:
  C_LINK_SET &LinkSet;

  Restituisce RTNORM in caso di successo altrimenti restituisce RTERROR. 
-*/  
/*********************************************************/
int hy_PutSym_sstopo(C_LINK_SET &LinkSet)
{
	CString msg;
   presbuf p;

   if (LinkSet.ptr_SelSet()->length() == 0)
   {
      if ((p = acutBuildList(RTNIL, 0)) == NULL) return GS_BAD;
   }
   else
   {
      ads_name dummy;

      LinkSet.ptr_SelSet()->get_selection(dummy);
      if ((p = acutBuildList(RTPICKS, dummy, 0)) == NULL) return GS_BAD;
	   msg = "\nE' stato impostato il gruppo di selezione <sstopo>.";
      acutPrintf(msg);
   }
   
   if (acedPutSym(_T("sstopo"), p) != RTNORM)
      { acutRelRb(p); return GS_BAD; }
   acutRelRb(p);

   return GS_GOOD;
}


/*********************************************************/
/*.doc gsui_topo_get_LinkSet                  <external> */
/*+                                                                       
  Funzione che ottiene un link set delle entità trovate dopo i
  controlli topologici.
  Parametri:
  C_TOPOLOGY &Topo;  Topologia

  Ritorna GS_GOOD in caso di successo altrimenti GS_BAD
-*/  
/*********************************************************/
int gsui_topo_selNode(const TCHAR *Msg, C_SUB **pSub, long *IdNode)
{
	ads_name  ent;
	ads_point pt;
   C_CLASS   *pCls;
   bool      Found = FALSE;

   if (get_GS_CURRENT_WRK_SESSION() == NULL)
   {
      gsui_alert(_T("Nessuna sessione di lavoro corrente."));
      set_GS_ERR_COD(eGSNotCurrentSession);
      return GS_BAD;
   }

   // Selezione di un elemento di partenza
   do
   {
      if (acedEntSel(Msg, ent, pt) == RTNORM)
      {
         if ((pCls = get_GS_CURRENT_WRK_SESSION()->find_class(ent)) != NULL &&
             pCls->is_extracted() == GS_GOOD &&
             pCls->is_subclass() == GS_GOOD && pCls->get_type() == TYPE_NODE)
         { 
            C_LINK Link(ent);

            if (Link.GetKey(IdNode) == GS_BAD) return GS_BAD;
            *pSub = (C_SUB *) pCls;
            Found = TRUE;
            break;
         }
      }
      else
         break;

      acutPrintf(_T("\nEntità non valida."));
   }
   while (1);

   return (Found) ? GS_GOOD : GS_BAD;
}


/******************************************************************************/
/*.doc gsui_extractEnt                                                        */
/*+
  Questa funzione ha lo scopo di ottenere un gruppo di selezione degli 
  oggetti di un'entità GEOsim. A tale scopo, se necessario, 
  chiede o estrae direttamente (a seconda del parametro Extract) 
  tutti gli oggetti di un'entità.
  Parametri:
  C_CLASS *pSub;     Puntatore ala sottoclasse;
  long Id;           Codice chiave
  int *Extract;      Flag per estrarre direttamente (GS_GOOD)
                     per richiedere di estrarre (GS_CAN) o
                     non estrarre (GS_BAD). Questo parametro verrà 
                     variato dalla funzione
  CString &msg;      messaggiO di richiesta se *Extract = GS_CAN
  C_SELSET &SelSet;  Gruppo di selezione risultante
  
  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/******************************************************************************/
int gsui_extractEnt(C_CLASS *pSub, long Id, int *Extract, CString &msg, C_SELSET &SelSet)
{
   int WhyNot;

   if (pSub->get_SelSet(Id, SelSet) == GS_BAD) return GS_BAD;

   if (SelSet.length() == 0)
   {
      if (*Extract == GS_CAN)
         // Chiedo all'utente se vuole estrarre gli oggetti
         gsc_ddgetconfirm(_T("Estrarre tutte le entità connesse ?"), Extract);

      // Provo ad estrarre totalmente le istanze grafiche dell'entità
      if (*Extract == GS_GOOD)
      {
         // Provo ad estrarre totalmente le istanze grafiche dell'entità
         if (pSub->entExtract(Id) == GS_BAD) return GS_BAD;
         pSub->get_SelSet(Id, SelSet);
      }
   }
   else
   {
      // se entità estratta parzialmente provo ad estrarre totalmente le istanze grafiche dell'entità
      if (pSub->is_updateableSS(Id, SelSet, &WhyNot, GS_BAD, GS_BAD) == GS_BAD &&
          WhyNot == eGSPartialEntExtract)
      {
         if (*Extract == GS_CAN)
            gsc_ddgetconfirm(_T("Estrarre tutte le entità connesse ?"), Extract);

         // Provo ad estrarre totalmente le istanze grafiche dell'entità
         if (*Extract == GS_GOOD)
         {
            if (pSub->entExtract(Id) == GS_BAD) return GS_BAD;
            pSub->get_SelSet(Id, SelSet);
         }
      }
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc gsui_topo_get_LinkSet                  <external> */
/*+                                                                       
  Funzione che ottiene un link set delle entità trovate dopo i
  controlli topologici.
  Parametri:
  C_TOPOLOGY &Topo;  Topologia

  Ritorna GS_GOOD in caso di successo altrimenti GS_BAD
-*/  
/*********************************************************/
int gsui_topo_get_LinkSet(C_TOPOLOGY &Topo, C_LINK_SET &LinkSet)
{
   // estraggo tutti gli elementi della simulazione
   C_INT_LONG *pItem;
   C_SELSET   SelSet;
   int        Extract = GS_CAN;
   CString    msg;
   C_SUB      *pSub;

   msg = "Estrarre tutte le entità connesse ?";

   LinkSet.clear();
   LinkSet.cls = 0;
   LinkSet.sub = 0;

   // links
   pItem = (C_INT_LONG *) Topo.ptr_NetLinks()->get_head();
   while (pItem)
   {
      pSub = (C_SUB *) Topo.get_cls()->ptr_sub_list()->search_key(pItem->get_key());

      if (gsui_extractEnt(pSub, pItem->get_id(), &Extract, msg, SelSet) == GS_BAD)
	      return GS_BAD;

      LinkSet.ptr_SelSet()->add_selset(SelSet);
         
      pItem = (C_INT_LONG *) pItem->get_next();
   }
   // nodes
   pItem = (C_INT_LONG *) Topo.ptr_NetNodes()->get_head();
   while (pItem)
   {
      pSub = (C_SUB *) Topo.get_cls()->ptr_sub_list()->search_key(pItem->get_key());

      if (gsui_extractEnt(pSub, pItem->get_id(), &Extract, msg, SelSet) == GS_BAD)
         return GS_BAD;

      LinkSet.ptr_SelSet()->add_selset(SelSet);

      pItem = (C_INT_LONG *) pItem->get_next();
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc gsui_topo_propagation                  <external> */
/*+                                                                       
  Funzione che evidenzia la rete connessa per propagazione da un elemento noto.
-*/  
/*********************************************************/
int gsui_topo_propagation(void)
{
   CAcModuleResourceOverride myResources;
   bool      Found = FALSE;
   C_CLASS   *pCls;
   C_SUB     *pSub;
   long      startId;
   double    MaxCost = 0.0;
   C_2STR_INT_LIST CostSQLList;

   acedRetVoid();

   // Selezione di un elemento di partenza
   if (gsui_topo_selNode(_T("\nSelezionare nodo di partenza"), &pSub, &startId) == GS_BAD)
      return GS_BAD;
   // Cerco la classe madre
   pCls = get_GS_CURRENT_WRK_SESSION()->find_class(pSub->ptr_id()->code);

   ////////////////////////////////////
   // Imposto le regole di visita nel grafo
   CTopoResistanceDlg Dlg(NULL, pCls, &CostSQLList);
   if (Dlg.DoModal() != IDOK) return GS_GOOD;
   Dlg.get_CostSQLList(CostSQLList);

   if (CostSQLList.get_count() > 0) // solo se è stabilito come calcolare il costo
      // Chiedo il costo massimo
      do
      {
         C_STRING MaxCostStr;

         if (gsc_ddinput(_T("Costo massimo: "), MaxCostStr, NULL, FALSE) != GS_GOOD)
            return GS_GOOD;
         MaxCostStr.alltrim();
         if (MaxCostStr.isDigit() == GS_GOOD)
         {
            MaxCost = MaxCostStr.tof();
            if (MaxCost >= 0) break;
         }
         gsc_ddalert(_T("Costo non valido."));
      }
      while (1);

   ////////////////////////////////////
   // visito il grafo
   C_TOPOLOGY Topo;
   Topo.set_type(TYPE_POLYLINE); // tipologia di tipo rete
   Topo.set_cls(pCls);
   if (Topo.LoadInMemory(&CostSQLList) == GS_BAD) return GS_BAD;
   if (MaxCost > 0) Topo.NetCost = MaxCost; // massimo costo

   long iNode = gsc_searchTopoNetNode(Topo.NodesVett, Topo.nNodesVett, startId);
   Topo.GetNetPropagation(iNode);

   ////////////////////////////////////

   // estraggo tutti gli elementi della simulazione
   if (gsui_topo_get_LinkSet(Topo, GS_LSTOPO) == GS_BAD)
      return GS_BAD;

   // Libero un pò di memoria
   gsc_freeTopoNetNode(&(Topo.NodesVett), &(Topo.nNodesVett));
   gsc_freeTopoNetLink(&(Topo.LinksVett), &(Topo.nLinksVett));

   hy_PutSym_sstopo(GS_LSTOPO);

   return GS_GOOD;
}


/*********************************************************/
/*.doc gsui_topo_shortestpath                 <external> */
/*+                                                                       
  Funzione che evidenzia la rete connessa per propagazione da un elemento noto.
-*/  
/*********************************************************/
int gsui_topo_shortestpath(void)
{
   CAcModuleResourceOverride myResources;
   bool      Found = FALSE;
   C_CLASS   *pCls;
   C_SUB     *pSub;
   long      startId, FinalId;
   C_2STR_INT_LIST CostSQLList;

   acedRetVoid();

   // Selezione di un elemento di partenza
   if (gsui_topo_selNode(_T("\nSelezionare nodo di partenza"), &pSub, &startId) == GS_BAD)
      return GS_BAD;
   // Cerco la classe madre
   pCls = get_GS_CURRENT_WRK_SESSION()->find_class(pSub->ptr_id()->code);
   
   do
   {
      // Selezione di un elemento di arrivo
      if (gsui_topo_selNode(_T("\nSelezionare nodo di arrivo"), &pSub, &FinalId) == GS_BAD)
         return GS_BAD;
      // gli elementi di partenza e arrivo devono appartenere alla stessa classe madre
      if (pCls == get_GS_CURRENT_WRK_SESSION()->find_class(pSub->ptr_id()->code))
         break;
      acutPrintf(_T("\nEntità non valida."));
   }
   // gli elementi di partenza e arrivo devono appartenere alla stessa classe madre
   while (1);

   ////////////////////////////////////
   // Imposto le regole di visita nel grafo
   CTopoResistanceDlg Dlg(NULL, pCls, &CostSQLList);
   if (Dlg.DoModal() != IDOK) return GS_GOOD;
   Dlg.get_CostSQLList(CostSQLList);

   ////////////////////////////////////
   // visito il grafo
   C_TOPOLOGY Topo;
   Topo.set_type(TYPE_POLYLINE); // tipologia di tipo rete
   Topo.set_cls(pCls);
   if (Topo.LoadInMemory(&CostSQLList) == GS_BAD) return GS_BAD;

   long iNode = gsc_searchTopoNetNode(Topo.NodesVett, Topo.nNodesVett, startId);
   Topo.NetFinalNode = FinalId;
   Topo.GetShortestNetPath(iNode, 0.0);

   ////////////////////////////////////

   // estraggo tutti gli elementi della simulazione
   if (gsui_topo_get_LinkSet(Topo, GS_LSTOPO) == GS_BAD)
      return GS_BAD;

   // Libero un pò di memoria
   gsc_freeTopoNetNode(&(Topo.NodesVett), &(Topo.nNodesVett));
   gsc_freeTopoNetLink(&(Topo.LinksVett), &(Topo.nLinksVett));

   hy_PutSym_sstopo(GS_LSTOPO);

   acutPrintf(_T("\nIl costo del percorso più breve è %f"), Topo.NetCost);

   return GS_GOOD;
}

void CTopoResistanceDlg::OnBnClickedButton3()
{
   gsc_help(IDH_Regoleperlevisitetopologiche);
}
