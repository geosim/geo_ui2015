//{{AFX_INCLUDES()
#include "GEOTreeCtrl.h"
#include "GEOListCtrl.h"
#include "afxwin.h"
//}}AFX_INCLUDES
#if !defined(AFX_GS_UI_ORGANIZER_H__A0E4C88B_CDD3_4A23_B45C_EF49CACB8B83__INCLUDED_)
#define AFX_GS_UI_ORGANIZER_H__A0E4C88B_CDD3_4A23_B45C_EF49CACB8B83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// gs_ui_organizer.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CDataModelDlg frame

class CDataModelDlg : public CFrameWnd
{
	DECLARE_DYNCREATE(CDataModelDlg)
protected:
	CDataModelDlg();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataModelDlg)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CDataModelDlg();

	// Generated message map functions
	//{{AFX_MSG(CDataModelDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// COneGEOComponentSelDlg dialog

class COneGEOComponentSelDlg : public CDialog
{
// Construction
public:
   CString Title;
   CString Msg;

   bool    GraphSelectionVis;  // Visibilità bottone per selezione di oggetti grafici
   bool    ConnectionCheckVis; // Visibilità per scelta controllo connessioni

   bool           UseCurrentPrjOnly;
   C_FAMILY_LIST  FilterOnCodes;
   C_INT_INT_LIST FilterOnClassCategoryTypeList;
   bool           FilterOnExtractedClass;
   bool           FilterOnUpdateableClass;
   bool           ConnectionCheck;    // Controllo connessioni

   enum Component { gsAny            = 0,
                    gsProject        = 1,
                    gsClass          = 2,
                    gsSubClass       = 4,
                    gsSecondaryTable = 8 };

   int ComponentToSelect; // Controllo su componente da scegliere: 0 = qualsiasi,
                          // 1 = progetto, 2 = classe, 4 = sottoclasse, 8 = secondaria

	int Sec;
	int Sub;
	int Cls;
	int Prj;
	COneGEOComponentSelDlg(CWnd* pParent = NULL);   // standard constructor
   ~COneGEOComponentSelDlg();

// Dialog Data
	//{{AFX_DATA(COneGEOComponentSelDlg)
	enum { IDD = IDD_ONEGEOCOMPONENTSELECT_DLG };
	CButton	    m_GraphSelection; 
   int          HelpStrId;
	CStatic	    m_Msg;
	CGEOTreeCtrl m_GEOTreeCtrl;
	CButton	    m_EnableConnection; 
	//}}AFX_DATA

	CToolTipCtrl *pToolTip;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COneGEOComponentSelDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

private:
   HBITMAP hBmpGraphSelection;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COneGEOComponentSelDlg)
	virtual void OnOK();
	afx_msg void OnHelp();
	virtual BOOL OnInitDialog();
	afx_msg void OnGraphSelection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
   void ChangeSelectionGeotreectrl();
   afx_msg void OnBnClickedEnableConnection();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnNMDblclkGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};


// CMultiGEOClassSelDlg dialog

class CMultiGEOClassSelDlg : public CDialog
{
	DECLARE_DYNAMIC(CMultiGEOClassSelDlg)

public:
	int Prj;
   CString Title;
   CString Msg;

   C_INT_INT_LIST FilterOnClassCategoryType;
   bool           FilterOnExtractedClass;
   bool           FilterOnUpdateableClass;
   C_INT_LIST     FilterOnClassCodeList;
   CString        FilterOnSecondaryTable;
   bool           StatusColumnVisibility;
   bool           MultiSelect;

   bool    GraphSelectionVis;   // Visibilità bottone per selezione di oggetti grafici
   bool    PrjSelectionEnabled; // Possibilità di scegliere un progetto
   bool    ColumnAutoSize;

   C_INT_LIST ClsCodeList;          // output - classi scelte

public:
	CMultiGEOClassSelDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMultiGEOClassSelDlg();

// Dialog Data
	enum { IDD = IDD_MULTIGEOCOMPONENTSELECT_DLG };

   CComboBox    m_ProjectComboCtrl;
	CButton	    m_GraphSelection; 
   int          HelpStrId;
	CStatic	    m_Msg;
   CGEOListCtrl m_GEOClassListCtrl;

	CToolTipCtrl *pToolTip;

private:
   HBITMAP hBmpGraphSelection;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
   void OnInitPrjList();

	DECLARE_MESSAGE_MAP()

   afx_msg void OnBnClickedGraphSelection();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
   afx_msg void OnCbnSelchangeProjectCombo();
   afx_msg void OnNMDblclkGeoclasslistctrl(NMHDR *pNMHDR, LRESULT *pResult);
};


int gsui_DisplayDWGExt(void);
int gsui_DelDataModelComponent(void);
int gsuiinsdata(void);
DllExport int gsc_ui_insdata(int cls = 0, int sub = 0, bool ConnectionCheck = TRUE);
int gsuidbdata(void);
int gsuidbGriddata(void);
int gsuidbGriddataOnKeyList(void);
int gsui_SelClass(void);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_GS_UI_ORGANIZER_H__A0E4C88B_CDD3_4A23_B45C_EF49CACB8B83__INCLUDED_)
#pragma once
