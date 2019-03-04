// gs_ui_attribvalueslistdlg.cpp : implementation file
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include "gs_ui_attribvalueslistdlg.h"
#include "gs_ui_utily.h"
#include "gs_ui_sql.h"

#include "gs_utily.h"
#include "gs_init.h"
#include "gs_prjct.h"
#include "gs_sec.h"

#include "d2hMap.h" // doc to help


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAttribValuesListDlg dialog


CAttribValuesListDlg::CAttribValuesListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAttribValuesListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttribValuesListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
   
   AllPrjs = true;
	Prj     = Cls = Sub = Sec = 0;
   Type    = DEF;

   Prev_iItem    = -1;
   Curr_iSubItem = -1;
}


void CAttribValuesListDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CAttribValuesListDlg)
   DDX_Control(pDX, IDC_ATTRIBNAME, m_ComboAttribName);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GeoTree);
   DDX_Control(pDX, IDC_GEOLISP_COMBO, m_GEOLispCombo);
   DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAttribValuesListDlg, CDialog)
	//{{AFX_MSG_MAP(CAttribValuesListDlg)
	ON_BN_CLICKED(IDC_ALL_PRJS, OnAllPrjs)
	ON_CBN_EDITCHANGE(IDC_ATTRIBNAME, OnEditchangeAttribname)
	ON_BN_CLICKED(IDC_RADIO_DEF, OnRadioDef)
	ON_BN_CLICKED(IDC_RADIO_FDEF, OnRadioFdef)
	ON_BN_CLICKED(IDC_RADIO_REF, OnRadioRef)
	ON_BN_CLICKED(IDC_RADIO_TAB, OnRadioTab)
	ON_CBN_SELCHANGE(IDC_ATTRIBNAME, OnSelchangeAttribname)
	ON_BN_CLICKED(IDHELP, OnHelp)
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
   ON_CBN_DROPDOWN(IDC_GEOLISP_COMBO, OnCbnDropdownGeolispCombo)
   ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CAttribValuesListDlg::OnTcnSelchangeTab1)
	//}}AFX_MSG_MAP
   ON_NOTIFY(TVN_SELCHANGED, IDC_GEOTREECTRL, &CAttribValuesListDlg::OnTvnSelchangedGeotreectrl)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttribValuesListDlg message handlers

BOOL CAttribValuesListDlg::OnInitDialog() 
{
	CButton *pBtn = NULL;

	CDialog::OnInitDialog();

   // tutte le classi ad esclusione degli spaghetti che non hanno DB
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_POLYLINE);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_TEXT);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_NODE);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_SIMPLEX, TYPE_SURFACE);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_EXTERN, 0);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_POLYLINE);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_TEXT);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_NODE);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_SUBCLASS, TYPE_SURFACE);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GROUP, TYPE_GROUP);
   m_GeoTree.FilterOnClassCategoryTypeList.values_add_tail(CAT_GRID, TYPE_GRID);
   m_GeoTree.FilterOnUpdateable = true; // solo quelle modificabili
   m_GeoTree.FilterOnSecondaryTabType.add_tail_int((int) GSInternalSecondaryTable); // solo tab. secondarie di GEOsim
   m_GeoTree.FinalObjectType = GSSecondaryTab;

   if (AllPrjs)
   {
      pBtn = NULL;
	   if ((pBtn = (CButton *) GetDlgItem(IDC_ALL_PRJS)))
      {
         pBtn->SetCheck(BST_CHECKED);
         OnAllPrjs();
      }
   }

   m_GeoTree.LoadFromDB();
   m_GeoTree.Refresh();

   if (Prj > 0)
      if (Cls > 0)
         if (Sub > 0)
            if (Sec > 0)
               m_GeoTree.SetSelectedSec(Prj, Cls, Sub, Sec);
            else
               m_GeoTree.SetSelectedSub(Prj, Cls, Sub);
         else
            m_GeoTree.SetSelectedCls(Prj, Cls);
      else
         m_GeoTree.SetSelectedPrj(Prj);

   RefreshAttribList();
   RefreshMsg();
   RefreshListTypeRadioButton();

   int OffSetX = 1, OffSetY = 21;
   m_GSValuesTab.Create(IDD_GSVALUESLIST_TAB, this);
   m_GSValuesTab.pParentDlg = this;
   m_DBValuesTab.Create(IDD_DBVALUESLIST_TAB, this);
   m_DBValuesTab.pParentDlg = this;
   m_TabCtrl.InsertItem(0, _T("Lista GEOsim"));
   m_TabCtrl.InsertItem(1, _T("Database esterno"));
   CRect TabRect;
   m_TabCtrl.GetWindowRect(&TabRect);
   ScreenToClient(TabRect);
   TabRect.MoveToXY(TabRect.left + OffSetX, TabRect.top + OffSetY);
   TabRect.bottom -= (OffSetY + 3); TabRect.right -= (OffSetX + 3);
   m_GSValuesTab.MoveWindow(TabRect, FALSE);
   m_GSValuesTab.EnableWindow(TRUE);
   m_GSValuesTab.ShowWindow(SW_SHOW);
   m_DBValuesTab.MoveWindow(TabRect, FALSE);

   RefreshValuesList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttribValuesListDlg::RefreshListTypeRadioButton(void) 
{
	CButton *pBtn = NULL;

   if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_TAB))) pBtn->SetCheck(BST_UNCHECKED);
   if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_REF))) pBtn->SetCheck(BST_UNCHECKED);
   if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_FDEF))) pBtn->SetCheck(BST_UNCHECKED);
   if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_DEF))) pBtn->SetCheck(BST_UNCHECKED);

   switch (Type)
   {
      case TAB:
         pBtn = (CButton *) GetDlgItem(IDC_RADIO_TAB); break;
      case REF:
         pBtn = (CButton *) GetDlgItem(IDC_RADIO_REF); break;
      case FDF:
         pBtn = (CButton *) GetDlgItem(IDC_RADIO_FDEF); break;
      case DEF:
         pBtn = (CButton *) GetDlgItem(IDC_RADIO_DEF); break;
   }
	if (pBtn) pBtn->SetCheck(BST_CHECKED);
}

void CAttribValuesListDlg::OnAllPrjs() 
{
	// TODO: Add your control notification handler code here
	CButton *pBtn = NULL;

	if ((pBtn = (CButton *) GetDlgItem(IDC_ALL_PRJS)) == NULL) return;
   if (pBtn->GetCheck() == 1)
   {
      AllPrjs = true;
      m_GeoTree.EnableWindow(FALSE);
      Prj = Cls = Sub = Sec = 0;
      RefreshAttribList();
      RefreshMsg();
   }
   else
   {
      AllPrjs = false;
      m_GeoTree.EnableWindow(TRUE);
      OnChangeSelectionGeotreectrl();
   }
}


void CAttribValuesListDlg::OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   OnChangeSelectionGeotreectrl();

   *pResult = 0;
}

void CAttribValuesListDlg::OnChangeSelectionGeotreectrl() 
{
   Prj = m_GeoTree.GetSelected(GSProject);
   Cls = m_GeoTree.GetSelected(GSClass);
   Sub = m_GeoTree.GetSelected(GSSubClass);
   Sec = m_GeoTree.GetSelected(GSSecondaryTab);
      
   RefreshAttribList();
   RefreshMsg();
}


void CAttribValuesListDlg::GetAttribList(C_STR_LIST &AttribNameList)
{
   C_CLASS       *pCls;
   C_ATTRIB_LIST *pAttribList = NULL;
   C_ATTRIB      *pAttrib;
   C_STRING      KeyAttribName;
   C_STR         *pAttribName;

   AttribNameList.remove_all();
   
   if (Cls == 0) return;
   if ((pCls = gsc_find_class(Prj, Cls, Sub)) == NULL) return;
   
   if (Sec > 0)
   {
      C_SECONDARY *pSec;
 
      if ((pSec = gsc_find_sec(Prj, Cls, Sub, Sec)) == NULL) return;
      pAttribList = pSec->ptr_attrib_list();
      if ((pSec->ptr_info()) == NULL) return;
      KeyAttribName = pSec->ptr_info()->key_attrib;
   }
   else
   {
      pAttribList = pCls->ptr_attrib_list();
      if ((pCls->ptr_info()) == NULL) return;
      KeyAttribName = pCls->ptr_info()->key_attrib;
   }

   if (!pAttribList) return;

   // escludo l'attributo chiave e quelli calcolati
   pAttrib = (C_ATTRIB *) pAttribList->get_head();
   while (pAttrib)
   {
      if (KeyAttribName.comp(pAttrib->get_name()) != 0 &&
          pAttrib->is_calculated() == GS_BAD)
      {
         if ((pAttribName = new C_STR(pAttrib->get_name())) == NULL) return;
         AttribNameList.add_tail(pAttribName);
      }

      pAttrib = (C_ATTRIB *) pAttribList->get_next();
   }
}

void CAttribValuesListDlg::RefreshAttribList(void)
{
   C_STR_LIST AttribNameList;
   C_STR      *pAttribName;

   while (m_ComboAttribName.DeleteString(0) != CB_ERR); // svuoto la combo

   GetAttribList(AttribNameList);

   // escludo l'attributo chiave e quelli calcolati
   pAttribName = (C_STR *) AttribNameList.get_head();
   while (pAttribName)
   {
      m_ComboAttribName.AddString(pAttribName->get_name());

      pAttribName = (C_STR *) AttribNameList.get_next();
   }
   
   m_ComboAttribName.SetWindowText((AttribName.get_name()) ? AttribName.get_name() : _T(""));
}

void CAttribValuesListDlg::RefreshMsg(void)
{
   C_PROJECT   *pPrj;
   C_CLASS     *pCls, *pSub;
   C_SECONDARY *pSec;
	CWnd        *pWnd;
   C_STRING    Msg;

	if ((pWnd = GetDlgItem(IDC_MSG)) == NULL) return;

   Msg = _T("La lista di scelta sarà applicata per l'attributo <");
   Msg += AttribName;
   Msg += _T("> ");
   if (Prj == 0)
      Msg += _T("di tutte le classi di tutti i progetti.");
   else
   {
      if (!(pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj))) return;

      if (Cls == 0)
         Msg += _T("di tutte le classi ");
      else
      {
         if (!(pCls = pPrj->find_class(Cls))) return;

         if (pCls->ptr_id()->category == CAT_EXTERN)
         {
            if (Sub == 0)
               Msg += _T("di tutte le sottoclassi ");
            else
            {
               if (!(pSub = pPrj->find_class(Cls, Sub))) return;

               if (Sec > 0)
               {
                  if (!(pSec = (C_SECONDARY *) pSub->find_sec(Sec))) return;
                  Msg += _T("della tabella secondaria <");
                  Msg += pSec->name;
                  Msg += _T("> ");
               }

               Msg += _T("della sottoclasse <");
               Msg += pSub->get_name();
               Msg += _T("> ");
            }
         }
         else
            if (Sec > 0)
            {
               if (!(pSec = (C_SECONDARY *) pCls->find_sec(Sec))) return;
               Msg += _T("della tabella secondaria <");
               Msg += pSec->name;
               Msg += _T("> ");
            }

         Msg += _T("della classe <");
         Msg += pCls->get_name();
         Msg += _T("> ");
      }

      Msg += _T("del progetto <");
      Msg += pPrj->get_name();
      Msg += _T(">.");
   }

   pWnd->SetWindowText(Msg.get_name());
}


void CAttribValuesListDlg::RefreshValuesList(void)
{
   switch (m_TabCtrl.GetCurSel())
   {
      case 0: // Lista di GEOsim
         m_GSValuesTab.RefreshValuesList();
         break;
      case 1: // Database esterno
         m_DBValuesTab.RefreshValues();

         break;
   }

}

void CAttribValuesListDlg::OnEditchangeAttribname() 
{
   CString _AttribName;

   m_ComboAttribName.GetWindowText(_AttribName);
   AttribName = LPCTSTR(_AttribName);
   AttribName.alltrim();
   RefreshMsg();
}

void CAttribValuesListDlg::OnSelchangeAttribname() 
{
   C_STR_LIST AttribNameList;
   C_STR      *pAttribName;
   CString    dummy;

   m_ComboAttribName.GetLBText(m_ComboAttribName.GetCurSel(), dummy);
   GetAttribList(AttribNameList);
   if ((pAttribName = (C_STR *) AttribNameList.search_name(dummy, GS_BAD)))
      AttribName = pAttribName->get_name();

   AttribName.alltrim();
   RefreshMsg();
}

void CAttribValuesListDlg::OnRadioDef() 
{
   Type = DEF;
   RefreshValuesList();
}

void CAttribValuesListDlg::OnRadioFdef() 
{
   Type = FDF;
   RefreshValuesList();
}

void CAttribValuesListDlg::OnRadioRef() 
{
   Type = REF;
   RefreshValuesList();
}

void CAttribValuesListDlg::OnRadioTab() 
{
   Type = TAB;
   RefreshValuesList();
}

int CAttribValuesListDlg::getFile(C_STRING &Path)
{
   Path.clear();
   if (AttribName.len() == 0)
   {
      gsui_alert(_T("Nome attributo mancante."), m_hWnd);
      return GS_BAD;
   }

   if (!AllPrjs && Prj == 0)
   {
      gsui_alert(_T("Riferimento al progetto mancante."), m_hWnd);
      return GS_BAD;
   }

   Path = get_GEODIR();
   Path += _T("\\");
   Path += GEOSUPPORTDIR;
   Path += _T("\\");
   Path += AttribName;

   if (!AllPrjs)
   {
      Path += _T("_PRJ");
      Path += Prj;

      if (Cls != 0)
      {
         Path += _T("_CLS");
         Path += Cls;

         if (Sub != 0)
         {
            Path += _T("_SUB");
            Path += Sub;
         }
         
         if (Sec != 0)
         {
            Path += _T("_SEC");
            Path += Sec;
         }
      }
   }

   switch (Type)
   {
      case TAB:
         Path += _T(".TAB"); break;
      case REF:
         Path += _T(".REF"); break;
      case FDF:
         Path += _T(".FDF"); break;
      case DEF:
         Path += _T(".DEF"); break;
      default:
         gsui_alert(_T("Riferimento al tipo di lista mancante."), m_hWnd);
         return GS_BAD;
   }

   return GS_GOOD;
}

void CAttribValuesListDlg::OnOK() 
{
	CDialog::OnOK();
}

void CAttribValuesListDlg::OnHelp() 
{
   gsc_help(IDH_Modalitguidata);
}

void CAttribValuesListDlg::OnBnClickedOk()
{
   OnOK();
}
void CAttribValuesListDlg::OnCbnDropdownGeolispCombo()
{
   C_STRING   Path;
   C_STR_LIST CondList;
   C_STR      *pCond;
   FILE       *f;
   bool       Unicode = false;
   ValuesListTypeEnum ValuesListType;

   while (m_GEOLispCombo.DeleteString(0) != CB_ERR); // svuoto la combo

   if (gsc_FindSupportFiles(AttribName.get_name(), 
                            (AllPrjs) ? 0 : Prj, Cls, Sub, Sec,
                            Path, &ValuesListType) == GS_BAD)
      return;
   if ((f = gsc_open_profile(Path, READONLY, MORETESTS, &Unicode)) == NULL) return;
   if (gsc_get_profile(f, CondList, Unicode) == GS_BAD) { gsc_close_profile(f); return; }
   gsc_close_profile(f);

   pCond = (C_STR *) CondList.get_head();
   while (pCond)
   {
      m_GEOLispCombo.AddString(pCond->get_name());

      pCond = (C_STR *) CondList.get_next();
   }
}


void CAttribValuesListDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
   switch (m_TabCtrl.GetCurSel())
   {
      case 0: // Lista di GEOsim
	      m_DBValuesTab.EnableWindow(FALSE);
	      m_DBValuesTab.ShowWindow(SW_HIDE);
	      m_GSValuesTab.EnableWindow(TRUE);
	      m_GSValuesTab.ShowWindow(SW_SHOW);
         break;
      case 1: // Database esterno
	      m_GSValuesTab.EnableWindow(FALSE);
	      m_GSValuesTab.ShowWindow(SW_HIDE);
	      m_DBValuesTab.EnableWindow(TRUE);
	      m_DBValuesTab.ShowWindow(SW_SHOW);
         break;
   }
   *pResult = 0;
}


/*************************************************************************/
/*.doc gsui_AttribValuesList                                             */
/*+
  Comando per definire le liste di scelta per attributi.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_AttribValuesList(void)
{
   CAttribValuesListDlg AttribValuesListDlg;

   acedRetVoid();

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   AttribValuesListDlg.DoModal();

   return GS_GOOD;  
}


/**************************************************************/
/*.doc gsui_getValuesListFilePath
/*+
   Funzione che il file di supporto contenente una lista di valori
   per un dato attributo di una classe o sottoclasse o secondaria.
   L'ordine di ricerca è TAB, REF, FDF, DEF prima nel direttorio del progetto
   e succesivamente nel direttorio server.
   Parametri:
   C_STRING &PrjPath;      direttorio del progetto (che potrebbe essere già stato cancellato)
   int prj;
   int cls;
   int sub;                può essere = 0
   int sec;                può essere = 0
   C_STRING &AttribName;   
   C_STRING &FilePath;     (out)
   int      ExactMode;     Flag di ricerca. Se = GS_GOOD la ricerca avviene
                           cercando rigidamente il file utilizzabile esclusivamente
                           da una classe o sottoclasse o secondaria.
                           Se = GS_BAD allora vengono considerati anche i file generici
                           utilizzabili anche per altre strutture di GEOsim
                           (es. <attrib>_PRJ1.TAB per tutte le classi del progetto 1)
                           (default = GS_BAD).

  Ritorna GS_GOOD in caso di successo altrimenti GS_BAD
-*/                                             
/**************************************************************/
int gsui_ValuesListPath_exist(C_STRING &PrjPath, C_STRING &ServerPath, C_STRING &FileName,
                             C_STRING &FoundFilePath)
{
   static TCHAR *Extensions[] = {_T(".TAB"), _T(".REF"), _T(".FDF"), _T(".DEF")};

   for (int i = 0; i < ELEMENTS(Extensions); i++)
   {
      FoundFilePath = PrjPath;
      FoundFilePath += FileName;
      FoundFilePath += Extensions[i];
      if (gsc_path_exist(FoundFilePath, GS_BAD) == GS_GOOD) return GS_GOOD;
      FoundFilePath = ServerPath;
      FoundFilePath += FileName;
      FoundFilePath += Extensions[i];
      if (gsc_path_exist(FoundFilePath, GS_BAD) == GS_GOOD) return GS_GOOD;
   }

   return GS_BAD;
}
int gsui_getValuesListFilePath(C_STRING &PrjPath, int prj, int cls, int sub, int sec, C_STRING &AttribName,
                               C_STRING &FilePath, int ExactMode)
{
   C_STRING  SupportPrjPath, SupportServerPath, FileName;
   int       Result;
   
   SupportPrjPath = SupportPrjPath;
   SupportPrjPath += _T("\\");
   SupportPrjPath += GEOSUPPORTDIR;
   SupportPrjPath += _T("\\");

   SupportServerPath = get_GEODIR();
   SupportServerPath += _T("\\");
   SupportServerPath += GEOSUPPORTDIR;
   SupportServerPath += _T("\\");

   FileName = AttribName;
   FileName += _T("_PRJ");
   FileName += prj; 
   FileName += _T("_CLS"); 
   FileName+= cls; 
   if (sub > 0)
   {
      FileName += _T("_SUB"); 
      FileName += sub; 
   }
   if (sec > 0)
   {
      FileName += _T("_SEC");
      FileName += sec; 
   }
   Result = gsui_ValuesListPath_exist(SupportPrjPath, SupportServerPath, FileName, FilePath);

   if (ExactMode == GS_GOOD) return Result;

   if (sec > 0)
   {
      // tutte le tabelle seondarie della sottoclasse sub
      FileName = AttribName;
      FileName += _T("_PRJ");
      FileName += prj; 
      FileName += _T("_CLS"); 
      FileName+= cls; 
      if (sub > 0)
      {
         FileName += _T("_SUB"); 
         FileName += sub; 
      }
      if (gsui_ValuesListPath_exist(SupportPrjPath, SupportServerPath, FileName, FilePath) == GS_GOOD)
         return GS_GOOD;
   }

   if (sub > 0)
   {
      // tutte le sottoclassi della classe cls
      FileName = AttribName;
      FileName += _T("_PRJ");
      FileName += prj; 
      FileName += _T("_CLS"); 
      FileName+= cls; 
      if (gsui_ValuesListPath_exist(SupportPrjPath, SupportServerPath, FileName, FilePath) == GS_GOOD)
         return GS_GOOD;
   }

   // tutte le classi del progetto prj
   FileName = AttribName;
   FileName += _T("_PRJ");
   FileName += prj; 
   if (gsui_ValuesListPath_exist(SupportPrjPath, SupportServerPath, FileName, FilePath) == GS_GOOD)
      return GS_GOOD;

   // tutte le classi di tutti i progetti
   FileName = AttribName;
   if (gsui_ValuesListPath_exist(SupportPrjPath, SupportServerPath, FileName, FilePath) == GS_GOOD)
      return GS_GOOD;

   return GS_BAD;
}


///////////////////////////////////////////////////////////////////////////////
// Tab per la lista di scelta GEOsim  -  INIZIO
// CAttribGSValuesListTabDlg dialog
///////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC(CAttribGSValuesListTabDlg, CDialog)

CAttribGSValuesListTabDlg::CAttribGSValuesListTabDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAttribGSValuesListTabDlg::IDD, pParent)
{
   Prev_iItem    = -1;
   Curr_iSubItem = -1;
}

CAttribGSValuesListTabDlg::~CAttribGSValuesListTabDlg()
{
}

BOOL CAttribGSValuesListTabDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ValuesListCtrl.SetExtendedStyle(m_ValuesListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
   m_GenericEdit.ModifyStyle(0L, WS_BORDER); // con bordino

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttribGSValuesListTabDlg::OnPaint()
{
   CPaintDC dc(this); // device context for painting
   // TODO: Add your message handler code here
   // Do not call CDialog::OnPaint() for painting messages

   // Questo redraw serve perchè non fa il refresh video della lista dei valori
   // nel caso in cui la finestra venga sovrapposta con un'altra
   m_ValuesListCtrl.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
}

void CAttribGSValuesListTabDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_VALUES_LIST, m_ValuesListCtrl);
   DDX_Control(pDX, IDC_GENERIC_EDIT, m_GenericEdit);
}


BEGIN_MESSAGE_MAP(CAttribGSValuesListTabDlg, CDialog)
   ON_BN_CLICKED(IDOK, &CAttribGSValuesListTabDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDCANCEL, &CAttribGSValuesListTabDlg::OnBnClickedCancel)
   ON_BN_CLICKED(IDC_ADD, &CAttribGSValuesListTabDlg::OnBnClickedAdd)
   ON_BN_CLICKED(IDC_REMOVE, &CAttribGSValuesListTabDlg::OnBnClickedRemove)
   ON_BN_CLICKED(IDC_REMOVE_ALL, &CAttribGSValuesListTabDlg::OnBnClickedRemoveAll)
   ON_BN_CLICKED(IDC_SORT, &CAttribGSValuesListTabDlg::OnBnClickedSort)
   ON_BN_CLICKED(IDC_LOAD, &CAttribGSValuesListTabDlg::OnBnClickedLoad)
   ON_BN_CLICKED(IDC_SAVE, &CAttribGSValuesListTabDlg::OnBnClickedSave)
   ON_NOTIFY(NM_CLICK, IDC_VALUES_LIST, &CAttribGSValuesListTabDlg::OnNMClickValuesList)
   ON_EN_KILLFOCUS(IDC_GENERIC_EDIT, &CAttribGSValuesListTabDlg::OnEnKillfocusGenericEdit)
   ON_WM_PAINT()
END_MESSAGE_MAP()


// CAttribGSValuesListTabDlg message handlers

void CAttribGSValuesListTabDlg::OnBnClickedOk() {}
void CAttribGSValuesListTabDlg::OnBnClickedCancel() {}

void CAttribGSValuesListTabDlg::RefreshValuesList(void)
{
	int     iRow = 0, WidthLastColumn, VirtualCtrlWidth;
	LV_ITEM lvitem;
	CRect   rect;
   C_2STR  *pValues;

   m_ValuesListCtrl.DeleteAllItems();
   while (m_ValuesListCtrl.DeleteColumn(0) != 0); // svuoto la lista

	m_ValuesListCtrl.GetWindowRect(&rect);
   VirtualCtrlWidth = rect.Width() - 4;

   switch (pParentDlg->Type)
   {
      case TAB: case REF:
	      m_ValuesListCtrl.InsertColumn(0, _T("Valori"), LVCFMT_LEFT,
   		                              VirtualCtrlWidth * 8/20, 0);
         WidthLastColumn = VirtualCtrlWidth - (VirtualCtrlWidth * 8/20);
	      m_ValuesListCtrl.InsertColumn(1, _T("Note"), LVCFMT_LEFT,
		                                 WidthLastColumn, 0);

         pValues = (C_2STR *) ValuesList.get_head();
         while (pValues)
         {  // Valore
            lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
		      lvitem.iItem    = iRow++;
            lvitem.iSubItem = 0;
            lvitem.pszText  = pValues->get_name();
            lvitem.lParam   = (LPARAM) pValues;
         	m_ValuesListCtrl.InsertItem(&lvitem);

            // Note
            lvitem.mask     = LVIF_TEXT;
            lvitem.iSubItem++;
            lvitem.pszText  = pValues->get_name2();
   	      m_ValuesListCtrl.SetItem(&lvitem);

            pValues = (C_2STR *) ValuesList.get_next();
         }
         if (m_ValuesListCtrl.GetItemCount() > m_ValuesListCtrl.GetCountPerPage())
            // la scrollbar verticale è visibile
            WidthLastColumn -= 16;
         m_ValuesListCtrl.SetColumnWidth(1, WidthLastColumn);
         break;

      case FDF: case DEF:
         WidthLastColumn = VirtualCtrlWidth;
	      m_ValuesListCtrl.InsertColumn(0, _T("Valori"), LVCFMT_LEFT,
		                                 WidthLastColumn, 0);

         lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
         lvitem.iSubItem = 0;
         pValues = (C_2STR *) ValuesList.get_head();
         while (pValues)
         {  // Valore
		      lvitem.iItem   = iRow++;
            lvitem.pszText = pValues->get_name();
            lvitem.lParam  = (LPARAM) pValues;
         	m_ValuesListCtrl.InsertItem(&lvitem);

            pValues = (C_2STR *) ValuesList.get_next();
         }
         if (m_ValuesListCtrl.GetItemCount() > m_ValuesListCtrl.GetCountPerPage())
            WidthLastColumn -= 16;
         m_ValuesListCtrl.SetColumnWidth(0, WidthLastColumn);

         break;
   }
}

void CAttribGSValuesListTabDlg::OnBnClickedAdd()
{
	LV_ITEM    lvitem;
   C_STRING   Alias;
   C_STR_LIST AvailableDriveList;
   int        i = 1;
   C_2STR    *pValues;

   if ((pValues = new C_2STR(_T(""), _T(""))) == NULL) return;
   if (ValuesList.add_tail(pValues) == GS_BAD) return;

   lvitem.iItem  = m_ValuesListCtrl.GetItemCount();

	// Valore
   lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
   lvitem.lParam   = (LPARAM) pValues;
   lvitem.iSubItem = 0;
   lvitem.pszText  = pValues->get_name();
   m_ValuesListCtrl.InsertItem(&lvitem);

	// Note
 	lvitem.mask     = LVIF_TEXT;
   lvitem.iSubItem++;
   lvitem.pszText  = pValues->get_name2();
   m_ValuesListCtrl.SetItem(&lvitem);

   // Lo seleziono
   m_ValuesListCtrl.SetFocus();
   m_ValuesListCtrl.SetItemState(lvitem.iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
   m_ValuesListCtrl.EnsureVisible(lvitem.iItem, FALSE);
   Prev_iItem = lvitem.iItem;

   CRect rcItem;
   long  OffSetX, OffSetY, Width;

   m_ValuesListCtrl.GetWindowRect(rcItem);
	ScreenToClient(rcItem);
   OffSetX = rcItem.left;
   OffSetY = rcItem.top;

   m_ValuesListCtrl.GetSubItemRect(lvitem.iItem, 0, LVIR_LABEL, rcItem);
   OffSetX += 3;
   Width   = rcItem.Width() - 2;
   Curr_iSubItem = 0;

   m_GenericEdit.SetWindowText(_T(""));
	m_GenericEdit.SetWindowPos(NULL, rcItem.left + OffSetX, rcItem.top + OffSetY + 2,
   		                     Width, rcItem.Height(), SWP_SHOWWINDOW);
   m_GenericEdit.SetFocus();
   m_GenericEdit.SetSel(0, -1); // Seleziona tutto il testo
}

void CAttribGSValuesListTabDlg::OnBnClickedRemove()
{
   POSITION iItem = m_ValuesListCtrl.GetFirstSelectedItemPosition();
   
   if (iItem)
   {
      C_STRING Msg(_T("Cancellare il valore \""));
      CString  Value;

      Value = m_ValuesListCtrl.GetItemText((int) iItem - 1, 0); // Valore
      Msg += Value;
      Msg += _T("\" ?");
      if (gsui_confirm(Msg.get_name(), GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
      {
         if (ValuesList.search_name(Value)) ValuesList.remove_at();
         m_ValuesListCtrl.DeleteItem((int) iItem - 1); // cancella riga corrente
      }
   }		
}

void CAttribGSValuesListTabDlg::OnBnClickedRemoveAll()
{
   if (gsui_confirm(_T("Cancellare tutto il contenuto della lista ?"),
                    GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
   {
      ValuesList.remove_all();
      RefreshValuesList();
   }
}

void CAttribGSValuesListTabDlg::OnBnClickedSort()
{
   C_CLASS       *pCls;
   C_ATTRIB_LIST *pAttribList = NULL;
   C_ATTRIB      *pAttrib = NULL;

   if ((pCls = gsc_find_class(pParentDlg->Prj, pParentDlg->Cls, pParentDlg->Sub)) &&
       pCls->ptr_attrib_list())
   {
      if (pParentDlg->Sec > 0)
      {
         C_SECONDARY *pSec;
    
         if ((pSec = gsc_find_sec(pParentDlg->Prj, pParentDlg->Cls,
                                  pParentDlg->Sub, pParentDlg->Sec)))
         {
            pAttribList = pSec->ptr_attrib_list();
            if (pAttribList->init_ADOType(pSec->ptr_info()->getDBConnection(OLD)) == GS_GOOD)
               // cerco attributo in lista
               pAttrib = (C_ATTRIB *) pAttribList->search_name(pParentDlg->AttribName.get_name());
         }
      }
      else
      {
         pAttribList = pCls->ptr_attrib_list();
         if (pAttribList->init_ADOType(pCls->ptr_info()->getDBConnection(OLD)) == GS_GOOD)
            // cerco attributo in lista
            pAttrib = (C_ATTRIB *) pAttribList->search_name(pParentDlg->AttribName.get_name());
      }
   }
   
   if (pAttrib)
   {
     // Ordino i valori come numeri
     if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD)
        ValuesList.sort_nameByNum();
     else // Ordino i valori come date o timestamp
     if (gsc_DBIsDate(pAttrib->ADOType) == GS_GOOD || gsc_DBIsTimestamp(pAttrib->ADOType) == GS_GOOD)
        ValuesList.sort_nameByDate();
     else // Ordino i valori come caratteri
        ValuesList.sort_name();
   }
   else  // Ordino i valori come caratteri
      ValuesList.sort_name();

   RefreshValuesList();
}

void CAttribGSValuesListTabDlg::OnBnClickedLoad()
{
   C_STRING           Path;
   ValuesListTypeEnum ValuesListType;
   CString            Cond;

   if (gsc_FindSupportFiles(pParentDlg->AttribName.get_name(), 
                            (pParentDlg->AllPrjs) ? 0 : pParentDlg->Prj,
                            pParentDlg->Cls, pParentDlg->Sub, pParentDlg->Sec,
                            Path, &ValuesListType) == GS_BAD)
   {
      gsui_alert(_T("Lista di valori non trovata."), m_hWnd);
      return;
   }

   pParentDlg->Type = ValuesListType;
   pParentDlg->RefreshListTypeRadioButton();

   pParentDlg->m_GEOLispCombo.GetWindowText(Cond);
   ValuesList.load(Path.get_name(), _T(';'), Cond);
   RefreshValuesList();
}

void CAttribGSValuesListTabDlg::OnBnClickedSave()
{
   C_STRING    Path;
   CString     Cond;
   C_2STR_LIST dummy;
      
   if (pParentDlg->getFile(Path) == GS_BAD) return;
   pParentDlg->m_GEOLispCombo.GetWindowText(Cond);

   if (gsc_path_exist(Path, GS_BAD) == GS_GOOD &&
       dummy.load(Path.get_name(), _T(';'), Cond) == GS_GOOD && dummy.get_count() > 0)
         if (gsui_confirm(_T("Lista di valori già esistente. Sovrascrivere la lista ?"),
                          GS_GOOD, FALSE, FALSE, m_hWnd) != GS_GOOD)
            return;

   ValuesList.save(Path.get_name(), _T(';'), Cond);
}

void CAttribGSValuesListTabDlg::OnNMClickValuesList(NMHDR *pNMHDR, LRESULT *pResult)
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

   m_ValuesListCtrl.GetWindowRect(rcItem);
	ScreenToClient(rcItem);
   OffSetX = rcItem.left;
   OffSetY = rcItem.top;

   if (pNMListView->iSubItem > 0)
   {
      m_ValuesListCtrl.GetSubItemRect(pNMListView->iItem, pNMListView->iSubItem, LVIR_BOUNDS, rcItem);
      OffSetX += 7;
      Width   = rcItem.Width() - 6;
   }
   else
   {
      m_ValuesListCtrl.GetSubItemRect(pNMListView->iItem, pNMListView->iSubItem, LVIR_LABEL, rcItem);
      OffSetX += 3;
      Width   = rcItem.Width() - 2;
   }

   ActualValue   = m_ValuesListCtrl.GetItemText(pNMListView->iItem, pNMListView->iSubItem);
   Curr_iSubItem = pNMListView->iSubItem;

   m_GenericEdit.SetWindowText(ActualValue);
	m_GenericEdit.SetWindowPos(NULL, rcItem.left + OffSetX, rcItem.top + OffSetY + 2,
   		                     Width, rcItem.Height(), SWP_SHOWWINDOW);
   m_GenericEdit.SetFocus();
   m_GenericEdit.SetSel(0, -1); // Seleziona tutto il testo
	
	*pResult = 0;
}

void CAttribGSValuesListTabDlg::OnEnKillfocusGenericEdit()
{
   m_GenericEdit.ShowWindow(SW_HIDE);	

   C_2STR *pPrevValues;
   CString dummy;

	// Aggiorno valore in memoria
   dummy = m_ValuesListCtrl.GetItemText(Prev_iItem, 0); // Valore
   if ((pPrevValues = (C_2STR *) ValuesList.search_name(dummy)) == NULL) return;
   m_GenericEdit.GetWindowText(dummy);

   if (Curr_iSubItem == 0) // Valore
      pPrevValues->set_name(dummy);
   else // Note
      pPrevValues->set_name2(dummy);

   // Aggiorno alias in CListCtrl
   C_STRING Value(dummy);
	m_ValuesListCtrl.SetItemText(Prev_iItem, Curr_iSubItem, Value.get_name());	
}


///////////////////////////////////////////////////////////////////////////////
// Tab per la lista di scelta GEOsim  -  FINE
// Tab per la lista di scelta da DB esterno  -  INIZIO
// CAttribDBValuesListTabDlg dialog
///////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC(CAttribDBValuesListTabDlg, CDialog)

CAttribDBValuesListTabDlg::CAttribDBValuesListTabDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAttribDBValuesListTabDlg::IDD, pParent)
{

}

CAttribDBValuesListTabDlg::~CAttribDBValuesListTabDlg()
{
}

BOOL CAttribDBValuesListTabDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ValuesListCtrl.SetExtendedStyle(m_ValuesListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttribDBValuesListTabDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_UDL_LBL, m_UdlNameLbl);
   DDX_Control(pDX, IDC_VALUES_LIST, m_ValuesListCtrl);
   DDX_Control(pDX, IDC_EDIT1, m_SqlEditCtrl);
}


BEGIN_MESSAGE_MAP(CAttribDBValuesListTabDlg, CDialog)
   ON_BN_CLICKED(IDOK, &CAttribDBValuesListTabDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDCANCEL, &CAttribDBValuesListTabDlg::OnBnClickedCancel)
   ON_BN_CLICKED(IDC_DBCONN_BUTTON, &CAttribDBValuesListTabDlg::OnBnClickedDbconnButton)
   ON_BN_CLICKED(IDC_SQLTEST_BUTTON, &CAttribDBValuesListTabDlg::OnBnClickedSqltestButton)
   ON_BN_CLICKED(IDC_LOAD, &CAttribDBValuesListTabDlg::OnBnClickedLoad)
   ON_BN_CLICKED(IDC_SAVE, &CAttribDBValuesListTabDlg::OnBnClickedSave)
   ON_EN_CHANGE(IDC_EDIT1, &CAttribDBValuesListTabDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CAttribDBValuesListTabDlg message handlers

void CAttribDBValuesListTabDlg::OnBnClickedOk() {}

void CAttribDBValuesListTabDlg::OnBnClickedCancel() {}

void CAttribDBValuesListTabDlg::getDBInfo(C_STRING &ConnStrUDLFile,
                                          C_STRING &UdlProperties, 
                                          C_STRING &SelectStm)
{
   C_2STR *p;

   ConnStrUDLFile.clear();
   UdlProperties.clear();
   SelectStm.clear();

   // Se le righe lette contengono il collegamento ad un DB
   // allora la lista dei valori va letta dal DB
   if ((p = (C_2STR *) ValuesList.search_name(_T("CONNSTRUDLFILE"))))
   {
      ConnStrUDLFile = p->get_name2();
      // UDL_PROPERTIES
      if ((p = (C_2STR *) ValuesList.search_name(_T("UDLPROPERTIES"))))
      {
         UdlProperties = p->get_name2();
         // SELECT_STATEMENT
         // (per compatibilità con versione passate verifico anche SELECTSTAMENT)
         if ((p = (C_2STR *) ValuesList.search_name(_T("SELECTSTATEMENT"))) ||
             (p = (C_2STR *) ValuesList.search_name(_T("SELECTSTAMENT"))))
            SelectStm = p->get_name2();
      }
   }
}

void CAttribDBValuesListTabDlg::setDBInfo(C_STRING &ConnStrUDLFile,
                                          C_STRING &UdlProperties, 
                                          C_STRING &SelectStm,
                                          C_2STR_LIST *pList)
{
   C_2STR *p;
   C_2STR_LIST *pDummy;

   if (pList) pDummy = pList;
   else pDummy = &ValuesList;

   if (!(p = (C_2STR *) pDummy->search_name(_T("CONNSTRUDLFILE"))))
   {
      p = new C_2STR(_T("CONNSTRUDLFILE"), ConnStrUDLFile.get_name());
      pDummy->add_tail(p);
   }
   else p->set_name2(ConnStrUDLFile.get_name());

   // UDL_PROPERTIES
   if (!(p = (C_2STR *) pDummy->search_name(_T("UDLPROPERTIES"))))
   {
      p = new C_2STR(_T("UDLPROPERTIES"), UdlProperties.get_name());
      pDummy->add_tail(p);
   }
   else p->set_name2(UdlProperties.get_name());

   // SELECT_STATEMENT
   if (!(p = (C_2STR *) pDummy->search_name(_T("SELECTSTATEMENT"))))
   {
      // (per compatibilità con versione passate verifico anche SELECTSTAMENT)
      if (!(p = (C_2STR *) pDummy->search_name(_T("SELECTSTAMENT"))))
      {
         p = new C_2STR(_T("SELECTSTATEMENT"), SelectStm.get_name());
         pDummy->add_tail(p);
      }
      else p->set_name2(SelectStm.get_name());
   }
   else p->set_name2(SelectStm.get_name());
}

void CAttribDBValuesListTabDlg::RefreshValues(void)
{
	int         iRow = 0, WidthLastColumn, VirtualCtrlWidth;
	LV_ITEM     lvitem;
	CRect       rect;
   C_2STR_LIST DataList;
   C_2STR      *pValues;
   C_STRING    ConnStrUDLFile, UdlProperties, SelectStm;

   // Leggo le informazioni di collegamento al DB
   getDBInfo(ConnStrUDLFile, UdlProperties, SelectStm);
   if (SelectStm.len() > 0)
      gsc_C_2STR_LIST_load(DataList, ConnStrUDLFile, UdlProperties, SelectStm, (presbuf) NULL);

   m_UdlNameLbl.SetWindowText(ConnStrUDLFile.get_name());
   m_SqlEditCtrl.SetWindowText(SelectStm.get_name());

   m_ValuesListCtrl.DeleteAllItems();
   while (m_ValuesListCtrl.DeleteColumn(0) != 0); // svuoto la lista

	m_ValuesListCtrl.GetWindowRect(&rect);
   VirtualCtrlWidth = rect.Width() - 4;

   lvitem.mask = LVIF_TEXT;

   switch (pParentDlg->Type)
   {
      case TAB: case REF:
	      m_ValuesListCtrl.InsertColumn(0, _T("Valori"), LVCFMT_LEFT,
   		                              VirtualCtrlWidth * 8/20, 0);
         WidthLastColumn = VirtualCtrlWidth - (VirtualCtrlWidth * 8/20);
	      m_ValuesListCtrl.InsertColumn(1, _T("Note"), LVCFMT_LEFT,
		                                 WidthLastColumn, 0);

         pValues = (C_2STR *) DataList.get_head();
         while (pValues)
         {  // Valore
		      lvitem.iItem    = iRow++;
            lvitem.iSubItem = 0;
            lvitem.pszText  = pValues->get_name();
         	m_ValuesListCtrl.InsertItem(&lvitem);

            // Note
            lvitem.iSubItem++;
            lvitem.pszText  = pValues->get_name2();
   	      m_ValuesListCtrl.SetItem(&lvitem);

            pValues = (C_2STR *) DataList.get_next();
         }
         if (m_ValuesListCtrl.GetItemCount() > m_ValuesListCtrl.GetCountPerPage())
            WidthLastColumn -= 16;
         m_ValuesListCtrl.SetColumnWidth(1, WidthLastColumn);
         break;

      case FDF: case DEF:
         WidthLastColumn = VirtualCtrlWidth;
	      m_ValuesListCtrl.InsertColumn(0, _T("Valori"), LVCFMT_LEFT,
		                                 WidthLastColumn, 0);

         lvitem.iSubItem = 0;
         pValues = (C_2STR *) DataList.get_head();
         while (pValues)
         {  // Valore
		      lvitem.iItem   = iRow++;
            lvitem.pszText = pValues->get_name();
         	m_ValuesListCtrl.InsertItem(&lvitem);

            pValues = (C_2STR *) DataList.get_next();
         }
         if (m_ValuesListCtrl.GetItemCount() > m_ValuesListCtrl.GetCountPerPage())
            WidthLastColumn -= 16;
         m_ValuesListCtrl.SetColumnWidth(0, WidthLastColumn);

         break;
   }
}

void CAttribDBValuesListTabDlg::OnBnClickedDbconnButton()
{
   C_STRING ConnStrUDLFile, UdlPropertiesStr, SelectStm;

   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   CDBConnDlg DBConnlg;

   // Leggo le informazioni di collegamento al DB
   getDBInfo(ConnStrUDLFile, UdlPropertiesStr, SelectStm);
   
   DBConnlg.Flags = CONN_DLG_DB_CONN_ONLY;
   DBConnlg.UdlFile = ConnStrUDLFile;
   UdlPropertiesStr.alltrim();
   if (UdlPropertiesStr.len() > 0)
      gsc_PropListFromConnStr(UdlPropertiesStr.get_name(), DBConnlg.UdlProperties);

   if (DBConnlg.DoModal() == IDOK)
   {
      ConnStrUDLFile = DBConnlg.UdlFile;
      UdlPropertiesStr.paste(gsc_PropListToConnStr(DBConnlg.UdlProperties));

      setDBInfo(ConnStrUDLFile, UdlPropertiesStr, SelectStm);
      RefreshValues();
   }
}

void CAttribDBValuesListTabDlg::OnBnClickedSqltestButton()
{
   RefreshValues();
}

void CAttribDBValuesListTabDlg::OnBnClickedLoad()
{
   C_STRING Path, ConnStrUDLFile, UdlPropertiesStr, SelectStm;
   CString  Cond;
   ValuesListTypeEnum ValuesListType;

   if (gsc_FindSupportFiles(pParentDlg->AttribName.get_name(), 
                            (pParentDlg->AllPrjs) ? 0 : pParentDlg->Prj,
                            pParentDlg->Cls, pParentDlg->Sub, pParentDlg->Sec,
                            Path, &ValuesListType) == GS_BAD)
   {
      gsui_alert(_T("Lista di valori non trovata."), m_hWnd);
      return;
   }

   pParentDlg->m_GEOLispCombo.GetWindowText(Cond);
   if (ValuesList.load(Path.get_name(), _T(';'), Cond) == GS_GOOD)
   {
      getDBInfo(ConnStrUDLFile, UdlPropertiesStr, SelectStm);

      // traduco dir da assoluto in dir relativo
      gsc_UDLProperties_nethost2drive(ConnStrUDLFile.get_name(), UdlPropertiesStr);
      setDBInfo(ConnStrUDLFile, UdlPropertiesStr, SelectStm);
   }

   RefreshValues();
}

void CAttribDBValuesListTabDlg::OnBnClickedSave()
{
   C_STRING    Path, ConnStrUDLFile, UdlPropertiesStr, SelectStm;
   CString     Cond;
   C_2STR_LIST dummy;

   // Leggo le informazioni di collegamento al DB
   getDBInfo(ConnStrUDLFile, UdlPropertiesStr, SelectStm);

   if (pParentDlg->getFile(Path) == GS_BAD) return;
   pParentDlg->m_GEOLispCombo.GetWindowText(Cond);

   if (gsc_path_exist(Path, GS_BAD) == GS_GOOD &&
       dummy.load(Path.get_name(), _T(';'), Cond) == GS_GOOD && dummy.get_count() > 0)
         if (gsui_confirm(_T("Lista di valori già esistente. Sovrascrivere la lista ?"),
                          GS_GOOD, FALSE, FALSE, m_hWnd) != GS_GOOD)
            return;

   if (SelectStm.len() > 0)
   {
      // traduco dir da relativo in dir assoluto
      if (gsc_UDLProperties_drive2nethost(ConnStrUDLFile.get_name(), UdlPropertiesStr) == GS_BAD)
      {
         gsui_alert(_T("Un percorso di rete è senza alias di rete corrispondente."), m_hWnd);
         return;
      }
      setDBInfo(ConnStrUDLFile, UdlPropertiesStr, SelectStm, &dummy);
   }
   else
      dummy.remove_all();

   dummy.save(Path.get_name(), _T(';'), Cond);
}

void CAttribDBValuesListTabDlg::OnEnChangeEdit1()
{
   // TODO:  If this is a RICHEDIT control, the control will not
   // send this notification unless you override the CDialog::OnInitDialog()
   // function and call CRichEditCtrl().SetEventMask()
   // with the ENM_CHANGE flag ORed into the mask.
   C_STRING ConnStrUDLFile, UdlPropertiesStr, SelectStm;
   CString  Stm;

   // Leggo le informazioni di collegamento al DB
   getDBInfo(ConnStrUDLFile, UdlPropertiesStr, SelectStm);
   m_SqlEditCtrl.GetWindowText(Stm);
   SelectStm = Stm.Trim();
   setDBInfo(ConnStrUDLFile, UdlPropertiesStr, SelectStm);
}
