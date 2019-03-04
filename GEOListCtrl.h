#pragma once

#ifndef _gs_list_h
   #include "gs_list.h"
#endif


#ifndef IDD_GEOLISTCTRL_DLG
   #include "GEOListCtrlResource.h"
#endif


#define ID_FILTER_BY_NAME_MENU 101
#define ID_FILTER_BY_TYPE_MENU 102
#define ID_LOAD_SEL_MENU       103
#define ID_SAVE_SEL_MENU       104
#define ID_REFRESH_MENU        105
#define ID_SELECT_ALL_MENU     106
#define ID_DESELECT_ALL_MENU   107
#define ID_INVERT_SEL_MENU     108


//----------------------------------------------------------------------------//
//    class CGEOList_Item                                                    //
//----------------------------------------------------------------------------//
class CGEOList_Item : public C_INT_STR // codice e nome
{
   friend class CGEOList_ItemList;

public:
	CGEOList_Item();
	virtual ~CGEOList_Item();

   C_STRING	Descr;      // descrizione classe CString
   int		category;	// categoria (solo per classi)
	int		type;		   // tipo (per classi, sottoclassi e tabelle secondarie)
	GSDataPermissionTypeEnum updatable;	// GSReadOnlyData = sola lettura, GSUpdateableData = modificabile,
                        // GSExclusiveUseByAnother = Usata in modo esclusivo da un'altro utente
	bool		extracted;	// true se estratta nella sessione corrente

	int		image;		// indice dell'immagine assegnata e inserita nella lista (vedi CGEOList_ItemList)

	bool		selected;	// true se l'elemento è stato selezionato
   int      get_state();
   int      get_category();
   int      get_type();
};
//----------------------------------------------------------------------------//
//    class CGEOList_ItemList                                                  //
//----------------------------------------------------------------------------//
class CGEOList_ItemList : public C_LIST
{
   public :
      DllExport CGEOList_ItemList();
      DllExport virtual ~CGEOList_ItemList();  // chiama ~C_LIST

      CImageList ImageList;

   CGEOList_Item *AddItem(int _Key, const TCHAR *_Name, const TCHAR *_Descr);
   int LoadPrjs(void);
   int LoadClasses(int prj);
   int RefreshClasses(int prj);
   int LoadSubClasses(int prj, int cls);
   int LoadSecTabs(int prj, int cls, int sub = 0);
};



// CGEOListCtrl - inizio

class CGEOListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CGEOListCtrl)

public:
	CGEOListCtrl();
	virtual ~CGEOListCtrl();

   GSDataModelObjectTypeEnum ObjectType; // Tipo di elementi da listare
	BOOL MultiSelect;           // se vero è permessa la selezione multipla
	BOOL SelectedLinkedClass;   // se vero selezionando una classe gruppo vengono selezionate 
                               // anche quelle che ne fanno parte (se MultiSelect = vero e ObjectType = GSClass)

   // FILTRI DI VISUALIZZAZIONE
	int FilterOnPrj;            // codice progetto (usato se ObjectType = GSClass, GSSubClass, GSSecondaryTab)
	int FilterOnCls;            // codice classe (usato se ObjectType = GSSubClass, GSSecondaryTab)
	int FilterOnSub;            // codice sotto-classe (usato se ObjectType = GSSecondaryTab)
   C_INT_INT_LIST FilterOnClassCategoryTypeList; // Lista delle categorie e tipi di classi e sottoclassi
                                                 // usata come filtro se ObjectType = GSClass, GSSubClass
   C_INT_LIST FilterOnSecondaryTabType; // Lista dei tipi di tabelle secondarie
                                        // usata come filtro se ObjectType = GSSecondaryTab
   bool       UseFilterOnCodes;
   C_INT_LIST FilterOnCodes;   // Lista degli elementi con codici noti
   C_STRING   FilterOnName;    // Maschera usata come filtro
	BOOL       FilterOnExtractedInCurrWrkSession; // Solo elementi estratti nella sessione corrente
	BOOL       FilterOnUpdateable; // Solo elementi aggiornabili

	bool    ColumnClassStatusVisibility; // Flag per visualizzare la colonna di "classe estratta" (la lampadina)
	                                     // + "classe bloccata" (il lucchetto) (usato se ObjectType = GSClass)
	bool    ColumnAutosize;          // Flag per calcolare automaticamente la larghezza delle colonne

   int     LoadFromDB(bool OnlyRefresh = false); // Legge la lista da DB
   int     Refresh(bool OnlyRefresh = false); // Applica le impostazioni di visualizzazione della lista

   virtual BOOL PreTranslateMessage(MSG* pMsg);
	
   int  Prev_HeaderPos;
   bool AscendingOrder;

   void SetSelectedCodes(C_INT_LIST &SelectedCodes, bool AppendMode = false);
   void GetSelectedCodes(C_INT_LIST &SelectedCodes);

   void OnItemChanged(CGEOList_Item *pItem, bool selected);
   void OnFilterByNameMenu(void);
   void OnFilterByTypeMenu(void);
   void OnLoadSelMenu(void);
   void OnSaveSelMenu(void);
   void OnRefreshMenu(void);
   void OnSelectAllMenu(void);
   void OnDeselectAllMenu(void);
   void OnInvertSelMenu(void);

protected:

	CToolTipCtrl      *pToolTip;
	CImageList        ImageHdrList;
   CGEOList_ItemList ItemList;
   bool              OnRefresh;
   bool              insideCalcColumnSize;

   void CalcColumnSize(void);
   void CalcColumnSize_Prjs(void);
   void CalcColumnSize_Classes(void);
   void CalcColumnSize_SubClasses(void);
   void CalcColumnSize_SecTabs(void);

   bool Filter(CGEOList_Item *pItem);
   void Reset_List(void);
   int Refresh_Prjs(bool OnlyRefresh = false);
   int Refresh_Classes(bool OnlyRefresh = false);
   int Refresh_SubClasses(bool OnlyRefresh = false);
   int Refresh_SecTabs(bool OnlyRefresh = false);

   BOOL InitTooltip(void);
   BOOL DisplayTooltip(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);  
   afx_msg BOOL OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg BOOL OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
   void sendITEMCHANGEDEventToParent(void);
   void RefreshSelection(bool EnsureVisibleFlag = true);
};

// finestra di dialogo GEOListCtrlFilterDlg

class GEOListCtrlFilterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(GEOListCtrlFilterDlg)

public:
   enum FilterTypeEnum
   {
      ByName = 0,
      ByType = 1
   };

   GEOListCtrlFilterDlg(GSDataModelObjectTypeEnum _ObjectType, CWnd* pParent = NULL);
   GEOListCtrlFilterDlg(TCHAR *_Name, CWnd* pParent = NULL);
	virtual ~GEOListCtrlFilterDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_GEOLISTCTRL_DLG };

   FilterTypeEnum FilterType;
	CString	      Name;
   GSDataModelObjectTypeEnum ObjectType; // Tipo di elementi da listare
   int            Type;  // Se  ObjectType = GSClass:
                         // 0 = gruppo,    1 = griglia,     2 = nodi
                         // 3 = polilinee, 4 = simulazioni, 5 = spaghetti
                         // 6 = superfici, 7 = testi
                         // Se  ObjectType = GSSubClass:
                         // 0 = nodi, 1 = polilinee, 2 = superfici, 3 = testi
                         // Se  ObjectType = GSSecondaryTab:
                         // 0 = secondarie di GEOsim, 1 = secondarie esterne
   int ClassCategoryTypeToType(int _ClassCategory, int _ClassType);
   int SecTypeToType(int _SecType);
   bool TypeToClassCategoryType(int *_ClassCategory, int *_ClassType);
   bool TypeToSecType(int *_SecType);

protected:
   CImageList  ImageList;
   CComboBoxEx m_ComboCtrl;
   CEdit       m_Edit;

   void InitializeImageList();
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedOk();
};

