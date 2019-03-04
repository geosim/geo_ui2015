#include "afxwin.h"
#if !defined(AFX_VALUESLISTDLG_H__3C08E834_9D64_450C_B042_4F853439CB39__INCLUDED_)
#define AFX_VALUESLISTDLG_H__3C08E834_9D64_450C_B042_4F853439CB39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ValuesListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CValuesListDlg dialog

class CValuesListDlg : public CDialog
{
// Construction
public:
   CString m_Title;
   int     m_Width;
   CString m_Msg;
   BOOL    m_SingleSel; // Se TRUE è permessa solo la selezione singola (default TRUE)
	CString m_ColsHeader;// Intestazione delle colonne separate da ";"
                        // (es. "col1;col2") (default = "" che corrisponde a 1 colonna)
                        // Non usato nel caso in cui m_OriginType = DB_TYPE
	CString m_ColsWidth; // Larghezza delle colonne (es. "10;15;20")
                        // se = "" vengono calcolate automaticamente (default = "")
   enum OriginTypes
   {
      STRING_TYPE = 0,
      DB_TYPE     = 1,
      FILE_TYPE   = 2,
      RESBUF_LIST = 3
   } m_OriginType;      // Tipo di origine dati: 
                        // se STRING_TYPE i dati sono contenuti in una stringa e separati
                        // tra loro da un carattere separatore
                        // se DB_TYPE i dati sono contenuti in un recordset precedentemente
                        // aperto su cui sia possibile fare operazioni di skip in avanti
                        // se FILE_TYPE i dati sono contenuti in un file separati 
                        // tra loro da un carattere separatore (default = STRING_TYPE)
                        // se RESBUF_LIST i dati sono contenuti in una lista di resbuf
                        // del tipo ((<valore1><valore2>) (<valore3><valore4>) ...)
   CString       m_StringList; // Usato se m_OriginType = STRING_TYPE (es. "riga1;riga2")
   _RecordsetPtr pRs;          // Usato se m_OriginType = DB_TYPE
   CString       m_FilePath;   // Usato se m_OriginType = FILE_TYPE
   C_RB_LIST     RbList;       // Usato se m_OriginType = RESBUF_LIST
   TCHAR         m_Sep;        // Separatore di valori usato per origine dati stringa e
                               // origine dati File (default = ';')
   C_INT_LIST    m_ValueList;  // righe selezionate (0-index)

	CValuesListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CValuesListDlg)
	enum { IDD = IDD_VALUESLISTDLG };
	CStatic	 m_label;
	CListCtrl m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CValuesListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CValuesListDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
   CButton m_ok;
   CButton m_cancel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VALUESLISTDLG_H__3C08E834_9D64_450C_B042_4F853439CB39__INCLUDED_)
