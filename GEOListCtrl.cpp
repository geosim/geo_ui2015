// GEOList.cpp : file di implementazione
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "GEOListCtrl.h"

#include "gs_prjct.h"
#include "gs_class.h" 
#include "gs_sec.h"


#define HDR_BITMAP_WIDTH   8
#define HDR_BITMAP_HEIGHT  8
#define LARGE_BITMAP_WIDTH 32
#define BITMAP_WIDTH   16
#define BITMAP_HEIGHT  16

//----------------------------------------------------------------------------//
//   INIZIO class CGEOList_Item                                               //
//----------------------------------------------------------------------------//

CGEOList_Item::CGEOList_Item() : C_INT_STR()
{
   category = 0;
	type     = 0;
	updatable = GSReadOnlyData;
	extracted = false;
	image     = 0;
   selected  = false;
}

CGEOList_Item::~CGEOList_Item()
{}

int CGEOList_Item::get_type()
   { return type; }

int CGEOList_Item::get_category()
   { return category; }

int CGEOList_Item::get_state()
{
	if (extracted)
   {
      if (updatable != GSUpdateableData)
         return 1; // lampadina accesa - lucchetto chiuso
      else
         return 2; // lampadina accesa - lucchetto aperto
   }
	else
      if (updatable != GSUpdateableData)
         return 3; // lampadina spenta - lucchetto chiuso
      else
         return 4; // lampadina spenta - lucchetto aperto
}


//----------------------------------------------------------------------------//
//    FINE class CGEOList_Item                                                //
//    INIZIO class CGEOList_ItemList                                          //
//----------------------------------------------------------------------------//


CGEOList_ItemList::CGEOList_ItemList() : C_LIST() {}
      
CGEOList_ItemList::~CGEOList_ItemList()
{
   ImageList.DeleteImageList();
}

CGEOList_Item *CGEOList_ItemList::AddItem(int _Key, const TCHAR *_Name, const TCHAR *_Descr)
{
   CGEOList_Item *p;

   if ((p = new (CGEOList_Item)) == NULL) return NULL;
   p->set_key(_Key);
   p->set_name(_Name);
   p->Descr = _Descr;
   add_tail(p);

   return p;
}


/*********************************************************/
/*.doc CGEOList_ItemList::LoadPrjs            <external> */
/*+
  Questa funzione carica la lista dei progetti di GEOsim.
  Parametri:

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOList_ItemList::LoadPrjs(void)
{
   C_PROJECT     *pPrj = (C_PROJECT *) get_GS_PROJECT()->get_head();
   CGEOList_Item *pItem;
   CBitmap        CBMP;

   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   remove_all();
   ImageList.DeleteImageList();

   if (pPrj)
   {
      // creazione lista immagini
      if (ImageList.Create(LARGE_BITMAP_WIDTH, BITMAP_HEIGHT,
                           ILC_MASK | ILC_COLORDDB, 
                           1, // n. bitmap
                           0) == NULL)
		   return GS_BAD;

      if (pPrj->get_Bitmap(true, CBMP) == GS_BAD) return GS_BAD; // carico la bitmap larga 32 pixel
      ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente
 
      while (pPrj)
      {
         if ((pItem = AddItem(pPrj->get_key(), pPrj->get_name(), pPrj->get_descr())) == NULL)
            return GS_BAD;
         pItem->category = pItem->type = 0;
         pItem->updatable = pPrj->get_level();
         if (get_GS_CURRENT_WRK_SESSION() && get_GS_CURRENT_WRK_SESSION()->get_PrjId() == pPrj->get_key())
            pItem->extracted = true;
         else
            pItem->extracted = false;
         pItem->image = 0;

         pPrj = (C_PROJECT *) pPrj->get_next(); 
      }
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOList_ItemList::LoadClasses            <external> */
/*+
  Questa funzione carica la lista delle classi di un progetto di GEOsim.
  Parametri:
  int prj;                 Codice progetto

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOList_ItemList::LoadClasses(int prj)
{
   C_PROJECT          *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(prj);
   C_SINTH_CLASS_LIST SinthClassList;
   C_SINTH_CLASS      *pSinthClass;
   CGEOList_Item      *pItem;
   CBitmap            CBMP;
   int                i = 0;

   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   remove_all();
   ImageList.DeleteImageList();

   if (!pPrj) return GS_BAD;

   if (pPrj->getSinthClassList(SinthClassList) == GS_BAD) return GS_BAD;
   pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_head();

   if (pSinthClass)
   {
      // creazione lista immagini
      if (ImageList.Create(LARGE_BITMAP_WIDTH, BITMAP_HEIGHT,
                           ILC_MASK | ILC_COLORDDB, 
                           SinthClassList.get_count(), // n. bitmap
                           0) == NULL)
		   return GS_BAD;
    
      while (pSinthClass)
      {
         if ((pItem =AddItem(pSinthClass->get_key(), pSinthClass->get_name(), pSinthClass->get_Descr())) == NULL)
            return GS_BAD;
         pItem->category  = pSinthClass->get_category();
         pItem->type      = pSinthClass->get_type();
         pItem->updatable = pSinthClass->get_level();
         pItem->extracted = (pSinthClass->get_extracted() == EXTRACTED ||
                             pSinthClass->get_extracted() == EXTR_SEL) ? true : false;
         pItem->image     = i++;

         if (pSinthClass->get_Bitmap(true, CBMP) == GS_BAD) return GS_BAD; // carico la bitmap larga 32 pixel
         ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente

         pSinthClass = (C_SINTH_CLASS *) pSinthClass->get_next();
      }

      // Aggiungo in fondo alla lista delle bitmap altre 4 immagini
      // bitmap simbolo "estratta e non modificabile" = "lampadina accesa - lucchetto chiuso"
      if (gsc_get_Bitmap(GSExtractedLockedBmp_32X16, CBMP) == GS_BAD) return GS_BAD;
      ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente  
      // bitmap simbolo "estratta e modificabile" = "lampadina accesa - lucchetto aperto"
      if (gsc_get_Bitmap(GSExtractedUnLockedBmp_32X16, CBMP) == GS_BAD) return GS_BAD;
      ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente  
      // bitmap simbolo "non estratta e non modificabile" = "lampadina spenta - lucchetto chiuso"
      if (gsc_get_Bitmap(GSUnextractedLockedBmp_32X16, CBMP) == GS_BAD) return GS_BAD;
      ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente  
      // bitmap simbolo "non estratta e modificabile" = "lampadina spenta - lucchetto aperto"
      if (gsc_get_Bitmap(GSUnextractedUnLockedBmp_32X16, CBMP) == GS_BAD) return GS_BAD;
      ImageList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente  
   }

   return GS_GOOD;
}


/****************************************************************/
/*.doc CGEOList_ItemList::RefreshClasses             <external> */
/*+
  Questa funzione aggiorna la lista rileggendo nome, descrizione,
  flag di modificabilità e di estrazione dai DB di GEOsim.
  Parametri:
  int prj;                 Codice progetto
  
  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/****************************************************************/
int CGEOList_ItemList::RefreshClasses(int prj)
{
   C_PROJECT          *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(prj);
   C_SINTH_CLASS_LIST SinthClassList;
   C_SINTH_CLASS      *pSinthClass;
   CGEOList_Item      *pItem;

   if (!pPrj) return GS_BAD;

   if (pPrj->getSinthClassList(SinthClassList) == GS_BAD) return GS_BAD;

   pItem = (CGEOList_Item *) get_head();
   while (pItem)
   {
      if ((pSinthClass = (C_SINTH_CLASS *) SinthClassList.search_key(pItem->get_key())))
      {
         pItem->set_name(pSinthClass->get_name());
         pItem->Descr = pSinthClass->get_Descr();
         pItem->updatable = pSinthClass->get_level();
         pItem->extracted = (pSinthClass->get_extracted() == EXTRACTED ||
                             pSinthClass->get_extracted() == EXTR_SEL) ? true : false;
      }

      pItem = (CGEOList_Item *) get_next();
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOList_ItemList::LoadSubClasses            <external> */
/*+
  Questa funzione carica la lista delle sottoclassi di GEOsim.
  Parametri:
  int prj;                 Codice progetto
  int cls;                 Codice classe
  
  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOList_ItemList::LoadSubClasses(int prj, int cls)
{
   C_PROJECT          *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(prj);
   C_SINTH_CLASS_LIST SinthClassList;
   C_SINTH_CLASS      *pSinthClass;
   CGEOList_Item      *pItem;
   CBitmap            CBMP;
   int                i = 0;

   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   remove_all();
   ImageList.DeleteImageList();

   if (!pPrj) return GS_BAD;

   if (pPrj->getSinthClassList(SinthClassList) == GS_BAD) return GS_BAD;
   if ((pSinthClass = (C_SINTH_CLASS *) SinthClassList.search_key(cls)) == NULL)
      return GS_BAD;

   if (pSinthClass)
   {
      pSinthClass = (C_SINTH_CLASS *) pSinthClass->ptr_sub_list()->get_head();
      if (pSinthClass)
      {
         // creazione lista immagini
         if (ImageList.Create(LARGE_BITMAP_WIDTH, BITMAP_HEIGHT,
                              ILC_MASK | ILC_COLORDDB, 
                              pSinthClass->ptr_sub_list()->get_count(), // n. bitmap
                              0) == NULL)
		      return GS_BAD;

         while (pSinthClass)
         {
            if ((pItem = AddItem(pSinthClass->get_key(), pSinthClass->get_name(), pSinthClass->get_Descr())) == NULL)
               return GS_BAD;
            pItem->category  = pSinthClass->get_category();
            pItem->type      = pSinthClass->get_type();
            pItem->updatable = pSinthClass->get_level();
            pItem->extracted = (pSinthClass->get_extracted() == EXTRACTED ||
                                pSinthClass->get_extracted() == EXTR_SEL) ? true : false;
            pItem->image     = i++;

            if (pSinthClass->get_Bitmap(true, CBMP) == GS_BAD) return GS_BAD; // carico la bitmap larga 32 pixel
            ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente

            pSinthClass = (C_SINTH_CLASS *) pSinthClass->get_next();
         }
      }
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOList_ItemList::LoadSecTabs         <external> */
/*+
  Questa funzione carica la lista delle tabelle secondarie di una classe 
  o sottoclasse di GEOsim.
  Parametri:
  int prj;                 Codice progetto
  int cls;                 Codice classe
  int sub;                 Opzionale, Codice sottoclasse (default = 0)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOList_ItemList::LoadSecTabs(int prj, int cls, int sub)
{
   C_PROJECT            *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(prj);
   C_CLASS              *pCls = pPrj->find_class(cls, sub);
   C_SINTH_SEC_TAB_LIST SinthSecList;
   C_SINTH_SEC_TAB      *pSinthSec;
   C_SECONDARY          *pSecondary;
   CGEOList_Item        *pItem;
   CBitmap              CBMP;

   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   remove_all();
   ImageList.DeleteImageList();

   if (!pCls) return GS_BAD;

   if (pPrj->getSinthClsSecondaryTabList(cls, sub, SinthSecList) == GS_BAD) return GS_BAD;

   if ((pSinthSec = (C_SINTH_SEC_TAB *) SinthSecList.get_head()))
   {
      // creazione lista immagini
      if (ImageList.Create(LARGE_BITMAP_WIDTH, BITMAP_HEIGHT,
                           ILC_MASK | ILC_COLORDDB, 
                           1, // n. bitmap
                           0) == NULL)
		   return GS_BAD;

      if (gsc_getSecTabBitmap(true, CBMP) == GS_BAD)
         return GS_BAD; // carico la bitmap larga 32 pixel
      ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente

      while (pSinthSec)
      {
         if ((pSecondary = (C_SECONDARY *) pCls->find_sec(pSinthSec->get_key())) == NULL)
            return GS_BAD;

         if ((pItem = AddItem(pSinthSec->get_key(), pSinthSec->get_name(), pSinthSec->get_Descr())) == NULL)
            return GS_BAD;
         pItem->category  = 0;
         pItem->type      = pSinthSec->get_type();
         if (pCls->ptr_id()->abilit != GSUpdateableData || 
             pSinthSec->get_type() == GSExternalSecondaryTable)
            pItem->updatable = GSReadOnlyData;
         else
            pItem->updatable = GSUpdateableData;
         pItem->extracted = (pCls->is_extracted() == GS_GOOD) ? true : false;
         pItem->image     = 0;

         pSinthSec = (C_SINTH_SEC_TAB *) pSinthSec->get_next();
      }
   }

   return GS_GOOD;
}


// CGEOListCtrl

IMPLEMENT_DYNAMIC(CGEOListCtrl, CListCtrl)

CGEOListCtrl::CGEOListCtrl()
{
   FilterOnPrj = 0;
   FilterOnCls = 0;
   FilterOnSub = 0;
   ObjectType = GSClass; // Lista classi
   pToolTip = NULL;
   UseFilterOnCodes = false;
   FilterOnExtractedInCurrWrkSession  = false;
   FilterOnUpdateable = false;
   ColumnClassStatusVisibility = true;
   ColumnAutosize = true;

   Prev_HeaderPos = -1;
   AscendingOrder = false;
   OnRefresh      = false;
   insideCalcColumnSize = false;
}

CGEOListCtrl::~CGEOListCtrl()
{
   if (pToolTip) delete pToolTip;
}


/*********************************************************/
/*.doc CGEOListCtrl::LoadFromDB            <external> */
/*+
  Questa funzione carica la lista da DB.
  Parametri:
  bool OnlyRefresh;     usato solo per le classi di entità

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOListCtrl::LoadFromDB(bool OnlyRefresh)
{
   switch (ObjectType)
   {
      case GSProject:
         return ItemList.LoadPrjs();
      case GSClass:
         if (OnlyRefresh)
            return ItemList.RefreshClasses(FilterOnPrj);
         else
            return ItemList.LoadClasses(FilterOnPrj);
      case GSSubClass:
         return ItemList.LoadSubClasses(FilterOnPrj, FilterOnCls);
      case GSSecondaryTab:
         return ItemList.LoadSecTabs(FilterOnPrj, FilterOnCls, FilterOnSub);
      default:
         return GS_BAD;
   }
}


/*********************************************************/
/*.doc CGEOListCtrl::Refresh                  <external> */
/*+
  Questa funzione visualizza gli elementi della lista applicando
  le impostazioni di visualizzazione.
  Parametri:
  bool OnlyRefresh;  Se true aggiorna solo la lista altrimenti 
                     la ricostruisce da zero (default = false).

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOListCtrl::Refresh(bool OnlyRefresh)
{
   switch (ObjectType)
   {
      case GSProject:
         if (Refresh_Prjs(OnlyRefresh) == GS_BAD) return GS_BAD;
         break;
      case GSClass:
         if (Refresh_Classes(OnlyRefresh) == GS_BAD) return GS_BAD;
         break;
      case GSSubClass:
         if (Refresh_SubClasses(OnlyRefresh) == GS_BAD) return GS_BAD;
         break;
      case GSSecondaryTab:
         if (Refresh_SecTabs(OnlyRefresh) == GS_BAD) return GS_BAD;
         break;
      default:
         return GS_BAD;
   }

   if (!OnlyRefresh)
      if (InitTooltip() == FALSE) return GS_BAD;

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOListCtrl::Reset_List          <external> */
/*+
  Questa funzione svuota la lista e la imposta correttamente.
-*/  
/*********************************************************/
void CGEOListCtrl::Reset_List(void)
{
   CBitmap CBMP;

   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   // svuoto la lista
   while (DeleteColumn(0) != 0);
   DeleteAllItems();
	if (MultiSelect)
		ModifyStyle(LVS_SINGLESEL, LVS_REPORT | LVS_SHOWSELALWAYS);
	else
		ModifyStyle(0, LVS_SINGLESEL | LVS_REPORT | LVS_SHOWSELALWAYS);
   // Per evidenziare l'intera riga come selezionata e
   // per inserire immagini anche in colonne che non siano la prima
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES); 

   SetImageList(&(ItemList.ImageList), LVSIL_SMALL);

   ImageHdrList.DeleteImageList();
   ImageHdrList.Create(HDR_BITMAP_WIDTH, HDR_BITMAP_HEIGHT,
                       ILC_MASK | ILC_COLORDDB, 
                       2, // n. bitmap
                       0);
   // bitmap simbolo "ordine crescente"
   if (gsc_get_Bitmap(GSAscendingOrderBmp_8X8, CBMP) == GS_BAD) return;
   ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente  
   // bitmap simbolo "ordine decrescente"
   if (gsc_get_Bitmap(GSDescendingOrderBmp_8X8, CBMP) == GS_BAD) return;
   ImageHdrList.Add(&CBMP, RGB(255, 0, 0)); // il rosso diventa trasparente

   CHeaderCtrl* pHdrCtrl = GetHeaderCtrl();
   if (pHdrCtrl) pHdrCtrl->SetImageList(&ImageHdrList);
}


/*********************************************************/
/*.doc CGEOListCtrl::CalcColumnSize           <external> */
/*+
  Questa funzione calcola le dimensioni delle colonne del controllo.
-*/  
/*********************************************************/
void CGEOListCtrl::CalcColumnSize(void)
{
   switch (ObjectType)
   {
      case GSProject:
         return CalcColumnSize_Prjs();
      case GSClass:
         return CalcColumnSize_Classes();
      case GSSubClass:
         return CalcColumnSize_SubClasses();
      case GSSecondaryTab:
         return CalcColumnSize_SecTabs();
   }
}


/*********************************************************/
/*.doc CGEOListCtrl::CalcColumnSize_Prjs           <external> */
/*+
  Questa funzione calcola le dimensioni delle colonne del controllo
  quando lista i progetti.
-*/  
/*********************************************************/
void CGEOListCtrl::CalcColumnSize_Prjs(void)
{
   // poichè SetColumnWidth scatena un messaggio WM_SIZE che richiama la CalcColumnSize
   // si avrebbe un loop infinito che viene bloccato dalla variabile insideCalcColumnSize
   insideCalcColumnSize = true;

   CRect Rect;

   GetClientRect(&Rect);
   SetColumnWidth(0, Rect.Width());

   insideCalcColumnSize = false;
}


/*********************************************************/
/*.doc CGEOListCtrl::CalcColumnSize_Classes   <external> */
/*+
  Questa funzione calcola le dimensioni delle colonne del controllo
  quando lista le classi.
-*/  
/*********************************************************/
void CGEOListCtrl::CalcColumnSize_Classes(void)
{
   // poichè SetColumnWidth scatena un messaggio WM_SIZE che richiama la CalcColumnSize
   // si avrebbe un loop infinito che viene bloccato dalla variabile insideCalcColumnSize
   insideCalcColumnSize = true;

   int   Width, FirstColumnSize = 36, ThirdColumnSize = 40, SecondColumnMinSize = 50;
   CRect Rect;

   GetClientRect(&Rect);
   Width = Rect.Width();
   SetColumnWidth(0, FirstColumnSize);
   Width -= FirstColumnSize;

	if (ColumnClassStatusVisibility)
   {
      SetColumnWidth(2, ThirdColumnSize);
      Width -= ThirdColumnSize;
   }

   // se la scrollbar verticale è visibile non serve più perchè in win8
   // la GetClientRect ritorna la dimensione tenendo conto della presenza della scrollbar verticale
   //if (GetItemCount() > GetCountPerPage()) Width -= 17;
   if (Width < SecondColumnMinSize) Width = SecondColumnMinSize;     
   SetColumnWidth(1, Width);

   insideCalcColumnSize = false;
}


/*********************************************************/
/*.doc CGEOListCtrl::CalcColumnSize_SubClasses <external> */
/*+
  Questa funzione calcola le dimensioni delle colonne del controllo
  quando lista le sotto-classi.
-*/  
/*********************************************************/
void CGEOListCtrl::CalcColumnSize_SubClasses(void)
   { return CalcColumnSize_Classes(); }


/*********************************************************/
/*.doc CGEOListCtrl::CalcColumnSize_SecTabs   <internal> */
/*+
  Questa funzione calcola le dimensioni delle colonne del controllo
  quando lista le tabelle secondarie.
-*/  
/*********************************************************/
void CGEOListCtrl::CalcColumnSize_SecTabs(void)
   { return CalcColumnSize_Classes(); }


/*****************************************************************************/
/*.doc CGEOListCtrl::SetSelectedCodes e GetSelectedCodes          <external> */
/*+
  Questa funzione setta e legge i codici degli elementi da selezionare.
  Parametri:
  C_INT_LIST &SelectedCodes;
  bool AppendMode;            Opzionale; Flag di modo: se true la selezione viene aggiunta alla
                              selezione già esistente (default = false)
-*/  
/*****************************************************************************/
void CGEOListCtrl::SetSelectedCodes(C_INT_LIST &SelectedCodes, bool AppendMode)
{
   C_INT         *pSelectedCode = (C_INT *) SelectedCodes.get_head();
   CGEOList_Item *pItem;

   if (!AppendMode)
   {
      // deseleziono tutto
      pItem = (CGEOList_Item *) ItemList.get_head();
      while (pItem)
      {  
         OnItemChanged(pItem,  false);
         pItem = (CGEOList_Item *) pItem->get_next();
      }
   }

   while (pSelectedCode)
   {
      if ((pItem = (CGEOList_Item*) ItemList.search_key(pSelectedCode->get_key())))
         OnItemChanged(pItem, true);

      pSelectedCode = (C_INT *) SelectedCodes.get_next();
   }

   RefreshSelection();
}
void CGEOListCtrl::GetSelectedCodes(C_INT_LIST &SelectedCodes)
{
   CGEOList_Item *pItem = (CGEOList_Item *) ItemList.get_head();

   SelectedCodes.remove_all();
   while (pItem)
   {
      if (pItem->selected) SelectedCodes.add_tail_int(pItem->get_key());

      pItem = (CGEOList_Item *) pItem->get_next();
   }
}


/*********************************************************/
/*.doc CGEOListCtrl::Filter                   <external> */
/*+
  Questa funzione filtra un elemento in base alle regole attive.
  Parametri:
  CGEOList_Item *pItem;

  Restituisce true in caso di successo altrimenti false.
-*/  
/*********************************************************/
bool CGEOListCtrl::Filter(CGEOList_Item *pItem)
{
   // Filtro per codice
   if (UseFilterOnCodes)
      if (FilterOnCodes.search_key(pItem->get_key()) == NULL)
         return false;
   // Filtro per nome
   if (FilterOnName.len() > 0)
   {
      C_STRING dummy(pItem->get_name());

      if (dummy.wildcomp(FilterOnName) == 0)
         return false;
   }
   // Filtro per estratta nella sessione corrente
   if (FilterOnExtractedInCurrWrkSession)
      if (pItem->extracted == false)
         return false;
   // Filtro per modificabilità
   if (FilterOnUpdateable)
      if (pItem->updatable != GSUpdateableData)
         return false;
   // Filtro per categoria e tipo (per classi o sottoclassi)
   if ((ObjectType == GSClass || ObjectType == GSSubClass) && 
       FilterOnClassCategoryTypeList.get_count() > 0)
      if (FilterOnClassCategoryTypeList.search(pItem->get_category(), pItem->get_type()) == NULL)
         return false;
   // Filtro per tipo secondaria
   if (ObjectType == GSSecondaryTab && FilterOnSecondaryTabType.get_count() > 0)
      if (FilterOnSecondaryTabType.search_key(pItem->get_type()) == NULL)
         return false;

   return true;
}


/*********************************************************/
/*.doc CGEOListCtrl::Refresh_Prjs             <external> */
/*+
  Questa funzione visualizza i progetti applicando
  le impostazioni di visualizzazione.
  Parametri:
  bool OnlyRefresh;  Se true aggiorna solo la lista altrimenti 
                     la ricostruisce da zero (default = false).

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOListCtrl::Refresh_Prjs(bool OnlyRefresh)
{
   LVFINDINFO    FindInfo;
	int           nItem;
	LV_ITEM       lvitem;
   CGEOList_Item *pItem;

   if (!OnlyRefresh)
   {
      // svuoto la lista
      Reset_List();
      InsertColumn(0, _T("Nome"), LVCFMT_LEFT, 0, 0);
   }

   lvitem.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
   lvitem.iSubItem = 0;

   FindInfo.flags = LVFI_PARAM;

   pItem = (CGEOList_Item *) ItemList.get_head();
   while (pItem)
   {
      FindInfo.lParam = (LPARAM) pItem;
      nItem = FindItem(&FindInfo);  // verifico se è già in lista

      // Filtro progetto
      if (Filter(pItem) == NULL)
      {
         if (nItem != -1) DeleteItem(nItem); // era già in lista quindi lo cancello         
         pItem = (CGEOList_Item *) pItem->get_next();
         continue;
      }

      if (pItem->selected)
	   {
		   lvitem.state = LVIS_SELECTED;      
		   lvitem.stateMask = LVIS_SELECTED;  
	   }
	   else
	   {
		   lvitem.state = 0;      
		   lvitem.stateMask = 0;  
	   }
       
      // nome progetto
      lvitem.lParam  = (LPARAM) pItem;     
      lvitem.pszText = pItem->get_name();
      lvitem.iImage  = pItem->image;

      if (nItem != -1) // era già in lista quindi lo aggiorno
      {
   		lvitem.iItem = nItem;
         SetItem(&lvitem);
      }
      else // non era già in lista quindi lo aggiungo
      {
   		lvitem.iItem = GetItemCount();
         InsertItem(&lvitem);
      }

      pItem = (CGEOList_Item *) pItem->get_next();
   }

   CalcColumnSize_Prjs();
   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOListCtrl::Refresh_Classes          <external> */
/*+
  Questa funzione visualizza le classi applicando
  le impostazioni di visualizzazione.
  Parametri:
  bool OnlyRefresh;  Se true aggiorna solo la lista altrimenti 
                     la ricostruisce da zero (default = false).

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOListCtrl::Refresh_Classes(bool OnlyRefresh)
{
   LVFINDINFO    FindInfo;
	int           nItem;
	LV_ITEM       lvitem;
   CGEOList_Item *pItem;

   if (!OnlyRefresh)
   {
      // svuoto la lista
      Reset_List();
      InsertColumn(0, _T("Tipo"), LVCFMT_LEFT, 0, 0);
      InsertColumn(1, _T("Nome"), LVCFMT_LEFT, 0, 0); // la ridimensiono alla fine
	   if (ColumnClassStatusVisibility)
         InsertColumn(2, _T("Stato"), LVCFMT_LEFT, 0, 0);
   }

   FindInfo.flags = LVFI_PARAM;

   pItem = (CGEOList_Item *) ItemList.get_head();
   while (pItem)
   {
      FindInfo.lParam = (LPARAM) pItem;
      nItem = FindItem(&FindInfo);  // verifico se è già in lista

      // Filtra classe
      if (Filter(pItem) == NULL)
      {
         if (nItem != -1) DeleteItem(nItem); // era già in lista quindi lo cancello         
         pItem = (CGEOList_Item *) pItem->get_next();
         continue;
      }

      if (pItem->selected)
	   {
		   lvitem.state     = LVIS_SELECTED;      
		   lvitem.stateMask = LVIS_SELECTED;  
	   }
	   else
	   {
		   lvitem.state     = 0;      
		   lvitem.stateMask = 0;  
	   }

      // tipo classe
      lvitem.iSubItem = 0;
      lvitem.iImage   = pItem->image;
      lvitem.lParam   = (LPARAM) pItem; 
      lvitem.mask     = LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
      lvitem.iSubItem = 0;

      if (nItem != -1) // era già in lista quindi lo aggiorno
      {
   		lvitem.iItem = nItem;
         SetItem(&lvitem);
      }
      else // non era già in lista quindi lo aggiungo
      {
   		lvitem.iItem = GetItemCount();
         InsertItem(&lvitem);
      }

      // nome classe
      lvitem.mask     = LVIF_TEXT;
      lvitem.iSubItem = 1;
      lvitem.pszText  = pItem->get_name();
      SetItem(&lvitem);

	   if (ColumnClassStatusVisibility)
      {  // stato della classe
         int i_over_list = pItem->get_state();

	      lvitem.mask     = LVIF_IMAGE;
	      lvitem.iSubItem = 2;
	      lvitem.iImage   = ItemList.get_count() - 1 + i_over_list;
         SetItem(&lvitem);
      }

      pItem = (CGEOList_Item *) pItem->get_next();
   }
      
   if (!OnlyRefresh)
   {
      CalcColumnSize_Classes();

      // ordino la lista per nome
      NM_LISTVIEW NMListView;
      LRESULT     Result;

      NMListView.iSubItem = 1;
      AscendingOrder = false; // OnColumnClick inverte AscendingOrder
      OnColumnClick((NMHDR *) (&NMListView), &Result);
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOListCtrl::Refresh_SubClasses          <external> */
/*+
  Questa funzione visualizza le sotto-classi applicando
  le impostazioni di visualizzazione.
  Parametri:
  bool OnlyRefresh;  Se true aggiorna solo la lista altrimenti 
                     la ricostruisce da zero (default = false).

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOListCtrl::Refresh_SubClasses(bool OnlyRefresh)
{
   return Refresh_Classes(OnlyRefresh);
}


/*********************************************************/
/*.doc CGEOListCtrl::Refresh_SecTabs          <external> */
/*+
  Questa funzione visualizza le tabelle secondarie applicando
  le impostazioni di visualizzazione.
  Parametri:
  bool OnlyRefresh;  Se true aggiorna solo la lista altrimenti 
                     la ricostruisce da zero (default = false).

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOListCtrl::Refresh_SecTabs(bool OnlyRefresh)
{
   return Refresh_Classes(OnlyRefresh);
}


///////////////////////////////////////////////////////////////////////////////
// FUNZIONI PER TOOLTIP - INIZIO
///////////////////////////////////////////////////////////////////////////////


BOOL CGEOListCtrl::InitTooltip(void)
{
   if (pToolTip) delete pToolTip;
   pToolTip = new CToolTipCtrl;
   if (!pToolTip->Create(this, TTS_ALWAYSTIP)) return FALSE;
	if (m_hWnd) pToolTip->AddTool(this, _T(""));

   pToolTip->SetDelayTime(TTDT_INITIAL, 0);
   pToolTip->Activate(TRUE);
   
   return TRUE;
}

// Le coordinate espresse da pt sono "Screen"
BOOL CGEOListCtrl::DisplayTooltip(MSG* pMsg)
{
   CRect Rect, ScreenRect;
   int   iItem, StartItem, EndItem, iSubItem;

   if (!pToolTip || !pToolTip->m_hWnd) return FALSE;

   StartItem = ListView_GetTopIndex(pMsg->hwnd);
   EndItem   = StartItem + ListView_GetCountPerPage(pMsg->hwnd);

   for (iItem = StartItem; iItem < EndItem; iItem++)
   {
      if (ColumnClassStatusVisibility)
         if (ListView_GetSubItemRect(pMsg->hwnd, iItem, 2, LVIR_BOUNDS, &Rect) == TRUE) // colonna dello stato
         {
            ScreenRect = Rect;
            ClientToScreen(ScreenRect);
            if (ScreenRect.PtInRect(pMsg->pt))
               { iSubItem = 2; break; }
         }
      if (ListView_GetSubItemRect(pMsg->hwnd, iItem, 1, LVIR_BOUNDS, &Rect) == TRUE) // colonna del nome
      {
         ScreenRect = Rect;
         ClientToScreen(ScreenRect);
         if (ScreenRect.PtInRect(pMsg->pt))
            { iSubItem = 1; break; }
      }
      // messa per ultimo perchè 0 valuta tutta la riga non solo la colonna 0
      if (ListView_GetSubItemRect(pMsg->hwnd, iItem, 0, LVIR_BOUNDS, &Rect) == TRUE) // colonna del tipo
      {
         ScreenRect = Rect;
         ClientToScreen(ScreenRect);
         if (ScreenRect.PtInRect(pMsg->pt))
            { iSubItem = 0; break; }
      }
   }

   // Se non era su una riga del controllo
   if (iItem == EndItem)
      return TRUE;

   CGEOList_Item *pItem;

   if ((pItem = (CGEOList_Item *) GetItemData(iItem)) != NULL)
   {
      CString ItemText, PrevItemText;

      switch (iSubItem)
      {
         case 0: // colonna del tipo
         {
            switch (ObjectType)
            {
               case GSClass:
               case GSSubClass:
               {
                  C_STRING Buffer;

                  if (gsc_getClsCategoryTypeDescr(pItem->category, pItem->type, Buffer) == GS_GOOD)
                     ItemText = Buffer.get_name();
                  break;
               }
               case GSSecondaryTab:
                  if (pItem->type == (int) GSExternalSecondaryTable)
                     ItemText = _T("Tabella secondaria di GEOsim");
                  else
                     ItemText = _T("Tabella secondaria esterna");
                  break;
            }

            break;
         }

         case 1: // colonna del nome
            if (pItem->Descr.len() > 0)
               ItemText = pItem->Descr.get_name();

            break;

         case 2: // colonna dello stato
            switch (ObjectType)
            {
               case GSClass:
               case GSSubClass:
                  ItemText = _T("Classe ");

           	      if (pItem->extracted)
                     ItemText += _T("estratta");
                  else
                     ItemText += _T("non estratta");
            
                  ItemText += _T(" - ");
                  if (pItem->updatable != GSUpdateableData)
                     ItemText += _T("non modificabile");
                  else
                     ItemText += _T("modificabile");

                  break;
            }

            break;
      }
    
      if (gsc_strlen(ItemText) == 0)
         ItemText = _T("");

      pToolTip->GetText(PrevItemText, this);
      if (PrevItemText.Compare(ItemText) != 0)
         pToolTip->UpdateTipText(ItemText, this);
      pToolTip->RelayEvent(pMsg);
   }

   return TRUE;
}

BOOL CGEOListCtrl::PreTranslateMessage(MSG* pMsg)
{
   DisplayTooltip(pMsg);
   return CListCtrl::PreTranslateMessage(pMsg);
}

///////////////////////////////////////////////////////////////////////////////
// FUNZIONI PER TOOLTIP - FINE
///////////////////////////////////////////////////////////////////////////////


BEGIN_MESSAGE_MAP(CGEOListCtrl, CListCtrl)
	ON_WM_CHAR()
   ON_WM_KEYDOWN()
   ON_WM_SIZE()
   ON_WM_LBUTTONDOWN()
   ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnColumnClick)
   ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, OnItemChanged)
   ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_FILTER_BY_NAME_MENU, OnFilterByNameMenu)
	ON_COMMAND(ID_FILTER_BY_TYPE_MENU, OnFilterByTypeMenu)
	ON_COMMAND(ID_LOAD_SEL_MENU, OnLoadSelMenu)
	ON_COMMAND(ID_SAVE_SEL_MENU, OnSaveSelMenu)
	ON_COMMAND(ID_REFRESH_MENU, OnRefreshMenu)
	ON_COMMAND(ID_SELECT_ALL_MENU, OnSelectAllMenu)
	ON_COMMAND(ID_DESELECT_ALL_MENU, OnDeselectAllMenu)
	ON_COMMAND(ID_INVERT_SEL_MENU, OnInvertSelMenu)
END_MESSAGE_MAP()


// gestori di messaggi CGEOListCtrl
void CGEOListCtrl::sendITEMCHANGEDEventToParent(void)
{
   if (GetParent() == NULL) return;

   NMLISTVIEW nmlv;
   memset(&nmlv, 0, sizeof (nmlv));
   nmlv.hdr.hwndFrom = m_hWnd;
   nmlv.hdr.idFrom   = GetDlgCtrlID();
   nmlv.hdr.code     = LVN_ITEMCHANGED;

   GetParent()->SendMessage(WM_NOTIFY, 0, (LPARAM)&nmlv); 
}

void CGEOListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
   CGEOList_Item *pItem;

   CListCtrl::OnLButtonDown(nFlags, point);

   for (int iItem = 0; iItem < GetItemCount(); iItem++)
      if ((pItem = (CGEOList_Item *) GetItemData(iItem)) != NULL)
         OnItemChanged(pItem, (GetItemState(iItem, LVIS_SELECTED) & LVIS_SELECTED) ? true : false);

   sendITEMCHANGEDEventToParent();
}


void CGEOListCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
   int     i, nItem, nSubTxtItem;
   CString value, CmpValue((TCHAR)nChar);

   SetFocus();
   for (i = 0; i < GetItemCount(); i++)
      if (GetItemState(i, LVIS_FOCUSED) == LVIS_FOCUSED) break;

   if (i >= GetItemCount()) i = 0; // nessun elemento aveva fuoco
   else i = i + 1;

   switch (ObjectType)
   {
      case GSProject:
         nSubTxtItem = 0;
         break;
      case GSClass:
      case GSSubClass:
      case GSSecondaryTab:
         nSubTxtItem = 1;
         break;
      default:
         return;
   }

   CmpValue.MakeLower();
   for (nItem = i; nItem < GetItemCount(); nItem++)
   {
      value = GetItemText(nItem, nSubTxtItem); // Valore del testo
      value.MakeLower();
      if (value.Find(CmpValue) == 0) break;
   }

   if (nItem < GetItemCount()) // elemento trovato
   {
      for (i = 0; i < GetItemCount(); i++)
      {
         SetItemState(i, 0, LVIS_SELECTED);
         OnItemChanged((CGEOList_Item *) GetItemData(i), false);
      }

      SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
      EnsureVisible(nItem, FALSE);
      OnItemChanged((CGEOList_Item *) GetItemData(nItem), true);
      sendITEMCHANGEDEventToParent();
   }
	CListCtrl::OnChar(nChar, nRepCnt, nFlags);
}


void CGEOListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   CGEOList_Item *pItem;

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

   for (int iItem = 0; iItem < GetItemCount(); iItem++)
      if ((pItem = (CGEOList_Item *) GetItemData(iItem)) != NULL)
         OnItemChanged(pItem, (GetItemState(iItem, LVIS_SELECTED) & LVIS_SELECTED) ? true : false);

   sendITEMCHANGEDEventToParent();
}


/*********************************************************/
// Funzione che fa la comparazione tra 2 valori
// per l'ordinamento delle righe della listctrl
/*********************************************************/
static int CALLBACK DataCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   CGEOListCtrl  *pListCtrl = (CGEOListCtrl *) lParamSort;
   CGEOList_Item *pItem1 = (CGEOList_Item *) lParam1, *pItem2 = (CGEOList_Item *) lParam2;
   C_STRING       strItem1, strItem2;

   switch (pListCtrl->ObjectType)
   {
      case GSProject:
         strItem1 = pItem1->get_name();
         strItem2 = pItem2->get_name();
         break;
      case GSClass:
      case GSSubClass:
      case GSSecondaryTab:
         switch (pListCtrl->Prev_HeaderPos)
         {
            case 0: // tipo
               strItem1 = pItem1->get_type();
               strItem2 = pItem2->get_type();
               break;
            case 1: // nome
               strItem1 = pItem1->get_name();
               strItem2 = pItem2->get_name();
               break;
            case 2: // stato
               strItem1 = pItem1->get_state();
               strItem2 = pItem2->get_state();
               break;
            default:
               return 0;
         }
         break;
      default:
         return 0;
   }

   if (pListCtrl->AscendingOrder) return strItem1.comp(strItem2, FALSE); // ordine crescente
   else return strItem2.comp(strItem1, FALSE); // ordine rescente
}

BOOL CGEOListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
   NM_LISTVIEW* pNMListView = (NM_LISTVIEW*) pNMHDR;
   int          iColumn = pNMListView->iSubItem;
   HD_ITEM      curItem;
	CHeaderCtrl  *pHdrCtrl = GetHeaderCtrl();
   TCHAR        Buffer[100] = _T("");

   // alloco buffer
   curItem.pszText    = Buffer;
   curItem.cchTextMax = 100 - 1; 

   // se non era già stata selezionata questa colonna
   if (Prev_HeaderPos != iColumn)
   {
      AscendingOrder = true;
      if (Prev_HeaderPos != -1)
      {
         curItem.mask = HDI_TEXT;
	      pHdrCtrl->GetItem(Prev_HeaderPos, &curItem);
  	      curItem.mask = HDI_FORMAT; //  Testo
       	curItem.fmt  = HDF_LEFT | HDF_STRING;
	      pHdrCtrl->SetItem(Prev_HeaderPos, &curItem);
      }
      AscendingOrder = true;
      Prev_HeaderPos = iColumn;
   }
   else
      AscendingOrder = !AscendingOrder;

   // add bmaps to header item
   curItem.mask = HDI_TEXT;
	pHdrCtrl->GetItem(iColumn, &curItem);
	curItem.mask = HDI_IMAGE | HDI_FORMAT; //  Testo e immagine
   curItem.iImage = (AscendingOrder) ? 0 : 1; // crescente oppure decrescente
	curItem.fmt = HDF_STRING | HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
	pHdrCtrl->SetItem(iColumn, &curItem);

   SortItems(DataCompare, (LPARAM) this);

   *pResult = 0;

   // If you use ON_NOTIFY_REFLECT_EX() in your message map, your message handler may or may not allow
   // the parent window to handle the message. If the handler returns FALSE, the message will be handled
   // by the parent as well
   return FALSE;
}

BOOL CGEOListCtrl::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
   //// If you use ON_NOTIFY_REFLECT_EX() in your message map, your message handler may or may not allow
   //// the parent window to handle the message. If the handler returns FALSE, the message will be handled
   //// by the parent as well

   // Non usare questo evento perchè fa casino se ad esempio si seleziona una classe gruppo e 
   // si devono selezionare anche le classi membro. In questo caso inspiegabilmente partono eventi
   // che deselezionano tutte le classi che sono state selezionate via codice da questa routine

   return FALSE;
}


/*********************************************************/
/*.doc CGEOListCtrl::OnItemChanged          <external> */
/*+
  Questa funzione setta lo stato di selezione di un elemento.
  
  Ritorna true se è stata selezionata e deselezionata qualche altra classe
  raggruppata a quella data come parametro della funzione
-*/  
/*********************************************************/
void CGEOListCtrl::OnItemChanged(CGEOList_Item *pItem, bool selected)
{
   pItem->selected = selected;

   // Se è abilitata la selezione multipla di classi
   // e si vuole che le classi membro ereditino la selezione della classe gruppo 
   if (MultiSelect && ObjectType == GSClass && selected &&
       SelectedLinkedClass && pItem->get_category() == CAT_GROUP)
   {
		C_CLASS *pCls = gsc_find_class(FilterOnPrj, pItem->get_key());
                        
		if (pCls)
		{
         C_GROUP_LIST *pGroupList = (C_GROUP_LIST *) pCls->ptr_group_list();
			
         if (pGroupList)	// verifico che le classi che compongono il gruppo siano in lista
         {
            C_INT_INT *pGroup = (C_INT_INT*) pGroupList->get_head();
            CGEOList_Item *pPrevCsr = (CGEOList_Item *) ItemList.get_cursor(); // Posizione attuale del cursore
						
            while (pGroup)
				{
					if ((pItem = (CGEOList_Item*) ItemList.search_key(pGroup->get_key())))
               {
                  LVFINDINFO FindInfo;
                  int        iItem;

                  FindInfo.flags = LVFI_PARAM;
                  FindInfo.lParam = (LPARAM) pItem;

                  OnItemChanged(pItem, selected);
                  if ((iItem = FindItem(&FindInfo)) != -1) 
                     SetItemState(iItem, selected ? LVIS_SELECTED : 0, LVIS_SELECTED);
               }
					pGroup = (C_INT_INT*) pGroup->get_next();
				}

            ItemList.set_cursor(pPrevCsr); // Riposiziono il cursore
         }
      }
   }
}

void CGEOListCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CListCtrl::OnSize(nType, cx, cy);
   if (ColumnAutosize && !insideCalcColumnSize) CalcColumnSize();
}

void CGEOListCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CMenu Menu;
   UINT  nFlags;

   // Create a new menu for the application window.
   if (Menu.CreateMenu() == FALSE) return;

   // Create a "File" popup menu and insert this popup menu to the menu
   if (Menu.CreatePopupMenu() == FALSE) return;

 	if (MultiSelect)
   {
      if (ObjectType == GSClass || ObjectType == GSSubClass)
      {
         if (Menu.AppendMenu(MF_STRING, ID_LOAD_SEL_MENU, _T("&Carica selezione...")) == FALSE) return;
         if (Menu.AppendMenu(MF_STRING, ID_SAVE_SEL_MENU, _T("&Salva selezione...")) == FALSE) return;     
         if (Menu.AppendMenu(MF_SEPARATOR) == FALSE) return; // Separatore
      }
      if (Menu.AppendMenu(MF_STRING, ID_SELECT_ALL_MENU, _T("Seleziona &Tutto")) == FALSE) return;     
      if (Menu.AppendMenu(MF_STRING, ID_DESELECT_ALL_MENU, _T("&Deseleziona tutto")) == FALSE) return;     
      if (Menu.AppendMenu(MF_STRING, ID_INVERT_SEL_MENU, _T("&Inverti selezione")) == FALSE) return;     
      if (Menu.AppendMenu(MF_SEPARATOR) == FALSE) return; // Separatore
   }

   // se filtro per nome impostato
   nFlags = MF_STRING | (FilterOnName.len() > 0) ? MF_CHECKED : MF_UNCHECKED;
   if (Menu.AppendMenu(nFlags, ID_FILTER_BY_NAME_MENU, _T("Filtra per &Nome...")) == FALSE) return;

   if (ObjectType != GSProject)
   {
      // se filtro per categoria e tipo impostato
      nFlags = MF_STRING | (FilterOnClassCategoryTypeList.get_count() > 0) ? MF_CHECKED : MF_UNCHECKED;
      if (Menu.AppendMenu(nFlags, ID_FILTER_BY_TYPE_MENU, _T("Filtra per &Tipo...")) == FALSE) return;
   }

   if (Menu.AppendMenu(MF_SEPARATOR) == FALSE) return; // Separatore
   if (Menu.AppendMenu(MF_STRING, ID_REFRESH_MENU, _T("&Aggiorna")) == FALSE) return;

	// Draw and track the "floating" menu. 
 	TrackPopupMenu(Menu.m_hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON,	
                  point.x, point.y, 0, pWnd->m_hWnd, NULL);
}

void CGEOListCtrl::OnFilterByNameMenu(void) 
{
   if (gsc_ddinput(_T("Filtra per nome:"), FilterOnName, FilterOnName.get_name(), FALSE, FALSE) == GS_GOOD &&
       FilterOnName.len() > 0)
   {
      Refresh();
      RefreshSelection();
   }
}

void CGEOListCtrl::OnFilterByTypeMenu(void) 
{
   GEOListCtrlFilterDlg Dlg(ObjectType, this);

   switch (ObjectType)
   {
      case GSClass:
      case GSSubClass:
         if (FilterOnClassCategoryTypeList.get_count() == 1)
         {
            C_INT_INT *p = (C_INT_INT *) FilterOnClassCategoryTypeList.get_head();
            Dlg.ClassCategoryTypeToType(p->get_key(), p->get_type());
         }
         break;
      case GSSecondaryTab:
         if (FilterOnSecondaryTabType.get_count() == 1)
         {
            C_INT *p = (C_INT_INT *) FilterOnSecondaryTabType.get_head();
            Dlg.SecTypeToType(p->get_key());
         }
   }

   if (Dlg.DoModal() == IDOK)
   {
      switch (ObjectType)
      {
         case GSClass:
         case GSSubClass:
         {
            int ClsCat, ClsType;
            if (Dlg.TypeToClassCategoryType(&ClsCat, &ClsType))
            {
               FilterOnClassCategoryTypeList.remove_all();
               if (ClsCat != -1) FilterOnClassCategoryTypeList.values_add_tail(ClsCat, ClsType);
            }
            break;
         }
         case GSSecondaryTab:
         {
            int SecType;

            if (Dlg.TypeToSecType(&SecType))
            {
               FilterOnSecondaryTabType.remove_all();
               if (SecType != -1) FilterOnSecondaryTabType.add_tail_int((int) SecType);
            }
            break;
         }
      }

      Refresh();
      RefreshSelection();
   }
}

void CGEOListCtrl::OnLoadSelMenu(void) 
{
   if (ObjectType == GSClass || ObjectType == GSSubClass)
   {
      C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(FilterOnPrj);

	   if (pPrj)
	   {
         CString dir;
		      
         dir = pPrj->get_dir();
		   dir += _T("\\");
		   dir += GEOSELCLSDIR;

         CFileDialog FileDlg(TRUE, _T("sel"), NULL, OFN_FILEMUSTEXIST,
                             _T("File di selezione (*.sel)|*.sel|Tutti i File (*.*)|*.*||"), this);
         FileDlg.m_ofn.lpstrInitialDir = dir; // set an initial directory
         if (FileDlg.DoModal() == IDOK)
         {
         	C_STR_LIST list;

            if (list.load((LPCTSTR) FileDlg.GetPathName()) == GS_GOOD && list.get_count() > 0)
            {
               C_STR         *p = (C_STR *) list.get_head();
               CGEOList_Item *pItem = (CGEOList_Item *) ItemList.get_head();

               // Il primo elemento è il progetto che deve essere uguale a quello impostato
               if (_wtoi(p->get_name()) != FilterOnPrj) return;

               // deseleziono tutto
               while (pItem)
               {  
                  pItem->selected = false;
                  pItem = (CGEOList_Item *) pItem->get_next();
               }

               p = (C_STR *) p->get_next();
               while (p)
               {
                  if ((pItem = (CGEOList_Item *) ItemList.search_key(_wtoi(p->get_name()))))
                     OnItemChanged(pItem, true);

                  p = (C_STR *) p->get_next();
               }

               RefreshSelection();
            }
         }
      }
   }
}

void CGEOListCtrl::OnSaveSelMenu(void) 
{
   if (ObjectType == GSClass || ObjectType == GSSubClass)
   {
      C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(FilterOnPrj);

	   if (pPrj)
	   {
         CString dir;
		      
         dir = pPrj->get_dir();
		   dir += _T("\\");
		   dir += GEOSELCLSDIR;

         CFileDialog FileDlg(FALSE, _T("sel"), NULL, OFN_OVERWRITEPROMPT,
                             _T("File di selezione (*.sel)|*.sel|Tutti i File (*.*)|*.*||"), this);
         FileDlg.m_ofn.lpstrInitialDir = dir; // set an initial directory
         if (FileDlg.DoModal() == IDOK)
         {
            CGEOList_Item *pItem = (CGEOList_Item *) ItemList.get_head();
            C_STR_LIST    list;
            C_STRING      dummy;

            dummy = FilterOnPrj;
            list.add_tail_str(dummy.get_name());

            while (pItem)
            {
               if (pItem->selected)
               {
                  dummy = pItem->get_key();
                  list.add_tail_str(dummy.get_name());
               }

               pItem = (CGEOList_Item *) pItem->get_next();
            }

            list.save((LPCTSTR) FileDlg.GetPathName());
         }
      }
   }
}

void CGEOListCtrl::OnRefreshMenu(void) 
{
   C_INT_LIST SelectedCodes;

   GetSelectedCodes(SelectedCodes);
   LoadFromDB();
   Refresh();
   SetSelectedCodes(SelectedCodes, true);

   // ordine la lista come prima
   if (Prev_HeaderPos != -1)
   {
      NM_LISTVIEW NMListView;
      LRESULT     Result;

      NMListView.iSubItem = Prev_HeaderPos;
      OnColumnClick((NMHDR *) (&NMListView), &Result);
   }
}

void CGEOListCtrl::OnSelectAllMenu(void) 
{
   CGEOList_Item *pItem = (CGEOList_Item *) ItemList.get_head();
   while (pItem)
   {  // Filtra elemento
      if (Filter(pItem)) OnItemChanged(pItem, true);

      pItem = (CGEOList_Item *) pItem->get_next();
   }
   RefreshSelection();
}

void CGEOListCtrl::OnDeselectAllMenu(void) 
{
   CGEOList_Item *pItem = (CGEOList_Item *) ItemList.get_head();
   while (pItem)
   {  // Filtra elemento
      if (Filter(pItem)) OnItemChanged(pItem, false);

      pItem = (CGEOList_Item *) pItem->get_next();
   }
   RefreshSelection();
}

void CGEOListCtrl::OnInvertSelMenu(void) 
{
   CGEOList_Item *pItem = (CGEOList_Item *) ItemList.get_head();
   C_INT_LIST    CodeList;

   while (pItem)
   {  // Filtra elemento
      if (Filter(pItem))
      {
         // Se è abilitata la selezione multipla di classi
         // e si vuole che le classi membro ereditino la selezione della classe gruppo 
         if (MultiSelect && ObjectType == GSClass && 
             SelectedLinkedClass && pItem->get_category() == CAT_GROUP)          
            CodeList.add_tail_int(pItem->get_key()); // Le classi gruppo le tratto alla fine
         else
            OnItemChanged(pItem, !pItem->selected);
      }

      pItem = (CGEOList_Item *) pItem->get_next();
   }

   // Seleziono le classi gruppo e le classi membro
   C_INT *pCode = (C_INT *) CodeList.get_head();
   while (pCode)
   {
      pItem = (CGEOList_Item *) ItemList.search_key(pCode->get_key());
      OnItemChanged(pItem, !pItem->selected);

      pCode = (C_INT *) CodeList.get_next();
   }

   RefreshSelection();
}

void CGEOListCtrl::RefreshSelection(bool EnsureVisibleFlag) 
{
   LVFINDINFO FindInfo;
   int        nIndex;

   OnRefresh = true;

   FindInfo.flags = LVFI_PARAM;
   CGEOList_Item *pItem = (CGEOList_Item *) ItemList.get_head();
   while (pItem)
   {
      FindInfo.lParam = (LPARAM) pItem;
      if ((nIndex = FindItem(&FindInfo)) != -1)
         SetItemState(nIndex, (pItem->selected) ? LVIS_SELECTED : 0, LVIS_SELECTED);

      pItem = (CGEOList_Item *) pItem->get_next();
   }

   if (EnsureVisibleFlag)
   {
      // Mi assicuro che il primo selezionato sia visibile
      POSITION pos = GetFirstSelectedItemPosition();
      if (pos)
         EnsureVisible(GetNextSelectedItem(pos), FALSE);
   }

   OnRefresh = false;
}


///////////////////////////////////////////
// finestra di dialogo GEOListCtrlFilterDlg

IMPLEMENT_DYNAMIC(GEOListCtrlFilterDlg, CDialogEx)

GEOListCtrlFilterDlg::GEOListCtrlFilterDlg(TCHAR *_Name, CWnd* pParent /*=NULL*/)
	: CDialogEx(GEOListCtrlFilterDlg::IDD, pParent)
{
   FilterType = ByName;
   Name       = _Name;
}
GEOListCtrlFilterDlg::GEOListCtrlFilterDlg(GSDataModelObjectTypeEnum _ObjectType, CWnd* pParent /*=NULL*/)
	: CDialogEx(GEOListCtrlFilterDlg::IDD, pParent)
{
   FilterType = ByType;
   ObjectType = _ObjectType;
}

GEOListCtrlFilterDlg::~GEOListCtrlFilterDlg()
{
}

int GEOListCtrlFilterDlg::ClassCategoryTypeToType(int _ClassCategory, int _ClassType)
{
   switch (ObjectType)
   {
      case GSClass:
         // 1 = griglia,   2 = gruppo,      3 = nodi
         // 4 = polilinee, 5 = simulazioni, 6 = spaghetti
         // 7 = superfici, 8 = testi
         switch (_ClassCategory)
         {
            case CAT_GRID:
               return Type = 1;
            case CAT_GROUP:
               return Type = 2;
            case CAT_SIMPLEX:
            case CAT_SUBCLASS:
               switch (_ClassType)
               {
                  case TYPE_POLYLINE:
                     return Type = 4;
                  case TYPE_TEXT:
                     return Type = 8;
                  case TYPE_NODE:
                     return Type = 3;
                  case TYPE_SURFACE:
                     return Type = 7;
               }
               break;
            case CAT_EXTERN:
               return Type = 5;
            case CAT_SPAGHETTI:
               return Type = 6;
         }
   }
   return Type = 0;
}

bool GEOListCtrlFilterDlg::TypeToClassCategoryType(int *_ClassCategory, int *_ClassType)
{
   switch (ObjectType)
   {
      case GSClass:
         // 0 = tutto,     1 = griglia,     2 = gruppo,      3 = nodi
         // 4 = polilinee, 5 = simulazioni, 6 = spaghetti
         // 7 = superfici, 8 = testi
         switch (Type)
         {
            case 1:
               *_ClassCategory = CAT_GRID;
               *_ClassType     = TYPE_GRID;
               return true;
            case 2:
               *_ClassCategory = CAT_GROUP;
               *_ClassType     = TYPE_GROUP;
               return true;
            case 3:
               *_ClassCategory = CAT_SIMPLEX;
               *_ClassType     = TYPE_NODE;
               return true;
            case 4:
               *_ClassCategory = CAT_SIMPLEX;
               *_ClassType     = TYPE_POLYLINE;
               return true;
            case 5:
               *_ClassCategory = CAT_EXTERN;
               *_ClassType     = 0;
               return true;
            case 6:
               *_ClassCategory = CAT_SPAGHETTI;
               *_ClassType     = TYPE_SPAGHETTI;
               return true;
            case 7:
               *_ClassCategory = CAT_SIMPLEX;
               *_ClassType     = TYPE_SURFACE;
               return true;
            case 8:
               *_ClassCategory = CAT_SIMPLEX;
               *_ClassType     = TYPE_TEXT;
               return true;
         }
         break;
      case GSSubClass:
         // 0 = tutto, 1 = nodi, 2 = polilinee, 3 = superfici, 4 = testi
         switch (Type)
         {
            case 1:
               *_ClassCategory = CAT_SUBCLASS;
               *_ClassType     = TYPE_NODE;
               return true;
            case 2:
               *_ClassCategory = CAT_SUBCLASS;
               *_ClassType     = TYPE_POLYLINE;
               return true;
            case 3:
               *_ClassCategory = CAT_SUBCLASS;
               *_ClassType     = TYPE_SURFACE;
               return true;
            case 4:
               *_ClassCategory = CAT_SUBCLASS;
               *_ClassType     = TYPE_TEXT;
               return true;
         }
         break;
   }

   *_ClassCategory = -1;
   *_ClassType     = -1;

   return true;
}

bool GEOListCtrlFilterDlg::TypeToSecType(int *_SecType)
{
   // 0 = tutto, 1 = secondarie di GEOsim, 2= secondarie esterne
   switch (Type)
   {
      case 1:
         *_SecType = (int) GSInternalSecondaryTable;
         return true;
      case 2:
         *_SecType = (int) GSExternalSecondaryTable;
         return true;
      default:
         *_SecType = -1;
   }

   return false;
}

int GEOListCtrlFilterDlg::SecTypeToType(int _SecType)
{
   // 0 = tutto, 1 = secondarie di GEOsim, 2 = secondarie esterne
   switch (_SecType)
   {
      case GSInternalSecondaryTable:
         return Type = 1;
      case GSExternalSecondaryTable:
         return Type = 2;
   }

   return Type = 0;
}

void GEOListCtrlFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GEOLISTCTRL_DLG_COMBO, m_ComboCtrl);
   DDX_Control(pDX, IDC_GEOLISTCTRL_DLG_EDIT, m_Edit);
}


BEGIN_MESSAGE_MAP(GEOListCtrlFilterDlg, CDialogEx)
   ON_BN_CLICKED(IDOK, &GEOListCtrlFilterDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// gestori di messaggi GEOListCtrlFilterDlg
BOOL GEOListCtrlFilterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

   switch (FilterType)
   {
      case ByName:
         SetWindowText(_T("GEOsim - Filtro per nome"));
         m_ComboCtrl.ShowWindow(SW_HIDE);
         m_Edit.ShowWindow(SW_SHOW);
		   m_Edit.SetWindowText(Name);
		   m_Edit.SetFocus();
         break;
      case ByType:
         SetWindowText(_T("GEOsim - Filtro per tipo"));
		   InitializeImageList();
         m_Edit.ShowWindow(SW_HIDE);
         m_ComboCtrl.ShowWindow(SW_SHOW);
		   m_ComboCtrl.SetCurSel(Type);
		   m_ComboCtrl.SetFocus();
         break;
   }

   return TRUE; 
}

void GEOListCtrlFilterDlg::InitializeImageList()
{
   CBitmap        CBMP;
   COMBOBOXEXITEM pCBItem;
    
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   // creazione lista immagini
   if (ImageList.Create(BITMAP_WIDTH, BITMAP_HEIGHT,
                        ILC_MASK | ILC_COLORDDB, 
                        1, // n. bitmap
                        0) == NULL)
		return;

   // 0 - bitmap simbolo della griglia
   if (gsc_get_Bitmap(GSGroupMaskBmp_16X16, CBMP) == GS_BAD) return;
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente  
   // 1 - bitmap simbolo del gruppo
   if (gsc_get_Bitmap(GSGridMaskBmp_16X16, CBMP) == GS_BAD) return;
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente  
   // 2 - bitmap simbolo dei nodi
   if (gsc_get_Bitmap(GSNodeMaskBmp_16X16, CBMP) == GS_BAD) return;
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente    
   // 3 - bitmap simbolo delle polilinee
   if (gsc_get_Bitmap(GSPolylineMaskBmp_16X16, CBMP) == GS_BAD) return;
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente    
   // 4 - bitmap simbolo delle simulazioni
   if (gsc_get_Bitmap(GSSimulationMaskBmp_16X16, CBMP) == GS_BAD) return;
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente    
   // 5 - bitmap simbolo degli spaghetti
   if (gsc_get_Bitmap(GSSpaghettiMaskBmp_16X16, CBMP) == GS_BAD) return;
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente    
   // 6 - bitmap simbolo delle superfici
   if (gsc_get_Bitmap(GSSurfaceMaskBmp_16X16, CBMP) == GS_BAD) return;
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente    
   // 7 - bitmap simbolo dei testi
   if (gsc_get_Bitmap(GSTextMaskBmp_16X16, CBMP) == GS_BAD) return;
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente    

   m_ComboCtrl.SetImageList(&ImageList);

   // riga vuota
   pCBItem.iItem   = 0;
	pCBItem.mask    = CBEIF_TEXT;
	pCBItem.pszText = _T("");
	m_ComboCtrl.InsertItem(&pCBItem);

   pCBItem.mask   = CBEIF_IMAGE|CBEIF_SELECTEDIMAGE|CBEIF_TEXT;

	pCBItem.iItem = 1;
   pCBItem.pszText = _T("Griglia");
   pCBItem.iImage = pCBItem.iSelectedImage = 0;
   m_ComboCtrl.InsertItem(&pCBItem);

	pCBItem.iItem = 2;
   pCBItem.pszText = _T("Gruppo");
   pCBItem.iImage = pCBItem.iSelectedImage = 1;
   m_ComboCtrl.InsertItem(&pCBItem);

	pCBItem.iItem = 3;
   pCBItem.pszText = _T("Nodo");
   pCBItem.iImage = pCBItem.iSelectedImage = 2;
   m_ComboCtrl.InsertItem(&pCBItem);

	pCBItem.iItem = 4;
   pCBItem.pszText = _T("Polilinea");
   pCBItem.iImage = pCBItem.iSelectedImage = 3;
   m_ComboCtrl.InsertItem(&pCBItem);
   
	pCBItem.iItem = 5;
   pCBItem.pszText = _T("Simulazione");
   pCBItem.iImage = pCBItem.iSelectedImage = 4;
   m_ComboCtrl.InsertItem(&pCBItem);

	pCBItem.iItem = 6;
   pCBItem.pszText = _T("Spaghetti");
   pCBItem.iImage = pCBItem.iSelectedImage = 5;
   m_ComboCtrl.InsertItem(&pCBItem);

	pCBItem.iItem = 7;
   pCBItem.pszText = _T("Superficie");
   pCBItem.iImage = pCBItem.iSelectedImage = 6;
   m_ComboCtrl.InsertItem(&pCBItem);

	pCBItem.iItem = 8;
   pCBItem.pszText = _T("Testo");
   pCBItem.iImage = pCBItem.iSelectedImage = 7;
   m_ComboCtrl.InsertItem(&pCBItem);
}

void GEOListCtrlFilterDlg::OnBnClickedOk()
{
   switch (FilterType)
   {
      case ByName:
		   m_Edit.GetWindowText(Name);
         break;
      case ByType:
         Type = m_ComboCtrl.GetCurSel();
         break;
   }

   CDialogEx::OnOK();
}
