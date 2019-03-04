#pragma once
#include "afxwin.h"


// finestra di dialogo CLyrDisplayModelDialog

class CLyrDisplayModelDialog : public CDialog
{
	DECLARE_DYNAMIC(CLyrDisplayModelDialog)

public:
	CLyrDisplayModelDialog(CWnd* pParent = NULL);   // costruttore standard
	virtual ~CLyrDisplayModelDialog();

// Dati della finestra di dialogo
	enum { IDD = IDD_LYR_DISPLAY_MODEL };

protected:
   int Prev_iItem;
   int Curr_iSubItem;

	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	void LoadFromGS(void);
   void RefreshIntervalsList(void);
   C_LAYER_DISPLAY_MODEL m_LyrDispModel;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedNew();
   afx_msg void OnBnClickedRemove();
   afx_msg void OnBnClickedSave();
   afx_msg void OnBnClickedLoad();
   afx_msg void OnBnClickedClose();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnEnKillfocusEdit();
   afx_msg void OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedBrowseFile();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

   CListCtrl m_IntervalsList;
   CStatic m_CurrLyrDispModel;
   CEdit m_GenericEdit;
   CButton m_BrowseButton;
   CEdit m_ModelDescriptionEdit;
   afx_msg void OnBnClickedActivate();
   afx_msg void OnBnClickedDeactivate();
   CButton m_SaveButton;
   CButton m_LoadButton;
};

int gsui_lyrdispmodel(void);
