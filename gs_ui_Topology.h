#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CTopoResistanceDlg dialog

class CTopoResistanceDlg : public CDialog
{
	DECLARE_DYNAMIC(CTopoResistanceDlg)

public:
	CTopoResistanceDlg(CWnd* pParent = NULL, 
                      C_CLASS *pCls = NULL, C_2STR_INT_LIST *pCostSQLList = NULL);   // standard constructor
	virtual ~CTopoResistanceDlg();

// Dialog Data
	enum { IDD = IDD_TOPO_RESISTANCE };


protected:
   BOOL         isChanged;
   int          Prev_iItem;
   int          Curr_iSubItem;

   C_CLASS         *m_pCls;
   C_2STR_INT_LIST m_CostSQLList; // Lista dei costi per la visita topologica
                                  // vedi C_TOPOLOGY::LoadInMemory in GS_TOPO.CPP

   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void RefreshList(void);
   int CComboBoxToC_2STR_INT_LIST(C_2STR_INT_LIST &CostSQLList);

	DECLARE_MESSAGE_MAP()
public:
   CListCtrl m_ResistanceList;

   int set_cls(C_CLASS *pCls);
   int set_CostSQLList(C_2STR_INT_LIST &CostSQLList);
   void get_CostSQLList(C_2STR_INT_LIST &CostSQLList);
	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedCancel();
   CComboBox m_Attrib_Combo;
   afx_msg void OnNMClickResistanceList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnCbnSelchangeCombo1();
   afx_msg void OnCbnKillfocusCombo1();
   afx_msg void OnBnClickedLoadButton();
   afx_msg void OnBnClickedSaveButton();
   CStatic m_LblStaticCtrl;
   afx_msg void OnCbnEditchangeCombo1();
   afx_msg void OnBnClickedButton3();
};


int gsui_topo_propagation(void);
int gsui_topo_shortestpath(void);
