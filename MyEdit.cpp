// MyEdit.cpp : implementation file
//

#include "stdafx.h"
#include "MyEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMyEdit

CMyEdit::CMyEdit()
{
}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	//{{AFX_MSG_MAP(CMyEdit)
	ON_WM_CONTEXTMENU()
   ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyEdit message handlers

void CMyEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
{
   LPARAM lParam;

   if (!GetParent()) return;

   // Trasformo in coordinate relative alla finestra parent
   ClientToScreen(&point);
   GetParent()->ScreenToClient(&point);

   lParam = MAKELONG(point.x, point.y);
   int xPos = LOWORD(lParam); 
   int yPos = HIWORD(lParam); 

   ::SendMessage(GetParent()->m_hWnd, WM_LBUTTONDBLCLK , (WPARAM) nFlags, lParam);
}

void CMyEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here
   LPARAM lParam;

   if (!GetParent()) return;

   lParam = MAKELONG(point.x, point.y);
   int xPos = LOWORD(lParam); 
   int yPos = HIWORD(lParam); 
   ::SendMessage(GetParent()->m_hWnd, WM_CONTEXTMENU, (WPARAM) m_hWnd, lParam);
}

BOOL CMyEdit::PreTranslateMessage(MSG* pMsg) 
{
   // se viene premuto il tasto Alt non deve scatenare alcuna azione
   // (di default Alt viene passato alla finestra madre che lo passa ad autocad
   // che lo interpreta come richiesta di menu)
	if (pMsg->message == WM_SYSKEYDOWN)
      if (pMsg->wParam == VK_MENU)
         return TRUE;

	if (GetParent() && (pMsg->message == WM_KEYDOWN) && 
       (pMsg->wParam == VK_DOWN || pMsg->wParam == VK_UP ||
        pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE ||
        pMsg->wParam == VK_TAB))

   {
      ::SendMessage(GetParent()->m_hWnd, WM_KEYDOWN, (WPARAM) pMsg->wParam, 0);
      return 1;
   }
   else
		return CEdit::PreTranslateMessage(pMsg);
}


/////////////////////////////////////////////////////////////////////////////
// CMyMaskedEdit

CMyMaskedEdit::CMyMaskedEdit()
{
}

CMyMaskedEdit::~CMyMaskedEdit()
{
}


BEGIN_MESSAGE_MAP(CMyMaskedEdit, CMFCMaskedEdit)
	//{{AFX_MSG_MAP(CMyEdit)
	ON_WM_CONTEXTMENU()
   ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyEdit message handlers
void CMyMaskedEdit::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
   if (!GetParent()) return;

   LPARAM lParam;

   lParam = MAKELONG(point.x, point.y);
   int xPos = LOWORD(lParam); 
   int yPos = HIWORD(lParam); 
   ::SendMessage(GetParent()->m_hWnd, WM_CONTEXTMENU, (WPARAM) m_hWnd, lParam);
}

void CMyMaskedEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
{
   LPARAM lParam;

   if (!GetParent()) return;

   // Trasformo in coordinate relative alla finestra parent
   ClientToScreen(&point);
   GetParent()->ScreenToClient(&point);

   lParam = MAKELONG(point.x, point.y);
   int xPos = LOWORD(lParam); 
   int yPos = HIWORD(lParam); 

   ::SendMessage(GetParent()->m_hWnd, WM_LBUTTONDBLCLK , (WPARAM) nFlags, lParam);
}

BOOL CMyMaskedEdit::PreTranslateMessage(MSG* pMsg) 
{
   // se viene premuto il tasto Alt non deve scatenare alcuna azione
   // (di default Alt viene passato alla finestra madre che lo passa ad autocad
   // che lo interpreta come richiesta di menu)
	if (pMsg->message == WM_SYSKEYDOWN)
      if (pMsg->wParam == VK_MENU)
         return TRUE;

	if (GetParent() && (pMsg->message == WM_KEYDOWN) && 
       (pMsg->wParam == VK_DOWN || pMsg->wParam == VK_UP || 
        pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE ||
        pMsg->wParam == VK_TAB))
   {
      ::SendMessage(GetParent()->m_hWnd, WM_KEYDOWN, (WPARAM) pMsg->wParam, 0);
      return 1;
   }
   else
	   return CMFCMaskedEdit::PreTranslateMessage(pMsg);
}
