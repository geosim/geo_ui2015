#if !defined(AFX_GS_UI_ATTRIBVALUESLISTDLG_H__635896A9_242A_4034_B921_10EDC9B1A4E3__INCLUDED_)
#define AFX_GS_UI_ATTRIBVALUESLISTDLG_H__635896A9_242A_4034_B921_10EDC9B1A4E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// gs_ui_attribvalueslistdlg.h : header file
//

#include "gs_list.h"
#include "GEOTreeCtrl.h"
#include "afxcmn.h"
#include "afxwin.h"

class CAttribValuesListDlg;

///////////////////////////////////////////////////////////////////////////////
// CAttribGSValuesListTabDlg dialog
///////////////////////////////////////////////////////////////////////////////
class CAttribGSValuesListTabDlg : public CDialog
{
	DECLARE_DYNAMIC(CAttribGSValuesListTabDlg)

public:
	CAttribGSValuesListTabDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAttribGSValuesListTabDlg();

// Dialog Data
	enum { IDD = IDD_GSVALUESLIST_TAB };

   CAttribValuesListDlg *pParentDlg;
   C_2STR_LIST          ValuesList;

   void RefreshValuesList(void);

private:
   int Prev_iItem;
   int Curr_iSubItem;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
   //virtual void OnPaint();

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedCancel();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedRemove();
   afx_msg void OnBnClickedRemoveAll();
   afx_msg void OnBnClickedSort();
   afx_msg void OnBnClickedLoad();
   afx_msg void OnBnClickedSave();
   afx_msg void OnNMClickValuesList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnEnKillfocusGenericEdit();
   afx_msg void OnPaint();

   CListCtrl m_ValuesListCtrl;
   CEdit     m_GenericEdit;
};


///////////////////////////////////////////////////////////////////////////////
// CAttribDBValuesListTabDlg dialog
///////////////////////////////////////////////////////////////////////////////
class CAttribDBValuesListTabDlg : public CDialog
{
	DECLARE_DYNAMIC(CAttribDBValuesListTabDlg)

public:
	CAttribDBValuesListTabDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAttribDBValuesListTabDlg();

// Dialog Data
	enum { IDD = IDD_DBVALUESLIST_TAB };

   CAttribValuesListDlg *pParentDlg;
   C_2STR_LIST          ValuesList;

   void RefreshValues(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   BOOL OnInitDialog();

   void getDBInfo(C_STRING &ConnStrUDLFile, C_STRING &UdlProperties, C_STRING &SelectStm);
   void setDBInfo(C_STRING &ConnStrUDLFile, C_STRING &UdlProperties, C_STRING &SelectStm,
                  C_2STR_LIST *pList = NULL);

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedCancel();
   afx_msg void OnBnClickedDbconnButton();
   afx_msg void OnBnClickedSqltestButton();

   CStatic   m_UdlNameLbl;
   CListCtrl m_ValuesListCtrl;
   CEdit     m_SqlEditCtrl;
public:
   afx_msg void OnBnClickedLoad();
public:
   afx_msg void OnBnClickedSave();
public:
   afx_msg void OnEnChangeEdit1();
};


/////////////////////////////////////////////////////////////////////////////
// CAttribValuesListDlg dialog
/////////////////////////////////////////////////////////////////////////////
class CAttribValuesListDlg : public CDialog
{
// Construction
public:
	bool AllPrjs;
	int  Prj;
   int  Cls;
   int  Sub;
   int  Sec;
   C_STRING AttribName;

	CAttribValuesListDlg(CWnd* pParent = NULL);   // standard constructor

   ValuesListTypeEnum Type;
   C_2STR_LIST ValuesList;
   int getFile(C_STRING &Path);

// Dialog Data
	//{{AFX_DATA(CAttribValuesListDlg)
	enum { IDD = IDD_DEFTABREF_DLG };
	CComboBox	 m_ComboAttribName;
   CGEOTreeCtrl m_GeoTree;
	CComboBox    m_GEOLispCombo;
   CTabCtrl     m_TabCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttribValuesListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

   CAttribGSValuesListTabDlg m_GSValuesTab;
   CAttribDBValuesListTabDlg m_DBValuesTab;

	void OnChangeSelectionGeotreectrl();

	// Generated message map functions
	//{{AFX_MSG(CAttribValuesListDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAllPrjs();
	afx_msg void OnEditchangeAttribname();
	afx_msg void OnRadioDef();
	afx_msg void OnRadioFdef();
	afx_msg void OnRadioRef();
	afx_msg void OnRadioTab();
	afx_msg void OnSelchangeAttribname();
	virtual void OnOK();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
   int Prev_iItem;
   int Curr_iSubItem;

	void RefreshMsg(void);
   void RefreshAttribList(void);
   void RefreshValuesList(void);
	void GetAttribList(C_STR_LIST &AttribNameList);
public:
   afx_msg void OnBnClickedOk();
   afx_msg void OnCbnDropdownGeolispCombo();
   afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
	void RefreshListTypeRadioButton(void);
};

int gsui_AttribValuesList(void);

int gsui_getValuesListFilePath(C_STRING &PrjPath, int prj, int cls, int sub, int sec, C_STRING &AttribName,
                               C_STRING &FilePath, int ExactMode = GS_BAD);



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GS_UI_ATTRIBVALUESLISTDLG_H__635896A9_242A_4034_B921_10EDC9B1A4E3__INCLUDED_)
