//{{AFX_INCLUDES()
#include "GEOListCtrl.h"
#include "afxwin.h"
#include "afxcmn.h"
//}}AFX_INCLUDES

#if !defined(AFX_GS_UI_POSTGRESQL_H__A0E4C88B_CDD3_4A23_B45C_EF49CACB8B83__INCLUDED_)
#define AFX_GS_UI_POSTGRESQL_H__A0E4C88B_CDD3_4A23_B45C_EF49CACB8B83__INCLUDED_
#pragma once


// CPGViewClassSelDlg dialog

class CPGViewClassSelDlg : public CDialog
{
	DECLARE_DYNAMIC(CPGViewClassSelDlg)

public:
	CPGViewClassSelDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPGViewClassSelDlg();

// Dialog Data
	enum { IDD = IDD_PGVIEW_DLG };

   enum OpModeEnum
   {
      create = 0,
      erase = 1
   };

	int        Prj;                           // output - progetto scelto
   C_INT_LIST ClsCodeList;                   // output - classi scelte
   bool       Geometry_columns_table_clean;  // output - opzione di pulizia tabella geometry_columns
   OpModeEnum OpMode;                        // output - tipo di operazione (creazione o cancellazione viste)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
   void OnInitPrjList();
   void OnInitClsList();

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnCbnSelchangeProjectCombo();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnBnClickedGeometryColumnsCheck();

   CComboBox m_ProjectComboCtrl;
   CGEOListCtrl m_GEOClassListCtrl;
   CButton m_Geometry_columns_check;
   CProgressCtrl mProgressCtrl;
   afx_msg void OnBnClickedDelView();
};


// CQGISPropertiesDlg dialog

class CQGISPropertiesDlg : public CDialog
{
	DECLARE_DYNAMIC(CQGISPropertiesDlg)

public:
	CQGISPropertiesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQGISPropertiesDlg();

// Dialog Data
	enum { IDD = IDD_QGIS_PROPERTIES };

   void SaveToFile(C_PROFILE_SECTION_BTREE &ProfileSections);
   void LoadFromFile(C_PROFILE_SECTION_BTREE &ProfileSections);
   void Refresh(void);
   bool CheckValues(void);

   C_STRING QGISPathFile, Host, Port, Database, User, Password;
   int      SRID;
   C_2LONG  LblScaleFactors;
   bool     UseLblScaleFactors;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedBrowseButton();
   afx_msg void OnBnClickedScaleFactorCheck();
   CEdit m_QGISPrjFile_Edit;
   CEdit m_Host_Edit;
   CEdit m_Port_Edit;
   CEdit m_DB_Edit;
   CEdit m_UsrName_Edit;
   CEdit m_Pwd_Edit;
   CEdit m_SRID_Edit;
   CButton m_LblScaleFactor_Check;
   CEdit m_LblMinScaleFactor_Edit;
   CEdit m_LblMaxScaleFactor_Edit;
};


// CQGIS_ClassDlg dialog

class CQGIS_ClassDlg : public CDialog
{
	DECLARE_DYNAMIC(CQGIS_ClassDlg)

public:
	CQGIS_ClassDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQGIS_ClassDlg();

// Dialog Data
	enum { IDD = IDD_QGIS_CLASS };

	int Prj;                               // output - progetto scelto
   C_INT_LIST ClsCodeList;                // output - classi scelte
   C_2LONG_INT_LIST ClsScaleFactorsList;  // output - fattori di scala per classe
   void OnInitPrjList();
   void OnInitClsList();
   void SaveToFile(C_PROFILE_SECTION_BTREE &ProfileSections);
   void LoadFromFile(C_PROFILE_SECTION_BTREE &ProfileSections);
   bool CheckValues(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()


public:
   CComboBox m_ProjectComboCtrl;
   CEdit m_GeomMinScaleFactor_Edit;
   CEdit m_GeomMaxScaleFactor_Edit;
   afx_msg void OnBnClickedAddClassButton();
   afx_msg void OnBnClickedDelClassButton();
   CGEOListCtrl m_GEOSrcClassListCtrl;
   CGEOListCtrl m_GEODstClassListCtrl;
   afx_msg void OnEnKillfocusMinFactorEdit();
   afx_msg void OnEnKillfocusMaxFactorEdit();
   afx_msg void OnCbnSelchangeProjectCombo();
   CProgressCtrl mProgressCtrl;
   afx_msg void OnLvnItemchangedGeoclasslistctrl2(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnLvnItemchangedGeoclasslistToExport(NMHDR *pNMHDR, LRESULT *pResult);

};


// CQGIS_ClassGroupDlg dialog

class CQGIS_ClassGroupDlg : public CDialog
{
	DECLARE_DYNAMIC(CQGIS_ClassGroupDlg)

public:
	CQGIS_ClassGroupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQGIS_ClassGroupDlg();

// Dialog Data
	enum { IDD = IDD_QGIS_CLASS_GRP };

   void SaveToFile(C_PROFILE_SECTION_BTREE &ProfileSections);
   void LoadFromFile(C_PROFILE_SECTION_BTREE &ProfileSections);
   void Refresh(void);
   bool CheckValues(void);

   C_2STR_LIST GroupList;

private:
   int Prev_iItem;
   int Curr_iSubItem;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedRemove();
   afx_msg void OnBnClickedRemoveAll();
   CListCtrl m_GroupListCtrl;
   CEdit m_GenericEdit;
   afx_msg void OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnEnKillfocusGenericEdit();
   afx_msg void OnLvnKeydownList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnPaint();
};


// CQGIS_Dlg dialog

class CQGIS_Dlg : public CDialog
{
	DECLARE_DYNAMIC(CQGIS_Dlg)

public:
	CQGIS_Dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQGIS_Dlg();

// Dialog Data
	enum { IDD = IDD_QGIS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
   CTabCtrl m_TabCtrl;

   CQGISPropertiesDlg  m_PropertiesDlg;
   CQGIS_ClassDlg      m_ClassDlg;
   CQGIS_ClassGroupDlg m_ClassGroupDlg;

   afx_msg void OnBnClickedHelp();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedSave();
   afx_msg void OnBnClickedLoad();

   void SaveToFile(C_STRING &Path);
   void LoadFromFile(C_STRING &Path);

   afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
};



// CHistoryClassSelDlg dialog

class CHistoryClassSelDlg : public CDialog
{
	DECLARE_DYNAMIC(CHistoryClassSelDlg)

public:
	CHistoryClassSelDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHistoryClassSelDlg();

// Dialog Data
	enum { IDD = IDD_HISTORY_DLG };

   enum OpModeEnum
   {
      enable_and_createcls = 0,
      enable = 1,
      disable = 2
   };

	int Prj;                               // output - progetto scelto
   C_INT_LIST ClsCodeList;                // output - classi scelte
   OpModeEnum OpMode;                     // output - tipo di operazione (creazione o disabilita storicizzazione)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
   void OnInitPrjList();
   void OnInitClsList();
   bool checkSelections(void);

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnCbnSelchangeProjectCombo();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   CComboBox m_ProjectComboCtrl;
   CGEOListCtrl m_GEOClassListCtrl;
   CProgressCtrl mProgressCtrl;
   afx_msg void OnBnClickedDelHistory();
   afx_msg void OnBnClickedEnableHistory();
};


int gsui_pgview(void);
int gsui_qgis(void);
int gsui_CreateHistorySystem(void);


#endif // !defined(AFX_GS_UI_POSTGRESQL_H__A0E4C88B_CDD3_4A23_B45C_EF49CACB8B83__INCLUDED_)
