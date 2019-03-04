#if !defined(AFX_MYEDIT_H__F40B1ECE_D529_4E38_8909_D508B8E268B2__INCLUDED_)
#define AFX_MYEDIT_H__F40B1ECE_D529_4E38_8909_D508B8E268B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyEdit.h : header file
//

#include <afxmaskededit.h>

/////////////////////////////////////////////////////////////////////////////
// CMyEdit window

class CMyEdit : public CEdit
{
// Construction
public:
	CMyEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyEdit)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMyEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMyEdit window

class CMyMaskedEdit : public CMFCMaskedEdit
{
// Construction
public:
	CMyMaskedEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyMaskedEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyMaskedEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyMaskedEdit)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMyMaskedEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYEDIT_H__F40B1ECE_D529_4E38_8909_D508B8E268B2__INCLUDED_)
