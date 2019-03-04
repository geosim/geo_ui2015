// ValuesListDlg.cpp : implementation file
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"

#include "gs_def.h"
#include "gs_list.h"
#include "gs_resbf.h"
#include "gs_utily.h"
#include "gs_sql.h"

#include "ValuesListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CValuesListDlg dialog


CValuesListDlg::CValuesListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CValuesListDlg::IDD, pParent)
{
   m_SingleSel  = TRUE;
	m_ColsHeader = GS_EMPTYSTR;
	m_ColsWidth  = GS_EMPTYSTR;
   m_OriginType = STRING_TYPE;
   m_StringList = GS_EMPTYSTR;
   m_FilePath   = GS_EMPTYSTR;
   m_Sep        = _T(';');
   m_Width      = 0;

	//{{AFX_DATA_INIT(CValuesListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CValuesListDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CValuesListDlg)
   DDX_Control(pDX, IDC_MSG, m_label);
   DDX_Control(pDX, IDC_LIST, m_List);
   //}}AFX_DATA_MAP
   DDX_Control(pDX, IDOK, m_ok);
   DDX_Control(pDX, IDCANCEL, m_cancel);
}


BEGIN_MESSAGE_MAP(CValuesListDlg, CDialog)
	//{{AFX_MSG_MAP(CValuesListDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CValuesListDlg message handlers

BOOL CValuesListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	LV_ITEM    lvitem;
   int        nItem = 0, nCols, iSubItem, MaxColumnWidth, ColumnWidth, n;
   C_INT_LIST WidthList;
   C_INT      *pWidth;
   CString    ItemText;

   SetWindowText(m_Title);
   if (m_Width > 0)
   {
      CRect rcItem1, rcItem2;
      int   WndX;
   
      GetWindowRect(rcItem1);
      n    = (m_Width - rcItem1.Width()) / 2;
      WndX = rcItem1.left - n;
   	SetWindowPos(NULL, WndX, rcItem1.top, m_Width, rcItem1.Height(), SWP_SHOWWINDOW);

      m_List.GetWindowRect(rcItem1);
    	ScreenToClient(rcItem1);
   	m_List.SetWindowPos(this, rcItem1.left, rcItem1.top, 
                          m_Width - (2 * rcItem1.left), rcItem1.Height(), SWP_NOZORDER);     

      m_ok.GetWindowRect(rcItem1);
    	ScreenToClient(rcItem1);
      n = m_Width - rcItem1.Width();
      m_cancel.GetWindowRect(rcItem2);
    	ScreenToClient(rcItem2);
      n = n - rcItem2.Width();
      n = n / 3;
      m_ok.SetWindowPos(this, n, rcItem1.top, rcItem1.Width(), rcItem1.Height(), SWP_NOZORDER);
      m_cancel.SetWindowPos(this, n + rcItem1.Width() + n, rcItem2.top, rcItem2.Width(), rcItem2.Height(), SWP_NOZORDER);
   }

   m_label.SetWindowText(m_Msg);

	if (m_SingleSel)
		m_List.ModifyStyle(0, LVS_SINGLESEL); // Selezione singola
	else
		m_List.ModifyStyle(LVS_SINGLESEL, 0); // Selezione mutipla

   m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

   lvitem.mask     = LVIF_TEXT;
   lvitem.iSubItem = 0;

   switch (m_OriginType)
   {
      case STRING_TYPE:
      {
         C_STRING Buffer;
         TCHAR    *str;

         // Intestazioni
         Buffer = (LPCTSTR) m_ColsHeader;
         str    = Buffer.get_name();
         nCols  = gsc_strsep(str, _T('\0'), _T(';')) + 1;
         for (nItem = 0; nItem < nCols; nItem++)
         {
         	m_List.InsertColumn(nItem, str, LVCFMT_LEFT, 1, 0);
            while (*str != _T('\0')) str++; str++;
         }

         // Lettura dati       
         Buffer = (LPCTSTR) m_StringList;
         str = Buffer.get_name();
         n   = gsc_strsep(str, _T('\0'), m_Sep) + 1;
         for (nItem = 0; nItem < n; nItem++)
         {
      		lvitem.iItem   = nItem;
            lvitem.pszText = str;
            m_List.InsertItem(&lvitem);
            while (*str != _T('\0')) str++; str++;
         }

         break;
      }
      case DB_TYPE:    
      {
         C_RB_LIST ColValues;
         presbuf   p;
         C_STRING  msg;

         if (gsc_InitDBReadRow(pRs, ColValues) == GS_BAD) return TRUE;
         // Intestazioni
         nCols = 0;
         while ((p = gsc_nth(0, ColValues.nth(nCols))))
         	m_List.InsertColumn(nCols++, p->resval.rstring, LVCFMT_LEFT, 1, 0);

         // Lettura dati
         if (!(p = gsc_nth(1, ColValues.nth(0)))) return TRUE;
         nItem = 0;
         while (gsc_isEOF(pRs) == GS_BAD)
         {
            // leggo riga
            if (gsc_DBReadRow(pRs, ColValues) == GS_BAD) break;

            if (p->restype != RTNONE && p->restype != RTVOID && p->restype != RTNIL)
               msg = p;
            else
               msg = GS_EMPTYSTR;

      		lvitem.iItem   = nItem++;
            lvitem.pszText = msg.get_name();
            m_List.InsertItem(&lvitem);

            gsc_Skip(pRs);
         }

         break;
      }
      case FILE_TYPE:
      {
         C_STRING Buffer;
         TCHAR    *str;
         FILE     *f;

         // Intestazioni
         Buffer = (LPCTSTR) m_ColsHeader;
         str = Buffer.get_name();
         nCols   = gsc_strsep(str, _T('\0'), _T(';')) + 1;
         for (nItem = 0; nItem < nCols; nItem++)
         {
         	m_List.InsertColumn(nItem, str, LVCFMT_LEFT, 1, 0);
            while (*str != _T('\0')) str++; str++;
         }
   
         // Lettura dati
         if ((f = gsc_fopen(m_FilePath, _T("r"))) == NULL) return TRUE;
         nItem = 0;
         while (gsc_readline(f, Buffer) == GS_GOOD)
         {
            str          = Buffer.get_name();
            n            = gsc_strsep(Buffer.get_name(), _T('\0'), m_Sep) + 1;
      		lvitem.iItem = nItem;
            for (iSubItem = 0; iSubItem < n; iSubItem++)
            {
               lvitem.iSubItem = iSubItem;
               lvitem.pszText  = str;
               m_List.InsertItem(&lvitem);
               while (*str != _T('\0')) str++; str++;
            }
         }
         gsc_fclose(f);

         break;
      }
      case RESBUF_LIST:
      {
         C_STRING Buffer;
         TCHAR    *str;
         presbuf  p = RbList.get_head();
         C_STRING msg;

         // Intestazioni
         Buffer = (LPCTSTR) m_ColsHeader;
         str    = Buffer.get_name();
         nCols  = gsc_strsep(str, _T('\0'), _T(';')) + 1;
         for (nItem = 0; nItem < nCols; nItem++)
         {
         	m_List.InsertColumn(nItem, str, LVCFMT_LEFT, 1, 0);
            while (*str != _T('\0')) str++; str++;
         }

         // Lettura dati
         nItem = 0;
         if (p)
            while ((p = p->rbnext))
            {
               for (iSubItem = 0; iSubItem < nCols; iSubItem++)
               {
                  if (!(p = p->rbnext)) break;

                  if (p->restype != RTNONE && p->restype != RTVOID && p->restype != RTNIL)
                     msg = p;
                  else
                     msg = GS_EMPTYSTR;

   		         lvitem.iItem    = nItem;
                  lvitem.iSubItem = iSubItem;
                  lvitem.pszText  = msg.get_name();
                  if (iSubItem == 0)
                     m_List.InsertItem(&lvitem);
                  else
                     m_List.SetItem(&lvitem);

               }
               if (!p || !(p = p->rbnext)) break;
               nItem++;
            }

         break;
      }
   }

   // Larghezza colonne
   WidthList.from_str( m_ColsWidth, _T(';'));
   
   // Calcolo la larghezza delle colonne che non sono dichiarate in m_ColsWidth
   if (nCols > WidthList.get_count())
      for (iSubItem = WidthList.get_count(); iSubItem < nCols; iSubItem++)
      {
         MaxColumnWidth = 15;

         // Ciclo tuttle le righe di quella colonna
         for (n = 0; n < m_List.GetItemCount(); n++)
         {
            ItemText = m_List.GetItemText(n, iSubItem);
            ColumnWidth = m_List.GetStringWidth((LPCTSTR) ItemText) + 15;
            if (MaxColumnWidth < ColumnWidth) MaxColumnWidth = ColumnWidth;
         }
         if ((pWidth = new C_INT(MaxColumnWidth)))
            WidthList.add_tail(pWidth);
      }

   int IncrementalWidth = 0, VirtualListWidth;
   n = 0;
   pWidth = (C_INT *) WidthList.get_head();
   while (pWidth)
   {
      m_List.SetColumnWidth(n++, pWidth->get_key());
      IncrementalWidth += pWidth->get_key();
      pWidth = (C_INT *) WidthList.get_next();
   }
   
   // Se la somma delle larghezze delle colonne è inferiore 
   // alla larghezza della lista allora allargo l'ultima colonna
   CRect Rect;
   m_List.GetWindowRect(&Rect);
   VirtualListWidth = Rect.Width() - 6; // 4
   // Se è comparsa la barra di scorrimento verticale
   if (m_List.GetItemCount() > m_List.GetCountPerPage()) VirtualListWidth -= 16;
   if (IncrementalWidth < VirtualListWidth)
      if ((pWidth = (C_INT *) WidthList.get_tail()))
         m_List.SetColumnWidth(--n, pWidth->get_key() + (VirtualListWidth - IncrementalWidth));
   
   C_INT *p_Value;

   if ((p_Value = (C_INT *) m_ValueList.search_min_key()))
      m_List.EnsureVisible(p_Value->get_key(), FALSE);

   // Seleziono le righe correnti
   p_Value = (C_INT *) m_ValueList.get_head();
   while (p_Value)
   {
      m_List.SetItemState(p_Value->get_key(), LVIS_SELECTED, LVIS_SELECTED);
      p_Value = (C_INT *) m_ValueList.get_next();
   }
   m_List.SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CValuesListDlg::OnOK() 
{
   C_INT *pValue;
   int   nItem;

   m_ValueList.remove_all();
   POSITION pos = m_List.GetFirstSelectedItemPosition();
   while (pos)
   {
      nItem = m_List.GetNextSelectedItem(pos);     
      if ((pValue = new C_INT(nItem)) != NULL)
         m_ValueList.add_tail(pValue);
   }
   
	CDialog::OnOK();
}

void CValuesListDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
   OnOK();
	*pResult = 0;
}
