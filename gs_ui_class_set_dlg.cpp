// gs_ui_class_set_dlg.cpp : file di implementazione
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include "gs_ui_class_set_dlg.h"

#include "gs_init.h"
#include "gs_class.h"

#include "gs_ui_utily.h"

#include "d2hMap.h" // doc to help


// controllo albero personalizzato per gestire drag and drop - INIZIO

CGEOClsSetTreeCtrl::CGEOClsSetTreeCtrl() : CGEOTreeCtrl()
{
   m_bLDragging = false;
   m_hItemDrag = m_hItemDrop  = NULL;
   m_dropCursor = LoadCursor(NULL,IDC_ARROW);
   m_noDropCursor = LoadCursor(NULL,IDC_NO);

   Modified = false;
   DisplayEmptyClassSet = true; // per visualizzare i set di classi anche se vuoti
}
CGEOClsSetTreeCtrl::~CGEOClsSetTreeCtrl() {}


BEGIN_MESSAGE_MAP(CGEOClsSetTreeCtrl, CTreeCtrl)
   ON_WM_MOUSEMOVE()
   ON_WM_LBUTTONUP()
   ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_GEOTREECTRL_SEARCH_BY_NAME_MENU, OnSearchByNameMenu)
	ON_COMMAND(ID_GEOTREECTRL_REFRESH_MENU, OnRefreshMenu)
	ON_COMMAND(ID_GEOTREECTRL_CUT_MENU, OnCutMenu)
	ON_COMMAND(ID_GEOTREECTRL_PASTE_MENU, OnPasteMenu)
END_MESSAGE_MAP()

void CGEOClsSetTreeCtrl::OnTvnBegindragTree(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW  pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
   CGEOTree_Item *pItem;
   *pResult = 0;

   m_hItemDrop = NULL;
   if (!(m_hItemDrag = pNMTreeView->itemNew.hItem)) return;
   if ((pItem = (CGEOTree_Item *) GetItemData(m_hItemDrag)) == NULL) return;

   // drag and drop ammesso solo per classi e set di classi
   if (pItem->ObjectType != GSClass && pItem->ObjectType != GSClassSet) return;

   m_bLDragging = true;

   m_pDragImage = CreateDragImage(m_hItemDrag);  // get the image list for dragging
   // CreateDragImage() returns NULL if no image list
   // associated with the tree view control
   if (!m_pDragImage) return;
 
   m_pDragImage->BeginDrag(0, CPoint(-15, -15));
   POINT pt = pNMTreeView->ptDrag;
   ClientToScreen(&pt);
   m_pDragImage->DragEnter(NULL, pt);
   SetCapture();
}

void CGEOClsSetTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
   HTREEITEM hitem;
   UINT      flags;
     
   if (m_bLDragging)
   {
      POINT pt = point;
      ClientToScreen( &pt );
      CImageList::DragMove(pt);
      if ((hitem = HitTest(point, &flags)) != NULL)
      {
         CImageList::DragShowNolock(FALSE);
         SelectDropTarget(hitem);
         m_hItemDrop = hitem;
         CImageList::DragShowNolock(TRUE);
      }

      if (hitem)
         SetCursor(m_dropCursor);
      else
         SetCursor(m_noDropCursor);
   }
   CTreeCtrl::OnMouseMove(nFlags, point);
}

void CGEOClsSetTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
   CTreeCtrl::OnLButtonUp(nFlags, point);
 
   if (m_bLDragging)
   {
      m_bLDragging = false;
      CImageList::DragLeave(this);
      CImageList::EndDrag();
      ReleaseCapture();
 
      delete m_pDragImage;
 
      // Remove drop target highlighting
      SelectDropTarget(NULL);
 
      if (m_hItemDrag == m_hItemDrop) return;
 
      // If Drag item is an ancestor of Drop item then return
      HTREEITEM htiParent = m_hItemDrop;
      while ((htiParent = GetParentItem( htiParent )) != NULL)
      {
         if (htiParent == m_hItemDrag) return;
      }
	   Expand(m_hItemDrop, TVE_EXPAND);
 
      Modified = true;

      // Se l'item di destinazione è unn set di classi
      // chiedo se si vuole inserire dentro il set di classi
      // oppure come fratello del set
      CGEOTree_Item *pInsertAfterItem = (CGEOTree_Item *) GetItemData(m_hItemDrop);

      switch (pInsertAfterItem->ObjectType)
      {
         case GSProject:
            // se si vuole inserire nel progetto
            MoveItem(m_hItemDrag, m_hItemDrop);
            break;
         case GSClassSet:
         {
            C_STRING Msg(_T("Inserire nel set di classi \""));
            Msg += pInsertAfterItem->get_name();
            Msg += _T("\" ?");
         
            if (gsui_confirm(Msg.get_name(), GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
               // se si vuole inserire nel set
               MoveItem(m_hItemDrag, m_hItemDrop);
            else // si vuole inserire come fratello successivo del set
               MoveItem(m_hItemDrag, GetParentItem(m_hItemDrop), m_hItemDrop);

            break;
         }
         case GSClass: // si vuole inserire come fratello successivo
            MoveItem(m_hItemDrag, GetParentItem(m_hItemDrop), m_hItemDrop);
            break;
      }
   }
}

void CGEOClsSetTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
   { CGEOTreeCtrl::OnContextMenu(pWnd, point); }
void CGEOClsSetTreeCtrl::OnRClick(NMHDR* pNMHDR, LRESULT* pResult)
   { CGEOTreeCtrl::OnRClick(pNMHDR, pResult); }
void CGEOClsSetTreeCtrl::OnSearchByNameMenu(void)
   { CGEOTreeCtrl::OnSearchByNameMenu(); }
void CGEOClsSetTreeCtrl::OnRefreshMenu(void)
   { CGEOTreeCtrl::OnRefreshMenu(); }
void CGEOClsSetTreeCtrl::OnCutMenu(void)
   { CGEOTreeCtrl::OnCutMenu(); }
void CGEOClsSetTreeCtrl::OnPasteMenu(void)
{
   CGEOTreeCtrl::OnPasteMenu(); 
   Modified = true;
}

int CGEOClsSetTreeCtrl::AddItem(CGEOTree_Item *pItem, HTREEITEM hParentItem,
                                HTREEITEM hInsertAfterItem)
{
   int res = CGEOTreeCtrl::AddItem(pItem, hParentItem, hInsertAfterItem);
   if (res == GS_GOOD) Modified = true;
   return res;
}
   
int CGEOClsSetTreeCtrl::DelItem(HTREEITEM hItem)
{
   int res = CGEOTreeCtrl::DelItem(hItem);
   if (res == GS_GOOD) Modified = true;
   return res;
}

int CGEOClsSetTreeCtrl::UpdItem(CGEOTree_Item *pItem, const TCHAR *_Name,
                                const TCHAR *_Descr, const TCHAR *_ImagePath, HTREEITEM hItem)
{
   int res = CGEOTreeCtrl::UpdItem(pItem, _Name, _Descr, _ImagePath, pItem->updatable,
                                   hItem);
   if (res == GS_GOOD) Modified = true;
   return res;
}


// controllo albero personalizzato per gestire drag and drop - FINE


// finestra di dialogo CClassSetDlg

IMPLEMENT_DYNAMIC(CClassSetDlg, CDialog)

CClassSetDlg::CClassSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClassSetDlg::IDD, pParent)
{
   Prj     = 0;
   hBitmap = NULL;
}

CClassSetDlg::~CClassSetDlg()
{
   if (hBitmap) DeleteObject(hBitmap);
}

void CClassSetDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_CLASS_SET_NAME_EDIT, m_ClassSet_name);
   DDX_Control(pDX, IDC_CLASS_SET_DESCR_EDIT, m_ClassSet_descr);
   DDX_Control(pDX, IDC_CLASS_SET_IMAGE_EDIT, m_ClassSet_image);
   DDX_Control(pDX, IDC_CLASS_SET_IMAGE_PICTURE, m_ClassSet_picture);
   DDX_Control(pDX, IDC_CLASS_SET_IMAGE_BROWSE, m_ClassSet_image_browse);
   DDX_Control(pDX, IDC_UPD_CLASS_SET, m_ClassSet_upd);
   DDX_Control(pDX, IDC_ADD_CLASS_SET, m_ClassSet_add);
   DDX_Control(pDX, IDC_DEL_CLASS_SET, m_ClassSet_del);
   DDX_Control(pDX, IDC_TREE, m_ClassSetTreeCtrl);
   DDX_Control(pDX, IDC_PROJECT_COMBO, m_ProjectComboCtrl);
   DDX_Control(pDX, IDC_STATIC_CODE, m_ClassSet_code);
}


BEGIN_MESSAGE_MAP(CClassSetDlg, CDialog)
   ON_BN_CLICKED(IDC_ADD_CLASS_SET, &CClassSetDlg::OnBnClickedAddClassSet)
   ON_BN_CLICKED(IDC_DEL_CLASS_SET, &CClassSetDlg::OnBnClickedDelClassSet)
   ON_BN_CLICKED(IDHELP, &CClassSetDlg::OnBnClickedHelp)
   ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, &CClassSetDlg::OnTvnSelchangedTree)
   ON_CBN_SELCHANGE(IDC_PROJECT_COMBO, &CClassSetDlg::OnCbnSelchangeProjectCombo)
   ON_EN_KILLFOCUS(IDC_CLASS_SET_IMAGE_EDIT, &CClassSetDlg::OnEnKillfocusClassSetImageEdit)
   ON_BN_CLICKED(IDC_CLASS_SET_IMAGE_BROWSE, &CClassSetDlg::OnBnClickedClassSetImageBrowse)
   ON_BN_CLICKED(IDC_UPD_CLASS_SET, &CClassSetDlg::OnBnClickedUpdClassSet)
   ON_NOTIFY(TVN_BEGINDRAG, IDC_TREE, &CClassSetDlg::OnTvnBegindragTree)
   ON_BN_CLICKED(IDOK, &CClassSetDlg::OnBnClickedOk)
   ON_WM_CLOSE()
   ON_NOTIFY(NM_RCLICK, IDC_TREE, &CClassSetDlg::OnNMRClickTree)
END_MESSAGE_MAP()


// gestori di messaggi CClassSetDlg


void CClassSetDlg::OnBnClickedAddClassSet()
{
   HTREEITEM     hItem = m_ClassSetTreeCtrl.GetSelectedItem(), hParentItem, hInsertAfterItem;
   CGEOTree_Item *pItem;
   CString       dummy;
   C_STRING      ClassSetName, ClassSetDescr, ClassSetImagePath;

   if (!hItem) return;

   m_ClassSet_name.GetWindowText(dummy);
   ClassSetName = dummy;
   m_ClassSet_descr.GetWindowText(dummy);
   ClassSetDescr = dummy;
   m_ClassSet_image.GetWindowText(dummy);
   ClassSetImagePath = dummy;

   if ((pItem = (CGEOTree_Item *) m_ClassSetTreeCtrl.GetItemData(hItem)) == NULL) return;

   if (pItem->ObjectType == GSClass)
   {
      hParentItem = m_ClassSetTreeCtrl.GetParentItem(hItem);
      hInsertAfterItem = hItem;
   }
   else
   {
      hParentItem = hItem;
      hInsertAfterItem = NULL;
   }

   if ((pItem = new CGEOTree_Item(-1 * (NextSetClassId + 1), ClassSetName.get_name(), 
                                  ClassSetDescr.get_name(), ClassSetImagePath.get_name(),
                                  GSClassSet)) == NULL)
   {
      gsc_ddalert(_T("Set di classi non valido"), m_hWnd);
      return;
   }

   if (m_ClassSetTreeCtrl.AddItem(pItem, hParentItem, hInsertAfterItem) != GS_GOOD)
   {
      delete pItem;
      gsc_ddalert(_T("Set di classi non valido"), m_hWnd);
      return;
   }
   NextSetClassId++;
}


void CClassSetDlg::OnBnClickedDelClassSet()
{
   HTREEITEM     hItem = m_ClassSetTreeCtrl.GetSelectedItem();
   CGEOTree_Item *pItem;

   if (!hItem) return;
   if ((pItem = (CGEOTree_Item *) m_ClassSetTreeCtrl.GetItemData(hItem)) == NULL) return;
   if (pItem->ObjectType != GSClassSet) return;

   C_STRING Msg;

   Msg = _T("Cancellare il set di classi \"");
   Msg += pItem->get_name();
   Msg += _T("\" ?");
   if (gsui_confirm(Msg.get_name(), GS_GOOD, FALSE, FALSE, m_hWnd) == GS_GOOD)
      m_ClassSetTreeCtrl.DelItem(hItem);
}


void CClassSetDlg::OnBnClickedUpdClassSet()
{
   HTREEITEM     hItem = m_ClassSetTreeCtrl.GetSelectedItem();
   CGEOTree_Item *pItem;
   CString       dummy;
   C_STRING      ClassSetName, ClassSetDescr, ClassSetImagePath;

   if (!hItem) return;

   m_ClassSet_name.GetWindowText(dummy);
   ClassSetName = dummy;
   m_ClassSet_descr.GetWindowText(dummy);
   ClassSetDescr = dummy;
   m_ClassSet_image.GetWindowText(dummy);
   ClassSetImagePath = dummy;

   if ((pItem = (CGEOTree_Item *) m_ClassSetTreeCtrl.GetItemData(hItem)) == NULL) return;
   if (pItem->ObjectType != GSClassSet) return;

   if (m_ClassSetTreeCtrl.UpdItem(pItem, ClassSetName.get_name(), 
                                  ClassSetDescr.get_name(), ClassSetImagePath.get_name(),
                                  hItem) != GS_GOOD)
   {
      gsc_ddalert(_T("Set di classi non valido"), m_hWnd);
      return;
   }
}


void CClassSetDlg::OnBnClickedHelp()
{
   gsc_help(IDH_GestioneSetdiclassi);
}

BOOL CClassSetDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   NextSetClassId = 1;
   m_ClassSetTreeCtrl.FinalObjectType = GSClass;
   m_ClassSetTreeCtrl.MultiSelect = false;
   m_ClassSetTreeCtrl.CutAndPaste = true;

   C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->get_head();
   while (m_ProjectComboCtrl.DeleteString(0) != CB_ERR); // svuoto la combo

   while (pPrj)
   {
      m_ProjectComboCtrl.AddString(pPrj->get_name());
      pPrj = (C_PROJECT *) get_GS_PROJECT()->get_next(); 
   }

   if (Prj == 0)
      if (get_GS_CURRENT_WRK_SESSION())
         Prj  = get_GS_CURRENT_WRK_SESSION()->get_pPrj()->get_key();
      else
         gsc_getLastUsedPrj(&Prj);

   if ((pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj)) != NULL)
   {
      m_ProjectComboCtrl.SetWindowText(pPrj->get_name());
      OnInitPrj();      
   }

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CClassSetDlg::OnInitPrj() 
{
   C_PROJECT *pPrj;

   if (!(pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj))) return FALSE;

   m_ClassSetTreeCtrl.FilterOnCodes.remove_all();
   m_ClassSetTreeCtrl.FilterOnCodes.add_tail_int(Prj);
   m_ClassSetTreeCtrl.LoadFromDB(Prj);
   m_ClassSetTreeCtrl.Refresh();
   m_ClassSetTreeCtrl.ExpandAll();
   m_ClassSetTreeCtrl.SetSelectedPrj(Prj);
   m_ClassSetTreeCtrl.SetFocus();

   NextSetClassId = pPrj->getNextClassSetId();
   
   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CClassSetDlg::OnCbnSelchangeProjectCombo()
{
   int       CurSel;
   CString   dummy;
   C_PROJECT *pPrj;

   if ((CurSel = m_ProjectComboCtrl.GetCurSel()) == CB_ERR) return;
   m_ProjectComboCtrl.GetLBText(CurSel, dummy);
   if (!(pPrj = (C_PROJECT *) get_GS_PROJECT()->search_name(dummy))) return;
   if (Prj == pPrj->get_key()) return; // se non è cambiato
   
   if (SaveToDB(true) != GS_GOOD) // ripristino la selezione precedente
   {
      if (!(pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj))) return;
      CurSel = m_ProjectComboCtrl.FindStringExact(-1, pPrj->get_name());
      m_ProjectComboCtrl.SetCurSel(CurSel);
      return;
   }

   Prj = pPrj->get_key();
   OnInitPrj();
}

void CClassSetDlg::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
   NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW*)pNMHDR;
   CGEOTree_Item *pItem = (CGEOTree_Item *) pNMTreeView->itemNew.lParam;

   if (pItem->ObjectType == GSClassSet)
   {
      C_STRING Str;

      Str = pItem->get_key();
      m_ClassSet_code.SetWindowText(Str.get_name());
      m_ClassSet_name.SetWindowText(pItem->get_name());
      m_ClassSet_descr.SetWindowText(pItem->Descr.get_name());
      m_ClassSet_image.SetWindowText(pItem->ImagePath.get_name());
      m_ClassSet_upd.EnableWindow(TRUE);
      m_ClassSet_del.EnableWindow(TRUE);
   }
   else
   {
      m_ClassSet_code.SetWindowText(_T(""));
      m_ClassSet_name.SetWindowText(_T(""));
      m_ClassSet_descr.SetWindowText(_T(""));
      m_ClassSet_image.SetWindowText(_T(""));
      m_ClassSet_upd.EnableWindow(FALSE);
      m_ClassSet_del.EnableWindow(FALSE);
   }

   OnChangeImage();

   *pResult = 0;
}


void CClassSetDlg::OnEnKillfocusClassSetImageEdit()
{
   OnChangeImage();
}

void CClassSetDlg::OnChangeImage()
{
   CString  dummy;
   C_STRING tmp_path;

   if (hBitmap)
   {
      DeleteObject(hBitmap); hBitmap = NULL;
      m_ClassSet_picture.SetBitmap(NULL);
   }

   m_ClassSet_image.GetWindowText(dummy);
   tmp_path = dummy;
   if (gsc_path_exist(tmp_path) == GS_GOOD)
   {
      gsc_nethost2drive(tmp_path);

      hBitmap = (HBITMAP) ::LoadImage(NULL, tmp_path.get_name(),
                                      IMAGE_BITMAP, 0, 0,
                                      LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
      if (hBitmap)
      {
         m_ClassSet_picture.SetBitmap(hBitmap);
         if (tmp_path.comp(dummy) != 0)
            m_ClassSet_image.SetWindowText(tmp_path.get_name());
      }
   }
}

void CClassSetDlg::OnBnClickedClassSetImageBrowse()
{
   C_STRING filename, str;
   CString  dummy;

   m_ClassSet_image.GetWindowText(dummy);

   if (dummy.GetLength() == 0)
   {
      C_PROJECT *pPrj;
         
      if ((pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj)) != NULL)
      {
         str = pPrj->get_dir();
         str += _T('\\');
         str += GEOSUPPORTDIR;
         str += _T('\\');
      }
   }
   else
      str = dummy;

   if (gsc_GetFileD(_T("GEOsim - Selezionare immagine"),
                    str, _T("bmp"), 4, filename) == RTNORM)
   {
      m_ClassSet_image.SetWindowText(filename.get_name());
      OnChangeImage();
   }
}

void CClassSetDlg::OnTvnBegindragTree(NMHDR *pNMHDR, LRESULT *pResult)
{
   m_ClassSetTreeCtrl.OnTvnBegindragTree(pNMHDR, pResult);
}

void CClassSetDlg::OnBnClickedOk()
{
   if (SaveToDB(false) != GS_GOOD) return;
   gsc_setLastUsedPrj(Prj);

   CDialog::OnOK();
}

void CClassSetDlg::OnClose()
{
   if (SaveToDB(true) != GS_GOOD) return;

   EndDialog(IDOK);
}

int CClassSetDlg::SaveToDB(bool Ask)
{
   if (!m_ClassSetTreeCtrl.Modified) return GS_GOOD;
   
   if (Ask)
      if (gsui_confirm(_T("Salvare le modifiche apportate ?"), GS_GOOD, FALSE, FALSE, m_hWnd) != GS_GOOD)
         { m_ClassSetTreeCtrl.Modified = false; return GS_GOOD; }

   C_PROJECT   *pPrj;
   C_CLASS_SET ClassSet;

   if ((pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(Prj)) == NULL) return GS_BAD;
 
   if (ItemList_to_ClassSet(*(m_ClassSetTreeCtrl.get_ItemList_ptr()), ClassSet) != GS_GOOD) return GS_BAD;
   if (ClassSet.to_db(pPrj) != GS_GOOD) return GS_BAD;

   m_ClassSetTreeCtrl.Modified = false;
   return GS_GOOD;
}

int CClassSetDlg::ItemList_to_ClassSet(CGEOTree_ItemList &ItemList, C_CLASS_SET &ClassSet)
{
   CGEOTree_Item *pItem = (CGEOTree_Item *) ItemList.get_head();
   
   while (pItem)
   {
      switch (pItem->ObjectType)
      {
         case GSProject: // la root
            ClassSet.Id = 0;
            ClassSet.Name = _T("root");
            if (ItemList_to_ClassSet(pItem->ChildItemList, ClassSet) == GS_BAD) return GS_BAD;
            break;
         case GSClassSet:
         {
            // Il primo elemento è il codice e il secondo 
            // se = GSClassSet significa set di classi, se = GSClass significa classe di entità
            ClassSet.ClsCodeList.values_add_tail(pItem->get_key(), GSClassSet); 

            C_CLASS_SET *pClsSet;
            if ((pClsSet = new C_CLASS_SET) == NULL) return GS_BAD;
            pClsSet->Id        = pItem->get_key();
            pClsSet->Name      = pItem->get_name();
            pClsSet->Descr     = pItem->Descr;
            pClsSet->ImagePath = pItem->ImagePath;
            ClassSet.ClassSetList.add_tail(pClsSet);
            if (ItemList_to_ClassSet(pItem->ChildItemList, *pClsSet) == GS_BAD) return GS_BAD;
            break;
         }
         case GSClass:
            // Il primo elemento è il codice e il secondo 
            // se = GSClassSet significa set di classi, se = GSClass significa classe di entità
            ClassSet.ClsCodeList.values_add_tail(pItem->get_key(), GSClass); 
            break;
      }

      pItem = (CGEOTree_Item *) pItem->get_next();
   }

   return GS_GOOD;
}


void CClassSetDlg::OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult)
{
   m_ClassSetTreeCtrl.OnRClick(pNMHDR, pResult);
}


/*************************************************************************/
/*.doc gsui_ClassSet                                                     */
/*+
    Comando per gestire i set di classi dei progetti di GEOsim.
-*/
/*************************************************************************/
int gsui_ClassSet(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   acedRetVoid();

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   CClassSetDlg ClassSetDlg;
   if (ClassSetDlg.DoModal() != IDOK) return GS_GOOD;

   return GS_GOOD;
}
