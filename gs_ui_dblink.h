#pragma once


// CAlfaNumDBLinkDlg dialog

class CAlfaNumDBLinkDlg : public CDialog
{
	DECLARE_DYNAMIC(CAlfaNumDBLinkDlg)

public:
	CAlfaNumDBLinkDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlfaNumDBLinkDlg();

// Dialog Data
	enum { IDD = IDD_ALFANUM_DB_LINK_DLG };

   int         ClassType;      // tipo di classe
   C_STRING    ConnStrUDLFile; // Stringa di connessione o File di tipo .UDL
   C_2STR_LIST UDLProperties;  // Lista di proprietà UDL 
   C_STRING    TableRef;       // Riferimento completo alla tabella
   C_STRING    SqlCondOnTable; // eventuale condizione SQL da applicare per cercare i dati nella
                               // tabella nel caso questa non contenga dati eclusivamente della classe
   C_STRING ent_key_attrib;    // Nome del campo contenente il codice 
                               // dell'entita' di appartenenza dell'oggetto grafico
   C_STRING text_attrib;        // Nome del campo chiave per la ricerca 
                               // dell'oggetto grafico della classe


private:
   C_STR_LIST m_NumericAttribNameList; // Lista degli attributi numerici della tabella
   C_STR_LIST m_CharAttribNameList;    // Lista degli attributi carattere della tabella

   void RefreshAttribList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CEdit       m_UdlEdit;
   CEdit       m_FullReTableEdit;
   CEdit       m_SqlCondOnTableEdit;
   CComboBox   m_EntKeyAttribCombo;
   CComboBox   m_TxtAttribCombo;

   int FromRb(presbuf p);
   presbuf ToRb(void);

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedDataSourceButton();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnCbnSelchangeEntityIdCombo();
   afx_msg void OnCbnSelchangeTextCombo();
   afx_msg void OnEnChangeSqlCondEdit();
};
#pragma once


// CGeomDBLinkDlg dialog

class CGeomDBLinkDlg : public CDialog
{
	DECLARE_DYNAMIC(CGeomDBLinkDlg)

public:
	CGeomDBLinkDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGeomDBLinkDlg();

// Dialog Data
	enum { IDD = IDD_GEOM_DB_LINK_DLG };

   int         ClassType;      // tipo di classe
   C_STRING    ConnStrUDLFile; // Stringa di connessione o File di tipo .UDL
   C_2STR_LIST UDLProperties;  // Lista di proprietà UDL 
   C_STRING    TableRef;       // Riferimento completo alla tabella
   C_STRING    SqlCondOnTable; // eventuale condizione SQL da applicare per cercare i dati nella
                               // tabella nel caso questa non contenga dati eclusivamente della classe
   C_STRING key_attrib;        // Nome del campo chiave per la ricerca 
                               // dell'oggetto grafico della classe
   C_STRING ent_key_attrib;    // Nome del campo contenente il codice 
                               // dell'entita' di appartenenza dell'oggetto grafico
   C_STRING aggr_factor_attrib; // Nome del campo contenente il fattore di aggregazione
                                // degli oggetti grafici relativi alla stessa entità

   C_STRING SRID;              // sistema di coordinate
   GeomDimensionEnum geom_dim; // dato geometrico 2D o 3D (lo standard OpenGIS prevede 2D)
   bool GeomNumericFormat;
   C_STRING geom_attrib;      // Nome del campo contenente la geometria dell'oggetto grafico
   // In alternativa (geom_attrib è vuoto), se la geometria è sotto forma di 3 campi numerici
   C_STRING x_attrib;         // Nome del campo contenente la coordinata x dell'oggetto grafico
   C_STRING y_attrib;         // Nome del campo contenente la coordinata y dell'oggetto grafico
   C_STRING z_attrib;         // Nome del campo contenente la coordinata z dell'oggetto grafico

   C_STRING rotation_attrib;  // Nome del campo contenente la rotazione dell'oggetto 
                              // grafico (per blocchi, testi, riempimenti)
   RotationMeasureUnitsEnum rotation_unit; // Unita' di misura della rotazione indicata da rotation_attrib 
   C_STRING vertex_parent_attrib; // Nome del campo contenente un codice per 
                                  // raggruppare i vertici per comporre una linea (polilinee)
                                  // (se le coordinate sono memorizzate con 2 o 3 campi numerici, polilinee)
   C_STRING bulge_attrib;     // Nome del campo contenente il bulge tra un vertice e 
                              // l'altro dell'oggetto grafico lineare della classe 
                              // (se le coordinate sono memorizzate con 2 o 3 campi numerici, polilinee)
   C_STRING layer_attrib;     // Nome del campo contenente il layer dell'oggetto grafico
   C_STRING color_attrib;     // Nome del campo contenente il codice colore dell'oggetto grafico
   GSFormatColorEnum color_format; // Formato del colore indicato da color_attrib e hatch_color_attrib
   C_STRING thickness_attrib; // Nome del campo contenente lo spessore verticale dell'oggetto grafico

   C_STRING line_type_attrib; // Nome del campo contenente il nome del tipo linea dell'oggetto grafico
   C_STRING line_type_scale_attrib; // Nome del campo contenente il fattore di scala del tipolinea dell'oggetto grafico
   C_STRING width_attrib;     // Nome del campo contenente la larghezza dell'oggetto grafico

   C_STRING text_attrib;      // Nome del campo contenente il testo dell'oggetto grafico
   C_STRING text_style_attrib; // Nome del campo contenente lo stile testo dell'oggetto grafico
   C_STRING h_text_attrib;    // Nome del campo contenente l'altezza testo dell'oggetto grafico
   C_STRING horiz_align_attrib; // Nome del campo contenente il codice acad di allineamento orizzontale dell'oggetto grafico
   C_STRING vert_align_attrib; // Nome del campo contenente il codice acad di allineamento verticale dell'oggetto grafico
   C_STRING oblique_angle_attrib; // Nome del campo contenente l'angolo obliquo del testo dell'oggetto grafico
   
   C_STRING block_attrib;     // Nome del campo contenente il nome del blocco dell'oggetto grafico
   C_STRING block_scale_attrib; // Nome del campo contenente il fattore di scala dell'oggetto grafico

   C_STRING hatch_attrib;     // Nome del campo contenente il nome del riempimento dell'oggetto grafico
   C_STRING hatch_layer_attrib; // Nome del campo contenente il layer del riempimento dell'oggetto grafico
   C_STRING hatch_color_attrib; // Nome del campo contenente il codice colore del riempimento dell'oggetto grafico
   C_STRING hatch_scale_attrib; // Nome del campo contenente il fattore di scala del riempimento dell'oggetto grafico
   C_STRING hatch_rotation_attrib; // Nome del campo contenente la rotazione del riempimento dell'oggetto grafico

private:
   C_STR_LIST m_NumericAttribNameList; // Lista degli attributi numerici della tabella
   C_STR_LIST m_CharAttribNameList;    // Lista degli attributi carattere della tabella
   C_STR_LIST m_GeomAttribNameList;    // Lista degli attributi geometrici della tabella

   void RefreshSelection();
   void RefreshAttribList();
   
   void EnableNumericGeomCtrls(BOOL bEnable = TRUE);
   void EnableLineCtrls();
   void EnableTxtCtrls();
   void EnableBlockCtrls();
   void EnableSurfaceCtrls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
   CEdit       m_UdlEdit;
   CEdit       m_FullReTableEdit;
   CEdit       m_SqlCondOnTableEdit;
   CComboBox   m_key_attribCombo;
   CComboBox   m_ent_key_attribCombo;
   CComboBox   m_aggr_factor_attribCombo;
   
   CComboBox   m_geom_dimCombo;
   CButton     m_geom_field_Radio;
   CButton     m_numeric_field_Radio;
   CComboBox   m_geom_attribCombo;
   CComboBox   m_x_attribCombo;
   CComboBox   m_y_attribCombo;
   CComboBox   m_z_attribCombo;
   CComboBox   m_bulge_attribCombo;
   CComboBox   m_vertex_parent_attribCombo;
   CComboBox   m_rotation_attribCombo;
   CComboBox   m_rotation_unitCombo;

   CComboBox   m_layer_attribCombo;
   CComboBox   m_color_attribCombo;
   CComboBox   m_color_formatCombo;
   CComboBox   m_thickness_attribCombo;

   CComboBox   m_line_type_attribCombo;
   CComboBox   m_line_type_scale_attribCombo;
   CComboBox   m_width_attribCombo;

   CComboBox   m_text_attribCombo;
   CComboBox   m_text_style_attribCombo;
   CComboBox   m_h_text_attribCombo;
   CComboBox   m_horiz_align_attribCombo;
   CComboBox   m_vert_align_attribCombo;
   CComboBox   m_oblique_angle_attribCombo;

   CComboBox   m_block_attribCombo;
   CComboBox   m_block_scale_attribCombo;
   
   CComboBox   m_hatch_attribCombo;
   CComboBox   m_hatch_layer_attribCombo;
   CComboBox   m_hatch_color_attribCombo;
   CComboBox   m_hatch_scale_attribCombo;
   CComboBox   m_hatch_rotation_attribCombo;

   int FromRb(presbuf p);
   presbuf ToRb(void);

	virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedDataSourceButton();
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedHelp();
   afx_msg void OnCbnSelchangeGeomUniqueIdCombo();
   afx_msg void OnCbnSelchangeEntityIdCombo();
   afx_msg void OnCbnSelchangeNDimCombo();
   afx_msg void OnCbnSelchangeGeomFieldCombo();
   afx_msg void OnBnClickedGeomFieldRadio();
   afx_msg void OnBnClickedNumericFieldRadio();
   afx_msg void OnCbnSelchangeXFieldCombo();
   afx_msg void OnCbnSelchangeYFieldCombo();
   afx_msg void OnCbnSelchangeZFieldCombo();
   afx_msg void OnCbnSelchangeBulgeFieldCombo();
   afx_msg void OnCbnSelchangeLineIdCombo();
   afx_msg void OnCbnSelchangeRotCombo();
   afx_msg void OnCbnSelchangeRotUnitCombo();
   afx_msg void OnCbnSelchangeLayerCombo();
   afx_msg void OnCbnSelchangeColorCombo();
   afx_msg void OnCbnSelchangeThicknessCombo();
   afx_msg void OnCbnSelchangeLinetypeCombo();
   afx_msg void OnCbnSelchangeLinetypeScaleCombo();
   afx_msg void OnCbnSelchangeLinetypeWidthCombo();
   afx_msg void OnCbnSelchangeHatchCombo();
   afx_msg void OnCbnSelchangeHatchLayerCombo();
   afx_msg void OnCbnSelchangeHatchColorCombo();
   afx_msg void OnCbnSelchangeHatchScaleCombo();
   afx_msg void OnCbnSelchangeHatchRotCombo();
   afx_msg void OnCbnSelchangeTextCombo();
   afx_msg void OnCbnSelchangeHTextCombo();
   afx_msg void OnCbnSelchangeTextStyleCombo();
   afx_msg void OnCbnSelchangeHorizAlignCombo();
   afx_msg void OnCbnSelchangeVertAlignCombo();
   afx_msg void OnCbnSelchangeObliqueAngleCombo();
   afx_msg void OnCbnSelchangeBlockCombo();
   afx_msg void OnCbnSelchangeBlockScaleCombo();
   afx_msg void OnEnChangeSqlCondEdit();
   afx_msg void OnCbnSelchangeAggrFactorCombo();
   afx_msg void OnCbnSelchangeColorFmtCombo();
};


int gsui_getAlfaNumDBLink(void);
int gsui_getGeomDBLink(void);
