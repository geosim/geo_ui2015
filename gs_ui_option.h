#if !defined(AFX_GS_UI_OPTION_H__93E3EE0D_ECC5_44C9_B133_6F6876503B08__INCLUDED_)
#define AFX_GS_UI_OPTION_H__93E3EE0D_ECC5_44C9_B133_6F6876503B08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// gs_ui_option.h : header file
//

#ifndef _gs_netw_h
   #include "gs_netw.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionNet dialog

class COptionNet : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionNet)

// Construction
public:
	COptionNet();
	~COptionNet();
	void LoadFromGS();
   void SaveToGS();

// Dialog Data
	//{{AFX_DATA(COptionNet)
	enum { IDD = IDD_OPTION_NET };
	CEdit	      m_GenericEdit;
	CComboBox	m_GenericCombo;
   CComboBox   m_HostCombo;
	CListCtrl	m_AliasList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionNet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
   BOOL         AscendingOrder; // usato da funzione di ordinamento esterna
   int          Prev_iHeader;   // usato da funzione di ordinamento esterna

protected:
   BOOL         isChanged;
   int          Prev_iItem;
   int          Curr_iSubItem;
   C_ALIAS_LIST AliasList;
   C_STR_LIST   DriveList;
   C_ALIAS_LIST SrvSideHostList;
	CImageList   m_ImageList;
	CImageList   m_ImageHdrList;
   void         getAvailableDriveList(C_STR_LIST &AvailableDriveList,
                                      const TCHAR *ActualDrive = NULL);
   void         getOpSysList(C_STR_LIST &OpSysList);
   void         getAvailableHostList(C_STR_LIST &AvailableHostNameList, 
                                     const TCHAR *ActualHost = NULL);
	void         InitAliasList(void);

   // Generated message map functions
	//{{AFX_MSG(COptionNet)
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnclickAliasList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickAliasList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusGenericCombo();
	afx_msg void OnKillfocusGenericEdit();
	afx_msg void OnKeydownAliasList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewAlias();
	afx_msg void OnChangeGenericEdit();
	afx_msg void OnSelchangeGenericCombo();
	afx_msg void OnEraseAlias();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnCbnKillfocusAliasCombo();
public:
   afx_msg void OnCbnSelchangeAliasCombo();
public:
   afx_msg void OnCbnEditchangeAliasCombo();
};

/////////////////////////////////////////////////////////////////////////////
// COptionEnv dialog

class COptionEnv : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionEnv)

// Construction
public:
	void LoadFromGS();
   void SaveToGS();
	COptionEnv();
	~COptionEnv();

// Dialog Data
	//{{AFX_DATA(COptionEnv)
	enum { IDD = IDD_OPTION_ENV };
	CButton	m_AddASELink;
	CButton	m_AutoSyncronize;
	CButton	m_DynamicExtraction;
	CButton	m_UpdGraphOnExtractSim;
	CButton	m_UpdGraphOnExtract;
	CEdit	   m_WaitTime;
	CEdit	   m_NumTest;
	CButton	m_LogFile;
	CButton	m_InsYScale;
	CButton	m_InsXScale;
	CButton	m_InsRot;
	CButton	m_InsPos;
	CButton	m_InsHText;
	CButton	m_AlignHighlightedFasOnSave;
	CButton	m_AddEntityToSaveSet;
	CEdit	   m_AutoZoomMinXDim;
	CEdit	   m_AutoZoomMinYDim;
   CButton  m_SelPreviousExtractedClasses;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionEnv)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionEnv)
	afx_msg void OnAddEntityToSaveset();
	afx_msg void OnAlignhighlightedfasonsave();
	afx_msg void OnInsHText();
	afx_msg void OnInsXScale();
	afx_msg void OnInsYScale();
	afx_msg void OnInsPos();
	afx_msg void OnChangeNumTest();
	afx_msg void OnChangeWaitTime();
	afx_msg void OnLogFile();
	afx_msg void OnInsRot();
	afx_msg void OnRadioData1();
	afx_msg void OnRadioData2();
	afx_msg void OnRadioLogic1();
	afx_msg void OnRadioLogic2();
	afx_msg void OnRadioLogic3();
	afx_msg void OnRadioLogic4();
	afx_msg void OnDimAssoc0();
	afx_msg void OnDimAssoc1();
	afx_msg void OnDimAssoc2();
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdGraphOnExtr();
	afx_msg void OnUpdGraphOnExtrSim();
	afx_msg void OnKillfocusNumTest();
	afx_msg void OnKillfocusWaitTime();
	afx_msg void OnDynamicExtraction();
	afx_msg void OnAutoSyncro();
	afx_msg void OnAddAseLink();
	afx_msg void OnChangeAutoZoomMinXDim();
	afx_msg void OnKillfocusAutoZoomMinXDim();   
	afx_msg void OnChangeAutoZoomMinYDim();
	afx_msg void OnKillfocusAutoZoomMinYDim();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   int m_ShortDataFmt;
   int m_LogicFmt;
   int m_DimAssoc;
   BOOL isChanged;
};


/////////////////////////////////////////////////////////////////////////////
// CGSOptions

class CGSOptions : public CPropertySheet
{
	DECLARE_DYNAMIC(CGSOptions)

// Construction
public:
	CGSOptions(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CGSOptions(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGSOptions)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGSOptions();
   void SaveToGS();

	// Generated message map functions
protected:
	CImageList m_ImageList;
	void AddControlPages();
   void InitializeImageList();
	COptionNet m_OptionNet;
	COptionEnv m_OptionEnv;
	//{{AFX_MSG(CGSOptions)
	afx_msg void OnApplyNow();
	afx_msg void OnOk();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

int gsui_options(void);

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GS_UI_OPTION_H__93E3EE0D_ECC5_44C9_B133_6F6876503B08__INCLUDED_)
#include "afxwin.h"
