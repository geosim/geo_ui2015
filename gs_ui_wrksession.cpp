// gs_ui_WrkSession.cpp : implementation file
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include <rxmfcapi.h>         // ACAD MFC stuff

#include "gs_ui_utily.h"
#include "gs_ui_WrkSession.h"
#include <adeads.h>

#include "gs_error.h"
#include "gs_utily.h"
#include "gs_area.h"
#include "gs_ade.h"
#include "gs_cmd.h"
#include "gs_thm.h"

#include "GEOListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// A custom message to work around a problem getting the miniframe pointer
#define WM_DOCKSTATE_CHANGED (WM_USER + 17)

// {DC2C3BF3-46BB-4dc4-91AA-932063335066}
CLSID CLSID_DockPaneWrkSession = 
   { 0xdc2c3bf3, 0x46bb, 0x4dc4, { 0x91, 0xaa, 0x93, 0x20, 0x63, 0x33, 0x50, 0x66 } };


CDockPaneWrkSession *pDockPaneWrkSession = NULL;
// Input context reactor
GS_UI_InputContextReactor GS_UI_INCONTXTREACTOR;


//---------------------------------------------------------------------------
// Funzioni per salvare e caricare le zone di estrazione
int gsui_WriteExtrWindowList(int cls, C_RECT_LIST &WindowList);
int gsui_getExtrWindowList(int cls, C_RECT &Window, C_RECT_LIST &WindowList);

//---------------------------------------------------------------------------
// Funzioni per il drag and drop
BOOL acedStartOverrideDropTarget(COleDropTarget* pTarget);
BOOL acedEndOverrideDropTarget(COleDropTarget* pTarget);
BOOL acedAddDropTarget(COleDropTarget* pTarget);
BOOL acedRemoveDropTarget(COleDropTarget* pTarget);

COleDataSource* gsui_generateDataSource(C_INT_LIST &ClsCodeList)
{
    COleDataSource *pSource = new COleDataSource();
    TCHAR          *buf = NULL;

    if ((buf = ClsCodeList.to_str(_T(';'))) == NULL) return NULL;

    HGLOBAL temp = GlobalAlloc(GHND, (wcslen(buf) + 1) * sizeof(TCHAR));
    TCHAR *data = (TCHAR *) GlobalLock(temp);
    wcscpy(data, buf);
    GlobalUnlock(temp);
    pSource->CacheGlobalData(CF_UNICODETEXT, temp);
    free(buf);
    
    return pSource;                               
}

BOOL PasteFromData(COleDataObject* pDataObject, int *Mode, int *SpatialCond, C_INT_LIST &CodeClsList)
{
   STGMEDIUM medium;

   if (!SUCCEEDED(pDataObject->GetData(CF_UNICODETEXT, &medium))) return FALSE;
    
   // leggo la lista delle classi da estrarre con la particolarità che
   // il primo elemento rappresenta Mode (PREVIEW, EXTRACTION)
   // il secondo elemento rappresenta SpatialCond (0 = query corrente, 1 = zoom corrente)
   TCHAR *pstr = (TCHAR *) GlobalLock (medium.hGlobal);
   if (CodeClsList.from_str(pstr, _T(';')) == GS_BAD ||
       CodeClsList.get_count() < 3)
      { GlobalUnlock(medium.hGlobal); ReleaseStgMedium(&medium); return FALSE; }
   
   *Mode = CodeClsList.get_head()->get_key();
   CodeClsList.remove_at();
   
   *SpatialCond = CodeClsList.get_head()->get_key();
   CodeClsList.remove_at();
   
   if (CodeClsList.get_count() == 0)
      { GlobalUnlock(medium.hGlobal); ReleaseStgMedium(&medium); return FALSE; }

   GlobalUnlock(medium.hGlobal);
   ReleaseStgMedium(&medium);

   return TRUE;
}

//---------------------------------------------------------------------------
// Classe CDropSource
//
SCODE CDropSource::QueryContinueDrag(BOOL fEsc, DWORD grfKeyState)
{
    if (fEsc) return DRAGDROP_S_CANCEL;

    if (!(grfKeyState & MK_LBUTTON)) // No longer pressed
        return DRAGDROP_S_DROP;

    return S_OK;
}

SCODE CDropSource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}



//---------------------------------------------------------------------------
// Classe CMyOverrideDropTarget
//
CMyOverrideDropTarget::CMyOverrideDropTarget()
{
   Mode        = EXTRACTION;
   SpatialCond = 1; // zoom corrente
}

CMyOverrideDropTarget::~CMyOverrideDropTarget()
{}

DROPEFFECT CMyOverrideDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point)
{
    if (!pDataObject->IsDataAvailable(CF_UNICODETEXT)) return DROPEFFECT_NONE;
    if (!::PasteFromData(pDataObject, &Mode, &SpatialCond, CodeClsList)) 
       return DROPEFFECT_NONE;

    return OnDragOver(pWnd, pDataObject, dwKeyState, point); 
}


DROPEFFECT CMyOverrideDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
		                                       DWORD dwKeyState, CPoint point)
{
   if (!pDataObject) return DROPEFFECT_NONE;

	return DROPEFFECT_COPY;
}

void CMyOverrideDropTarget::OnDragLeave(CWnd* pWnd)
{
}
 
DROPEFFECT CMyOverrideDropTarget::OnDropEx(CWnd* pWnd, COleDataObject* pDataObject,
		                                     DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
{
    return dropDefault; // calls OnDrop();
}


BOOL CMyOverrideDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
	                                DROPEFFECT dropEffect, CPoint point)
{
   if (!pDataObject) return FALSE;
   
   if (!::PasteFromData(pDataObject, &Mode, &SpatialCond, CodeClsList)) 
      return FALSE;
   
   // Eseguo estrazione delle classi
   if (pDockPaneWrkSession)
   {
      // lock DB
      AcApDocument *pDocument = acDocManager->document(AcApGetDatabase((CView*)pWnd));
      if (acDocManager->lockDocument(pDocument) != Acad::eOk) return FALSE;
   
      pDockPaneWrkSession->m_WrkSessionTab.m_WrkSessionClasses.extract(Mode, SpatialCond, CodeClsList);

      acDocManager->unlockDocument(pDocument);
   }
   // Set Focus to AutoCAD because AutoCAD doesn't update its
   // display if it's not in focus.
   acedGetAcadFrame()->SetActiveWindow();
   acedGetAcadFrame()->SetFocus();

   return TRUE;
}

/////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// CDockPaneWindowHook
//
// The purpose of this class is to hook the floating mini frame so we can
// intercept the AutoCAD custom message WM_ACAD_KEEPFOCUS
IMPLEMENT_DYNAMIC(CDockPaneWindowHook, CSubclassWnd);

LRESULT CDockPaneWindowHook::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg) {
	    case WM_ACAD_KEEPFOCUS:
            return TRUE;
        default:break;
    }
	return CSubclassWnd::WindowProc(msg, wp, lp);
}


// controllo albero personalizzato per gestire drag and drop - INIZIO

CGEOClsTreeCtrl::CGEOClsTreeCtrl() : CGEOTreeCtrl()
{}
CGEOClsTreeCtrl::~CGEOClsTreeCtrl() {}


BEGIN_MESSAGE_MAP(CGEOClsTreeCtrl, CTreeCtrl)
   ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_GEOTREECTRL_SEARCH_BY_NAME_MENU, OnSearchByNameMenu)
	ON_COMMAND(ID_GEOTREECTRL_LOAD_SEL_MENU, OnLoadSelMenu)
	ON_COMMAND(ID_GEOTREECTRL_SAVE_SEL_MENU, OnSaveSelMenu)
	ON_COMMAND(ID_GEOTREECTRL_REFRESH_MENU, OnRefreshMenu)
	ON_COMMAND(ID_GEOTREECTRL_INVERT_SEL_MENU, OnInvertSelMenu)
END_MESSAGE_MAP()


BOOL CGEOClsTreeCtrl::OnNMClickGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
   { return CGEOTreeCtrl::OnClick(pNMHDR, pResult); }
void CGEOClsTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
   { CGEOTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags); }
void CGEOClsTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
   { CGEOTreeCtrl::OnContextMenu(pWnd, point); }
void CGEOClsTreeCtrl::OnRClick(NMHDR* pNMHDR, LRESULT* pResult)
   { CGEOTreeCtrl::OnRClick(pNMHDR, pResult); }
void CGEOClsTreeCtrl::OnSearchByNameMenu(void)
   { CGEOTreeCtrl::OnSearchByNameMenu(); }
void CGEOClsTreeCtrl::OnLoadSelMenu(void)
   { CGEOTreeCtrl::OnLoadSelMenu(); ((CWrkSessionClasses *)pParent)->OnChangeSelectionGeotreectrl(); }
void CGEOClsTreeCtrl::OnSaveSelMenu(void)
   { CGEOTreeCtrl::OnSaveSelMenu(); }
void CGEOClsTreeCtrl::OnRefreshMenu(void)
   { CGEOTreeCtrl::OnRefreshMenu(); }
void CGEOClsTreeCtrl::OnInvertSelMenu(void)
   { CGEOTreeCtrl::OnInvertSelMenu(); ((CWrkSessionClasses *)pParent)->OnChangeSelectionGeotreectrl(); }


// controllo albero personalizzato per gestire drag and drop - FINE


/////////////////////////////////////////////////////////////////////////////
// CWrkSessionClasses property page

CWrkSessionClasses::CWrkSessionClasses() : CPropertyPage(CWrkSessionClasses::IDD)
{
	//{{AFX_DATA_INIT(CWrkSessionClasses)
	//}}AFX_DATA_INIT
   LastExtractedWindow.fromStr(_T("0,0,0;0,0,0"));

   hBmpTreeDisplayMode = NULL;
   hBmpListDisplayMode = NULL;

   pToolTip          = NULL;
   m_ClassesDisplayMode = List;

   m_GEOClassTreeCtrl.pParent = this;
   AlignmentControl = false;
}

CWrkSessionClasses::~CWrkSessionClasses()
{
   if (hBmpTreeDisplayMode) DeleteObject((HGDIOBJ) hBmpTreeDisplayMode);
   if (hBmpListDisplayMode) DeleteObject((HGDIOBJ) hBmpListDisplayMode);

   if (pToolTip) delete pToolTip;
}

void CWrkSessionClasses::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CWrkSessionClasses)
   DDX_Control(pDX, IDC_SELECTED_COUNT, m_SelectedCount);
   DDX_Control(pDX, IDC_ZONE, m_Zone);
   DDX_Control(pDX, IDC_PREVIEW, m_Preview);
   DDX_Control(pDX, IDC_RADIO_QUERY, m_SpatialQuery);
   DDX_Control(pDX, IDC_GEOCLASSLISTCTRL, m_GEOClassListCtrl);
   DDX_Control(pDX, IDC_GEOTREECTRL, m_GEOClassTreeCtrl);
   DDX_Control(pDX, IDC_DISPLAYMODE_BUTTON, m_DisplayModeButton);
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWrkSessionClasses, CDialog)
	//{{AFX_MSG_MAP(CWrkSessionClasses)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
   ON_BN_CLICKED(IDC_RADIO_QUERY, OnBnClickedRadioQuery)
   ON_BN_CLICKED(IDC_DISPLAYMODE_BUTTON, OnBnClickedDisplayModeButton)
   ON_NOTIFY(LVN_BEGINDRAG, IDC_GEOCLASSLISTCTRL, &CWrkSessionClasses::OnLvnBegindragGeoclasslistctrl)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_GEOCLASSLISTCTRL, &CWrkSessionClasses::OnLvnItemchangedGeoclasslistctrl)
   ON_NOTIFY(NM_CLICK, IDC_GEOTREECTRL, &CWrkSessionClasses::OnNMClickGeotreectrl)
   ON_NOTIFY(TVN_BEGINDRAG, IDC_GEOTREECTRL, &CWrkSessionClasses::OnTvnBegindragGeotreectrl)
   ON_NOTIFY(TVN_KEYDOWN, IDC_GEOTREECTRL, &CWrkSessionClasses::OnTvnKeydownGeotreectrl)
   ON_NOTIFY(NM_RCLICK, IDC_GEOTREECTRL, &CWrkSessionClasses::OnNMRClickGeotreectrl)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWrkSessionClasses message handlers

void CWrkSessionClasses::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
   CRect   rect;
   CButton *pBtn;
   int     Top, Left, RadioActWinWidth, RadioQueryWidth, XMargin = 5;
   
   // m_Preview
   m_Preview.GetWindowRect(&rect);
	ScreenToClient(rect);
   Top  = (cy - rect.Height() >= 0) ? cy - rect.Height() - 10 : 1;
   m_Preview.SetWindowPos(this, XMargin, Top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

   Left = XMargin + rect.Width() + 5;

   // IDC_RADIO_ACTUAL_WINDOW
   if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_ACTUAL_WINDOW)))
   {
      pBtn->SetWindowPos(&m_Zone, 5 + Left, Top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
      pBtn->GetWindowRect(&rect);
	   ScreenToClient(rect);
      RadioActWinWidth = rect.Width();

      // IDC_RADIO_QUERY
      if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_QUERY)))
      {
         pBtn->SetWindowPos(&m_Zone, 5 + Left + RadioActWinWidth + 5, Top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
         pBtn->GetWindowRect(&rect);
	      ScreenToClient(rect);
         RadioQueryWidth = rect.Width();
      }

      Top = (Top - 15 >= 0) ? Top - 15 : 1;
      m_Zone.SetWindowPos(this, Left, Top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

      // m_SelectedCount
      m_SelectedCount.SetWindowPos(this, XMargin, Top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
   }

   m_DisplayModeButton.GetWindowRect(rect);
	ScreenToClient(rect);
   int Height_DispModeButton = rect.Height(), Top_DispModeButton = rect.top;

   Top  = (Top - 5 >= 0) ? Top - 5 : 1;

   int OffSetX = 5, OffSetY = 5;
   int Height_GEOClassListCtrl = (Top - OffSetY) - (Top_DispModeButton + Height_DispModeButton + OffSetY);

   // m_GEOClassListCtrl
   Top  = (Top - 5 >= 0) ? Top - 5 : 1;
   m_GEOClassListCtrl.SetWindowPos(this, 
                                   XMargin, // x
                                   Top_DispModeButton + Height_DispModeButton + OffSetY, // y
                                   cx - (2 * XMargin),
                                   Height_GEOClassListCtrl,
                                   SWP_NOZORDER);

   m_GEOClassTreeCtrl.SetWindowPos(this, 
                                   XMargin, // x
                                   Top_DispModeButton + Height_DispModeButton + OffSetY, // y
                                   cx - (2 * XMargin),
                                   Height_GEOClassListCtrl,
                                   SWP_NOZORDER);

   RedrawWindow();
}

void CWrkSessionClasses::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
   m_DummyDropTarget.Revoke();
   OleUninitialize();	
}

BOOL CWrkSessionClasses::OnInitDialog() 
{
	CDialog::OnInitDialog();
	  
   // usato per drag and drop
   if (FAILED(OleInitialize(NULL)))
        TRACE("Can't initialize ole\n");

	// TODO: Add your specialized code here and/or call the base class
   m_DummyDropTarget.Register(this);

   // settaggio di class list control
   m_GEOClassListCtrl.ObjectType                  = GSClass;
   m_GEOClassListCtrl.MultiSelect                 = true;
   m_GEOClassListCtrl.SelectedLinkedClass         = true;
   m_GEOClassListCtrl.ColumnClassStatusVisibility = true;
   if (get_GS_CURRENT_WRK_SESSION())
   {
      m_GEOClassListCtrl.FilterOnPrj = get_GS_CURRENT_WRK_SESSION()->get_PrjId();
      m_GEOClassListCtrl.LoadFromDB();
      m_GEOClassListCtrl.Refresh();
   }

   // settaggio di class tree control
   m_GEOClassTreeCtrl.FilterOnCodes.remove_all();
   m_GEOClassTreeCtrl.FinalObjectType     = GSClass;
   m_GEOClassTreeCtrl.MultiSelect         = true;
   m_GEOClassTreeCtrl.SelectedLinkedClass = true;
   if (get_GS_CURRENT_WRK_SESSION())
   {
      m_GEOClassTreeCtrl.FilterOnCodes.add_tail_int(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
      m_GEOClassTreeCtrl.LoadFromDB(get_GS_CURRENT_WRK_SESSION()->get_PrjId());
      m_GEOClassTreeCtrl.Refresh();
   }
   //m_GEOClassTreeCtrl.ExpandAll();

   CButton *pBtn;
   if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_ACTUAL_WINDOW)))
      pBtn->SetCheck(BST_CHECKED);
	
   m_SelectedCount.SetWindowText(_T("0/0"));

   // determine location of the bitmap in resource fork
 	HINSTANCE Instance;
   COLORREF  crFrom;

   crFrom = RGB(255, 0, 0); // rosso
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_TREE_DISPLAY_MODE), RT_BITMAP);  
   hBmpTreeDisplayMode = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_TREE_DISPLAY_MODE));
   gsui_SetBmpColorToDlgBkColor(hBmpTreeDisplayMode, crFrom);
   hBmpListDisplayMode = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_LIST_DISPLAY_MODE));
   gsui_SetBmpColorToDlgBkColor(hBmpListDisplayMode, crFrom);

   InitTooltip();
   DisplayMode();

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// temporaneamente usato per test di drag and drop
void CWrkSessionClasses::StartDrag(void) 
{
   C_INT_LIST ClsCodeList;

   if (m_ClassesDisplayMode == List)
   {
      CGEOList_Item *pItem;

      for (int iItem = 0; iItem < m_GEOClassListCtrl.GetItemCount(); iItem++)
         if ((pItem = (CGEOList_Item *) m_GEOClassListCtrl.GetItemData(iItem)) != NULL)
            m_GEOClassListCtrl.OnItemChanged(pItem, (m_GEOClassListCtrl.GetItemState(iItem, LVIS_SELECTED) & LVIS_SELECTED) ? true : false);

      RefreshSelectedCount();
      m_GEOClassListCtrl.GetSelectedCodes(ClsCodeList);
   }
   else
   {
      m_GEOClassTreeCtrl.GetSelectedCodes(ClsCodeList);
      ClsCodeList.Inverse(); // l'ultimo elemento va estratto per prima
   }

   if (get_GS_CURRENT_WRK_SESSION() == NULL || ClsCodeList.get_count() == 0) return;

   // leggo la lista delle classi da estrarre con la particolarità che
   // il primo elemento rappresenta Mode (PREVIEW, EXTRACTION)
   int ExtractionMode;
   ExtractionMode = (m_Preview.GetCheck() == 1) ? PREVIEW : EXTRACTION;

   // il secondo elemento rappresenta SpatialCond (0 = query corrente, 1 = zoom corrente)
   int     SpatialCond = 1;
   if (m_SpatialQuery.GetCheck() == 1)
   {
      if (gsc_ade_qlqrygetid(_T("spaz_estr")) == ADE_NULLID) // non esiste query corrente
      {
         ((CButton *) GetDlgItem(IDC_RADIO_ACTUAL_WINDOW))->SetCheck(BST_CHECKED);
         m_SpatialQuery.SetCheck(BST_UNCHECKED);
      }
      else
         SpatialCond = 0;
   }

   C_STRING StrList, dummy;

   dummy.paste(ClsCodeList.to_str(_T(';')));

   StrList = ExtractionMode;
   StrList += _T(';');
   StrList += SpatialCond;
   StrList += _T(';');
   StrList += dummy;
   ClsCodeList.from_str(StrList.get_name(), _T(';'));

   COleDataSource *pSource = gsui_generateDataSource(ClsCodeList);
   CMyOverrideDropTarget myDT;
   CDropSource           myDS;
    
    // Start overriding AutoCAD's Droptarget
    if (!acedStartOverrideDropTarget(&myDT))
        TRACE("Error in overriding Custom drop target!\n");
    
    DROPEFFECT dwEffect = pSource->DoDragDrop(DROPEFFECT_COPY, NULL, &myDS);

    // End overriding AutoCAD default droptarget
    if (!acedEndOverrideDropTarget(&myDT))
        TRACE("Error in ending override drop target\n");

    delete pSource;
}

/*********************************************************/
/*.doc CWrkSessionClasses::extract                       */
/*+
  Questa funzione estrae nella sessione corrente un set di classi.
  Parametri:
  int        Mode;            modo estrazione (EXTRACTION, PREVIEW)
  int        SpatialCond;     0 = query corrente, 1 = zoom corrente
  C_INT_LIST &CodeClsList;    Lista codici classi
  
  Restituisce GS_GOOD in caso di successo altrimenti restituisce GS_BAD.
-*/  
/*********************************************************/
int CWrkSessionClasses::extract(int Mode, int SpatialCond, C_INT_LIST &CodeClsList)
{
   C_WRK_SESSION      *pCurrSession;
   C_PROJECT          *pCurrPrj;
   C_SINTH_CLASS_LIST SinthClassList;
   C_SINTH_CLASS      *pSinthClass;
   C_INT              *pCodeCls;
   int                Result;
   C_RECT_LIST        WindowList;
   
   if (!(pCurrSession = get_GS_CURRENT_WRK_SESSION())) return GS_BAD;
   if (!(pCurrPrj = pCurrSession->get_pPrj())) return GS_BAD;

   switch (SpatialCond)
   {
      case 0:
         // verifico se esiste una condizione spaziale di query salvata e la carico
         if (gsc_ade_qlloadqry(_T("spaz_estr")) == RTERROR) return GS_BAD;
         break;
      case 1: // carico zoom corrente
      {
         C_RB_LIST Cond;
         ads_point pt1, pt2;
      
         gsc_getCurrentScreenCoordinate(pt1, pt2);
         LastExtractedWindow.BottomLeft.Set(pt1[X], pt1[Y]);
         LastExtractedWindow.TopRight.Set(pt2[X], pt2[Y]);

         break;
      }
      default:
         return GS_BAD;
   }

   // Leggo una lista sintetica delle classi del progetto
   if (pCurrPrj->getSinthClassList(SinthClassList) == GS_BAD) return GS_BAD;

   // Rimuovo tutte le classi che non compaiono nella lista CodeClsList
   pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_head();
   while (pSinthClass)
   {
      if (!CodeClsList.search_key(pSinthClass->get_key()))
      {
         SinthClassList.remove(pSinthClass);
         pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_cursor();
      }
      else
         pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_next();
   }

   if (m_ClassesDisplayMode == List) // selezione con controllo a lista
   {
      // La ordino per livello di complessità e a parità di livello le classi
      // vengono ordinate come segue:
      // 1) superfici 2) spaghetti 3) polilinee 4) punti 5) testi.
      if (SinthClassList.sort_for_extraction(false) == GS_BAD) return GS_BAD;
   }
   else // selezione con controllo ad albero
      // sposto i gruppi alla fine della lista
      if (SinthClassList.sort_for_extraction(true) == GS_BAD) return GS_BAD;

   // ricostruisco la lista di interi
   CodeClsList.remove_all();
   pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_head();
   while (pSinthClass)
   {
      CodeClsList.add_tail_int(pSinthClass->get_key());
      pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_next();
   }


   Result = GS_GOOD;
   // per ogni classe da estrarre
   pCodeCls = (C_INT *) CodeClsList.get_head();
   if (pCodeCls)
   {
      C_INT_INT_LIST ErrClsCodeList, PartialErrClsCodeList;
      C_INT_INT      *pErrClsCode;
      C_INT_LIST dummyList;
      C_INT      *pPartialCodeCls;
      C_CLASS    *pCls;
      int        tentativi = 1, UsrBrk = FALSE, old_reactor_abilit, ExtrRes;
      long       extracted;

      if ((pPartialCodeCls = new C_INT) == NULL) return GS_BAD;
      dummyList.add_tail(pPartialCodeCls);

      C_STATUSBAR_PROGRESSMETER StatusBarProgressMeter(gsc_msg(1089)); // "Visualizzazione territorio"
      StatusBarProgressMeter.Init(SinthClassList.get_count());

      old_reactor_abilit = gsc_disable_reactors(); // disabilito i reattori di GEOsim

      do
      {
         pCodeCls = (C_INT *) CodeClsList.get_head();
         extracted = 0;

         // Ciclo di estrazione per classi
         while (pCodeCls)
         {
            Result = GS_BAD;

            // se l'utente vuole bloccare l'estrazione
            if (!UsrBrk && acedUsrBrk()) UsrBrk = TRUE;
            if (UsrBrk) break;

            if (!(pCls = pCurrPrj->find_class(pCodeCls->get_key())))
            {
               pCodeCls = (C_INT *) CodeClsList.get_next();
               continue;
            }

            pPartialCodeCls->set_key(pCodeCls->get_key());

            //if (pCurrSession->SelClasses(dummyList) == GS_GOOD)
            if (gsc_profile_int_func(gsc_get_profile_curr_time(), pCurrSession->SelClasses(dummyList), gsc_get_profile_curr_time(), "C_WRK_SESSION::SelClasses") == GS_GOOD)
            {
               if (SpatialCond == 1)  // carico zoom corrente
               {
                  ade_id qry_id;

                  // cancello la query ade precedente
                  if ((qry_id = gsc_ade_qlqrygetid(_T("spaz_estr"))) != ADE_NULLID)
                     if (ade_qldelquery(qry_id) != RTNORM)
                     {
                        pCodeCls = (C_INT *) CodeClsList.get_next();
                        continue;
                     }

                  // Ricavo la nuova query ade
                  if (gsui_getExtrWindowList(pCodeCls->get_key(), LastExtractedWindow, 
                                             WindowList) == GS_BAD)
                  {
                     pCodeCls = (C_INT *) CodeClsList.get_next();
                     continue;
                  }

                  if (WindowList.get_count() == 0)
                  {  // in questa zona la classe era già stata estratta
                     CodeClsList.remove_at();
                     pCodeCls = (C_INT *) CodeClsList.get_cursor();
                     Result   = GS_GOOD;
                     continue;
                  }

                  // Imposto la nuova query ade
                  if (ade_qryclear() != RTNORM ||
                        WindowList.AdeQryDefine() == GS_BAD ||
                        gsc_save_qry() == GS_BAD)
                  {
                     pCodeCls = (C_INT *) CodeClsList.get_next();
                     continue;
                  }
               }

               ErrClsCodeList.remove_key(pCodeCls->get_key());

               //ExtrRes = pCurrSession->extract(Mode,            // Modo estrazione
               //                                FALSE,           // Exclusive
               //                                &ErrClsCodeList, // Lista che verrà riempita con i
               //                                                 // codici delle classi non estratte e relativi errori
               //                                NULL,            // Gruppo oggetti estratti
               //                                ONETEST);        // Un solo test
               ExtrRes = gsc_profile_int_func(gsc_get_profile_curr_time(), 
                                                pCurrSession->extract(Mode, FALSE, &PartialErrClsCodeList, NULL, ONETEST),
                                                gsc_get_profile_curr_time(), "C_WRK_SESSION::extract");

               if (ExtrRes == GS_CAN) UsrBrk = TRUE; // Interruzione dell'utente
               else if (ExtrRes == GS_GOOD && PartialErrClsCodeList.get_count() == 0)
               {  // estratta
                  if (SpatialCond == 1)  // carico zoom corrente
                     gsui_WriteExtrWindowList(pCodeCls->get_key(), WindowList);
 
                  CodeClsList.remove_at();
                  pCodeCls = (C_INT *) CodeClsList.get_cursor();
                  Result   = GS_GOOD;
                  StatusBarProgressMeter.Set(++extracted);
               }
               else if (ExtrRes == GS_BAD)
               {
                  if ((pErrClsCode = (C_INT_INT *) PartialErrClsCodeList.get_head()))
                     ErrClsCodeList.values_add_tail(pErrClsCode->get_key(), pErrClsCode->get_type());
                  else
                     gsc_print_error();
               }
            }
                        
            if (Result == GS_BAD) 
               pCodeCls = (C_INT *) CodeClsList.get_next();
         }

         if (CodeClsList.get_count() == 0) break; // tutto OK
         
         tentativi++;
      }
      while (tentativi <= get_GS_GLOBALVAR()->get_NumTest());
      StatusBarProgressMeter.End(gsc_msg(1090)); // "Terminato."

      // Se alcune classi non sono state estratte visualizzo un warning
      pCurrSession->DisplayMsgsForClassNotExtracted(ErrClsCodeList);

      pCurrSession->SelClasses(CodeClsList);

      // ripristino il controllo sul reattore come in precedenza
      if (old_reactor_abilit == GS_GOOD) gsc_enable_reactors();

      // aggiorno lista classi selezionate (flag di stato)
      m_GEOClassListCtrl.LoadFromDB(true);
      m_GEOClassListCtrl.Refresh(true);
   }

   return Result;
}

void CWrkSessionClasses::OnLvnBegindragGeoclasslistctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
   
   StartDrag();

   *pResult = 0;
}

void CWrkSessionClasses::OnTvnBegindragGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   StartDrag();

   *pResult = 0;
}

void CWrkSessionClasses::OnLvnItemchangedGeoclasslistctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
   
   if (AlignmentControl == false)
      OnChangeSelectionGeolistctrl(pNMLV); // Se non si è in fase di allineamento dei controlli

   if (pResult) *pResult = 0;
}


void CWrkSessionClasses::OnBnClickedRadioQuery()
{
   C_STRING filename, str;

   if (get_GS_CURRENT_WRK_SESSION())
   {
      str = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_dir();
      str += _T("\\");
      str += GEOQRYDIR;
      str += _T("\\");
   }

   if (gsc_GetFileD(_T("GEOsim - Selezionare file di Query"), str,
                    _T("qry"), UI_LOADFILE_FLAGS, filename) == RTNORM)
   {
      // Carico la query, la rendo corrente quindi la salvo 
      // come query di GEOsim (solo coordinate)
      if (gsc_save_qry(_T("estrazione"), _T("spaz_estr"), TRUE, 
                       filename.get_name()) == GS_BAD)
      {
         CButton *pBtn;

         gsc_ddalert(_T("Query spaziale non caricata."));

         if ((pBtn = (CButton *) GetDlgItem(IDC_RADIO_ACTUAL_WINDOW)))
         {
            pBtn->SetCheck(BST_CHECKED);
            m_SpatialQuery.SetCheck(BST_UNCHECKED);
         }
      }
   }
}


void CWrkSessionClasses::OnBnClickedDisplayModeButton()
{
   switch (m_ClassesDisplayMode)
   {
      case List:
         m_ClassesDisplayMode = Tree;
         break;
      case Tree:
         m_ClassesDisplayMode = List;
   }
   DisplayMode();
}

void CWrkSessionClasses::DisplayMode(void)
{
   switch (m_ClassesDisplayMode)
   {
      case List:
         m_DisplayModeButton.SetBitmap(hBmpTreeDisplayMode);
         if (pToolTip && pToolTip->m_hWnd) pToolTip->UpdateTipText(_T("Modalità di visualizzazione ad albero"), &m_DisplayModeButton);
         m_GEOClassListCtrl.ShowWindow(SW_SHOW);
         m_GEOClassTreeCtrl.ShowWindow(SW_HIDE);
         break;
      case Tree:
         m_DisplayModeButton.SetBitmap(hBmpListDisplayMode);
         if (pToolTip && pToolTip->m_hWnd) pToolTip->UpdateTipText(_T("Modalità di visualizzazione a lista"), &m_DisplayModeButton);
         m_GEOClassListCtrl.ShowWindow(SW_HIDE);
         m_GEOClassTreeCtrl.ShowWindow(SW_SHOW);
   }
}

BOOL CWrkSessionClasses::InitTooltip(void)
{
   if (pToolTip) delete pToolTip;
   pToolTip = new CToolTipCtrl;
   if (!pToolTip->Create(this, TTS_ALWAYSTIP)) return FALSE;
   if (m_DisplayModeButton.m_hWnd) pToolTip->AddTool(&m_DisplayModeButton, _T(""));

   pToolTip->SetDelayTime(TTDT_INITIAL, 0);
   pToolTip->Activate(TRUE);
   
   return TRUE;
}

BOOL CWrkSessionClasses::DisplayTooltip(MSG* pMsg)
{
   if (!pToolTip || !pToolTip->m_hWnd) return TRUE;
   
   // tooltip per i bottoni
   if (pMsg->hwnd == m_DisplayModeButton.m_hWnd)
   {
      pToolTip->RelayEvent(pMsg);
      return TRUE;
   }

   return TRUE;
}

BOOL CWrkSessionClasses::PreTranslateMessage(MSG* pMsg) 
{
   BOOL Res;

   DisplayTooltip(pMsg);

   Res = CPropertyPage::PreTranslateMessage(pMsg);

   // Refresh dei controlli
   if (pMsg->hwnd == m_GEOClassTreeCtrl.m_hWnd && 
       pMsg->message == WM_COMMAND && pMsg->wParam == ID_GEOTREECTRL_REFRESH_MENU)
      m_GEOClassListCtrl.OnRefreshMenu() ; // refresh dell'altro controllo
   else
   // Refresh del controllo a lista delle classi 
   if (pMsg->hwnd == m_GEOClassListCtrl.m_hWnd && 
       pMsg->message == WM_COMMAND && pMsg->wParam == ID_REFRESH_MENU)
      m_GEOClassTreeCtrl.OnRefreshMenu() ; // refresh dell'altro controllo

   return Res;
}


void CWrkSessionClasses::OnNMClickGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   m_GEOClassTreeCtrl.OnNMClickGeotreectrl(pNMHDR, pResult);
   CPoint pt;
   UINT   uFlags;

   GetCursorPos(&pt);
   m_GEOClassTreeCtrl.ScreenToClient(&pt);
   HTREEITEM hItem = m_GEOClassTreeCtrl.HitTest(pt, &uFlags);
      
   if (hItem && (uFlags & TVHT_ONITEMSTATEICON))
      OnChangeSelectionGeotreectrl();
   *pResult = 0;

   *pResult = 0;
}

void CWrkSessionClasses::OnChangeSelectionGeotreectrl(void)
{
   C_INT_LIST SelectedCodes;
   
   AlignmentControl = true; // inizio fase di allineamento dei controlli
   m_GEOClassTreeCtrl.GetSelectedCodes(SelectedCodes);
   m_GEOClassListCtrl.SetSelectedCodes(SelectedCodes);
   m_GEOClassListCtrl.Refresh(true);
   RefreshSelectedCount();
   AlignmentControl = false;
}
void CWrkSessionClasses::OnChangeSelectionGeolistctrl(LPNMLISTVIEW pNMLV)
{
   C_INT_LIST SelectedCodes;

   AlignmentControl = true; // inizio fase di allineamento dei controlli
   if (pNMLV == NULL)
   {
      m_GEOClassListCtrl.GetSelectedCodes(SelectedCodes);
      m_GEOClassTreeCtrl.SetSelectedCodes(SelectedCodes);
   }
   else // se è cambiato lo stato di selezione roby
      if (pNMLV->uChanged & LVIF_STATE && ((pNMLV->uOldState & LVNI_SELECTED) != (pNMLV->uNewState & LVNI_SELECTED))) 
      {
         CGEOList_Item *pItem = (CGEOList_Item *) m_GEOClassListCtrl.GetItemData(pNMLV->iItem);
         m_GEOClassTreeCtrl.SetSelectedCode(pItem->get_key(), (pNMLV->uNewState & LVNI_SELECTED) ? true : false);
      }

   RefreshSelectedCount();
   AlignmentControl = false;
}

void CWrkSessionClasses::RefreshSelectedCount(void)
{
   C_INT_LIST SelectedCodes;
   C_STRING   Msg;

   m_GEOClassListCtrl.GetSelectedCodes(SelectedCodes);
   Msg = SelectedCodes.get_count();
   Msg += _T('/');
   Msg += m_GEOClassListCtrl.GetItemCount(); 
	m_SelectedCount.SetWindowText(Msg.get_name());
}

void CWrkSessionClasses::OnTvnKeydownGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

   m_GEOClassTreeCtrl.OnKeyDown(pTVKeyDown->wVKey, 1, 57); // pTVKeyDown->flags è sempre 0
   OnChangeSelectionGeotreectrl();

   *pResult = 0;
}

void CWrkSessionClasses::OnNMRClickGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult)
{
   m_GEOClassTreeCtrl.OnRClick(pNMHDR, pResult);
}


//////////////////////////////////////////////////////////////////////////////
// CDockPaneWrkSession

IMPLEMENT_DYNAMIC(CDockPaneWrkSession, CAdUiPaletteSet)

CDockPaneWrkSession::CDockPaneWrkSession()
{
}

CDockPaneWrkSession::~CDockPaneWrkSession()
{
   UINT  nPreferredOrientation;
   CRect DockSize, FloatingSize;

	if (m_MiniFrameHook.IsHooked())
	{
      Hide();
		// Unhook the miniframe window
		m_MiniFrameHook.HookWindow(NULL);
	}

   GetFloatingRect(&FloatingSize);
   m_WrkSessionTab.m_WrkSessionClasses.m_GEOClassTreeCtrl.getExpanded_prj_clsSet_list(m_Expanded_prj_clsSet_List);

   if (IsFloating())
   {
      nPreferredOrientation = AFX_IDW_DOCKBAR_FLOAT;
      SaveInfoControlBar(nPreferredOrientation, NULL, &FloatingSize, GetOpacity(), GetAutoRollup(),
                         m_WrkSessionTab.m_WrkSessionClasses.m_ClassesDisplayMode,
                         m_Expanded_prj_clsSet_List);
   }
   else
      if (GetParent())
      {
         nPreferredOrientation = GetParent()->GetDlgCtrlID(); // posizione del pannello
         GetWindowRect(&DockSize);
         SaveInfoControlBar(nPreferredOrientation, &DockSize, &FloatingSize, GetOpacity(), GetAutoRollup(),
                            m_WrkSessionTab.m_WrkSessionClasses.m_ClassesDisplayMode,
                            m_Expanded_prj_clsSet_List);
      }
      else
      {
         nPreferredOrientation = AFX_IDW_DOCKBAR_LEFT;
         SaveInfoControlBar(nPreferredOrientation, NULL, &FloatingSize, GetOpacity(), GetAutoRollup(),
                           m_WrkSessionTab.m_WrkSessionClasses.m_ClassesDisplayMode,
                           m_Expanded_prj_clsSet_List);
      }
}

BOOL CDockPaneWrkSession::Create(CWnd *pParentWnd)
{
	// For now, the toolID will be hard-coded, until Alain gives us a unique value
	// AutoCAD uses this ID to retrieve the location of the pane in the registry
	UINT  nControlBarID = AFX_IDW_CONTROLBAR_FIRST + 5;
   UINT  nPreferredOrientation;
   CRect DockSize, FloatingSize;
   int   Opacity, InfoControlBarLoaded;
   BOOL  AutoRollup;

   InfoControlBarLoaded = LoadInfoControlBar(nPreferredOrientation, &DockSize, &FloatingSize,
                                             &Opacity, &AutoRollup,
                                             &(m_WrkSessionTab.m_WrkSessionClasses.m_ClassesDisplayMode),
                                             m_Expanded_prj_clsSet_List, m_ExtractedClasses);


	// Create the dockable pane window
	CString sWndClass;
	sWndClass = AfxRegisterWndClass(CS_DBLCLKS);
	CRect rect(0, 0, 264, 400);
	if (!CAdUiPaletteSet::Create(_T("GEOsim - Sessione di lavoro"),
		                          WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN,
		                          rect,
		                          pParentWnd))
		return FALSE;
	
	// Move the dockable pane window to the last known location
	EnableDocking(CBRS_ALIGN_ANY);
   SetToolID(&CLSID_DockPaneWrkSession);

   if (InfoControlBarLoaded == GS_GOOD)
   {
      // non so perchè non riesco a visualizzare in floating (roby)
      if (nPreferredOrientation == AFX_IDW_DOCKBAR_FLOAT) nPreferredOrientation = AFX_IDW_DOCKBAR_LEFT;

      if (nPreferredOrientation != AFX_IDW_DOCKBAR_FLOAT)
   	   DockControlBar(nPreferredOrientation, &DockSize);
      else
      {
         InitFloatingPosition(&FloatingSize);
      	EnableDocking(CBRS_NOALIGN);
      }
      SetOpacity(Opacity);
      SetAutoRollup(AutoRollup);
   }
   else
	   RestoreControlBar();

   // setto i nodi espansi
   m_WrkSessionTab.m_WrkSessionClasses.m_GEOClassTreeCtrl.expand_prj_clsSet_list(m_Expanded_prj_clsSet_List);
   // setto le classi selezionate
   m_WrkSessionTab.m_WrkSessionClasses.m_GEOClassListCtrl.SetSelectedCodes(m_ExtractedClasses);

   return TRUE;
}

int CDockPaneWrkSession::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CAdUiPaletteSet::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
   
   m_WrkSessionTab.Create(this);
	return 0;
}

void CDockPaneWrkSession::SizeChanged(CRect *lpRect, BOOL bFloating, int flags)
{
   if (!m_bFloating && ADUI_DOCK_NF_SIZECHANGED) // docked
	{  // se allinata in alto o in basso
      if (GetBarStyle() & CBRS_ALIGN_TOP || GetBarStyle() & CBRS_ALIGN_BOTTOM)
         m_WrkSessionTab.SetWindowPos(NULL, 12, 0, lpRect->Width() + 8, 
                                      lpRect->Height() + 3, SWP_NOZORDER);
      else
         // se è allinata a sinistra o a destra
         if (GetBarStyle() & CBRS_ALIGN_LEFT || GetBarStyle() & CBRS_ALIGN_RIGHT)
            m_WrkSessionTab.SetWindowPos(NULL, 0, 23, lpRect->Width() + 3,
                                         lpRect->Height() + 7, SWP_NOZORDER);
	}
	else // floating
		m_WrkSessionTab.SetWindowPos(NULL, -1, 0, lpRect->Width() + 5, lpRect->Height() + 6,
		                             SWP_NOZORDER);
}

void CDockPaneWrkSession::OnMouseMove(UINT nFlags, CPoint point) 
{
	CAdUiPaletteSet::OnMouseMove(nFlags, point);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}

bool CDockPaneWrkSession::CanFrameworkTakeFocus() // per non perdere il fuoco
{
   return CAdUiPaletteSet::CanFrameworkTakeFocus();
	// return FALSE; test
}

BEGIN_MESSAGE_MAP(CDockPaneWrkSession, CAdUiPaletteSet)
    //{{AFX_MSG_MAP(CDockPaneWrkSession)
   ON_WM_CREATE()
   ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_DOCKSTATE_CHANGED, OnChangedDockedState)
END_MESSAGE_MAP()


BOOL CDockPaneWrkSession::IsWindowVisible() const
{
   if (!CAdUiPaletteSet::IsWindowVisible())
      return FALSE;

   return TRUE;
}

void CDockPaneWrkSession::HookMiniFrame()
{
    CWnd* pMiniFrameWnd = GetParentFrame();
    m_MiniFrameHook.HookWindow(pMiniFrameWnd);
}

LRESULT CDockPaneWrkSession::OnChangedDockedState(WPARAM, LPARAM)
{
    //if we are not floating, unhook the window procedure, otherwise hook it
    if(m_bFloating)
        HookMiniFrame();
    return 0L;
}

void CDockPaneWrkSession::OnBarStyleChange(DWORD oldStyle, DWORD newStyle)
{
    m_bFloating = (newStyle & CBRS_FLOATING);

    //This is lame - when this notification comes we can't get a pointer
    //to the miniframe even though its already been created, so we'll
    //post a message to ourselves to swtich it.
    PostMessage(WM_DOCKSTATE_CHANGED);
    //CAdUiPaletteSet::OnBarStyleChange(oldStyle, newStyle)
}

////////////////////////////////////////////////////////////////////////////
// This method iterates over all the Object Data tables in the drawing
// and updates the drop down list boxes in the docking window.
////////////////////////////////////////////////////////////////////////////
Adesk::Boolean CDockPaneWrkSession::OnLoadDwg()
{
	return Adesk::kTrue;
}

Adesk::Boolean CDockPaneWrkSession::OnUnloadDwg()
{
	return Adesk::kTrue;
}

void CDockPaneWrkSession::SetFloatingSize(const CSize& size)
{
	// TODO
}

void CDockPaneWrkSession::OnSaveComplete(AcDbDatabase *pDwg, const TCHAR *pActualName)
{
	// TODO
}

void CDockPaneWrkSession::DBQuery(C_SELSET &ss)
{
   if (!IsWindowVisible()) return;
   int ItemCount = m_WrkSessionTab.GetItemCount();

   // Se esistono oggetti appartenenti a classi GEOsim
   if (m_WrkSessionTab.m_EntDBValuesList.RefreshCls(ss) > 0)
   {
      int i;

      // Cancello tutti i tab escluso il primo e il secondo
      for (i = ItemCount; i > 2; i--)
         m_WrkSessionTab.DeleteItem(i - 1);

      if (ItemCount == 1) // c'era un solo tab
      {
	      TC_ITEM TabCtrlItem;
	      TabCtrlItem.mask = TCIF_TEXT;
	      TabCtrlItem.pszText = _T("Attributi");
	      m_WrkSessionTab.InsertItem(1, &TabCtrlItem);
      }

      if (m_WrkSessionTab.GetCurSel() != 1)
         m_WrkSessionTab.SetEntDBValuesListItem();

      i = 0;
      while (i < m_WrkSessionTab.m_EntDBValuesList.m_ClassAndSecListCombo.GetCount())
         if (m_WrkSessionTab.m_EntDBValuesList.SelchangeClasslistCombo(i++) > 0)
            break;
   }
   else
   {
      // Cancello tutti i tab escluso il primo
      for (int i = ItemCount; i > 1; i--)
         m_WrkSessionTab.DeleteItem(i - 1);

      if (ItemCount > 1)
         m_WrkSessionTab.SetWrkSessionClassesItem();
   }
}


/*************************************************************************/
/*.doc CDockPaneWrkSession::GridDBQuery                                  */
/*+ 
   Funzione che interroga i dati alfanumerici legati alle entità
   che intersecano un rettangolo o le cui celle contengono
   un punto dato.
   Parametri:
   ads_point pt1;   Primo punto (se = NULL utilizza i punti precedentemente
                    selezionati)
   ads_point pt2;   Opzionale, se = NULL si devono cercare le celle
                    che contengono il punto pt1 altrimenti
                    si devono cercare le celle che intersecano
                    il rettangolo formato da pt1 (angolo basso-sinistra)
                    e pt2 (angolo alto-destra).

   oppure:
   C_CGRID      *pCls;    Puntatore a classe griglia
   C_LONG_BTREE &KeyList; Lista dei codici delle celle

   Ritorna GS_BAD in caso di errore, GS_GOOD altrimenti.
-*/
/*************************************************************************/
void CDockPaneWrkSession::GridDBQuery(ads_point pt1, ads_point pt2)
{
   if (!IsWindowVisible()) return;
   int ItemCount = m_WrkSessionTab.GetItemCount();

   // Se esistono oggetti appartenenti a classi GEOsim
   if (m_WrkSessionTab.m_EntDBValuesList.RefreshGridCls(pt1, pt2) > 0)
   {
      int i;

      // Cancello tutti i tab escluso il primo e il secondo
      for (i = ItemCount; i > 2; i--)
         m_WrkSessionTab.DeleteItem(i - 1);

      if (ItemCount == 1) // c'era un solo tab
      {
	      TC_ITEM TabCtrlItem;
	      TabCtrlItem.mask = TCIF_TEXT;
	      TabCtrlItem.pszText = _T("Attributi");
	      m_WrkSessionTab.InsertItem(1, &TabCtrlItem);
      }

      if (m_WrkSessionTab.GetCurSel() != 1)
         m_WrkSessionTab.SetEntDBValuesListItem();

      i = 0;
      while (i < m_WrkSessionTab.m_EntDBValuesList.m_ClassAndSecListCombo.GetCount())
         if (m_WrkSessionTab.m_EntDBValuesList.SelchangeClasslistCombo(i++) > 0)
            break;
   }
   else
   {
      // Cancello tutti i tab escluso il primo
      for (int i = ItemCount; i > 1; i--)
         m_WrkSessionTab.DeleteItem(i - 1);

      if (ItemCount > 1)
         m_WrkSessionTab.SetWrkSessionClassesItem();
   }
}
void CDockPaneWrkSession::GridDBQuery(C_CGRID *pCls, C_LONG_BTREE &KeyList)
{
   if (!IsWindowVisible()) return;
   int ItemCount = m_WrkSessionTab.GetItemCount();

   // Se esistono oggetti appartenenti a classi GEOsim
   if (m_WrkSessionTab.m_EntDBValuesList.RefreshGridCls(pCls, KeyList) > 0)
   {
      int i;

      // Cancello tutti i tab escluso il primo e il secondo
      for (i = ItemCount; i > 2; i--)
         m_WrkSessionTab.DeleteItem(i - 1);

      if (ItemCount == 1) // c'era un solo tab
      {
	      TC_ITEM TabCtrlItem;
	      TabCtrlItem.mask = TCIF_TEXT;
	      TabCtrlItem.pszText = _T("Attributi");
	      m_WrkSessionTab.InsertItem(1, &TabCtrlItem);
      }

      if (m_WrkSessionTab.GetCurSel() != 1)
         m_WrkSessionTab.SetEntDBValuesListItem();

      i = 0;
      while (i < m_WrkSessionTab.m_EntDBValuesList.m_ClassAndSecListCombo.GetCount())
         if (m_WrkSessionTab.m_EntDBValuesList.SelchangeClasslistCombo(i++) > 0)
            break;
   }
   else
   {
      // Cancello tutti i tab escluso il primo
      for (int i = ItemCount; i > 1; i--)
         m_WrkSessionTab.DeleteItem(i - 1);

      if (ItemCount > 1)
         m_WrkSessionTab.SetWrkSessionClassesItem();
   }
}


/*************************************************************************/
/*.doc CDockPaneWrkSession::IsDBGridMode                                 */
/*+ 
   Funzione che verifica se si stanno interrogando dei dati griglia.

   Ritorna true in caso affermativo, false altrimenti.
-*/
/*************************************************************************/
bool CDockPaneWrkSession::IsDBGridMode(void)
{
   return m_WrkSessionTab.m_EntDBValuesList.IsDBGridMode();
}


//---------------------------------------------------------------------------
// Show the dockable pane. If it is not created,
// then create it and then show it
//
void CDockPaneWrkSession::Show()
{
	CMDIFrameWnd *pAcadFrame = acedGetAcadFrame();
	assert(pAcadFrame);
	pAcadFrame->ShowControlBar(this, TRUE, FALSE);	
}

//---------------------------------------------------------------------------
// Hide the dockable pane
//
void CDockPaneWrkSession::Hide(void)
{
	CMDIFrameWnd *pAcadFrame = acedGetAcadFrame();
	assert(pAcadFrame);
	pAcadFrame->ShowControlBar(this, FALSE, FALSE);
}


/*************************************************************************/
/*.doc CDockPaneWrkSession::SaveInfoControlBar                            */
/*+ 
   Scrive sul file di inizializzazione di GEOsim le dimensioni della control-bar.
   Parametri:
   UINT nPreferredOrientation;   lo stato della control-bar
                                 (AFX_IDW_DOCKBAR_TOP se attaccata in alto,
                                 AFX_IDW_DOCKBAR_LEFT se attaccata a sinistra,
                                 AFX_IDW_DOCKBAR_RIGHT se attaccata a destra,
                                 AFX_IDW_DOCKBAR_BOTTOM se attaccata in basso,
                                 AFX_IDW_DOCKBAR_FLOAT se non attaccata)
   CRect *pDockSize              Opzionale, dimensioni della finestra in modo dock
                                 (default = NULL)
   CRect *pFloatingSize;         Opzionale, dimensioni della finestra in modo floating
                                 (default = NULL)
   int   *Opacity;               Opzionale, 0 is fully transparent and 100 is opaque
                                 (default = 100)
   BOOL  AutoRollup;             Opzionale, TRUE if auto roll-up is enabled
                                 (default = TRUE)
   CWrkSessionClasses::DisplayModeTypeEnum ClassesDisplayMode; Opzionale, modalità 
                                                               di visualizzazione delle classi
                                                               (default = List)
   C_INT_INT_LIST &Expanded_prj_clsSet_List; nodi espansi di progetti e set di classi 
                                             dell'albero di visualizzazione
-*/
/*************************************************************************/
int CDockPaneWrkSession::SaveInfoControlBar(UINT nPreferredOrientation,
                                           CRect *pDockSize, CRect *pFloatingSize,
                                           int Opacity, BOOL AutoRollup,
                                           CWrkSessionClasses::DisplayModeTypeEnum ClassesDisplayMode,
                                           C_INT_INT_LIST &Expanded_prj_clsSet_List)
{
   C_STRING                pathfile, Value;
   C_PROFILE_SECTION_BTREE ProfileSections;
   C_BPROFILE_SECTION      *ProfileSection;
   bool                    Unicode = false;

   // scrivo in GEOSIM.INI
   pathfile = get_CURRUSRDIR(); // Directory locale dell'utente corrente
   pathfile += _T('\\');
   pathfile += GS_INI_FILE;

   if (gsc_path_exist(pathfile) == GS_GOOD)
      if (gsc_read_profile(pathfile, ProfileSections, &Unicode) == GS_BAD) return GS_BAD;

   if (!(ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("WrkSessionPanel"))))
   {
      if (ProfileSections.add(_T("WrkSessionPanel")) == GS_BAD) return GS_BAD;
      ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.get_cursor();
   }

   ProfileSection->set_entry(_T("Orientation"), (long) nPreferredOrientation);

   if (pDockSize)
   {
      Value = pDockSize->left;
      Value += _T(',');
      Value += pDockSize->top;
      Value += _T(';');
      Value += pDockSize->right;
      Value += _T(',');
      Value += pDockSize->bottom;
      ProfileSection->set_entry(_T("DockSize"), Value.get_name());
   }

   if (pFloatingSize)
   {
      Value = pFloatingSize->left;
      Value += _T(',');
      Value += pFloatingSize->top;
      Value += _T(';');
      Value += pFloatingSize->right;
      Value += _T(',');
      Value += pFloatingSize->bottom;
      ProfileSection->set_entry(_T("FloatingSize"), Value.get_name());
   }

   ProfileSection->set_entry(_T("Opacity"), Opacity);

   ProfileSection->set_entry(_T("AutoRollup"), AutoRollup);

   // modalità di visualizzazione delle classi
   ProfileSection->set_entry(_T("WrkSessionClasses_DisplayMode"), (int) ClassesDisplayMode);

   int      prj;
   C_STRING entry;

   if (get_GS_CURRENT_WRK_SESSION())
      prj  = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_key();
   else
      gsc_getLastUsedPrj(&prj);

   // nodi espansi di progetti e set di classi dell'albero di visualizzazione
   entry = _T("WrkSessionClasses_ExpandedPrjClsSetNodes_PRJ");
   entry += prj;
   Value = Expanded_prj_clsSet_List.to_str();
   ProfileSection->set_entry(entry.get_name(), Value.get_name());

   return gsc_write_profile(pathfile.get_name(), ProfileSections, Unicode);
}


/*************************************************************************/
/*.doc CDockPaneWrkSession::LoadInfoControlBar                            */
/*+ 
   Carica dal file di inizializzazione di GEOsim le dimensioni della control-bar.
   Parametri:
   UINT &nPreferredOrientation;  lo stato della control-bar
                                 (AFX_IDW_DOCKBAR_TOP se attaccata in alto,
                                 AFX_IDW_DOCKBAR_LEFT se attaccata a sinistra,
                                 AFX_IDW_DOCKBAR_RIGHT se attaccata a destra,
                                 AFX_IDW_DOCKBAR_BOTTOM se attaccata in basso,
                                 AFX_IDW_DOCKBAR_FLOAT se non attaccata)
   CRect *pDockSize              dimensioni della finestra in modo dock
   CRect *pFloatingSize;         dimensioni della finestra in modo floating
   int   *Opacity;               0 is fully transparent and 100 is opaque
   BOOL  *AutoRollup;            TRUE if auto roll-up is enabled
   CWrkSessionClasses::DisplayModeTypeEnum *ClassesDisplayMode; modalità di visualizzazione
                                                                delle classi
   C_INT_INT_LIST &Expanded_prj_clsSet_List;
   C_INT_LIST &ExtractedClasses;
-*/
/*************************************************************************/
int CDockPaneWrkSession::LoadInfoControlBar(UINT &nPreferredOrientation, 
                                           CRect *pDockSize, CRect *pFloatingSize,
                                           int *Opacity, BOOL *AutoRollup,
                                           CWrkSessionClasses::DisplayModeTypeEnum *ClassesDisplayMode,
                                           C_INT_INT_LIST &Expanded_prj_clsSet_List, C_INT_LIST &ExtractedClasses)
{
   C_STRING                pathfile;
   C_POINT_LIST            Ptlist;
   C_POINT                 *pPt1, *pPt2;
   C_PROFILE_SECTION_BTREE ProfileSections;
   C_BPROFILE_SECTION      *ProfileSection;
   C_2STR_BTREE            *pProfileEntries;
   C_B2STR                 *pProfileEntry;
   
   // leggo da GEOSIM.INI
   pathfile = get_CURRUSRDIR(); // Directory locale dell'utente corrente
   pathfile += _T('\\');
   pathfile += GS_INI_FILE;

   if (gsc_read_profile(pathfile, ProfileSections) == GS_BAD) return GS_BAD;
   if (!(ProfileSection = (C_BPROFILE_SECTION *) ProfileSections.search(_T("WrkSessionPanel"))))
      return GS_BAD;
   pProfileEntries = (C_2STR_BTREE *) ProfileSection->get_ptr_EntryList();

   // Orientation
   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("Orientation"))))
      return GS_BAD;
   nPreferredOrientation = (UINT) _wtol(pProfileEntry->get_name2());
   if (nPreferredOrientation == 0) return GS_BAD; // qualcosa non va

   // DockSize
   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("DockSize"))))
      return GS_BAD;
   Ptlist.fromStr(pProfileEntry->get_name2());

   if ((pPt1 = (C_POINT *) Ptlist.get_head()) == NULL) return GS_BAD;
   if ((pPt2 = (C_POINT *) pPt1->get_next()) == NULL) return GS_BAD;
   pDockSize->SetRect((int) pPt1->x(), (int) pPt1->y(), (int) pPt2->x(), (int) pPt2->y());

   // FloatingSize
   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("FloatingSize"))))
      return GS_BAD;
   Ptlist.fromStr(pProfileEntry->get_name2());

   if ((pPt1 = (C_POINT *) Ptlist.get_head()) == NULL) return GS_BAD;
   if ((pPt2 = (C_POINT *) pPt1->get_next()) == NULL) return GS_BAD;
   pFloatingSize->SetRect((int) pPt1->x(), (int) pPt1->y(), (int) pPt2->x(), (int) pPt2->y());

   // Opacity
   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("Opacity")))) *Opacity = 100;
   else *Opacity = _wtoi(pProfileEntry->get_name2());

   if (*Opacity < 0 || *Opacity > 100) *Opacity = 100;

   // AutoRollup
   if (!(pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("AutoRollup")))) *AutoRollup = TRUE;
   else *AutoRollup = (_wtoi(pProfileEntry->get_name2()) == 0) ? FALSE : TRUE;

   // modalità di visualizzazione delle classi
   *ClassesDisplayMode = CWrkSessionClasses::List;
   if ((pProfileEntry = (C_B2STR *) pProfileEntries->search(_T("WrkSessionClasses_DisplayMode"))) &&
       _wtoi(pProfileEntry->get_name2()) == (int) CWrkSessionClasses::Tree)
         *ClassesDisplayMode = CWrkSessionClasses::Tree;

   int      prj;
   C_STRING entry;

   if (get_GS_CURRENT_WRK_SESSION())
      prj  = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_key();
   else
      gsc_getLastUsedPrj(&prj);

   // nodi espansi di progetti e set di classi dell'albero di visualizzazione
   entry = _T("WrkSessionClasses_ExpandedPrjClsSetNodes_PRJ");
   entry += prj;

   Expanded_prj_clsSet_List.remove_all();
   if ((pProfileEntry = (C_B2STR *) pProfileEntries->search(entry.get_name())))
      Expanded_prj_clsSet_List.from_str(pProfileEntry->get_name2());

   //////////////////////
   // Riallineamento automatico
   if (get_GS_GLOBALVAR()->get_SelectPreviousExtractedClasses() == GS_GOOD)
      // Ricavo la lista delle classi estratte del progetto corrente
      gsc_getLastExtractedClassesFromINI(prj, ExtractedClasses);

   return GS_GOOD;
}


/////////////////////////////////////////////////////////////////////////////
// CGSWrkSessionTab
/////////////////////////////////////////////////////////////////////////////


CGSWrkSessionTab::CGSWrkSessionTab()
{
}

CGSWrkSessionTab::~CGSWrkSessionTab()
{
}


BOOL CGSWrkSessionTab::Create(CWnd *pParentWnd)
{
   // Create the TabCtrl
	CRect rect(0, 0, 254, 200);
	if (!CTabCtrl::Create(TCS_TABS, rect, pParentWnd, 0))
		return FALSE;

	TC_ITEM TabCtrlItem;
	TabCtrlItem.mask    = TCIF_TEXT;
	TabCtrlItem.pszText = _T("Estrazione");
	InsertItem(0, &TabCtrlItem);

   ShowWindow(SW_SHOW);

   // con la Create viene chiamata la OnInitDialog
   m_WrkSessionClasses.Create(CWrkSessionClasses::IDD, this);
   m_WrkSessionClasses.ShowWindow(SW_SHOW);

   // con la Create viene chiamata la OnInitDialog
   m_EntDBValuesList.Create(CEntDBValuesListDlg::IDD, this);

   return TRUE;
}

void CGSWrkSessionTab::OnSize(UINT nType, int cx, int cy) 
{
	CTabCtrl::OnSize(nType, cx, cy);
	
	m_WrkSessionClasses.SetWindowPos(NULL, 1, 24, cx - 4, cy - 27,
	                                 SWP_NOZORDER);
	m_EntDBValuesList.SetWindowPos(NULL, 1, 24, cx - 4, cy - 27,
	                               SWP_NOZORDER);
}

void CGSWrkSessionTab::OnSelchange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int nTabSelected = GetCurSel();
	
   if (nTabSelected == 0) // accendo solo il tab di estrazione
      SetWrkSessionClassesItem();
   else if (nTabSelected == 1) // accendo solo il tab di scheda principale
      SetEntDBValuesListItem();

   RedrawWindow();

	*pResult = 0;
}

void CGSWrkSessionTab::SetWrkSessionClassesItem(void)
{  // accendo solo il tab di estrazione
   SetCurSel(0);
   m_EntDBValuesList.ShowWindow(SW_HIDE);
   m_WrkSessionClasses.ShowWindow(SW_HIDE); // per forzare un refresh
   m_WrkSessionClasses.ShowWindow(SW_SHOW);
}

void CGSWrkSessionTab::SetEntDBValuesListItem(void)
{  // accendo solo il tab di scheda principale
   SetCurSel(1);
   m_WrkSessionClasses.ShowWindow(SW_HIDE);
   m_EntDBValuesList.ShowWindow(SW_SHOW);
}

BEGIN_MESSAGE_MAP(CGSWrkSessionTab, CTabCtrl)
	//{{AFX_MSG_MAP(CGSWrkSessionTab)
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGSWrkSessionTab message handlers


/////////////////////////////////////////////////////////////////////////////
// GS_UI_InputContextReactor Input context reactor

void GS_UI_InputContextReactor::beginQuiescentState()
{
   // MAP in stato tranquillo = non si sta facendo una mazza
   if (!get_GS_CURRENT_WRK_SESSION()) return;

   int old_reactor_abilit;
   long AlteredEntities = 0;

   //////////////////////
   // prova lo sbloccaggio di tutte le classi della sessione corrente che 
   // erano in attesa di essere sbloccate ma che per motivi legati alla
   // multiutenza non è stato possibile sbloccare.
   get_GS_CURRENT_WRK_SESSION()->UnlockWaitingClsList();
   // fine sbloccaggio
   ///////////////////////////

   C_INIT *pGS_GLOBALVAR = get_GS_GLOBALVAR();

   //////////////////////
   // Riallineamento automatico
   if (pGS_GLOBALVAR->get_AutoSyncro() == GS_GOOD)
   {
      old_reactor_abilit = gsc_disable_reactors(); // disabilito i reattori di GEOsim
      // controllo se ci sono oggetti da riallineare
      AlteredEntities = gsc_align_data_base();
      // ripristino il controllo sul reattore come in precedenza
      if (old_reactor_abilit == GS_GOOD) gsc_enable_reactors();
   }
   // fine Riallineamento automatico
   ///////////////////////////

   //////////////////////
   // Modello di visualizzazione dinamico
   if (get_LAYER_DISPLAY_MODEL() && get_LAYER_DISPLAY_MODEL()->get_count() > 0)
   {
      static C_STRING LastLayerSetting = _T("");

      C_LAYER_SETTINGS* pLyrSetting = get_LAYER_DISPLAY_MODEL()->searchByZoomFactor();

      // Se è cambiato il settaggio dei layer
      if (pLyrSetting && 
            LastLayerSetting.comp(pLyrSetting->get_path(), FALSE) != 0)
         if (pLyrSetting->apply() == GS_GOOD)
            LastLayerSetting = pLyrSetting->get_path();
   }

	if (!pDockPaneWrkSession) return;    // Pannello non esistente

   //////////////////////
   // Estrazione dinamica
   if (pGS_GLOBALVAR->get_DynamicExtraction() == GS_GOOD)
   {
      C_RECT    CurrWindow;
      ads_point pt1, pt2;

      gsc_getCurrentScreenCoordinate(pt1, pt2);
      CurrWindow.BottomLeft.Set(pt1[X], pt1[Y]);
      CurrWindow.TopRight.Set(pt2[X], pt2[Y]);

      // Se la finestra precedente è stata inizializzata ed ora è cambiata
      if ((pDockPaneWrkSession->m_WrkSessionTab.m_WrkSessionClasses.LastExtractedWindow.Bottom() != 0 ||
           pDockPaneWrkSession->m_WrkSessionTab.m_WrkSessionClasses.LastExtractedWindow.Left() != 0 ||
           pDockPaneWrkSession->m_WrkSessionTab.m_WrkSessionClasses.LastExtractedWindow.Top() != 0 ||
           pDockPaneWrkSession->m_WrkSessionTab.m_WrkSessionClasses.LastExtractedWindow.Right() != 0) &&
          pDockPaneWrkSession->m_WrkSessionTab.m_WrkSessionClasses.LastExtractedWindow != CurrWindow)
      {
         C_INT_LIST ClassCodeList;
         C_INT      *pClassCode;
         C_CLASS    *pCls;

         // ricavo le classi selezionate per la prossima estrazione
         pCls = (C_CLASS *) get_GS_CURRENT_WRK_SESSION()->get_pPrj()->ptr_classlist()->get_head();
         while (pCls)
         { 
            // se la classe è da estrarre
            if (pCls->ptr_id()->sel == SELECTED || pCls->ptr_id()->sel == EXTR_SEL)
               if ((pClassCode = new C_INT(pCls->ptr_id()->code)) != NULL) 
                  ClassCodeList.add_tail(pClassCode);

            // classe successiva
            pCls = (C_CLASS *) pCls->get_next();
         }

         int ExtractionMode;
         ExtractionMode = (pDockPaneWrkSession->m_WrkSessionTab.m_WrkSessionClasses.m_Preview.GetCheck() == 1) ? PREVIEW : EXTRACTION;

         // zoom corrente
         pDockPaneWrkSession->m_WrkSessionTab.m_WrkSessionClasses.extract(ExtractionMode,
                                                                          1, // zoom corrente
                                                                          ClassCodeList);
      }
   }
   // fine estrazione dinamica
   ///////////////////////////

   /////////////////////////////////
   // Lettura DB eventuale PickFirst
   C_SELSET PickSet;
   
   // Se il pannello non è visibile oppure se è in stato "congelato"
   if (!pDockPaneWrkSession->IsWindowVisible() || 
       pDockPaneWrkSession->m_WrkSessionTab.m_EntDBValuesList.m_Frozen)
      return;

   int ItemCount = pDockPaneWrkSession->m_WrkSessionTab.GetItemCount();
   static C_SELSET PrevPickSet;

   // Se esistono oggetti appartenenti a classi GEOsim
   if (gsc_get_pick_first(PickSet) == GS_GOOD)
   {
      if (PrevPickSet != PickSet || AlteredEntities)
      {
         if (PickSet.length() > 0)
            pDockPaneWrkSession->DBQuery(PickSet);
         else
            pDockPaneWrkSession->GridDBQuery(NULL, NULL);

         PickSet.copy(PrevPickSet);
      }
   }
   else
   {
      // Cancello tutti i tab escluso il primo
      for (int i = ItemCount; i > 1; i--)
         pDockPaneWrkSession->m_WrkSessionTab.DeleteItem(i - 1);

      if (ItemCount > 1)
         pDockPaneWrkSession->m_WrkSessionTab.SetWrkSessionClassesItem();
   }
   // fine Lettura DB eventuale PickFirst
   //////////////////////////////////////
}

void GS_UI_InputContextReactor::endQuiescentState()
{  // MAP non più in stato tranquillo
	//if (!pDockPaneWrkSession) return;    // Pannello non esistente
 // 
 //  int ItemCount = pDockPaneWrkSession->m_WrkSessionTab.GetItemCount();

 //  // Cancello tutti i tab escluso il primo
 //  for (int i = ItemCount; i > 1; i--)
 //     pDockPaneWrkSession->m_WrkSessionTab.DeleteItem(i - 1);

 //  if (ItemCount > 1)
 //     pDockPaneWrkSession->m_WrkSessionTab.SetWrkSessionClassesItem();
}

// aggiungo reattore contestuale
void cmdAddInputContextReactor()
{
   curDoc()->inputPointManager()->addInputContextReactor(&GS_UI_INCONTXTREACTOR);
}

// rimuovo reattore contestuale
void cmdRemoveInputContextReactor()
{
   curDoc()->inputPointManager()->removeInputContextReactor(&GS_UI_INCONTXTREACTOR);
}


/*************************************************************************/
/*.doc gsui_WrkSessionPanel                                              */
/*+
  Crea il pannello della sessione di lavoro di GEOsim.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_WrkSessionPanel()
{
   resbuf   *arg = acedGetArgs();
   bool     Visible = TRUE;

   acedRetNil();

   if (arg && arg->restype == RTNIL) Visible = FALSE;
   if (gsc_WrkSessionPanel(Visible) == GS_BAD) return GS_BAD;

   // Se si deve forzare la lettura del pickset
   if ((arg = arg->rbnext) && arg->restype == RTT)
   {
      C_SELSET PickSet;
      // Se esistono oggetti appartenenti a classi GEOsim
      if (gsc_get_pick_first(PickSet) == GS_GOOD)
         pDockPaneWrkSession->DBQuery(PickSet);
   }

   acedRetT();
   return GS_GOOD;
}
int gsc_WrkSessionPanel(bool Visible)
{
   // se non esiste una sessione di lavoro corrente
   if (get_GS_CURRENT_WRK_SESSION() == NULL)
      { set_GS_ERR_COD(eGSNotCurrentSession); return GS_BAD; }

	if (pDockPaneWrkSession) // already open
   {
      if (!Visible) // nascondi
      {
         if (pDockPaneWrkSession->IsWindowVisible()) pDockPaneWrkSession->Hide();
      }
      else // rendi visibile
         if (!pDockPaneWrkSession->IsWindowVisible()) pDockPaneWrkSession->Show();

   	return GS_GOOD;
   }

   if (!Visible) return GS_GOOD; // nascondi

   // when resource from this ARX app is needed,
   // instantiate a local object of CAcModuleResourceOverride    
   CAcModuleResourceOverride resOverride;

	CFrameWnd *pMainFrame = acedGetAcadFrame();
	ASSERT(pMainFrame);

	pDockPaneWrkSession = new CDockPaneWrkSession;
	assert(pDockPaneWrkSession);

	if (!pDockPaneWrkSession->Create(pMainFrame)) return GS_BAD;

   // Set Focus to AutoCAD because AutoCAD doesn't update its
   // display if it's not in focus.
   acedGetAcadFrame()->SetActiveWindow();
   acedGetAcadFrame()->SetFocus();

   return GS_GOOD;
}


/*************************************************************************/
/*.doc int gsui_WriteExtrWindowList                                      */
/*+ 
   Funzione che scrive nel file CLASS.GEO della sessione di lavoro,
   le coordinate di una regione rettangolare in cui si
   è fatta l'estrazione dei dati di una certa classe di entità.
   Parametri:
   int cls;                   Codice classe
   C_RECT_LIST &WindowList;   Lista delle finestre di estrazione

   Ritorna GS_BAD in caso di errore, GS_GOOD altrimenti.
-*/
/*************************************************************************/
int gsui_WriteExtrWindowList(int cls, C_RECT_LIST &WindowList)
{
   FILE        *file;
   C_STRING    path, sez, Buffer, dummy;
   C_2STR_LIST WindowStrList;
   int         n = 1;
   C_RECT      *pWindow;
   bool        Unicode = false;

   // se non esiste una sessione di lavoro corrente
   if (get_GS_CURRENT_WRK_SESSION() == NULL) return GS_BAD;

   path = get_GS_CURRENT_WRK_SESSION()->get_dir();
   path += _T('\\');
   path += GEOTEMPSESSIONDIR;
   path += _T('\\');
   path += GS_CLASS_FILE;
   if ((file = gsc_open_profile(path, UPDATEABLE, MORETESTS, &Unicode)) == NULL) return GS_BAD;
 
   sez = cls;
   sez += _T(".0.EXTRACTED_WINDOWS");
   // leggo dal file CLASS.GEO (classi estratte nell'area di lavoro)
   // la lista delle finestre di estrazione già fatte per la classe
   if (gsc_get_profile(file, sez.get_name(), WindowStrList, Unicode) == GS_GOOD)
       n = WindowStrList.get_count() + 1;

   pWindow = (C_RECT *) WindowList.get_head();
   while (pWindow)
   {
      pWindow->toStr(Buffer);
      path = _T("WINDOW_");
      path += n++;

      if (gsc_set_profile(file, sez.get_name(), path.get_name(), Buffer.get_name(), Unicode) == GS_BAD)
         { gsc_fclose(file); return GS_BAD; }

      pWindow = (C_RECT *) WindowList.get_next();
   }

   return gsc_fclose(file);
}


/*************************************************************************/
/*.doc int gsui_getExtrWindowList                                        */
/*+ 
   Funzione che calcola quali sono le zone in cui bisogna estrarre i dati della 
   classe. Leggendo dal file CLASS.GEO della sessione di lavoro si possono 
   sapere quali sono state le zone rettangolari delle estrazioni precedenti.
   Ricevendo le coordinate di una regione rettangolare in cui si
   vuole fare l'estrazione dei dati di una certa classe di entità la funzione
   restituisce una o più finestre rappresentanti le zone in cui ancora non
   sono stati estratti i dati della classe.
   Parametri:
   int         cls;           Codice classe
   C_RECT      &Window;       Finestra di estrazione (in)
   C_RECT_LIST &WindowList;   Lista delle finestre di estrazione (out)

   Ritorna GS_BAD in caso di errore, GS_GOOD altrimenti.
-*/
/*************************************************************************/
int gsui_getExtrWindowList(int cls, C_RECT &Window, C_RECT_LIST &WindowList)
{
   FILE        *file;
   C_STRING    path, sez, Buffer, dummy;
   C_2STR_LIST WindowStrList;
   C_2STR      *pWindowStr;
   int         n = 1;
   C_RECT      *pWindow;
   C_RECT_LIST _WindowList;
   bool        Unicode = false;

   WindowList.remove_all();

   // se non esiste una sessione di lavoro corrente
   if (get_GS_CURRENT_WRK_SESSION() == NULL) return GS_BAD;

   path = get_GS_CURRENT_WRK_SESSION()->get_dir();
   path += _T('\\');
   path += GEOTEMPSESSIONDIR;
   path += _T('\\');
   path += GS_CLASS_FILE;
   if ((file = gsc_open_profile(path, READONLY, MORETESTS, &Unicode)) == NULL) return GS_BAD;
 
   sez = cls;
   sez += _T(".0.EXTRACTED_WINDOWS");
   // leggo dal file CLASS.GEO (classi estratte nell'area di lavoro)
   // la lista delle finestre di estrazione già fatte per la classe
   gsc_get_profile(file, sez.get_name(), WindowStrList, Unicode);
   if (gsc_fclose(file) == GS_BAD) return GS_BAD;

   pWindowStr = (C_2STR *) WindowStrList.get_head();
   while (pWindowStr)
   {
      if ((pWindow = new C_RECT(pWindowStr->get_name2())) == NULL) return GS_BAD;
      _WindowList.add_tail(pWindow);

      pWindowStr = (C_2STR *) WindowStrList.get_next();
   }

   Window.Not(_WindowList, WindowList);

   return GS_GOOD;
}