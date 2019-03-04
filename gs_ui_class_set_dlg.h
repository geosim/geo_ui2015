#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "GEOTreeCtrl.h"

// controllo albero personalizzato per gestire drag and drop
class CGEOClsSetTreeCtrl : public CGEOTreeCtrl
{
   public:
      CGEOClsSetTreeCtrl();
	   virtual ~CGEOClsSetTreeCtrl();

   bool Modified;

   // per drag and drop
   BOOL       m_bLDragging;
   CImageList *m_pDragImage;
   HTREEITEM  m_hItemDrag, m_hItemDrop;
   HCURSOR    m_dropCursor, m_noDropCursor;

   afx_msg void OnTvnBegindragTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg void OnRClick(NMHDR* pNMHDR, LRESULT* pResult);

   int AddItem(CGEOTree_Item *pItem, HTREEITEM hParentItem, HTREEITEM hInsertAfterItem = NULL);
   int DelItem(HTREEITEM hItem);
   int UpdItem(CGEOTree_Item *pItem, const TCHAR *_Name,
               const TCHAR *_Descr, const TCHAR *_ImagePath, HTREEITEM hItem);

   void OnSearchByNameMenu(void);
   void OnRefreshMenu(void);
   void OnCutMenu(void);
   void OnPasteMenu(void);

protected:
   	DECLARE_MESSAGE_MAP()

};

// finestra di dialogo CClassSetDlg

class CClassSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CClassSetDlg)

public:
	CClassSetDlg(CWnd* pParent = NULL);   // costruttore standard
	virtual ~CClassSetDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_CLASS_SET_DIALOG };

   int Prj; 

protected:
   int     NextSetClassId;
   HBITMAP hBitmap;

	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()

   BOOL OnInitDialog(void);
   BOOL OnInitPrj(void);
   void OnChangeImage(void);
   int SaveToDB(bool Ask = true);
   int ItemList_to_ClassSet(CGEOTree_ItemList &ItemList, C_CLASS_SET &ClassSet);

public:
   afx_msg void OnBnClickedAddClassSet();
   afx_msg void OnBnClickedDelClassSet();
   afx_msg void OnBnClickedHelp();

   CStatic m_ClassSet_code;
   CEdit m_ClassSet_name;
   CEdit m_ClassSet_descr;
   CEdit m_ClassSet_image;
   CStatic m_ClassSet_picture;
   CButton m_ClassSet_image_browse;
   CButton m_ClassSet_upd;
   CButton m_ClassSet_add;
   CButton m_ClassSet_del;
   CGEOClsSetTreeCtrl m_ClassSetTreeCtrl;
   CComboBox m_ProjectComboCtrl;

   afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnCbnSelchangeProjectCombo();
   afx_msg void OnEnChangeClassSetImageEdit();
   afx_msg void OnEnKillfocusClassSetImageEdit();
   afx_msg void OnBnClickedClassSetImageBrowse();
   afx_msg void OnBnClickedUpdClassSet();
   afx_msg void OnTvnBegindragTree(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedOk();
   afx_msg void OnClose();
   afx_msg void OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult);
};

int gsui_ClassSet(void);
