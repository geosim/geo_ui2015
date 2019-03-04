/*************************************************************************/
/*   GS_UI_SEC.H                                                          */
/*************************************************************************/


#ifndef _gs_ui_sec_h
#define _gs_ui_sec_h 1
#include "afxwin.h"
#include "GEOTreeCtrl.h"

#pragma once


// CDynSegmentationCalibrateDlg dialog

class CDynSegmentationCalibrateDlg : public CDialog
{
	DECLARE_DYNAMIC(CDynSegmentationCalibrateDlg)

public:
	CDynSegmentationCalibrateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDynSegmentationCalibrateDlg();

// Dialog Data
	enum { IDD = IDD_DYNSEGMENTATIONCALIBRATE_DLG };

   int       DynSegmCls;   // Codice classe che supporta la segmentazione dinamica
   int       DynSegmSub;   // Codice sotto-classe che supporta la segmentazione dinamica
   C_SELSET  AllObjs;      // Tutti gli oggetti in grafica da elaborare
   C_SELSET  SelSet;       // Selezione manuale oggetti da elaborare
   bool      AutoSel;      // Tipo di selezione di oggetti

   int       RefCls;       // Codice classe di riferimento distanziometrico
   int       RefSub;       // Codice sotto-classe di riferimento distanziometrico
   C_STRING  DistAttrib;   // Nome attributo che memorizza la distanza 
   double    Tolerance;    // Tolleranza di distanza tra una entità da calibrare e il
                           // punto di riferimento distanziometrico   
private:
   C_STR_LIST m_SrcNumericAttribNameList; // Lista degli attributi numerici per le distanze

   void RefreshSelection();
   void RefreshAttribList();

   HBITMAP hBmpSelObjs;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CGEOTreeCtrl m_GeoTree;
   CButton      m_AutoSelection;
   CButton      m_ManualSelection;
   CStatic      mLbl_nSelected;
   CGEOTreeCtrl m_GeoTree_Ref;
   CComboBox    m_DistAttribCombo;
   CEdit        m_ToleranceEdit;
   CButton      m_ObjsSelection;

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedRadioSelectall();
   afx_msg void OnBnClickedRadioManualselection();
   afx_msg void OnBnClickedButtonObjsselection();
   afx_msg void OnCbnSelchangeDistAttribute();
   DECLARE_EVENTSINK_MAP()
   void ChangeSelectionGeotreectrl();
   afx_msg void OnBnClickedOk();
   void ChangeSelectionGeotreectrlRef();
   afx_msg void OnEnKillfocusToleranceEdit();
   afx_msg void OnBnClickedHelp();
};

// CDynSegmentationInitDlg dialog

class CDynSegmentationInitDlg : public CDialog
{
	DECLARE_DYNAMIC(CDynSegmentationInitDlg)

public:
	CDynSegmentationInitDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDynSegmentationInitDlg();

// Dialog Data
	enum { IDD = IDD_DYNSEGMENTATION_DLG };

   C_STRING    UdlFile;         // Nome del file UDL da usare
   C_2STR_LIST UdlProperties;   // Lista delle proprietà UDL
   C_STRING real_init_distance_attrib;     // per la segmentazione dinamica, Nome attributo per
                                           // la distanza reale tra il primo vertice della 
                                           // polilinea e l'inizio del segmento
   C_STRING real_final_distance_attrib;    // per la segmentazione dinamica, Nome attributo per
                                           // la distanza reale tra il primo vertice della 
                                           // polilinea e la fine del segmento
   C_STRING nominal_init_distance_attrib;  // per la segmentazione dinamica, Nome attributo per
                                           // la distanza nominale tra il primo vertice della 
                                           // polilinea e l'inizio del segmento
   C_STRING nominal_final_distance_attrib; // per la segmentazione dinamica, Nome attributo per
                                           // la distanza nominale tra il primo vertice della 
                                           // polilinea e la fine del segmento
   C_STRING real_offset_attrib;            // per la segmentazione dinamica, Nome attributo per
                                           // la distanza (offset) dalla linea. usato solo
                                           // nel caso GSPunctualDynSegmentation
   C_ATTRIB_LIST attrib_list;

   void FromRbList(resbuf *RbList);
   resbuf *ToRbList(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
   C_STR_LIST m_NumericAttribNameList; // Lista degli attributi numerici tabella sorgente
   DynamicSegmentationTypeEnum DynSegmentationType;
   void RefreshSelection();
   void RefreshAttribList();
public:
   afx_msg void OnCbnSelchangeRealInitDistanceAttribCombo();
   afx_msg void OnCbnSelchangeNominalInitDistanceAttribCombo();
   afx_msg void OnCbnSelchangeRealFinalDistanceAttribCombo();
   afx_msg void OnCbnSelchangeNominalFinalDistanceAttribCombo();
   afx_msg void OnCbnSelchangeRealOffsetAttribCombo();
   afx_msg void OnBnClickedPunctualRadio();
   afx_msg void OnBnClickedLinearRadio();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();

   CComboBox m_RealInitDistanceAttribCombo;
   CComboBox m_NominalInitDistanceAttribCombo;
   CComboBox m_RealFinalDistanceAttribCombo;
   CComboBox m_NominalFinalDistanceAttribCombo;
   CComboBox m_RealOffsetAttribCombo;
   CButton   m_PunctualRadio;
   CButton   m_LinearRadio;
};


int gsuiInsDynSegmentationData(void);
int gsuiCalibrateDynSegmentationData(void);
int gsui_InitDynSegmentation(void);

#endif
#pragma once


