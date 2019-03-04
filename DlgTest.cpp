// DlgTest.cpp : file di implementazione
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"

#include "DlgTest.h"
#include "GEOListCtrl.h"
#include "gs_utily.h"
#include "gs_ui_utily.h"

#include "afxdialogex.h"


// finestra di dialogo CDlgMyTest

IMPLEMENT_DYNAMIC(CDlgMyTest, CDialog)

CDlgMyTest::CDlgMyTest(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMyTest::IDD, pParent)
{

}

CDlgMyTest::~CDlgMyTest()
{
}

void CDlgMyTest::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_LIST1, myList);
   DDX_Control(pDX, IDC_TREE1, myTree);
   DDX_Control(pDX, IDC_MFCMASKEDEDIT1, masked_edit_ctrl);
   DDX_Text(pDX, IDC_MFCMASKEDEDIT1, m_strFLEK);
   DDX_Control(pDX, IDC_EDIT1, myedit);
   DDX_Control(pDX, IDC_LIST3, mLista);
}


BEGIN_MESSAGE_MAP(CDlgMyTest, CDialog)
   //ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CDlgMyTest::OnLvnItemchangedList1)
   ON_EN_CHANGE(IDC_MFCMASKEDEDIT1, &CDlgMyTest::OnEnChangeMfcmaskededit1)
   ON_EN_UPDATE(IDC_MFCMASKEDEDIT1, &CDlgMyTest::OnEnUpdateMfcmaskededit1)
   ON_WM_CTLCOLOR()
   ON_WM_KEYDOWN()
   ON_WM_PAINT()
   ON_EN_KILLFOCUS(IDC_MFCMASKEDEDIT1, &CDlgMyTest::OnEnKillfocusMfcmaskededit1)
   ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CDlgMyTest::OnTvnSelchangedTree1)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST3, &CDlgMyTest::OnLvnItemchangedList3)
   ON_NOTIFY(NM_CLICK, IDC_LIST3, &CDlgMyTest::OnNMClickList3)
END_MESSAGE_MAP()


// gestori di messaggi CDlgMyTest

BOOL CDlgMyTest::OnInitDialog() 
{
	CDialog::OnInitDialog();

   myList.FilterOnPrj = 1;
   myList.ObjectType = GSClass;
   myList.LoadFromDB();
   myList.Refresh();

   C_INT_LIST SelectedCodes;
   SelectedCodes.add_tail_int(1);
   SelectedCodes.add_tail_int(2);

   myTree.FilterOnCodes.add_tail_int(1);
   myTree.FinalObjectType = GSClass;
   myTree.MultiSelect = true;
   myTree.LoadFromDB();
   myTree.Refresh();
   myTree.SetSelectedCodes(SelectedCodes, true);
   
   masked_edit_ctrl.EnableGetMaskedCharsOnly(TRUE);
   
   mLista.InsertColumn(0, _T("Nome"), LVCFMT_LEFT, 50, 0);
   mLista.InsertItem(0, _T("aaa"));
   mLista.InsertItem(1, _T("bbb"));
   mLista.InsertItem(2, _T("ccc"));
   mLista.RedrawItems(0, 2);

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//void CDlgMyTest::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
//{
//   LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
//   // TODO: aggiungere qui il codice per la gestione della notifica del controllo.
//   *pResult = 0;
//}


void CDlgMyTest::OnEnChangeMfcmaskededit1()
{
   CString dummy;
   CWnd *p;

   //masked_edit_ctrl.UpdateData();
   //dummy = m_strFLEK;
   //masked_edit_ctrl.SendMessage( WM_COMMAND, MAKEWPARAM( IDC_MFCMASKEDEDIT1, EN_UPDATE ), (LPARAM)masked_edit_ctrl.m_hWnd );
   //masked_edit_ctrl.GetWindowText(dummy);
   p = (CWnd *) GetDlgItem(IDC_MFCMASKEDEDIT1);
   ((CWnd *) p)->GetWindowText(dummy);

   TCHAR Buffer[256];
	::GetWindowText(masked_edit_ctrl.m_hWnd, Buffer, 256);
}


void CDlgMyTest::OnEnUpdateMfcmaskededit1()
{
   CString dummy;
   masked_edit_ctrl.GetWindowText(dummy);
}

HBRUSH CDlgMyTest::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

   //if(pWnd->GetDlgCtrlID() == masked_edit_ctrl.GetDlgCtrlID())
   //{
   //   CString dummy;
   //   masked_edit_ctrl.GetWindowText(dummy);
   //   myedit.SetWindowText(dummy);
   //}

return hbr;
}
void CDlgMyTest::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
      //CString dummy;
      //masked_edit_ctrl.GetWindowText(dummy);
      //myedit.SetWindowText(dummy);
}
void CDlgMyTest::OnPaint(void)
{
   CDialog::OnPaint();
      //CString dummy;
      //masked_edit_ctrl.GetWindowText(dummy);
      //myedit.SetWindowText(dummy);
}

void CDlgMyTest::OnEnKillfocusMfcmaskededit1()
{
      CString dummy;
      masked_edit_ctrl.GetWindowText(dummy);
      myedit.SetWindowText(dummy);
}


void CDlgMyTest::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
   // TODO: aggiungere qui il codice per la gestione della notifica del controllo.
   *pResult = 0;
}


void CDlgMyTest::OnLvnItemchangedList3(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMLISTVIEW pNMListView = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
   *pResult = 0;

   if (pNMListView->uChanged & LVIF_STATE &&
      ((pNMListView->uNewState & LVNI_SELECTED) != (pNMListView->uOldState & LVNI_SELECTED)))
      if (pNMListView->iItem != 0)
         mLista.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
   return;
}


void CDlgMyTest::OnNMClickList3(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

   for (int i = 0; i < mLista.GetItemCount(); i++)
      mLista.GetItemState(i, LVIS_SELECTED);

   *pResult = 0;
}
