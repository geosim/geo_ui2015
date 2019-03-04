#pragma once

#include "GEOListCtrl.h"
#include "GEOTreeCtrl.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "afxmaskededit.h"

// finestra di dialogo CDlgMyTest

class CDlgMyTest : public CDialog
{
	DECLARE_DYNAMIC(CDlgMyTest)

public:
	CDlgMyTest(CWnd* pParent = NULL);   // costruttore standard
	virtual ~CDlgMyTest();

// Dati della finestra di dialogo
	enum { IDD = IDD_DIALOG4 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();

public:
   CImageList ImageList;

   CGEOListCtrl myList;
   CGEOTreeCtrl myTree;
   //afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
   CMFCMaskedEdit masked_edit_ctrl;
   afx_msg void OnEnChangeMfcmaskededit1();
   afx_msg void OnEnUpdateMfcmaskededit1();
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   afx_msg void OnPaint(void);
   CString m_strFLEK;
   CEdit myedit;
   afx_msg void OnEnKillfocusMfcmaskededit1();
   afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
   CListCtrl mLista;
   afx_msg void OnLvnItemchangedList3(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnNMClickList3(NMHDR *pNMHDR, LRESULT *pResult);
};
