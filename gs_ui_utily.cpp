/**********************************************************
Name: gs_ui_utily

Module description: File contenente funzioni di utilità di GEOUI (GEOsim User Interface)
            
Author: Caterina Gaeta & Poltini Roberto

(c) Copyright 2002-2012 by IREN ACQUA GAS  S.p.A.

              
Modification history:
              
Notes and restrictions on use: 


**********************************************************/


/*********************************************************/
/* INCLUDES */
/*********************************************************/


#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

                     
#include <stdio.h>
#include <direct.h>
#include <stdlib.h>
#include <io.h>

#include "rxmfcapi.h"

#include "ads.h"
#include "actrans.h"

#include "gs_list.h"
#include "gs_def.h"       // definizioni globali
#include "gs_error.h"     // codici errori
#include "gs_resbf.h"     // gestione resbuf
#include "gs_init.h"      // direttori globali
#include "gs_utily.h" 
#include "gs_lisp.h"

#include "resource.h"

#include "GEOUIAppl.h"
#include "gs_ui_utily.h"
#include "gs_ui_option.h"
#include "gs_ui_organizer.h"
#include "gs_ui_user.h"
#include "ValuesListDlg.h"

#include "DlgTest.h"

#include "d2hMap.h" // doc to help

#define DSPDxax 0x00E20746L  


/*************************************************************************/
/*.doc gsc_register_control                                             */
/*  
    Questa funzione registra un controllo
    Parametri: 
    TCHAR    &file_ocx    percorso completo di installazione del controllo
    int      reg         =1 registratione forzata
*/
/*************************************************************************/
int gsc_register_control(const TCHAR *file_ocx, int reg) 
{
   int res = GS_GOOD;
   HINSTANCE hDLL = NULL;
   int reg_found = 0;

   do
   {
      /*
      if (reg == 0)//controllo se è già registrato
      {
         do
         {
            //se lo è già esco dalla funzione
            HKEY hKeyClsid, hKeyX, hKeyControl;
            LONG regResult = ERROR_SUCCESS, lSize;
            DWORD dwIndex = 0;
            TCHAR szClsidName[MAX_PATH+1];
            TCHAR szBuffer[MAX_PATH*2];

            regResult = ::RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", &hKeyClsid);
            if (regResult != ERROR_SUCCESS) //warning - errore nella lettura dei registri
            { break;}
            
            while ((::RegEnumKey(hKeyClsid, dwIndex++, szClsidName, MAX_PATH + 1 ) == ERROR_SUCCESS)
                    &&  (reg_found == 0))
            {
               regResult = ::RegOpenKey(hKeyClsid, szClsidName, &hKeyX);
               if (regResult != ERROR_SUCCESS) //warning - errore nella lettura dei registri
               { break;}

               //controllo che ci sia la key "Control"
               regResult = ::RegOpenKey(hKeyX, "Control", &hKeyControl);
               if (regResult == ERROR_SUCCESS)
               { 
                  //è un controllo
                  lSize = sizeof(szBuffer);
                  ::RegQueryValue(hKeyX, "InProcServer32", szBuffer, &lSize);
                  
                  if (gsc_strcmp(szBuffer, file_ocx, FALSE) == 0)
                  //se è quello cercato:
                     reg_found = 1;
                  
                  ::RegCloseKey(hKeyControl);
               }
               ::RegCloseKey(hKeyX);
            }
            ::RegCloseKey(hKeyClsid);
         
         }
         while (0);
      }
      */

      //se reg == 1 (registrazione forzata)
      //oppure il controllo non è ancora registrato lo registro 
      if (!reg_found) 
      {
         hDLL = AfxLoadLibrary(file_ocx);
         if (NULL == hDLL) {res = GS_BAD; break;}

          typedef HRESULT (CALLBACK *HCRET)(void);
          HCRET lpfnDllRegisterServer;

          lpfnDllRegisterServer =
                  (HCRET)GetProcAddress(hDLL, "DllRegisterServer");
          if(NULL == lpfnDllRegisterServer)
          {res = GS_BAD; break;}
   
          if(FAILED((*lpfnDllRegisterServer)()))
          {res = GS_BAD; break;}
      }
   }
   while (0);

   return res;
}

/*************************************************************************/
/*.doc gsc_register_all_controls                                         */
/*  
    Questa funzione registra i controlli in GEOUI_PATH
    Parametri: 
    int      reg         =1 registratione forzata
*/
/*************************************************************************/
int gsc_register_all_controls(int reg) 
{
   int res = GS_GOOD;
   C_STRING dir_ocx, dir_mask, file_ocx;
   presbuf filename;

   do
   {
      dir_ocx = get_WORKDIR();
      dir_ocx += _T("\\");
      dir_ocx += GEOUI_PATH;
      
      if (gsc_dir_exist(dir_ocx) == GS_GOOD)
      {
         dir_mask = dir_ocx;
         dir_mask += _T("\\*.ocx");
         //ottengo l'elenco dei files nella cartella GEOUI
         if (gsc_adir(dir_mask.get_name(), &filename) > 0)
         {
            while (filename != NULL)
            {
               while ((filename != NULL) && (filename->restype != RTSTR))
                  filename = filename->rbnext;
               if (filename == NULL) break;
               
               file_ocx = dir_ocx;
               file_ocx += _T("\\");
               file_ocx += filename->resval.rstring;

               if (gsc_register_control(file_ocx.get_name(), reg) == GS_BAD)
               {
                  res = GS_BAD; 
                  // messaggio di avvertimento
                  acutPrintf(_T("\nProblemi nella registrazione di %s\n"), file_ocx.get_name());
               }
            
               filename = filename->rbnext;
            }
         }
      }
   }
   while (0);

   if (filename) ads_relrb(filename);

   return res;
}


/*************************************************************************/
/*.doc gs_register_all_controls                                           */
/*  
    Questa funzione registra i controlli in GEOUI_PATH
    anche se sono già registrati
*/
/*************************************************************************/
int gs_register_all_controls(void) 
{
   presbuf arg = acedGetArgs();
   int reg = 1;

   if ((arg != NULL) && (arg->restype == RTSHORT))
      reg = arg->resval.rint;
      if (gsc_register_all_controls(reg) == GS_BAD) return RTERROR;

   return RTNORM;

}

/*************************************************************************/
/*.doc gsui_SetBmpColorToDlgBkColor                                      */
/*+
  Funzione per variare il colore di sfondo di una bitmap. Si può usare questa funzione quando
  si deve inserire una BMP in una finestra e cambiare il colore dello sfondo di 
  quest'ultima per adeguarlo a quello della finestra.

  La funzione restituisce TUE in caso di successo altrimenti FALSE.  
-*/
/*************************************************************************/
bool gsui_SetBmpColorToDlgBkColor(HBITMAP hBitmap, COLORREF crFrom)
{
   COLORREF crTo;

   // Get the color of the dialog background. 
   if ((crTo = GetSysColor(COLOR_BTNFACE)) == 0) return FALSE;

   return gsui_ChangeColorToBmp(hBitmap, crFrom, crTo);
}


/*************************************************************************/
/*.doc gsui_ChangeColorToBmp                                             */
/*+
  Funzione per variare il colore di una bitmap.

  La funzione restituisce TRUE in caso di successo altrimenti FALSE.  
-*/
/*************************************************************************/
bool gsui_ChangeColorToBmp(HBITMAP hBitmap, COLORREF crFrom, COLORREF crTo)
{
   register int cx, cy;     
   BITMAP       bm;     
   HDC          hdcBmp, hdcMask;     
   HBITMAP      hbmMask, hbmOld1, hbmOld2;     
   HBRUSH       hBrush, hbrOld;      
   
   if (!hBitmap) return FALSE;      

   GetObject (hBitmap, sizeof (bm), &bm);     
   cx = bm.bmWidth;     
   cy = bm.bmHeight;      
   hbmMask = CreateBitmap(cx, cy, 1, 1, NULL);     
   hdcMask = CreateCompatibleDC(NULL);     
   hdcBmp  = CreateCompatibleDC(NULL);     
   hBrush  = CreateSolidBrush(crTo);      

   if (!hdcMask || !hdcBmp || !hBrush || !hbmMask)     
   {         
      DeleteObject(hbmMask);         
      DeleteObject(hBrush);         
      DeleteDC(hdcMask);         
      DeleteDC(hdcBmp);         
      return FALSE;     
   }      
   
   hbmOld1 = (HBITMAP) SelectObject (hdcBmp,  hBitmap);     
   hbmOld2 = (HBITMAP) SelectObject (hdcMask, hbmMask);     
   hbrOld  = (HBRUSH) SelectObject (hdcBmp, hBrush);      

   SetBkColor(hdcBmp, crFrom);
   BitBlt(hdcMask, 0, 0, cx, cy, hdcBmp,  0, 0, SRCCOPY);     
   SetBkColor(hdcBmp, RGB(255,255,255));     
   BitBlt(hdcBmp,  0, 0, cx, cy, hdcMask, 0, 0, DSPDxax);      

   SelectObject(hdcBmp,  hbmOld1);     
   SelectObject(hdcMask, hbmOld2);     
   DeleteDC(hdcBmp);     
   DeleteDC(hdcMask);     
   DeleteObject(hBrush);     
   DeleteObject(hbmMask);      

   return TRUE; 
}


/*************************************************************************/
/*.doc gsui_alert                                                        */
/*+
   Visualizza un messaggio di allerta.
   Parametri:
   const TCHAR *Msg;  Messaggio
   HWND hOwnerWnd;   Handle finestra proprietaria (default = NULL)
-*/
/*************************************************************************/
int gslui_alert(void)
{
   presbuf arg = ads_getargs();
   
   acedRetVoid();
   if (!arg || arg->restype != RTSTR) return GS_BAD;
   gsui_alert(arg->resval.rstring);

   return GS_GOOD;
}
void gsui_alert(C_STRING &Msg, HWND hOwnerWnd) 
   { gsui_alert(Msg.get_name(), hOwnerWnd); }
void gsui_alert(const TCHAR *Msg, HWND hOwnerWnd) 
{ 
   HWND _hOwnerWnd;

   _hOwnerWnd = (hOwnerWnd) ? hOwnerWnd : adsw_acadMainWnd();

   MessageBox(_hOwnerWnd, Msg, _T("GEOsim"), MB_OK + MB_ICONWARNING);
} 


/*************************************************************************/
/*.doc gsui_info                                                         */
/*+
   Visualizza un messaggio informativo.
   Parametri:
   const TCHAR *Msg;  Messaggio
   HWND hOwnerWnd;   Handle finestra proprietaria (default = NULL)
-*/
/*************************************************************************/
int gslui_info(void)
{
   presbuf arg = ads_getargs();
   
   acedRetVoid();
   if (!arg || arg->restype != RTSTR) return GS_BAD;
   gsui_info(arg->resval.rstring);

   return GS_GOOD;
}
void gsui_info(C_STRING &Msg, HWND hOwnerWnd) 
   { gsui_info(Msg.get_name(), hOwnerWnd); }
void gsui_info(const TCHAR *Msg, HWND hOwnerWnd) 
{ 
   HWND _hOwnerWnd;

   _hOwnerWnd = (hOwnerWnd) ? hOwnerWnd : adsw_acadMainWnd();

   MessageBox(_hOwnerWnd, Msg, _T("GEOsim"), MB_OK + MB_ICONINFORMATION);
} 


/*************************************************************************/
/*.doc gsui_confirm                                                      */
/*+
   Richiede una conferma visualizzando un messaggio.
   Parametri:
   const TCHAR *Msg;         Messaggio
   int        Default;      se = GS_GOOD la risposta di (default GS_GOOD cioè SI)
   bool       UsePassword;  se = TRUE viene richiesta la password dell'utente corrente
                            (default = FALSE)
   bool       CancelButton; se = TRUE appare anche il bottone CANCEL (default = FALSE)
   HWND       hOwnerWnd;    Handle finestra proprietaria (default = quella di autocad)

  Ritorna GS_GOOD se risponde SI, GS_BAD se risponde NO, GS_CAN se risponde CANCEL.
-*/
/*************************************************************************/
int gslui_confirm(void)
{
   presbuf     arg = ads_getargs();
   int         Default = GS_GOOD;
   bool        UsePassword = FALSE, CancelButton = TRUE;
   const TCHAR *Msg;
   
   acedRetNil();
   if (!arg || arg->restype != RTSTR) return GS_BAD;
   Msg = arg->resval.rstring;

   if ((arg = arg->rbnext))
   {
      // Bottone di default
      if (arg->restype != RTT) Default = GS_BAD;
      // richiesta di password utente corrente
      if ((arg = arg->rbnext))
      {
         if (arg->restype == RTT) UsePassword = TRUE;
         // Flag per bottone CANCEL
         if ((arg = arg->rbnext) && arg->restype == RTNIL) CancelButton = FALSE;
      }
   }

   switch (gsui_confirm(Msg, Default, UsePassword, CancelButton))
   {
      case GS_GOOD:
         if (UsePassword && get_CURRENT_USER())
            acedRetStr(get_CURRENT_USER()->pwd);
         else
            acedRetInt(1);
         break;
      case GS_BAD:
         acedRetInt(0);
         break;
      case GS_CAN:
         acedRetInt(2);
         break;
   }

   return GS_GOOD;
}
int gsui_confirm(const TCHAR *Msg, int Default, bool UsePassword, bool CancelButton,
                 HWND hOwnerWnd) 
{ 
   HWND    _hOwnerWnd;
   INT_PTR Result = Default;

   _hOwnerWnd = (hOwnerWnd) ? hOwnerWnd : adsw_acadMainWnd();
   
   if (!UsePassword)
   {
      UINT uType = MB_ICONQUESTION;

      if (CancelButton) uType += MB_YESNOCANCEL;
      else  uType += MB_YESNO;

      uType += (Default == GS_GOOD) ? MB_DEFBUTTON1 : MB_DEFBUTTON2;

      Result = MessageBox(_hOwnerWnd, Msg, _T("GEOsim"), uType);

      if (Result == IDYES) return GS_GOOD;
      else if (Result == IDNO) return GS_BAD;
      else if (Result == IDCANCEL) return GS_CAN;
   }
   else
   {
      CClass_login ConfirmDlg;
      SetParent(ConfirmDlg.m_hWnd, _hOwnerWnd);
      ConfirmDlg.m_msg_for_password_only = Msg;

      ConfirmDlg.DefID = (Default == GS_GOOD) ? IDOK : IDCANCEL;

      Result = ConfirmDlg.DoModal();

      if (Result == IDOK) return GS_GOOD;
      else if (Result == IDCANCEL) return GS_BAD;
   }

   return (int) Result;
} 


/*************************************************************************/
/*.doc gsui_getDriveList                                                 */
/*+
   Ottiene una lista dei drive usati in ordine alfabetico.
   Parametri:
   C_LIST_STR &DriveList;  Lista dei drive usati
-*/
/*************************************************************************/
void gsui_getDriveList(C_STR_LIST &DriveList) 
{ 
   int   drive, curdrive;
   char  dummy[_MAX_DRIVE];
   C_STR *pDrive;

   HCURSOR PrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

   DriveList.remove_all();

   // Save current drive
   curdrive = _getdrive();

   // If we can switch to the drive, it exists.
   for (drive = 1; drive <= 26; drive++)
      if (!_chdrive(drive))
      {
         sprintf(dummy, "%c:", drive + 'A' - 1);
         if ((pDrive = new C_STR(dummy)) != NULL) DriveList.add_tail(pDrive);
      }

   // Restore original drive
   _chdrive(curdrive);

   SetCursor(PrevCursor);
} 


/////////////////////////////////////////////////////////////////////////////
// CMathOperatDlg dialog inizio

void gsui_GetAttribMathOpMsg(double Value, CString &Operat, bool Perc, CString &Msg)
{
   C_STRING dummy, _Msg;

   gsc_conv_Number(Value, dummy);

   if (Operat == _T("+"))      _Msg = _T("<Somma ");
   else if (Operat == _T("-")) _Msg = _T("<Sottrai ");
   else if (Operat == _T("*")) _Msg = _T("<Moltipica per ");
   else if (Operat == _T("/")) _Msg = _T("<Dividi per ");

   _Msg += dummy;

   if (Perc) _Msg += _T(" %>");
   else _Msg += _T(">");

   Msg = _Msg.get_name();
}
int gsui_GetAttribMathOp(CString &Msg, double *Value, CString &Operat, bool *Perc)
{
   CString StrValue;
   size_t  start, end = Msg.GetLength() - 1;

   // Se la stringa NON è racchiusa tra '<' e '>'
   if (end < 1 || Msg.GetAt(0) != _T('<') || Msg.GetAt((int) end) != _T('>')) return GS_BAD;

   if (Msg.Find(_T("Somma "), 1) == 1) { Operat = _T("+"); start = wcslen(_T("Somma ")) + 1; }
   else
   if (Msg.Find(_T("Sottrai "), 1) == 1) { Operat = _T("-"); start = wcslen(_T("Sottrai ")) + 1; }
   else
   if (Msg.Find(_T("Moltipica per "), 1) == 1) { Operat = _T("*"); start = wcslen(_T("Moltipica per ")) + 1; }
   else
   if (Msg.Find(_T("Dividi per "), 1) == 1) { Operat = _T("/"); start = wcslen(_T("Dividi per ")) + 1; }
   else
      return GS_BAD;
   
   end -= 1;
   if (Msg.GetAt((int) end) == _T('%'))
   {
      *Perc = TRUE;
      end -= 1;
   }
   else
      *Perc = FALSE;

   StrValue = Msg.Mid((int) start, (int) (end - start) + 1);

   if (gsc_conv_Number(StrValue, Value) == GS_BAD) return GS_BAD;

   return GS_GOOD;
}
void gsui_AddAttribMathOp(C_RB_LIST &ColValues, const TCHAR *Name,
                          double Value, CString &Operat, bool Perc)
{
   // Inserimento di nuovo attributo
   ColValues.link_head(acutBuildList(RTLB, 
                                     RTSTR, Name,
                                     RTREAL, Value,
                                     RTSTR, Operat, 
                                     (Perc) ? RTT : RTNIL, 
                                     RTLE, 0));
}
void gsui_SetAttribMathOp(C_RB_LIST &ColValues, int i,
                          double Value, CString &Operat, bool Perc)
{
   presbuf p;

   ColValues.nth(i);         // i-esimo attributo
   ColValues.get_next();     // Nome attributo
   p = ColValues.get_next(); // Valore attributo
   gsc_RbSubst(p, Value);
   if ((p = p->rbnext) && p->restype == RTSTR) // esiste già l'operatore
   {
      gsc_RbSubst(p, (LPCTSTR) Operat);
      p = p->rbnext;
      if (Perc) gsc_RbSubstT(p);
      else gsc_RbSubstNIL(p);
   }
   else // bisogna aggiungere l'operatore e se si tratta di valore percentuale
      ColValues.link_atCursor(acutBuildList(RTSTR, (LPCTSTR) Operat, 
                                            (Perc) ? RTT : RTNIL, 0));
}

int gsui_GetAttribMathOp(C_RB_LIST &ColValues, int i, 
                         double *Value, CString &Operat, bool *Perc)
{
   presbuf pColValue = gsc_nth(1, ColValues.nth(i)); // Valore i-esimo attributo

   return gsui_GetAttribMathOp(pColValue, Value, Operat, Perc);
}

int gsui_GetAttribMathOp(presbuf pValue, double *Value, CString &Operat, bool *Perc)
{
   // Valore
   if (gsc_rb2Dbl(pValue, Value) == GS_BAD) return GS_BAD;
   // Operatore
   if ((pValue = pValue->rbnext) && pValue->restype == RTSTR) Operat = pValue->resval.rstring;
   else return GS_BAD;
   // Percentuale
   if ((pValue = pValue->rbnext) && (pValue->restype == RTT || pValue->restype == RTNIL))
      *Perc = (pValue->restype == RTT) ? TRUE : FALSE;
   else return GS_BAD;

   return GS_GOOD;
}

CMathOperatDlg::CMathOperatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMathOperatDlg::IDD, pParent)
{
   C_2STR *pStrOperat;

   m_Operat = _T("+");
   m_Value  = 0;
   m_Perc   = FALSE;

   if ((pStrOperat = new C_2STR(_T("Somma"), _T("+"))) != NULL)
      StrOperatList.add_tail(pStrOperat);
   if ((pStrOperat = new C_2STR(_T("Sottrai"), _T("-"))) != NULL)
      StrOperatList.add_tail(pStrOperat);
   if ((pStrOperat = new C_2STR(_T("Moltiplica"), _T("*"))) != NULL)
      StrOperatList.add_tail(pStrOperat);
   if ((pStrOperat = new C_2STR(_T("Dividi"), _T("/"))) != NULL)
      StrOperatList.add_tail(pStrOperat);

	//{{AFX_DATA_INIT(CMathOperatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMathOperatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMathOperatDlg)
	DDX_Control(pDX, IDC_CHECK_PERC, m_CheckPerc);
	DDX_Control(pDX, IDC_EDIT_VALUE, m_EditValue);
	DDX_Control(pDX, IDC_COMBO_OPERAT, m_ComboOp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMathOperatDlg, CDialog)
	//{{AFX_MSG_MAP(CMathOperatDlg)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDC_CHECK_PERC, OnCheckPerc)
	ON_CBN_SELCHANGE(IDC_COMBO_OPERAT, OnSelchangeComboOperat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CMathOperatDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   C_2STR   *pStrOperat;
   C_STRING dummy;
   int      i = 0, CurSel = 0;

   pStrOperat = (C_2STR *) StrOperatList.get_head();
   while (pStrOperat)
   {
      m_ComboOp.AddString(pStrOperat->get_name());
      if (m_Operat.CompareNoCase(pStrOperat->get_name2()) == 0) CurSel = i;
      pStrOperat = (C_2STR *) StrOperatList.get_next();
      i++;
   }
   m_ComboOp.SetCurSel(CurSel);

   gsc_conv_Number(m_Value, dummy);
   m_EditValue.SetWindowText(dummy.get_name());

   m_CheckPerc.SetCheck((m_Perc) ? BST_CHECKED : BST_UNCHECKED);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CMathOperatDlg message handlers

void CMathOperatDlg::OnHelp() 
{
   gsc_help(IDH_Aggiornamentodegliattributi);
}

void CMathOperatDlg::OnCheckPerc() 
{
   m_Perc = (m_CheckPerc.GetCheck() == 1) ? TRUE : FALSE;	
}

void CMathOperatDlg::OnSelchangeComboOperat() 
{
   C_2STR *pStrOperat;

   if (m_ComboOp.GetCurSel() == CB_ERR) return;
   if ((pStrOperat = (C_2STR *) StrOperatList.getptr_at(m_ComboOp.GetCurSel() + 1)) != NULL)
      m_Operat = pStrOperat->get_name2();
}

void CMathOperatDlg::OnOK() 
{
	// TODO: Add extra validation here
   CString  ActualTxt;
	double   ActualNum;
   C_STRING dummy;

   m_EditValue.GetWindowText(ActualTxt);
   if (gsc_conv_Number(ActualTxt, &ActualNum) == GS_BAD)
   {
      gsui_alert(_T("Il valore deve essere un numero."), m_hWnd);
      gsc_conv_Number(m_Value, dummy);
      m_EditValue.SetWindowText(dummy.get_name());
      m_EditValue.SetFocus();
      return;
   }

   gsc_conv_Number(ActualNum, dummy);
   if (ActualTxt.Compare(dummy.get_name()) != 0)
   {
      C_STRING Msg(_T("Il valore è stato interpretato come "));
      Msg += dummy;
      Msg += _T(". Confermi ?");
      if (gsui_confirm(Msg.get_name(), GS_GOOD, FALSE, FALSE, m_hWnd) != GS_GOOD)
         return;
   }
   m_Value = ActualNum;

	CDialog::OnOK();
}

// CMathOperatDlg dialog fine
/////////////////////////////////////////////////////////////////////////////


/******************************************************************************/
/*.doc gsui_CanIRecalcOnLine                                                  */
/*+
  Questa funzione è usata nell'interfaccia grafica di modifica dei valori
  di una scheda per stabilire se, dopo la modifica di un attributo,
  sia possibile effettuare il ricalcolo di tutti gli attributi che dipendono
  dall'attributo modificato.
  Parametri:
  C_ATTRIB      *pAttrib;     Attributo modificato
  CString       &Value;       Nuovo valore dell'attributo
  C_ATTRIB_LIST *pAttribList; Lista di tutti gli attributi della classe
  C_RB_LIST     &ColValues;   Valori della scheda (senza il nuovo valore dell'attributo)
  long          EntQty;       Numero di entità che l'interfaccia grafica sta gestendo

  Ritorn TRUE se è possibile fare il ricalcolo altrimenti FALSE
-*/  
/******************************************************************************/
bool gsui_CanIRecalcOnLine(C_ATTRIB *pAttrib, CString &Value,
                           C_ATTRIB_LIST *pAttribList, C_RB_LIST &ColValues, long EntQty)
{
   C_ATTRIB *punt, *punt2;
   presbuf  pValue;

   // Verifico che abbia un valore utilizzabile
   if (!gsui_IsUsefulAttribValue(Value)) return FALSE;

   punt = (C_ATTRIB *) pAttribList->get_head();
   while (punt)
   {  // per ogni attributo che deve essere ricalcolato dalla modifica di pAttrib
      if (punt->is_calculated() == GS_GOOD &&
          gsc_is_param(punt->calc_file, punt->calc_func, pAttrib->get_name()) == GS_GOOD)
      {
         punt2 = (C_ATTRIB *) pAttribList->get_head();
         while (punt2)
         {  // per ogni attributo parametro di questa funzione di calcolo escluso pAttrib
            if (gsc_strcmp(punt2->get_name(), pAttrib->get_name()) != 0 &&
                gsc_is_param(punt->calc_file, punt->calc_func, punt2->get_name()) == GS_GOOD)
            {
               if ((pValue = ColValues.CdrAssoc(punt2->get_name())) == NULL) return FALSE;

               // Verifico che abbia un valore utilizzabile
               if (!gsui_IsUsefulAttribValue(pValue, EntQty)) return FALSE;
            }

            punt2 = (C_ATTRIB *) punt2->get_next();
         }
      }
      
      punt = (C_ATTRIB *) punt->get_next();
   }

   return TRUE;
}


/******************************************************************************/
/*.doc gsui_CanIValidOnLine                                                   */
/*+
  Questa funzione è usata nell'interfaccia grafica di modifica dei valori
  di una scheda per stabilire se, dopo la modifica di un attributo,
  sia possibile effettuare la validazione di tutti gli attributi che dipendono
  dall'attributo modificato.
  Parametri:
  C_ATTRIB      *pAttrib;     Attributo modificato
  CString       &Value;       Nuovo valore  dell'attributo
  C_ATTRIB_LIST *pAttribList; Lista di tutti gli attributi della classe

  Ritorn TRUE se è possibile fare la validazione altrimenti FALSE
-*/  
/******************************************************************************/
bool gsui_CanIValidOnLine(C_ATTRIB *pAttrib, CString &Value,
                          C_ATTRIB_LIST *pAttribList)
{
   C_ATTRIB *punt;

   // Verifico che abbia un valore utilizzabile
   if (!gsui_IsUsefulAttribValue(Value)) return FALSE;

   punt = (C_ATTRIB *) pAttribList->get_head();
   while (punt)
   {  // se esiste un attributo escluso pAttrib che deve essere validato dalla modifica 
      // di pAttrib
      if (gsc_strcmp(punt->get_name(), pAttrib->get_name()) != 0 &&
          punt->is_validated() == GS_GOOD &&
          gsc_is_param(punt->valid_file, punt->valid_func, pAttrib->get_name()) == GS_GOOD)
         return FALSE;
      
      punt = (C_ATTRIB *) punt->get_next();
   }

   return TRUE;
}


/******************************************************************************/
/*.doc gsui_IsUsefulAttribValue                                               */
/*+
  Questa funzione è usata nell'interfaccia grafica di modifica dei valori
  di una scheda per stabilire se, dopo la modifica di un attributo,
  sia possibile effettuare la validazione di tutti gli attributi che dipendono
  dall'attributo modificato.
  Parametri:
  presbuf Value;     Nuovo valore dell'attributo
  long    EntQty;    Numero di entità che l'interfaccia grafica sta gestendo
                     (default = 0)                   

  Ritorn TRUE se è possibile fare la validazione altrimenti FALSE
-*/  
/******************************************************************************/
bool gsui_IsUsefulAttribValue(CString &Value, long EntQty)
{
   double   NumValue;
   CString  Operat;
   bool     Perc;

   // Verifico che non abbia operatore matematico
   if (gsui_GetAttribMathOp(Value, &NumValue, Operat, &Perc) == GS_GOOD) return FALSE;
   // Se si sta gestendo più di una entità il valore non deve essere nullo
   // perchè vorrebbe dire che il valore differisce nelle varie entità
   if (EntQty > 1 && Value.GetLength() == 0) return FALSE;

   return TRUE;
}
bool gsui_IsUsefulAttribValue(presbuf pValue, long EntQty)
{
   double   NumValue;
   CString  Operat;
   bool     Perc;

   // Verifico che non abbia operatore matematico
   if (gsui_GetAttribMathOp(pValue, &NumValue, Operat, &Perc) == GS_GOOD) return FALSE;
   // Se si sta gestendo più di una entità il valore non deve essere nullo
   // perchè vorrebbe dire che il valore differisce nelle varie entità
   if (EntQty > 1 && 
       (pValue->restype == RTNIL || pValue->restype == RTNONE))
      return FALSE;

   return TRUE;
}


///////////////////////////////////////////////////////////////////////////////



/*************************************************************************/
/*.doc gsui_alert                                                        */
/*+
  Permette la scelta di uno SRID tramite interfaccia grafica.
  Parametri:
  (("UDL_FILE" <file UDL> || <stringa di connessione>) ("UDL_PROP" <Properties>))
  dove
  <Properties> = stringa delle proprietà | ((<prop1><value>)(<prop1><value>)...)
-*/
/*************************************************************************/
int gsui_getSRID(void)
{
   presbuf        arg = ads_getargs();
   C_DBCONNECTION *pConn;
   CValuesListDlg ListDlg;
   C_2STR_LIST    SRID_descr_list;
   C_RB_LIST      RbList;
   
   acedRetNil();

   // Legge nella lista dei parametri i riferimenti alla prima connessione OLE-DSB
   if ((pConn = gsc_getConnectionFromLisp(arg)) == NULL) return RTERROR;

   ListDlg.m_Title      = _T("GEOsim - Scelta sistema di coordinate (SRID)");
   ListDlg.m_Width      = 500;
   ListDlg.m_Msg        = _T("Scegli un valore dalla lista:");
   ListDlg.m_ColsHeader = _T("SRID;Descrizione");
   ListDlg.m_SingleSel  = TRUE;
   ListDlg.m_OriginType = CValuesListDlg::RESBUF_LIST;

   // Leggo la lista degli SRID
   if (pConn->get_SRIDList(SRID_descr_list) == GS_BAD) return RTERROR;
   ListDlg.RbList << acutBuildList(RTLB, 0);
   ListDlg.RbList += SRID_descr_list.to_rb();
   ListDlg.RbList += acutBuildList(RTLE, 0);

   if (ListDlg.DoModal() == IDOK && ListDlg.m_ValueList.get_count() == 1)
   {
      // getptr_at (1 è il primo)
      C_2STR *p = (C_2STR *) SRID_descr_list.getptr_at(ListDlg.m_ValueList.get_head()->get_key() + 1);

      if (p) acedRetStr(p->get_name());
   }

   return GS_GOOD;
}


void refreshDisplay(void)
{
	actrTransactionManager->queueForGraphicsFlush();
	actrTransactionManager->flushGraphics();
	acedUpdateDisplay();
}


/******************************************************************************/
/*.doc gsui_PSelect                                                           */
/*+
  Siccome in MAP 3D 2005 il comando Pselect non funziona, questa funzione 
  è usata per imitarne il funzionamento.
-*/  
/******************************************************************************/
int gsui_PSelect(void)
{
   AcApDocument* pDoc = acDocManager->curDocument();
   acDocManager->activateDocument(pDoc);
   acDocManager->sendStringToExecute(pDoc, _T("_.PSELECT\n"));

   POSITION viewPosition = pDoc->cDoc()->GetFirstViewPosition();
   CView *pView = pDoc->cDoc()->GetNextView(viewPosition);
   pView->SetFocus();

   //  if (gsc_callCmd(_T("_.PSELECT"), RTPICKS, ent, RTSTR, _T(""), 0) != RTNORM) return GS_GOOD;

   //ads_name ss;

   //acedSSSetFirst(NULL, NULL);
   //if (gsc_ssget(NULL, NULL, NULL, NULL, ss) != RTNORM) return GS_BAD;
   //acedSSSetFirst(NULL, NULL);
   //if (acedSSSetFirst(ss, ss) != RTNORM)
   //{
   //   acedSSFree(ss);
   //   return GS_BAD;
   //}
   //acedSSFree(ss);

   return GS_GOOD;
}


/*************************************************************************/
/*.doc gsui_getFirstExtractedClsSubCode                                  */
/*+
  Funzione che restituisce il codice e sottocodice della prima classe estratta
  (in ordine alfabetico).
  Parametri:
  int *Cls;                         ouput; codice classe
  int *Sub;                         ouput; codice sotto-classe
  int FilterCategory;               Se <> 0 codice della categoria da filtrare
                                    (default = 0)
  int FilterType;                   Se <> 0 codice del tipo da filtrare (default = 0)
  int DynSegmentationSupportedOnly; Se = GS_GOOD filtra solo le classi che supportano
                                    La segmentazione dinamica (default = GS_BAD)

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/
/*************************************************************************/
int gsui_getFirstExtractedClsSubCode(int *Cls, int *Sub,
                                     C_STRING &FilterCategory, C_STRING &FilterType,
                                     int DynSegmentationSupportedOnly)
{
   C_CLS_PUNT_LIST ExtrClsList;
   C_CLS_PUNT      *pExtrCls;

   if (!get_GS_CURRENT_WRK_SESSION()) { set_GS_ERR_COD(eGSNotCurrentSession); return GS_BAD; }

   // Ottiene la lista delle classi griglia estratte nella sessione corrente
   if (get_GS_CURRENT_WRK_SESSION()->get_pPrj()->extracted_class(ExtrClsList, 
                                                                 FilterCategory,
                                                                 FilterType,
                                                                 DynSegmentationSupportedOnly) == GS_BAD)
      return 0;
   ExtrClsList.sort_name(); // Ordino la lista alfabeticamente
   if (!(pExtrCls = (C_CLS_PUNT *) ExtrClsList.get_head())) return 0;
   *Cls = ((C_CLASS *) pExtrCls->cls)->ptr_id()->code;
   *Sub = ((C_CLASS *) pExtrCls->cls)->ptr_id()->sub_code;

   return GS_GOOD;
}


#include "gs_ui_sql.h"
#include "gs_ui_Topology.h"


int gsui_prova(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   
   //CDBConnDlg MyDlg;
   //MyDlg.Flags = CONN_DLG_CHOICE_ON_TABLE | CONN_DLG_CHOICE_ON_VIEW;
   //MyDlg.DoModal();

   C_INT_LIST SelectedCodes;
   CDlgMyTest MyDlg;
   MyDlg.DoModal();
   MyDlg.myTree.GetSelectedCodes(SelectedCodes);

   acedRetVoid();
   return GS_GOOD;
}
