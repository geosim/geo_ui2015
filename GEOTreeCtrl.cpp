// GEOTreeCtrl.cpp : file di implementazione
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "GEOTreeCtrl.h"

#include "gs_class.h" 
#include "gs_sec.h"

#include "resource.h"
#include "gs_ui_utily.h"

#define BITMAP_WIDTH   16
#define BITMAP_HEIGHT  16

//----------------------------------------------------------------------------//
//    INIZIO class CGEOTree_ItemList                                          //
//----------------------------------------------------------------------------//


CGEOTree_ItemList::CGEOTree_ItemList() : C_LIST() {}
     
CGEOTree_ItemList::~CGEOTree_ItemList() {}

bool CGEOTree_ItemList::isValidNewItem(CGEOTree_Item *pItem)
{
   CGEOTree_Item *p;
   
   if (gsc_strlen(pItem->get_name()) == 0) return false;

   // controllo dati x codice
   if ((p = (CGEOTree_Item *) search_key(pItem->get_key())))
   {
      if (p->ObjectType == pItem->ObjectType) return false; // Oggetto dello stesso tipo già esistente
      while ((p = (CGEOTree_Item *) search_next_key(pItem->get_key())))
         if (p->ObjectType == pItem->ObjectType) return false; // Oggetto dello stesso tipo già esistente
   }
   // controllo dati x nome
   if ((p = (CGEOTree_Item *) search_name(pItem->get_name())))
   {
      if (p->ObjectType == pItem->ObjectType) return false; // Oggetto dello stesso tipo già esistente
      while ((p = (CGEOTree_Item *) search_next_name(pItem->get_name())))
         if (p->ObjectType == pItem->ObjectType) return false; // Oggetto dello stesso tipo già esistente
   }

   return true;
}

int CGEOTree_ItemList::AddItem(CGEOTree_Item *pItem, CGEOTree_Item *pParentItem,
                               CGEOTree_Item *pInsertAfterItem)
{ 
   if (!isValidNewItem(pItem)) return GS_CAN;

   pItem->pParentItem = pParentItem;

   if (!pInsertAfterItem) // se non è indicato pInsertAfterItem allora inserisco all'inizio
      add_head(pItem);
   else
   {
      int pos = getpos(pInsertAfterItem);

      if (pos == 0) add_tail(pItem);
      else insert_after(pos, pItem);
   }

   return GS_GOOD;
}

int CGEOTree_ItemList::MoveItem(CGEOTree_Item *pItem, CGEOTree_Item *pParentItem,
                                CGEOTree_Item *pInsertAfterItem)
{
   // Solo classi e set di classi possono essere spostati ma solo nello stesso progetto
   if (pItem->ObjectType != GSClass && pItem->ObjectType != GSClassSet)
      return GS_CAN;
   if (!pParentItem || pItem->get_prj() != pParentItem->get_prj())
      return GS_CAN;

   if (pItem->pParentItem != pParentItem && pParentItem)
      if (!(pParentItem->ChildItemList.isValidNewItem(pItem))) return GS_CAN;

   if (pItem->pParentItem->ChildItemList.cut(pItem) != GS_GOOD) return GS_BAD;

   return pParentItem->ChildItemList.AddItem(pItem, pParentItem, pInsertAfterItem);
}

int CGEOTree_ItemList::DelItem(CGEOTree_Item *pItem)
{
   // se cancello un set di classi allora le classi figlie si spostano al padre del set
   // (un altro set di classi o il progetto) nella posizione del set da cancellare
   if (pItem->ObjectType == GSClassSet)
   {
      int      pos = getpos(pItem);
      CGEOTree_Item *p, *pInsertAfterItem;

      if (pos == 0 || !pItem->pParentItem) return GS_BAD;
      pInsertAfterItem = (CGEOTree_Item *) getptr_at(pos - 1);
      p = (CGEOTree_Item *) pItem->ChildItemList.get_head();
      while ((p = (CGEOTree_Item *) pItem->ChildItemList.get_head()))
      {
         if (pItem->ChildItemList.MoveItem(p, pItem->pParentItem, pInsertAfterItem) != GS_GOOD)
            return GS_BAD;
         pInsertAfterItem = p;
      }
   }

   remove(pItem);

   return GS_GOOD;
}

int CGEOTree_ItemList::UpdItem(CGEOTree_Item *pItem, const TCHAR *_Name,
                               const TCHAR *_Descr, const TCHAR *_ImagePath,
                               GSDataPermissionTypeEnum _updateable)
{
   CGEOTree_Item *p;
   
   if (gsc_strlen(_Name) == 0) return GS_CAN;

   // controllo dati x nome
   if ((p = (CGEOTree_Item *) search_name(_Name)))
   {
      if (p->ObjectType == pItem->ObjectType && p != pItem) return GS_BAD; // Oggetto dello stesso tipo già esistente
      while ((p = (CGEOTree_Item *) search_next_name(pItem->get_name())))
         if (p->ObjectType == pItem->ObjectType && p != pItem) return GS_BAD; // Oggetto dello stesso tipo già esistente
   }

   pItem->set_name(_Name);
   pItem->Descr     = _Descr;
   pItem->ImagePath = _ImagePath;
   pItem->updatable = _updateable;

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTree_ItemList::search_selected     <external> */
/*+
  Questa funzione cerca il primo elemento selezionato.
  Parametri:
  CGEOTree_Item *pCurrent; Opzionale; Puntatore all'elemento da cui iniziare la ricerca
                                      (la ricerca inizia dall'elemento successivo)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
CGEOTree_Item *CGEOTree_ItemList::search_selected(CGEOTree_Item *pCurrent)
{
   CGEOTree_Item *p = NULL;

   if (!pCurrent) p = (CGEOTree_Item *) get_head();
   else p = (CGEOTree_Item *) pCurrent->get_next();

   while (p)
   {
      if (p->selected) break;
      p = (CGEOTree_Item *) p->get_next();
   }

   return p;
}

CGEOTree_Item *CGEOTree_ItemList::search_cls(int prj, int cls)
{
   CGEOTree_Item *p = (CGEOTree_Item *) get_head();

   while (p)
   {
      switch (p->ObjectType)
      {
         case GSProject:
            if (p->get_key() == prj)
               return p->ChildItemList.search_cls(prj, cls);
            break;
         case GSClassSet:
         {
            CGEOTree_Item *pItem = p->ChildItemList.search_cls(prj, cls);
            if (pItem) return pItem;
            break;
         }
         case GSClass:
            if (p->get_key() == cls) return p;
            break;
         default:
            return NULL;
      }            
      
      p = (CGEOTree_Item *) p->get_next();
   }

   return p;
}

CGEOTree_Item *CGEOTree_ItemList::search_sec(int prj, int cls, int sub, int sec)
{
   CGEOTree_Item *p = (CGEOTree_Item *) get_head();
   
   while (p)
   {
      switch (p->ObjectType)
      {
         case GSProject:
            if (p->get_key() == prj)
               return p->ChildItemList.search_sec(prj, cls, sub, sec);
            break;
         case GSClassSet:
         {
            CGEOTree_Item *pItem = p->ChildItemList.search_sec(prj, cls, sub, sec);
            if (pItem) return pItem;
            break;
         }
         case GSClass:
            if (p->get_key() == cls)
               return p->ChildItemList.search_sec(prj, cls, sub, sec);
            break;
         case GSSubClass:
            if (p->get_key() == sub)
               return p->ChildItemList.search_sec(prj, cls, sub, sec);
            break;
         case GSSecondaryTab:
            if (p->get_key() == sec) return p;
            break;
         default:
            return NULL;
      }            
      
      p = (CGEOTree_Item *) p->get_next();
   }

   return p;
}

/*********************************************************/
/*.doc CGEOTree_ItemList::LoadPrjs            <external> */
/*+
  Questa funzione carica la lista dei progetti di GEOsim.
  Parametri:
  GSDataModelObjectTypeEnum FinalObjectType; Tipologia di oggetto che sarà 
                                             nelle foglie dell'albero. 
                                             Serve per decidere quando fermare 
                                             il caricamento ricorsivo della 
                                             struttura della banca dati.
  int prj;  Opzionale, se <> 0 indica il progetto da caricare altrimenti 
            vengono caricati tutti i progetti (default = 0)
  C_INT_INT_LIST *pPrjPermissionList;  Opzionale; Lista delle abilitazioni ai progetti;
                                       (default = NULL)
  C_INT_VOIDPTR_LIST *pClassPermissionList; Opzionale; Lista delle abilitazioni alle classi del progetto;
                                            (default = NULL)
  C_INT_VOIDPTR_LIST *pSecPermissionList; Opzionale; Lista delle abilitazioni alle tabelle secondarie del progetto;
                                          (default = NULL)
  C_STR_LIST     *pInheritanceUserNames; lista opzionale degli utenti da cui ereditare 
                                         i permessi se non settati per un progetto o una classe;
                                         (default = NULL)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTree_ItemList::LoadPrjs(GSDataModelObjectTypeEnum FinalObjectType, int prj,
                                C_INT_INT_LIST     *pPrjPermissionList,
                                C_INT_VOIDPTR_LIST *pClassPermissionList,
                                C_INT_VOIDPTR_LIST *pSecPermissionList,
                                C_STR_LIST         *pInheritanceUserNames)
{
   C_PROJECT     *pPrj = (C_PROJECT *) get_GS_PROJECT()->get_head();
   CGEOTree_Item *pItem, *pInsertAfterItem = NULL;

   remove_all();

   if (prj == 0)
   {
      pPrj = (C_PROJECT *) get_GS_PROJECT()->get_head();
   }
   else
      pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(prj);

   while (pPrj)
   {
      if ((pItem = new CGEOTree_Item(pPrj->get_key(), pPrj->get_name(),
                                     pPrj->get_descr(), NULL, GSProject)) == NULL)
         return GS_BAD;
      if (AddItem(pItem, NULL, pInsertAfterItem) != GS_GOOD) return GS_BAD;
      pInsertAfterItem = pItem;

      // Se devo leggere le abilitazioni da una lista
      if (pPrjPermissionList)
      {
         if (pPrjPermissionList->search_key(pPrj->get_key()))
            pItem->updatable = (GSDataPermissionTypeEnum) ((C_INT_INT *) pPrjPermissionList->get_cursor())->get_type();
         else
            if (pInheritanceUserNames && pInheritanceUserNames->get_count() > 0) // se ci sono utenti da cui ereditare i permessi
               pItem->updatable = GSNonePermission;
            else
               pItem->updatable = GSInvisibleData;
      }
      else
         pItem->updatable = pPrj->get_level();

      if (get_GS_CURRENT_WRK_SESSION() && get_GS_CURRENT_WRK_SESSION()->get_PrjId() == pPrj->get_key())
         pItem->extracted = true;
      else
         pItem->extracted = false;

      if (FinalObjectType != GSProject) // carico le classi
      {
         C_INT_VOIDPTR *p;
         C_INT_INT_LIST  *pClasses = NULL, ClassList;
         C_4INT_STR_LIST *pSecTabs = NULL, SecList;

         if (pClassPermissionList)
            if ((p = (C_INT_VOIDPTR *) pClassPermissionList->search_key(pItem->get_key())) != NULL)
               pClasses = (C_INT_INT_LIST *) p->get_VoidPtr();
            else
               pClasses = &ClassList;

         if (pSecPermissionList)
            if ((p = (C_INT_VOIDPTR *) pSecPermissionList->search_key(pItem->get_key())) != NULL)
               pSecTabs = (C_4INT_STR_LIST *) p->get_VoidPtr();
            else
               pSecTabs = &SecList;

         if (pItem->ChildItemList.LoadClasses(pPrj->get_key(), pItem, FinalObjectType,
                                                pClasses, pSecTabs,
                                                pInheritanceUserNames) == GS_BAD) return GS_BAD;
      }

      if (prj == 0) pPrj = (C_PROJECT *) pPrj->get_next(); 
      else break;
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTree_ItemList::LoadClasses            <external> */
/*+
  Questa funzione carica la lista delle classi di un progetto di GEOsim.
  Parametri:
  int prj;                    Codice progetto
  CGEOTree_Item *pParentItem; Puntatore al padre della lista
  GSDataModelObjectTypeEnum FinalObjectType; Tipologia di oggetto che sarà 
                                             nelle foglie dell'albero. 
                                             Serve per decidere quando fermare 
                                             il caricamento ricorsivo della 
                                             struttura della banca dati.
  C_INT_INT_LIST *pClassPermissionList;   Opzionale; Lista delle abilitazioni alle classi;
                                          (default = NULL)
  C_4INT_STR_LIST *pSecPermissionList;    Opzionale; Lista delle abilitazioni alle tabelle secondarie;
                                          (default = NULL)
  C_STR_LIST     *pInheritanceUserNames;  lista opzionale degli utenti da cui ereditare 
                                          i permessi se non settati per un progetto o una classe;
                                          (default = NULL)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTree_ItemList::LoadClasses(int prj, CGEOTree_Item *pParentItem,
                                   GSDataModelObjectTypeEnum FinalObjectType,
                                   C_INT_INT_LIST *pClassPermissionList,
                                   C_4INT_STR_LIST *pSecPermissionList,
                                   C_STR_LIST     *pInheritanceUserNames)
{
   C_PROJECT          *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(prj);
   C_SINTH_CLASS_LIST SinthClassList;
   C_SINTH_CLASS      *pSinthClass;
   C_CLASS_SET        RootClassSet;
   CGEOTree_Item      *pInsertAfterItem = NULL;
   C_INT_INT          *pCode;

   remove_all();

   if (!pPrj) return GS_BAD;
   // Leggo la lista delle classi del progetto
   if (pPrj->getSinthClassList(SinthClassList) == GS_BAD) return GS_BAD;

   // Leggo la lista dei set di classi del progetto
   if (RootClassSet.from_db(pPrj) == GS_BAD) return GS_BAD;

   // carico i componenti del set (classi e set)
   pCode = (C_INT_INT *) RootClassSet.ClsCodeList.get_head();
   while (pCode)
   {
      if (pCode->get_type() == GSClassSet)
      {
         if ((pInsertAfterItem = LoadClassSet(prj, pParentItem, pInsertAfterItem,
                                                (C_CLASS_SET *) (RootClassSet.ClassSetList.search_key(pCode->get_key())), 
                                                SinthClassList, FinalObjectType, 
                                                pClassPermissionList, pSecPermissionList,
                                                pInheritanceUserNames)) == NULL)
            return NULL;
      }
      else // classe
      {
         if ((pSinthClass = (C_SINTH_CLASS *) SinthClassList.search_key(pCode->get_key())))
         {
            if ((pInsertAfterItem = pParentItem->ChildItemList.LoadClass(prj, pParentItem, pInsertAfterItem, 
                                                                         pSinthClass, FinalObjectType,
                                                                         pClassPermissionList, 
                                                                         pSecPermissionList,
                                                                         pInheritanceUserNames)) == NULL)
               return NULL;
            SinthClassList.remove(pSinthClass);
         }
      }

      pCode = (C_INT_INT *) pCode->get_next();
   }

   // ordino le classi che non sono nei set di classi del progetto in ordine alfabetico
   SinthClassList.sort_name();

   // Carica le classi che non sono nei set di classi del progetto
   return LoadClasses(prj, pParentItem, pInsertAfterItem, SinthClassList, 
                      FinalObjectType, true, pClassPermissionList, pSecPermissionList, pInheritanceUserNames);
}
int CGEOTree_ItemList::LoadClasses(int prj, CGEOTree_Item *pParentItem, CGEOTree_Item *pInsertAfterItem,
                                   C_SINTH_CLASS_LIST &SinthClassList,
                                   GSDataModelObjectTypeEnum FinalObjectType, bool append,
                                   C_INT_INT_LIST *pClassPermissionList, 
                                   C_4INT_STR_LIST *pSecPermissionList,
                                   C_STR_LIST *pInheritanceUserNames)
{
   C_SINTH_CLASS *pSinthClass = (C_SINTH_CLASS *) SinthClassList.get_head();

   if (!append) remove_all();

   while (pSinthClass)
   {
      if ((pInsertAfterItem = LoadClass(prj, pParentItem, pInsertAfterItem, pSinthClass, 
                                        FinalObjectType, pClassPermissionList,
                                        pSecPermissionList, pInheritanceUserNames)) == GS_BAD)
         return GS_BAD;

      pSinthClass = (C_SINTH_CLASS *) pSinthClass->get_next();
   }

   return GS_GOOD;
}
CGEOTree_Item *CGEOTree_ItemList::LoadClass(int prj, CGEOTree_Item *pParentItem,
                                            CGEOTree_Item *pInsertAfterItem,
                                            C_SINTH_CLASS *pSinthClass,
                                            GSDataModelObjectTypeEnum FinalObjectType,
                                            C_INT_INT_LIST *pClassPermissionList,
                                            C_4INT_STR_LIST *pSecPermissionList,
                                            C_STR_LIST *pInheritanceUserNames)
{
   CGEOTree_Item *pItem;

   if ((pItem = new CGEOTree_Item(pSinthClass->get_key(), pSinthClass->get_name(),
                                  pSinthClass->get_Descr(), NULL, GSClass)) == NULL)
      return NULL;

   if (pParentItem)
   {
      if (pParentItem->ChildItemList.AddItem(pItem, pParentItem, pInsertAfterItem) != GS_GOOD) return NULL;
   }
   else
      if (AddItem(pItem, NULL, pInsertAfterItem) != GS_GOOD) return NULL;

   pItem->category  = pSinthClass->get_category();
   pItem->type      = pSinthClass->get_type();

   // Se devo leggere le abilitazioni da una lista
   if (pClassPermissionList)
   {
      if (pClassPermissionList->search_key(pSinthClass->get_key()))
         pItem->updatable = (GSDataPermissionTypeEnum) ((C_INT_INT *) pClassPermissionList->get_cursor())->get_type();
      else
         if (pInheritanceUserNames && pInheritanceUserNames->get_count() > 0) // se ci sono utenti da cui ereditare i permessi
            pItem->updatable = GSNonePermission;
         else
            pItem->updatable = GSInvisibleData;
   }
   else
      pItem->updatable = pSinthClass->get_level();

   pItem->extracted = (pSinthClass->get_extracted() == EXTRACTED ||
                       pSinthClass->get_extracted() == EXTR_SEL) ? true : false;

   pSinthClass->get_color(pItem->color);

   if (FinalObjectType != GSClass)
      if (pItem->category == CAT_EXTERN) // carico le sottoclassi
      {
         if (pItem->ChildItemList.LoadSubClasses(prj, pSinthClass, pItem,
                                                 FinalObjectType, pSecPermissionList, pInheritanceUserNames) == GS_BAD) return NULL;
      }
      else if (FinalObjectType == GSSecondaryTab) // carico le tabelle secondarie
         if (pItem->ChildItemList.LoadSecTabs(prj, pSinthClass->get_key(), 0,
                                              pItem, pSecPermissionList, pInheritanceUserNames) == GS_BAD) return NULL;

   return pItem;
}


/*********************************************************/
/*.doc CGEOTree_ItemList::LoadClassSet        <external> */
/*+
  Questa funzione carica un set di classi di un progetto di GEOsim.
  Parametri:
  int prj;                    Codice progetto
  CGEOTree_Item *pParentItem; Puntatore al padre della lista
  C_CLASS_SET *pClassSet;     Set di classi
  C_SINTH_CLASS_LIST &SinthClassList;
  GSDataModelObjectTypeEnum FinalObjectType; Tipologia di oggetto che sarà 
                                             nelle foglie dell'albero. 
                                             Serve per decidere quando fermare 
                                             il caricamento ricorsivo della 
                                             struttura della banca dati.
  C_INT_INT_LIST *pClassPermissionList;  Opzionale; Lista delle abilitazioni alle classi;
                                         (default = NULL)
  C_4INT_STR_LIST *pSecPermissionList;   Opzionale; Lista delle abilitazioni alle tabelle secondarie;
                                         (default = NULL)
  C_STR_LIST     *pInheritanceUserNames; lista opzionale degli utenti da cui ereditare 
                                         i permessi se non settati per un progetto o una classe;
                                         (default = NULL)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
CGEOTree_Item *CGEOTree_ItemList::LoadClassSet(int prj, CGEOTree_Item *pParentItem, CGEOTree_Item *pInsertAfterItem,
                                               C_CLASS_SET *pClassSet, C_SINTH_CLASS_LIST &SinthClassList,
                                               GSDataModelObjectTypeEnum FinalObjectType,
                                               C_INT_INT_LIST *pClassPermissionList,
                                               C_4INT_STR_LIST *pSecPermissionList,
                                               C_STR_LIST     *pInheritanceUserNames)
{
   C_SINTH_CLASS *pSinthClass;
   CGEOTree_Item *pItem, *pLastItem = NULL;
   C_INT_INT     *pCode;

   if ((pItem = new CGEOTree_Item(pClassSet->get_key(), pClassSet->get_name(), 
                                  pClassSet->get_Descr(), pClassSet->ImagePath.get_name(),
                                  GSClassSet)) == NULL)
      return NULL;
   if (pParentItem->ChildItemList.AddItem(pItem, pParentItem, pInsertAfterItem) != GS_GOOD) return NULL;

   // carico i componenti del set (classi e set)
   pCode = (C_INT_INT *) pClassSet->ClsCodeList.get_head();
   while (pCode)
   {
      if (pCode->get_type() == GSClassSet)
      {
         if ((pLastItem = LoadClassSet(prj, pItem, pLastItem,
                                       (C_CLASS_SET *) (pClassSet->ClassSetList.search_key(pCode->get_key())), 
                                       SinthClassList, FinalObjectType, pClassPermissionList,
                                       pSecPermissionList, pInheritanceUserNames)) == NULL)
            return NULL;
      }
      else // classe
      {
         if ((pSinthClass = (C_SINTH_CLASS *) SinthClassList.search_key(pCode->get_key())))
         {
            if ((pLastItem = pItem->ChildItemList.LoadClass(prj, pItem, pLastItem, 
                                                            pSinthClass, FinalObjectType,
                                                            pClassPermissionList,
                                                            pSecPermissionList,
                                                            pInheritanceUserNames)) == NULL)
               return NULL;
            SinthClassList.remove(pSinthClass);
         }
      }

      pCode = (C_INT_INT *) pCode->get_next();
   }
   
   return pItem;
}


/***************************************************************/
/*.doc CGEOTree_ItemList::LoadSubClasses            <external> */
/*+
  Questa funzione carica la lista delle sottoclassi di GEOsim.
  Parametri:
  int prj;                                   Codice progetto
  C_SINTH_CLASS *pParentSinthClass;          Classe madre già caricata in memoria.
  CGEOTree_Item *pParentItem;                Puntatore al padre della lista
  GSDataModelObjectTypeEnum FinalObjectType; Tipologia di oggetto che sarà 
                                             nelle foglie dell'albero. 
                                             Serve per decidere quando fermare 
                                             il caricamento ricorsivo della 
                                             struttura della banca dati.
  C_4INT_STR_LIST *pSecPermissionList;   Opzionale; Lista delle abilitazioni alle tabelle secondarie;
                                         (default = NULL)
  C_STR_LIST     *pInheritanceUserNames; lista opzionale degli utenti da cui ereditare 
                                         i permessi se non settati per un progetto o una classe;
                                         (default = NULL)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/***************************************************************/
int CGEOTree_ItemList::LoadSubClasses(int prj, C_SINTH_CLASS *pParentSinthClass, CGEOTree_Item *pParentItem,
                                      GSDataModelObjectTypeEnum FinalObjectType,
                                      C_4INT_STR_LIST *pSecPermissionList,
                                      C_STR_LIST      *pInheritanceUserNames)
{
   C_SINTH_CLASS_LIST SinthClassList;
   C_SINTH_CLASS      *pSinthClass;
   CGEOTree_Item      *pItem, *pLastItem = NULL;
   int                cls = pParentSinthClass->get_key();

   remove_all();

   if ((pSinthClass = pParentSinthClass) == NULL)
      return GS_BAD;

   pSinthClass = (C_SINTH_CLASS *) pSinthClass->ptr_sub_list()->get_head();
   if (pSinthClass)
   {
      while (pSinthClass)
      {
         if ((pItem = new CGEOTree_Item(pSinthClass->get_key(), pSinthClass->get_name(),
                                        pSinthClass->get_Descr(), NULL,
                                        GSSubClass)) == NULL)
            return GS_BAD;
         if (pParentItem->ChildItemList.AddItem(pItem, pParentItem, pLastItem) != GS_GOOD) return GS_BAD;
         pLastItem = pItem;

         pItem->category  = pSinthClass->get_category();
         pItem->type      = pSinthClass->get_type();
         pItem->updatable = pSinthClass->get_level();
         pItem->extracted = (pSinthClass->get_extracted() == EXTRACTED ||
                             pSinthClass->get_extracted() == EXTR_SEL) ? true : false;
         pSinthClass->get_color(pItem->color);

         if (FinalObjectType == GSSecondaryTab) // carico le tabelle secondarie
            if (pItem->ChildItemList.LoadSecTabs(prj, cls, pSinthClass->get_key(),
                                                 pItem, pSecPermissionList, pInheritanceUserNames) == GS_BAD) return GS_BAD;

         pSinthClass = (C_SINTH_CLASS *) pSinthClass->get_next();
      }
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTree_ItemList::LoadSecTabs         <external> */
/*+
  Questa funzione carica la lista delle tabelle secondarie di una classe 
  o sottoclasse di GEOsim.
  Parametri:
  int prj;                    Codice progetto
  int cls;                    Codice classe
  int sub;                    Codice sottoclasse
  CGEOTree_Item *pParentItem; Puntatore al padre della lista
  C_4INT_STR_LIST *pSecPermissionList; Opzionale; Lista delle abilitazioni alle tabelle secondarie;
                                       (default = NULL)
  C_STR_LIST *pInheritanceUserNames;   lista opzionale degli utenti da cui ereditare 
                                       i permessi se non settati per un progetto o una classe;
                                       (default = NULL)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTree_ItemList::LoadSecTabs(int prj, int cls, int sub, CGEOTree_Item *pParentItem,
                                   C_4INT_STR_LIST *pSecPermissionList,
                                   C_STR_LIST      *pInheritanceUserNames)
{
   C_PROJECT            *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(prj);
   C_SINTH_SEC_TAB_LIST SinthSecList;
   C_SINTH_SEC_TAB      *pSinthSec;
   CGEOTree_Item        *pItem, *pLastItem = NULL;

   remove_all();

   if (pPrj->getSinthClsSecondaryTabList(cls, sub, SinthSecList) == GS_BAD) return GS_BAD;

   pSinthSec = (C_SINTH_SEC_TAB *) SinthSecList.get_head();
   while (pSinthSec)
   {
      if ((pItem = new CGEOTree_Item(pSinthSec->get_key(), pSinthSec->get_name(),
                                     pSinthSec->get_Descr(), NULL,
                                     GSSecondaryTab)) == NULL)
         return GS_BAD;
      if (pParentItem->ChildItemList.AddItem(pItem, pParentItem, pLastItem) != GS_GOOD) return GS_BAD;
      pLastItem   = pItem;
      pItem->type = pSinthSec->get_type();

      // Se devo leggere le abilitazioni da una lista
      if (pSecPermissionList)
      {
         if (pSecPermissionList->search(cls, sub, pSinthSec->get_key()))
            pItem->updatable = ((C_4INT_STR *) pSecPermissionList->get_cursor())->get_level();
         else
            if (pInheritanceUserNames && pInheritanceUserNames->get_count() > 0) // se ci sono utenti da cui ereditare i permessi
               pItem->updatable = GSNonePermission;
            else
               pItem->updatable = GSInvisibleData;
      }
      else
         if (pParentItem->updatable != GSUpdateableData || 
             pSinthSec->get_type() == GSExternalSecondaryTable)
            pItem->updatable = GSReadOnlyData;
         else
            pItem->updatable = GSUpdateableData;

      pItem->extracted = pParentItem->extracted; // come la classe madre

      pSinthSec = (C_SINTH_SEC_TAB *) pSinthSec->get_next();
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTree_ItemList::GetTotalImageCount  <external> */
/*+
  Questa funzione conta quante sono le immagini di tutto l'albero.
  Parametri:
  int *ImageCount;      Puntatore al conteggio;

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTree_ItemList::GetTotalImageCount(int *ImageCount)
{
   (*ImageCount) += get_count();
   CGEOTree_Item *pItem = (CGEOTree_Item *) get_head();

   while (pItem)
   {
      pItem->ChildItemList.GetTotalImageCount(ImageCount);
      pItem = (CGEOTree_Item *) get_next();
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTree_ItemList::SetTotalImageList  <external> */
/*+
  Questa funzione ottiene le immagini di tutto l'albero in una ImageList
  settando per ogni elemento la posizione image e settando le singole
  ImageList.
  Parametri:
  CImageList &TotalImageList; ImageList di destinazione

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTree_ItemList::SetTotalImageList(CImageList &TotalImageList)
{
   CGEOTree_Item *pItem;
   CBitmap       CBMP;
   
   pItem = (CGEOTree_Item *) get_head();
   while (pItem)
   {
      if (pItem->get_bitmap(CBMP) == GS_BAD) return GS_BAD; // carico la bitmap
      pItem->image = TotalImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente

      pItem = (CGEOTree_Item *) get_next();
   }

   pItem = (CGEOTree_Item *) get_head();
   while (pItem)
   {
      if (pItem->ChildItemList.SetTotalImageList(TotalImageList) == GS_BAD)
         return GS_BAD;
      pItem = (CGEOTree_Item *) get_next();
   }

   return GS_GOOD;
}


//----------------------------------------------------------------------------//
//   FINE   class CGEOTree_ItemList                                           //
//   INIZIO class CGEOList_Item                                               //
//----------------------------------------------------------------------------//

CGEOTree_Item::CGEOTree_Item() : C_INT_STR()
{
   category = 0;
	type     = 0;
	updatable = GSInvisibleData;
	extracted = false;
	image     = 0;
   selected  = false;
   n_selected  = 0; // roby 2016
   pParentItem = NULL;
}

CGEOTree_Item::CGEOTree_Item(int _Key, const TCHAR *_Name,
                             const TCHAR *_Descr, const TCHAR *_ImagePath,
                             GSDataModelObjectTypeEnum _ObjectType) : C_INT_STR()
{
   category = 0;
	type     = 0;
	updatable = GSInvisibleData;
	extracted = false;
	image     = 0;
   selected  = false;
   n_selected  = 0; // roby 2016
   pParentItem = NULL;

   set_key(_Key);
   set_name(_Name);
   Descr      = _Descr;
   ImagePath  = _ImagePath;
   ObjectType = _ObjectType;

   switch (ObjectType)
   {
      case GSProject:
      case GSClassSet:
         category = type = 0;
         break;
      case GSSecondaryTab:
         category = CAT_SECONDARY;
         break;
      case GSSubClass:
         category = CAT_SUBCLASS;
         break;
   }
}

CGEOTree_Item::CGEOTree_Item(CGEOTree_Item &item)
{
   item.copy(*this);
}

CGEOTree_Item::~CGEOTree_Item()
{
   ChildItemList.remove_all();
}

int CGEOTree_Item::get_type()
   { return type; }

int CGEOTree_Item::get_category()
   { return category; }

int CGEOTree_Item::get_prj()
{
   CGEOTree_Item *p = this;

   while (p)
      if (p->ObjectType == GSProject) break;
      else p = p->pParentItem;

   return (p) ? p->get_key() : 0;
}

int CGEOTree_Item::get_cls_set()
{
   CGEOTree_Item *p = this;

   while (p)
      if (p->ObjectType == GSClassSet) break;
      else p = p->pParentItem;

   return (p) ? p->get_key() : 0;
}

int CGEOTree_Item::get_cls()
{
   CGEOTree_Item *p = this;

   while (p)
      if (p->ObjectType == GSClass) break;
      else p = p->pParentItem;

   return (p) ? p->get_key() : 0;
}

int CGEOTree_Item::get_sub()
{
   CGEOTree_Item *p = this;

   while (p)
      if (p->ObjectType == GSSubClass) break;
      else p = p->pParentItem;

   return (p) ? p->get_key() : 0;
}

int CGEOTree_Item::get_sec()
{
   CGEOTree_Item *p = this;

   while (p)
      if (p->ObjectType == GSSecondaryTab) break;
      else p = p->pParentItem;

   return (p) ? p->get_key() : 0;
}

int CGEOTree_Item::get_bitmap(CBitmap &CBMP)
{
   bool BMPAlreadyLoad = false;;

   if (gsc_path_exist(ImagePath) == GS_GOOD)
   {
      HBITMAP hBitmap;
      if ((hBitmap = (HBITMAP)LoadImage(NULL, ImagePath.get_name(),
                                        IMAGE_BITMAP, 0, 0,
                                        LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE)) != NULL)
      if (CBMP.Attach(hBitmap))
         BMPAlreadyLoad = true;
   }

   if (!BMPAlreadyLoad)
      switch (ObjectType)
      {
         case GSProject:
         {
            C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(get_prj());
            if (!pPrj || pPrj->get_Bitmap(false, CBMP) == GS_BAD)
               return GS_BAD; // carico la bitmap larga 16 pixel
            break;
         }
         case GSClassSet:
            if (gsc_getClassSet_StandardBitmap(false, CBMP) == GS_BAD)
               return GS_BAD; // carico la bitmap larga 16 pixel
            break;
         case GSClass:
         case GSSubClass:
            if (gsc_getClassBitmap(get_category(), get_type(), color, false, CBMP) == GS_BAD)
               return GS_BAD; // carico la bitmap larga 16 pixel
            break;
         case GSSecondaryTab:
            if (gsc_getSecTabBitmap(false, CBMP) == GS_BAD)
               return GS_BAD; // carico la bitmap larga 16 pixel
            break;
      }

   return GS_GOOD;
}

void CGEOTree_Item::copy(CGEOTree_Item &out)
{
   out.set_key(get_key());
   out.set_name(get_name());
   out.Descr = Descr;
   out.category = category;
   out.type = type;
   out.color = color;
   out.updatable = updatable;
   out.extracted = extracted;
   out.image = image;
   out.ImagePath = ImagePath;
   out.selected = selected;
   out.n_selected = n_selected; // roby 2016
   out.ObjectType = ObjectType;
}

// CGEOTreeCtrl

IMPLEMENT_DYNCREATE(CGEOTreeCtrl, CTreeCtrl)

CGEOTreeCtrl::CGEOTreeCtrl()
{
   MultiSelect         = false;
   SelectedLinkedClass = false;
   FinalObjectType     = GSClass;
   DisplayEmptyClassSet = false;
   FilterOnExtracted   = false;
   FilterOnUpdateable  = false;
   CutAndPaste         = false;

   PrjPermissionVisibility   = false;
   ClassPermissionVisibility = false;
   SecPermissionVisibility   = false;

   pToolTip  = NULL;
   OnRefresh = false;
   m_CutAndPasteItem = NULL;
   m_SelectedItemOnRClick = NULL;
}

CGEOTreeCtrl::~CGEOTreeCtrl()
{
   if (pToolTip) delete pToolTip;
}

BEGIN_MESSAGE_MAP(CGEOTreeCtrl, CTreeCtrl)
   ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnSelChanged)
   ON_NOTIFY_REFLECT_EX(NM_CLICK, OnClick)
   ON_WM_KEYDOWN()
   ON_NOTIFY_REFLECT(NM_RCLICK, OnRClick)
   ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_GEOTREECTRL_SEARCH_BY_NAME_MENU, OnSearchByNameMenu)
	ON_COMMAND(ID_GEOTREECTRL_LOAD_SEL_MENU, OnLoadSelMenu)
	ON_COMMAND(ID_GEOTREECTRL_SAVE_SEL_MENU, OnSaveSelMenu)
	ON_COMMAND(ID_GEOTREECTRL_REFRESH_MENU, OnRefreshMenu)
	ON_COMMAND(ID_GEOTREECTRL_INVERT_SEL_MENU, OnInvertSelMenu)
	ON_COMMAND(ID_GEOTREECTRL_CUT_MENU, OnCutMenu)
	ON_COMMAND(ID_GEOTREECTRL_PASTE_MENU, OnPasteMenu)
   ON_WM_MOUSEMOVE()
   //ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// diagnostica di CGEOTreeCtrl

#ifdef _DEBUG
void CGEOTreeCtrl::AssertValid() const
{
	CTreeCtrl::AssertValid();
}

#ifndef _WIN32_WCE
void CGEOTreeCtrl::Dump(CDumpContext& dc) const
{
	CTreeCtrl::Dump(dc);
}
#endif
#endif //_DEBUG


// gestori di messaggi CGEOTreeCtrl


///////////////////////////////////////////////////////////////////////////////
// FUNZIONI PER TOOLTIP - INIZIO
///////////////////////////////////////////////////////////////////////////////


BOOL CGEOTreeCtrl::InitTooltip(void)
{
   if (pToolTip) delete pToolTip;
   pToolTip = new CToolTipCtrl;
   if (!pToolTip->Create(this)) return FALSE;
   //if (!pToolTip->Create(this, TTS_ALWAYSTIP)) return FALSE;
	if (m_hWnd) pToolTip->AddTool(this, _T(""));
   //pToolTip->SetDelayTime(TTDT_INITIAL, 0);
   pToolTip->Activate(TRUE);
   
   return TRUE;
}

// Le coordinate espresse da pt sono "Screen"
BOOL CGEOTreeCtrl::DisplayTooltip(MSG* pMsg)
{
   CRect     Rect, ScreenRect;
   HTREEITEM pItem;

   if (!pToolTip || !pToolTip->m_hWnd) return FALSE;

   if ((pItem = TreeView_GetFirstVisible(pMsg->hwnd)) == NULL) return TRUE;
   while (pItem)
   {
      if (TreeView_GetItemRect(pMsg->hwnd, pItem, &Rect, FALSE) == TRUE)
      {
         ScreenRect = Rect;
         ClientToScreen(ScreenRect);
         if (ScreenRect.PtInRect(pMsg->pt))
            break;
      }

      pItem = TreeView_GetNextVisible(pMsg->hwnd, pItem);
   }

   // Se non era su una riga del controllo
   if (!pItem) return TRUE;

   TV_ITEM tvI;

   tvI.hItem = pItem;
   tvI.mask  = TVIF_PARAM | TVIF_HANDLE;

   if (TreeView_GetItem(pMsg->hwnd, &tvI) && tvI.lParam)
   {
      CString ItemText, PrevItemText;

      ItemText = ((CGEOTree_Item *) tvI.lParam)->Descr.get_name();
      ItemText.Trim();
    
      if (gsc_strlen(ItemText) == 0)
         ItemText = _T("");
      
      pToolTip->GetText(PrevItemText, this);
      if (PrevItemText.Compare(ItemText) != 0)
         pToolTip->UpdateTipText(ItemText, this);
      pToolTip->RelayEvent(pMsg);
   }

   return TRUE;
}

BOOL CGEOTreeCtrl::PreTranslateMessage(MSG* pMsg)
{
   DisplayTooltip(pMsg);
   return CTreeCtrl::PreTranslateMessage(pMsg);
}

///////////////////////////////////////////////////////////////////////////////
// FUNZIONI PER TOOLTIP - FINE
///////////////////////////////////////////////////////////////////////////////


/*********************************************************/
/*.doc CGEOTreeCtrl::LoadFromDB            <external> */
/*+
  Questa funzione carica la lista da DB.
  Parametri:
  int prj;  Opzionale, se <> 0 indica il progetto da caricare altrimenti 
            vengono caricati tutti i progetti (default = 0)
  C_INT_INT_LIST *pPrjPermissionList; lista opzionale dei permessi per i progetti (default = NULL)
  C_INT_VOIDPTR_LIST *pClassPermissionList; lista opzionale dei permessi per le classi del progetto (default = NULL)
  C_INT_VOIDPTR_LIST *pSecPermissionList; lista opzionale dei permessi per le secondarie del progetto (default = NULL)
  C_STR_LIST     *pInheritanceUserNames; lista opzionale degli utenti da cui ereditare 
                                         i permessi se non settati per un progetto o una classe
                                         (default = NULL)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTreeCtrl::LoadFromDB(int prj,
                             C_INT_INT_LIST     *pPrjPermissionList,
                             C_INT_VOIDPTR_LIST *pClassPermissionList,
                             C_INT_VOIDPTR_LIST *pSecPermissionList,
                             C_STR_LIST         *pInheritanceUserNames)
{
   if (ItemList.LoadPrjs(FinalObjectType, prj, 
                         pPrjPermissionList, 
                         pClassPermissionList, 
                         pSecPermissionList, 
                         pInheritanceUserNames) != GS_GOOD)
      return GS_BAD;

   // Creazione lista totale delle immagini
   return SetTotalImageList();
}


/*********************************************************/
/*.doc CGEOTreeCtrl::AddItem                  <external> */
/*+
  Questa funzione aggiunge un item all'albero, alla lista degli item e
  alla lista delle immagini
  CGEOTree_Item *pItem;
  HTREEITEM hParentItem;
  HTREEITEM hInsertAfterItem;

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTreeCtrl::AddItem(CGEOTree_Item *pItem, HTREEITEM hParentItem,
                          HTREEITEM hInsertAfterItem)
{
   CGEOTree_Item *pParentItem = NULL, *pInsertAfterItem = NULL;

   if (hParentItem) pParentItem = (CGEOTree_Item *) GetItemData(hParentItem);
   if (hInsertAfterItem) pInsertAfterItem = (CGEOTree_Item *) GetItemData(hInsertAfterItem);

   // Creazione lista delle immagini
   CBitmap CBMP;

   if (pItem->get_bitmap(CBMP) == GS_BAD) return GS_BAD; // carico la bitmap

   pItem->image = ImageList.GetImageCount();
   ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente

   CGEOTree_ItemList *pList;

   if (pParentItem == NULL) pList = &ItemList;
   else pList = &(pParentItem->ChildItemList);

   if (pList->AddItem(pItem, pParentItem, pInsertAfterItem) != GS_GOOD)
      { ImageList.Remove(pItem->image); return GS_BAD; }

   HTREEITEM hItem;
   if ((hItem = FilteredInsertItemOnTree(pItem, pParentItem, hParentItem, hInsertAfterItem)) == NULL)
      return GS_BAD;
   SelectItem(hItem);

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::UpdItem                  <external> */
/*+
  Questa funzione aggiorna un item dall'albero, dalla lista degli item e
  dalla lista delle immagini.
  Parametri:
  CGEOTree_Item *pItem;
  const TCHAR *_Name;      Nuovo nome
  const TCHAR *_Descr;     Nuova descrizione
  const TCHAR *_ImagePath; Nuova path dell'immagine
  GSDataPermissionTypeEnum _updateable;         Nuovo livello di abilitazione
  HTREEITEM hItem;

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTreeCtrl::UpdItem(CGEOTree_Item *pItem, const TCHAR *_Name,
                          const TCHAR *_Descr, const TCHAR *_ImagePath,
                          GSDataPermissionTypeEnum _updateable, HTREEITEM hItem)
{
   bool differentImage = false;

   if (pItem->ImagePath.comp(_ImagePath) != 0) differentImage = true;   
   if (ItemList.UpdItem(pItem, _Name, _Descr, _ImagePath, _updateable) == GS_BAD) return GS_BAD;
   if (differentImage)
   {
      CBitmap CBMP;

      if (pItem->get_bitmap(CBMP) == GS_GOOD) // carico la bitmap
      {
         pItem->image = ImageList.GetImageCount();
         ImageList.Add(&CBMP, RGB(255, 255, 255)); // il bianco diventa trasparente
      }
   }

   if (UpdateItemOnTree(pItem, hItem) == GS_BAD) return GS_BAD;

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::DelItem                  <external> */
/*+
  Questa funzione cancella un item dall'albero e dalla lista degli item.
  Parametri:
  HTREEITEM hItem;

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTreeCtrl::DelItem(HTREEITEM hItem)
{  
   CGEOTree_Item *pItem = (CGEOTree_Item *) GetItemData(hItem);

   if (pItem->pParentItem)
   {
      CGEOTree_Item *pParentItem = pItem->pParentItem;
      HTREEITEM     hParentItem = GetParentItem(hItem);
      C_INT_LIST    SelectedList;
      C_INT         *pSelected;

      // memorizzo lo stato dei figli perchè cancellandoli succede
      // che tutti si selezionano (quando si cancella un item viene selezionato il successivo)
      if (pParentItem->ChildItemList.DelItem(pItem) == GS_BAD) return GS_BAD;
      pItem = (CGEOTree_Item *) pParentItem->ChildItemList.get_head();
      while (pItem)
      {
         SelectedList.add_tail_int((pItem->selected) ? 1 : 0);

         pItem = (CGEOTree_Item *) pItem->get_next();
      }

      DeleteChildItems(hParentItem);

      // ripristino lo stato di selezione
      pItem     = (CGEOTree_Item *) pParentItem->ChildItemList.get_head();
      pSelected = (C_INT *) SelectedList.get_head();
      while (pSelected)
      {
         pItem->selected = (pSelected->get_key() == 1) ? true : false;

         pItem     = (CGEOTree_Item *) pItem->get_next();
         pSelected = (C_INT *) pSelected->get_next();
      }

      Refresh(pParentItem, hParentItem);
   }
   else
   {
      if (ItemList.DelItem(pItem) == GS_BAD) return GS_BAD;
      DeleteItem(hItem);
   }

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::MoveItem                 <external> */
/*+
  Questa funzione sposta un item dall'albero e dalla lista degli item
  HTREEITEM hItem;
  HTREEITEM hParentItem:
  HTREEITEM hInsertAfterItem;

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTreeCtrl::MoveItem(HTREEITEM hItem,
                           HTREEITEM hParentItem, HTREEITEM hInsertAfterItem)
{
   CGEOTree_Item *pItem, *pParentItem = NULL, *pInsertAfterItem = NULL;

   pItem = (CGEOTree_Item *) GetItemData(hItem);
   if (hParentItem) pParentItem = (CGEOTree_Item *) GetItemData(hParentItem);
   if (hInsertAfterItem) pInsertAfterItem = (CGEOTree_Item *) GetItemData(hInsertAfterItem);

   // memorizzo lo stato dei figli perchè cancellandoli succede
   // che tutti si selezionano (quando si cancella un item viene selezionato il successivo)
   if (ItemList.MoveItem(pItem, pParentItem, pInsertAfterItem) != GS_GOOD) return GS_BAD;

   DeleteItem(hItem);

   if ((hItem = FilteredInsertItemOnTree(pItem, pParentItem, hParentItem, hInsertAfterItem)) == NULL)
      return GS_BAD;
   SelectItem(hItem);

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::SetTotalImageList        <external> */
/*+
  Questa crea la lista totale delle immagini.

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTreeCtrl::SetTotalImageList(void)
{
   if (ImageList.GetImageCount() > 0)
      if (ImageList.DeleteImageList() == 0)
         return GS_BAD;
   if (ImageList.Create(BITMAP_WIDTH, BITMAP_HEIGHT,
                        ILC_MASK | ILC_COLORDDB, 
                        0,
                        1) == NULL)
		return GS_BAD;

   return ItemList.SetTotalImageList(ImageList);
}


/*********************************************************/
/*.doc CGEOTreeCtrl::FindNextItem             <external> */
/*+
  Questa funzione cerca l'elemento successivo dell'albero.
  Parametri:
  HTREEITEM hCurrentItem;   se = NULL viene considerata la root

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
HTREEITEM CGEOTreeCtrl::FindNextItem(HTREEITEM hCurrentItem)
{
   HTREEITEM hNextItem;

   if (hCurrentItem)
   {
      if ((hNextItem = GetChildItem(hCurrentItem)) == NULL) // Se non ha figli
         if ((hNextItem = GetNextSiblingItem(hCurrentItem)) == NULL) // Se non ha fratelli
            while ((hCurrentItem = GetParentItem(hCurrentItem)) != NULL) // Se ha un parente
            {
               if ((hNextItem = GetNextSiblingItem(hCurrentItem)) != NULL) // se ha un fratello successivo
                  break;                
            }
   }
   else // restitusco la root
       hNextItem = GetRootItem();

   return hNextItem;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::FindItemInSubTree        <external> */
/*+
  Questa funzione cerca un elemento nel sotto albero.
  Parametri:
  HTREEITEM hParentItem;   Item padre, se = NULL viene considerata la root
  LPCTSTR lpszItem;        Testo da cercare
  bool wildcomp;           Se true la stringa viene considerata una mascehra 
                           di ricerca con i caratteri * e ? (default= false)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
HTREEITEM CGEOTreeCtrl::FindItemInSubTree(HTREEITEM hCurrentItem, LPCTSTR lpszItem, bool wildcomp)
{
   HTREEITEM hItem, hChildItem;
   CString   ItemText;

   if (!hCurrentItem) hCurrentItem = GetRootItem();
   if ((hChildItem = GetChildItem(hCurrentItem)) == NULL) return NULL; // Se non ha figli

   ItemText = GetItemText(hChildItem);
   if (wildcomp)
   {
      C_STRING dummy((LPCTSTR) ItemText);
      if (dummy.wildcomp(lpszItem)) return hChildItem;
   }
   else
      if (ItemText.Compare(lpszItem) == 0) return hChildItem;

   // Controllo i fratelli
   while ((hChildItem = GetNextSiblingItem(hChildItem)))  // fratello successivo
   {
      ItemText = GetItemText(hChildItem);
      if (wildcomp)
      {
         C_STRING dummy((LPCTSTR) ItemText);
         if (dummy.wildcomp(lpszItem)) return hChildItem;
      }
      else
         if (ItemText.Compare(lpszItem) == 0) return hChildItem;

      if ((hItem = FindItem(hChildItem, lpszItem, wildcomp))) return hItem;
   }

   return NULL;
}
/*********************************************************/
/*.doc CGEOTreeCtrl::FindItem                 <external> */
/*+
  Questa funzione cerca un elemento nell'albero intero.
  Parametri:
  HTREEITEM hParentItem;   Item padre, se = NULL viene considerata la root
  LPCTSTR lpszItem;        Testo da cercare
  bool wildcomp;           Se true la stringa viene considerata una mascehra 
                           di ricerca con i caratteri * e ? (default= false)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
HTREEITEM CGEOTreeCtrl::FindItem(HTREEITEM hCurrentItem, LPCTSTR lpszItem, bool wildcomp)
{
   HTREEITEM hItem = hCurrentItem;
   CString   ItemText;

   while ((hItem = FindNextItem(hItem)))
   {
      ItemText = GetItemText(hItem);

      if (wildcomp)
      {
         C_STRING dummy((LPCTSTR) ItemText);
         if (dummy.wildcomp(lpszItem)) return hItem;
      }
      else
         if (ItemText.Compare(lpszItem) == 0) return hItem;
   }

   return NULL;
}
HTREEITEM CGEOTreeCtrl::FindItem(HTREEITEM hCurrentItem, DWORD_PTR lParam)
{
   HTREEITEM hItem;

   while ((hItem = FindNextItem(hCurrentItem)))
      if (lParam == GetItemData(hItem)) return hItem;
      else hCurrentItem = hItem;

   return NULL;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::Refresh                  <external> */
/*+
  Questa funzione visualizza gli elementi dell'albero applicando
  le impostazioni di visualizzazione.
  Parametri:
  CGEOTree_Item *pParentItem; uso interno (default = NULL)
  HTREEITEM hParent           uso interno (default = NULL)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int CGEOTreeCtrl::Refresh(CGEOTree_Item *pParentItem, HTREEITEM hParent)
{
   CGEOTree_Item *pItem;
   HTREEITEM     hItem = NULL, PrevHItem = NULL;

   OnRefresh = true;

   if (pParentItem == NULL || hParent == NULL) // svuoto l'albero
   {
      Reset_Tree();
      hParent = TVI_ROOT;
      if (InitTooltip() == FALSE) return GS_BAD;
   }

   if (pParentItem == NULL)
      pItem = (CGEOTree_Item *) ItemList.get_head();
   else
      pItem = (CGEOTree_Item *) pParentItem->ChildItemList.get_head();

   while (pItem)
   {
      if ((hItem = FilteredInsertItemOnTree(pItem, pParentItem, hParent, PrevHItem)) != NULL) // elemento non filtrato
         PrevHItem = hItem;
      pItem = (CGEOTree_Item *) pItem->get_next();
   }

   if (hParent == TVI_ROOT && DisplayEmptyClassSet == false)
      deleteEmptyClassSet();

   OnRefresh = false;

   return GS_GOOD;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::deleteEmptyClassSet      <internal> */
/*+
  Questa funzione rimuove i class set dell'albero se sono vuoti.
  Parametri:
  HTREEITEM hItem           uso interno (default = NULL)

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
void CGEOTreeCtrl::deleteEmptyClassSet(HTREEITEM hItem)
{
   CGEOTree_Item *pItem;
   HTREEITEM hChildItem, hNextSiblingItem;

   if (!hItem) hItem = GetRootItem();
   if (!hItem) return;

   do
   {
      if (!(pItem = (CGEOTree_Item *) GetItemData(hItem))) return;

      hChildItem = GetChildItem(hItem);

      if (hChildItem) // Se ha figli
         deleteEmptyClassSet(hChildItem); // cerco nei figli

      hNextSiblingItem = GetNextSiblingItem(hItem);

      // riverifico se anche ora è senza figli (set di classi figlio di un set di classi)
      hChildItem = GetChildItem(hItem);
      if (pItem->ObjectType == GSClassSet && hChildItem == NULL) // Se è un set di classi senza ha figli
         DeleteItem(hItem);
   }
   // cerco nei fratelli
   while ((hItem = hNextSiblingItem)); // fratello successivo

   return;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::FilteredInsertItemOnTree <external> */
/*+
  Questa funzione inserisce un elemento nell'albero applicando
  le impostazioni di visualizzazione.
  Parametri:
  CGEOTree_Item *pItem;       elemento da inserire
  CGEOTree_Item *pParentItem; elemento padre
  HTREEITEM hParent           elemento CTreeCtrl padre
  HTREEITEM hInsertAfterItem; elemento CTreeCtrl fratello precedente

  Restituisce l'elemento inserito HTREEITEM in caso di successo, altrimenti NULL.
-*/  
/*********************************************************/
HTREEITEM CGEOTreeCtrl::FilteredInsertItemOnTree(CGEOTree_Item *pItem, CGEOTree_Item *pParentItem,
                                                 HTREEITEM hParent, HTREEITEM hInsertAfterItem)
{
   HTREEITEM hItem;
   C_STRING  ItemName;

   // Filtro
   if (Filter(pItem) == NULL) return NULL;

   ItemName = pItem->get_name();
   if ((pItem->ObjectType == GSProject && PrjPermissionVisibility) ||
       (pItem->ObjectType == GSClass && ClassPermissionVisibility) ||
       (pItem->ObjectType == GSSecondaryTab && SecPermissionVisibility))
   {
      C_STRING dummy;

      GetPermissionMsg(pItem->updatable, dummy);
      ItemName += dummy;
   }

   // se non è indicato hInsertAfterItem allora inserisco all'inizio
   if ((hItem = InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM,
                           ItemName.get_name(),
                           pItem->image,
                           pItem->image,
                           (pItem->selected && !MultiSelect) ? TVIS_SELECTED : 0,
                           (pItem->selected && !MultiSelect) ? TVIS_SELECTED : 0,
                           (LPARAM) pItem,
                           hParent,
                           (hInsertAfterItem) ? hInsertAfterItem : TVI_FIRST)) == NULL)
      return NULL;

   if (MultiSelect)
      SetCheck(hItem, pItem->selected);

   if (Refresh(pItem, hItem) != GS_GOOD) return NULL;

   return hItem;
}

int CGEOTreeCtrl::UpdateItemOnTree(CGEOTree_Item *pItem, HTREEITEM hItem)
{
   C_STRING ItemName = pItem->get_name();
   if ((pItem->ObjectType == GSProject && PrjPermissionVisibility) ||
       (pItem->ObjectType == GSClass && ClassPermissionVisibility) ||
       (pItem->ObjectType == GSSecondaryTab && SecPermissionVisibility))
   {
      C_STRING dummy;

      GetPermissionMsg(pItem->updatable, dummy);
      ItemName += dummy;
   }

	if (SetItemText(hItem, ItemName.get_name()) == NULL) return GS_BAD;
	if (SetItemImage(hItem, pItem->image, pItem->image) == NULL) return GS_BAD;

   if (MultiSelect)
      SetCheck(hItem, pItem->selected);

   return GS_GOOD;
}

/*********************************************************/
/*.doc CGEOTreeCtrl::Reset_Tree               <external> */
/*+
  Questa funzione svuota l'albero e lo imposta correttamente.
-*/  
/*********************************************************/
void CGEOTreeCtrl::Reset_Tree(void)
{
   // svuoto l'albero
   DeleteAllItems();
	if (MultiSelect)
      ModifyStyle(0, TVS_CHECKBOXES | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS);
	else
      ModifyStyle(TVS_CHECKBOXES, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS);

   SetImageList(&ImageList, TVSIL_NORMAL);
}


/*********************************************************/
/*.doc CGEOTreeCtrl::Filter                   <external> */
/*+
  Questa funzione filtra un elemento in base alle regole attive.
  Parametri:
  CGEOList_Item *pItem;

  Restituisce true in caso di successo altrimenti false.
-*/  
/*********************************************************/
bool CGEOTreeCtrl::Filter(CGEOTree_Item *pItem)
{
   if (pItem->ObjectType == GSClassSet)
      if (DisplayEmptyClassSet)
         return true;
      else
         return (pItem->ChildItemList.get_count() == 0) ? false : true;

   // Se esiste un filtro per codice progetto
   if (FilterOnCodes.get_count() > 0)
   {
      C_FAMILY *pFamily;
      int      cls, sub, sec;

      cls = pItem->get_cls();
      sub = pItem->get_sub();
      sec = pItem->get_sec();

      if ((pFamily = (C_FAMILY *) FilterOnCodes.search_key(pItem->get_prj())) == NULL) return false;

      // Se esiste un filtro per codice classe
      if (cls > 0 && pFamily->relation.get_count() > 0)
      {
         if ((pFamily = (C_FAMILY *) pFamily->relation.search_key(cls)) == NULL) return false;
         // Se esiste un filtro per sottoclasse
         if (sub > 0 && pFamily->relation.get_count() > 0)
            if ((pFamily = (C_FAMILY *) pFamily->relation.search_key(sub)) == NULL) return false;
         // Se esiste un filtro per secondaria
         if (sec > 0 && pFamily->relation.get_count() > 0)
            if ((pFamily = (C_FAMILY *) pFamily->relation.search_key(sec)) == NULL) return false;
      }
   }

   // Filtro per estratta nella sessione corrente
   if (FilterOnExtracted)
      if (pItem->extracted == false)
         return false;

   // Filtro per modificabilità (tranne che per set di classi)
   if (FilterOnUpdateable)
      if (pItem->ObjectType != GSClassSet)
         if (pItem->updatable != GSUpdateableData)
            return false;

   // Filtro per categoria e tipo x classi o sottoclassi
   if (FilterOnClassCategoryTypeList.get_count() > 0 && 
       (pItem->ObjectType == GSClass || pItem->ObjectType == GSSubClass))
   {
      // se si tratta di simulazioni
      if (pItem->ObjectType == GSClass && pItem->get_category() == CAT_EXTERN)
      { // cerco solo la categoria
         if (FilterOnClassCategoryTypeList.search_key(pItem->get_category()) == NULL)
            return false;
      }
      else // cerco la categoria e il tipo
         if (FilterOnClassCategoryTypeList.search(pItem->get_category(), pItem->get_type()) == NULL)
            return false;
   }

   // Filtro per tipo secondaria
   if (pItem->ObjectType == GSSecondaryTab && FilterOnSecondaryTabType.get_count() > 0)
      if (FilterOnSecondaryTabType.search_key(pItem->get_type()) == NULL)
         return false;

   return true;
}

BOOL CGEOTreeCtrl::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
   NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW*)pNMHDR;

   // Solo se non sono in un refresh del controllo e senza la multiselezione
   if (!OnRefresh && !MultiSelect)
   {
      CGEOTree_Item *pItem = (CGEOTree_Item *) pNMTreeView->itemOld.lParam;
      if (pItem) OnSelChanged(pItem, (pNMTreeView->itemOld.state & TVIS_SELECTED) ? true : false);
      pItem = (CGEOTree_Item *) pNMTreeView->itemNew.lParam;
      if (pItem) OnSelChanged(pItem, (pNMTreeView->itemNew.state & TVIS_SELECTED) ? true : false);
   }
   *pResult = 0;

   // If you use ON_NOTIFY_REFLECT_EX() in your message map, your message handler may or may not allow
   // the parent window to handle the message. If the handler returns FALSE, the message will be handled
   // by the parent as well
   return FALSE;
}


/*********************************************************/
/*.doc CGEOTreeCtrl::OnSelChanged             <external> */
/*+
  Questa funzione setta lo stato di selezione interna di un elemento.
-*/  
/*********************************************************/
void CGEOTreeCtrl::OnSelChanged(CGEOTree_Item *pItem, bool selected)
{
   HTREEITEM hItem = FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);

   if (!hItem) return; // se non era nell'albero

   pItem->selected = selected;

   if (MultiSelect)
   {
      SelChildItems(hItem); // tratto anche i figli
      SelParentItems(hItem); // tratto anche il padre

      // Se è abilitata la selezione multipla di classi e si sta selezionando una classe gruppo
      // e si vuole che le classi membro ereditino la selezione della classe 
      if (FinalObjectType == GSClass &&
          pItem->ObjectType == GSClass && pItem->get_category() == CAT_GROUP && 
          SelectedLinkedClass)
      {
         int     prj = pItem->get_prj();
		   C_CLASS *pCls = gsc_find_class(prj, pItem->get_cls());
                        
		   if (pCls)
		   {
            C_GROUP_LIST *pGroupList = (C_GROUP_LIST *) pCls->ptr_group_list();
			
            if (pGroupList)	// verifico che le classi che compongono il gruppo siano in lista
            {
               C_INT_INT *pGroup = (C_INT_INT*) pGroupList->get_head();
						
               while (pGroup)
				   {
					   if ((pItem = ItemList.search_cls(prj, pGroup->get_key())))
                  {
                     if ((hItem = FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem)))
                        SetCheck(hItem, selected);
                     OnSelChanged(pItem, selected);
                  }
					   pGroup = (C_INT_INT*) pGroup->get_next();
				   }
            }
         }
      }
   }
}


BOOL CGEOTreeCtrl::OnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
   if (!MultiSelect) return FALSE;

   CPoint pt;
   UINT   uFlags;

   GetCursorPos(&pt);
   ScreenToClient(&pt);
   HTREEITEM hItem = HitTest(pt, &uFlags);
      
   if (hItem)
   {
      // There is an item under the cursor.
      SelectItem(hItem);

      if (uFlags & TVHT_ONITEMSTATEICON)
      {
         // It is the icon (checkbox)
         CGEOTree_Item *pItem = (CGEOTree_Item *) GetItemData(hItem);
         // questo evento avviene prima che la checkbox cambi quindi ho invertito la condizione
         if (pItem) OnSelChanged(pItem, GetCheck(hItem) ? false : true);
         //SetBoldItem(hItem, false);
      }
   }

   *pResult = 0;

   // If you use ON_NOTIFY_REFLECT_EX() in your message map, your message handler may or may not allow
   // the parent window to handle the message. If the handler returns FALSE, the message will be handled
   // by the parent as well
   return FALSE;
}

void CGEOTreeCtrl::OnRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
   CPoint pt;
   UINT   uFlags;

   // il bottone destro del mouse non seleziona l'item quindi devo vedere la posizione del mouse
   GetCursorPos(&pt);
   ScreenToClient(&pt);
   m_SelectedItemOnRClick = HitTest(pt, &uFlags);

	// Send WM_CONTEXTMENU to self
	SendMessage(WM_CONTEXTMENU, (WPARAM) m_hWnd, GetMessagePos());
	// Mark message as handled and suppress default handling
	*pResult = 1;
}

void CGEOTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);

   switch (nChar)
   {
   case VK_SPACE: // barra spaziatrice
      {
         HTREEITEM hItem = GetSelectedItem();
         CGEOTree_Item *pItem = (CGEOTree_Item *) GetItemData(hItem);   
   
         if (pItem) OnSelChanged(pItem, GetCheck(hItem) ? true : false);
         //SetBoldItem(hItem, false);
         break;
      }
      case VK_F3: // F3
      {
         if (SearchByName.len() == 0) break;
         HTREEITEM hItem = FindItem(GetSelectedItem(), SearchByName.get_name(), true);

         if (hItem)
         {
            SelectSetFirstVisible(hItem);
            SelectItem(hItem);
         }
         else
         {
            C_STRING Msg(_T("Impossibile trovare il testo: "));
            Msg += SearchByName;
            gsc_ddalert(Msg.get_name(), m_hWnd);
         }
         break;
      }
   }
}


/*********************************************************/
/*.doc CGEOTreeCtrl::SelChildItems             <external> */
/*+
  Questa funzione seleziona o deseleziona gli elementi gerarchicamente 
  più in alto o uguali al tipo 
  impostato da FinalObjectType che sono figli dell'elemento hItem.
  Parametri:
  HTREEITEM hItem;

  Restituisce il numero elementi di tipo FinalObjectType sono selezionati, -1 in caso di errore.
-*/  
/*********************************************************/
int CGEOTreeCtrl::SelChildItems(HTREEITEM hItem)
{
   HTREEITEM     hNextItem;
   CGEOTree_Item *pItem = (CGEOTree_Item *) GetItemData(hItem);
   int           n = 0;
   bool          selected = pItem->selected;

   if ((hNextItem = GetChildItem(hItem)) == NULL) return 0; // Se non ha figli
   
   if ((pItem = (CGEOTree_Item *) GetItemData(hNextItem)) && pItem->ObjectType <= FinalObjectType)
   {
      SetCheck(hNextItem, selected);
      pItem->selected = selected;
      if (selected && pItem->ObjectType == FinalObjectType) n++; // roby 2016
      if (pItem->ObjectType < FinalObjectType) n += SelChildItems(hNextItem); // roby 2016
   }

   // Ciclo sui fratelli
   while ((hNextItem = GetNextSiblingItem(hNextItem))) // fratello successivo
   {
      if ((pItem = (CGEOTree_Item *) GetItemData(hNextItem)) && pItem->ObjectType <= FinalObjectType)
      {
         SetCheck(hNextItem, selected);
         pItem->selected = selected;
         if (selected && pItem->ObjectType == FinalObjectType) n++; // roby 2016
         if (pItem->ObjectType < FinalObjectType) n += SelChildItems(hNextItem); // roby 2016
      }
   }

   if ((pItem = (CGEOTree_Item *) GetItemData(hItem)))
   {
      pItem->n_selected = n;
      UpdItemTextOnSelected(hItem); // roby 2016
   }

   return n;
}


/***********************************************************/
/*.doc CGEOTreeCtrl::SelParentItems             <external> */
/*+
  Questa funzione seleziona o deseleziona il/i padre/i se l'elemento
  hItem e tutti i suoi fratelli sono selezionati altrimenti li deseleziona.
  Parametri:
  HTREEITEM hItem;
-*/  
/*********************************************************/
void CGEOTreeCtrl::SelParentItems(HTREEITEM hItem) // roby 2016
{
   HTREEITEM     hSiblingItem, hParentItem;
   CGEOTree_Item *pItem = (CGEOTree_Item *) GetItemData(hItem), *pParentItem;
   int           n = 0;
   bool          notAllSelected = false;

   if (!(hParentItem = GetParentItem(hItem)) ||
       !(pParentItem = (CGEOTree_Item *) GetItemData(hParentItem)))
      return; // Se non ha un parente

   // verifico se tutti i figli sono selezionati
   hSiblingItem = GetChildItem(hParentItem);
   while (hSiblingItem)
   {
      pItem = (CGEOTree_Item *) GetItemData(hSiblingItem);
      if (!pItem->selected) notAllSelected = true;

      if (pItem->ObjectType == FinalObjectType)
      {
         if (pItem->selected) n++; // roby 2016
      }
      else
         n += pItem->n_selected;

      hSiblingItem = GetNextSiblingItem(hSiblingItem); // fratello successivo
   }

   SetCheck(hParentItem, !notAllSelected);
   pParentItem->selected = !notAllSelected;
   pParentItem->n_selected = n;
   UpdItemTextOnSelected(hParentItem); // roby 2016

   return SelParentItems(hParentItem);
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::GetSelected                                  <external> */
/*+
  Questa funzione ritorna il codice dell'oggetto selezionato.
  Funzione valida solo se MultiSelect = false.
  Parametri:
  GSDataModelObjectTypeEnum _ObjectType;  Tipo di oggetto
-*/  
/*****************************************************************************/
int CGEOTreeCtrl::GetSelected(GSDataModelObjectTypeEnum _ObjectType)
{
   if (MultiSelect) return 0;
   HTREEITEM hItem = GetSelectedItem();
   if (!hItem) return 0;

   CGEOTree_Item *pItem = (CGEOTree_Item *) GetItemData(hItem);
   while (pItem && pItem->ObjectType != _ObjectType)
      pItem = pItem->pParentItem;

   return (pItem) ? pItem->get_key() : 0;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::SetSelectedPrj                               <external> */
/*+
  Questa funzione seleziona/deseleziona il progetto selezionato.
  Parametri:
  int _prj;       codice progetto
  bool selected;
-*/  
/*****************************************************************************/
void CGEOTreeCtrl::SetSelectedPrj(int _prj, bool selected)
{
   CGEOTree_Item *pItem = (CGEOTree_Item *) ItemList.search_key(_prj);

   if (!pItem) return;
   if (pItem->ObjectType != GSProject) return;
   
   if (MultiSelect) 
   {
      HTREEITEM hItem = FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);
      if (!hItem) return; // se non era nell'albero
      SetCheck(hItem, selected);
   }

   return OnSelChanged(pItem, selected);
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::SetSelectedCls                               <external> */
/*+
  Questa funzione imposta la classe selezionato.
  Parametri:
  int _prj;       codice progetto
  int _cls;       codice classe
  bool selected;
-*/
/*****************************************************************************/
void CGEOTreeCtrl::SetSelectedCls(int _prj, int _cls, bool selected)
{
   CGEOTree_Item *pItem = (CGEOTree_Item *) ItemList.search_cls(_prj, _cls);

   if (!pItem) return;

   HTREEITEM hItem = FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);
   if (!hItem) return; // se non era nell'albero
   // Ensure the item is visible.
   EnsureVisible(hItem);
   
   Select(hItem, TVGN_CARET); // Sets the selection to the specified item

   if (MultiSelect) SetCheck(hItem, selected);

   return OnSelChanged(pItem, selected);
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::SetSelectedSub                               <external> */
/*+
  Questa funzione imposta la sotto-classe selezionato.
  Parametri:
  int _prj;       codice progetto
  int _cls;       codice classe
  int _sub;       codice sotto-classe
  bool selected;
-*/
/*****************************************************************************/
void CGEOTreeCtrl::SetSelectedSub(int _prj, int _cls, int _sub, bool selected)
{
   CGEOTree_Item *pItem = (CGEOTree_Item *) ItemList.search_cls(_prj, _cls);

   if (!pItem) return;
   if (!(pItem = (CGEOTree_Item *) pItem->ChildItemList.search_key(_sub))) return;
   if (pItem->ObjectType != GSSubClass) return;
   
   HTREEITEM hItem = FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);
   if (!hItem) return; // se non era nell'albero
   // Ensure the item is visible.
   EnsureVisible(hItem);

   Select(hItem, TVGN_CARET); // Sets the selection to the specified item

   if (MultiSelect) SetCheck(hItem, selected);

   return OnSelChanged(pItem, selected);
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::SetSelectedSec                               <external> */
/*+
  Questa funzione imposta la tabella secondaria selezionato.
  Parametri:
  int _prj;       codice progetto
  int _cls;       codice classe
  int _sub;       codice sotto-classe (0 se non usato)
  int _sec;       codice tabella secondaria
  bool selected;
-*/
/*****************************************************************************/
void CGEOTreeCtrl::SetSelectedSec(int _prj, int _cls, int _sub, int _sec, bool selected)
{
   CGEOTree_Item *pItem = (CGEOTree_Item *) ItemList.search_cls(_prj, _cls);

   if (!pItem) return;
   if (_sub > 0)
      if (!(pItem = (CGEOTree_Item *) pItem->ChildItemList.search_key(_sub))) return;
   if (!(pItem = (CGEOTree_Item *) pItem->ChildItemList.search_key(_sec))) return;
   if (pItem->ObjectType != GSSecondaryTab) return;

   HTREEITEM hItem = FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);
   if (!hItem) return; // se non era nell'albero
   // Ensure the item is visible.
   EnsureVisible(hItem);

   Select(hItem, TVGN_CARET); // Sets the selection to the specified item

   if (MultiSelect) SetCheck(hItem, selected);

   return OnSelChanged(pItem, selected);
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::get_ItemList_ptr                               <external> */
/*+
  Questa funzione restituisce il puntatore alla lista degli elementi del controllo.
-*/
/*****************************************************************************/
CGEOTree_ItemList* CGEOTreeCtrl::get_ItemList_ptr(void)
{
   return &ItemList;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::get_ImageList_ptr                            <external> */
/*+
  Questa funzione restituisce il puntatore alla lista delle immagini del controllo.
-*/
/*****************************************************************************/
CImageList* CGEOTreeCtrl::get_ImageList_ptr(void)
{
   return &ImageList;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::SetSelectedCodes                             <external> */
/*+
  Questa funzione imposta gli elementi con codici noti. I codici si riferiscono
  ad oggetti del tipo indicato dal membro FinalObjectType. Saranno considerati
  i primi parenti (es. se FinalObjectType=GSClass i codici delle classi 
  saranno riferiti al primo progetto dell'albero)
  Parametri:
  C_INT_LIST &SelectedCodes;  Lista di codici
  bool       Selected;        valore da impostare per le classi indicate
-*/
/*****************************************************************************/
void CGEOTreeCtrl::SetSelectedCodes(C_INT_LIST &SelectedCodes, bool Selected)
{
   CGEOTree_ItemList *pItemList;
   C_INT             *pSelectedCode;
   
   DeselectAll(); // deseleziono tutto

   pItemList = &ItemList;

   if (FinalObjectType == GSClass || FinalObjectType == GSSubClass || FinalObjectType == GSSecondaryTab)
   {
      CGEOTree_Item     *pItem;
      int       prj, cls;
      C_FAMILY *p = (C_FAMILY * ) FilterOnCodes.get_head();

      if (!p) return;
      prj = p->get_key();
      pItem = (CGEOTree_Item *) pItemList->get_head(); // primo progetto
      if (!pItem) return;
      pItemList = &pItem->ChildItemList;
      if (FinalObjectType == GSSubClass || FinalObjectType == GSSecondaryTab)
      {
         if (!(p = (C_FAMILY * ) p->relation.get_head())) return; // prima classe
         cls = p->get_key();
         pItem = pItemList->search_cls(prj, cls);
         if (!pItem) return;
         pItemList = &pItem->ChildItemList;
         if (FinalObjectType == GSSecondaryTab && pItem->ObjectType == GSSubClass)
         {
            pItem = (CGEOTree_Item *) pItemList->get_head(); // prima sotto-classe
            if (!pItem) return;
            pItemList = &pItem->ChildItemList;
         }
      }
   }

   pSelectedCode = (C_INT *) SelectedCodes.get_head();
   while (pSelectedCode)
   {
      SetSelectedCode(pSelectedCode->get_key(), Selected, pItemList);

      pSelectedCode = (C_INT *) pSelectedCode->get_next();
   }

   return;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::SetSelectedCode                             <external> */
/*+
  Questa funzione imposta l'elemento con codice noto. Il codice si riferisce
  ad oggetto del tipo indicato dal membro FinalObjectType. Saranno considerati
  i primi parenti (es. se FinalObjectType=GSClass il codice della classe 
  sarà riferito al primo progetto dell'albero)
  Parametri:
  int  SelectedCode;  codici
  bool Selected;      valore da impostare per la classe indicata
  CGEOTree_ItemList *_pItemList; uso interno
-*/
/*****************************************************************************/
void CGEOTreeCtrl::SetSelectedCode(int SelectedCode, bool Selected, CGEOTree_ItemList *_pItemList)
{
   CGEOTree_Item     *pItem;
   CGEOTree_ItemList *pItemList;
   HTREEITEM         hItem;
   C_FAMILY          *p;
   int               prj, cls;

   if (FinalObjectType == GSClass || FinalObjectType == GSSubClass || FinalObjectType == GSSecondaryTab)
      if ((p = (C_FAMILY * ) FilterOnCodes.get_head()) == NULL)
         return;
      else
         prj = p->get_key();

   // se _pItemList non viene passato come parametro me lo ricavo
   if (_pItemList == NULL)
   {
      pItemList = &ItemList;

      if (FinalObjectType == GSClass || FinalObjectType == GSSubClass || FinalObjectType == GSSecondaryTab)
      {
         pItem = (CGEOTree_Item *) pItemList->get_head(); // primo progetto
         if (!pItem) return;
         pItemList = &pItem->ChildItemList;
         if (FinalObjectType == GSSubClass || FinalObjectType == GSSecondaryTab)
         {
            if (!(p = (C_FAMILY * ) p->relation.get_head())) return; // prima classe
            cls = p->get_key();
            pItem = pItemList->search_cls(prj, cls);
            if (!pItem) return;
            pItemList = &pItem->ChildItemList;
            if (FinalObjectType == GSSecondaryTab && pItem->ObjectType == GSSubClass)
            {
               pItem = (CGEOTree_Item *) pItemList->get_head(); // prima sotto-classe
               if (!pItem) return;
               pItemList = &pItem->ChildItemList;
            }
         }
      }
   }
   else
   {
      pItemList = _pItemList;
      pItem = (CGEOTree_Item *) pItemList->get_head(); // primo progetto
   }

   if (FinalObjectType == GSClass)
      pItem = pItemList->search_cls(prj, SelectedCode);
   else
      pItem = (CGEOTree_Item *) pItemList->search_key(SelectedCode);

   if (pItem)
      if ((hItem = FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem)))
      {
         SetCheck(hItem, Selected);
         OnSelChanged(pItem, Selected);
      }

   return;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::DeselectAll                                  <external> */
/*+
  Questa funzione deseleziona tutti gli elementi.
-*/
/*****************************************************************************/
void CGEOTreeCtrl::DeselectAll(void)
{
   HTREEITEM     hItem = GetRootItem();
   CGEOTree_Item *pItem;

   while (hItem) // ciclo sui progetti
   {
      SetCheck(hItem, false);
      if ((pItem = (CGEOTree_Item *) GetItemData(hItem))) OnSelChanged(pItem, false);
      hItem = GetNextSiblingItem(hItem); // fratello successivo
   }

   return;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::ExpandAll                                    <external> */
/*+
  Questa funzione espande tutti rami dell'albero.
-*/
/*****************************************************************************/
void CGEOTreeCtrl::ExpandAll(HTREEITEM hItem)
{
   if (!hItem) hItem = GetRootItem();

   while (hItem)
   {
      Expand(hItem, TVE_EXPAND);
      if (GetChildItem(hItem)) ExpandAll(GetChildItem(hItem));
      hItem = GetNextSiblingItem(hItem); // fratello successivo
   }

   return;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::expand_prj_clsSet_list                       <external> */
/*+
  Questa funzione espande i nodi relativi a progetti e set di visibilità contenuti
  nella lista dell'albero.
  Parametri:
  C_INT_INT_LIST &ClsSetList; Lista di coppie <progetto> e <set di classe> espansi
                              se il <set di classe> = 0 significa progetto espanso
  HTREEITEM      hItem;       Uso interno Handle del nodo dell'albero (NULL la prima volta)
  int            prj;         Uso Interno
-*/
/*****************************************************************************/
void CGEOTreeCtrl::expand_prj_clsSet_list(C_INT_INT_LIST &prj_clsSet_List, HTREEITEM hItem, int prj)
{
   HTREEITEM     hChildItem;
   CGEOTree_Item *pItem;

   if (!hItem) hItem = GetRootItem();

   while (hItem)
   {
      if (!(pItem = (CGEOTree_Item *) GetItemData(hItem))) return;
      
      switch (pItem->ObjectType)
      {
         case GSProject: // se l'elemento è un progetto
            prj = pItem->get_key();
            if (prj_clsSet_List.search(prj, 0)) // se è espanso
               Expand(hItem, TVE_EXPAND);
            break;
         case GSClassSet: // se l'elemento è un set di classi
            if (prj_clsSet_List.search(prj, pItem->get_key())) // se è espanso
               Expand(hItem, TVE_EXPAND);
      }
      if ((hChildItem = GetChildItem(hItem)))
         expand_prj_clsSet_list(prj_clsSet_List, hChildItem, prj);
      hItem = GetNextSiblingItem(hItem); // fratello successivo
   }

   return;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::getExpanded_prj_clsSet_list                  <external> */
/*+
  Questa funzione ottiene una lista dei nodi progetto e set di classi dell'albero espansi (aperti).
  Parametri:
  C_INT_INT_LIST &ClsSetList; Lista di coppie <progetto> e <set di classe> espansi
                              se il <set di classe> = 0 significa progetto espanso
  HTREEITEM      hItem;       Uso interno Handle del nodo dell'albero (NULL la prima volta)
  int            prj;         Uso Interno
-*/
/*****************************************************************************/
void CGEOTreeCtrl::getExpanded_prj_clsSet_list(C_INT_INT_LIST &prj_clsSet_List, HTREEITEM hItem, int prj)
{
   HTREEITEM      hChildItem;
   CGEOTree_Item *pItem;

   if (!hItem)
   {
      hItem = GetRootItem();
      prj_clsSet_List.remove_all();
   }

   while (hItem)
   {
      if (!(pItem = (CGEOTree_Item *) GetItemData(hItem))) return;
      
      switch (pItem->ObjectType)
      {
         case GSProject: // se l'elemento è un progetto
            prj = pItem->get_key();
            if (GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED) // se è espanso
               prj_clsSet_List.values_add_tail(prj, 0);
            break;
         case GSClassSet: // se l'elemento è un set di classi
            if (GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED) // se è espanso
               prj_clsSet_List.values_add_tail(prj, pItem->get_key());
      }
      if ((hChildItem = GetChildItem(hItem)))
         getExpanded_prj_clsSet_list(prj_clsSet_List, hChildItem, prj);
      hItem = GetNextSiblingItem(hItem); // fratello successivo
   }

   return;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::DeleteChildItems                             <external> */
/*+
  Questa funzione cancella dall'albero i figli dell'item indicato.
-*/
/*****************************************************************************/
BOOL CGEOTreeCtrl::DeleteChildItems(HTREEITEM hItem)
{
   HTREEITEM hChildItem;

   while ((hChildItem = GetChildItem(hItem)))
      if (DeleteItem(hChildItem) == FALSE) return FALSE;

   return TRUE;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::GetSelectedCodes                               <external> */
/*+
  Questa funzione ricava gli elementi selezionati o non selezionati.
  I codici si riferiscono ad oggetti del tipo indicato dal membro FinalObjectType.
  Saranno considerati i primi parenti (es. se FinalObjectType=GSClass i codici 
  delle classi sarano riferiti al primo progetto dell'albero)
  Parametri:
  C_INT_LIST &SelectedCodes;     lista di codici
  bool       Selected;           Flag di ricerca, se = true cerca i codici degli
                                 elementi selezionati altrimenti di quelli non selezionati
                                 (default = true)
  HTREEITEM  hItem;              uso interno
-*/
/*****************************************************************************/
void CGEOTreeCtrl::GetSelectedCodes(C_INT_LIST &SelectedCodes, bool Selected, HTREEITEM hItem)
{
   CGEOTree_Item *pItem;

   if (!hItem)
   {
      hItem = GetRootItem();
      SelectedCodes.remove_all();
   }

   if (!(pItem = (CGEOTree_Item *) GetItemData(hItem))) return;

   switch (FinalObjectType)
   {
      case GSClass: case GSSubClass: // se si devono cercare classi o sottoclassi
         if (pItem->ObjectType == GSProject) // se l'elemento è un progetto
            if ((hItem = GetChildItem(hItem)) == NULL) // Se non ha figli
               return;
            else
               return GetSelectedCodes(SelectedCodes, Selected, hItem); // cerco nei figli del primo progetto
         break;
      case GSSecondaryTab: // se si devono cercare tabelle secondarie
         // se l'elemento è un progetto o una classe o una sottoclasse
         if (pItem->ObjectType == GSProject ||
             pItem->ObjectType == GSClass || pItem->ObjectType == GSSubClass)
            if ((hItem = GetChildItem(hItem)) == NULL) // Se non ha figli
               return;
            else
               return GetSelectedCodes(SelectedCodes, Selected, hItem); // cerco nei figli del primo progetto o classe o sottoclasse
         break;
   }

   while (pItem)
   {
      if (pItem->ObjectType == GSClassSet) // se l'elemento è un set di classi
      {
         if (GetChildItem(hItem)) // Se ha figli
            GetSelectedCodes(SelectedCodes, Selected, GetChildItem(hItem)); // cerco nelle classi del set di classi
      }
      else
         if (pItem->selected == Selected) SelectedCodes.add_tail_int(pItem->get_key());

      if (!(hItem = GetNextSiblingItem(hItem))) // fratello successivo
         pItem = NULL;
      else
         if (!(pItem = (CGEOTree_Item *) GetItemData(hItem))) return;
   }

   return;
}


void CGEOTreeCtrl::SetBoldItem(HTREEITEM hItem, bool bold)
{
   TV_ITEM tvi;
   tvi.mask = TVIF_STATE | TVIF_HANDLE;
   tvi.hItem = hItem;
   tvi.state = (bold) ? TVIS_BOLD : 0;
   tvi.stateMask = TVIS_BOLD;
   SetItem(&tvi);
}


void CGEOTreeCtrl::GetPermissionMsg(GSDataPermissionTypeEnum permission, C_STRING &Msg)
{
   Msg.clear();

   switch (permission)
   {
      case GSNonePermission:
         Msg = _T(" (Ereditato)");
         break;
      case GSInvisibleData:
         Msg = _T(" (Invisibile)");
         break;
      case GSReadOnlyData:
         Msg = _T(" (Sola lettura)");
         break;
      case GSUpdateableData:
         Msg = _T(" (Modifica)");
         break;
   }
}


void CGEOTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
   CMenu Menu;
   UINT  nFlags;
	HTREEITEM htItem;

   // if Shift-F10
	if (point.x == -1 && point.y == -1)
		point = (CPoint) GetMessagePos();

	ScreenToClient(&point);
	
	htItem = HitTest(point, &nFlags);
	if (htItem == NULL) return;

   // Create a new menu for the application window.
   if (Menu.CreateMenu() == FALSE) return;

   // Create a "File" popup menu and insert this popup menu to the menu
   if (Menu.CreatePopupMenu() == FALSE) return;

   // se ricerca per nome impostata
   if (Menu.AppendMenu(MF_STRING, ID_GEOTREECTRL_SEARCH_BY_NAME_MENU, _T("Cerca per &Nome...")) == FALSE) return;
   if (Menu.AppendMenu(MF_SEPARATOR) == FALSE) return; // Separatore

 	if (MultiSelect)
   {
      if (FinalObjectType == GSClass || FinalObjectType == GSSubClass)
      {
         if (Menu.AppendMenu(MF_STRING, ID_GEOTREECTRL_LOAD_SEL_MENU, _T("&Carica selezione...")) == FALSE) return;
         if (Menu.AppendMenu(MF_STRING, ID_GEOTREECTRL_SAVE_SEL_MENU, _T("&Salva selezione...")) == FALSE) return;     
         if (Menu.AppendMenu(MF_SEPARATOR) == FALSE) return; // Separatore
      }
      if (Menu.AppendMenu(MF_STRING, ID_GEOTREECTRL_INVERT_SEL_MENU, _T("&Inverti selezione")) == FALSE) return;     
      if (Menu.AppendMenu(MF_SEPARATOR) == FALSE) return; // Separatore
   }

 	if (CutAndPaste)
   {
      if (Menu.AppendMenu(MF_STRING, ID_GEOTREECTRL_CUT_MENU, _T("&Taglia")) == FALSE) return;     
      if (m_CutAndPasteItem)
         if (Menu.AppendMenu(MF_STRING, ID_GEOTREECTRL_PASTE_MENU, _T("&Incolla")) == FALSE) return;     
      if (Menu.AppendMenu(MF_SEPARATOR) == FALSE) return; // Separatore
   }
   if (Menu.AppendMenu(MF_STRING, ID_GEOTREECTRL_REFRESH_MENU, _T("&Aggiorna")) == FALSE) return;

	// Draw and track the "floating" menu. 
   ClientToScreen(&point);
 	TrackPopupMenu(Menu.m_hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON,	
                  point.x, point.y, 0, pWnd->m_hWnd, NULL);
}

void CGEOTreeCtrl::OnSearchByNameMenu(void)
{
   if (gsc_ddinput(_T("Cerca per nome:"), SearchByName, SearchByName.get_name(), FALSE, FALSE) == GS_GOOD &&
       SearchByName.len() > 0)
   {
      HTREEITEM hItem = FindItem(GetSelectedItem(), SearchByName.get_name(), true);

      if (hItem)
      {
         SelectSetFirstVisible(hItem);
         SelectItem(hItem);
      }
      else
      {
         C_STRING Msg(_T("Impossibile trovare il testo: "));
         Msg += SearchByName;
         gsc_ddalert(Msg.get_name(), m_hWnd);
      }
   }
}

void CGEOTreeCtrl::OnLoadSelMenu(void) 
{
   if (FinalObjectType == GSClass)
   {
      CGEOTree_Item *p = (CGEOTree_Item *) ItemList.get_head(); // primo progetto

      if (!p) return;

      C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(p->get_key());

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
               C_INT_LIST SelectedCodes;
               C_STR      *p = (C_STR *) list.get_head();

               // Il primo elemento è il progetto che deve essere uguale a quello impostato
               if (_wtoi(p->get_name()) != pPrj->get_key()) return;

               p = (C_STR *) p->get_next();
               while (p)
               {
                  SelectedCodes.add_tail_int(_wtoi(p->get_name()));
                  p = (C_STR *) p->get_next();
               }

               SetSelectedCodes(SelectedCodes, true);
            }
         }
      }
   }
}

void CGEOTreeCtrl::OnSaveSelMenu(void)
{
   if (FinalObjectType == GSClass)
   {
      CGEOTree_Item *p = (CGEOTree_Item *) ItemList.get_head(); // primo progetto

      if (!p) return;

      C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(p->get_key());

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
            C_INT_LIST SelectedCodes;
            C_INT      *pSelectedCode;
            C_STR_LIST list;
            C_STRING   dummy;

            dummy = pPrj->get_key();
            list.add_tail_str(dummy.get_name());

            GetSelectedCodes(SelectedCodes);
            pSelectedCode = (C_INT *) SelectedCodes.get_head();
            while (pSelectedCode)
            {
               dummy = pSelectedCode->get_key();
               list.add_tail_str(dummy.get_name());

               pSelectedCode = (C_INT *) pSelectedCode->get_next();
            }

            list.save((LPCTSTR) FileDlg.GetPathName());
         }
      }
   }
}

void CGEOTreeCtrl::OnRefreshMenu(void) 
{
   C_INT_LIST SelectedCodes;

   GetSelectedCodes(SelectedCodes);
   LoadFromDB();
   Refresh();
   SetSelectedCodes(SelectedCodes);
}


void CGEOTreeCtrl::OnInvertSelMenu(void) 
{
   C_INT_LIST SelectedCodes, UnselectedCodes;

   GetSelectedCodes(SelectedCodes); // codici degli elementi selezionati
   GetSelectedCodes(UnselectedCodes, false); // codici degli elementi non selezionati
   DeselectAll(); // deseleziono tutto
   SetSelectedCodes(UnselectedCodes, true);
}

void CGEOTreeCtrl::OnCutMenu(void) 
{
   CGEOTree_Item *pItem;
   C_FAMILY      *p = (C_FAMILY * ) FilterOnCodes.get_head();

   if (m_SelectedItemOnRClick == NULL) return; // vedi rclick

   // deve essere impostato il filtro su un progetto 
   // (altrimenti una classe potrebbe essere spostata in un altro progetto)
   if (!p) return;
   // cut and paste ammesso solo per classi e set di classi
   if (FinalObjectType != GSClassSet && FinalObjectType != GSClass) return;
   if ((pItem = (CGEOTree_Item *) GetItemData(m_SelectedItemOnRClick)) == NULL) return;
   // cut and paste ammesso solo per classi e set di classi
   if (pItem->ObjectType != GSClass && pItem->ObjectType != GSClassSet) return;
   
   m_CutAndPasteItem = m_SelectedItemOnRClick;
   SetItemState(m_CutAndPasteItem, TVIS_CUT, TVIS_CUT);
}

void CGEOTreeCtrl::OnPasteMenu(void)
// ritorna true se ha modificato qualcosa
{
   if (!m_CutAndPasteItem || !m_SelectedItemOnRClick) return; // vedi rclick

   // Se l'item di destinazione è unn set di classi
   // chiedo se si vuole inserire dentro il set di classi
   // oppure come fratello del set
   CGEOTree_Item *pInsertAfterItem = (CGEOTree_Item *) GetItemData(m_SelectedItemOnRClick);

   switch (pInsertAfterItem->ObjectType)
   {
      case GSProject:
         // se si vuole inserire nel progetto
         MoveItem(m_CutAndPasteItem, m_SelectedItemOnRClick);
         break;
      case GSClassSet:
      {
         C_STRING Msg(_T("Inserire nel set di classi \""));
         Msg += pInsertAfterItem->get_name();
         Msg += _T("\" ?");
         
         if (MessageBox(Msg.get_name(), _T("GEOsim"), MB_ICONQUESTION + MB_YESNO + MB_DEFBUTTON1) == IDYES)
            // se si vuole inserire nel set
            MoveItem(m_CutAndPasteItem, m_SelectedItemOnRClick);
         else // si vuole inserire come fratello successivo del set
            MoveItem(m_CutAndPasteItem, GetParentItem(m_SelectedItemOnRClick), m_SelectedItemOnRClick);

         break;
      }
      case GSClass: // si vuole inserire come fratello successivo
         MoveItem(m_CutAndPasteItem, GetParentItem(m_SelectedItemOnRClick), m_SelectedItemOnRClick);
         break;
   }
   m_CutAndPasteItem = NULL;
   SetItemState(m_CutAndPasteItem, 0, TVIS_CUT);

   return;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::UpdParentCountSelected                       <external> */
/*+
  Questa funzione aggiorna il numero di elementi selezionati tra i parenti dell'elemento 
  selezionato\deselezionato.
  Parametri:
  HTREEITEM hItem;   elemento che è stato selezionato\deselezionato
  int increment;
-*/
/*****************************************************************************/
void CGEOTreeCtrl::UpdParentCountSelected(HTREEITEM hItem, int increment) // roby 2016
{
   HTREEITEM hParentItem = hItem;
   CGEOTree_Item *pItem;

   pItem = (CGEOTree_Item *) GetItemData(hItem);
   if (pItem->ObjectType != FinalObjectType) return;

   while ((hParentItem = GetParentItem(hParentItem)))
   {
      pItem = (CGEOTree_Item *) GetItemData(hParentItem);

      pItem->n_selected += increment;
      UpdItemTextOnSelected(hParentItem);
   }

   return;
}


/*****************************************************************************/
/*.doc CGEOTreeCtrl::UpdCountSelected                             <external> */
/*+
  Questa funzione aggiorna il testo dell'elemento con numero di elementi selezionati.
  Parametri:
  HTREEITEM hItem;
-*/
/*****************************************************************************/
void CGEOTreeCtrl::UpdItemTextOnSelected(HTREEITEM hItem) // roby 2016
{
   CGEOTree_Item *pItem = (CGEOTree_Item *) GetItemData(hItem);

   if (pItem->n_selected > 0)
   {
      C_STRING msg(pItem->get_name());

      msg += _T(" (");
      msg += pItem->n_selected;
      msg += _T(" sel.)");
      SetItemText(hItem, msg.get_name());
      SetBoldItem(hItem, true);
   }
   else
   {
      SetItemText(hItem, pItem->get_name());
      SetBoldItem(hItem, false);
   }
   return;
}