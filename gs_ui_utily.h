
#ifndef _gs_ui_utily_h
#define _gs_ui_utily_h 1

#include "gs_def.h"       // definizioni globali
#include "gs_class.h" 

/////////////////////////////////////////////////////////////////////////////
// CMathOperatDlg dialog

class CMathOperatDlg : public CDialog
{
// Construction
public:
	CMathOperatDlg(CWnd* pParent = NULL);   // standard constructor
   CString m_Operat;
   double  m_Value;
   bool    m_Perc;

// Dialog Data
	//{{AFX_DATA(CMathOperatDlg)
	enum { IDD = IDD_MATHOPERAT_DLG };
	CButton	m_CheckPerc;
	CEdit	m_EditValue;
	CComboBox	m_ComboOp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMathOperatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   C_2STR_LIST StrOperatList;

	// Generated message map functions
	//{{AFX_MSG(CMathOperatDlg)
	afx_msg void OnHelp();
	afx_msg void OnCheckPerc();
	afx_msg void OnSelchangeComboOperat();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

int gsc_register_control(const TCHAR *file_ocx, int reg = 0); 
int gsc_register_all_controls(int reg = 0); 

int gs_register_all_controls(void);
bool gsui_SetBmpColorToDlgBkColor(HBITMAP hBitmap, COLORREF crFrom);
bool gsui_ChangeColorToBmp(HBITMAP hBitmap, COLORREF crFrom, COLORREF crTo);

int gslui_alert(void);
void gsui_alert(C_STRING &Msg, HWND hOwnerWnd = NULL);
void gsui_alert(const TCHAR *Msg, HWND hOwnerWnd = NULL);

int gslui_info(void);
void gsui_info(C_STRING &Msg, HWND hOwnerWnd = NULL);
void gsui_info(const TCHAR *Msg, HWND hOwnerWnd = NULL);

int gslui_confirm(void);
int gsui_confirm(const TCHAR *Msg, int Default = GS_GOOD, bool UsePassword = FALSE,
                 bool CancelButton = FALSE, HWND hOwnerWnd = NULL);

void gsui_getDriveList(C_STR_LIST &DriveList);

void gsui_GetAttribMathOpMsg(double Value, CString &Operat, bool Perc, CString &Msg);
void gsui_AddAttribMathOp(C_RB_LIST &ColValues, const TCHAR *Name,
                          double Value, CString &Operat, bool Perc);
void gsui_SetAttribMathOp(C_RB_LIST &ColValues, int i,
                          double Value, CString &Operat, bool Perc);
int gsui_GetAttribMathOp(C_RB_LIST &ColValues, int i, 
                         double *Value, CString &Operat, bool *Perc);
int gsui_GetAttribMathOp(presbuf pValue, double *Value, CString &Operat, bool *Perc);
int gsui_GetAttribMathOp(CString &Msg, double *Value, CString &Operat, bool *Perc);

bool gsui_CanIRecalcOnLine(C_ATTRIB *pAttrib, CString &Value,
                           C_ATTRIB_LIST *pAttribList, C_RB_LIST &ColValues, long EntQty);
bool gsui_CanIValidOnLine(C_ATTRIB *pAttrib, CString &Value,
                          C_ATTRIB_LIST *pAttribList);
bool gsui_IsUsefulAttribValue(CString &Value, long EntQty = 0);
bool gsui_IsUsefulAttribValue(presbuf pValue, long EntQty = 0);

int gsui_getSRID(void);

void refreshDisplay(void);

int gsui_PSelect(void);

int gsui_getFirstExtractedClsSubCode(int *Cls, int *Sub,
                                     C_STRING &FilterCategory, C_STRING &FilterType,
                                     int DynSegmentationSupportedOnly = GS_BAD);

int gsui_prova(void);

#endif
