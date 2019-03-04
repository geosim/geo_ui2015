//{{AFX_INCLUDES()
#include "GEOListCtrl.h"
#include "GEOTreeCtrl.h"

#if !defined(AFX_GS_UI_WRKSESSION_H__25ECC9CD_C34E_461A_AADB_C40B02E24451__INCLUDED_)
#define AFX_GS_UI_WRKSESSION_H__25ECC9CD_C34E_461A_AADB_C40B02E24451__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// gs_ui_WrkSession.h : header file
//


// PURPOSE:
// This class implements a dockable pane, MSVC-like. It is based
// on some sample code so the code still has the "sample" look.
//
#include "AdUiPaletteSet.h" // for CAdUiPaletteSet
#include "subclass.h" // for CSubclassWnd


#include "adslib.h"
#include "aced.h"
#include "dbmain.h"
#ifndef AD_ACDBABB_H
   #include "acdbabb.h"
#endif
#include "adeskabb.h"
#include "rxregsvc.h"
#include "acgi.h"
#include "acdocman.h"
#include "acedinpt.h"
#include "dbapserv.h"

#include "gs_list.h"
#include "gs_init.h"
#include "gs_graph.h"

#include "aced.h"
#include "adscodes.h"
#include "acdocman.h"
#include "acedinpt.h"

#include "gs_ui_query.h"
#include "afxwin.h"
#include "afxcmn.h"

/////////////////////////////////////////////////////////////////////////////
// reattore di ACAD

class GS_UI_InputContextReactor : public AcEdInputContextReactor
{
public:

    virtual void beginQuiescentState();
    virtual void endQuiescentState();
};


/////////////////////////////////////////////////////////////////////////////
// drag and drop

class CDropSource : public COleDropSource
{
public:
	virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState);
	virtual SCODE GiveFeedback(DROPEFFECT dropEffect);
};


class CDummyDropTarget : public COleDropTarget
{
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point)
    {   return DROPEFFECT_MOVE;}; // just to display move icon
};

class CMyOverrideDropTarget  : public COleDropTarget
{
public:
    CMyOverrideDropTarget();
    ~CMyOverrideDropTarget();

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);

private:
    int Mode;        // modo estrazione (EXTRACTION, PREVIEW)
    int SpatialCond; // 0 = query corrente, 1 = zoom corrente
    C_INT_LIST CodeClsList;
};


// controllo albero personalizzato per gestire drag and drop
class CGEOClsTreeCtrl : public CGEOTreeCtrl
{
   public:
      CGEOClsTreeCtrl();
	   virtual ~CGEOClsTreeCtrl();

      CPropertyPage *pParent;

   afx_msg BOOL OnNMClickGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg void OnRClick(NMHDR* pNMHDR, LRESULT* pResult);

   void OnSearchByNameMenu(void);
   void OnLoadSelMenu(void);
   void OnSaveSelMenu(void);
   void OnRefreshMenu(void);
   void OnInvertSelMenu(void);

protected:
   	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CWrkSessionClasses dialog

class CWrkSessionClasses : public CPropertyPage
{
// Construction
public:
	CWrkSessionClasses();
    ~CWrkSessionClasses();

   int extract(int Mode, int SpatialCond, C_INT_LIST &CodeClsList);

// Dialog Data
	//{{AFX_DATA(CWrkSessionClasses)
	enum { IDD = IDD_WRKSESSION_CLASSES };
	CStatic	m_SelectedCount;
	CButton	m_Zone;
	CButton	m_Preview;
	CButton	m_SpatialQuery;
   CGEOListCtrl     m_GEOClassListCtrl;
   CGEOClsTreeCtrl  m_GEOClassTreeCtrl;
   
   enum DisplayModeTypeEnum
   {
      List = 0,
      Tree = 1
   };
   DisplayModeTypeEnum m_ClassesDisplayMode;
   CButton m_DisplayModeButton;

	CToolTipCtrl *pToolTip;
   bool AlignmentControl;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWrkSessionClasses)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

   CDummyDropTarget m_DummyDropTarget;
   void StartDrag(void);

   void DisplayMode(void);

	// Generated message map functions
	//{{AFX_MSG(CWrkSessionClasses)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   HBITMAP hBmpTreeDisplayMode;
   HBITMAP hBmpListDisplayMode;


private:
   BOOL InitTooltip(void);
   BOOL DisplayTooltip(MSG* pMsg);

public:
   C_RECT LastExtractedWindow;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
   afx_msg void OnBnClickedRadioQuery();
   afx_msg void OnBnClickedDisplayModeButton();
   afx_msg void OnEnChangeStaticClasslistBorder();
   afx_msg void OnLvnBegindragGeoclasslistctrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnLvnItemchangedGeoclasslistctrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnNMClickGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnTvnBegindragGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnTvnKeydownGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnNMRClickGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);

   void OnChangeSelectionGeotreectrl(void);
   void OnChangeSelectionGeolistctrl(LPNMLISTVIEW pNMLV = NULL);
   void RefreshSelectedCount(void);
};


/////////////////////////////////////////////////////////////////////////////
// CGSWrkSessionTab window

class CGSWrkSessionTab : public CTabCtrl
{
// Construction
public:
	CGSWrkSessionTab();

// Attributes
public:
   CWrkSessionClasses  m_WrkSessionClasses;
   CEntDBValuesListDlg m_EntDBValuesList;

// Operations
public:
   void SetWrkSessionClassesItem();
   void SetEntDBValuesListItem();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGSWrkSessionTab)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL Create(CWnd *pParentWnd);
	virtual ~CGSWrkSessionTab();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGSWrkSessionTab)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDockPaneWindowHook

class CDockPaneWindowHook : public CSubclassWnd
{
	friend class IPM;
public:
	DECLARE_DYNAMIC(CDockPaneWindowHook)
    CDockPaneWindowHook(){}
    ~CDockPaneWindowHook(){}
protected:
    // window proc to hook frame using CSubclassWnd implementation
	virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);
};

/////////////////////////////////////////////////////////////////////////////
// CDockPaneWrkSession dialog

class CDockPaneWrkSession : public CAdUiPaletteSet
{
	DECLARE_DYNAMIC(CDockPaneWrkSession)

// Construction
public:
   CDockPaneWrkSession();
	virtual ~CDockPaneWrkSession();

   CGSWrkSessionTab m_WrkSessionTab;

// Operations
public:
   BOOL Create(CWnd* pParentWnd);
   BOOL IsWindowVisible() const;
   void SetFloatingSize(const CSize& size);

	Adesk::Boolean OnLoadDwg();
	Adesk::Boolean OnUnloadDwg();
	void OnSaveComplete(AcDbDatabase *pDwg, const TCHAR *pActualName);
   void DBQuery(C_SELSET &ss);
   void GridDBQuery(ads_point pt1, ads_point pt2);
   void GridDBQuery(C_CGRID *pCls, C_LONG_BTREE &KeyList);
   bool IsDBGridMode(void);

   void Hide(void);
   void Show(void);

protected:
    //functions for keeping the focus
   void HookMiniFrame();
	void OnBarStyleChange(DWORD oldStyle, DWORD newStyle);
   LRESULT OnChangedDockedState(WPARAM, LPARAM);

   bool CanFrameworkTakeFocus(); // per non perdere il fuoco

	int SaveInfoControlBar(UINT nPreferredOrientation, CRect *pDockSize,
                         CRect *pFloatingSize, int Opacity, BOOL AutoRollup,
                         CWrkSessionClasses::DisplayModeTypeEnum ClassesDisplayMode,
                         C_INT_INT_LIST &Expanded_prj_clsSet_List);
   int LoadInfoControlBar(UINT &nPreferredOrientation, CRect *pDockSize,
                         CRect *pFloatingSize, int *Opacity, BOOL *AutoRollup,
                         CWrkSessionClasses::DisplayModeTypeEnum *ClassesDisplayMode,
                         C_INT_INT_LIST &Expanded_prj_clsSet_List,
                         C_INT_LIST &ExtractedClasses);

	//{{AFX_MSG(CDockPaneWrkSession)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Attributes
protected:
	virtual void SizeChanged(CRect *lpRect, BOOL bFloating, int flags);
	
	CDockPaneWindowHook m_MiniFrameHook;

	BOOL m_bFloating; //TRUE if we are not docked.
   C_INT_INT_LIST m_Expanded_prj_clsSet_List;
   C_INT_LIST     m_ExtractedClasses;
};


extern CDockPaneWrkSession *pDockPaneWrkSession;

int gsui_WrkSessionPanel(void);
int gsc_WrkSessionPanel(bool Visible);

//---------------------------------------------------------------------------
// Funzioni per il reattore contestuale
void cmdAddInputContextReactor();
void cmdRemoveInputContextReactor();


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GS_UI_WRKSESSION_H__25ECC9CD_C34E_461A_AADB_C40B02E24451__INCLUDED_)


/////////////////////////////////////////////////////////////////////////////
