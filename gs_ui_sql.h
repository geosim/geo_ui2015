#pragma once
#include "afxwin.h"
#include "afxcmn.h"


#ifndef _gs_list_h
   #include "gs_list.h"
#endif

#ifndef _gs_sql_h
   #include "gs_sql.h"
#endif


#define CONN_DLG_DB_CONN_ONLY    1 // impostazione della sola connessione a DB
#define CONN_DLG_CREATE          2 // creazione di una nuova tabella
#define CONN_DLG_CHOICE_ON_TABLE 4 // scelta tra le tabelle presenti 
                                   // (usato se non presenti i bit CONN_DLG_DB_CONN_ONLY e CONN_DLG_CREATE)
#define CONN_DLG_CHOICE_ON_VIEW  8 // scelta tra le viste presenti 
                                   // (usato se non presenti i bit CONN_DLG_DB_CONN_ONLY e CONN_DLG_CREATE)
#define CONN_DLG_CREATE_INPUT_ONLY_PREFIX 16 // creazione di una nuova tabella ma l'utente 
                                             // deve solo inserire un prefisso della tabella
                                             // e non il nome intero (usato se presente il bit CONN_DLG_CREATE)
#define CONN_DLG_CHOICE_CONTAINER_ONLY 32 // impostazione della connessione a un contenitore di tabelle
                                          // (connessione a DB, catalogo e schema) senza il nome della tabella

// CDBConnDlg dialog

class CDBConnDlg : public CDialog
{
	DECLARE_DYNAMIC(CDBConnDlg)

public:
	CDBConnDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDBConnDlg();

// Dialog Data
	enum { IDD = IDD_DB_CONN_DLG };

   int Flags; // bit 1 (=1) Se attivo si vuole selezionare solo una connessione
              //            a database senza indicare un nome di una tabella
              // bit 2 (=2) Se attivo si vuole creare una nuova tabella
              //            altrimenti si vuole scegliere una tabella esistente
              // bit 3 (=4) Se attivo permette la scelta di tabelle
              // bit 4 (=8) Se attivo permette la scelta di viste
   C_STRING    UdlFile;       // Nome del file UDL da usare
   C_2STR_LIST UdlProperties; // Lista delle proprietà UDL
   C_STRING    FullRefTable;  // riferimento completo alla tabella o alla vista

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   C_DBCONNECTION *pConn;
   C_RB_LIST UDLPropertiesDescrList;
   int       Prev_iItem;

	virtual BOOL OnInitDialog();
   virtual BOOL PreTranslateMessage(MSG* pMsg);

   BOOL InitTooltip(void);
   BOOL DisplayTooltip(MSG* pMsg);

   void InitUDLFileList(void);
   void InitCtrls(void);
   void InitPropListCtrl(void);
   void InitTableViewList(void);
   bool getCurrPropertyType(ResourceTypeEnum *PropType = NULL, CString *DefExt = NULL);
   bool EnableOKBotton(void);
   bool SetCurrPropValue(CString &NewValue);
   bool TestConnection(bool Prompt = false, int PrintError = GS_BAD);

   CComboBox m_UdlComboCtrl;
   CStatic   m_TableNameLbl;
   CEdit     m_TableEditCtrl;
   CComboBox m_TableComboCtrl;
   CEdit     m_SchemaEditCtrl;
   CStatic   m_SchemaNameLbl;
   CButton   m_PropButtonCtrl;
   CEdit     m_PropEditCtrl;
   CListCtrl m_PropListCtrl;
   CButton   m_OkButton;
   CButton   m_CancelButton;
   CButton   m_HelpButton;
	CToolTipCtrl *pToolTip;
   CStatic   m_ConnectionState;

public:
   afx_msg void OnCbnSelchangeUdlListCombo();
   afx_msg void OnNMClickUdlPropertiesList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnEnKillfocusPropEdit();
   afx_msg void OnBnClickedPropButton();
   afx_msg void OnEnChangeTableEdit();
   afx_msg void OnBnClickedOk();
   afx_msg void OnCbnSelchangeTableCombo();
   afx_msg void OnCbnEditchangeTableCombo();

   void FromRbList(resbuf *RbList);
   resbuf *ToRbList(void);
   afx_msg void OnBnClickedConnect();
   afx_msg void OnEnChangeSchemaEdit();
};


// Funzioni LISP
int gsui_DBConn(void);
int gsui_ExistingTabConn(void);
int gsui_NewTabConn(void);
int gsui_NewPrefixTabConn(void);
int gsui_TabContainer(void);
