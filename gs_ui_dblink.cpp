/**********************************************************
Name: GS_UI_DBLINK
                                   
Module description: File funzioni per la connessione a tabelle già esistenti
                    in modalità GEOUI (GEOsim User Interface)
            
Author: Roberto Poltini

(c) Copyright 2005-2012 by IREN ACQUA GAS  S.p.A

**********************************************************/


#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include "gs_ui_utily.h"
#include "gs_ui_sql.h"
#include "gs_ui_dblink.h"

#include "gs_error.h" 
#include "gs_utily.h"
#include "gs_init.h"
#include "gs_resbf.h"
#include "gs_prjct.h"
#include "gs_filtr.h"   // per gsc_define_window
#include "gs_evid.h"
#include "gs_grid.h"

#include "d2hMap.h" // doc to help


//**********************************************************
// INIZIO INTERFACCIA PER IL COLLEGAMENTO CON TABELLA ALFANUMERICA
//**********************************************************


/*************************************************************************/
/*.doc gsui_getAlfaNumDBLink                                             */
/*+
  Funzione lisp per impostare il collegamento con una tabella esistente che contenga
  le informazioni alfanumeriche per una classe di entità di GEOsim.
  Parametri:
  (("TYPE"<class type>)("UDL_FILE"<ConnStrUDLFile>)("UDL_PROP"<UDLProperties>)
   ("TABLE_REF"<TableRef>)("SQL_COND_ON_TABLE"<SqlCondOnTable>)
   ("KEY_ATTRIB"<key_attrib>)("TEXT_ATTRIB"<text_attrib>))


  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_getAlfaNumDBLink(void)
{
   presbuf           arg = acedGetArgs();
   C_RB_LIST         ret;
   CAlfaNumDBLinkDlg AlfaNumDBLinkDlg;

   acedRetNil();

   if (arg) 
      if (AlfaNumDBLinkDlg.FromRb(arg) == GS_BAD) return RTERROR;

 	if (AlfaNumDBLinkDlg.DoModal() == IDOK)
      if ((ret << AlfaNumDBLinkDlg.ToRb()))
         ret.LspRetList();

   return RTNORM;
}

// CAlfaNumDBLinkDlg dialog

IMPLEMENT_DYNAMIC(CAlfaNumDBLinkDlg, CDialog)

CAlfaNumDBLinkDlg::CAlfaNumDBLinkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlfaNumDBLinkDlg::IDD, pParent)
{
   ClassType = -1;
}

CAlfaNumDBLinkDlg::~CAlfaNumDBLinkDlg()
{
}

void CAlfaNumDBLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_UDL_EDIT, m_UdlEdit);
   DDX_Control(pDX, IDC_FULLREFTABLE_EDIT, m_FullReTableEdit);
   DDX_Control(pDX, IDC_SQL_COND_EDIT, m_SqlCondOnTableEdit);

   DDX_Control(pDX, IDC_ENTITY_ID_COMBO, m_EntKeyAttribCombo);
   DDX_Control(pDX, IDC_TEXT_COMBO, m_TxtAttribCombo);
}


BEGIN_MESSAGE_MAP(CAlfaNumDBLinkDlg, CDialog)
   ON_BN_CLICKED(IDC_DATA_SOURCE_BUTTON, &CAlfaNumDBLinkDlg::OnBnClickedDataSourceButton)
   ON_BN_CLICKED(IDOK, &CAlfaNumDBLinkDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CAlfaNumDBLinkDlg::OnBnClickedHelp)
   ON_CBN_SELCHANGE(IDC_ENTITY_ID_COMBO, &CAlfaNumDBLinkDlg::OnCbnSelchangeEntityIdCombo)
   ON_CBN_SELCHANGE(IDC_TEXT_COMBO, &CAlfaNumDBLinkDlg::OnCbnSelchangeTextCombo)
   ON_EN_CHANGE(IDC_SQL_COND_EDIT, &CAlfaNumDBLinkDlg::OnEnChangeSqlCondEdit)
END_MESSAGE_MAP()


BOOL CAlfaNumDBLinkDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   RefreshAttribList();

   int Pos = m_NumericAttribNameList.getpos_name(ent_key_attrib.get_name(), FALSE);
   if (Pos > 0) m_EntKeyAttribCombo.SetCurSel(Pos - 1);
   else ent_key_attrib.clear();

   Pos = m_CharAttribNameList.getpos_name(text_attrib.get_name(), FALSE);
   if (Pos > 0) m_TxtAttribCombo.SetCurSel(Pos - 1);
   else text_attrib.clear();

   if (ClassType == TYPE_TEXT) m_TxtAttribCombo.EnableWindow(TRUE);
   else m_TxtAttribCombo.EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// CAlfaNumDBLinkDlg message handlers

void CAlfaNumDBLinkDlg::OnBnClickedDataSourceButton()
{
   CDBConnDlg MyDlg;

   // se non specificato prova a caricare l'ultima connessione usata
   if (ConnStrUDLFile.len() == 0 && UDLProperties.get_count() == 0)
      MyDlg.FromRbList(NULL);
   else
   {
      MyDlg.UdlFile = ConnStrUDLFile;
      UDLProperties.copy(MyDlg.UdlProperties);
   }
   MyDlg.FullRefTable = TableRef;
   MyDlg.Flags = CONN_DLG_CHOICE_ON_TABLE | CONN_DLG_CHOICE_ON_VIEW;

   if (MyDlg.DoModal() == IDOK)
   {
      ConnStrUDLFile = MyDlg.UdlFile;
      MyDlg.UdlProperties.copy(UDLProperties);
      TableRef = MyDlg.FullRefTable;
      RefreshAttribList();
   }
}

void CAlfaNumDBLinkDlg::OnBnClickedOk()
{
   if (ent_key_attrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un campo per l'identificatore univoco delle entità."));
      return;
   }

   if (ClassType == TYPE_TEXT)
      if (text_attrib.get_name() == NULL)
      {
         gsui_alert(_T("Scegliere un campo per il valore del testo."));
         return;
      }

   OnOK();
}

void CAlfaNumDBLinkDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Caratteristichegrafichedaticollegati);
}

void CAlfaNumDBLinkDlg::OnCbnSelchangeEntityIdCombo()
{
   if (m_EntKeyAttribCombo.GetCurSel() == CB_ERR) return;
   ent_key_attrib = m_NumericAttribNameList.getptr_at(m_EntKeyAttribCombo.GetCurSel() + 1)->get_name();
}

void CAlfaNumDBLinkDlg::OnCbnSelchangeTextCombo()
{
   if (m_TxtAttribCombo.GetCurSel() == CB_ERR) return;
   text_attrib = m_CharAttribNameList.getptr_at(m_TxtAttribCombo.GetCurSel() + 1)->get_name();
}

void CAlfaNumDBLinkDlg::OnEnChangeSqlCondEdit()
{
   CString dummy;
   m_SqlCondOnTableEdit.GetWindowText(dummy);
   SqlCondOnTable = dummy.Trim();
}

void CAlfaNumDBLinkDlg::RefreshAttribList()
{
   C_ATTRIB *pAttrib;
   C_DBCONNECTION *pConn;
   C_ATTRIB_LIST SrcStru;

   m_UdlEdit.SetWindowText(ConnStrUDLFile.get_name());
   m_FullReTableEdit.SetWindowText(TableRef.get_name());
   m_SqlCondOnTableEdit.SetWindowText(SqlCondOnTable.get_name());

   m_NumericAttribNameList.remove_all();
   m_CharAttribNameList.remove_all();
   while (m_EntKeyAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo
   while (m_TxtAttribCombo.DeleteString(0) != CB_ERR); // svuoto la combo

   // Verifico la connessione OLE-DB
   if (ConnStrUDLFile.get_name() && TableRef.get_name() &&
       (pConn = get_pDBCONNECTION_LIST()->get_Connection(ConnStrUDLFile.get_name(),
                                                         &UDLProperties,
                                                         false,
                                                         GS_BAD)) != NULL)
      if (SrcStru.from_DB(pConn, pConn, TableRef.get_name()) == GS_GOOD)
      {
         pAttrib = (C_ATTRIB *) SrcStru.get_head();

         while (pAttrib)
         {
            if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD) // se è numerico
            {
               m_EntKeyAttribCombo.AddString(pAttrib->name.get_name());
               m_NumericAttribNameList.add_tail_str(pAttrib->name.get_name());
            }
            else
            if (gsc_DBIsChar(pAttrib->ADOType) == GS_GOOD) // se è carattere
            {
               m_TxtAttribCombo.AddString(pAttrib->name.get_name());
               m_CharAttribNameList.add_tail_str(pAttrib->name.get_name());
            }

            pAttrib = (C_ATTRIB *) SrcStru.get_next();
         }
      }

   m_NumericAttribNameList.sort_name(); // li ordino in modo crescente
   m_CharAttribNameList.sort_name();    // li ordino in modo crescente

   // verifico se i valori di default dei nomi degli attributi esistono nella struttura
   C_DBGPH_INFO TestGphInfo;
   int          Pos;

   // ent_key_attrib
   if ((Pos = m_NumericAttribNameList.getpos_name(ent_key_attrib.get_name(), FALSE)) > 0)
      m_EntKeyAttribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.key_attrib.get_name(), FALSE)) > 0)
         { m_EntKeyAttribCombo.SetCurSel(Pos - 1); ent_key_attrib = TestGphInfo.key_attrib; }
      else
         ent_key_attrib.clear();

   if (ClassType == TYPE_TEXT)
   {
      // text
      if ((Pos = m_CharAttribNameList.getpos_name(text_attrib.get_name(), FALSE)) > 0)
         m_TxtAttribCombo.SetCurSel(Pos - 1);
      else
         if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.text_attrib.get_name(), FALSE)) > 0)
            { m_TxtAttribCombo.SetCurSel(Pos - 1); text_attrib = TestGphInfo.text_attrib; }
         else
            text_attrib.clear();
   }
}
   
int CAlfaNumDBLinkDlg::FromRb(presbuf rb)
{
   presbuf  p;

   if ((p = gsc_CdrAssoc(_T("TYPE"), rb, FALSE))) // class type
      if (gsc_rb2Int(p, &ClassType) == GS_BAD) return GS_BAD;

   if ((p = gsc_CdrAssoc(_T("UDL_FILE"), rb, FALSE)))
      if (p->restype == RTSTR)
      {
         ConnStrUDLFile = p->resval.rstring;
         ConnStrUDLFile.alltrim();
         // traduco dir assoluto in dir relativo
         if (gsc_nethost2drive(ConnStrUDLFile) == GS_BAD) return GS_BAD;
      }
      else
         ConnStrUDLFile.clear();

   if ((p = gsc_CdrAssoc(_T("UDL_PROP"), rb, FALSE)))
      if (p->restype == RTSTR) // Le proprietà sono in forma di stringa
      {
         if (gsc_PropListFromConnStr(p->resval.rstring, UDLProperties) == GS_BAD)
            return GS_BAD;
      }
      else
      if (p->restype == RTLB) // Le proprietà sono in forma di lista
      {
         if (gsc_getUDLProperties(&p, UDLProperties) == GS_BAD) return GS_BAD;
      }
      else
         UDLProperties.remove_all();

   // Conversione path UDLProperties da assoluto in dir relativo
   if (gsc_UDLProperties_nethost2drive(ConnStrUDLFile.get_name(), UDLProperties) == GS_BAD)
      return GS_BAD;

   C_DBCONNECTION *pConn;
   
   if ((pConn = get_pDBCONNECTION_LIST()->get_Connection(ConnStrUDLFile.get_name(),
                                                         &UDLProperties,
                                                         false,
                                                         GS_BAD)) != NULL)
      if ((p = gsc_CdrAssoc(_T("TABLE_REF"), rb, FALSE)))
         if (p->restype == RTSTR)
         {
            // Conversione riferimento tabella da dir relativo in assoluto
            if (TableRef.paste(pConn->FullRefTable_nethost2drive(p->resval.rstring)) == NULL)
               return GS_BAD;
         }
         else
         if (p->restype == RTLB)
         {  // (<cat><sch><tab>)
            C_STRING Catalog, Schema, Table;

            if (!(p = p->rbnext)) return GS_BAD;
            if (p->restype == RTSTR) Catalog = p->resval.rstring;
            if (!(p = p->rbnext)) return GS_BAD;
            if (p->restype == RTSTR) Schema = p->resval.rstring;
            if (!(p = p->rbnext)) return GS_BAD;
            if (p->restype == RTSTR) Table = p->resval.rstring;
            if (TableRef.paste(pConn->get_FullRefTable(Catalog, Schema, Table)) == NULL)
               return GS_BAD;
         }
         else
            TableRef.clear();

   if ((p = gsc_CdrAssoc(_T("SQL_COND_ON_TABLE"), rb, FALSE)))
      if (p->restype == RTSTR) SqlCondOnTable = p->resval.rstring;
      else SqlCondOnTable.clear();

   if ((p = gsc_CdrAssoc(_T("KEY_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) ent_key_attrib = p->resval.rstring;
      else ent_key_attrib.clear();

   if (ClassType == TYPE_TEXT) 
      if ((p = gsc_CdrAssoc(_T("TEXT_ATTRIB"), rb, FALSE)))
         if (p->restype == RTSTR) text_attrib = p->resval.rstring;
         else text_attrib.clear();

   return GS_GOOD;
}
   
presbuf CAlfaNumDBLinkDlg::ToRb(void)
{
   C_RB_LIST List;
   C_STRING  Buffer;

   if ((List << acutBuildList(RTLB,
                                 RTSTR, _T("TYPE"),
                                 RTSHORT, ClassType,
                              RTLE, 0)) == NULL) return NULL;

   if ((List += acutBuildList(RTLB,
                                 RTSTR, _T("UDL_FILE"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(ConnStrUDLFile)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("UDL_PROP"),
                              0)) == NULL) return NULL;
   Buffer.paste(gsc_PropListToConnStr(UDLProperties));  
   if ((List += gsc_str2rb(Buffer)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("TABLE_REF"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(TableRef)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("SQL_COND_ON_TABLE"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(SqlCondOnTable)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("KEY_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(ent_key_attrib)) == NULL) return NULL;

   if (ClassType == TYPE_TEXT)
   {
      if ((List += acutBuildList(RTLE,
                                 RTLB,
                                    RTSTR, _T("TEXT_ATTRIB"),
                                 0)) == NULL) return NULL;
      if ((List += gsc_str2rb(text_attrib)) == NULL) return NULL;
   }

   if ((List += acutBuildList(RTLE, 0)) == NULL) return NULL;
   List.ReleaseAllAtDistruction(GS_BAD);

   return List.get_head();
}

//**********************************************************
// FINE INTERFACCIA PER IL COLLEGAMENTO CON TABELLA ALFANUMERICA
// INIZIO INTERFACCIA PER IL COLLEGAMENTO CON TABELLA GEOMETRICA
//**********************************************************


/*************************************************************************/
/*.doc gsui_getGeomDBLink                                                  */
/*+
  Funzione lisp per impostare il collegamento con una tabella esistente che contenga
  le informazioni geometriche per una classe di entità di GEOsim.
  Parametri:
  (("TYPE"<class type>)("UDL_FILE"<ConnStrUDLFile>)("UDL_PROP"<UDLProperties>)
   ("TABLE_REF"<TableRef>)("SQL_COND_ON_TABLE"<SqlCondOnTable>)
   ("KEY_ATTRIB"<key_attrib>)("ENT_KEY_ATTRIB"<ent_key_attrib>)
   ("AGGR_FACTOR_ATTRIB"<aggr_factor_attrib>)
   ("COORDINATE"<SRID>)("GEOM_DIM"<geom_dim>)("GEOM_ATTRIB"<geom_attrib>)
   [("X_ATTRIB"<x_attrib>)("Y_ATTRIB"<y_attrib>)[("Z_ATTRIB"<z_attrib>)]]
   [("VERTEX_PARENT_ATTRIB"<vertex_parent_attrib>)(BULGE_ATTRIB<bulge_attrib>)
   ("ROTATION_ATTRIB"<rotation_attrib>)("ROTATION_UNIT"<rotation_unit>)
   ("LAYER_ATTRIB"<layer_attrib>)("COLOR_ATTRIB"<color_attrib>)("COLOR_FORMAT"<color_format>)
   ("THICKNESS_ATTRIB"<thickness_attrib>)
   ("LINE_TYPE_ATTRIB"<line_type_attrib>)("LINE_TYPE_SCALE_ATTRIB"<line_type_scale_attrib>)
   ("WIDTH_ATTRIB"<width_attrib>)
   ("HATCH_ATTRIB"<hatch_attrib>)("HATCH_LAYER_ATTRIB"<hatch_layer_attrib>)
   ("HATCH_COLOR_ATTRIB"<hatch_color_attrib>)("HATCH_SCALE_ATTRIB"<hatch_scale_attrib>)
   ("HATCH_ROTATION_ATTRIB"<hatch_rotation_attrib>)
   ("TEXT_ATTRIB"<text_attrib>)("TEXT_STYLE_ATTRIB"<text_style_attrib>)
   ("H_TEXT_ATTRIB"<h_text_attrib>)("HORIZ_ALIGN_ATTRIB"<horiz_align_attrib>)
   ("VERT_ALIGN_ATTRIB"<vert_align_attrib>)("OBLIQUE_ANGLE_ATTRIB"<oblique_angle_attrib>)
   ("BLOCK_ATTRIB"<block_attrib>)("BLOCK_SCALE_ATTRIB"<block_scale_attrib>))

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_getGeomDBLink(void)
{
   presbuf        arg = acedGetArgs();
   C_RB_LIST      ret;
   CGeomDBLinkDlg GeomDBLinkDlg;

   acedRetNil();

   if (arg)
      if (GeomDBLinkDlg.FromRb(arg) == GS_BAD) return RTERROR;

 	if (GeomDBLinkDlg.DoModal() == IDOK)
      if ((ret << GeomDBLinkDlg.ToRb()))
         ret.LspRetList();

   return RTNORM;
}

// CGeomDBLinkDlg dialog

IMPLEMENT_DYNAMIC(CGeomDBLinkDlg, CDialog)

CGeomDBLinkDlg::CGeomDBLinkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGeomDBLinkDlg::IDD, pParent)
{
   ClassType = -1;
   GeomNumericFormat = false;
   geom_dim = GS_3D;
}

CGeomDBLinkDlg::~CGeomDBLinkDlg()
{
}

void CGeomDBLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_UDL_EDIT, m_UdlEdit);
   DDX_Control(pDX, IDC_FULLREFTABLE_EDIT, m_FullReTableEdit);
   DDX_Control(pDX, IDC_SQL_COND_EDIT, m_SqlCondOnTableEdit);

   DDX_Control(pDX, IDC_GEOM_UNIQUE_ID_COMBO, m_key_attribCombo);
   DDX_Control(pDX, IDC_ENTITY_ID_COMBO, m_ent_key_attribCombo);
   DDX_Control(pDX, IDC_AGGR_FACTOR_COMBO, m_aggr_factor_attribCombo);

   DDX_Control(pDX, IDC_N_DIM_COMBO, m_geom_dimCombo);

   DDX_Control(pDX, IDC_GEOM_FIELD_RADIO, m_geom_field_Radio);
   DDX_Control(pDX, IDC_GEOM_FIELD_COMBO, m_geom_attribCombo);
   
   DDX_Control(pDX, IDC_NUMERIC_FIELD_RADIO, m_numeric_field_Radio);
   DDX_Control(pDX, IDC_X_FIELD_COMBO, m_x_attribCombo);
   DDX_Control(pDX, IDC_Y_FIELD_COMBO, m_y_attribCombo);
   DDX_Control(pDX, IDC_Z_FIELD_COMBO, m_z_attribCombo);
   DDX_Control(pDX, IDC_ROT_COMBO, m_rotation_attribCombo);
   DDX_Control(pDX, IDC_ROT_UNIT_COMBO, m_rotation_unitCombo);
   DDX_Control(pDX, IDC_BULGE_FIELD_COMBO, m_bulge_attribCombo);
   DDX_Control(pDX, IDC_LINE_ID_COMBO, m_vertex_parent_attribCombo);

   DDX_Control(pDX, IDC_LAYER_COMBO, m_layer_attribCombo);
   DDX_Control(pDX, IDC_COLOR_COMBO, m_color_attribCombo);
   DDX_Control(pDX, IDC_COLOR_FMT_COMBO, m_color_formatCombo);
   DDX_Control(pDX, IDC_THICKNESS_COMBO, m_thickness_attribCombo);

   DDX_Control(pDX, IDC_LINETYPE_COMBO, m_line_type_attribCombo);
   DDX_Control(pDX, IDC_LINETYPE_SCALE_COMBO, m_line_type_scale_attribCombo);
   DDX_Control(pDX, IDC_LINETYPE_WIDTH_COMBO, m_width_attribCombo);

   DDX_Control(pDX, IDC_TEXT_COMBO, m_text_attribCombo);
   DDX_Control(pDX, IDC_H_TEXT_COMBO, m_h_text_attribCombo);
   DDX_Control(pDX, IDC_TEXT_STYLE_COMBO, m_text_style_attribCombo);
   DDX_Control(pDX, IDC_HORIZ_ALIGN_COMBO, m_horiz_align_attribCombo);
   DDX_Control(pDX, IDC_VERT_ALIGN_COMBO, m_vert_align_attribCombo);
   DDX_Control(pDX, IDC_OBLIQUE_ANGLE_COMBO, m_oblique_angle_attribCombo);

   DDX_Control(pDX, IDC_BLOCK_COMBO, m_block_attribCombo);
   DDX_Control(pDX, IDC_BLOCK_SCALE_COMBO, m_block_scale_attribCombo);

   DDX_Control(pDX, IDC_HATCH_COMBO, m_hatch_attribCombo);
   DDX_Control(pDX, IDC_HATCH_LAYER_COMBO, m_hatch_layer_attribCombo);
   DDX_Control(pDX, IDC_HATCH_COLOR_COMBO, m_hatch_color_attribCombo);
   DDX_Control(pDX, IDC_HATCH_SCALE_COMBO, m_hatch_scale_attribCombo);
   DDX_Control(pDX, IDC_HATCH_ROT_COMBO, m_hatch_rotation_attribCombo);
}


BEGIN_MESSAGE_MAP(CGeomDBLinkDlg, CDialog)
   ON_BN_CLICKED(IDC_DATA_SOURCE_BUTTON, &CGeomDBLinkDlg::OnBnClickedDataSourceButton)
   ON_BN_CLICKED(IDOK, &CGeomDBLinkDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &CGeomDBLinkDlg::OnBnClickedHelp)
   ON_CBN_SELCHANGE(IDC_GEOM_UNIQUE_ID_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeGeomUniqueIdCombo)
   ON_CBN_SELCHANGE(IDC_ENTITY_ID_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeEntityIdCombo)
   ON_CBN_SELCHANGE(IDC_N_DIM_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeNDimCombo)
   ON_CBN_SELCHANGE(IDC_GEOM_FIELD_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeGeomFieldCombo)
   ON_BN_CLICKED(IDC_GEOM_FIELD_RADIO, &CGeomDBLinkDlg::OnBnClickedGeomFieldRadio)
   ON_BN_CLICKED(IDC_NUMERIC_FIELD_RADIO, &CGeomDBLinkDlg::OnBnClickedNumericFieldRadio)
   ON_CBN_SELCHANGE(IDC_X_FIELD_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeXFieldCombo)
   ON_CBN_SELCHANGE(IDC_Y_FIELD_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeYFieldCombo)
   ON_CBN_SELCHANGE(IDC_Z_FIELD_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeZFieldCombo)
   ON_CBN_SELCHANGE(IDC_BULGE_FIELD_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeBulgeFieldCombo)
   ON_CBN_SELCHANGE(IDC_LINE_ID_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeLineIdCombo)
   ON_CBN_SELCHANGE(IDC_ROT_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeRotCombo)
   ON_CBN_SELCHANGE(IDC_ROT_UNIT_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeRotUnitCombo)
   ON_CBN_SELCHANGE(IDC_LAYER_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeLayerCombo)
   ON_CBN_SELCHANGE(IDC_COLOR_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeColorCombo)
   ON_CBN_SELCHANGE(IDC_THICKNESS_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeThicknessCombo)
   ON_CBN_SELCHANGE(IDC_LINETYPE_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeLinetypeCombo)
   ON_CBN_SELCHANGE(IDC_LINETYPE_SCALE_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeLinetypeScaleCombo)
   ON_CBN_SELCHANGE(IDC_LINETYPE_WIDTH_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeLinetypeWidthCombo)
   ON_CBN_SELCHANGE(IDC_HATCH_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeHatchCombo)
   ON_CBN_SELCHANGE(IDC_HATCH_LAYER_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeHatchLayerCombo)
   ON_CBN_SELCHANGE(IDC_HATCH_COLOR_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeHatchColorCombo)
   ON_CBN_SELCHANGE(IDC_HATCH_SCALE_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeHatchScaleCombo)
   ON_CBN_SELCHANGE(IDC_HATCH_ROT_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeHatchRotCombo)
   ON_CBN_SELCHANGE(IDC_TEXT_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeTextCombo)
   ON_CBN_SELCHANGE(IDC_H_TEXT_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeHTextCombo)
   ON_CBN_SELCHANGE(IDC_TEXT_STYLE_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeTextStyleCombo)
   ON_CBN_SELCHANGE(IDC_HORIZ_ALIGN_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeHorizAlignCombo)
   ON_CBN_SELCHANGE(IDC_VERT_ALIGN_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeVertAlignCombo)
   ON_CBN_SELCHANGE(IDC_OBLIQUE_ANGLE_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeObliqueAngleCombo)
   ON_CBN_SELCHANGE(IDC_BLOCK_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeBlockCombo)
   ON_CBN_SELCHANGE(IDC_BLOCK_SCALE_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeBlockScaleCombo)
   ON_EN_CHANGE(IDC_SQL_COND_EDIT, &CGeomDBLinkDlg::OnEnChangeSqlCondEdit)
   ON_CBN_SELCHANGE(IDC_AGGR_FACTOR_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeAggrFactorCombo)
   ON_CBN_SELCHANGE(IDC_COLOR_FMT_COMBO, &CGeomDBLinkDlg::OnCbnSelchangeColorFmtCombo)
END_MESSAGE_MAP()

BOOL CGeomDBLinkDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   m_geom_dimCombo.AddString(_T("2"));
   m_geom_dimCombo.AddString(_T("3"));

   // 1=DEG_CW, 2=DEG_CCW, 3=RAD_CW, 4=RAD_CCW, 5=GONS_CW, 6=GONS_CCW, 7=GONS_TB
   // CW -> senso orario; CCW -> senso antiorario
   m_rotation_unitCombo.AddString(_T("Gradi in senso orario"));
   m_rotation_unitCombo.AddString(_T("Gradi in senso antiorario"));
   m_rotation_unitCombo.AddString(_T("Radianti in senso orario"));
   m_rotation_unitCombo.AddString(_T("Radianti in senso antiorario"));
   m_rotation_unitCombo.AddString(_T("Gradi centesimali in senso orario"));
   m_rotation_unitCombo.AddString(_T("Gradi centesimali in senso antiorario"));
   m_rotation_unitCombo.AddString(_T("Gradi centesimali topobase"));
   
   // 1=GSAutoCADColorIndexFormatColor, 2=GSHTMLFormatColor, 3=GSHexFormatColor,
   // 4=GSRGBDecColonFormatColor, 5=GSRGBDecBlankFormatColor, 6=GSRGBDXFFormatColor
   // 7=GSRGBCOLORREFFormatColor
   m_color_formatCombo.AddString(_T("Codice AutoCAD [0-256]"));
   m_color_formatCombo.AddString(_T("HTML (Es. #FFFFFF)"));
   m_color_formatCombo.AddString(_T("Esadecimale (Es. FFFFFF)"));
   m_color_formatCombo.AddString(_T("RGB separati da , (Es. 255,255,255)"));
   m_color_formatCombo.AddString(_T("RGB separati da spazio (Es. 255 255 255)"));
   m_color_formatCombo.AddString(_T("RGB decimale"));
   m_color_formatCombo.AddString(_T("BGR decimale"));

   RefreshAttribList();
   RefreshSelection();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGeomDBLinkDlg::RefreshAttribList()
{
   C_ATTRIB       *pAttrib;
   C_DBCONNECTION *pConn;
   C_ATTRIB_LIST  SrcStru;

   m_UdlEdit.SetWindowText(ConnStrUDLFile.get_name());
   m_FullReTableEdit.SetWindowText(TableRef.get_name());
   m_SqlCondOnTableEdit.SetWindowText(SqlCondOnTable.get_name());

   m_NumericAttribNameList.remove_all();
   m_CharAttribNameList.remove_all();
   m_GeomAttribNameList.remove_all();

   // svuoto le combo
   while (m_key_attribCombo.DeleteString(0) != CB_ERR);
   while (m_ent_key_attribCombo.DeleteString(0) != CB_ERR);
   while (m_aggr_factor_attribCombo.DeleteString(0) != CB_ERR);

   while (m_geom_attribCombo.DeleteString(0) != CB_ERR);
   while (m_x_attribCombo.DeleteString(0) != CB_ERR);
   while (m_y_attribCombo.DeleteString(0) != CB_ERR);
   while (m_z_attribCombo.DeleteString(0) != CB_ERR);
   while (m_bulge_attribCombo.DeleteString(0) != CB_ERR);
   while (m_vertex_parent_attribCombo.DeleteString(0) != CB_ERR);
   while (m_rotation_attribCombo.DeleteString(0) != CB_ERR);

   while (m_layer_attribCombo.DeleteString(0) != CB_ERR);
   while (m_color_attribCombo.DeleteString(0) != CB_ERR);
   while (m_thickness_attribCombo.DeleteString(0) != CB_ERR);

   while (m_line_type_attribCombo.DeleteString(0) != CB_ERR);
   while (m_line_type_scale_attribCombo.DeleteString(0) != CB_ERR);
   while (m_width_attribCombo.DeleteString(0) != CB_ERR);

   while (m_text_attribCombo.DeleteString(0) != CB_ERR);
   while (m_text_style_attribCombo.DeleteString(0) != CB_ERR);
   while (m_h_text_attribCombo.DeleteString(0) != CB_ERR);
   while (m_horiz_align_attribCombo.DeleteString(0) != CB_ERR);
   while (m_vert_align_attribCombo.DeleteString(0) != CB_ERR);
   while (m_oblique_angle_attribCombo.DeleteString(0) != CB_ERR);

   while (m_block_attribCombo.DeleteString(0) != CB_ERR);
   while (m_block_scale_attribCombo.DeleteString(0) != CB_ERR);
   
   while (m_hatch_attribCombo.DeleteString(0) != CB_ERR);
   while (m_hatch_layer_attribCombo.DeleteString(0) != CB_ERR);
   while (m_hatch_color_attribCombo.DeleteString(0) != CB_ERR);
   while (m_hatch_scale_attribCombo.DeleteString(0) != CB_ERR);
   while (m_hatch_rotation_attribCombo.DeleteString(0) != CB_ERR);

   // inserisco una riga vuota ai controlli di valori opzionali
   m_aggr_factor_attribCombo.AddString(_T(""));

   m_bulge_attribCombo.AddString(_T(""));
   m_rotation_attribCombo.AddString(_T(""));

   m_layer_attribCombo.AddString(_T(""));
   m_color_attribCombo.AddString(_T(""));
   m_thickness_attribCombo.AddString(_T(""));

   m_line_type_attribCombo.AddString(_T(""));
   m_line_type_scale_attribCombo.AddString(_T(""));
   m_width_attribCombo.AddString(_T(""));

   m_text_style_attribCombo.AddString(_T(""));
   m_h_text_attribCombo.AddString(_T(""));
   m_horiz_align_attribCombo.AddString(_T(""));
   m_vert_align_attribCombo.AddString(_T(""));
   m_oblique_angle_attribCombo.AddString(_T(""));

   m_block_scale_attribCombo.AddString(_T(""));

   m_hatch_attribCombo.AddString(_T(""));
   m_hatch_layer_attribCombo.AddString(_T(""));
   m_hatch_color_attribCombo.AddString(_T(""));
   m_hatch_scale_attribCombo.AddString(_T(""));
   m_hatch_rotation_attribCombo.AddString(_T(""));

   // Verifico la connessione OLE-DB
   if (ConnStrUDLFile.get_name() && TableRef.get_name() &&
       (pConn = get_pDBCONNECTION_LIST()->get_Connection(ConnStrUDLFile.get_name(),
                                                         &UDLProperties,
                                                         false,
                                                         GS_BAD)) != NULL)
      if (SrcStru.from_DB(pConn, pConn, TableRef.get_name()) == GS_GOOD)
      {
         pAttrib = (C_ATTRIB *) SrcStru.get_head();

         while (pAttrib)
         {
            m_GeomAttribNameList.add_tail_str(pAttrib->name.get_name());
            m_geom_attribCombo.AddString(pAttrib->name.get_name());

            if (gsc_DBIsNumeric(pAttrib->ADOType) == GS_GOOD) // se è numerico
            {
               m_NumericAttribNameList.add_tail_str(pAttrib->name.get_name());

               m_key_attribCombo.AddString(pAttrib->name.get_name());
               m_ent_key_attribCombo.AddString(pAttrib->name.get_name());
               m_aggr_factor_attribCombo.AddString(pAttrib->name.get_name());

               m_x_attribCombo.AddString(pAttrib->name.get_name());
               m_y_attribCombo.AddString(pAttrib->name.get_name());
               m_z_attribCombo.AddString(pAttrib->name.get_name());
               m_bulge_attribCombo.AddString(pAttrib->name.get_name());
               m_vertex_parent_attribCombo.AddString(pAttrib->name.get_name());
               m_rotation_attribCombo.AddString(pAttrib->name.get_name());

               m_thickness_attribCombo.AddString(pAttrib->name.get_name());

               m_line_type_scale_attribCombo.AddString(pAttrib->name.get_name());
               m_width_attribCombo.AddString(pAttrib->name.get_name());

               m_h_text_attribCombo.AddString(pAttrib->name.get_name());
               m_horiz_align_attribCombo.AddString(pAttrib->name.get_name());
               m_vert_align_attribCombo.AddString(pAttrib->name.get_name());
               m_oblique_angle_attribCombo.AddString(pAttrib->name.get_name());

               m_block_scale_attribCombo.AddString(pAttrib->name.get_name());

               m_hatch_scale_attribCombo.AddString(pAttrib->name.get_name());
               m_hatch_rotation_attribCombo.AddString(pAttrib->name.get_name());
            }
            else
            if (gsc_DBIsChar(pAttrib->ADOType) == GS_GOOD) // se è carattere
            {
               m_CharAttribNameList.add_tail_str(pAttrib->name.get_name());

               m_color_attribCombo.AddString(pAttrib->name.get_name());
               m_layer_attribCombo.AddString(pAttrib->name.get_name());

               m_line_type_attribCombo.AddString(pAttrib->name.get_name());

               m_text_attribCombo.AddString(pAttrib->name.get_name());
               m_text_style_attribCombo.AddString(pAttrib->name.get_name());

               m_block_attribCombo.AddString(pAttrib->name.get_name());

               m_hatch_attribCombo.AddString(pAttrib->name.get_name());
               m_hatch_color_attribCombo.AddString(pAttrib->name.get_name());
               m_hatch_layer_attribCombo.AddString(pAttrib->name.get_name());
            }

            pAttrib = (C_ATTRIB *) SrcStru.get_next();
         }
      }

   m_NumericAttribNameList.sort_name(); // li ordino in modo crescente
   m_CharAttribNameList.sort_name();    // li ordino in modo crescente
   m_GeomAttribNameList.sort_name();    // li ordino in modo crescente

   m_geom_dimCombo.SetCurSel((geom_dim == GS_3D) ? 1 : 0);

   // 1=DEG_CW, 2=DEG_CCW, 3=RAD_CW, 4=RAD_CCW, 5=GONS_CW, 6=GONS_CCW
   m_rotation_unitCombo.SetCurSel((int) rotation_unit - 1);

   // 1=GSAutoCADColorIndexFormatColor, 2=GSHTMLFormatColor, 3=GSHexFormatColor,
   // 4=GSRGBDecColonFormatColor, 5=GSRGBDecBlankFormatColor, 6=GSRGBDXFFormatColor
   // 7=GSRGBCOLORREFFormatColor
   m_color_formatCombo.SetCurSel((int) color_format - 1);

   // verifico se i valori di default dei nomi degli attributi esistono nella struttura
   C_DBGPH_INFO TestGphInfo;
   int          Pos;

   // key_attrib
   if ((Pos = m_NumericAttribNameList.getpos_name(key_attrib.get_name(), FALSE)) > 0)
      m_key_attribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.key_attrib.get_name(), FALSE)) > 0)
         { m_key_attribCombo.SetCurSel(Pos - 1); key_attrib = TestGphInfo.key_attrib; }
      else
         key_attrib.clear();
   
   // ent_key_attrib
   if ((Pos = m_NumericAttribNameList.getpos_name(ent_key_attrib.get_name(), FALSE)) > 0)
      m_ent_key_attribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.ent_key_attrib.get_name(), FALSE)) > 0)
         { m_ent_key_attribCombo.SetCurSel(Pos - 1); ent_key_attrib = TestGphInfo.ent_key_attrib; }
      else
         ent_key_attrib.clear();

   // aggr_factor
   if ((Pos = m_NumericAttribNameList.getpos_name(aggr_factor_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_aggr_factor_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.aggr_factor_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_aggr_factor_attribCombo.SetCurSel(Pos); aggr_factor_attrib = TestGphInfo.aggr_factor_attrib; }
      else
         aggr_factor_attrib.clear();

   // geom
   if ((Pos = m_GeomAttribNameList.getpos_name(geom_attrib.get_name(), FALSE)) > 0)
   {
      m_geom_attribCombo.SetCurSel(Pos - 1);
      OnCbnSelchangeGeomFieldCombo();
   }
   else
      if ((Pos = m_GeomAttribNameList.getpos_name(TestGphInfo.geom_attrib.get_name(), FALSE)) > 0)
      {
         m_geom_attribCombo.SetCurSel(Pos - 1);
         OnCbnSelchangeGeomFieldCombo();
      }
      else
         geom_attrib.clear();

   // x
   if ((Pos = m_NumericAttribNameList.getpos_name(x_attrib.get_name(), FALSE)) > 0)
      m_x_attribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.x_attrib.get_name(), FALSE)) > 0)
         { m_x_attribCombo.SetCurSel(Pos - 1); x_attrib = TestGphInfo.x_attrib; }
      else
         x_attrib.clear();

   // y
   if ((Pos = m_NumericAttribNameList.getpos_name(y_attrib.get_name(), FALSE)) > 0)
      m_y_attribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.y_attrib.get_name(), FALSE)) > 0)
         { m_y_attribCombo.SetCurSel(Pos - 1); y_attrib = TestGphInfo.y_attrib; }
      else
         y_attrib.clear();

   // z
   if ((Pos = m_NumericAttribNameList.getpos_name(z_attrib.get_name(), FALSE)) > 0)
      m_z_attribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.z_attrib.get_name(), FALSE)) > 0)
         { m_z_attribCombo.SetCurSel(Pos - 1); z_attrib = TestGphInfo.z_attrib; }
      else
         z_attrib.clear();

   // bulge (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(bulge_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_bulge_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.bulge_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_bulge_attribCombo.SetCurSel(Pos); bulge_attrib = TestGphInfo.bulge_attrib; }
      else
         bulge_attrib.clear();

   // vertex_parent
   if ((Pos = m_NumericAttribNameList.getpos_name(vertex_parent_attrib.get_name(), FALSE)) > 0)
      m_vertex_parent_attribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.vertex_parent_attrib.get_name(), FALSE)) > 0)
         { m_vertex_parent_attribCombo.SetCurSel(Pos - 1); vertex_parent_attrib = TestGphInfo.vertex_parent_attrib; }
      else
         vertex_parent_attrib.clear();

   // rotation (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(rotation_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_rotation_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.rotation_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_rotation_attribCombo.SetCurSel(Pos); rotation_attrib = TestGphInfo.rotation_attrib; }
      else
         rotation_attrib.clear();

   // layer_attrib (prima riga vuota)
   if ((Pos = m_CharAttribNameList.getpos_name(layer_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_layer_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.layer_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_layer_attribCombo.SetCurSel(Pos); layer_attrib = TestGphInfo.layer_attrib; }
      else
         layer_attrib.clear();

   // color (prima riga vuota)
   if ((Pos = m_CharAttribNameList.getpos_name(color_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_color_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.color_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_color_attribCombo.SetCurSel(Pos); color_attrib = TestGphInfo.color_attrib; }
      else
         color_attrib.clear();

   // thickness (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(thickness_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_thickness_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.thickness_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_thickness_attribCombo.SetCurSel(Pos); thickness_attrib = TestGphInfo.thickness_attrib; }
      else
         thickness_attrib.clear();

   // line_type (prima riga vuota)
   if ((Pos = m_CharAttribNameList.getpos_name(line_type_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_line_type_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.line_type_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_line_type_attribCombo.SetCurSel(Pos); line_type_attrib = TestGphInfo.line_type_attrib; }
      else
         line_type_attrib.clear();

   // line_type_scale (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(line_type_scale_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_line_type_scale_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.line_type_scale_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_line_type_scale_attribCombo.SetCurSel(Pos); line_type_scale_attrib = TestGphInfo.line_type_scale_attrib; }
      else
         line_type_scale_attrib.clear();

   // width (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(width_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_width_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.width_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_width_attribCombo.SetCurSel(Pos); width_attrib = TestGphInfo.width_attrib; }
      else
         width_attrib.clear();

   // hatch (prima riga vuota)
   if ((Pos = m_CharAttribNameList.getpos_name(hatch_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_hatch_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.hatch_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_hatch_attribCombo.SetCurSel(Pos); hatch_attrib = TestGphInfo.hatch_attrib; }
      else
         hatch_attrib.clear();

   // hatch_layer (prima riga vuota)
   if ((Pos = m_CharAttribNameList.getpos_name(hatch_layer_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_hatch_layer_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.hatch_layer_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_hatch_layer_attribCombo.SetCurSel(Pos); hatch_layer_attrib = TestGphInfo.hatch_layer_attrib; }
      else
         hatch_layer_attrib.clear();

   // hatch_color (prima riga vuota)
   if ((Pos = m_CharAttribNameList.getpos_name(hatch_color_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_hatch_color_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.hatch_color_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_hatch_color_attribCombo.SetCurSel(Pos); hatch_color_attrib = TestGphInfo.hatch_color_attrib; }
      else
         hatch_color_attrib.clear();

   // hatch_scale (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(hatch_scale_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_hatch_scale_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.hatch_scale_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_hatch_scale_attribCombo.SetCurSel(Pos); hatch_scale_attrib = TestGphInfo.hatch_scale_attrib; }
      else
         hatch_scale_attrib.clear();

   // hatch_rotation (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(hatch_rotation_attrib.get_name(), FALSE)) > 0)
      m_hatch_rotation_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.hatch_rotation_attrib.get_name(), FALSE)) > 0)
         { m_hatch_rotation_attribCombo.SetCurSel(Pos); hatch_rotation_attrib = TestGphInfo.hatch_rotation_attrib; }
      else
         hatch_rotation_attrib.clear();

   // text
   if ((Pos = m_CharAttribNameList.getpos_name(text_attrib.get_name(), FALSE)) > 0)
      m_text_attribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.text_attrib.get_name(), FALSE)) > 0)
         { m_text_attribCombo.SetCurSel(Pos - 1); text_attrib = TestGphInfo.text_attrib; }
      else
         text_attrib.clear();

   // text_style (prima riga vuota)
   if ((Pos = m_CharAttribNameList.getpos_name(text_style_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_text_style_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.text_style_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_text_style_attribCombo.SetCurSel(Pos); text_style_attrib = TestGphInfo.text_style_attrib; }
      else
         text_style_attrib.clear();

   // h_text (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(h_text_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_h_text_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.h_text_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_h_text_attribCombo.SetCurSel(Pos); h_text_attrib = TestGphInfo.h_text_attrib; }
      else
         h_text_attrib.clear();

   // horiz_align (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(horiz_align_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_horiz_align_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.horiz_align_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_horiz_align_attribCombo.SetCurSel(Pos); horiz_align_attrib = TestGphInfo.horiz_align_attrib; }
      else
         horiz_align_attrib.clear();

   // vert_align (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(vert_align_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_vert_align_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.vert_align_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_vert_align_attribCombo.SetCurSel(Pos); vert_align_attrib = TestGphInfo.vert_align_attrib; }
      else
         vert_align_attrib.clear();

   // oblique_angle (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(oblique_angle_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_oblique_angle_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.oblique_angle_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_oblique_angle_attribCombo.SetCurSel(Pos); oblique_angle_attrib = TestGphInfo.oblique_angle_attrib; }
      else
         oblique_angle_attrib.clear();

   // block
   if ((Pos = m_CharAttribNameList.getpos_name(block_attrib.get_name(), FALSE)) > 0)
      m_block_attribCombo.SetCurSel(Pos - 1);
   else
      if ((Pos = m_CharAttribNameList.getpos_name(TestGphInfo.block_attrib.get_name(), FALSE)) > 0)
         { m_block_attribCombo.SetCurSel(Pos - 1); block_attrib = TestGphInfo.block_attrib; }
      else
         block_attrib.clear();

   // block_scale (prima riga vuota)
   if ((Pos = m_NumericAttribNameList.getpos_name(block_scale_attrib.get_name(), FALSE)) > 0) // 1-indexed
      m_block_scale_attribCombo.SetCurSel(Pos);
   else
      if ((Pos = m_NumericAttribNameList.getpos_name(TestGphInfo.block_scale_attrib.get_name(), FALSE)) > 0) // 1-indexed
         { m_block_scale_attribCombo.SetCurSel(Pos); block_scale_attrib = TestGphInfo.block_scale_attrib; }
      else
         block_scale_attrib.clear();
}

// CGeomDBLinkDlg message handlers

void CGeomDBLinkDlg::OnBnClickedDataSourceButton()
{
   CDBConnDlg MyDlg;

   // se non specificato prova a caricare l'ultima connessione usata
   if (ConnStrUDLFile.len() == 0 && UDLProperties.get_count() == 0)
      MyDlg.FromRbList(NULL);
   else
   {
      MyDlg.UdlFile = ConnStrUDLFile;
      UDLProperties.copy(MyDlg.UdlProperties);
   }
   MyDlg.FullRefTable = TableRef;
   MyDlg.Flags = CONN_DLG_CHOICE_ON_TABLE | CONN_DLG_CHOICE_ON_VIEW;
   if (MyDlg.DoModal() == IDOK)
   {
      ConnStrUDLFile = MyDlg.UdlFile;
      MyDlg.UdlProperties.copy(UDLProperties);
      TableRef = MyDlg.FullRefTable;
      RefreshAttribList();
   }
}

int CGeomDBLinkDlg::FromRb(presbuf rb)
{
   presbuf  p;

   if ((p = gsc_CdrAssoc(_T("TYPE"), rb, FALSE))) // class type
      if (gsc_rb2Int(p, &ClassType) == GS_BAD) return GS_BAD;

   if ((p = gsc_CdrAssoc(_T("UDL_FILE"), rb, FALSE)))
      if (p->restype == RTSTR)
      {
         ConnStrUDLFile = p->resval.rstring;
         ConnStrUDLFile.alltrim();
         // traduco dir assoluto in dir relativo
         if (gsc_nethost2drive(ConnStrUDLFile) == GS_BAD) return GS_BAD;
      }
      else
         ConnStrUDLFile.clear();

   if ((p = gsc_CdrAssoc(_T("UDL_PROP"), rb, FALSE)))
      if (p->restype == RTSTR) // Le proprietà sono in forma di stringa
      {
         if (gsc_PropListFromConnStr(p->resval.rstring, UDLProperties) == GS_BAD)
            return GS_BAD;
      }
      else
      if (p->restype == RTLB) // Le proprietà sono in forma di lista
      {
         if (gsc_getUDLProperties(&p, UDLProperties) == GS_BAD) return GS_BAD;
      }
      else
         UDLProperties.remove_all();

   // Conversione path UDLProperties da assoluto in dir relativo
   if (gsc_UDLProperties_nethost2drive(ConnStrUDLFile.get_name(), UDLProperties) == GS_BAD)
      return GS_BAD;

   C_DBCONNECTION *pConn;
   
   if ((pConn = get_pDBCONNECTION_LIST()->get_Connection(ConnStrUDLFile.get_name(),
                                                         &UDLProperties,
                                                         false,
                                                         GS_BAD)) != NULL)
      if ((p = gsc_CdrAssoc(_T("TABLE_REF"), rb, FALSE)))
         if (p->restype == RTSTR)
         {
            // Conversione riferimento tabella da dir relativo in assoluto
            if (TableRef.paste(pConn->FullRefTable_nethost2drive(p->resval.rstring)) == NULL)
               return GS_BAD;
         }
         else
         if (p->restype == RTLB)
         {  // (<cat><sch><tab>)
            C_STRING Catalog, Schema, Table;

            if (!(p = p->rbnext)) return GS_BAD;
            if (p->restype == RTSTR) Catalog = p->resval.rstring;
            if (!(p = p->rbnext)) return GS_BAD;
            if (p->restype == RTSTR) Schema = p->resval.rstring;
            if (!(p = p->rbnext)) return GS_BAD;
            if (p->restype == RTSTR) Table = p->resval.rstring;
            if (TableRef.paste(pConn->get_FullRefTable(Catalog, Schema, Table)) == NULL)
               return GS_BAD;
         }
         else
            TableRef.clear();

   if ((p = gsc_CdrAssoc(_T("SQL_COND_ON_TABLE"), rb, FALSE)))
      if (p->restype == RTSTR) SqlCondOnTable = p->resval.rstring;
      else SqlCondOnTable.clear();

   if ((p = gsc_CdrAssoc(_T("KEY_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) key_attrib = p->resval.rstring;
      else key_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("ENT_KEY_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) ent_key_attrib = p->resval.rstring;
      else ent_key_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("AGGR_FACTOR_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) aggr_factor_attrib = p->resval.rstring;
      else aggr_factor_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("COORDINATE"), rb, FALSE)))
      if (p->restype == RTSTR) SRID = p->resval.rstring;
      else SRID.clear();

   if ((p = gsc_CdrAssoc(_T("GEOM_DIM"), rb, FALSE)))
      if (gsc_rb2Int(p, (int *) &geom_dim) == GS_BAD) geom_dim = GS_2D;

   if ((p = gsc_CdrAssoc(_T("GEOM_NUMERIC_FMT"), rb, FALSE)))
      gsc_rb2Bool(p, &GeomNumericFormat);

   if ((p = gsc_CdrAssoc(_T("GEOM_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) geom_attrib = p->resval.rstring;
      else geom_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("X_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) x_attrib = p->resval.rstring;
      else x_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("Y_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) y_attrib = p->resval.rstring;
      else y_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("Z_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) z_attrib = p->resval.rstring;
      else z_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("ROTATION_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) rotation_attrib = p->resval.rstring;
      else rotation_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("ROTATION_UNIT"), rb, FALSE)))
      gsc_rb2Int(p, (int *) &rotation_unit);

   if ((p = gsc_CdrAssoc(_T("VERTEX_PARENT_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) vertex_parent_attrib = p->resval.rstring;
      else vertex_parent_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("BULGE_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) bulge_attrib = p->resval.rstring;
      else bulge_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("LAYER_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) layer_attrib = p->resval.rstring;
      else layer_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("COLOR_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) color_attrib = p->resval.rstring;
      else color_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("COLOR_FORMAT"), rb, FALSE)))
      gsc_rb2Int(p, (int *) &color_format);

   if ((p = gsc_CdrAssoc(_T("THICKNESS_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) thickness_attrib = p->resval.rstring;
      else thickness_attrib.clear();

   if (ClassType == TYPE_TEXT || ClassType == TYPE_SPAGHETTI)
      if ((p = gsc_CdrAssoc(_T("TEXT_ATTRIB"), rb, FALSE)))
         if (p->restype == RTSTR) text_attrib = p->resval.rstring;
         else text_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("LINE_TYPE_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) line_type_attrib = p->resval.rstring;
      else line_type_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("LINE_TYPE_SCALE_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) line_type_scale_attrib = p->resval.rstring;
      else line_type_scale_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("WIDTH_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) width_attrib = p->resval.rstring;
      else width_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("HATCH_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) hatch_attrib = p->resval.rstring;
      else hatch_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("HATCH_LAYER_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) hatch_layer_attrib = p->resval.rstring;
      else hatch_layer_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("HATCH_COLOR_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) hatch_color_attrib = p->resval.rstring;
      else hatch_color_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("HATCH_SCALE_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) hatch_scale_attrib = p->resval.rstring;
      else hatch_scale_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("HATCH_ROTATION_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) hatch_rotation_attrib = p->resval.rstring;
      else hatch_rotation_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("TEXT_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) text_attrib = p->resval.rstring;
      else text_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("TEXT_STYLE_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) text_style_attrib = p->resval.rstring;
      else text_style_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("H_TEXT_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) h_text_attrib = p->resval.rstring;
      else h_text_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("HORIZ_ALIGN_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) horiz_align_attrib = p->resval.rstring;
      else horiz_align_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("VERT_ALIGN_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) vert_align_attrib = p->resval.rstring;
      else vert_align_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("OBLIQUE_ANGLE_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) oblique_angle_attrib = p->resval.rstring;
      else oblique_angle_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("BLOCK_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) block_attrib = p->resval.rstring;
      else block_attrib.clear();

   if ((p = gsc_CdrAssoc(_T("BLOCK_SCALE_ATTRIB"), rb, FALSE)))
      if (p->restype == RTSTR) block_scale_attrib = p->resval.rstring;
      else block_scale_attrib.clear();

   return GS_GOOD;
}
   
presbuf CGeomDBLinkDlg::ToRb(void)
{
   C_RB_LIST List;
   C_STRING  Buffer;

   if ((List << acutBuildList(RTLB,
                                 RTSTR, _T("TYPE"),
                                 RTSHORT, ClassType,
                              RTLE, 0)) == NULL) return NULL;

   if ((List += acutBuildList(RTLB,
                                 RTSTR, _T("UDL_FILE"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(ConnStrUDLFile)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("UDL_PROP"),
                              0)) == NULL) return NULL;
   Buffer.paste(gsc_PropListToConnStr(UDLProperties));  
   if ((List += gsc_str2rb(Buffer)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("TABLE_REF"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(TableRef)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("SQL_COND_ON_TABLE"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(SqlCondOnTable)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("KEY_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(key_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("ENT_KEY_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(ent_key_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("AGGR_FACTOR_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(aggr_factor_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("COORDINATE"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(SRID)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("GEOM_DIM"),
                                 RTSHORT, (int) geom_dim,
                              0)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("GEOM_NUMERIC_FMT"),
                                 (GeomNumericFormat) ? RTT : RTNIL,
                              0)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("GEOM_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(geom_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("X_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(x_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("Y_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(y_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("Z_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(z_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("ROTATION_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(rotation_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("ROTATION_UNIT"),
                                 RTSHORT, (int) rotation_unit,
                              0)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("VERTEX_PARENT_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(vertex_parent_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("BULGE_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(bulge_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("LAYER_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(layer_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("COLOR_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(color_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("COLOR_FORMAT"),
                                 RTSHORT, (int) color_format,
                              0)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("THICKNESS_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(thickness_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("TEXT_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(text_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("LINE_TYPE_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(line_type_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("LINE_TYPE_SCALE_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(line_type_scale_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("WIDTH_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(width_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("HATCH_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(hatch_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("HATCH_LAYER_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(hatch_layer_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("HATCH_COLOR_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(hatch_color_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("HATCH_SCALE_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(hatch_scale_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("HATCH_ROTATION_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(hatch_rotation_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("TEXT_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(text_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("TEXT_STYLE_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(text_style_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("H_TEXT_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(h_text_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("HORIZ_ALIGN_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(horiz_align_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("VERT_ALIGN_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(vert_align_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("OBLIQUE_ANGLE_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(oblique_angle_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("BLOCK_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(block_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE,
                              RTLB,
                                 RTSTR, _T("BLOCK_SCALE_ATTRIB"),
                              0)) == NULL) return NULL;
   if ((List += gsc_str2rb(block_scale_attrib)) == NULL) return NULL;

   if ((List += acutBuildList(RTLE, 0)) == NULL) return NULL;
   List.ReleaseAllAtDistruction(GS_BAD);

   return List.get_head();
}

void CGeomDBLinkDlg::OnBnClickedOk()
{
   if (key_attrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un campo per l'identificatore univoco delle geometrie."));
      return;
   }

   if (ent_key_attrib.get_name() == NULL)
   {
      gsui_alert(_T("Scegliere un campo per l'identificatore delle entità."));
      return;
   }

   if (GeomNumericFormat) // geometria sotto forma di campi numerici
   {
      if (x_attrib.get_name() == NULL)
      {
         gsui_alert(_T("Scegliere un campo per la coordinata X della geometria."));
         return;
      }

      if (y_attrib.get_name() == NULL)
      {
         gsui_alert(_T("Scegliere un campo per la coordinata Y della geometria."));
         return;
      }

      if (geom_dim == GS_3D)
         if (z_attrib.get_name() == NULL)
         {
            gsui_alert(_T("Scegliere un campo per la coordinata Z della geometria."));
            return;
         }

      if (ClassType == TYPE_POLYLINE)
         if (vertex_parent_attrib.get_name() == NULL)
         {
            gsui_alert(_T("Scegliere un campo per l'identificatore delle linee."));
            return;
         }
   }
   else // campo geometrico
      if (geom_attrib.get_name() == NULL)
      {
         gsui_alert(_T("Scegliere un campo per la geometria."));
         return;
      }

   if (ClassType == TYPE_TEXT)
      if (text_attrib.get_name() == NULL)
      {
         gsui_alert(_T("Scegliere un campo per il valore del testo."));
         return;
      }

   OnOK();
}

void CGeomDBLinkDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Caratteristichegrafichedaticollegati);
}

void CGeomDBLinkDlg::OnCbnSelchangeGeomUniqueIdCombo()
{
   if (m_key_attribCombo.GetCurSel() == CB_ERR) return;
   key_attrib = m_NumericAttribNameList.getptr_at(m_key_attribCombo.GetCurSel() + 1)->get_name();
}

void CGeomDBLinkDlg::OnCbnSelchangeEntityIdCombo()
{
   if (m_ent_key_attribCombo.GetCurSel() == CB_ERR) return;
   ent_key_attrib = m_NumericAttribNameList.getptr_at(m_ent_key_attribCombo.GetCurSel() + 1)->get_name();
}

void CGeomDBLinkDlg::OnCbnSelchangeAggrFactorCombo()
{
   if (m_aggr_factor_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_aggr_factor_attribCombo.GetCurSel() == 0) aggr_factor_attrib.clear(); // la prima è una riga vuota
   else aggr_factor_attrib = m_NumericAttribNameList.getptr_at(m_aggr_factor_attribCombo.GetCurSel())->get_name();
}

void CGeomDBLinkDlg::OnCbnSelchangeNDimCombo()
{
   if (m_geom_dimCombo.GetCurSel() == CB_ERR || m_geom_dimCombo.GetCurSel() == 0)
      geom_dim = GS_2D;
   else
      geom_dim = GS_3D;

   RefreshSelection();
}

void CGeomDBLinkDlg::RefreshSelection()
{
   if (GeomNumericFormat) // geometria in formato numerico
   {
      m_numeric_field_Radio.SetCheck(BST_CHECKED);
      m_geom_field_Radio.SetCheck(BST_UNCHECKED);
   }
   else
   {
      m_geom_field_Radio.SetCheck(BST_CHECKED);
      m_numeric_field_Radio.SetCheck(BST_UNCHECKED);
   }

   switch (ClassType)
   {
      case TYPE_SURFACE:
         EnableSurfaceCtrls();
         break;
      case TYPE_POLYLINE:
         EnableLineCtrls();
         break;
      case TYPE_TEXT:
         EnableTxtCtrls();
         break;
      case TYPE_NODE:
         EnableBlockCtrls();
         break;
      case TYPE_SPAGHETTI:
         EnableLineCtrls();
         EnableSurfaceCtrls();
         EnableTxtCtrls();
         EnableBlockCtrls();
         m_aggr_factor_attribCombo.EnableWindow(FALSE); // gli spaghetti non hanno fattore di aggregazione
         GeomNumericFormat = false; // gli spaghetti non possono avere geometria in campi numerici
         m_numeric_field_Radio.EnableWindow(FALSE);
         EnableNumericGeomCtrls(FALSE);

         break;
   }
}

void CGeomDBLinkDlg::EnableNumericGeomCtrls(BOOL bEnable)
{
   m_geom_attribCombo.EnableWindow(!bEnable);

   m_x_attribCombo.EnableWindow(bEnable);
   m_y_attribCombo.EnableWindow(bEnable);
   m_z_attribCombo.EnableWindow((geom_dim == GS_3D) ? bEnable : FALSE);
   m_bulge_attribCombo.EnableWindow((ClassType == TYPE_POLYLINE) ? bEnable : FALSE);
   m_vertex_parent_attribCombo.EnableWindow((ClassType == TYPE_POLYLINE) ? bEnable : FALSE);
}

void CGeomDBLinkDlg::EnableLineCtrls()
{
   m_numeric_field_Radio.EnableWindow(TRUE);
   EnableNumericGeomCtrls((GeomNumericFormat) ? TRUE : FALSE);

   m_thickness_attribCombo.EnableWindow(TRUE);

   m_line_type_attribCombo.EnableWindow(TRUE);
   m_line_type_scale_attribCombo.EnableWindow(TRUE);
   m_width_attribCombo.EnableWindow(TRUE);
}

void CGeomDBLinkDlg::EnableTxtCtrls()
{
   m_numeric_field_Radio.EnableWindow(TRUE);
   EnableNumericGeomCtrls((GeomNumericFormat) ? TRUE : FALSE);
   m_rotation_attribCombo.EnableWindow(TRUE);
   m_rotation_unitCombo.EnableWindow(TRUE);

   m_thickness_attribCombo.EnableWindow(TRUE);

   m_text_attribCombo.EnableWindow(TRUE);
   m_text_style_attribCombo.EnableWindow(TRUE);
   m_h_text_attribCombo.EnableWindow(TRUE);
   m_horiz_align_attribCombo.EnableWindow(TRUE);
   m_vert_align_attribCombo.EnableWindow(TRUE);
   m_oblique_angle_attribCombo.EnableWindow(TRUE);
}

void CGeomDBLinkDlg::EnableBlockCtrls()
{
   m_numeric_field_Radio.EnableWindow(TRUE);
   EnableNumericGeomCtrls((GeomNumericFormat) ? TRUE : FALSE);
   m_rotation_attribCombo.EnableWindow(TRUE);
   m_rotation_unitCombo.EnableWindow(TRUE);

   m_block_attribCombo.EnableWindow(TRUE);
   m_block_scale_attribCombo.EnableWindow(TRUE);
}

void CGeomDBLinkDlg::EnableSurfaceCtrls()
{
   m_line_type_attribCombo.EnableWindow(TRUE);
   m_line_type_scale_attribCombo.EnableWindow(TRUE);
   m_width_attribCombo.EnableWindow(TRUE);

   m_thickness_attribCombo.EnableWindow(TRUE);

   m_hatch_attribCombo.EnableWindow(TRUE);
   m_hatch_layer_attribCombo.EnableWindow(TRUE);
   m_hatch_color_attribCombo.EnableWindow(TRUE);
   m_hatch_scale_attribCombo.EnableWindow(TRUE);
   m_hatch_rotation_attribCombo.EnableWindow(TRUE);
}

void CGeomDBLinkDlg::OnBnClickedGeomFieldRadio()
{
   GeomNumericFormat = false;
   RefreshSelection();
}

void CGeomDBLinkDlg::OnBnClickedNumericFieldRadio()
{
   GeomNumericFormat = true;
   RefreshSelection();
}

void CGeomDBLinkDlg::OnCbnSelchangeGeomFieldCombo()
{
   C_STRING _SRID;
   int      _NDim;

   if (m_geom_attribCombo.GetCurSel() == CB_ERR) return;
   geom_attrib = m_GeomAttribNameList.getptr_at(m_geom_attribCombo.GetCurSel() + 1)->get_name();

   C_DBCONNECTION *pConn;

   // Verifico la connessione OLE-DB
   if (ConnStrUDLFile.get_name() && TableRef.get_name() &&
       (pConn = get_pDBCONNECTION_LIST()->get_Connection(ConnStrUDLFile.get_name(),
                                                         &UDLProperties,
                                                         false,
                                                         GS_BAD)) != NULL)
   {
      C_STRING Catalog, Schema, Table;

      // Nome completo tabella
      if (pConn->split_FullRefTable(TableRef.get_name(), Catalog, Schema, Table) == GS_GOOD)
         if (pConn->get_SRIDFromField(Schema, Table, geom_attrib,
                                      _SRID, &_NDim) == GS_GOOD)
         {
            SRID = _SRID;
            // Se si conosce anche il n. di dimensioni
            if (_NDim == 2 && _NDim != (int) geom_dim) 
               { geom_dim = GS_2D; RefreshAttribList(); }
            else if (_NDim == 3 && _NDim != (int) geom_dim)
               { geom_dim = GS_3D; RefreshAttribList(); }
         }
   }
}

void CGeomDBLinkDlg::OnCbnSelchangeXFieldCombo()
{
   if (m_x_attribCombo.GetCurSel() == CB_ERR) return;
   x_attrib = m_NumericAttribNameList.getptr_at(m_x_attribCombo.GetCurSel() + 1)->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeYFieldCombo()
{
   if (m_y_attribCombo.GetCurSel() == CB_ERR) return;
   y_attrib = m_NumericAttribNameList.getptr_at(m_y_attribCombo.GetCurSel() + 1)->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeZFieldCombo()
{
   if (m_z_attribCombo.GetCurSel() == CB_ERR) return;
   z_attrib = m_NumericAttribNameList.getptr_at(m_z_attribCombo.GetCurSel() + 1)->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeBulgeFieldCombo()
{
   if (m_bulge_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_bulge_attribCombo.GetCurSel() == 0) bulge_attrib.clear(); // la prima è una riga vuota
   else bulge_attrib = m_NumericAttribNameList.getptr_at(m_bulge_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeLineIdCombo()
{
   if (m_vertex_parent_attribCombo.GetCurSel() == CB_ERR) return;
   vertex_parent_attrib = m_NumericAttribNameList.getptr_at(m_vertex_parent_attribCombo.GetCurSel() + 1)->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeRotCombo()
{
   if (m_rotation_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_rotation_attribCombo.GetCurSel() == 0) rotation_attrib.clear(); // la prima è una riga vuota
   else rotation_attrib = m_NumericAttribNameList.getptr_at(m_rotation_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeRotUnitCombo()
{
   if (m_rotation_unitCombo.GetCurSel() == CB_ERR) return;
   rotation_unit = (RotationMeasureUnitsEnum) (m_rotation_unitCombo.GetCurSel() + 1);
}

void CGeomDBLinkDlg::OnCbnSelchangeLayerCombo()
{
   if (m_layer_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_layer_attribCombo.GetCurSel() == 0) layer_attrib.clear(); // la prima è una riga vuota
   else layer_attrib = m_CharAttribNameList.getptr_at(m_layer_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeColorCombo()
{
   if (m_color_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_color_attribCombo.GetCurSel() == 0) color_attrib.clear(); // la prima è una riga vuota
   else color_attrib = m_CharAttribNameList.getptr_at(m_color_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeColorFmtCombo()
{
   if (m_color_formatCombo.GetCurSel() == CB_ERR) return;
   color_format = (GSFormatColorEnum) (m_color_formatCombo.GetCurSel() + 1);
}
void CGeomDBLinkDlg::OnCbnSelchangeThicknessCombo()
{
   if (m_thickness_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_thickness_attribCombo.GetCurSel() == 0) thickness_attrib.clear(); // la prima è una riga vuota
   else thickness_attrib = m_NumericAttribNameList.getptr_at(m_thickness_attribCombo.GetCurSel())->get_name();
}

void CGeomDBLinkDlg::OnCbnSelchangeLinetypeCombo()
{
   if (m_line_type_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_line_type_attribCombo.GetCurSel() == 0) line_type_attrib.clear(); // la prima è una riga vuota
   else line_type_attrib = m_CharAttribNameList.getptr_at(m_line_type_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeLinetypeScaleCombo()
{
   if (m_line_type_scale_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_line_type_scale_attribCombo.GetCurSel() == 0) line_type_scale_attrib.clear(); // la prima è una riga vuota
   else line_type_scale_attrib = m_NumericAttribNameList.getptr_at(m_line_type_scale_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeLinetypeWidthCombo()
{
   if (m_width_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_width_attribCombo.GetCurSel() == 0) width_attrib.clear(); // la prima è una riga vuota
   else width_attrib = m_NumericAttribNameList.getptr_at(m_width_attribCombo.GetCurSel())->get_name();
}

void CGeomDBLinkDlg::OnCbnSelchangeHatchCombo()
{
   if (m_hatch_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_hatch_attribCombo.GetCurSel() == 0) hatch_attrib.clear(); // la prima è una riga vuota
   else hatch_attrib = m_CharAttribNameList.getptr_at(m_hatch_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeHatchLayerCombo()
{
   if (m_hatch_layer_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_hatch_layer_attribCombo.GetCurSel() == 0) hatch_layer_attrib.clear(); // la prima è una riga vuota
   else hatch_layer_attrib = m_CharAttribNameList.getptr_at(m_hatch_layer_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeHatchColorCombo()
{
   if (m_hatch_color_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_hatch_color_attribCombo.GetCurSel() == 0) hatch_color_attrib.clear(); // la prima è una riga vuota
   else hatch_color_attrib = m_CharAttribNameList.getptr_at(m_hatch_color_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeHatchScaleCombo()
{
   if (m_hatch_scale_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_hatch_scale_attribCombo.GetCurSel() == 0) hatch_scale_attrib.clear(); // la prima è una riga vuota
   else hatch_scale_attrib = m_NumericAttribNameList.getptr_at(m_hatch_scale_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeHatchRotCombo()
{
   if (m_hatch_rotation_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_hatch_rotation_attribCombo.GetCurSel() == 0) hatch_rotation_attrib.clear(); // la prima è una riga vuota
   else hatch_rotation_attrib = m_NumericAttribNameList.getptr_at(m_hatch_rotation_attribCombo.GetCurSel())->get_name();
}

void CGeomDBLinkDlg::OnCbnSelchangeTextCombo()
{
   if (m_text_attribCombo.GetCurSel() == CB_ERR) return;
   text_attrib = m_CharAttribNameList.getptr_at(m_text_attribCombo.GetCurSel() + 1)->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeHTextCombo()
{
   if (m_h_text_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_h_text_attribCombo.GetCurSel() == 0) h_text_attrib.clear(); // la prima è una riga vuota
   else h_text_attrib = m_NumericAttribNameList.getptr_at(m_h_text_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeTextStyleCombo()
{
   if (m_text_style_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_text_style_attribCombo.GetCurSel() == 0) text_style_attrib.clear(); // la prima è una riga vuota
   else text_style_attrib = m_CharAttribNameList.getptr_at(m_text_style_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeHorizAlignCombo()
{
   if (m_horiz_align_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_horiz_align_attribCombo.GetCurSel() == 0) horiz_align_attrib.clear(); // la prima è una riga vuota
   else horiz_align_attrib = m_NumericAttribNameList.getptr_at(m_horiz_align_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeVertAlignCombo()
{
   if (m_vert_align_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_vert_align_attribCombo.GetCurSel() == 0) vert_align_attrib.clear(); // la prima è una riga vuota
   else vert_align_attrib = m_NumericAttribNameList.getptr_at(m_vert_align_attribCombo.GetCurSel())->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeObliqueAngleCombo()
{
   if (m_oblique_angle_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_oblique_angle_attribCombo.GetCurSel() == 0) oblique_angle_attrib.clear(); // la prima è una riga vuota
   else oblique_angle_attrib = m_NumericAttribNameList.getptr_at(m_oblique_angle_attribCombo.GetCurSel())->get_name();
}

void CGeomDBLinkDlg::OnCbnSelchangeBlockCombo()
{
   if (m_block_attribCombo.GetCurSel() == CB_ERR) return;
   block_attrib = m_CharAttribNameList.getptr_at(m_block_attribCombo.GetCurSel() + 1)->get_name();
}
void CGeomDBLinkDlg::OnCbnSelchangeBlockScaleCombo()
{
   if (m_block_scale_attribCombo.GetCurSel() == CB_ERR) return;
   if (m_block_scale_attribCombo.GetCurSel() == 0) block_scale_attrib.clear(); // la prima è una riga vuota
   else block_scale_attrib = m_NumericAttribNameList.getptr_at(m_block_scale_attribCombo.GetCurSel())->get_name();
}

void CGeomDBLinkDlg::OnEnChangeSqlCondEdit()
{
   // TODO:  If this is a RICHEDIT control, the control will not
   // send this notification unless you override the CDialog::OnInitDialog()
   // function and call CRichEditCtrl().SetEventMask()
   // with the ENM_CHANGE flag ORed into the mask.
   CString dummy;
   m_SqlCondOnTableEdit.GetWindowText(dummy);
   SqlCondOnTable = dummy.Trim();
}
