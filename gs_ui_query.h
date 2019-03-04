#ifndef   _GS_UI_QUERY_H
#define   _GS_UI_QUERY_H

/******************************************************************************/
/*    GS_UI_QUERY.H                                                           */
/******************************************************************************/

#include "MyEdit.h"
#include "MyList.h"


class MyCListCtrl : public CListCtrl
{
// Construction
public:
	MyCListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyCListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~MyCListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(MyCListCtrl)
	//}}AFX_MSG

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(MyCListCtrl)
	public:
	virtual BOOL OnWndMsg( UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	//}}AFX_VIRTUAL
};

/////////////////////////////////////////////////////////////////////////////
// CEntDBValuesListDlg dialog

class CEntDBValuesListDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CEntDBValuesListDlg)

// Construction
public:
   C_CLASS     *pActualCls;        // se <> NULL la finestra visualizza dati di classe
   C_SECONDARY *pActualSec;        // se <> NULL la finestra visualizza dati di tab. sec.
   BOOL        AscendingOrder;     // usato da funzione di ordinamento
   int         Prev_iHeader;       // usato da funzione di ordinamento
   BOOL        m_Zoom;             // se TRUE fa lo zoom dell'entità selezionata
   BOOL        m_Highlight;        // se TRUE evidenzia l'entità selezionata
   BOOL        isForClass;         // se TRUE la finestra gestisce entità di classi
                                   // altrimenti gestisce tabelle secondarie

   bool m_Frozen;       // Se congelato non viene aggiornato dalla selezione degli oggetti
                        // effettuata nella finestra grafica di AutoCAD

   C_CACHE_ATTRIB_VALUES *pChacheAttribValues; // puntatore alla lista di valori tipo look-up, domain table
                                               // relativi ad un attributo che in GEOsim vengono implementati da lista
                                               // di valori attributi a 2 colonne (codice-descrizione)

   int  OperationMode; // MODIFY oppure INSERT (default=MODIFY)
	long SelchangeClasslistCombo(int nItem, bool SecRefresh = FALSE);
   void Clear(void);
   int  RefreshCls(C_SELSET &_SelSet);
   int  RefreshGridCls(ads_point pt1, ads_point pt2);
   int  RefreshGridCls(C_CGRID *_pCls, C_LONG_BTREE &_KeyList);
   int  RefreshSec(C_CLASS *_pCls, C_RB_LIST &_MotherColValues);
   int  SingleToHtml(const TCHAR *path, bool RunAppl = TRUE);
   int  MultiToHtml(const TCHAR *path, bool RunAppl = TRUE);
   void EntDBValuesListColumnSizing(int cx, int cy);
   bool m_VertAttrMode;
   void HideEditCtrls(void);
   void ZoomHighlightEnt(void);
   bool IsDBGridMode(void);
   bool IsDBGridFromWindow(void);

	CEntDBValuesListDlg();
	~CEntDBValuesListDlg();

// Dialog Data
	//{{AFX_DATA(CEntDBValuesListDlg)
	enum { IDD = IDD_ENT_DBVALUES };
	CMyList       m_List;
	CButton       m_OK;
	CButton	     m_ComboButton;
	CMyEdit       m_Edit;
	CMyMaskedEdit m_MaskedEdit;
	CComboBox     m_ClassAndSecListCombo;
	CStatic	     m_Remark;
	MyCListCtrl	  m_EntDBValuesList;
	CButton	     m_VertAttrModeButton;
   CButton       m_GraphSel;
	CButton	     m_SecTabButton;
	CButton	     m_AddSecButton;
	CButton	     m_DelSecButton;
	//}}AFX_DATA

	CToolTipCtrl *pToolTip;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEntDBValuesListDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CImageList m_ImageList;
	CImageList m_ImageHdrList;

	BOOL       OnChangeValue(CString &Value);
	BOOL       OnChangeVertValue(CString &Value);
	BOOL       OnChangeHorizValue(CString &Value);
   BOOL       GoPrevUpdItem(int nItem, int nSubItem);
   BOOL       GoNextUpdItem(int nItem, int nSubItem);
   void       InitList(void);
   BOOL       IsForClass(void);
   int        FillClsRecords(void);
   int        FillSecRecords(void);
   int        DisplayRecords(int _row = -1);
   int        DisplayHorizRecord(int m_cls, int m_sub, int m_sec, C_ATTRIB_LIST *pAttribList,
                                 int row, presbuf pEntColValues, bool RefreshExistingRow = false);
   int        DisplayVertRecord(int m_cls, int m_sub, int m_sec, C_ATTRIB_LIST *pAttribList,
                                presbuf pEntColValues);
   int        EditValue(void);
   void       SetPosEntDBValuesListEditCtrls(void);
   void       EntDBValuesListVertColumnSizing(int cx, int cy);
   void       SetPosEntDBValuesListVertEditCtrls(void);
   void       EntDBValuesListHorizColumnSizing(int cx, int cy);
   void       SetPosEntDBValuesListHorizEditCtrls(void);
   C_ATTRIB*  get_AttribPtr(int _Item = -1, int _SubItem = -1);
   presbuf    get_presbufItemData(int _Item = -1, int _SubItem = -1);
   BOOL       EnsureVisibleItem(int _Item = -1, int _SubItem = -1);
   void       OnOkVert(void);
   void       OnOkHoriz(void);
   void       SetWindowPosRemark(void);
   void       SetVisSecTabButton(void);

	// Generated message map functions
	//{{AFX_MSG(CEntDBValuesListDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeClasslistCombo();
	afx_msg void OnChangeEdit();
	afx_msg void OnChangeMaskedbox();
   afx_msg void OnEnKillfocusMaskedbox();
	afx_msg void OnOk();
	afx_msg void OnKeydownEntdbvaluesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnComboButton();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRclickEntdbvaluesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDBList();
	afx_msg void OnSuggest();
	afx_msg void OnMathOp();
	afx_msg void OnEditUndo();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCopySheet();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditRemove();
	afx_msg void OnEditSelectAll();
	afx_msg void OnPrintSheet();
	afx_msg void OnPrintSheets();
	afx_msg void OnZoom();
	afx_msg void OnHighlight();
   afx_msg void OnFrozenPanel();
	afx_msg void OnDblclkEntdbvaluesList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   C_RB_LIST ColValues;          // Lettura di tutti i record
   C_RB_LIST EntColValues;       // Record corrente soggetto alle modifiche
   C_RB_LIST PrevEntColValues;   // Record corrente prima delle modifiche
   int       ActualItem;
   int       ActualSubItem;
   CString   TempItemValue;
   BOOL      TempItemValueChanged;

   HBITMAP hBmpButtonSupport;
   HBITMAP hBmpSuggest;
   HBITMAP hBmpDBList;
   HBITMAP hBmpMathOp;
   HBITMAP hBmpPrinter;
   HBITMAP hBmpVertMode;
   HBITMAP hBmpHorizMode;
   HBITMAP hBmpSelObjs;
   HBITMAP hBmpSecTab;
   HBITMAP hBmpAddSec;
   HBITMAP hBmpDelSec;

   BOOL ComboButtonVis;
   BOOL EditVis;
   BOOL MaskedEditVis;

   C_SELSET      SelSet;          // usato per gli attributi delle classi

   ads_point     pt1;             // zona selezionata nel caso si vogliano
   ads_point     pt2;             // gestire le entità griglia (che non hanno grafica)
                                  // se pt1 = pt2 = 0,0,0 non si stanno interrogando griglie
                                  // se pt1 = al minimo reale consentito (std::numeric_limits<double>::min)();
                                  // e pt1 = pt2 si sta interrogando una grigla i cui i codici sono in KeyList
   
   C_RB_LIST     MotherColValues; // usato per gli attributi delle secondarie
   
   C_LONG_BTREE  KeyList;

   C_SELSET      ZoomHighlightss; // gruppo di oggetti evidenziati

   ads_point     ZoomHighlight_pt1; // zona evidenziata nel caso si vogliano
   ads_point     ZoomHighlight_pt2; // gestire le entità griglia (che non hanno grafica)

private:
   BOOL InitTooltip(void);
   BOOL DisplayTooltip(MSG* pMsg);
public:
   afx_msg void OnBnClickedVerticalMode();
   afx_msg void OnLvnColumnclickEntdbvaluesList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnHdnTrackEntdbvaluesList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedGraphSelection();
   afx_msg void OnBnClickedSecondaryTab();
   afx_msg void OnBnClickedAddSec();
   afx_msg void OnBnClickedDelSec();

	afx_msg void OnClickEntdbvaluesList(NMHDR* pNMHDR, LRESULT* pResult);
};


class CSecDBValuesListDlg : public CDialog
{
// Construction
public:
	CSecDBValuesListDlg(CWnd* pParent = NULL,
                       C_SECONDARY *in = NULL, long id = 0);

// Dialog Data
	//{{AFX_DATA(CSecDBValuesListDlg)
	enum { IDD = IDD_SEC_DBVALUES };
   CEntDBValuesListDlg m_EntDBValuesList;
	//}}AFX_DATA
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSecDBValuesListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSecDBValuesListDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   HICON m_hIcon; 

public:
   C_CLASS   *pActualCls;
   C_RB_LIST MotherColValues;    // usato per gli attributi delle secondarie
   C_SECONDARY *pInitSec;
   long        InitId;
};

#endif
