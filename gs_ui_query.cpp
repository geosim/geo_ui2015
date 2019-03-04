/**********************************************************
Name: GS_UI_QUERY
                                   
Module description: File funzioni per interrogazione e modifica
                    delle entità di GEOUI (GEOsim User Interface)
            
Author: Roberto Poltini

(c) Copyright 2002-2014 by IREN ACQUA GAS  S.p.A

**********************************************************/


/**********************************************************/
/*   INCLUDE                                              */
/**********************************************************/


// GS_UI_InputContextReactor methods
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include <rxmfcapi.h>         // ACAD MFC stuff
#include <aced.h>

#include "gs_def.h"       // definizioni globali
#include "gs_error.h" 
#include "gs_utily.h" 
#include "gs_resbf.h" 
#include "gs_init.h"      // per get_REFUSED_SS()
#include "gs_area.h" 
#include "gs_class.h" 
#include "gs_sec.h" 
#include "gs_selst.h"
#include "gs_graph.h" 
#include "gs_cmd.h"
#include "gs_netw.h"

#include "gs_ui_utily.h"
#include "ValuesListDlg.h"
#include "gs_ui_query.h"
#include "gs_ui_WrkSession.h"

const int BITMAP_WIDTH  = 16;
const int BITMAP_HEIGHT = 16;
const int HDR_BITMAP_WIDTH  = 8;
const int HDR_BITMAP_HEIGHT = 8;
const TCHAR MANDATORY_SYMBOL = _T('°');


#if defined(GSDEBUG) // se versione per debugging
   #include <sys/timeb.h>  // Solo per debug
   #include <time.h>       // Solo per debug
   double  tempo=0, tempo1=0, tempo2=0, tempo3=0, tempo4=0, tempo5=0, tempo6=0, tempo7=0;
   double  tempo8=0, tempo9=0, tempo10=0, tempo11=0, tempo12=0, tempo13=0, tempo14=0;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMyEdit
/////////////////////////////////////////////////////////////////////////////
MyCListCtrl::MyCListCtrl()
{
}

MyCListCtrl::~MyCListCtrl()
{
}

BOOL MyCListCtrl::OnWndMsg( UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
   if (GetParent() && (message == WM_HSCROLL || message == WM_VSCROLL))
      ((CEntDBValuesListDlg*) GetParent())->HideEditCtrls();  

	return CListCtrl::OnWndMsg(message, wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////////////////////
// MyCListCtrl - fine
// CEntDBValuesListDlg property page - inizio
/////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNCREATE(CEntDBValuesListDlg, CPropertyPage)

CEntDBValuesListDlg::CEntDBValuesListDlg() : CPropertyPage(CEntDBValuesListDlg::IDD)
{
   C_INIT *pGS_GLOBALVAR = get_GS_GLOBALVAR();

   MotherColValues.remove_all();
   pActualSec     = NULL;
   pActualCls     = NULL;
   OperationMode  = MODIFY;
   ActualItem     = 0;
   ActualSubItem  = 0;
   TempItemValueChanged = FALSE;
   ComboButtonVis = EditVis = MaskedEditVis = FALSE;
   hBmpButtonSupport = NULL;
   hBmpSuggest       = NULL;
   hBmpDBList        = NULL;
   hBmpMathOp        = NULL;
   hBmpPrinter       = NULL;
   pToolTip          = NULL;
   hBmpVertMode      = NULL;
   hBmpHorizMode     = NULL;
   hBmpSelObjs       = NULL;
   hBmpSecTab        = NULL;
   hBmpAddSec        = NULL;
   hBmpDelSec        = NULL;
   m_VertAttrMode    = TRUE; // singola scheda

   m_Frozen = false;

   pt1[X] = pt1[Y] = pt1[Z] = pt2[X] = pt2[Y] = pt2[Z] = 0.0;

   if (pGS_GLOBALVAR)
   {
      m_Zoom = (pGS_GLOBALVAR->get_AutoZoom() == GS_GOOD) ? TRUE : FALSE;
      m_Highlight = (pGS_GLOBALVAR->get_AutoHighlight() == GS_GOOD) ? TRUE : FALSE;
   }

   isForClass   = TRUE;

   Prev_iHeader = -1;

   pChacheAttribValues = NULL;

	//{{AFX_DATA_INIT(CEntDBValuesListDlg)
	//}}AFX_DATA_INIT
}

CEntDBValuesListDlg::~CEntDBValuesListDlg()
{
   if (hBmpButtonSupport) DeleteObject((HGDIOBJ) hBmpButtonSupport);
   if (hBmpSuggest) DeleteObject((HGDIOBJ) hBmpSuggest);
   if (hBmpDBList) DeleteObject((HGDIOBJ) hBmpDBList);
   if (hBmpMathOp) DeleteObject((HGDIOBJ) hBmpMathOp);
   if (hBmpPrinter) DeleteObject((HGDIOBJ) hBmpPrinter);
   if (hBmpVertMode) DeleteObject((HGDIOBJ) hBmpVertMode);
   if (hBmpHorizMode) DeleteObject((HGDIOBJ) hBmpHorizMode);
   if (hBmpSelObjs) DeleteObject((HGDIOBJ) hBmpSelObjs);
   if (hBmpSecTab) DeleteObject((HGDIOBJ) hBmpSecTab);
   if (hBmpAddSec) DeleteObject((HGDIOBJ) hBmpAddSec);
   if (hBmpDelSec) DeleteObject((HGDIOBJ) hBmpDelSec);
   
   if (pToolTip) delete pToolTip;
}

void CEntDBValuesListDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEntDBValuesListDlg)
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_OK, m_OK);
	DDX_Control(pDX, IDC_COMBO_BUTTON, m_ComboButton);
	DDX_Control(pDX, IDC_EDIT, m_Edit);
	DDX_Control(pDX, IDC_MASKEDBOX, m_MaskedEdit);
	DDX_Control(pDX, IDC_CLASSLIST_COMBO, m_ClassAndSecListCombo);
	DDX_Control(pDX, IDC_REM, m_Remark);
	DDX_Control(pDX, IDC_ENTDBVALUES_LIST, m_EntDBValuesList);
	DDX_Control(pDX, IDC_VERTICAL_MODE, m_VertAttrModeButton);
	DDX_Control(pDX, IDC_GRAPH_SELECTION, m_GraphSel);
	DDX_Control(pDX, IDC_SECONDARY_TAB, m_SecTabButton);
	DDX_Control(pDX, IDC_ADD_SEC, m_AddSecButton);
	DDX_Control(pDX, IDC_DEL_SEC, m_DelSecButton);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEntDBValuesListDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CEntDBValuesListDlg)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_CLASSLIST_COMBO, OnSelchangeClasslistCombo)
	ON_EN_CHANGE(IDC_EDIT, OnChangeEdit)
	ON_EN_CHANGE(IDC_MASKEDBOX, OnChangeMaskedbox)
   ON_EN_KILLFOCUS(IDC_MASKEDBOX, OnEnKillfocusMaskedbox)
	ON_NOTIFY(NM_CLICK, IDC_ENTDBVALUES_LIST, OnClickEntdbvaluesList)
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_NOTIFY(LVN_KEYDOWN, IDC_ENTDBVALUES_LIST, OnKeydownEntdbvaluesList)
	ON_BN_CLICKED(IDC_COMBO_BUTTON, OnComboButton)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	ON_NOTIFY(NM_KILLFOCUS, IDC_LIST, OnKillfocusList)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_RCLICK, IDC_ENTDBVALUES_LIST, OnRclickEntdbvaluesList)
	ON_COMMAND(IDC_DB_LIST, OnDBList)
	ON_COMMAND(IDC_SUGGEST, OnSuggest)  
	ON_COMMAND(IDC_MATH_OP, OnMathOp)
	ON_COMMAND(IDC_UNDO, OnEditUndo)  
	ON_COMMAND(IDC_COPY, OnEditCopy)  
	ON_COMMAND(IDC_COPY_SHEET, OnEditCopySheet)  
	ON_COMMAND(IDC_CUT, OnEditCut)  
	ON_COMMAND(IDC_PASTE, OnEditPaste)  
	ON_COMMAND(IDC_REMOVE, OnEditRemove)  
	ON_COMMAND(IDC_SELECT_ALL, OnEditSelectAll)  
	ON_COMMAND(IDC_PRINT_SHEET, OnPrintSheet)  
	ON_COMMAND(IDC_PRINT_SHEETS, OnPrintSheets)  
	ON_COMMAND(IDC_ZOOM, OnZoom)  
	ON_COMMAND(IDC_HIGHLIGHT, OnHighlight)  
	ON_COMMAND(IDC_FROZEN_PANEL, OnFrozenPanel)  
	ON_NOTIFY(NM_DBLCLK, IDC_ENTDBVALUES_LIST, OnDblclkEntdbvaluesList)
   ON_BN_CLICKED(IDC_VERTICAL_MODE, OnBnClickedVerticalMode)
   ON_NOTIFY(LVN_COLUMNCLICK, IDC_ENTDBVALUES_LIST, OnLvnColumnclickEntdbvaluesList)
   ON_NOTIFY(HDN_TRACK, 0, OnHdnTrackEntdbvaluesList)
   ON_BN_CLICKED(IDC_GRAPH_SELECTION, OnBnClickedGraphSelection)
   ON_BN_CLICKED(IDC_SECONDARY_TAB, OnBnClickedSecondaryTab)
   ON_BN_CLICKED(IDC_ADD_SEC, OnBnClickedAddSec)
   ON_BN_CLICKED(IDC_DEL_SEC, OnBnClickedDelSec)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEntDBValuesListDlg message handlers


void CEntDBValuesListDlg::OnSize(UINT nType, int cx, int cy) 
{
   CRect rect;
   int   Margin = 5;
   int   Top, Height, OffSet = 5, TopEntDBValuesList, LeftPos;
   int   m_OKWidth, m_VertAttrModeButtonWidth, m_SecTabButtonWidth, m_GraphSelButtonWidth;

   CPropertyPage::OnSize(nType, cx, cy);

   m_GraphSel.GetWindowRect(&rect);
   m_GraphSelButtonWidth = rect.Width();

   //	m_ClassAndSecListCombo;
   m_ClassAndSecListCombo.GetWindowRect(&rect);
   if (isForClass)
   {
      m_ClassAndSecListCombo.SetWindowPos(NULL, Margin, Margin,
                                          cx - (2 * Margin) - m_GraphSelButtonWidth - OffSet,
                                          rect.Height(),
                                          SWP_NOZORDER);

      m_GraphSel.SetWindowPos(NULL, cx - m_GraphSelButtonWidth - Margin, Margin,
                              0, 0,
                              SWP_NOSIZE | SWP_NOZORDER);
   }
   else
      m_ClassAndSecListCombo.SetWindowPos(NULL, Margin, Margin,
                                          cx - (2 * Margin),
                                          rect.Height(),
                                          SWP_NOZORDER);

   TopEntDBValuesList = Margin + rect.Height() + OffSet;

   m_VertAttrModeButton.GetWindowRect(&rect);
   m_VertAttrModeButtonWidth = rect.Width();
   Height = rect.Height();

   m_OK.GetWindowRect(&rect);
   m_OKWidth = rect.Width();

   m_Remark.GetWindowRect(&rect);
   Top = cy - rect.Height() - Margin;
   if (Top <= 0) Top = 1;

   // m_VertAttrModeButton
   m_VertAttrModeButton.SetWindowPos(NULL, Margin, Top,
                                     m_VertAttrModeButtonWidth, Height, SWP_NOZORDER);
   LeftPos = Margin + m_VertAttrModeButtonWidth + OffSet;

   // m_SecTabButton & m_AddSecButton & m_DelSecButton
   if (isForClass)
   {
      m_SecTabButton.GetWindowRect(&rect);
      m_SecTabButtonWidth = rect.Width();
      m_SecTabButton.SetWindowPos(NULL, LeftPos, Top,
                                 m_SecTabButtonWidth, Height, SWP_NOZORDER);
      LeftPos += m_SecTabButtonWidth + OffSet;
   }
   else
   {
      m_AddSecButton.GetWindowRect(&rect);
      m_SecTabButtonWidth = rect.Width();
      m_AddSecButton.SetWindowPos(NULL, LeftPos, Top,
                                  m_SecTabButtonWidth, Height, SWP_NOZORDER);
      LeftPos += m_SecTabButtonWidth + OffSet;
      m_DelSecButton.SetWindowPos(NULL, LeftPos, Top,
                                  m_SecTabButtonWidth, Height, SWP_NOZORDER);
      LeftPos += m_SecTabButtonWidth + OffSet;
   }

   // m_Remark
   m_Remark.SetWindowPos(NULL, LeftPos, Top, 
                         cx - (m_OKWidth + OffSet + Margin) - LeftPos,
                         Height, SWP_NOZORDER);

   // m_OK
   m_OK.SetWindowPos(NULL, cx - m_OKWidth - Margin, Top, m_OKWidth, Height, SWP_NOZORDER);

   // m_EntDBValuesList
   m_EntDBValuesList.SetWindowPos(NULL, Margin, TopEntDBValuesList, 
                                  cx - (2 * Margin), 
                                  (Top - OffSet - Margin) - TopEntDBValuesList, SWP_NOZORDER);

   if (m_VertAttrMode) // singola scheda
      // Dimensionamento colonne EntDBValuesList
      EntDBValuesListColumnSizing(cx - (2 * Margin), cy);

   HideEditCtrls();

   InitTooltip();
}


/***************************************/
// Dimensionamento colonne EntDBValuesList
/***************************************/
void CEntDBValuesListDlg::EntDBValuesListColumnSizing(int cx, int cy) 
{
   if (m_VertAttrMode) // disposizione degli attributi in verticale
      EntDBValuesListVertColumnSizing(cx, cy);
   else // disposizione degli attributi in orizzontale
      EntDBValuesListHorizColumnSizing(cx, cy);
}
/***************************************/
// Posizionamento controlli EntDBValuesList
/***************************************/
void CEntDBValuesListDlg::SetPosEntDBValuesListEditCtrls(void)
{
   if (m_VertAttrMode) // disposizione degli attributi in verticale
      SetPosEntDBValuesListVertEditCtrls();
   else // disposizione degli attributi in orizzontale
      SetPosEntDBValuesListHorizEditCtrls();
}
/***************************************/
// Dimensionamento colonne EntDBValuesList in modalità verticale
/***************************************/
void CEntDBValuesListDlg::EntDBValuesListVertColumnSizing(int cx, int cy) 
{
   // Dimensionamento colonne
   if (m_EntDBValuesList.GetItemCount() == 0)
   {
      m_EntDBValuesList.SetColumnWidth(0, 0);
      m_EntDBValuesList.SetColumnWidth(1, 0);
   }
   else
   {
      CString ItemText;
      int     Width1Column, MaxWidth1Column = BITMAP_WIDTH, Width2Column;
      int     VirtualCtrlWidth = cx - 10, i;

      // Calcolo la larghezza delle colonna "Nome"
      for (i = 0; i < m_EntDBValuesList.GetItemCount(); i++)
      {
         ItemText = m_EntDBValuesList.GetItemText(i, 0);
         Width1Column = m_EntDBValuesList.GetStringWidth((LPCTSTR) ItemText) + BITMAP_WIDTH + 15;
         if (MaxWidth1Column < Width1Column) MaxWidth1Column = Width1Column;
      }

      m_EntDBValuesList.SetColumnWidth(0, MaxWidth1Column);
      Width2Column = VirtualCtrlWidth - MaxWidth1Column;
      // Se è comparsa la barra di scorrimento verticale
      if (m_EntDBValuesList.GetItemCount() > m_EntDBValuesList.GetCountPerPage())
         Width2Column -= 16;
      m_EntDBValuesList.SetColumnWidth(1, Width2Column);

      CRect rect;
      if (ActualItem >= 0 && m_EntDBValuesList.GetSubItemRect(ActualItem, 1, LVIR_BOUNDS, rect))
      {
         m_ComboButton.SetWindowPos(&m_EntDBValuesList,
                                    rect.left + rect.Width() - rect.Height(),
                                    rect.top, 
                                    rect.Height(), rect.Height(), SWP_NOZORDER);

         int EditWidth = rect.Width(), OffSetX = 1, OffSetY = 0;

         if (ComboButtonVis) EditWidth -= rect.Height();

         m_Edit.SetWindowPos(&m_EntDBValuesList,
                             rect.left + OffSetX, rect.top + OffSetY, 
                             EditWidth - OffSetX, rect.Height() - OffSetY - 1, SWP_NOZORDER);

         m_MaskedEdit.SetWindowPos(&m_EntDBValuesList,
                                   rect.left + OffSetX, rect.top + OffSetY, 
                                   EditWidth - OffSetX, rect.Height() - OffSetY - 1, SWP_NOZORDER);
      }
   }
}
// Posizionamento controlli in modalità verticale
void CEntDBValuesListDlg::SetPosEntDBValuesListVertEditCtrls(void) 
{
   CRect rect;

   if (ActualItem >= 0 && m_EntDBValuesList.GetSubItemRect(ActualItem, 1, LVIR_BOUNDS, rect))
   {
      m_ComboButton.SetWindowPos(&m_EntDBValuesList,
                                 rect.left + rect.Width() - rect.Height(),
                                 rect.top, 
                                 rect.Height(), rect.Height(), SWP_NOZORDER);

      int EditWidth = rect.Width(), OffSetX = 1, OffSetY = 0;

      if (ComboButtonVis) EditWidth -= rect.Height();

      m_Edit.SetWindowPos(&m_EntDBValuesList,
                          rect.left + OffSetX, rect.top + OffSetY, 
                          EditWidth - OffSetX, rect.Height() - OffSetY - 1, SWP_NOZORDER);

      m_MaskedEdit.SetWindowPos(&m_EntDBValuesList,
                                rect.left + OffSetX, rect.top + OffSetY, 
                                EditWidth - OffSetX, rect.Height() - OffSetY - 1, SWP_NOZORDER);
   }
}


/***************************************/
// Dimensionamento colonne EntDBValuesList in modalità orizzontale
/***************************************/
void CEntDBValuesListDlg::EntDBValuesListHorizColumnSizing(int cx, int cy) 
{
   // Dimensionamento colonne
   LVCOLUMN col;
   int      i = 0, OffSet = 8;
   TCHAR    Txt[256];

   col.mask       = LVCF_TEXT;
   col.pszText    = Txt;
   col.cchTextMax = 256;
   while (m_EntDBValuesList.GetColumn(i, &col))
   {
      col.cx = m_EntDBValuesList.GetStringWidth((LPCTSTR) col.pszText);
      col.cx += (OffSet + (2 * BITMAP_WIDTH));
      col.mask = LVCF_WIDTH;
      m_EntDBValuesList.SetColumn(i, &col);
      col.mask   = LVCF_TEXT;
      i++;
   }
}


/***************************************/
// Posizionamento controlli in modalità orizzontale
/***************************************/
void CEntDBValuesListDlg::SetPosEntDBValuesListHorizEditCtrls(void) 
{
   // Dimensionamento colonne
   int   OffSet = 8, LimitX, XMin, ComboWidth;
   CRect rect;

   m_EntDBValuesList.GetClientRect(&rect);
   LimitX = rect.right;

   if (ActualItem >= 0 && ActualSubItem >= 0 && 
       m_EntDBValuesList.GetSubItemRect(ActualItem, ActualSubItem, LVIR_BOUNDS, rect))
   {
      int EditWidth, OffSetX = 1;

      if (LimitX < rect.right) rect.right = LimitX;

      XMin = rect.right - rect.Height();
      if (XMin < rect.left) XMin = rect.left;
      ComboWidth = rect.right - XMin;

      m_ComboButton.SetWindowPos(&m_EntDBValuesList,
                                 XMin,
                                 rect.top, 
                                 ComboWidth,
                                 rect.Height(), SWP_NOZORDER);

      if (ComboButtonVis)
         rect.right -= ComboWidth; // tolgo lo spazio per il bottone

      EditWidth = rect.Width() - OffSetX;

      m_Edit.SetWindowPos(&m_EntDBValuesList,
                          rect.left + OffSetX, rect.top, 
                          EditWidth,
                          rect.Height() - 1,
                          SWP_NOZORDER);

      m_MaskedEdit.SetWindowPos(&m_EntDBValuesList,
                                rect.left + OffSetX, rect.top, 
                                EditWidth, 
                                rect.Height() - 1,
                                SWP_NOZORDER);
   }
}

BOOL CEntDBValuesListDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

   InitList();

	// TODO: Add extra initialization here
   COLORREF  crFrom;
	HINSTANCE Instance;

   InitTooltip();

   crFrom = RGB(255, 0, 0); // rosso

   // determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_BUTTON_SUPPORT), RT_BITMAP);
   
   hBmpSuggest = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SUGGEST));
   hBmpDBList  = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_DBVALUES));
   hBmpMathOp  = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_MATHOP));
   hBmpPrinter = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_PRINTER));

   if (IsForClass()) // Se si stanno gestendo entità di classi GEOsim
   {
      hBmpSelObjs = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SELECT_OBJS));
      gsui_SetBmpColorToDlgBkColor(hBmpSelObjs, crFrom);
      m_GraphSel.SetBitmap(hBmpSelObjs);
      m_GraphSel.ShowWindow(SW_SHOW);

      hBmpSecTab = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_SECONDARY_TAB));
      gsui_SetBmpColorToDlgBkColor(hBmpSecTab, crFrom);
      m_SecTabButton.SetBitmap(hBmpSecTab);
      m_SecTabButton.ShowWindow(SW_SHOW);

      m_AddSecButton.ShowWindow(SW_HIDE);
      m_DelSecButton.ShowWindow(SW_HIDE);
   }
   else
   {
      m_GraphSel.ShowWindow(SW_HIDE);
      m_SecTabButton.ShowWindow(SW_HIDE);

      hBmpAddSec = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_ADD_RECORD));
      gsui_SetBmpColorToDlgBkColor(hBmpAddSec, crFrom);
      m_AddSecButton.SetBitmap(hBmpAddSec);
      m_AddSecButton.ShowWindow(SW_SHOW);

      hBmpDelSec = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_DEL_RECORD));
      gsui_SetBmpColorToDlgBkColor(hBmpDelSec, crFrom);
      m_DelSecButton.SetBitmap(hBmpDelSec);
      m_DelSecButton.ShowWindow(SW_SHOW);
   }

   hBmpButtonSupport = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_BUTTON_SUPPORT));  
   gsui_SetBmpColorToDlgBkColor(hBmpButtonSupport, crFrom);
   m_ComboButton.SetBitmap(hBmpButtonSupport);
   m_ComboButton.SetParent(&m_EntDBValuesList);

   hBmpVertMode = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_VERT_DBATTR));  
   gsui_SetBmpColorToDlgBkColor(hBmpVertMode, crFrom);
   hBmpHorizMode = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_HORIZ_DBATTR));  
   gsui_SetBmpColorToDlgBkColor(hBmpHorizMode, crFrom);
	m_EntDBValuesList.SetExtendedStyle(m_EntDBValuesList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   DisplayRecords(); // visualizza i dati letti
   ZoomHighlightEnt();

   m_Edit.SetParent(&m_EntDBValuesList);
   m_MaskedEdit.SetParent(&m_EntDBValuesList);

   m_List.ModifyStyle(0L, WS_BORDER); // con bordino
   m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
   m_List.SetParent(&m_EntDBValuesList);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEntDBValuesListDlg::InitList(void)
{
   CBitmap CBMP;
	CRect   rect;

   // creazione lista vuota
   m_ImageList.Create(BITMAP_WIDTH, BITMAP_HEIGHT,
                      ILC_MASK | ILC_COLORDDB, 
                      2, // n. bitmap
                      0);

   // bitmap vuota
   CBMP.LoadBitmap(MAKEINTRESOURCE(IDB_BLANK));
   m_ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente
   // bitmap simbolo "lampadina accesa"
   CBMP.LoadBitmap(MAKEINTRESOURCE(IDB_VISIBLE));
   m_ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente

   m_EntDBValuesList.SetImageList(&m_ImageList, LVSIL_SMALL);

   // Intestazione 
   CHeaderCtrl *pHdrCtrl= NULL;

   // creazione lista vuota
   m_ImageHdrList.Create(2 * HDR_BITMAP_WIDTH, HDR_BITMAP_HEIGHT,
                         ILC_MASK | ILC_COLORDDB, 
                         5, // n. bitmap
                         0);
   // bitmap simbolo "ordine crescente"
   CBMP.LoadBitmap(MAKEINTRESOURCE(IDB_8x16_ASCENDING_ORDER));
   m_ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente
   // bitmap simbolo "ordine decrescente"
   CBMP.LoadBitmap(MAKEINTRESOURCE(IDB_8x16_DESCENDING_ORDER));
   m_ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente
   // bitmap simbolo "lampadina accesa"
   CBMP.LoadBitmap(MAKEINTRESOURCE(IDB_8x16_VISIBLE));
   m_ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente
   // bitmap simbolo "lampadina accesa" + "ordine crescente"
   CBMP.LoadBitmap(MAKEINTRESOURCE(IDB_8x16_VISIBLE_ASCENDING_ORDER));
   m_ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente
   // bitmap simbolo "lampadina accesa" + "ordine decrescente"
   CBMP.LoadBitmap(MAKEINTRESOURCE(IDB_8x16_VISIBLE_DESCENDING_ORDER));
   m_ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente

   pHdrCtrl = m_EntDBValuesList.GetHeaderCtrl();
	if (pHdrCtrl) pHdrCtrl->SetImageList(&m_ImageHdrList);
}


/*********************************************************/
/*.doc CEntDBValuesListDlg::RefreshCls       <external> */
/*+                                                                       
  La funzione visualizza le informazioni contenute nel DB associato 
  alle entità passate come parametro.
  Parametri:
  C_SELSET &_SelSet;

  Restituisce il numero di classi e sottoclassi GEOsim presenti nel gruppo di selezione
  in caso di successo altrimenti restituisce -1. 
-*/  
/*********************************************************/
int CEntDBValuesListDlg::RefreshCls(C_SELSET &_SelSet)
{
   C_INT_INT_LIST  ClsSubList;
   C_INT_INT       *pClsSub;
   C_CLASS         *pCls;
   C_STRING        ClassName;
   C_CLS_PUNT_LIST ExtractedClsList;
   C_CLS_PUNT      *pExtractedCls;

   KeyList.remove_all();
   pActualCls = NULL;
   pActualSec = NULL;

   // disabilito l'interrogazione per zona (solo per griglie)
   pt1[X] = pt1[Y] = pt1[Z] = 0.0;
   pt2[X] = pt2[Y] = pt2[Z] = 0.0;

   // Verifico le classi degli oggetti grafici
   _SelSet.copy(SelSet);

   if (SelSet.get_ClsCodeList(ClsSubList) == GS_BAD) return -1;

   if (ClsSubList.get_count() == 0)
   {
      Clear();
      return 0;
   }

   // Lista delle classi estratte
   if (get_GS_CURRENT_WRK_SESSION()->get_pPrj()->extracted_class(ExtractedClsList) == GS_BAD)
      return -1;

   // Riempio la combo con i nomi delle classi
   m_ClassAndSecListCombo.ResetContent();
   pClsSub = (C_INT_INT *) ClsSubList.get_head();

   while (pClsSub)
   {
      if ((pCls = (C_CLASS *) get_GS_CURRENT_WRK_SESSION()->get_pPrj()->find_class(pClsSub->get_key(),
                                                                                   pClsSub->get_type())))
      {
         pCls->get_CompleteName(ClassName);
         m_ClassAndSecListCombo.AddString(ClassName.get_name());
         m_ClassAndSecListCombo.SetItemDataPtr(m_ClassAndSecListCombo.FindStringExact(-1, ClassName.get_name()), pCls);
         
         // se l'entità potenzialmente appartiene a complesse o a gruppi
         pExtractedCls = (C_CLS_PUNT *) ExtractedClsList.get_head();
         while (pExtractedCls)
         {
            // salto la classe attuale
            if (((C_CLASS *) pExtractedCls->cls)->ptr_id()->code != pCls->ptr_id()->code)
               if (((C_CLASS *) pExtractedCls->cls)->is_member_class(pCls->ptr_id()->code) == GS_GOOD)
               {
                  ((C_CLASS *) pExtractedCls->cls)->get_CompleteName(ClassName);

                  // Se non è stato ancora inserito il nome di questa classe
                  if (m_ClassAndSecListCombo.FindStringExact(-1, ClassName.get_name()) == CB_ERR)
                  {
                     m_ClassAndSecListCombo.AddString(ClassName.get_name());
                     m_ClassAndSecListCombo.SetItemDataPtr(m_ClassAndSecListCombo.FindStringExact(-1, ClassName.get_name()),
                                                           pExtractedCls->cls);
                  }
               }

	         pExtractedCls = (C_CLS_PUNT *) pExtractedCls->get_next();
         }
      }

      pClsSub = (C_INT_INT *) ClsSubList.get_next();
   }

   return ClsSubList.get_count();
}


/*********************************************************/
/*.doc CEntDBValuesListDlg::RefreshSec       <external> */
/*+                                                                       
  La funzione visualizza le informazioni contenute nel DB associato 
  alla tabella secondaria collegata all'entità passata come parametro.
  Parametri:
  C_CLASS *_pCls;    classe madre

  Restituisce il numero di tabelle secondarie associate all'entità
  in caso di successo altrimenti restituisce -1. 
-*/  
/*********************************************************/
int CEntDBValuesListDlg::RefreshSec(C_CLASS *_pCls, C_RB_LIST &_MotherColValues)
{
   C_SINTH_SEC_TAB_LIST SinthSecList;
   C_SINTH_SEC_TAB      *pSinthSec;
   C_SECONDARY          *pSec;

   _MotherColValues.copy(MotherColValues);
   KeyList.remove_all();
   pActualCls = NULL;
   pActualSec = NULL;

   // Verifico se la classe scelta ha delle tab. secondarie
   if (_pCls->get_pPrj()->getSinthClsSecondaryTabList(_pCls->ptr_id()->code, _pCls->ptr_id()->sub_code, SinthSecList) == GS_BAD)
      return -1;
      
   if (SinthSecList.get_count() == 0)
   {
      Clear();
      return 0;
   }

   // Riempio la combo con i nomi delle tabelle secondarie
   m_ClassAndSecListCombo.ResetContent();
   pSinthSec = (C_SINTH_SEC_TAB *) SinthSecList.get_head();

   while (pSinthSec)
   {
      if ((pSec = (C_SECONDARY *) _pCls->find_sec(pSinthSec->get_key())))
      {
         m_ClassAndSecListCombo.AddString(pSec->get_name());
         m_ClassAndSecListCombo.SetItemDataPtr(m_ClassAndSecListCombo.FindStringExact(-1, pSec->get_name()), pSec);
      }
      pSinthSec = (C_SINTH_SEC_TAB *) SinthSecList.get_next();
   }

   return SinthSecList.get_count();
}


/*********************************************************/
/*.doc CEntDBValuesListDlg::RefreshGridCls    <external> */
/*+                                                                       
  La funzione visualizza le informazioni contenute nel DB associato 
  alle entità griglia che intersecano un rettangolo o 
  le cui celle contengono un punto dato.
  Parametri:
  ads_point _pt1;   Primo punto (se = NULL utilizza i punti precedentemente
                    selezionati)
  ads_point _pt2;   Opzionale, se = NULL si devono cercare le celle
                    che contengono il punto pt1 altrimenti
                    si devono cercare le celle che intersecano
                    il rettangolo formato da pt1 (angolo basso-sinistra)
                    e pt2 (angolo alto-destra).
  oppure
  C_CGRID      *_pCls;    Puntatore a classe griglia
  C_LONG_BTREE &_KeyList; Lista dei codici delle celle

  Restituisce il numero di classi e sottoclassi GEOsim presenti nel gruppo di selezione
  in caso di successo altrimenti restituisce -1. 
-*/  
/*********************************************************/
int CEntDBValuesListDlg::RefreshGridCls(ads_point _pt1, ads_point _pt2)
{
   C_INT_LIST ClsCodeList;
   C_INT      *pClsCode;
   C_CLASS    *pCls;
   C_STRING   ClassName;
   C_RECT     Zone;
   double     dummy;

   KeyList.remove_all();
   pActualCls = NULL;
   pActualSec = NULL;

   // disabilito l'interrogazione per gruppo di selezione
   SelSet.clear();

   if (!_pt1)
   {
      if (!IsDBGridMode())
      {
         Clear();
         return 0;
      }
   }
   else
      ads_point_set(_pt1, pt1);

   if (_pt2) ads_point_set(_pt2, pt2);
   else if (_pt1) ads_point_set(_pt1, pt2);

   if (pt1[X] > pt2[X]) { dummy = pt1[X]; pt1[X] = pt2[X]; pt2[X] = dummy; }
   if (pt1[Y] > pt2[Y]) { dummy = pt1[Y]; pt1[Y] = pt2[Y]; pt2[Y] = dummy; }

   // verifico se tra le classi estratte vi sono delle classi tipo griglia
   // che intersecano la zona
   Zone.BottomLeft.Set(pt1[X], pt1[Y]);
   if (pt2) Zone.TopRight.Set(pt2[X], pt2[Y]);
   else Zone.TopRight.Set(pt1[X], pt1[Y]);

   if (Zone.get_ExtractedGridClsCodeList(ClsCodeList) == GS_BAD) return -1;

   if (ClsCodeList.get_count() == 0)
   {
      Clear();
      return 0;
   }

   // Riempio la combo con i nomi delle classi
   m_ClassAndSecListCombo.ResetContent();
   pClsCode = (C_INT *) ClsCodeList.get_head();

   while (pClsCode)
   {
      if ((pCls = (C_CLASS *) get_GS_CURRENT_WRK_SESSION()->get_pPrj()->find_class(pClsCode->get_key())))
      {
         pCls->get_CompleteName(ClassName);
         m_ClassAndSecListCombo.AddString(ClassName.get_name());
         m_ClassAndSecListCombo.SetItemDataPtr(m_ClassAndSecListCombo.FindStringExact(-1, ClassName.get_name()), pCls);
      }         

      pClsCode = (C_INT *) ClsCodeList.get_next();
   }

   return ClsCodeList.get_count();
}
int CEntDBValuesListDlg::RefreshGridCls(C_CGRID *_pCls, C_LONG_BTREE &_KeyList)
{
   C_STRING ClassName;

   // se pt1 = al minimo reale consentito (std::numeric_limits<double>::min)();
   // e pt1 = pt2 si sta interrogando una grigla i cui i codici sono in KeyList
   pt1[X] = pt1[Y] = pt1[Z] = (std::numeric_limits<double>::min)();
   ads_point_set(pt1, pt2);

   KeyList.remove_all();
   KeyList.add_list(_KeyList);

   pActualCls = _pCls;
   pActualSec = NULL;

   // disabilito l'interrogazione per gruppo di selezione
   SelSet.clear();

   if (_KeyList.get_count() == 0 || _pCls == NULL)
      if (!IsDBGridMode())
      {
         Clear();
         return 0;
      }

   // Riempio la combo con i nomi delle classi
   m_ClassAndSecListCombo.ResetContent();

   _pCls->get_CompleteName(ClassName);
   m_ClassAndSecListCombo.AddString(ClassName.get_name());
   m_ClassAndSecListCombo.SetItemDataPtr(m_ClassAndSecListCombo.FindStringExact(-1, ClassName.get_name()), _pCls);

   return 1;
}


/*********************************************************/
/*.doc CEntDBValuesListDlg::Clear            <external> */
/*+                                                                       
  La funzione pulisce le strutture della classe.
-*/  
/*********************************************************/
void CEntDBValuesListDlg::Clear(void)
{
   m_ClassAndSecListCombo.ResetContent();
   m_ClassAndSecListCombo.AddString(_T("Nessuna selezione"));

   m_EntDBValuesList.DeleteAllItems();
   while (m_EntDBValuesList.DeleteColumn(0) != 0); // cancello le colonne

   SelSet.clear();
   MotherColValues.remove_all();
   pActualCls = NULL;
   pActualSec = NULL;
   KeyList.remove_all();
}

/****************************************************************/
/*.doc CEntDBValuesListDlg::IsForClass               <external> */
/*+                                                                       
  La funzione ritorna TRUE se l'impostazione è per classi di entità 
  di GEOsim. Ritorna FALSE se l'impostazione è per tabelle secondarie.
-*/  
/****************************************************************/
BOOL CEntDBValuesListDlg::IsForClass()
{
   return (isForClass) ? TRUE : FALSE;
}


/*************************************************************************/
/*.doc CEntDBValuesListDlg::IsDBGridMode                                 */
/*+ 
   Funzione che verifica se si stanno interrogando dei dati griglia.

   Ritorna true in caso affermativo, false altrimenti.
-*/
/*************************************************************************/
bool CEntDBValuesListDlg::IsDBGridMode(void)
{
   // se pt1 = pt2 = 0,0,0 non si stanno interrogando griglie
   return (pt1[X] == 0 && pt1[Y] == 0 && pt1[Z] == 0 &&
           ads_point_equal(pt1, pt2)) ? false : true;
}


/*************************************************************************/
/*.doc CEntDBValuesListDlg::IsDBGridFromWindow                           */
/*+ 
   Funzione che verifica se si stanno interrogando dei dati griglia delle
   celle interne ad una finestra.

   Ritorna true in caso affermativo, false altrimenti.
-*/
/*************************************************************************/
bool CEntDBValuesListDlg::IsDBGridFromWindow(void)
{
   // se pt1 = al minimo reale consentito (std::numeric_limits<double>::min)());
   // e pt1 = pt2 si sta interrogando una grigla i cui i codici sono in KeyList
   return (pt1[X] == (std::numeric_limits<double>::min)() && 
           pt1[Y] == (std::numeric_limits<double>::min)() && 
           pt1[Z] == (std::numeric_limits<double>::min)() &&
           ads_point_equal(pt1, pt2)) ? false : true;
}


void CEntDBValuesListDlg::OnSelchangeClasslistCombo() 
{
   SelchangeClasslistCombo(m_ClassAndSecListCombo.GetCurSel());
}

/****************************************************************/
/*.doc CEntDBValuesListDlg::SelchangeClasslistCombo <external> */
/*+                                                                       
  La funzione risponde all'evento di cambio selezione della classe.
  Parametri: 
  int nItem;         Numero nella lista delle classi della combo (0 indexed)
  bool SecRefresh;   Flag, se = TRUE si tratta di un'operazione di
                     Refresh dei dati delle schede secondarie (default = FALSE)
  
  Ritorna il numero di entità della classe o delle schede secondarie,
  -1 in caso di errore.
  Nel caso di tabelle secondarie viene inizializzata la variabile
  ColValues che contiene tutti i record delle schede secondarie

-*/  
/****************************************************************/
long CEntDBValuesListDlg::SelchangeClasslistCombo(int nItem, bool SecRefresh)
{
   long     ndx = 0, Key, Tot = 0;
   C_STRING NameWithQty;
   ads_name ent;

   if (nItem == CB_ERR) return -1;

   TempItemValueChanged = FALSE;
   ActualItem = 0;
   ColValues.remove_all();
   EntColValues.remove_all();
   PrevEntColValues.remove_all();
   KeyList.remove_all();

   HideEditCtrls();

   m_EntDBValuesList.DeleteAllItems();
   while (m_EntDBValuesList.DeleteColumn(0) != 0); // cancello le colonne

   HCURSOR PrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

   m_OK.EnableWindow(FALSE);

   if (IsForClass()) // Ricavo la classe selezionata
   {
      if (!(pActualCls = (C_CLASS *) m_ClassAndSecListCombo.GetItemDataPtr(nItem)))
         { SetCursor(PrevCursor); return -1; }
      
      // se non si sta interrogando celle di griglia letti da KeyList
      // non svuoto la lista KeyList
      if (IsDBGridMode() && IsDBGridFromWindow()) KeyList.remove_all();

      // Se la classe ha collegamento a db
      if (pActualCls->ptr_attrib_list() && pActualCls->ptr_info())
      {
         // Se la classe ha rappresentazione grafica
         if (pActualCls->ptr_fas())
         {
            // Ricavo i codici delle entità del gruppo di selezione che
            // appartengono alla classe selezionata
            while (SelSet.entname(ndx++, ent) == GS_GOOD)
            {
               if (acedUsrBrk())
               { 
                  KeyList.remove_all();
                  set_GS_ERR_COD(eGSNoError); 
                  SetCursor(PrevCursor); 
                  return -1;
               }                  

               if (pActualCls->getKeyValue(ent, &Key) == GS_GOOD)
                  if (KeyList.add(&Key) == GS_BAD) // Non vengono inseriti elementi doppi
                  { 
                     KeyList.remove_all();
                     set_GS_ERR_COD(eGSOutOfMem); 
                     SetCursor(PrevCursor); 
                     return -1;
                  }
            }
            Tot = KeyList.get_count();
         }
         else
         if (pActualCls->ptr_id()->category == CAT_GRID) // se si tratta di griglia
         {
            // Se i codici delle celle sono da finestra
            if (IsDBGridFromWindow())
            {
               C_RECT Rect(pt1, pt2);

               // ricavo i codici delle celle intersecanti la finestra
               if (pActualCls->ptr_grid()->getKeyListInWindow(Rect, CROSSING, KeyList) == GS_BAD)
               { 
                  KeyList.remove_all();
                  set_GS_ERR_COD(eGSNoError); 
                  SetCursor(PrevCursor); 
                  return -1;
               }
            }

            Tot = KeyList.get_count();
         }
         else // si tratta di gruppo
         {
            C_EED          eed;
            C_PREPARED_CMD pTempCmd, pOldCmd;
            C_CLASS        *pMemberCls;
            C_LONG_LIST    KeyGroupList;
            C_LONG        *pKeyGroup;

            // inizializzo compilazioni per verifica apparatenenza a complesse o gruppi
            if (pActualCls->prepare_reldata_where_member(pTempCmd, TEMP) == GS_BAD ||
                pActualCls->prepare_reldata_where_member(pOldCmd, OLD) == GS_BAD)
            { 
               KeyList.remove_all();
               SetCursor(PrevCursor); 
               return -1;
            }

            // Ricavo i codici delle entità del gruppo di selezione che
            // appartengono alla classe selezionata
            while (SelSet.entname(ndx++, ent) == GS_GOOD)
            {
               if (acedUsrBrk())
               { 
                  KeyList.remove_all();
                  set_GS_ERR_COD(eGSNoError); 
                  SetCursor(PrevCursor); 
                  return -1;
               }                  

               // se l'entità appartiene alla complessa o al gruppo in questione
               if (eed.load(ent) == GS_GOOD &&
                   pActualCls->is_member_class(eed.cls) == GS_GOOD &&
                   (pMemberCls = (C_CLASS *) get_GS_CURRENT_WRK_SESSION()->get_pPrj()->find_class(eed.cls,
                                                                                                  eed.sub)) &&
                   pMemberCls->getKeyValue(ent, &Key) == GS_GOOD)
               {
                  // Leggo tutti i gruppi collegati all'entità
                  if (pActualCls->get_group_list(pTempCmd, pOldCmd, eed.cls, Key, KeyGroupList) == GS_GOOD)
                  {
                     pKeyGroup = (C_LONG *) KeyGroupList.get_head();
                     while (pKeyGroup)
                     {
                        Key = pKeyGroup->get_id();
                        if (KeyList.add(&Key) == GS_BAD) // Non vengono inseriti elementi doppi
                        { 
                           KeyList.remove_all();
                           set_GS_ERR_COD(eGSOutOfMem); 
                           SetCursor(PrevCursor); 
                           return -1;
                        }
                        pKeyGroup = (C_LONG *) pKeyGroup->get_next();
                     }
                  }
               }
            }
            Tot = KeyList.get_count();
         }
      }
      else
      {
         C_EED eed;

         // Conto quanti sono le entità della classe
         while (SelSet.entname(ndx++, ent) == GS_GOOD)
         {
            if (acedUsrBrk())
            { 
               set_GS_ERR_COD(eGSNoError); 
               SetCursor(PrevCursor); 
               return -1;
            }                  

            if (eed.load(ent) == GS_GOOD && 
                eed.cls == pActualCls->ptr_id()->code && eed.sub == pActualCls->ptr_id()->sub_code)
               Tot++; 
         }
      }

      pActualCls->get_CompleteName(NameWithQty);
   }
   else // Ricavo la tabella secondaria selezionata
   {
      presbuf   pRbKey;
      C_RB_LIST dummy;

      if (!(pActualSec = (C_SECONDARY *) m_ClassAndSecListCombo.GetItemDataPtr(nItem)))
         { SetCursor(PrevCursor); return -1; }
   
      if (pActualSec->ptr_attrib_list()->init_ADOType(pActualSec->ptr_info()->getDBConnection(OLD)) == GS_BAD)
         return -1;

      KeyList.remove_all();

      if (!(pRbKey = MotherColValues.CdrAssoc(pActualSec->ptr_info()->key_pri.get_name())))
         { SetCursor(PrevCursor); return -1; }

      // Ricavo i codici e i record delle schede secondarie che
      // appartengono all'entità selezionata
      if (pActualSec->query_AllData(pRbKey, ColValues) == GS_BAD)
         { SetCursor(PrevCursor); return -1; }
         
      Tot = ColValues.GetSubListCount();
      ColValues.remove_head(); // cancello la prima tonda
      ColValues.remove_tail(); // cancello l'ultima tonda
      NameWithQty = pActualSec->name;
   }

   NameWithQty += _T(" (");
   NameWithQty += Tot;
   NameWithQty += _T(")");

   m_ClassAndSecListCombo.DeleteString(nItem);
   m_ClassAndSecListCombo.AddString(NameWithQty.get_name());
   if (IsForClass())
      m_ClassAndSecListCombo.SetItemDataPtr(m_ClassAndSecListCombo.FindStringExact(-1, NameWithQty.get_name()), pActualCls);
   else
      m_ClassAndSecListCombo.SetItemDataPtr(m_ClassAndSecListCombo.FindStringExact(-1, NameWithQty.get_name()), pActualSec);
   m_ClassAndSecListCombo.SetCurSel(nItem);

   if (IsForClass()) // Leggo i record degli oggetti della classe
   {
      if (Tot <= 0) { SetCursor(PrevCursor);  return 0; }

      FillClsRecords();

      SetVisSecTabButton(); // visualizza o meno il bottone delle secondarie
   }
   else // Leggo i record della tabella secondaria
   {
      FillSecRecords();
      if (Tot <= 0) { SetCursor(PrevCursor);  return 0; }
   }
   // dimensiono le colonne
   DisplayRecords(); // visualizza i dati letti
   ZoomHighlightEnt();

   // Se si stanno gestendo entità oppure 
   // nel caso si tratti di schede secondarie, non si vuole un semplice refresh
   if (IsForClass() || !SecRefresh)
      // se esiste un solo record la visualizzazione è, per default, a colonna
      if (Tot == 1)
      {
         if (!m_VertAttrMode) OnBnClickedVerticalMode();
      }
      else // altrimenti è tabellare
         if (m_VertAttrMode) OnBnClickedVerticalMode();

   SetCursor(PrevCursor);

   return Tot;
}

/************************************************************/
// Lettura dei record della classe
// la procedura deve trovare KeyList già inizializzata
// la procedura inizializza le seguenti variabili:
// EntColValues      che contiene una scheda con i valori uguali tra 
//                   quelli letti dalle entità selezionate
// PrevEntColValues  copia di EntColValues
// ColValues         che contiene tutti i record delle entità selezionate
/************************************************************/
int CEntDBValuesListDlg::FillClsRecords() 
{
   C_PREPARED_CMD_LIST TempOldCmdList;
   C_BLONG             *pKey = (C_BLONG *) KeyList.go_top();
   C_RB_LIST           _ColValues;

   HideEditCtrls();

   m_EntDBValuesList.DeleteAllItems();
   while (m_EntDBValuesList.DeleteColumn(0) != 0); // cancello le colonne

   // Leggo valori DB
   if (pActualCls->query_AllData(KeyList, ColValues, EntColValues, GS_GOOD) == GS_BAD) return GS_BAD;
   ColValues.remove_head();
   ColValues.remove_tail();

   EntColValues.copy(PrevEntColValues);

   return GS_GOOD;
}


/************************************************************/
// Visualizzazione dei record della tabella secondaria
// la procedura deve trovare ColValues già inizializzata
// la procedura inizializza le seguenti variabili:
// EntColValues      che contiene una scheda con i valori uguali tra 
//                   quelli letti dalle entità selezionate
// PrevEntColValues  copia di EntColValues
// KeyList           lista dei codici delle schede secondarie
/************************************************************/
int CEntDBValuesListDlg::FillSecRecords() 
{
   C_RB_LIST _ColValues;
   presbuf   pEntColValues, rbKey;
   long      Key;

   HideEditCtrls();

   m_EntDBValuesList.DeleteAllItems();
   while (m_EntDBValuesList.DeleteColumn(0) != 0); // cancello le colonne

   EntColValues.remove_all();
   KeyList.remove_all();
   pEntColValues = ColValues.get_head();
   while (pEntColValues)
   {
      _ColValues << gsc_listcopy(pEntColValues);

      if (pActualSec->type == GSInternalSecondaryTable)
      {
         if ((rbKey = _ColValues.CdrAssoc(pActualSec->ptr_info()->key_attrib.get_name())))
         {
            gsc_rb2Lng(rbKey, &Key);
            if (KeyList.add(&Key) == GS_BAD) // Non vengono inseriti elementi doppi
            { 
               KeyList.remove_all();
               set_GS_ERR_COD(eGSOutOfMem); 
               return GS_BAD;
            }
         }
      }

      if (!EntColValues.get_head()) _ColValues.copy(EntColValues);
      else // Confronto i campi annullando quelli diversi
         EntColValues.SubstRTNONEtoDifferent(_ColValues); // no case sensitive

      if ((pEntColValues = gsc_scorri(pEntColValues))) // vado alla chiusa tonda successiva
         pEntColValues = pEntColValues->rbnext; // vado alla aperta tonda successiva
   }

   EntColValues.copy(PrevEntColValues);

   return GS_GOOD;
}


/*************************************/
// Visualizza i record letti precedentemente a seconda della
// variabile m_VertAttrMode che determina lo stile di visualizzazione
// deveono essere già state inizializzte le seguenti:
// EntColValues, ColValues
// Parametri:
// int _row;  se = -1 vengono visualizzati tutti i record altrimenti
//            viene ridisegnata solo la riga richiesta (usato solo in formato tabellare)
/*************************************/
int CEntDBValuesListDlg::DisplayRecords(int _row) 
{
   C_STRING      AttribValue, AttribName;
   C_ATTRIB_LIST *pAttribList;
   int           m_cls, m_sub, m_sec = 0;

   if (!get_GS_CURRENT_WRK_SESSION()) return GS_BAD;

   HideEditCtrls();

   if (IsForClass())
   {
      if (!pActualCls || !(pAttribList = pActualCls->ptr_attrib_list()))
      {
         m_VertAttrModeButton.SetBitmap(hBmpHorizMode);
         if (pToolTip && pToolTip->m_hWnd) pToolTip->UpdateTipText(_T("Formato a tabella"), &m_VertAttrModeButton);
         return GS_BAD;
      }

      m_cls = pActualCls->ptr_id()->code;
      m_sub = pActualCls->ptr_id()->sub_code;
   }
   else
   {
      if (!pActualSec || !(pAttribList = pActualSec->ptr_attrib_list()))
      {
         m_VertAttrModeButton.SetBitmap(hBmpHorizMode);
         if (pToolTip && pToolTip->m_hWnd) pToolTip->UpdateTipText(_T("Formato a tabella"), &m_VertAttrModeButton);
         return GS_BAD;
      }

      m_cls = pActualSec->ptr_class()->ptr_id()->code;
      m_sub = pActualSec->ptr_class()->ptr_id()->sub_code;
      m_sec = pActualSec->get_key();
   }

   // Inizializza le colonne della lista e le dimensiona
   if (m_VertAttrMode || _row == -1)
   {
      m_EntDBValuesList.DeleteAllItems();
      while (m_EntDBValuesList.DeleteColumn(0) != 0); // svuoto la lista

      if (m_VertAttrMode) // disposizione degli attributi in verticale
      {
         m_EntDBValuesList.ModifyStyle(0L, LVS_NOCOLUMNHEADER); // senza intestazione    
         m_EntDBValuesList.InsertColumn(0, _T("Attributo"), LVCFMT_LEFT, 0, 0);
         m_EntDBValuesList.InsertColumn(1, _T("Valore"),    LVCFMT_LEFT, 0, 1);
         m_VertAttrModeButton.SetBitmap(hBmpHorizMode);
         if (pToolTip && pToolTip->m_hWnd) pToolTip->UpdateTipText(_T("Formato a tabella"), &m_VertAttrModeButton);
      }
      else // disposizione degli attributi in orizzontale
      {
         int         i = 0;
         LVCOLUMN    col;
         C_ATTRIB    *pAttrib;
         C_STRING    AttribName;

         m_EntDBValuesList.ModifyStyle(LVS_NOCOLUMNHEADER, 0L); // con intestazione  
         pAttrib = (C_ATTRIB *) pAttribList->get_head();
         while (pAttrib)
         {
            col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
	         col.fmt  = LVCFMT_LEFT;

            AttribName   = pAttrib->Caption;
            if (pAttrib->is_mandatory() == GS_GOOD) AttribName += MANDATORY_SYMBOL; 
            col.pszText  = AttribName.get_name();
            col.iSubItem = i;
            m_EntDBValuesList.InsertColumn(i, &col);

            i++;
            pAttrib = (C_ATTRIB *) pAttrib->get_next();
         }

         // Aggiungo eventuali bitmaps
	      CHeaderCtrl *pHdrCtrl = m_EntDBValuesList.GetHeaderCtrl();
	      HD_ITEM      curItem;

         // alloco buffer
         curItem.pszText    = (TCHAR *) calloc(sizeof(TCHAR), 100);
         curItem.cchTextMax = 100 - 1;
         curItem.mask       = HDI_TEXT;
         curItem.iImage     = 2; // lampadina

         i = 0;
         while (pHdrCtrl->GetItem(i, &curItem))
         {
            if (!(pAttrib = (C_ATTRIB *) pAttribList->getptr_at(i + 1))) break;

            if (pAttrib->is_visible() == GS_GOOD)
            {
   	         curItem.mask = HDI_IMAGE | HDI_FORMAT; //  Testo e immagine
               curItem.fmt  = HDF_STRING | HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
               pHdrCtrl->SetItem(i, &curItem);
            }

            i++;
            curItem.mask = HDI_TEXT;
         }
         free(curItem.pszText);

         m_VertAttrModeButton.SetBitmap(hBmpVertMode);
         if (pToolTip && pToolTip->m_hWnd) pToolTip->UpdateTipText(_T("Formato a colonna"), &m_VertAttrModeButton);
      }
   }

   if (m_VertAttrMode) // singola scheda
   {
      if (DisplayVertRecord(m_cls, m_sub, m_sec, pAttribList, EntColValues.get_head()) == GS_BAD)
         return GS_BAD;
   }
   else // formato tabellare
   {
      presbuf pEntColValues;

      if (_row == -1)
      {
         int row = 0;
         pEntColValues = ColValues.get_head();

         while (pEntColValues)
         {
            if (DisplayHorizRecord(m_cls, m_sub, m_sec, pAttribList, row, pEntColValues) == GS_BAD)
               return GS_BAD;

            pEntColValues = gsc_scorri(pEntColValues); // vado alla chiusa tonda successiva
            // vado alla parentesi aperta successiva
            if (pEntColValues) pEntColValues = pEntColValues->rbnext;
            row++;
         }
      }
      else // solo aggiornamento di una riga
      {
         pEntColValues = (presbuf) m_EntDBValuesList.GetItemData(_row);
         if (DisplayHorizRecord(m_cls, m_sub, m_sec, pAttribList, _row, pEntColValues, true) == GS_BAD)
            return GS_BAD;
      }  
   }

   CRect rect;
   m_EntDBValuesList.GetWindowRect(&rect);
   EntDBValuesListColumnSizing(rect.Width(), rect.Height());

   m_OK.EnableWindow(FALSE);

   return GS_GOOD;
}


/*************************************/
// Visualizza i record letti precedentemente nella modalità orizzontale
// devono essere già state inizializzate le seguenti variabili:
// EntColValues, ColValues
// Parametri:
// int _row;  se = -1 vengono visualizzati tutti i record altrimenti
//            viene ridisegnata solo la riga richiesta (usato solo in formato tabellare)
/*************************************/
int CEntDBValuesListDlg::DisplayHorizRecord(int m_cls, int m_sub, int m_sec, C_ATTRIB_LIST *pAttribList,
                                            int row, presbuf pEntColValues, bool RefreshExistingRow) 
{
   presbuf  p;
   int      col;
   C_STRING AttribValue;
	LV_ITEM  lvitem;
   C_ATTRIB *pAttrib;
   bool     firstTime = true;

   lvitem.iItem = row;
   p            = (pEntColValues && pEntColValues->restype == RTLB) ? pEntColValues->rbnext : NULL;
   while (p && (p->restype == RTLB))
   {
      p = p->rbnext;
      if ((col = pAttribList->getpos_name(p->resval.rstring, FALSE)) == 0)
         return GS_BAD;
      col--;
      pAttrib = (C_ATTRIB *) pAttribList->get_cursor();
      p = p->rbnext;
      pAttrib->ParseToString(p, AttribValue, pEntColValues, m_cls, m_sub, m_sec);

      // Valore attributo
      lvitem.iSubItem = col;
      lvitem.lParam   = (LPARAM) pEntColValues;
      lvitem.pszText  = (AttribValue.len() == 0) ? _T("") : AttribValue.get_name();
      if (firstTime)
      {
         firstTime = false;
         lvitem.mask = LVIF_TEXT | LVIF_PARAM;
         if (!RefreshExistingRow) m_EntDBValuesList.InsertItem(&lvitem);
         else m_EntDBValuesList.SetItem(&lvitem);
      }
      else
      {
      	lvitem.mask = LVIF_TEXT;
         m_EntDBValuesList.SetItem(&lvitem);
      }
   
      p = ((p = p->rbnext) && p->restype == RTLE) ? p = p->rbnext : NULL;
   }

   return GS_GOOD;
}
int CEntDBValuesListDlg::DisplayVertRecord(int m_cls, int m_sub, int m_sec, C_ATTRIB_LIST *pAttribList,
                                           presbuf pEntColValues)
{
   presbuf  p;
   int      row = 0;
   C_STRING AttribValue, AttribName;
	LV_ITEM  lvitem;
   C_ATTRIB *pAttrib = (C_ATTRIB *) pAttribList->get_head();

   if (!pEntColValues) return GS_BAD;

   while (pAttrib)
   {
      p = gsc_CdrAssoc(pAttrib->get_name(), pEntColValues, FALSE);
      pAttrib->ParseToString(p, AttribValue, pEntColValues, m_cls, m_sub, m_sec);

      // Caption attributo con Flag di Visibilità
      lvitem.iItem    = row;
	   lvitem.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
      lvitem.iSubItem = 0;
      lvitem.iImage   = (pAttrib->is_visible() == GS_GOOD) ? 1 : 0;
      lvitem.lParam   = (LPARAM) pAttrib;
      AttribName      = pAttrib->Caption;
      if (pAttrib->is_mandatory() == GS_GOOD) AttribName += MANDATORY_SYMBOL; 
      lvitem.pszText  = AttribName.get_name();
      m_EntDBValuesList.InsertItem(&lvitem);

      // Valore attributo
      lvitem.mask     = LVIF_TEXT;
      lvitem.iSubItem = 1;
      lvitem.pszText  = (AttribValue.len() == 0) ? _T("") : AttribValue.get_name();
      m_EntDBValuesList.SetItem(&lvitem);

      row++;
      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   return GS_GOOD;
}


int CEntDBValuesListDlg::EditValue()
{
   C_ATTRIB       *pAttrib;
   CString        PrevValue;
   int            _prj = 0, _cls = 0, _sub = 0, _sec = 0;
   C_ATTRIB_LIST  *pAttribList;
   C_INFO         *pInfo;
   C_DBCONNECTION *pConn;

   TempItemValueChanged = FALSE;
    
   HideEditCtrls();

   if (!get_GS_CURRENT_WRK_SESSION() ||
       get_GS_CURRENT_WRK_SESSION()->isReadyToUpd() == GS_BAD) return GS_BAD;

   if (IsForClass()) // gestione entità
   {
      if (!(pInfo = pActualCls->ptr_info()) ||
          !(pAttribList = pActualCls->ptr_attrib_list()) ||
          pActualCls->ptr_id()->abilit != GSUpdateableData ||
          !(pAttrib = get_AttribPtr()))
         return GS_BAD;
      _prj  = pActualCls->get_PrjId();
      _cls  = pActualCls->ptr_id()->code;
      _sub  = pActualCls->ptr_id()->sub_code;
      pConn = pActualCls->ptr_info()->pOldTabConn;
   }
   else // gestione schede secondarie
   {
      if (pActualSec->type != GSInternalSecondaryTable)
      {
         // Nonostante non si possa modificare il valore setto comunque questa
         // variabile perchè possa funzionare il doppio click su una path (es.)
         TempItemValue = m_EntDBValuesList.GetItemText(ActualItem, ActualSubItem);
         return GS_BAD;
      }
      pInfo       = pActualSec->ptr_info();
      pAttribList = pActualSec->ptr_attrib_list();
      _prj  = pActualSec->ptr_class()->get_PrjId();
      _cls  = pActualSec->ptr_class()->ptr_id()->code;
      _sub  = pActualSec->ptr_class()->ptr_id()->sub_code;
      _sec  = pActualSec->code;
      pConn = pActualSec->ptr_info()->pOldTabConn;
   }

   if (!(pAttrib = get_AttribPtr())) return GS_BAD;

   // attributo non modificabile (calcolato o attributo chiave es. "GS_ID")
   if (pInfo->key_attrib.comp(pAttrib->get_name(), FALSE) == 0 ||
       pAttrib->is_calculated() == GS_GOOD)
      return GS_BAD;

   TempItemValue = PrevValue = m_EntDBValuesList.GetItemText(ActualItem, ActualSubItem);

   if (pAttribList->init_ADOType(pConn) == GS_BAD) return GS_BAD;

   // Per i valori di tipo data o timestamp
   if (gsc_DBIsDate(pAttrib->ADOType) == GS_GOOD || gsc_DBIsTimestamp(pAttrib->ADOType) == GS_GOOD)
   {
      C_STRING dummy;

      dummy = get_presbufItemData();
      PrevValue = dummy.get_name();
   }

   MaskedEditVis = EditVis = ComboButtonVis = FALSE;

   // Se bisogna usare una maschera e se il campo è lungo fino a 64
   if (pAttrib->InputMask.len() > 0 && pAttrib->len <= 64)
      MaskedEditVis = TRUE;
   else 
      EditVis = TRUE;

   // Se non ci sono file di scelta
   C_STRING OutPath;
   ValuesListTypeEnum FileType;
   if (gsc_FindSupportFiles(pAttrib->get_name(), _prj, _cls, _sub, _sec,
                            OutPath, &FileType) == GS_GOOD)
   {  // Se ci sono file di scelta
      ComboButtonVis = TRUE;
      // Se ci sono file di scelta obbligata
      if (FileType == TAB || FileType == FDF) MaskedEditVis = EditVis = FALSE;
   }
   
   SetPosEntDBValuesListEditCtrls();

   m_Remark.SetWindowText((pAttrib->Descr.len() > 0) ? pAttrib->Descr.get_name() : _T(""));

   if (ComboButtonVis)
   {
      m_ComboButton.ShowWindow(SW_SHOW);
      if (!EditVis && !MaskedEditVis) OnComboButton();
   }

   if (EditVis)
   {
      m_Edit.ShowWindow(SW_SHOW);
      m_Edit.SetWindowText(PrevValue);
      m_Edit.SetSel(0, -1);
      m_Edit.SetFocus();
   }

   if (MaskedEditVis)
   {
      CString InputTemplate;

      for (int i = 0; i < (int) pAttrib->InputMask.len(); i++) InputTemplate.AppendChar(_T('_'));

      m_MaskedEdit.ShowWindow(SW_SHOW);
      m_MaskedEdit.EnableMask(pAttrib->InputMask.get_name(), InputTemplate);
      m_MaskedEdit.SetValidChars(NULL);
      m_MaskedEdit.SetWindowText(PrevValue);
      m_MaskedEdit.EnableGetMaskedCharsOnly(TRUE);
      m_MaskedEdit.SetSel(0, -1);
      m_MaskedEdit.SetFocus();
   }

   return GS_GOOD;
}

void CEntDBValuesListDlg::OnChangeEdit() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::CPropertyPage()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
   m_Edit.GetWindowText(TempItemValue);
   TempItemValueChanged = TRUE;
   m_OK.EnableWindow(TRUE);
}

void CEntDBValuesListDlg::OnChangeMaskedbox()
{  // Siccome la clase CMFCMaskedEdit da cui è ereditato m_MaskedEdit
   // sull'evento onChange NON restituisce il testo corrente ma quello precedente la modifica
   // per leggere il vero valore devo aspettare l'evento KillFocus
   TempItemValueChanged = TRUE;
   m_OK.EnableWindow(TRUE);
}

void CEntDBValuesListDlg::OnEnKillfocusMaskedbox() 
{  // Siccome la clase CMFCMaskedEdit da cui è ereditato m_MaskedEdit
   // sull'evento onChange NON restituisce il testo corrente ma quello precedente la modifica
   // per leggere il vero valore devo aspettare l'evento KillFocus
   if (TempItemValueChanged)
      m_MaskedEdit.GetWindowText(TempItemValue);
}


BOOL CEntDBValuesListDlg::OnChangeValue(CString &Value) 
{
   if (m_VertAttrMode) // singola scheda
      return OnChangeVertValue(Value);
   else // formato tabellare
      return OnChangeHorizValue(Value);
}

BOOL CEntDBValuesListDlg::OnChangeVertValue(CString &Value) 
{
   C_ATTRIB_LIST *pAttribList;
   C_ATTRIB      *pAttrib, *pChangedAttrib;
   presbuf       pRb;
   TCHAR         *pmessage;
   C_STRING      AttribValue((LPCTSTR) Value);
   double        NumValue;
   CString       Operat, dummy;
   bool          Perc;
   int           m_cls, m_sub, m_sec = 0;

   if (!get_GS_CURRENT_WRK_SESSION()) return GS_BAD;
   if (!(pChangedAttrib = get_AttribPtr())) return GS_BAD;
   if (!pChangedAttrib || (pRb = EntColValues.Assoc(pChangedAttrib->get_name())) == NULL) return FALSE;
   //if (!pChangedAttrib || (pRb = EntColValues.nth(ActualItem)) == NULL) return FALSE;

   if (IsForClass()) // gestione entità
   {
      if (!pActualCls) return GS_BAD;
      if (!(pAttribList = pActualCls->ptr_attrib_list())) return GS_BAD;

      m_cls = pActualCls->ptr_id()->code;
      m_sub = pActualCls->ptr_id()->sub_code;
   }
   else // gestione schede secondarie
   {
      if (!pActualSec) return GS_BAD;
      if (!(pAttribList = pActualSec->ptr_attrib_list())) return GS_BAD;

      m_cls = pActualSec->ptr_class()->ptr_id()->code;
      m_sub = pActualSec->ptr_class()->ptr_id()->sub_code;
      m_sec = pActualSec->get_key();
   }

   if (!(pRb = gsc_nth(0, pRb)) || pRb->restype != RTSTR) return FALSE;

   // Verifico che abbia un valore utilizzabile
   if (gsui_IsUsefulAttribValue(Value))
   {
      C_RB_LIST TmpEntColValues;
      C_STRING  DummyStr;
   
      EntColValues.copy(TmpEntColValues);

      // Valore precedente dell'attributo
      pRb = TmpEntColValues.CdrAssoc(pChangedAttrib->get_name());
      //pRb = gsc_nth(1, TmpEntColValues.nth(ActualItem));
      
      // Se il valore non è cambiato rispetto quello precedente
      DummyStr.paste(gsc_rb2str(pRb));
      if (AttribValue.comp(DummyStr) == 0) return TRUE;

      // Se il valore precedente dell'attributo era un operatore matematico
      if (gsui_GetAttribMathOp(pRb, &NumValue, Operat, &Perc) == GS_GOOD)
      {  // elimino l'operatore matematico e pongo il valore dell'attributo = NIL
         presbuf p = pRb->rbnext;

         gsc_RbSubstNIL(pRb);
         pRb->rbnext = p->rbnext->rbnext;
         p->rbnext->rbnext = NULL;
         acutRelRb(p);
      }

      pRb = gsc_nth(0, TmpEntColValues.Assoc(pChangedAttrib->get_name()));
      //pRb = gsc_nth(0, TmpEntColValues.nth(ActualItem));

      // Verifico valore
      if (IsForClass()) // gestione entità
         pmessage = gsc_checkValue(pActualCls, pRb, pChangedAttrib, AttribValue.get_name());
      else // gestione schede secondarie
         pmessage = gsc_checkValue(pActualSec, pRb, pChangedAttrib, AttribValue.get_name());

      if (pmessage)
      {
         gsui_alert(pmessage);
         free(pmessage);
         return FALSE;
      }

      if (gsui_CanIRecalcOnLine(pChangedAttrib, Value, pAttribList,
                                TmpEntColValues, KeyList.get_count()))
      {
         // Ricalcola gli eventuali campi calcolati che dipendono dalla variazione
         // dell'attributo
         if (pAttribList->calc(pChangedAttrib->get_name(), TmpEntColValues,
                               OperationMode) == GS_BAD)
         {
            gsui_alert(_T("Funzione di calcolo fallita."));
            return FALSE;
         }
      }

      // Controlla i valori della scheda ad eccezione di pChangedAttrib
      presbuf  pValue;
      C_ATTRIB *punt = (C_ATTRIB *) pAttribList->get_head();

      while (punt)
      {
         if (punt != pChangedAttrib && (pValue = TmpEntColValues.CdrAssoc(punt->get_name())) != NULL)
            // Verifico che abbia un valore utilizzabile
            if (gsui_IsUsefulAttribValue(pValue, KeyList.get_count()))
               if (punt->CheckValue(pValue) == GS_BAD)
               {
                  C_STRING Msg;

                  Msg = _T("Valore dell'attributo <");
                  Msg += punt->get_name();
                  Msg += _T("> non permesso.");

                  gsui_alert(Msg);
                  return FALSE;
               }

         punt = (C_ATTRIB *) punt->get_next();   
      }

      if (gsui_CanIValidOnLine(pChangedAttrib, Value, pAttribList))
      {
         C_ATTRIB *pWrongAttrib;

         // Valida gli eventuali campi con funzione di validazione che dipende
         // dalla variazione dell'attributo
         if (pAttribList->validate(pChangedAttrib->get_name(), TmpEntColValues,
                                   &pWrongAttrib) == GS_BAD)
         {
            if (pWrongAttrib->ValidErrMsg.len() > 0) 
               gsui_alert(pWrongAttrib->ValidErrMsg);
            else
            {
               C_STRING Msg;

               if (pWrongAttrib)
               {
                  Msg = _T("Validità dell'attributo <");
                  Msg += pWrongAttrib->get_name();
                  Msg += _T("> non soddisfatta.");
               }
               else
                  Msg = _T("Validità di un attributo non soddisfatta.");

               gsui_alert(Msg);
            }

            return FALSE;
         }
      }

      // Se tutto è andato bene ricopio i valori in EntColValues
      TmpEntColValues.copy(EntColValues);
   }
   else // Se si tratta di un operatore matematico
   {
      if (gsui_GetAttribMathOp(Value, &NumValue, Operat, &Perc) == GS_GOOD)
         gsui_SetAttribMathOp(EntColValues, ActualItem, NumValue, Operat, Perc);
   }

   // rimuovo dalla memoria le liste di scelta che dipendono da pChangedAttrib
   get_GS_CURRENT_WRK_SESSION()->get_pCacheClsAttribValuesList()->removeDependingOnAttrib(pChangedAttrib->name,
                                                                                          get_GS_CURRENT_WRK_SESSION()->get_PrjId(),
                                                                                          m_cls, m_sub, m_sec);

   // ridisegna la lista dei valori
   int nItem = 0;
   pAttrib = (C_ATTRIB *) pAttribList->get_head();
   while (pAttrib)
   {
      if ((pRb = EntColValues.CdrAssoc(pAttrib->get_name())) != NULL)
      {
         if (gsui_GetAttribMathOp(pRb, &NumValue, Operat, &Perc) == GS_GOOD)
         {
            gsui_GetAttribMathOpMsg(NumValue, Operat, Perc, dummy);
            AttribValue = dummy;
         }
         else
            pAttrib->ParseToString(pRb, AttribValue, &EntColValues,
                                   m_cls, m_sub, m_sec);

         m_EntDBValuesList.SetItemText(nItem, 1, AttribValue.get_name());
      }
      nItem++;
      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_OK.EnableWindow(TRUE);
   
   return TRUE;
}

BOOL CEntDBValuesListDlg::OnChangeHorizValue(CString &Value) 
{
   C_ATTRIB_LIST *pAttribList;
   C_ATTRIB      *pAttrib, *pChangedAttrib;
   presbuf       pRb, pRb1;
   TCHAR         *pmessage;
   C_STRING      AttribValue((LPCTSTR) Value);
   double        NumValue;
   CString       Operat, dummy;
   bool          Perc;
   C_RB_LIST     TmpEntColValues;
   int           m_cls, m_sub, m_sec = 0;

   if (!get_GS_CURRENT_WRK_SESSION()) return GS_BAD;

   if (IsForClass()) // gestione entità
   {
      pAttribList = pActualCls->ptr_attrib_list();

      m_cls = pActualCls->ptr_id()->code;
      m_sub = pActualCls->ptr_id()->sub_code;
   }
   else // gestione schede secondarie
   {
      pAttribList = pActualSec->ptr_attrib_list();

      m_cls = pActualSec->ptr_class()->ptr_id()->code;
      m_sub = pActualSec->ptr_class()->ptr_id()->sub_code;
      m_sec = pActualSec->get_key();
   }

   if (!(pChangedAttrib = (C_ATTRIB *) pAttribList->getptr_at(ActualSubItem + 1)))
      return FALSE;

   if ((pRb = (presbuf) m_EntDBValuesList.GetItemData(ActualItem)) == NULL)
      return FALSE;
   if (TmpEntColValues << gsc_listcopy(pRb) == NULL) return FALSE;

   // Verifico che abbia un valore utilizzabile
   if (gsui_IsUsefulAttribValue(Value))
   {
      C_STRING DummyStr;

      // Valore precedente dell'attributo
      pRb = TmpEntColValues.CdrAssoc(pChangedAttrib->get_name());

      // Se il valore non è cambiato rispetto quello precedente
      DummyStr.paste(gsc_rb2str(pRb));
      if (AttribValue.comp(DummyStr) == 0) return TRUE;

      // Se il valore precedente dell'attributo era un operatore matematico
      if (gsui_GetAttribMathOp(pRb, &NumValue, Operat, &Perc) == GS_GOOD)
      {  // elimino l'operatore matematico e pongo il valore dell'attributo = NIL
         presbuf p = pRb->rbnext;

         gsc_RbSubstNIL(pRb);
         pRb->rbnext = p->rbnext->rbnext;
         p->rbnext->rbnext = NULL;
         acutRelRb(p);
      }

      pRb = gsc_nth(0, TmpEntColValues.Assoc(pChangedAttrib->get_name()));

      // Verifico valore
      if (IsForClass()) // gestione entità
         pmessage = gsc_checkValue(pActualCls, pRb, pChangedAttrib, AttribValue.get_name());
      else // gestione schede secondarie
         pmessage = gsc_checkValue(pActualSec, pRb, pChangedAttrib, AttribValue.get_name());

      if (pmessage)
      {
         gsui_alert(pmessage);
         free(pmessage);
         return FALSE;
      }

      // Ricalcola gli eventuali campi calcolati che dipendono dalla variazione
      // dell'attributo
      if (pAttribList->calc(pChangedAttrib->get_name(), TmpEntColValues,
                            OperationMode) == GS_BAD)
      {
         gsui_alert(_T("Funzione di calcolo fallita."));
         return FALSE;
      }

      // Controlla i valori della scheda ad eccezione di pChangedAttrib
      presbuf  pValue;
      C_ATTRIB *punt = (C_ATTRIB *) pAttribList->get_head();

      while (punt)
      {
         if (punt != pChangedAttrib && (pValue = TmpEntColValues.CdrAssoc(punt->get_name())) != NULL)
            // Verifico che abbia un valore utilizzabile
            if (gsui_IsUsefulAttribValue(pValue, KeyList.get_count()))
               if (punt->CheckValue(pValue) == GS_BAD)
               {
                  C_STRING Msg;

                  Msg = _T("Valore dell'attributo <");
                  Msg += punt->get_name();
                  Msg += _T("> non permesso.");

                  gsui_alert(Msg);
                  return FALSE;
               }

         punt = (C_ATTRIB *) punt->get_next();   
      }

      if (gsui_CanIValidOnLine(pChangedAttrib, Value, pAttribList))
      {
         C_ATTRIB *pWrongAttrib;

         // Valida gli eventuali campi con funzione di validazione che dipende
         // dalla variazione dell'attributo
         if (pAttribList->validate(pChangedAttrib->get_name(), TmpEntColValues,
                                   &pWrongAttrib) == GS_BAD)
         {
            if (pWrongAttrib->ValidErrMsg.len() > 0) 
               gsui_alert(pWrongAttrib->ValidErrMsg);
            else
            {
               C_STRING Msg;

               if (pWrongAttrib)
               {
                  Msg = _T("Validità dell'attributo <");
                  Msg += pWrongAttrib->get_name();
                  Msg += _T("> non soddisfatta.");
               }
               else
                  Msg = _T("Validità di un attributo non soddisfatta.");

               gsui_alert(Msg);
            }

            return FALSE;
         }
      }

      // Se tutto è andato bene ricopio i valori in ColValues
      if (!(pRb = (presbuf) m_EntDBValuesList.GetItemData(ActualItem)))
         return FALSE;
      pValue = TmpEntColValues.get_head();
      while (pValue && pRb)
      {
         if (gsc_RbSubst(pRb, pValue) == GS_BAD) return FALSE;
         pValue = pValue->rbnext;
         pRb    = pRb->rbnext;
      }
   }
   else // Se si tratta di un operatore matematico
   {
      if (gsui_GetAttribMathOp(Value, &NumValue, Operat, &Perc) == GS_GOOD)
      {
         int col = pAttribList->getpos_name(pChangedAttrib->get_name(), FALSE);
         col--;

         gsui_SetAttribMathOp(TmpEntColValues, col, NumValue, Operat, Perc);
         if (!(pRb = (presbuf) m_EntDBValuesList.GetItemData(ActualItem)) == NULL)
            return FALSE;
         pRb = gsc_nth(1, gsc_nth(col, pRb));
         pRb1 = gsc_nth(1, TmpEntColValues.nth(col));
         if (pRb1 && pRb)
            if (gsc_RbSubst(pRb, pRb1) == GS_BAD) return FALSE;
      }
   }

   // rimuovo dalla memoria le liste di scelta che dipendono da pChangedAttrib
   get_GS_CURRENT_WRK_SESSION()->get_pCacheClsAttribValuesList()->removeDependingOnAttrib(pChangedAttrib->name,
                                                                                          get_GS_CURRENT_WRK_SESSION()->get_PrjId(),
                                                                                          m_cls, m_sub, m_sec);

   // ridisegna la lista dei valori
   int nSubItem = 0;
   pAttrib = (C_ATTRIB *) pAttribList->get_head();
   while (pAttrib)
   {
      if ((pRb = gsc_CdrAssoc(pAttrib->get_name(), (presbuf) m_EntDBValuesList.GetItemData(ActualItem), FALSE)) != NULL)
      {
         if (gsui_GetAttribMathOp(pRb, &NumValue, Operat, &Perc) == GS_GOOD)
         {
            gsui_GetAttribMathOpMsg(NumValue, Operat, Perc, dummy);
            AttribValue = dummy;
         }
         else
            pAttrib->ParseToString(pRb, AttribValue,
                                   (presbuf) m_EntDBValuesList.GetItemData(ActualItem),
                                   m_cls, m_sub, m_sec);

         m_EntDBValuesList.SetItemText(ActualItem, nSubItem, AttribValue.get_name());
      }
      nSubItem++;
      pAttrib = (C_ATTRIB *) pAttrib->get_next();
   }

   m_OK.EnableWindow(TRUE);
   
   return TRUE;
}

void CEntDBValuesListDlg::OnDblclkEntdbvaluesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
   // Gestione doppio click
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

   if (pNMListView->iItem == -1) 
      { *pResult = 0; return; } // nessuna selezione

   if (IsForClass()) // se gestione entità della classe (non secondarie)
   {
      C_ATTRIB *pAttrib = get_AttribPtr(pNMListView->iItem, pNMListView->iSubItem);
      // Se si tratta del campo chiave faccio lo zoom 
      if (pAttrib && pActualCls->ptr_info()->key_attrib.comp(pAttrib->get_name()) == 0)
      {
         BOOL PrevZoom = m_Zoom;

         m_Zoom = true; // forzo lo zoom temporaneamente
         ZoomHighlightEnt();
         m_Zoom = PrevZoom; // ripristino il flag di zoom
      }
   }

   C_STRING Document((LPCTSTR) m_EntDBValuesList.GetItemText(pNMListView->iItem, pNMListView->iSubItem));

   // traduco la path
   if (gsc_nethost2drive(Document, GS_BAD) == GS_GOOD)
   {  // se Document rappresenta un file esistente
      if (gsc_path_exist(Document) == GS_BAD) return;
   }
   else // se Document rappresenta un indirizzo internet
       if (gsc_strstr(Document.get_name(), _T("http://"), FALSE) != Document.get_name())
          return;

   // visualizzo il documento
   gsc_DocumentExecute(Document.get_name(), FALSE);
   return;
	
	*pResult = 0;
}

void CEntDBValuesListDlg::OnRclickEntdbvaluesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
   OnClickEntdbvaluesList(pNMHDR, pResult);
	*pResult = 0;
}

void CEntDBValuesListDlg::OnClickEntdbvaluesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
   if (pNMListView->iItem == -1) return; // nessuna selezione

   // Aggiorno eventuale modifica valore
   if (TempItemValueChanged)
   {
      if (!OnChangeValue(TempItemValue))
      {
         EnsureVisibleItem();
         m_EntDBValuesList.SetItemState(ActualItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
         EditValue();
         return;
      }

      TempItemValueChanged = FALSE;
   }

   if (m_VertAttrMode) // singola scheda
   {
      if (GoNextUpdItem(pNMListView->iItem - 1, pNMListView->iSubItem - 1)) 
         EditValue();
   }
   else // formato tabellare
   {
      // se è stata selezionata una riga diversa da quella precedente
      if (pNMListView->iItem != ActualItem)
         // aggiorno la riga eventualmente modificata
         if (m_OK.IsWindowEnabled())
            OnOkHoriz();

      if (GoNextUpdItem(pNMListView->iItem, pNMListView->iSubItem - 1)) 
         EditValue();
   }

	*pResult = 0;
}

void CEntDBValuesListDlg::OnKeydownEntdbvaluesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

   BOOL ShiftState = (GetKeyState(VK_SHIFT) < 0) ? TRUE : FALSE;

   // Freccia giù o freccia destra o Tab o Return
   if (pLVKeyDow->wVKey == VK_DOWN || pLVKeyDow->wVKey == VK_RIGHT ||
       (pLVKeyDow->wVKey == VK_TAB && !ShiftState) ||
       pLVKeyDow->wVKey == VK_RETURN)
   {
      // se formato tabellare e freccia giù allora cambio riga
      if (!m_VertAttrMode && pLVKeyDow->wVKey == VK_DOWN)
      {
         // se esiste una riga su cui spostarsi
         if (ActualItem + 1 < m_EntDBValuesList.GetItemCount())
            // aggiorno la riga
            if (m_OK.IsWindowEnabled())
               OnOkHoriz();

         GoNextUpdItem(ActualItem + 1, ActualSubItem - 1);
      }
      else
         GoNextUpdItem(ActualItem, ActualSubItem);

      EditValue();

      *pResult = 1;
   }
   else 
   // Freccia sù o freccia sinistra o BackTab
   if (pLVKeyDow->wVKey == VK_UP || pLVKeyDow->wVKey == VK_LEFT ||
       (pLVKeyDow->wVKey == VK_TAB && ShiftState))
   {
      // se formato tabellare e freccia sù allora cambio riga
      if (pLVKeyDow->wVKey == VK_UP && !m_VertAttrMode)
      {
         // se esiste una riga su cui spostarsi
         if (ActualItem - 1 >= 0)
            // aggiorno la riga
            if (m_OK.IsWindowEnabled())
               OnOkHoriz();

         GoNextUpdItem(ActualItem - 1, ActualSubItem - 1);
      }
      else
         GoPrevUpdItem(ActualItem, ActualSubItem);

      EditValue();

      *pResult = 1;
   }
   else if (pLVKeyDow->wVKey == VK_ESCAPE) // Esc
   {
      TempItemValueChanged = FALSE;
      GoNextUpdItem(ActualItem, ActualSubItem);
      EditValue();

      *pResult = 1;
   }
   else
	   *pResult = 0;
}

BOOL CEntDBValuesListDlg::GoPrevUpdItem(int nItem, int nSubItem) 
{
   C_INFO        *pInfo;
   C_ATTRIB_LIST *pAttribList;
   C_ATTRIB      *pAttrib = NULL;

   if (IsForClass()) // gestione entità
   {
      if (!(pInfo = pActualCls->ptr_info()) ||
          !(pAttribList = pActualCls->ptr_attrib_list()))
         return FALSE;      
   }
   else // gestione schede secondarie
   {
      pInfo       = pActualSec->ptr_info();
      pAttribList = pActualCls->ptr_attrib_list();
   }

   // Aggiorno eventuale modifica valore
   if (TempItemValueChanged)
   {
      if (!OnChangeValue(TempItemValue)) return FALSE;
      TempItemValueChanged = FALSE;
   }

   if (m_VertAttrMode) // singola scheda
      while ((pAttrib = (C_ATTRIB *) m_EntDBValuesList.GetItemData(--nItem)))
      {
         // attributo Modificabile (calcolato o attributo chiave es. "GS_ID")
         if (pInfo->key_attrib.comp(pAttrib->get_name(), FALSE) != 0 &&
            pAttrib->is_calculated() == GS_BAD)
            break;
      }
   else // formato tabellare
      do
      {
         if (--nSubItem < 0) // vado all'ultimo attributo della riga precedente
         {
            if (--nItem < 0) return FALSE;
            // aggiorno la riga e mi sposto su quella precedente
            if (m_OK.IsWindowEnabled())
               OnOkHoriz();
            nSubItem = pAttribList->get_count() - 1;
         }

         if (!(pAttrib = (C_ATTRIB *) pAttribList->getptr_at(nSubItem + 1)))
            return FALSE;
      }
      // attributo Modificabile (calcolato o attributo chiave es. "GS_ID")
      while (pInfo->key_attrib.comp(pAttrib->get_name(), FALSE) == 0 ||
             pAttrib->is_calculated() == GS_GOOD);

   if (pAttrib)
   {
      ActualItem    = nItem;
      ActualSubItem = nSubItem;
      EnsureVisibleItem();
      m_EntDBValuesList.SetItemState(ActualItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
      if (!m_VertAttrMode)// formato tabellare
         ZoomHighlightEnt();

      return TRUE;
   }

   return FALSE;
}

BOOL CEntDBValuesListDlg::GoNextUpdItem(int nItem, int nSubItem) 
{
   C_INFO        *pInfo;
   C_ATTRIB_LIST *pAttribList;
   C_ATTRIB      *pAttrib;
   bool          AlmostOneEditable = FALSE;

   if (IsForClass()) // gestione entità
   {
      if (!pActualCls) return FALSE;
      if (!(pInfo = pActualCls->ptr_info()) ||
          !(pAttribList = pActualCls->ptr_attrib_list()))
         return FALSE;      
   }
   else // gestione schede secondarie
   {
      if (!pActualSec) return FALSE;

      pInfo       = pActualSec->ptr_info();
      pAttribList = pActualSec->ptr_attrib_list();
   }

   pAttrib = (C_ATTRIB *) pAttribList->get_head();
   while (pAttrib)
      // attributo Modificabile (calcolato o attributo chiave es. "GS_ID")
      if (pInfo->key_attrib.comp(pAttrib->get_name(), FALSE) != 0 &&
          pAttrib->is_calculated() == GS_BAD)
         { AlmostOneEditable = TRUE; break; }
      else pAttrib = (C_ATTRIB *) pAttrib->get_next();

   // Aggiorno eventuale modifica valore
   if (TempItemValueChanged)
   {
      if (!OnChangeValue(TempItemValue)) return FALSE;
      TempItemValueChanged = FALSE;
   }

   if (m_VertAttrMode) // singola scheda
   {
      nSubItem = 1;
      while ((pAttrib = (C_ATTRIB *) m_EntDBValuesList.GetItemData(++nItem)))
      {
         // attributo Modificabile (calcolato o attributo chiave es. "GS_ID")
         if (pInfo->key_attrib.comp(pAttrib->get_name(), FALSE) != 0 &&
             pAttrib->is_calculated() == GS_BAD)
            break;
      }
   }
   else // formato tabellare
   {
      if (nItem < 0) nItem = 0;
      if (nItem >= m_EntDBValuesList.GetItemCount()) return FALSE;

      do
      {
         if (++nSubItem >= pAttribList->get_count()) // vado al primo attributo della riga successiva
         {
            if (!AlmostOneEditable) // se non c'è neanche un attributo editabile
            {
               nSubItem = 0;
               pAttrib = (C_ATTRIB *) pAttribList->getptr_at(nSubItem + 1);
               break;
            }
            if (++nItem >= m_EntDBValuesList.GetItemCount()) return FALSE;
            // aggiorno la riga e mi sposto su quella successiva
            if (m_OK.IsWindowEnabled())
               OnOkHoriz();
            nSubItem = 0;
         }

         if (!(pAttrib = (C_ATTRIB *) pAttribList->getptr_at(nSubItem + 1)))
            return FALSE;
      }
      // attributo Modificabile (calcolato o attributo chiave es. "GS_ID")
      while (pInfo->key_attrib.comp(pAttrib->get_name(), FALSE) == 0 ||
             pAttrib->is_calculated() == GS_GOOD);
   }

   if (pAttrib)
   {
      HideEditCtrls();
      ActualItem = nItem;
      ActualSubItem = nSubItem;
      EnsureVisibleItem();
      m_EntDBValuesList.SetItemState(ActualItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
      if (!m_VertAttrMode)// formato tabellare
         ZoomHighlightEnt();

      return TRUE;
   }

   return FALSE;
}

void CEntDBValuesListDlg::OnOk()
{
   if (m_VertAttrMode) // singola scheda
      OnOkVert();
   else // formato tabellare
      OnOkHoriz();  

   // Set Focus to AutoCAD because AutoCAD doesn't update its
   // display if it's not in focus.
   acedGetAcadFrame()->SetActiveWindow();
   acedGetAcadFrame()->SetFocus();
   
   refreshDisplay();
   acutPrintf(_T("\n"));
}

void CEntDBValuesListDlg::OnOkVert() 
{
   long      i = 0;
   presbuf   rbPrevValue, rbValue, rbField;
   C_RB_LIST DiffEntColValues;
   double    Value;
   CString   Operat;
   bool      Perc, ToReRead = FALSE;
   int       old_reactor_abilit;

   // Aggiorno eventuale modifica valore
   if (TempItemValueChanged)
   {
      if (!OnChangeValue(TempItemValue))
      {
         EnsureVisibleItem();
         m_EntDBValuesList.SetItemState(ActualItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
         EditValue();
         return;
      }
      TempItemValueChanged = FALSE;
   }

   // Creo una lista contenente solo gli attributi che sono stati variati
   while ((rbField = EntColValues.nth(i)) != NULL)
   {
      // verifico tutta la lista dell'attributo per considerare 
      // anche eventuali operatori matematici
      if (gsui_GetAttribMathOp(EntColValues, i, &Value, Operat, &Perc) == GS_GOOD)
      {
         // Se ci sono operatori matematici di calcolo bisogna rileggere il DB
         ToReRead = TRUE;
         gsui_AddAttribMathOp(DiffEntColValues, gsc_nth(0, rbField)->resval.rstring,
                              Value, Operat, Perc);
      }
      else
      {
         rbValue = gsc_nth(1, rbField);
         rbPrevValue = gsc_nth(1, PrevEntColValues.nth(i));

         if (gsc_equal(rbValue, rbPrevValue) == GS_BAD) // case sensitive
         {  // sono diversi
            if ((DiffEntColValues += acutBuildList(RTLB,
                                                   RTSTR, gsc_nth(0, rbField)->resval.rstring, 0)) == NULL)
               return;
            if ((DiffEntColValues += gsc_copybuf(rbValue)) == NULL) return;
            if ((DiffEntColValues += acutBuildList(RTLE, 0)) == NULL) return;
         }
      }
      i++;
   }
   if (DiffEntColValues.GetCount() == 0)
   {
      m_OK.EnableWindow(FALSE);
      return;
   }

   if (DiffEntColValues.link_head(acutBuildList(RTLB, 0)) == NULL) return;
   if ((DiffEntColValues += acutBuildList(RTLE, 0)) == NULL) return;

   if (IsForClass()) // gestione entità
   {
      // lock DB
      HCURSOR PrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

      old_reactor_abilit = gsc_disable_reactors(); // disabilito i reattori di GEOsim
     
      if (pActualCls->upd_data(KeyList, DiffEntColValues, RECORD_MOD, GS_GOOD) != GS_GOOD)
      {
         ToReRead = TRUE;
         gsui_alert(_T("Aggiornamento fallito."));
         gsc_print_error();
      }
      else // Se ci sono alcune entità non aggiornate
         if (get_REFUSED_SS()->length() > 0)
         {
            ToReRead = TRUE;
            gsui_alert(_T("Alcune entità non sono state aggiornate."));
         }

      // ripristino il controllo sul reattore come in precedenza
      if (old_reactor_abilit == GS_GOOD) gsc_enable_reactors();

      // Se ci sono funzioni di calcolo bisogna rileggere il DB
      if (!ToReRead && pActualCls->ptr_attrib_list()->is_calculated() == GS_GOOD)
         ToReRead = TRUE;

      if (ToReRead)
      {
         FillClsRecords();
         DisplayRecords(); // visualizza i dati letti
      }
      else
      {
         presbuf pEntColValues;

         // Sostituisco i valori cambiati dall'utente
         i = 0;
         while ((rbField = DiffEntColValues.nth(i++)) != NULL)
         {
            rbValue = gsc_nth(1, rbField);
            PrevEntColValues.CdrAssocSubst(rbField->rbnext->resval.rstring, rbValue);

            // sostituisco i valori anche nella lista ColValues
            pEntColValues = ColValues.get_head();
            while (pEntColValues != NULL)
            {
               gsc_RbSubst(gsc_assoc(rbField->rbnext->resval.rstring, pEntColValues, FALSE)->rbnext->rbnext, rbValue);
               // vado alla chiusa tonda successiva
               pEntColValues = gsc_scorri(pEntColValues);
               // mi posiziono sulla aperta tonda successiva
               pEntColValues = pEntColValues->rbnext;
            }
         }
      }

      SetCursor(PrevCursor);
   }
   else // gestione schede secondarie
   {
      long key_pri;

      // lock DB
      HCURSOR PrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

      old_reactor_abilit = gsc_disable_reactors(); // disabilito i reattori di GEOsim

      if (!(rbValue = MotherColValues.CdrAssoc(pActualSec->ptr_info()->key_pri.get_name())))
         { SetCursor(PrevCursor); return; }
      gsc_rb2Lng(rbValue, &key_pri);

      if (pActualSec->upd_data(key_pri, KeyList, DiffEntColValues) != GS_GOOD)
      {
         ToReRead = TRUE;
         gsui_alert(_T("Aggiornamento fallito."));
         gsc_print_error();
      }

      // ripristino il controllo sul reattore come in precedenza
      if (old_reactor_abilit == GS_GOOD) gsc_enable_reactors();

      // Se ci sono funzioni di calcolo bisogna rileggere il DB
      if (!ToReRead && pActualSec->ptr_attrib_list()->is_calculated() == GS_GOOD)
         ToReRead = TRUE;

      if (ToReRead)
         // TRUE per segnalare che si tratta un semplice refresh
         SelchangeClasslistCombo(m_ClassAndSecListCombo.GetCurSel(), TRUE);
      else
      {
         presbuf pEntColValues;

         // Sostituisco i valori cambiati dall'utente
         i = 0;
         while ((rbField = DiffEntColValues.nth(i++)) != NULL)
         {
            rbValue = gsc_nth(1, rbField);
            PrevEntColValues.CdrAssocSubst(rbField->rbnext->resval.rstring, rbValue);

            // sostituisco i valori anche nella lista ColValues
            pEntColValues = ColValues.get_head();
            while (pEntColValues != NULL)
            {
               gsc_RbSubst(gsc_assoc(rbField->rbnext->resval.rstring, pEntColValues, FALSE)->rbnext->rbnext, rbValue);
               // vado alla chiusa tonda successiva
               pEntColValues = gsc_scorri(pEntColValues);
               // mi posiziono sulla aperta tonda successiva
               pEntColValues = pEntColValues->rbnext;
            }
         }
      }

      SetCursor(PrevCursor);
   }

   m_OK.EnableWindow(FALSE);
}

void CEntDBValuesListDlg::OnOkHoriz() 
{
   C_RB_LIST _ColValues;
   bool      ToReRead = FALSE;
   long      Key;
   presbuf   pRb;

   // Aggiorno eventuale modifica valore
   if (TempItemValueChanged)
   {
      if (!OnChangeValue(TempItemValue))
      {
         EnsureVisibleItem();
         m_EntDBValuesList.SetItemState(ActualItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
         EditValue();

         return;
      }
      TempItemValueChanged = FALSE;
   }

   if (!(pRb = (presbuf) m_EntDBValuesList.GetItemData(ActualItem))) return;
   if (_ColValues << gsc_listcopy(pRb) == NULL) return;

   if (IsForClass()) // gestione entità
   {
      presbuf pEntColValues;
      int     old_reactor_abilit;

      // lock DB
      HCURSOR PrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

      old_reactor_abilit = gsc_disable_reactors(); // disabilito i reattori di GEOsim
     
      if (!(pRb = _ColValues.CdrAssoc(pActualCls->ptr_info()->key_attrib.get_name())))
         return;
      if (gsc_rb2Lng(pRb, &Key) != GS_GOOD) return;
      if (pActualCls->upd_data(Key, _ColValues, NULL, RECORD_MOD) != GS_GOOD)
      {
         ToReRead = TRUE;
         gsui_alert(_T("Aggiornamento fallito."));
         gsc_print_error();
      }
      else // Se ci sono alcune entità non aggiornate
         if (get_REFUSED_SS()->length() > 0)
         {
            ToReRead = TRUE;
            gsui_alert(_T("Alcune entità non sono state aggiornate."));
         }

      // ripristino il controllo sul reattore come in precedenza
      if (old_reactor_abilit == GS_GOOD) gsc_enable_reactors();

      // Se ci sono funzioni di calcolo bisogna rileggere il DB
      if (ToReRead)
      {
         FillClsRecords();
         DisplayRecords(); // visualizza i dati letti
      }
      else 
      if (pActualCls->ptr_attrib_list()->is_calculated() == GS_GOOD)
      {
         int i = 0;

         if (pActualCls->query_data(Key, _ColValues) == GS_BAD) return;
         
         // aggiorno la lista ColValues
         pRb = (presbuf) m_EntDBValuesList.GetItemData(ActualItem);
         pEntColValues = _ColValues.get_head();
         while (pEntColValues != NULL)
         {
            gsc_RbSubst(pRb, pEntColValues);
            pEntColValues = pEntColValues->rbnext;
            pRb           = pRb->rbnext;
         }
         // aggiorno solo la una riga
         DisplayRecords(ActualItem);
      }

      EntColValues.remove_all();

      pEntColValues = ColValues.get_head();
      while (pEntColValues != NULL)
      {
         if (_ColValues << gsc_listcopy(pEntColValues) == NULL) return;

         if (!EntColValues.get_head()) _ColValues.copy(EntColValues);
         else // Confronto i campi annullando quelli diversi
            EntColValues.SubstRTNONEtoDifferent(_ColValues); // no case sensitive

         // vado alla chiusa tonda successiva
         pEntColValues = gsc_scorri(pEntColValues);
         // mi posiziono sulla aperta tonda successiva
         pEntColValues = pEntColValues->rbnext;
      }

      EntColValues.copy(PrevEntColValues);

      SetCursor(PrevCursor);
   }
   else // gestione schede secondarie
   {
      presbuf pEntColValues;
      int     old_reactor_abilit;
      long    key_pri;

      // lock DB
      HCURSOR PrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

      old_reactor_abilit = gsc_disable_reactors(); // disabilito i reattori di GEOsim
     
      if (!(pRb = MotherColValues.CdrAssoc(pActualSec->ptr_info()->key_pri.get_name())))
         { SetCursor(PrevCursor); return; }
      gsc_rb2Lng(pRb, &key_pri);

      if (!(pRb = _ColValues.CdrAssoc(pActualSec->ptr_info()->key_attrib.get_name())))
         { SetCursor(PrevCursor); return; }
      if (gsc_rb2Lng(pRb, &Key) != GS_GOOD) { SetCursor(PrevCursor); return; }
      if (pActualSec->upd_data(key_pri, Key, _ColValues) != GS_GOOD)
      {
         ToReRead = TRUE;
         gsui_alert(_T("Aggiornamento fallito."));
         gsc_print_error();
      }

      // ripristino il controllo sul reattore come in precedenza
      if (old_reactor_abilit == GS_GOOD) gsc_enable_reactors();

      // Se ci sono funzioni di calcolo bisogna rileggere il DB
      if (ToReRead)
      {
         FillSecRecords();
         DisplayRecords(); // visualizza i dati letti
      }
      else 
      if (pActualSec->ptr_attrib_list()->is_calculated() == GS_GOOD)
      {
         int i = 0;

         if (pActualSec->query_data(Key, _ColValues) == GS_BAD) return;
         
         // aggiorno la lista ColValues
         pRb = (presbuf) m_EntDBValuesList.GetItemData(ActualItem);
         pEntColValues = _ColValues.get_head();
         while (pEntColValues != NULL)
         {
            gsc_RbSubst(pRb, pEntColValues);
            pEntColValues = pEntColValues->rbnext;
            pRb           = pRb->rbnext;
         }
         // aggiorno solo la una riga
         DisplayRecords(ActualItem);
      }

      EntColValues.remove_all();

      pEntColValues = ColValues.get_head();
      while (pEntColValues != NULL)
      {
         if (_ColValues << gsc_listcopy(pEntColValues) == NULL) { SetCursor(PrevCursor); return; }

         if (!EntColValues.get_head()) _ColValues.copy(EntColValues);
         else // Confronto i campi annullando quelli diversi
            EntColValues.SubstRTNONEtoDifferent(_ColValues); // no case sensitive

         // vado alla chiusa tonda successiva
         pEntColValues = gsc_scorri(pEntColValues);
         // mi posiziono sulla aperta tonda successiva
         pEntColValues = pEntColValues->rbnext;
      }

      EntColValues.copy(PrevEntColValues);

      SetCursor(PrevCursor);
   }

   m_OK.EnableWindow(FALSE);
}

BOOL CEntDBValuesListDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if ((pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONDBLCLK)
       && pMsg->wParam == MK_LBUTTON && pMsg->hwnd == m_ComboButton.m_hWnd)
   {
      OnComboButton();
      return 1;
   }

   DisplayTooltip(pMsg);

   return CPropertyPage::PreTranslateMessage(pMsg);
}

void CEntDBValuesListDlg::OnComboButton() 
{
   C_ATTRIB           *pAttrib;
   C_STRING           SupportFile;
   ValuesListTypeEnum FileType;
   C_2STR_LIST        ValuesList;
   C_2STR             *pValues;
   LV_ITEM            lvitem;
   int                iRow = 0, _prj = 0, _cls = 0, _sub = 0, _sec = 0;
	CRect              Rect;
   C_ATTRIB_LIST      *pAttribList;
   C_DBCONNECTION     *pConn;

   if (IsForClass()) // gestione entità
   {
      if (!pActualCls->ptr_info() || !(pAttribList = pActualCls->ptr_attrib_list()) ||
         pActualCls->ptr_id()->abilit != GSUpdateableData ||
         !(pAttrib = get_AttribPtr()))
         return;
      _prj  = pActualCls->get_PrjId();
      _cls  = pActualCls->ptr_id()->code;
      _sub  = pActualCls->ptr_id()->sub_code;
      pConn = pActualCls->ptr_info()->pOldTabConn;
   }
   else // gestione schede secondarie
   {
      if (pActualSec->type != GSInternalSecondaryTable ||
          !(pAttribList = pActualSec->ptr_attrib_list()) ||
          !(pAttrib = get_AttribPtr()))
         return;
      _prj  = pActualSec->ptr_class()->get_PrjId();
      _cls  = pActualSec->ptr_class()->ptr_id()->code;
      _sub  = pActualSec->ptr_class()->ptr_id()->sub_code;
      _sec  = pActualSec->code;
      pConn = pActualSec->ptr_info()->pOldTabConn;
   }

   if (m_List.IsWindowVisible()) // Se era già visibile la spegno
   {
      m_List.ShowWindow(SW_HIDE);
      m_ComboButton.SetFocus();
      return;
   }

   if (gsc_FindSupportFiles(pAttrib->get_name(), _prj, _cls, _sub, _sec,
                            SupportFile, &FileType) == GS_BAD)
      return;

   // Carico la lista di eventuali condizioni GEOlisp
   FILE *f;
   bool  Unicode = false;

   if ((f = gsc_open_profile(SupportFile, READONLY, MORETESTS, &Unicode)) != NULL)
   {
      C_STR_LIST CondList;
      C_STR      *pCond;
      C_RB_LIST  _ColValues;

      if (m_VertAttrMode) // singola scheda
         EntColValues.copy(_ColValues);
      else // formato tabellare
      {
         presbuf rbValue = (presbuf) m_EntDBValuesList.GetItemData(ActualItem);
         _ColValues << gsc_listcopy(rbValue);
      }

      bool fromDB = true;
      gsc_get_profile(f, CondList, Unicode);
      if ((pCond = CondList.getFirstTrueGEOLispCond(_ColValues)))
         // Carico la lista di valori relativi alla sezione GEOLisp
         ValuesList.load(f, _T(';'), pCond->get_name(), fromDB, Unicode, &_ColValues);
      else
         // Carico la lista di valori di default
         ValuesList.load(f, _T(';'), NULL, fromDB, Unicode, &_ColValues);

      gsc_fclose(f);

      // Se il campo NON è obbligatorio aggiungo una riga vuota per valore nullo se non ancora presente in lista (roby 2018)
      if (pAttrib->man == GS_BAD)
         if (ValuesList.search_name(_T("")) == NULL)
            ValuesList.add_tail_2str(_T(""), _T(""));
   }

   m_List.DeleteAllItems();
   while (m_List.DeleteColumn(0) != 0); // svuoto la lista

   lvitem.mask = LVIF_TEXT;

   switch (FileType)
   {
      case TAB: case REF:
	      m_List.InsertColumn(0, _T(""), LVCFMT_LEFT, 1, 0);
	      m_List.InsertColumn(1, _T(""), LVCFMT_LEFT, 1, 1);
         pValues = (C_2STR *) ValuesList.get_head();
         while (pValues)
         {  // Valore
		      lvitem.iItem    = iRow++;
            lvitem.iSubItem = 0;
            lvitem.pszText  = pValues->get_name();
         	m_List.InsertItem(&lvitem);

            // Note
            lvitem.iSubItem++;
            lvitem.pszText  = pValues->get_name2();
   	      m_List.SetItem(&lvitem);

            pValues = (C_2STR *) ValuesList.get_next();
         }
         break;

      case FDF: case DEF:
	      m_List.InsertColumn(0, _T(""), LVCFMT_LEFT, 1, 0);
         lvitem.iSubItem = 0;
         pValues = (C_2STR *) ValuesList.get_head();
         while (pValues)
         {  // Valore
		      lvitem.iItem   = iRow++;
            lvitem.pszText = pValues->get_name();
         	m_List.InsertItem(&lvitem);

            pValues = (C_2STR *) ValuesList.get_next();
         }
         break;
   }

   // dimensiono le colonne - inizio
   m_EntDBValuesList.GetSubItemRect(ActualItem, 1, LVIR_BOUNDS, Rect);

   int     MaxWidth1Column = 0, Width1Column, MaxWidth2Column = 0;
   int     VirtualCtrlWidth = Rect.Width() - 2;
   CString ItemText;

   // Se è comparsa la barra di scorrimento verticale
   // lo spazio a disposizione diminuisce
   if (m_List.GetItemCount() > m_List.GetCountPerPage()) VirtualCtrlWidth -= 16;

   switch (FileType)
   {
      case TAB: case REF:
      {
         int Width2Column;

         // Calcolo la larghezza delle colonne
         for (int i = 0; i < m_List.GetItemCount(); i++)
         {
            ItemText = m_List.GetItemText(i, 0);
            Width1Column = m_List.GetStringWidth((LPCTSTR) ItemText) + 15;
            if (MaxWidth1Column < Width1Column)
               MaxWidth1Column = Width1Column;

            ItemText = m_List.GetItemText(i, 1);
            Width2Column = m_List.GetStringWidth((LPCTSTR) ItemText) + 15;
            if (MaxWidth2Column < Width2Column)
               MaxWidth2Column = Width2Column;
         }

         m_List.SetColumnWidth(0, MaxWidth1Column);
         // Se lo spazio disponibile è maggiore lo uso tutto
         if (VirtualCtrlWidth - MaxWidth1Column > MaxWidth2Column)
            MaxWidth2Column = VirtualCtrlWidth - MaxWidth1Column;
         m_List.SetColumnWidth(1, MaxWidth2Column);

         break;
      }

      case FDF: case DEF:
         // Calcolo la larghezza delle colonne
         for (int i = 0; i < m_List.GetItemCount(); i++)
         {
            ItemText = m_List.GetItemText(i, 0);
            Width1Column = m_List.GetStringWidth((LPCTSTR) ItemText) + 15;
            if (MaxWidth1Column < Width1Column)
               MaxWidth1Column = Width1Column;
         }

         m_List.SetColumnWidth(0, MaxWidth1Column);

         break;
   }
   // dimensiono le colonne - fine

   // Calcolo altezza lista
	CRect EntDBValuesListRect;
   int   TopList = Rect.top, LeftList, HeightList;

   m_EntDBValuesList.GetWindowRect(&EntDBValuesListRect);
   EntDBValuesListRect.bottom -= 10;
   // max 10 righe
   m_List.GetItemRect(0, Rect, LVIR_BOUNDS);
   HeightList = Rect.Height() * m_List.GetItemCount() + 2;

   m_EntDBValuesList.GetSubItemRect(ActualItem, ActualSubItem, LVIR_BOUNDS, Rect);
   LeftList = Rect.left;

   // Se è comparsa la barra di scorrimento orizzontale
   if (MaxWidth1Column + MaxWidth2Column > VirtualCtrlWidth) HeightList += 16;

   // Se la lista in giù ci sta
   if (Rect.bottom + HeightList + EntDBValuesListRect.top <= EntDBValuesListRect.bottom)
      TopList = Rect.bottom;
   else
      // Se la lista in sù ci sta
      if (Rect.top - HeightList >= 0)	// la sposto in sù
         TopList = Rect.top - HeightList;
      else
      {  // cerco dove c'è più spazio e ridimensiono la lista
         if (Rect.top > EntDBValuesListRect.Height() - Rect.bottom)
         {
            TopList = 0; // La lista va nella parte superiore
            HeightList = Rect.top;
         }
         else
         {
            TopList = Rect.bottom; // La lista va nella parte inferiore
            HeightList = EntDBValuesListRect.Height() - TopList;
            // Se è comparsa la barra di scorrimento orizzontale
            BOOL bScroll = (m_EntDBValuesList.GetStyle() & WS_HSCROLL);
            if (bScroll) HeightList -= (16 + 1);
         }
      }

   m_List.SetWindowPos(NULL, LeftList, TopList, Rect.Width(), HeightList, SWP_NOZORDER);
   
   CString PrevValue = m_EntDBValuesList.GetItemText(ActualItem, 1);

   if (PrevValue.GetLength() > 0)
   {
      LVFINDINFO FindInfo;
      int        nItem;

      FindInfo.flags = LVFI_STRING;
      FindInfo.psz = (LPCTSTR) PrevValue;
      if ((nItem = m_List.FindItem(&FindInfo)) != -1)
      {
         m_List.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
         m_List.EnsureVisible(nItem, FALSE);
      }
      else // deseleziono eventuale selezione
      {
         POSITION Pos = m_List.GetFirstSelectedItemPosition();
         if (Pos) m_List.SetItemState((int) Pos - 1, (UINT) 0, LVIS_SELECTED|LVIS_FOCUSED);
      }
   }
   else // deseleziono eventuale selezione
   {
      POSITION Pos = m_List.GetFirstSelectedItemPosition();
      if (Pos) m_List.SetItemState((int) Pos - 1, (UINT) 0, LVIS_SELECTED|LVIS_FOCUSED);
   }

   m_List.ShowWindow(SW_SHOW);
   m_List.SetFocus();
}

void CEntDBValuesListDlg::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

   if (pNMListView->iItem == -1) 
      { *pResult = 0; return; } // nessuna selezione
   TempItemValue = m_List.GetItemText(pNMListView->iItem, 0);
   TempItemValueChanged = TRUE;
   m_OK.EnableWindow(TRUE);

   if (GoNextUpdItem(ActualItem, ActualSubItem))
      EditValue();
   else
   {
      EditValue();
      m_List.ShowWindow(SW_HIDE);
   }

   *pResult = 1;	
}

void CEntDBValuesListDlg::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

   if (pNMListView->iItem == -1) 
      { *pResult = 0; return; } // nessuna selezione
   TempItemValue = m_List.GetItemText(pNMListView->iItem, 0);
   TempItemValueChanged = TRUE;
   m_OK.EnableWindow(TRUE);
   
	*pResult = 0;
}

void CEntDBValuesListDlg::OnKillfocusList(NMHDR* pNMHDR, LRESULT* pResult) 
{
   if (m_List.IsWindowVisible())
      m_List.ShowWindow(SW_HIDE);
	
	*pResult = 0;
}

void CEntDBValuesListDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CMenu Menu;
   CMenu *SubMenu;
   CRect Rect;
   HWND  ActiveWnd = NULL;

   m_Edit.GetWindowRect(&Rect);
   Menu.DestroyMenu();

   if (EditVis && Rect.PtInRect(point)) ActiveWnd = m_Edit.GetSafeHwnd();
   else 
   {
      m_MaskedEdit.GetWindowRect(&Rect);
      if (MaskedEditVis && Rect.PtInRect(point)) ActiveWnd = m_MaskedEdit.GetSafeHwnd();
   }

   if (ActiveWnd)
   {
      int            Start, End;
      int            _prj = 0, _cls = 0, _sub = 0, _sec = 0;
      C_DBCONNECTION *pConn;
      C_ATTRIB_LIST  *pAttribList;
      C_ATTRIB       *pAttrib;

      if (!(pAttrib = get_AttribPtr())) return;

      Menu.LoadMenu(IDR_MENU_EDIT);
      SubMenu = Menu.GetSubMenu(0);

      SetMenuItemBitmaps(SubMenu->m_hMenu, IDC_DB_LIST, MF_BYCOMMAND, hBmpDBList, hBmpDBList);
      SetMenuItemBitmaps(SubMenu->m_hMenu, IDC_SUGGEST, MF_BYCOMMAND, hBmpSuggest, hBmpSuggest);
      SetMenuItemBitmaps(SubMenu->m_hMenu, IDC_MATH_OP, MF_BYCOMMAND, hBmpMathOp, hBmpMathOp);

      if (m_Zoom)
         CheckMenuItem(SubMenu->m_hMenu, IDC_ZOOM, MF_CHECKED | MF_BYCOMMAND);
      else
         CheckMenuItem(SubMenu->m_hMenu, IDC_ZOOM, MF_UNCHECKED | MF_BYCOMMAND);

      if (m_Highlight)
         CheckMenuItem(SubMenu->m_hMenu, IDC_HIGHLIGHT, MF_CHECKED | MF_BYCOMMAND);
      else
         CheckMenuItem(SubMenu->m_hMenu, IDC_HIGHLIGHT, MF_UNCHECKED | MF_BYCOMMAND);

      if (m_Frozen)
         CheckMenuItem(SubMenu->m_hMenu, IDC_FROZEN_PANEL, MF_CHECKED | MF_BYCOMMAND);
      else
         CheckMenuItem(SubMenu->m_hMenu, IDC_FROZEN_PANEL, MF_UNCHECKED | MF_BYCOMMAND);

      if (IsForClass()) // gestione entità
      {
         _prj  = pActualCls->get_PrjId();
         _cls  = pActualCls->ptr_id()->code;
         _sub  = pActualCls->ptr_id()->sub_code;
         pConn = pActualCls->ptr_info()->pOldTabConn;
         pAttribList = pActualCls->ptr_attrib_list();
      }
      else // gestione schede secondarie
      {
         _prj  = pActualSec->ptr_class()->get_PrjId();
         _cls  = pActualSec->ptr_class()->ptr_id()->code;
         _sub  = pActualSec->ptr_class()->ptr_id()->sub_code;
         _sec  = pActualSec->code;
         pConn = pActualSec->ptr_info()->pOldTabConn;
         pAttribList = pActualSec->ptr_attrib_list();
         SubMenu->EnableMenuItem(IDC_ZOOM, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
         SubMenu->EnableMenuItem(IDC_HIGHLIGHT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
         SubMenu->EnableMenuItem(IDC_FROZEN_PANEL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }

      // Se ci sono file di scelta obbligata
      C_STRING OutPath;
      ValuesListTypeEnum FileType;
      if (gsc_FindSupportFiles(pAttrib->get_name(), _prj, _cls, _sub, _sec,
                               OutPath, &FileType) == GS_GOOD)
         if (FileType == TAB || FileType == FDF) // Se ci sono file di scelta obbligata
         {
            SubMenu->EnableMenuItem(IDC_DB_LIST, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);   
            SubMenu->EnableMenuItem(IDC_SUGGEST, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
         }

      // La voce "Operatori matematici..." deve essere disponibile solo per
      // attributi di tipo numerico su modalità singola scheda
      if (pAttribList->init_ADOType(pConn) == GS_BAD)
      {
         SubMenu->EnableMenuItem(IDC_MATH_OP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
         SubMenu->EnableMenuItem(IDC_SUGGEST, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }
      else
         if (gsc_DBIsNumeric(pAttrib->ADOType) != GS_GOOD || !m_VertAttrMode) // No singola scheda
         {
            SubMenu->EnableMenuItem(IDC_MATH_OP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            // Se non è numerico nè data nè timestamp nè carattere
            if (gsc_DBIsDate(pAttrib->ADOType) != GS_GOOD &&
                gsc_DBIsChar(pAttrib->ADOType) != GS_GOOD &&
                gsc_DBIsTimestamp(pAttrib->ADOType) != GS_GOOD)
               SubMenu->EnableMenuItem(IDC_SUGGEST, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
         }

      if (ActiveWnd == m_Edit.GetSafeHwnd())
      {
         if (m_Edit.CanUndo() == FALSE)
            SubMenu->EnableMenuItem(IDC_UNDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }
      else
         SubMenu->EnableMenuItem(IDC_UNDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

      if (ActiveWnd == m_Edit.GetSafeHwnd())
         m_Edit.GetSel(Start, End);
      else
         m_MaskedEdit.GetSel(Start, End);

      if (Start == End)
      {
         SubMenu->EnableMenuItem(IDC_CUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
         SubMenu->EnableMenuItem(IDC_COPY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
         SubMenu->EnableMenuItem(IDC_REMOVE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }

      if (OpenClipboard())
      {
         UINT Format = EnumClipboardFormats(0);

         if (Format != CF_UNICODETEXT && Format != CF_TEXT)
            SubMenu->EnableMenuItem(IDC_PASTE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
         CloseClipboard();
      }

      if (ActiveWnd == m_Edit.GetSafeHwnd())
      {
         if (m_Edit.GetWindowTextLength() == 0)
            SubMenu->EnableMenuItem(IDC_SELECT_ALL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }
      else
      {
         if (m_MaskedEdit.GetWindowTextLength() == 0)
            SubMenu->EnableMenuItem(IDC_SELECT_ALL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }

      SetMenuItemBitmaps(SubMenu->m_hMenu, IDC_PRINT_SHEET, MF_BYCOMMAND, hBmpPrinter, hBmpPrinter);

      SetMenuItemBitmaps(SubMenu->m_hMenu, IDC_PRINT_SHEETS, MF_BYCOMMAND, hBmpPrinter, hBmpPrinter);
      if (m_VertAttrMode) 
            SubMenu->EnableMenuItem(IDC_PRINT_SHEETS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

      SubMenu->TrackPopupMenu(0, point.x, point.y, this, NULL);
   }
   else
   {
      Menu.LoadMenu(IDR_MENU_SHEET);
      SubMenu = Menu.GetSubMenu(0);

      SetMenuItemBitmaps(SubMenu->m_hMenu, IDC_PRINT_SHEET, MF_BYCOMMAND, hBmpPrinter, hBmpPrinter);
      SetMenuItemBitmaps(SubMenu->m_hMenu, IDC_PRINT_SHEETS, MF_BYCOMMAND, hBmpPrinter, hBmpPrinter);

      if (IsForClass()) // gestione entità
      {
         if (!pActualCls->ptr_info() || !pActualCls->ptr_attrib_list())
            SubMenu->EnableMenuItem(IDC_COPY_SHEET, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

         if (!pActualCls->ptr_info() || !pActualCls->ptr_attrib_list())
            SubMenu->EnableMenuItem(IDC_PRINT_SHEET, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

         if (m_VertAttrMode || 
             (!pActualCls->ptr_info() || !pActualCls->ptr_attrib_list())) 
            SubMenu->EnableMenuItem(IDC_PRINT_SHEETS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }
      else
      {
         if (m_VertAttrMode) 
            SubMenu->EnableMenuItem(IDC_PRINT_SHEETS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      }

      if (m_Zoom)
         CheckMenuItem(SubMenu->m_hMenu, IDC_ZOOM, MF_CHECKED | MF_BYCOMMAND);
      else
         CheckMenuItem(SubMenu->m_hMenu, IDC_ZOOM, MF_UNCHECKED | MF_BYCOMMAND);

      if (m_Highlight)
         CheckMenuItem(SubMenu->m_hMenu, IDC_HIGHLIGHT, MF_CHECKED | MF_BYCOMMAND);
      else
         CheckMenuItem(SubMenu->m_hMenu, IDC_HIGHLIGHT, MF_UNCHECKED | MF_BYCOMMAND);

      if (m_Frozen)
         CheckMenuItem(SubMenu->m_hMenu, IDC_FROZEN_PANEL, MF_CHECKED | MF_BYCOMMAND);
      else
         CheckMenuItem(SubMenu->m_hMenu, IDC_FROZEN_PANEL, MF_UNCHECKED | MF_BYCOMMAND);

      SubMenu->TrackPopupMenu(0, point.x, point.y, this, NULL);
   }
}

void CEntDBValuesListDlg::OnDBList()
{
   C_STRING Title, AttribValue;
   C_ATTRIB *pAttrib;
   int      m_cls, m_sub, m_sec = 0;

   if (!get_GS_CURRENT_WRK_SESSION()) return;

   if (IsForClass()) // gestione entità
   {
      if (!pActualCls->ptr_info() ||
         pActualCls->ptr_id()->abilit != GSUpdateableData ||
         !(pAttrib = get_AttribPtr()))
         return;

      m_cls = pActualCls->ptr_id()->code;
      m_sub = pActualCls->ptr_id()->sub_code;
   }
   else // gestione schede secondarie
   {
      if (!(pAttrib = get_AttribPtr())) return;
   
      m_cls = pActualSec->ptr_class()->ptr_id()->code;
      m_sub = pActualSec->ptr_class()->ptr_id()->sub_code;
      m_sec = pActualSec->get_key();
   }

   Title = _T("GEOsim - Lista valori già inseriti nel database");
   if (gsc_dd_sel_uniqueVal(get_GS_CURRENT_WRK_SESSION()->get_PrjId(), m_cls, m_sub, m_sec,
                            pAttrib->name, AttribValue, Title) == GS_GOOD)
   {
      TempItemValue = AttribValue.get_name();
      TempItemValueChanged = TRUE;
      m_OK.EnableWindow(TRUE);

      GoNextUpdItem(ActualItem, ActualSubItem);
      EditValue();
   }
   else
      EditValue();

   return;
}

void CEntDBValuesListDlg::OnSuggest() 
{
   C_STRING dummy; 
   C_ATTRIB *pAttrib;

   if (IsForClass()) // gestione entità
   {
      if (!pActualCls->ptr_info() ||
         pActualCls->ptr_id()->abilit != GSUpdateableData ||
         !(pAttrib = get_AttribPtr()))
         return;

      if (pActualCls->ptr_attrib_list()->init_ADOType(pActualCls->ptr_info()->pOldTabConn) == GS_BAD)
         return;
   }
   else // gestione schede secondarie
   {
      if (!(pAttrib = get_AttribPtr())) return;

      if (pActualSec->ptr_attrib_list()->init_ADOType(pActualSec->ptr_info()->pOldTabConn) == GS_BAD)
         return;
   }

   dummy = ((LPCTSTR) TempItemValue);

   // Considero il valore come carattere
   if (gsc_DBIsChar(pAttrib->ADOType) == GS_GOOD)
   {
      if (gsc_ddSuggestDBValue(dummy, adVarChar) == GS_BAD || dummy.len() == 0) return;
   }
   else // Considero il valore come numero
   if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD)
   {
      if (gsc_ddSuggestDBValue(dummy, adDouble) == GS_BAD || dummy.len() == 0) return;
   }
   else // Considero il valore come data o timestamp
   if (gsc_DBIsDate(pAttrib->ADOType) == GS_GOOD || gsc_DBIsTimestamp(pAttrib->ADOType) == GS_GOOD)
   {
      if (gsc_ddSuggestDBValue(dummy, adDate) == GS_BAD || dummy.len() == 0) return;
   }
   else return;

   TempItemValue = dummy.get_name();
   TempItemValueChanged = TRUE;
   m_OK.EnableWindow(TRUE);

   GoNextUpdItem(ActualItem, ActualSubItem);
   EditValue();
}

void CEntDBValuesListDlg::OnMathOp() 
{
   CMathOperatDlg Dlg;
   double         Value;
   CString        Operat;
   bool           Perc;

   if (gsui_GetAttribMathOp(TempItemValue, &Value, Operat, &Perc) == GS_GOOD)
   {
      Dlg.m_Value  = Value;
      Dlg.m_Operat = Operat;
      Dlg.m_Perc   = Perc;
   }

   if (Dlg.DoModal() == IDOK)
   {
      gsui_GetAttribMathOpMsg(Dlg.m_Value, Dlg.m_Operat, Dlg.m_Perc, TempItemValue);
      TempItemValueChanged = TRUE;
      m_OK.EnableWindow(TRUE);

      GoNextUpdItem(ActualItem, ActualSubItem);
      EditValue();
   }
}

void CEntDBValuesListDlg::OnEditUndo()
{
   if (EditVis)
      if (m_Edit.CanUndo()) m_Edit.Undo();
}
void CEntDBValuesListDlg::OnEditCopy()
{
   if (EditVis) m_Edit.Copy();
   else m_MaskedEdit.Copy();
}
void CEntDBValuesListDlg::OnEditCopySheet()
{
   C_STRING      Msg, FldsValuesStr;
   C_ATTRIB_LIST *pAttribList;
   int           m_cls, m_sub, m_sec = 0;
   presbuf       pVal;
   C_RB_LIST     _ColValues;

   if (IsForClass()) // gestione entità
   {
      if (!pActualCls) return;
      if (!(pAttribList = pActualCls->ptr_attrib_list())) return;
      pActualCls->get_CompleteName(Msg);

      m_cls = pActualCls->ptr_id()->code;
      m_sub = pActualCls->ptr_id()->sub_code;
   }
   else // gestione schede secondarie
   {
      if (!pActualSec) return;

      pAttribList = pActualSec->ptr_attrib_list();
      Msg         = pActualSec->name;
      m_cls = pActualSec->ptr_class()->ptr_id()->code;
      m_sub = pActualSec->ptr_class()->ptr_id()->sub_code;
      m_sec = pActualSec->get_key();
   }

   if (m_VertAttrMode) // singola scheda
      PrevEntColValues.copy(_ColValues);
   else // formato tabellare
   {
      if (!(pVal = (presbuf) m_EntDBValuesList.GetItemData(ActualItem))) return;
      _ColValues << gsc_listcopy(pVal);
   }

   // Formatto i valori in una stringa
   if (pAttribList->ParseToString(_ColValues, FldsValuesStr, FALSE,
                                  m_cls, m_sub, m_sec) == GS_GOOD)
   {
      Msg += GS_LFSTR;
      Msg += FldsValuesStr;

      // Inserisco il testo nella clipboard
      if (OpenClipboard()) // Apertura OK
      {
         HANDLE m_hMemory = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, 
                                        (Msg.len() + 1) * sizeof(TCHAR));

         if (EmptyClipboard())
         {
				TCHAR *pstr = (TCHAR *) GlobalLock(m_hMemory);

				wcscpy_s(pstr, Msg.len() + 1, Msg.get_name());
   			GlobalUnlock(m_hMemory);

            SetClipboardData(CF_UNICODETEXT, m_hMemory);
         }
         CloseClipboard();
      }
   }
}
void CEntDBValuesListDlg::OnEditCut()
{
   if (EditVis) m_Edit.Cut();
   else m_MaskedEdit.Cut();
}
void CEntDBValuesListDlg::OnEditPaste()
{
   if (EditVis) m_Edit.Paste();
   else m_MaskedEdit.Paste();
}
void CEntDBValuesListDlg::OnEditRemove()
{
   if (EditVis) m_Edit.ReplaceSel(_T(""), TRUE);
   else
   {
	   TRY { m_MaskedEdit.ReplaceSel(_T(""), TRUE); }
      CATCH_ALL(e) {}
	   END_CATCH_ALL
   }
}
void CEntDBValuesListDlg::OnEditSelectAll()
{
   if (EditVis) m_Edit.SetSel(0, -1);
   else m_MaskedEdit.SetSel(0, -1);
}


/*****************************************************************************/
/*.doc CEntDBValuesListDlg::SingleTo                                        */
/*+
  Questa funzione esporta il contenuto della scheda corrente in un
  file HTML e se richiesto avvia un editor per la stampa.
  Parametri
  C_STRING &Path;    Path completa file html
  bool RunAppl;      Se = TRUE viene lanciata l'applicazione registrata da windows
                     per aprire il file (default = TRUE).
  
  Restituisce GS_GOOD in caso di successo altrimenti restituisce GS_BAD.
-*/  
/************************************************************************************************************/
int CEntDBValuesListDlg::SingleToHtml(const TCHAR *Path, bool RunAppl)
{
   presbuf   pVal;
   C_RB_LIST _ColValues;
   long      gs_id;

   if (m_VertAttrMode) // singola scheda
      PrevEntColValues.copy(_ColValues);
   else // formato tabellare
   {
      if (!(pVal = (presbuf) m_EntDBValuesList.GetItemData(ActualItem)))
         return GS_BAD;
      _ColValues << gsc_listcopy(pVal);
   }

   if (IsForClass()) // gestione entità
   {
      if (pActualCls->ptr_info() == NULL) return GS_BAD;
      pVal = _ColValues.CdrAssoc(pActualCls->ptr_info()->key_attrib.get_name());
      if (!pVal) return GS_BAD;
      if (gsc_rb2Lng(pVal, &gs_id) == GS_BAD) return GS_BAD;
      if (pActualCls->data_to_html(Path, gs_id) == GS_BAD) return GS_BAD;
   }
   else // gestione schede secondarie
   {
      pVal = _ColValues.CdrAssoc(pActualSec->ptr_info()->key_attrib.get_name());
      if (!pVal) return GS_BAD;
      if (gsc_rb2Lng(pVal, &gs_id) == GS_BAD) return GS_BAD;
      if (pActualSec->data_to_html(Path, gs_id) == GS_BAD) return GS_BAD;
   }

   if (RunAppl)
      return gsc_DocumentExecute(Path, FALSE);

   return GS_GOOD;
}


void CEntDBValuesListDlg::OnPrintSheet()
{
   C_STRING Path;

   Path = get_GS_CURRENT_WRK_SESSION()->get_dir();
   Path += _T("\\TEMP\\VALUES.HTM");
   SingleToHtml(Path.get_name());
}


/*****************************************************************************/
/*.doc CEntDBValuesListDlg::MultiToHtml                                     */
/*+
  Questa funzione esporta il contenuto delle schede correnti in un
  file HTML e se richiesto avvia un editor per la stampa.
  Parametri
  const TCHAR *Path; Path completa file html
  bool RunAppl;      Se = TRUE viene lanciata l'applicazione registrata da windows
                     per aprire il file (default = TRUE).
  
  Restituisce GS_GOOD in caso di successo altrimenti restituisce GS_BAD.
-*/  
/************************************************************************************************************/
int CEntDBValuesListDlg::MultiToHtml(const TCHAR *Path, bool RunAppl)
{
   C_STRING      TitleBorderColor("#808080"), TitleBgColor("#c0c0c0");
   C_STRING      BorderColor("#00CCCC"), BgColor("#99FFFF");
   C_STRING      Msg, PrjName, ClassName, FieldName;
   C_ATTRIB_LIST *pAttribList;
   C_ATTRIB      *pAttrib;
   presbuf       pVal;
   int           i = 0, Result = GS_BAD;
   FILE          *file;
   C_RB_LIST     _ColValues;
   presbuf       rbValue;
   int           m_cls, m_sub, m_sec = 0;

   if (IsForClass()) // gestione entità
   {
      if (!(pAttribList = pActualCls->ptr_attrib_list())) return GS_BAD;
      PrjName = pActualCls->get_pPrj()->get_name();
      pActualCls->get_CompleteName(ClassName);

      m_cls = pActualCls->ptr_id()->code;
      m_sub = pActualCls->ptr_id()->sub_code;
   }
   else // gestione schede secondarie
   {
      pAttribList = pActualSec->ptr_attrib_list();
      PrjName     = pActualSec->ptr_class()->get_pPrj()->get_name();
      pActualSec->ptr_class()->get_CompleteName(ClassName);

      m_cls = pActualSec->ptr_class()->ptr_id()->code;
      m_sub = pActualSec->ptr_class()->ptr_id()->sub_code;
      m_sec = pActualSec->get_key();
   }

   PrjName.toHTML();
   ClassName.toHTML();

   if ((file = gsc_fopen(Path, _T("w"))) == NULL) return GS_BAD;

   do
   {
      // Intestazione
      if (fwprintf(file, _T("<html>\n<head>\n<title>Schede GEOsim</title>\n</head>\n<body bgcolor=\"#FFFFFF\">\n")) < 0)
         break;

      if (fwprintf(file, _T("\n<table bordercolor=\"%s\" bgcolor=\"%s\" width=\"100%%\" border=\"1\">"),
                         TitleBorderColor.get_name(), TitleBgColor.get_name()) < 0)
         break;

      // Progetto: Classe:
      if (fwprintf(file, _T("\n<tr><td align=\"center\"><b><font size=\"6\">Progetto: %s<br>Classe: %s"),
                   PrjName.get_name(), ClassName.get_name()) < 0)
         break;

      if (!IsForClass()) // gestione schede secondarie
      {
         Msg = pActualSec->name;
         Msg.toHTML();
         // Tabella secondaria:
         if (fwprintf(file, _T("<br>Tabella secondaria: %s"), Msg.get_name()) < 0)
            break;
      }

      if (fwprintf(file, _T("</font></b></td></tr></table><br>")) < 0)
         break;

      // intestazione tabella
      if (fwprintf(file, _T("\n<table bordercolor=\"%s\" cellspacing=\"2\" cellpadding=\"2\" border=\"1\">"),
                   BorderColor.get_name()) < 0)
         break;

      Result = GS_GOOD;

      // Intestazione
      if (!(rbValue = (presbuf) m_EntDBValuesList.GetItemData(0))) break;
      _ColValues << gsc_listcopy(rbValue);
      if (fwprintf(file, _T("\n<tr>")) < 0) break;
      
      pAttrib = (C_ATTRIB *) pAttribList->get_head();
      while (pAttrib)
      {
         FieldName = pAttrib->get_name();
         FieldName.toHTML();

         if (fwprintf(file, _T("\n<td align=\"center\" bgcolor=\"%s\" width=\"30%%\"><b>%s</b></td>"),
                      BgColor.get_name(), FieldName.get_name()) < 0)
            { Result = GS_BAD; break; }

         pAttrib = (C_ATTRIB *) pAttrib->get_next();
      }
      if (Result == GS_BAD) break;
      if (fwprintf(file, _T("</tr>")) < 0) { Result = GS_BAD; break; }

      // Valori
      for (i = 0; i < m_EntDBValuesList.GetItemCount(); i++)
      {
         if (!(rbValue = (presbuf) m_EntDBValuesList.GetItemData(i)))
            { Result = GS_BAD; break; }
         _ColValues << gsc_listcopy(rbValue);
         if (fwprintf(file, _T("\n<tr>")) < 0) { Result = GS_BAD; break; }

         pAttrib = (C_ATTRIB *) pAttribList->get_head();
         while (pAttrib)
         {
            pVal = _ColValues.CdrAssoc(pAttrib->get_name());
            Msg.clear();

            pAttrib->ParseToString(pVal, Msg, &_ColValues, m_cls, m_sub, m_sec);

            Msg.toHTML();
            if (fwprintf(file, _T("<td>%s</td>"), Msg.get_name()) < 0)
               { Result = GS_BAD; break; }
            pAttrib = (C_ATTRIB *) pAttrib->get_next();
         }
         if (Result == GS_BAD) break;
         if (fwprintf(file, _T("</tr>")) < 0) { Result = GS_BAD; break; }
      }      

      // fine tabella
      if (fwprintf(file, _T("\n</table></body></html>")) < 0) { Result = GS_BAD; break; }
   }
   while (0);

   gsc_fclose(file);

   if (Result == GS_GOOD && RunAppl)
      Result = gsc_DocumentExecute(Path, FALSE);

   return Result;
}


void CEntDBValuesListDlg::OnPrintSheets()
{
   C_STRING Path;

   Path = get_GS_CURRENT_WRK_SESSION()->get_dir();
   Path += _T("\\TEMP\\VALUES.HTM");
   MultiToHtml(Path.get_name());
}


void CEntDBValuesListDlg::OnZoom()
{
   C_INIT *pGS_GLOBALVAR = get_GS_GLOBALVAR();

   m_Zoom = !m_Zoom;
   if (pGS_GLOBALVAR)
   {
      pGS_GLOBALVAR->set_AutoZoom((m_Zoom) ? GS_GOOD : GS_BAD);
      pGS_GLOBALVAR->Save();
   }
}


void CEntDBValuesListDlg::OnHighlight()
{
   C_INIT *pGS_GLOBALVAR = get_GS_GLOBALVAR();

   m_Highlight = !m_Highlight;
   if (pGS_GLOBALVAR)
   {
      pGS_GLOBALVAR->set_AutoHighlight((m_Highlight) ? GS_GOOD : GS_BAD);
      pGS_GLOBALVAR->Save();
   }
}

void CEntDBValuesListDlg::OnFrozenPanel()
{ m_Frozen = !m_Frozen; }


///////////////////////////////////////////////////////////////////////////////
// FUNZIONI PER TOOLTIP - INIZIO
///////////////////////////////////////////////////////////////////////////////


// Le coordinate espresse da pt sono "Screen"
BOOL CEntDBValuesListDlg::DisplayTooltip(MSG* pMsg)
{
   CRect   Rect1, Rect2, ScreenRect;
   int     iItem, StartItem, EndItem, Width2Column;
   CString ItemText, PrevItemText, Value;

   if (!pToolTip || !pToolTip->m_hWnd) return TRUE;
   
   // tooltip per i bottoni
   if (pMsg->hwnd == m_VertAttrModeButton.m_hWnd || 
       pMsg->hwnd == m_GraphSel.m_hWnd ||
       pMsg->hwnd == m_AddSecButton.m_hWnd ||
       pMsg->hwnd == m_DelSecButton.m_hWnd ||
       pMsg->hwnd == m_SecTabButton.m_hWnd)
   {
      pToolTip->RelayEvent(pMsg);
      return TRUE;
   }

   if (!m_VertAttrMode) return TRUE; // formato tabellare

   StartItem = m_EntDBValuesList.GetTopIndex();
   EndItem   = StartItem + m_EntDBValuesList.GetCountPerPage();

   for (iItem = StartItem; iItem < EndItem; iItem++)
      if (m_EntDBValuesList.GetSubItemRect(iItem, 0, LVIR_BOUNDS, Rect1) == TRUE)
      {
         ScreenRect = Rect1;
         m_EntDBValuesList.ClientToScreen(ScreenRect);
         if (ScreenRect.PtInRect(pMsg->pt))
         {
            m_EntDBValuesList.GetSubItemRect(iItem, 1, LVIR_BOUNDS, Rect2);
            break;
         }
         else
         if (m_EntDBValuesList.GetSubItemRect(iItem, 1, LVIR_BOUNDS, Rect2) == TRUE)
         {
            ScreenRect = Rect2;
            m_EntDBValuesList.ClientToScreen(ScreenRect);
            if (ScreenRect.PtInRect(pMsg->pt)) break;
         }
      }

   // Se non era su una riga del controllo
   if (iItem == EndItem) return TRUE;

   Value        = m_EntDBValuesList.GetItemText(iItem, 1);
   Width2Column = m_EntDBValuesList.GetStringWidth((LPCTSTR) Value);
   if (Width2Column + 15 >= Rect2.Width()) // Offset di 15
   {
      ItemText = m_EntDBValuesList.GetItemText(iItem, 0);
      ItemText += _T(" = ");
      ItemText += Value;
      if (ItemText.GetLength() > 1000) // MAX_TIP_TEXT_LENGTH
         ItemText = ItemText.Left(1000);
   }
   else
      ItemText = _T("");

   pToolTip->GetText(PrevItemText, &m_EntDBValuesList);
   if (PrevItemText.Compare(ItemText) != 0)
      pToolTip->UpdateTipText(ItemText, &m_EntDBValuesList);
   pToolTip->RelayEvent(pMsg);

   return TRUE;
}

// Le coordinate espresse da pt sono "Screen"
BOOL CEntDBValuesListDlg::InitTooltip(void)
{
   if (!m_EntDBValuesList.m_hWnd) return FALSE;

   if (pToolTip) delete pToolTip;
   pToolTip = new CToolTipCtrl;
   if (!pToolTip->Create(this, TTS_ALWAYSTIP)) return FALSE;
	pToolTip->AddTool(&m_EntDBValuesList, _T(""));
	pToolTip->AddTool(&m_GraphSel, _T("Seleziona oggetti"));
	pToolTip->AddTool(&m_SecTabButton, _T("Tabelle secondarie"));
	pToolTip->AddTool(&m_AddSecButton, _T("Nuova scheda secondaria"));
	pToolTip->AddTool(&m_DelSecButton, _T("Cancella scheda secondaria"));

   if (m_VertAttrMode == TRUE) // se era singola scheda
      pToolTip->AddTool(&m_VertAttrModeButton, _T("Formato a tabella"));
   else // se era formato tabellare
      pToolTip->AddTool(&m_VertAttrModeButton, _T("Formato a colonna"));

   pToolTip->SetDelayTime(TTDT_INITIAL, 0);
   pToolTip->Activate(TRUE);
   
   return TRUE;
}
void CEntDBValuesListDlg::OnBnClickedVerticalMode()
{
   // Aggiorno eventuale modifica valore
   if (TempItemValueChanged)
   {
      if (!OnChangeValue(TempItemValue)) return;
      TempItemValueChanged = FALSE;
   }

   m_VertAttrMode = !m_VertAttrMode;
   DisplayRecords(); // visualizza i dati letti
   ZoomHighlightEnt();
   SetVisSecTabButton(); // visualizza o meno il bottone delle secondarie
}

/*********************************************************/
// Funzione che fa la comparazione tra 2 valori
// per l'ordinamento delle righe della listctrl con i dati
// delle entità visualizzati in orizzontale 
/*********************************************************/
static int CALLBACK HorizEntDataCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   CEntDBValuesListDlg *pEntDBValuesList = (CEntDBValuesListDlg *) lParamSort;
   presbuf             pRow1 = (presbuf) lParam1, pRow2 = (presbuf) lParam2;
   presbuf             pVal1, pVal2;
   C_ATTRIB            *pAttrib;

   if (pEntDBValuesList->isForClass) // gestione entità
   {
      if (!(pAttrib = (C_ATTRIB *) pEntDBValuesList->pActualCls->ptr_attrib_list()->getptr_at(pEntDBValuesList->Prev_iHeader + 1)))
         return 0;
   }
   else
      // gestione schede secondarie
      if (!(pAttrib = (C_ATTRIB *) pEntDBValuesList->pActualSec->ptr_attrib_list()->getptr_at(pEntDBValuesList->Prev_iHeader + 1)))
         return 0;

   pVal1 = gsc_CdrAssoc(pAttrib->get_name(), pRow1, FALSE);
   pVal2 = gsc_CdrAssoc(pAttrib->get_name(), pRow2, FALSE);

   if (!pEntDBValuesList->pChacheAttribValues)
   {
      if (gsc_DBIsChar(pAttrib->ADOType) == GS_GOOD) // Stringa
      {
         C_STRING strItem1, strItem2;

         strItem1.paste(gsc_rb2str(pVal1));
         strItem2.paste(gsc_rb2str(pVal2));
         if (pEntDBValuesList->AscendingOrder) return strItem1.comp(strItem2, FALSE); // ordine crescente
         else return strItem2.comp(strItem1, FALSE); // ordine decrescente
      }
      else
      if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD) // Numero
      {
         double numItem1, numItem2;

         gsc_rb2Dbl(pVal1, &numItem1);
         gsc_rb2Dbl(pVal2, &numItem2);
         if (pEntDBValuesList->AscendingOrder)
            return (numItem1 > numItem2) ? 1 : ((numItem1 == numItem2) ? 0 : -1); // ordine crescente
         else 
            return (numItem1 < numItem2) ? 1 : ((numItem1 == numItem2) ? 0 : -1); // ordine decrescente
      }
      else
      if (gsc_DBIsDate(pAttrib->ADOType) == GS_GOOD ||
          gsc_DBIsTimestamp(pAttrib->ADOType) == GS_GOOD) // Data o timestamp
      {
         C_STRING strItem1, strItem2, Date1, Date2;

         strItem1.paste(gsc_rb2str(pVal1));
         strItem2.paste(gsc_rb2str(pVal2));
         // Conversione in formato di SQL = "yyyy-mm-dd hh:mm:ss"
         if (gsc_getSQLDateTime(strItem1.get_name(), Date1) == GS_BAD) Date1 = _T("");
         if (gsc_getSQLDateTime(strItem2.get_name(), Date2) == GS_BAD) Date2 = _T("");
         if (pEntDBValuesList->AscendingOrder) return Date1.comp(Date2, FALSE); // ordine crescente
         else return Date2.comp(Date1, FALSE); // ordine decrescente
      }
      else
      if (gsc_DBIsBoolean(pAttrib->ADOType) == GS_GOOD) // Booleano
      {
         C_STRING strItem1, strItem2;
         int      numItem1, numItem2;

         if (gsc_conv_Bool(pVal1, strItem1) == GS_BAD ||
             gsc_conv_Bool(strItem1.get_name(), &numItem1) == GS_BAD) return 0;
         if (gsc_conv_Bool(pVal2, strItem2) == GS_BAD ||
             gsc_conv_Bool(strItem2.get_name(), &numItem2) == GS_BAD) return 0;

         if (pEntDBValuesList->AscendingOrder)
            return (numItem1 > numItem2) ? 1 : ((numItem1 == numItem2) ? 0 : -1); // ordine crescente
         else 
            return (numItem1 < numItem2) ? 1 : ((numItem1 == numItem2) ? 0 : -1); // ordine decrescente
      }
      else
         return 0;
   }
   else // l'attributo corrente è con una lista di valori a 2 colone quindi utilizzo la descrizione
   {
      C_RB_LIST ColValues1, ColValues2;
      int       prj, cls, sub, sec = 0;
      C_STRING strItem1, strItem2;

#if defined(GSDEBUG) // se versione per debugging
   struct _timeb t1, t2, t3;
   _ftime(&t1);
#endif

      if (pEntDBValuesList->isForClass) // gestione entità
      {
         prj = pEntDBValuesList->pActualCls->get_PrjId();
         cls = pEntDBValuesList->pActualCls->ptr_id()->code;
         sub = pEntDBValuesList->pActualCls->ptr_id()->sub_code;
      }
      else
      {
         // gestione schede secondarie
         prj = pEntDBValuesList->pActualSec->ptr_class()->get_PrjId();
         cls = pEntDBValuesList->pActualSec->ptr_class()->ptr_id()->code;
         sub = pEntDBValuesList->pActualSec->ptr_class()->ptr_id()->sub_code;
         sec = pEntDBValuesList->pActualSec->code;
      }

#if defined(GSDEBUG) // se versione per debugging
   _ftime(&t2);
#endif

      ColValues1 << gsc_listcopy(pRow1);
      ColValues2 << gsc_listcopy(pRow2);

#if defined(GSDEBUG) // se versione per debugging
   _ftime(&t3);
   tempo1 += (t3.time + (double)(t3.millitm)/1000) - (t2.time + (double)(t2.millitm)/1000);
   _ftime(&t2);
#endif

      C_2STR *pDescr1 = pEntDBValuesList->pChacheAttribValues->get_Descr(pRow1, prj, cls, sub, sec);
      if (pDescr1)
         strItem1 = pDescr1->get_name2();
      else
         strItem1.paste(gsc_rb2str(pVal1));

      C_2STR *pDescr2 = pEntDBValuesList->pChacheAttribValues->get_Descr(pRow2, prj, cls, sub, sec);
      if (pDescr2)
         strItem2 = pDescr2->get_name2();
      else
         strItem2.paste(gsc_rb2str(pVal2));

#if defined(GSDEBUG) // se versione per debugging
   _ftime(&t3);
   tempo2 += (t3.time + (double)(t3.millitm)/1000) - (t2.time + (double)(t2.millitm)/1000);
   _ftime(&t2);
#endif

      if (pEntDBValuesList->AscendingOrder) return strItem1.comp(strItem2, FALSE); // ordine crescente
      else return strItem2.comp(strItem1, FALSE); // ordine decrescente

   }
}

void CEntDBValuesListDlg::OnLvnColumnclickEntdbvaluesList(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	HD_ITEM      curItem;
   C_ATTRIB     *pAttrib;
	CHeaderCtrl  *pHdrCtrl = m_EntDBValuesList.GetHeaderCtrl();
   int          prj, cls, sub, sec = 0;

   HideEditCtrls();
   // alloco buffer
   curItem.pszText    = (TCHAR *) calloc(sizeof(TCHAR), 100);
   curItem.cchTextMax = 100 - 1;
   curItem.iImage     = 2; // lampadina accesa

   // se non era già stata selezionata questa casella
   if (Prev_iHeader != pNMLV->iSubItem)
   {
      if (Prev_iHeader != -1)
      {
         curItem.mask = HDI_TEXT;
	      pHdrCtrl->GetItem(Prev_iHeader, &curItem);

         if (IsForClass()) // gestione entità
         {
            if (!(pAttrib = (C_ATTRIB *) pActualCls->ptr_attrib_list()->getptr_at(Prev_iHeader + 1)))
               return;
         }
         else // gestione schede secondarie
            if (!(pAttrib = (C_ATTRIB *) pActualSec->ptr_attrib_list()->getptr_at(Prev_iHeader + 1)))
               return;

         if (pAttrib->is_visible() == GS_GOOD)
         {
   	      curItem.mask = HDI_IMAGE | HDI_FORMAT; //  Testo e immagine
         	curItem.fmt  = HDF_STRING | HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
         }
         else
         {
   	      curItem.mask = HDI_FORMAT; //  Testo
         	curItem.fmt  = HDF_LEFT | HDF_STRING;
         }
	      pHdrCtrl->SetItem(Prev_iHeader, &curItem);
      }
      AscendingOrder = TRUE;
      Prev_iHeader = pNMLV->iSubItem;
   }
   else
      AscendingOrder = !AscendingOrder;

   if (IsForClass()) // gestione entità
   {
      prj = pActualCls->get_PrjId();
      cls = pActualCls->ptr_id()->code;
      sub = pActualCls->ptr_id()->sub_code;

      if (!(pAttrib = (C_ATTRIB *) pActualCls->ptr_attrib_list()->getptr_at(pNMLV->iSubItem + 1)))
         return;
   }
   else
   {
      // gestione schede secondarie
      prj = pActualSec->ptr_class()->get_PrjId();
      cls = pActualSec->ptr_class()->ptr_id()->code;
      sub = pActualSec->ptr_class()->ptr_id()->sub_code;
      sec = pActualSec->code;

      if (!(pAttrib = (C_ATTRIB *) pActualSec->ptr_attrib_list()->getptr_at(pNMLV->iSubItem + 1)))
         return;
   }

   // add bmaps to header item
   curItem.mask = HDI_TEXT;
	pHdrCtrl->GetItem(pNMLV->iSubItem, &curItem);

   // Nome attributo con Flag di Visibilità
	curItem.mask = HDI_IMAGE | HDI_FORMAT; //  Testo e immagine
   if (pAttrib->is_visible() == GS_GOOD)
      curItem.iImage = (AscendingOrder) ? 3 : 4; // lampadina + crescente oppure lampadina + decrescente
   else
      curItem.iImage = (AscendingOrder) ? 0 : 1; // crescente oppure decrescente

	curItem.fmt = HDF_STRING | HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
	pHdrCtrl->SetItem(pNMLV->iSubItem, &curItem);

   if (!get_GS_CURRENT_WRK_SESSION()) pChacheAttribValues = NULL;
   else
      if (pChacheAttribValues = get_GS_CURRENT_WRK_SESSION()->get_pCacheClsAttribValuesList()->get_pCacheAttribValues(pAttrib->name, prj, cls, sub, sec))
         if (pChacheAttribValues->isListNotExisting()) pChacheAttribValues = NULL;

   m_EntDBValuesList.SortItems(HorizEntDataCompare, (LPARAM) this);

   free(curItem.pszText);
   
   *pResult = 0;
}

void CEntDBValuesListDlg::HideEditCtrls(void)
{
   m_ComboButton.ShowWindow(SW_HIDE);
   m_Edit.ShowWindow(SW_HIDE);
   m_MaskedEdit.ShowWindow(SW_HIDE);
   m_List.ShowWindow(SW_HIDE);
}

C_ATTRIB *CEntDBValuesListDlg::get_AttribPtr(int _Item, int _SubItem)
{
   if (_Item == -1) _Item = ActualItem;
   if (_SubItem == -1) _SubItem = ActualSubItem;

   if (m_VertAttrMode) // singola scheda
      return (C_ATTRIB *) m_EntDBValuesList.GetItemData(_Item);
   else // formato tabellare
      if (IsForClass()) // gestione entità
         return (C_ATTRIB *) pActualCls->ptr_attrib_list()->getptr_at(_SubItem + 1);
      else // gestione schede secondarie
         return (C_ATTRIB *) pActualSec->ptr_attrib_list()->getptr_at(_SubItem + 1);

   return NULL;
}

presbuf CEntDBValuesListDlg::get_presbufItemData(int _Item, int _SubItem)
{
   if (_Item == -1) _Item = ActualItem;
   if (_SubItem == -1) _SubItem = ActualSubItem;

   if (m_VertAttrMode) // singola scheda
   {
      C_ATTRIB *pAttrib = get_AttribPtr(_Item, _SubItem);
      
      if (!pAttrib) return NULL;
      return EntColValues.CdrAssoc(pAttrib->get_name());
   }
   else // formato tabellare
   {
      presbuf pRb;

      C_ATTRIB *pAttrib = get_AttribPtr(_Item, _SubItem);
      if ((pRb = (presbuf) m_EntDBValuesList.GetItemData(_Item)))
         return gsc_CdrAssoc(pAttrib->get_name(), pRb, FALSE);
   }
      
   return NULL;
}

BOOL CEntDBValuesListDlg::EnsureVisibleItem(int _Item, int _SubItem)
{
   CRect _rect, _sub_rect;
   CSize _size;

   if (_Item == -1) _Item = ActualItem;
   if (_SubItem == -1) _SubItem = ActualSubItem;
   _size.cy = 0;

   m_EntDBValuesList.EnsureVisible(_Item, FALSE);
   m_EntDBValuesList.GetClientRect(&_rect);
   m_EntDBValuesList.GetSubItemRect(_Item, _SubItem, LVIR_BOUNDS, _sub_rect);
   if (_sub_rect.right > _rect.right) // la casella sborda a destra
   {
      int OffSet = _sub_rect.right - _rect.right;

      if (_sub_rect.left - OffSet < _rect.left) // la casella sborda anche sinistra
      {
         _size.cx = _sub_rect.left - _rect.left;
         m_EntDBValuesList.Scroll(_size);
      }
      else
      {
         _size.cx = _sub_rect.right - _rect.right;
         m_EntDBValuesList.Scroll(_size);
      }
   }
   else 
      if (_sub_rect.left < _rect.left) // la casella sborda a sinistra
      {
         _size.cx = _sub_rect.left - _rect.left;
         m_EntDBValuesList.Scroll(_size);
      }

   return TRUE;
}

void CEntDBValuesListDlg::OnHdnTrackEntdbvaluesList(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

   HideEditCtrls();

   *pResult = 0;
}

void CEntDBValuesListDlg::ZoomHighlightEnt(void)
{
   C_INIT *pGS_GLOBALVAR = get_GS_GLOBALVAR();

   if (ZoomHighlightss.length() == 0)
      gsc_get_pick_first(ZoomHighlightss);

   ZoomHighlightss.redraw(4); // Toglie evidenziazione al gruppo precedente
   acedRedraw(NULL, 1); // ridisegna il video per togliere le crocette
   ZoomHighlightss.clear();

   if (!IsForClass()) return; // gestione schede secondarie

   if (m_VertAttrMode) // singola scheda
   {
      C_BLONG  *pKey = (C_BLONG *) KeyList.go_top();
      C_SELSET entSS;

      if (!m_Zoom && !m_Highlight) return;

      if (IsDBGridMode())
      {
         ads_point pt;
         bool      First = true;

         if (m_Highlight)
            // Cancella il disegno dell'ultimo rettangolo
            gsc_insert_rectangle(ZoomHighlight_pt1, ZoomHighlight_pt2, NULL, 0, PREVIEW);

         // Considero tutte le entità
         while (pKey)
         {
            pActualCls->ptr_grid()->key2pt(pKey->get_key(), pt, false);
            
            if (First)
            {
               First = false;
               ads_point_set(pt, ZoomHighlight_pt1);
               ads_point_set(pt, ZoomHighlight_pt2);
            }
            else
            {
               if (pt[X] < ZoomHighlight_pt1[X]) ZoomHighlight_pt1[X] = pt[X];
               if (pt[Y] < ZoomHighlight_pt1[Y]) ZoomHighlight_pt1[Y] = pt[Y];
               if (pt[X] > ZoomHighlight_pt2[X]) ZoomHighlight_pt2[X] = pt[X];
               if (pt[Y] > ZoomHighlight_pt2[Y]) ZoomHighlight_pt2[Y] = pt[Y];
            }

            pKey = (C_BLONG *) KeyList.go_next();
         }

         ZoomHighlight_pt2[X] += pActualCls->ptr_grid()->dx;
         ZoomHighlight_pt2[Y] += pActualCls->ptr_grid()->dy;

         if (m_Zoom) gsc_zoom(ZoomHighlight_pt1, ZoomHighlight_pt2,
                              pGS_GLOBALVAR->get_AutoZoomMinXDim(),
                              pGS_GLOBALVAR->get_AutoZoomMinYDim());

         if (m_Highlight)
         {
            C_COLOR color;

            color.setForeground();
            gsc_insert_rectangle(ZoomHighlight_pt1, ZoomHighlight_pt2, NULL, &color, PREVIEW);
         }

         refreshDisplay();
      }
      else
      {
         // Considero tutte le entità
         while (pKey)
         {
            if (pActualCls->get_SelSet(pKey->get_key(), entSS) == GS_GOOD)
               ZoomHighlightss.add_selset(entSS);

            pKey = (C_BLONG *) KeyList.go_next();
         }
         if (m_Zoom) ZoomHighlightss.zoom(pGS_GLOBALVAR->get_AutoZoomMinXDim(),
                                          pGS_GLOBALVAR->get_AutoZoomMinYDim());
         if (m_Highlight)
         {
            ZoomHighlightss.redraw(3);   // Evidenzia gli oggetti grafici
            ZoomHighlightss.DrawCross(); // disegna crocette
            refreshDisplay();
         }
      }
   }
   else // formato tabellare
   {
      long    Key;
      presbuf pRb;

      // Considero solo l'entità corrente
      if (!(pRb = (presbuf) m_EntDBValuesList.GetItemData(ActualItem))) return;
      if (!pActualCls->ptr_info()) return;
      if (!(pRb = gsc_assoc(pActualCls->ptr_info()->key_attrib.get_name(), pRb, FALSE)->rbnext->rbnext))
         return;
      if (gsc_rb2Lng(pRb, &Key) != GS_GOOD) return;

      if (IsDBGridMode())
      {
         pActualCls->ptr_grid()->key2pt(Key, ZoomHighlight_pt1, false);
         ZoomHighlight_pt2[X] = ZoomHighlight_pt1[X] + pActualCls->ptr_grid()->dx;
         ZoomHighlight_pt2[Y] = ZoomHighlight_pt1[Y] + pActualCls->ptr_grid()->dy;

         if (m_Zoom) gsc_zoom(ZoomHighlight_pt1, ZoomHighlight_pt2,
                              pGS_GLOBALVAR->get_AutoZoomMinXDim(),
                              pGS_GLOBALVAR->get_AutoZoomMinYDim());

         if (m_Highlight)
         {
            C_COLOR color;

            color.setForeground();
            gsc_insert_rectangle(ZoomHighlight_pt1, ZoomHighlight_pt2, NULL, &color, PREVIEW);
         }

         refreshDisplay();
      }
      else
      {
         pActualCls->get_SelSet(Key, ZoomHighlightss);
         if (m_Zoom) ZoomHighlightss.zoom(pGS_GLOBALVAR->get_AutoZoomMinXDim(),
                                          pGS_GLOBALVAR->get_AutoZoomMinYDim());
         if (m_Highlight)
         {
            ZoomHighlightss.redraw(3);   // Evidenzia gli oggetti grafici
            ZoomHighlightss.DrawCross(); // disegna crocette
            refreshDisplay();
         }
      }
   }
}

void CEntDBValuesListDlg::OnBnClickedGraphSelection()
{
	//acDocManager->sendStringToExecute(acDocManager->curDocument(), "._PSELECT ");	
   acedGetAcadFrame()->SetFocus();   // explicitly set focus to AutoCAD. 
   acedGetAcadDwgView()->SendMessage(WM_MOUSEMOVE,NULL,NULL);  // send WM_MOUSEMOVE message.
   gsui_PSelect();

   //C_SELSET PickSet;
   //if (gsc_get_pick_first(PickSet) == GS_GOOD)
   //{
   //   gsc_WrkSessionPanel(TRUE);
   //   if (pDockPaneWrkSession) pDockPaneWrkSession->DBQuery(PickSet);
   //}
}


/*********************************************************/
/*.doc CEntDBValuesListDlg::SetWindowPosRemark <internal> */
/*+
  Questa funzione posizione il controllo remark dopo che sono stati 
  posizionati gli altri controlli.
-*/  
/*********************************************************/
void CEntDBValuesListDlg::SetWindowPosRemark(void)
{
   CRect rect;
   int   Top = 0, Height, OffSet = 5, m_OKLeft, LeftPos;

   m_OK.GetWindowRect(&rect);
	ScreenToClient(rect);
   m_OKLeft = rect.left;
   Top      = rect.top;
   Height   = rect.Height();

   if (m_SecTabButton.IsWindowVisible())
      m_SecTabButton.GetWindowRect(&rect);
   else
      m_VertAttrModeButton.GetWindowRect(&rect);

	ScreenToClient(rect);
   LeftPos = rect.right + OffSet;

   m_Remark.GetWindowRect(&rect);
   m_Remark.SetWindowPos(NULL, LeftPos, Top,
                         m_OKLeft - OffSet - LeftPos, Height, SWP_NOZORDER);
}


/*********************************************************/
/*.doc CEntDBValuesListDlg::SetVisSecTabButton   <internal> */
/*+
  Questa funzione setta le proprietà del bottone SecTabButton.
-*/  
/*********************************************************/
void CEntDBValuesListDlg::SetVisSecTabButton(void)
{
   C_SINTH_SEC_TAB_LIST SinthSecList;

   if (!IsForClass()) // Se non si stanno gestendo entità di classi GEOsim
      { m_SecTabButton.ShowWindow(SW_HIDE); return; }

   if (!pActualCls)
      { m_SecTabButton.ShowWindow(SW_HIDE); return; }

   // Verifico se la classe scelta ha delle tab. secondarie
   if (pActualCls->get_pPrj()->getSinthClsSecondaryTabList(pActualCls->ptr_id()->code, pActualCls->ptr_id()->sub_code, SinthSecList) == GS_GOOD &&
       SinthSecList.get_count() > 0)
   {
      if (m_VertAttrMode) // singola scheda
         // solo se è stata selezionata una sola entità
         // viene visualizzato il bottone delle secondarie
         if (KeyList.get_count() == 1)
            m_SecTabButton.ShowWindow(SW_SHOW);
         else
            m_SecTabButton.ShowWindow(SW_HIDE);
      else
         // solo se è stata selezionata una riga della lista
         // viene visualizzato il bottone delle secondarie
         m_SecTabButton.ShowWindow(SW_SHOW);
   }
   else
      m_SecTabButton.ShowWindow(SW_HIDE);

   // sistemo il controllo remark
   SetWindowPosRemark();
}


void CEntDBValuesListDlg::OnBnClickedSecondaryTab()
{
   if (!IsForClass()) return; // gestione schede secondarie

   CSecDBValuesListDlg m_SecDBValuesListDlg;

   m_SecDBValuesListDlg.pActualCls = pActualCls;

   if (m_VertAttrMode) // singola scheda
      PrevEntColValues.copy(m_SecDBValuesListDlg.MotherColValues);
   else // formato tabellare
   {
      presbuf rbValue;
      if (!(rbValue = (presbuf) m_EntDBValuesList.GetItemData(ActualItem))) return;
      m_SecDBValuesListDlg.MotherColValues << gsc_listcopy(rbValue);
   }

   m_SecDBValuesListDlg.DoModal();
}


void CEntDBValuesListDlg::OnBnClickedAddSec()
{
   long      key_pri;
   presbuf   rbValue;
   C_RB_LIST ColValues;

   if (IsForClass()) return; // gestione entità

   // Se stavo editando qualche campo devo confermare la modifica
   OnOkHoriz();

   if (!(rbValue = MotherColValues.CdrAssoc(pActualSec->ptr_info()->key_pri.get_name())))
      return;
   gsc_rb2Lng(rbValue, &key_pri);
   if (pActualSec->get_default_values(ColValues) == GS_BAD) return;
   if (pActualSec->ins_data(key_pri, ColValues) == GS_BAD)
   {
      gsui_alert(_T("Inserimento fallito."));
      gsc_print_error();
      return;
   }
   SelchangeClasslistCombo(m_ClassAndSecListCombo.GetCurSel());
}


void CEntDBValuesListDlg::OnBnClickedDelSec()
{
   long     key_pri, Key;
   presbuf  rbValue;
   C_STRING Msg;

   if (IsForClass()) return; // gestione entità

   // Codice entità
   if (!(rbValue = MotherColValues.CdrAssoc(pActualSec->ptr_info()->key_pri.get_name())))
      return;
   gsc_rb2Lng(rbValue, &key_pri);

   if (m_VertAttrMode) // singola scheda
   {
      if (KeyList.get_count() == 0) return;
      if (KeyList.get_count() > 1)
      {
         gsui_alert(_T("Non è possibile la cancellazione multipla delle schede secondarie."));
         return;
      }
      Key = ((C_BLONG *) KeyList.go_top())->get_key();
   }
   else // formato tabellare
   {
      C_RB_LIST _ColValues;

      // Codice scheda secondaria
      if (!(rbValue = (presbuf) m_EntDBValuesList.GetItemData(ActualItem))) return;
      if (_ColValues << gsc_listcopy(rbValue) == NULL) return;
      if (!(rbValue = _ColValues.CdrAssoc(pActualSec->ptr_info()->key_attrib.get_name())))
         return;
      if (gsc_rb2Lng(rbValue, &Key) != GS_GOOD) return;
   }

   Msg = _T("Cancellare la scheda secondaria n.");
   Msg += Key;
   Msg += _T(" ?");

   if (gsui_confirm(Msg.get_name(), GS_BAD, FALSE, FALSE, m_hWnd) == GS_GOOD)
   {
      int old_reactor_abilit = gsc_disable_reactors(); // disabilito i reattori di GEOsim
      
      if (pActualSec->del_data(key_pri, Key) == GS_BAD)
      {
         gsui_alert(_T("Cancellazione fallita."));
         gsc_print_error();
         return;
      }

      // ripristino il controllo sul reattore come in precedenza
      if (old_reactor_abilit == GS_GOOD) gsc_enable_reactors();

      SelchangeClasslistCombo(m_ClassAndSecListCombo.GetCurSel());
   }
}


/////////////////////////////////////////////////////////////////////////////
// CSecDBValuesListDlg dialog
/////////////////////////////////////////////////////////////////////////////

CSecDBValuesListDlg::CSecDBValuesListDlg(CWnd* pParent /*=NULL*/, C_SECONDARY *in, long id)
	: CDialog(CSecDBValuesListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSecDBValuesListDlg)
	//}}AFX_DATA_INIT
   m_hIcon = AfxGetApp()->LoadIcon(IDI_GEOSIM);
   pInitSec = in;
   InitId = id;
}

void CSecDBValuesListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSecDBValuesListDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSecDBValuesListDlg, CDialog)
	//{{AFX_MSG_MAP(CSecDBValuesListDlg)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSecDBValuesListDlg message handlers
BOOL CSecDBValuesListDlg::OnInitDialog() 
{
   CAcModuleResourceOverride myResources;
   CDialog::OnInitDialog();

	SetIcon(m_hIcon, FALSE);		// Set small icon
	SetIcon(m_hIcon, TRUE);		   // Set big icon

   m_EntDBValuesList.isForClass = FALSE; // per tabelle secondarie
   // con la Create viene chiamata la OnInitDialog
   m_EntDBValuesList.Create(CEntDBValuesListDlg::IDD, this);
   m_EntDBValuesList.RefreshSec(pActualCls, MotherColValues);

   int i = 0;

   if (pInitSec) // Se si deve selezionare una tabella secondaria nota
   {
      while (i < m_EntDBValuesList.m_ClassAndSecListCombo.GetCount())
      {
         if (((C_SECONDARY *) m_EntDBValuesList.m_ClassAndSecListCombo.GetItemDataPtr(i))->get_key() == 
             pInitSec->get_key())
         {
            m_EntDBValuesList.SelchangeClasslistCombo(i++);
            if (InitId != 0 && !m_EntDBValuesList.m_VertAttrMode) // seleziono quella scheda secondaria
            {
               presbuf     pSecRow, pKeyData;
               int         j = 0;
               long        dummyLng;

               while (j < m_EntDBValuesList.m_EntDBValuesList.GetItemCount())
               {
                  if ((pSecRow = (presbuf) m_EntDBValuesList.m_EntDBValuesList.GetItemData(j)) != NULL)
                     if ((pKeyData = gsc_CdrAssoc(m_EntDBValuesList.pActualSec->ptr_info()->key_attrib.get_name(),
                                                  pSecRow, FALSE)) &&
                         gsc_rb2Lng(pKeyData, &dummyLng) == GS_GOOD &&
                         dummyLng == InitId)
                     {
                        NM_LISTVIEW _NM;
                        LRESULT     _Result;

                        _NM.iItem    = j;
                        _NM.iSubItem = 0;
                        m_EntDBValuesList.m_EntDBValuesList.SetItemState(j, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
                        m_EntDBValuesList.OnClickEntdbvaluesList((NMHDR *) &_NM, &_Result);
                        break;
                     }
                  j++;
               }
            }
            break;
         }
         i++;
      }
   }
   else // ciclo tra le tabelle secondarie finchè ne trovo una con dei dati
      while (i < m_EntDBValuesList.m_ClassAndSecListCombo.GetCount())
         if (m_EntDBValuesList.SelchangeClasslistCombo(i++) > 0)
            break;
 
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSecDBValuesListDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
   CRect rect;

	CDialog::OnShowWindow(bShow, nStatus);

   if (!bShow) return;

   GetClientRect(&rect);
	m_EntDBValuesList.SetWindowPos(NULL, 5, 5, rect.Width() - 10, rect.Height() - 10,
	                               SWP_NOZORDER);
   m_EntDBValuesList.ShowWindow(SW_SHOW);
}

void CSecDBValuesListDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	m_EntDBValuesList.SetWindowPos(NULL, 5, 5, cx - 10, cy - 10,
	                               SWP_NOZORDER);
}