#include "afxwin.h"
#include "afxcmn.h"
#if !defined(AFX_CLASS_LOGIN_H__46EE741E_ADF5_476B_BF40_C99E072FB7E7__INCLUDED_)
#define AFX_CLASS_LOGIN_H__46EE741E_ADF5_476B_BF40_C99E072FB7E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Class_login.h : header file
//

#include "GEOTreeCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CClass_login dialog

class CClass_login : public CDialog
{
// Construction
public:
	CString m_msg_for_password_only; // se <> "" messaggio di richiesta della password (la memorizza in _password)
   CString _password;
	CClass_login(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClass_login)
	enum { IDD = IDD_LOGIN };
	CStatic	    m_image;
	CStatic	    m_label_msg;
	CStatic	    m_label_login;
	CEdit	       m_password;
	CEdit	       m_login;
	CAnimateCtrl m_animate;
   CButton      m_check_session;
   UINT DefID;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClass_login)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CClass_login)
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	virtual void OnOK();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// finestra di dialogo C_CreateUsrDlg

class C_CreateUsrDlg : public CDialog
{
	DECLARE_DYNAMIC(C_CreateUsrDlg)

public:
	C_CreateUsrDlg(CWnd* pParent = NULL);   // costruttore standard
	virtual ~C_CreateUsrDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_CREATE_USR };

protected:
   C_INT_INT_STR_LIST UsrList;
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedSuperuserRadio();
   afx_msg void OnBnClickeduserRadio();
   afx_msg void OnBnClickedPermissionFromUsrCheck();
   afx_msg void OnBnClickedUsrListButton();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   CEdit m_UsrName;
   CEdit m_UsrPwd;
   CEdit m_ConfirmUsrPwd;
   CButton m_CopyFromUsr;
   int m_UsrLevel;
   CComboBox m_UsrList;

	virtual BOOL OnInitDialog();
   CStatic m_image;
};

// finestra di dialogo C_ModiPwdUsrDlg

class C_ModiPwdUsrDlg : public CDialog
{
	DECLARE_DYNAMIC(C_ModiPwdUsrDlg)

public:
	C_ModiPwdUsrDlg(CWnd* pParent = NULL);   // costruttore standard
	virtual ~C_ModiPwdUsrDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_MODI_PWD_CURR_USR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
   CEdit m_UsrPwd;
   CEdit m_ConfirmUsrPwd;
   CEdit m_OldUsrPwd;
   CStatic m_UsrNameStatic;
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   CStatic m_image;

	virtual BOOL OnInitDialog();
};


// finestra di dialogo C_ModUsrDlg

class C_ModUsrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(C_ModUsrDlg)

public:
	C_ModUsrDlg(CWnd* pParent = NULL);   // costruttore standard
	virtual ~C_ModUsrDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_MOD_USR };

protected:
   C_INT_INT_STR_LIST UsrList;
   int                m_CurrUsrId;
   int                m_PrevUsrId;
   bool               Update;
   C_INT_LIST         Commands;
   C_INT_INT_LIST     ProjectsPermissionList;
   C_INT_VOIDPTR_LIST ClassesPermissionList;
   C_INT_VOIDPTR_LIST SecTabsPermissionList;
   C_STR_LIST         InheritanceUserNames;

	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

   int LoadUsrPermission(int UserId);
   int SaveUsrPermission(int UserId);

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_UsrName;
   CButton m_Command_Button;
   CButton m_Data_Button;
   CButton m_CopyFromUsr;
   CComboBox m_UsrList_CopyFrom;
   CStatic m_UsrCodeLbl;
   CButton m_OK;
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   int set_Usr(C_STRING &UsrName);
   int set_Usr(int UsrId);

	virtual BOOL OnInitDialog();
   afx_msg void OnCbnSelchangeUsrlist();
   afx_msg void OnBnClickedPermissionFromUsrCheck();
   afx_msg void OnBnClickedCmdList();
   afx_msg void OnBnClickedDataList();
};


// finestra di dialogo C_ModDataUsrDlg

class C_ModDataUsrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(C_ModDataUsrDlg)

public:
	C_ModDataUsrDlg(int _UsrId, CWnd* pParent = NULL);   // costruttore standard
	virtual ~C_ModDataUsrDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_MOD_DATA_USR };

protected:
   int                UsrId;
   C_INT_INT_LIST     ProjectsPermissionList;
   C_INT_VOIDPTR_LIST ClassesPermissionList;
   C_INT_VOIDPTR_LIST SecTabsPermissionList;
   C_STR_LIST         InheritanceUserNames;

   virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
   void ClassDowngradeOnPrjPermission(GSDataPermissionTypeEnum PrjPermission, CGEOTree_ItemList *pItemList = NULL);
   void SecDowngradeOnPermission(GSDataPermissionTypeEnum PrjPermission, CGEOTree_ItemList *pItemList = NULL, int _Sub = 0);
   int SetClassPermission(int Prj, int Cls, GSDataPermissionTypeEnum Permission);
   int SetSecPermission(int Prj, int Cls, int Sub, int Sec, GSDataPermissionTypeEnum Permission);
   void EnableRadioCtrl(GSDataPermissionTypeEnum Permission);
   GSDataPermissionTypeEnum RadioNumberToPermission(int RadioNumber);
   int PermissionToRadioNumber(GSDataPermissionTypeEnum Permission);

public:
   C_BIRELATION_LIST GroupList;

   CGEOTreeCtrl m_PrjTreeCtrl;
   CGEOTree_Item *pCurrPrjItem;
   afx_msg void OnBnClickedPrjUpd();
   int m_PrjPermission;
   CButton m_PrjInhRadioCtrl;
   CButton m_PrjInvRadioCtrl;
   CButton m_PrjVisRadioCtrl;
   CButton m_PrjModRadioCtrl;

   CTabCtrl m_TabCtrl;

   int m_ClsPermission;
   CGEOTreeCtrl m_ClassTreeCtrl;
   CGEOTree_Item *pCurrClsItem;

   CGEOTreeCtrl m_SecTreeCtrl;

   int m_Permission;
   CButton m_InhRadioCtrl;
   CButton m_InvRadioCtrl;
   CButton m_VisRadioCtrl;
   CButton m_ModRadioCtrl;
   CButton m_UpdButton;

   afx_msg void OnBnClickedHelp();

   int SetProjects(C_INT_INT_LIST &_ProjectsPermissionList);
   int GetProjects(C_INT_INT_LIST &_ProjectsPermissionList);
   int SetClasses(C_INT_VOIDPTR_LIST &_ClassesPermissionList);
   int GetClasses(C_INT_VOIDPTR_LIST &_ClassesPermissionList);
   int SetSecTabs(C_INT_VOIDPTR_LIST &_TabSecTabsPermissionList);
   int GetSecTabs(C_INT_VOIDPTR_LIST &_SecTabsPermissionList);
   int SetInheritanceUserNames(C_STR_LIST &_InheritanceUserNames);
   int GetInheritanceUserNames(C_STR_LIST &_InheritanceUserNames);

   int OnSelchangedPrjTree(void);
   afx_msg void OnTvnSelchangedPrjTree(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedUpdgrade();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedInheritanceButton();
   afx_msg void OnTvnSelchangedClassTree(NMHDR *pNMHDR, LRESULT *pResult);

   void OnBnClickedClassUpd();
   void OnBnClickedSecUpd();
   afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
   void OnChangeSelectionTabCtrl();
   GSDataModelObjectTypeEnum getGSDataModelObjectTypeSelByTab(void);
   CGEOTree_Item *getCurrClsItem(void);
   void GetSecSelectedCodes(C_INT_INT_LIST &Codes, HTREEITEM hItem = NULL, int sub = 0);
};


// finestra di dialogo C_InheritanceDataUsrDlg

class C_InheritanceDataUsrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(C_InheritanceDataUsrDlg)

public:
	C_InheritanceDataUsrDlg(int _UsrId, CWnd* pParent = NULL);   // costruttore standard
	virtual ~C_InheritanceDataUsrDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_INHERITANCE_DATA_USR };

protected:
   int                UsrId;
   C_INT_INT_STR_LIST UsrList;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
   void MoveSelectedUser(bool ToInheritanceList);
   void RefreshListCtrl(void);

	DECLARE_MESSAGE_MAP()
public:
   C_STR_LIST InheritanceUserNames;

   afx_msg void OnLbnDblclkUsrList();
   afx_msg void OnLbnDblclkInheritanceUsrList();
   afx_msg void OnBnClickedAddInheritanceButton();
   afx_msg void OnBnClickedDelInheritanceButton();
   afx_msg void OnBnClickedHelp();
   CListBox m_UsrList;
   CListBox m_UsrInheritanceList;
};


int gsui_login(void);
int gsui_createusr(void);
int gsui_modipwdcurrusr(void);
int gsui_delusr(void);
int gsui_modusr(void);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLASS_LOGIN_H__46EE741E_ADF5_476B_BF40_C99E072FB7E7__INCLUDED_)#pragma once
