#ifndef   _GS_UI_GRID_H
#define   _GS_UI_GRID_H

#include "GEOTreeCtrl.h"

/******************************************************************************/
/*    GS_UI_GRID.H                                                           */
/******************************************************************************/


#pragma once


///////////////////////////////////////////////////////////////////////////////
// finestra di dialogo CGridFlowDlg
///////////////////////////////////////////////////////////////////////////////
class CGridFlowDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridFlowDlg)

public:
	CGridFlowDlg(CWnd* pParent = NULL);   // costruttore standard
	virtual ~CGridFlowDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_GRIDFLOW_DLG };

   enum AreaType
   {
      Extension = 0,
      Window    = 1
   };

   int       Cls;          // Codice classe
   ads_point pt1;          // Zona da considerare per il calcolo
   ads_point pt2;          // se pt1 = pt2 si intende tutta la griglia
   C_STRING  ZAttrib;      // Nome attributo che memorizza la quota
   C_STRING  FlowAttrib;   // Nome attributo per memorizzare il codice cella a valle

private:
   C_STR_LIST m_ZAttribNameList;    // Lista degli attributi numerici per le quote
   C_STR_LIST m_FlowAttribNameList; // Lista degli attributi numerici per il flusso

   void RefreshAttribList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
   CGEOTreeCtrl m_GeoTree;
   CComboBox    m_SelAreaType;
   CComboBox    m_ZAttrib;
   CComboBox    m_FlowAttrib;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnBnClickedOk();
   afx_msg void OnCbnSelchangeGridGridareatype();
   afx_msg void OnCbnSelchangeZAttribute();
   afx_msg void OnCbnSelchangeDirectionAttribute();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};


///////////////////////////////////////////////////////////////////////////////
// finestra di dialogo CGridDrapeDlg
///////////////////////////////////////////////////////////////////////////////
class CGridDrapeDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridDrapeDlg)

public:
	CGridDrapeDlg(CWnd* pParent = NULL);   // costruttore standard
	virtual ~CGridDrapeDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_GRIDDRAPE };

   int       Cls;          // Codice classe
   C_SELSET  AllObjs;      // Tutti gli oggetti in grafica da elaborare
   C_SELSET  SelSet;       // Selezione manuale oggetti da elaborare
   C_STRING  ZAttrib;      // Nome attributo che memorizza la quota
   bool      AutoSel;      // Tipo di selezione di oggetti

private:
   C_STR_LIST m_ZAttribNameList;    // Lista degli attributi numerici per le quote

   void RefreshSelection();
   void RefreshAttribList();

   HBITMAP hBmpSelObjs;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
   CGEOTreeCtrl m_GeoTree;
   CComboBox    m_ZAttrib;
   CButton      m_ObjsSelection;
   CButton      m_AutoSelection;
   CButton      m_ManualSelection;
   CStatic      mLbl_nSelected;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedRadioSelectall();
   afx_msg void OnBnClickedRadioManualselection();
   afx_msg void OnBnClickedButtonObjsselection();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnCbnSelchangeZAttribute();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};


///////////////////////////////////////////////////////////////////////////////
// CGriDSpatialInterpolationIDWDlg dialog
///////////////////////////////////////////////////////////////////////////////
class CGriDSpatialInterpolationIDWDlg : public CDialog
{
	DECLARE_DYNAMIC(CGriDSpatialInterpolationIDWDlg)

public:
	CGriDSpatialInterpolationIDWDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGriDSpatialInterpolationIDWDlg();

// Dialog Data
	enum { IDD = IDD_GRIDSPATIALINTERPOLATIONIDW_DLG };

   enum AreaType
   {
      Extension = 0,
      Window    = 1
   };

   int       Cls;           // Codice classe
   ads_point pt1;           // Zona da considerare per il calcolo
   ads_point pt2;           // se pt1 = pt2 si intende tutta la griglia
   C_STRING  Attrib;        // Nome attributo oggetto del calcolo
   double    Ray;           // Raggio utilizzato per cercare un intorno di punti
                            // noti a quello sconosciuto (se = 0 non viene usato e
                            // verrà calcolato un raggio tale da includere almeno
                            // <MinKnownPts> celle note, in questo caso <MinKnownPts>
                            // non deve essere = 0).
   int       MinKnownPts;   // Numero minimo di punti noti affinchè sia accettabile 
                            // l'interpolazione spaziale (se = 0 non viene usato, in 
                            // questo caso <Ray> non deve essere = 0).
   bool      Quadratic;     // Flag, se true le distanze vengono elevate al quadrato 
                            // per dare meno importanza ai punti lontani.
   bool      Recursive;     // Se = TRUE il processo continua ad iterare finchè riesce
                            // a interpolare qualche punto altrimenti prova una sola
                            // volta
   C_STRING  BarrierAttrib; // Nome attributo per la barriera

private:
   HBITMAP    hBmpSelRay;
   C_STR_LIST m_AttribNameList;    // Lista degli attributi numerici per le quote
   C_STR_LIST m_BarrierAttribNameList; // Lista degli attributi numerici per il flusso
   bool       m_insideRayToggle;

   void RefreshSelection(void);
   void RefreshAttribList(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CGEOTreeCtrl m_GeoTree;
   CComboBox    m_SelAreaType;
   CComboBox    m_Attrib;
   CComboBox    m_BarrierAttrib;
   CButton      m_InsideRay;
   CEdit        m_Ray;
   CButton      m_SelRayButton;
   CEdit        m_MinKnownPts;
   CButton      m_InverseDistance;
   CButton      m_QuadraticInverseDistance;
   CButton      m_Recursive;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnCbnSelchangeGridGridareatype();
   afx_msg void OnBnClickedInsideRay();
   afx_msg void OnBnClickedInverseDistRadio();
   afx_msg void OnBnClickedQuadraticInverseDistRadio();
   afx_msg void OnBnClickedRayButton();
   afx_msg void OnEnKillfocusRay();
   afx_msg void OnEnKillfocusMinPts();
   afx_msg void OnBnClickedRecursive();
   afx_msg void OnCbnSelchangeAttribute();
   afx_msg void OnCbnSelchangeBarrierAttribute();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};


///////////////////////////////////////////////////////////////////////////////
// CGriDHydrologyCountUpstreamCellsDlg dialog
///////////////////////////////////////////////////////////////////////////////

class CGriDHydrologyCountUpstreamCellsDlg : public CDialog
{
	DECLARE_DYNAMIC(CGriDHydrologyCountUpstreamCellsDlg)

public:
	CGriDHydrologyCountUpstreamCellsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGriDHydrologyCountUpstreamCellsDlg();

// Dialog Data
	enum { IDD = IDD_GRIDCOUNTUPSTREAMCELLS_DLG };

   enum AreaType
   {
      Extension = 0,
      Window    = 1
   };

   int       Cls;          // Codice classe
   ads_point pt1;          // Zona da considerare per il calcolo
   ads_point pt2;          // se pt1 = pt2 si intende tutta la griglia
   C_STRING  FlowAttrib;   // Nome attributo per memorizzare il codice cella a valle
   C_STRING  OutputAttrib; // Nome attributo che memorizza il conteggio delle celle a monte

private:
   C_STR_LIST m_FlowAttribNameList;    // Lista degli attributi numerici per il flusso
   C_STR_LIST m_OutputAttribNameList;  // Lista degli attributi numerici per il conteggio

   void RefreshAttribList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
   afx_msg void OnCbnSelchangeGridGridareatype();
   afx_msg void OnCbnSelchangeAttribute();
   afx_msg void OnCbnSelchangeOutputAttribute();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();

   CGEOTreeCtrl m_GeoTree;
   CComboBox    m_SelAreaType;
   CComboBox    m_FlowAttrib;
   CComboBox    m_OutputAttrib;
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};



///////////////////////////////////////////////////////////////////////////////
// CGridGetCatchmentAreaCellsDlg dialog
///////////////////////////////////////////////////////////////////////////////

class CGridGetCatchmentAreaCellsDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridGetCatchmentAreaCellsDlg)

public:
	CGridGetCatchmentAreaCellsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridGetCatchmentAreaCellsDlg();

// Dialog Data
	enum { IDD = IDD_GRIDGETCATCHMENTAREACELLS_DLG };

   int       Cls;          // Codice classe
   ads_point pt;           // punto della cella da considerare per il calcolo
   C_STRING  FlowAttrib;   // Nome attributo per memorizzare il codice cella a valle
   bool      DirectionDown; // verso di ricerca (monte o valle; su o giu)

private:
   C_STR_LIST m_FlowAttribNameList;    // Lista degli attributi numerici per il flusso
   HBITMAP hBmpSelPt;

   void RefreshAttribList();
   void RefreshSelection();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CGEOTreeCtrl m_GeoTree;
   CEdit        m_X;
   CEdit        m_Y;
   CButton      m_PtSelection;
   CComboBox    m_FlowAttrib;
   CButton      m_DirectionDown;
   CButton      m_DirectionUp;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedPointSelection();
   afx_msg void OnCbnSelchangeDirectionAttribute();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnEnKillfocusCoordxEdit();
   afx_msg void OnEnKillfocusCoordyEdit();
   afx_msg void OnBnClickedDirectionDownRadio();
   afx_msg void OnBnClickedDirectionUpRadio();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};

// CGridDisplayValleyOrRidgeDlg dialog

class CGridDisplayValleyOrRidgeDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridDisplayValleyOrRidgeDlg)

public:
	CGridDisplayValleyOrRidgeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridDisplayValleyOrRidgeDlg();

// Dialog Data
	enum { IDD = IDD_GRID_VALLEY_RIDGE_DLG };

   enum AreaType
   {
      Extension = 0,
      Window    = 1
   };

   int       Cls;          // Codice classe
   ads_point pt1;          // Zona da considerare per il calcolo
   ads_point pt2;          // se pt1 = pt2 si intende tutta la griglia
   C_STRING  ZAttrib;      // Nome attributo che memorizza la quota
   bool      Valley;       // Flag, se = TRUE il programma valuta le valli altrimenti i
                           // crinali dei monti
   long      flag_set;     // flag a bit che identifica quali azioni si vogliono 
                           // intraprendere (vedi GraphSettingsEnum)
   double    MinZoomWindow;
   C_FAS     FAS;          // Caratteristiche grafiche
   int       DisplayMode;  // Se = PREVIEW non crea oggetti ACAD (più veloce) altrimenti 
                           // (= EXTRACTION) crea oggetti grafici (default = PREVIEW)
private:
   C_STR_LIST m_ZAttribNameList;    // Lista degli attributi numerici per le quote
   void RefreshAttribList();
   void RefreshSelection();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
   CGEOTreeCtrl m_GeoTree;
   CComboBox    m_SelAreaType;
   CComboBox    m_ZAttrib;
   CButton      m_Valley;
   CButton      m_Ridge;
   CButton      m_Preview;
   CButton      m_FAS;

	virtual BOOL OnInitDialog();

   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnBnClickedValleyRadio();
   afx_msg void OnBnClickedRidgeRadio();
   afx_msg void OnCbnSelchangeZAttribute();
   afx_msg void OnBnClickedPreviewCheck();
   afx_msg void OnBnClickedFasButton();
   afx_msg void OnCbnSelchangeGridGridareatype();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};


///////////////////////////////////////////////////////////////////////////////
// CGridUpdZFromGraphDlg dialog
///////////////////////////////////////////////////////////////////////////////
class CGridUpdZFromGraphDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridUpdZFromGraphDlg)

public:
	CGridUpdZFromGraphDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridUpdZFromGraphDlg();

// Dialog Data
	enum { IDD = IDD_GRID_UPDZVALUE };

   int       Cls;          // Codice classe
   C_SELSET  AllObjs;      // Tutti gli oggetti in grafica da elaborare
   C_SELSET  SelSet;       // Selezione manuale oggetti da elaborare
   C_STRING  ZAttrib;      // Nome attributo che memorizza la quota
   bool      AutoSel;      // Tipo di selezione di oggetti
   bool      ZFromGraph;   // Tipo di sorgente dati (se = false significa da attributo)
   C_STRING  srcAttribName; // Nome attributo sorgente dati

private:
   C_STR_LIST m_ZAttribNameList;    // Lista degli attributi numerici per le quote

   void RefreshSelection();
   void RefreshAttribList();

   HBITMAP hBmpSelObjs;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CGEOTreeCtrl m_GeoTree;
   CComboBox    m_ZAttrib;
   CButton      m_ObjsSelection;
   CButton      m_AutoSelection;
   CButton      m_ManualSelection;
   CStatic      mLbl_nSelected;
   CButton      m_SrcElevOption;
   CButton      m_SrcAttribOption;
   CEdit        mSrcAttribName;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedRadioSelectall();
   afx_msg void OnBnClickedRadioManualselection();
   afx_msg void OnBnClickedButtonObjsselection();
   afx_msg void OnCbnSelchangeZAttribute();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedRadioElevation();
   afx_msg void OnBnClickedRadioAttrib();
};


///////////////////////////////////////////////////////////////////////////////
// CGridUpdFromDBDlg dialog
///////////////////////////////////////////////////////////////////////////////
class CGridUpdFromDBDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridUpdFromDBDlg)

public:
	CGridUpdFromDBDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridUpdFromDBDlg();

// Dialog Data
	enum { IDD = IDD_GRID_UPDVALUE_FROMDB };

   int         Cls;             // Codice classe
   C_STRING    SrcXAttrib;      // Nome attributo che memorizza la X
   C_STRING    SrcYAttrib;      // Nome attributo che memorizza la Y
   C_STRING    SrcAttrib;       // Nome attributo che memorizza la quota tabella sorgente
   C_STRING    DstAttrib;       // Nome attributo che memorizza la quota tabella destinazione
   C_STRING    UdlFile;         // Nome del file UDL da usare
   C_2STR_LIST UdlProperties;   // Lista delle proprietà UDL
   C_STRING    FullRefTable;    // riferimento completo alla tabella o alla vista

private:
   C_STR_LIST m_SrcNumericAttribNameList; // Lista degli attributi numerici tabella sorgente
   C_STR_LIST m_SrcAttribNameList;        // Lista degli attributi tabella sorgente
   C_STR_LIST m_DstAttribNameList;        // Lista degli attributi tabella destinazione

   void RefreshAttribList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CGEOTreeCtrl m_GeoTree;
   CEdit        m_UdlEdit;
   CEdit        m_FullReTableEdit;
   CComboBox    m_SrcXAttribCombo;
   CComboBox    m_SrcYAttribCombo;
   CComboBox    m_SrcAttribCombo;
   CComboBox    m_DstAttribCombo;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnCbnSelchangeSrcxattribCombo();
   afx_msg void OnCbnSelchangeSrcyattribCombo();
   afx_msg void OnCbnSelchangeSrcattribCombo();
   afx_msg void OnCbnSelchangeDstattributeCombo();
   afx_msg void OnBnClickedButton1();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};


///////////////////////////////////////////////////////////////////////////////
// CGridAutoContoursDlg dialog
///////////////////////////////////////////////////////////////////////////////
class CGridAutoContoursDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridAutoContoursDlg)

public:
	CGridAutoContoursDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridAutoContoursDlg();

// Dialog Data
	enum { IDD = IDD_GRID_AUTO_CONTOURS_DLG };

   enum AutoCompositionTypeEnum
   {
      None          = 0,
      PerNIntervals = 1,
      PerSteps      = 2
   };

   AutoCompositionTypeEnum m_AutoCompositionType;
   int                     m_AutoCompositionNSteps;
   double                  m_AutoCompositionStep;
   double                  m_MinValue;
   double                  m_MaxValue;
   C_COLOR                 m_AutoCompositionStartColor;
   C_COLOR                 m_AutoCompositionEndColor;

private:
   C_COLOR_LIST AutoCompositionColorsList;
   void RefreshSelection();
   CFont m_BoldFont;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
   CButton     m_AutoPerIntervalsRadio;
   CButton     m_AutoPerStepsRadio;  
   CEdit       m_AutoIntervalsEdit;
   CEdit       m_AutoStepEdit;

   CEdit       m_MinValueEdit;
   CEdit       m_MaxValueEdit;

   CButton     m_StartColorButton;
   CButton     m_FinalColorButton;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnBnClickedAutoPerNIntervals();
   afx_msg void OnBnClickedAutoPerSteps();
   // Called to change color of text
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   afx_msg void OnBnClickedStartColor();
   afx_msg void OnBnClickedFinalColor();

};


///////////////////////////////////////////////////////////////////////////////
// CGridDisplayContoursDlg dialog
///////////////////////////////////////////////////////////////////////////////
class CGridDisplayContoursDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridDisplayContoursDlg)

public:
	CGridDisplayContoursDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridDisplayContoursDlg();

// Dialog Data
	enum { IDD = IDD_GRID_CONTOURS_DLG };

   enum AreaType
   {
      Extension = 0,
      Window    = 1
   };

   int       Cls;          // Codice classe
   ads_point pt1;          // Zona da considerare per il calcolo
   ads_point pt2;          // se pt1 = pt2 si intende tutta la griglia
   C_STRING  ZAttrib;      // Nome attributo che memorizza la quota
   int       DisplayMode;  // Se = PREVIEW non crea oggetti ACAD (più veloce) altrimenti 
                           // (= EXTRACTION) crea oggetti grafici (default = PREVIEW)
   bool      Join;         // Flag per unire le curve di livello (usato solo se DisplayMode = EXTRACTION)
   bool      Fit;          // Flag per curvare le curve di livello (usato solo se DisplayMode = EXTRACTION)
   C_FAS_LIST FAS_list;    // lista degli intervalli delle curve di livello

private:
   C_STR_LIST m_ZAttribNameList;    // Lista degli attributi numerici per le quote
   C_COLOR_LIST AutoCompositionColorsList;
   void RefreshAttribList();
   void RefreshSelection();
   void RefreshContourList();
   int Save(const TCHAR *Path);
   int Load(const TCHAR *Path);

   CGridAutoContoursDlg AutoContoursDlg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CGEOTreeCtrl m_GeoTree;
   CComboBox    m_SelAreaType;
   CComboBox    m_ZAttrib;
   CButton      m_Preview;
   CButton      m_Join;
   CButton      m_Fit;
   CButton      m_AutocompositionButton;
   CListCtrl    m_ContourList;
   CEdit        m_SelectIntervalEdit;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnCbnSelchangeGridGridareatype();
   afx_msg void OnCbnSelchangeZAttribute();
   afx_msg void OnBnClickedPreviewCheck();
   afx_msg void OnBnClickedJoinCheck();
   afx_msg void OnBnClickedCurveCheck();
   afx_msg void OnBnClickedAutocomposition();
   afx_msg void OnBnClickedNew();
   afx_msg void OnBnClickedModify();
   afx_msg void OnBnClickedErase();
   afx_msg void OnNMCustomdrawContourList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedSelectIntervals();
   afx_msg void OnNMDblclkContourList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedSave();
   afx_msg void OnBnClickedLoad();
   afx_msg void OnTvnSelchangedGeotreectrl(NMHDR *pNMHDR, LRESULT *pResult);
};


///////////////////////////////////////////////////////////////////////////////
// CGridPropContourDlg dialog
///////////////////////////////////////////////////////////////////////////////
class CGridPropContourDlg : public CDialog
{
	DECLARE_DYNAMIC(CGridPropContourDlg)

public:
	CGridPropContourDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridPropContourDlg();

// Dialog Data
	enum { IDD = IDD_GRID_PROP_CONTOUR_DLG };

   bool     m_Multi;

   double   m_Value;        // Valore
   C_COLOR  m_Color;        // Codice colore
   C_STRING m_Description;  // Descrizione
   C_STRING m_LineType;     // Tipolinea
   C_STRING m_Layer;        // Piano

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CEdit   m_ValueEdit;
   CEdit   m_ColorEdit;
   CEdit   m_DescriptionEdit;
   CEdit   m_LineTypeEdit;
   CEdit   m_LayerEdit;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedColorButton();
   afx_msg void OnBnClickedLinetypeButton();
   afx_msg void OnBnClickedLayerButton();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
};


int gsui_GridSetFlowDirection(void);
int gsui_GridDrape(void);
int gsui_GridSpatialInterpolationIDW(void);
int gsui_GridHydrologyCountUpstreamCells(void);
int gsui_GridGetCatchmentAreaCells(void);
int gsui_GridDisplayValleyOrRidge(void);
int gsui_GridUpdZFromGraph(void);
int gsui_GridUpdFromDB(void);
int gsui_GridDisplayContours(void);


#endif
