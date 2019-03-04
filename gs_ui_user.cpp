// Class_login.cpp : implementation file
//

#include "stdafx.h"

#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "resource.h"
#include "afxdialogex.h"

#include "gs_error.h" 
#include "gs_utily.h"
#include "gs_init.h"
#include "gs_user.h"
#include "GEOUIAppl.h"
#include "gs_ui_user.h"
#include "gs_ui_utily.h"
#include "ValuesListDlg.h"

#include "d2hMap.h" // doc to help

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/*********************************************************/
/*.doc gsc_ui_copyFromUserToUser              <internal> */
/*+
  Questa funzione copia le abilitazioni da un utnete all'altro.
  Parametri:
  int SourceUserCode;      codice utente sorgente
  int DestUserCode;        codice utente destinazione

  Restituisce GS_GOOD in caso di successo altrimenti GS_BAD.
-*/  
/*********************************************************/
int gsc_ui_copyFromUserToUser(int SourceUserCode, int DestUserCode)
{
   C_INT_LIST      Commands;
   C_INT_INT_LIST  ProjectsPermissionList, ClassesPermissionList;
   C_4INT_STR_LIST SecTabsPermissionList;
   C_INT_INT       *pPrj;

   if (gsc_getusrcomm(SourceUserCode, &Commands) == GS_BAD ||
       gsc_setusrcomm(DestUserCode, &Commands) == GS_BAD)
   {
      gsui_alert(_T("Abilitazione ai comandi non copiata."));
      return GS_BAD;
   }

   if (gsc_getPersonalPrjPermissions(SourceUserCode, &ProjectsPermissionList) == GS_BAD ||
       gsc_setPersonalPrjPermissions(DestUserCode, ProjectsPermissionList) == GS_BAD)
   {
      gsui_alert(_T("Abilitazione ai progetti non copiata."));
      return GS_BAD;
   }

   pPrj = (C_INT_INT *) ProjectsPermissionList.get_head();
   while (pPrj)
   {
      if (gsc_getPersonalClassPermissions(SourceUserCode, pPrj->get_key(), &ClassesPermissionList) == GS_BAD ||
          gsc_setPersonalClassPermissions(DestUserCode, pPrj->get_key(), ClassesPermissionList) == GS_BAD)
      {
         gsui_alert(_T("Abilitazione alle classi non copiata."));
         return GS_BAD;
      }

      pPrj = (C_INT_INT *) ProjectsPermissionList.get_next();
   }

   pPrj = (C_INT_INT *) ProjectsPermissionList.get_head();
   while (pPrj)
   {
      if (gsc_getPersonalSecPermissions(SourceUserCode, pPrj->get_key(), &SecTabsPermissionList) == GS_BAD ||
          gsc_setPersonalSecPermissions(DestUserCode, pPrj->get_key(), SecTabsPermissionList) == GS_BAD)
      {
         gsui_alert(_T("Abilitazione alle tabelle secondarie non copiata."));
         return GS_BAD;
      }

      pPrj = (C_INT_INT *) ProjectsPermissionList.get_next();
   }

   return GS_GOOD;
}


/////////////////////////////////////////////////////////////////////////////
// CClass_login dialog


CClass_login::CClass_login(CWnd* pParent /*=NULL*/)
	: CDialog(CClass_login::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClass_login)
	//}}AFX_DATA_INIT
   DefID = 0;
}


void CClass_login::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClass_login)
	DDX_Control(pDX, IDC_IMAGE, m_image);
	DDX_Control(pDX, IDC_STATIC_MSG, m_label_msg);
	DDX_Control(pDX, IDC_STATIC_LOGIN, m_label_login);
	DDX_Control(pDX, IDC_PASSWORD, m_password);
	DDX_Control(pDX, IDC_LOGIN, m_login);
	DDX_Control(pDX, IDC_ANIMATE1, m_animate);
	DDX_Control(pDX, IDC_CHECK_SESSION, m_check_session);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CClass_login, CDialog)
	//{{AFX_MSG_MAP(CClass_login)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClass_login message handlers

BOOL CClass_login::OnInitDialog() 
{
	C_STRING Path(get_WORKDIR());
  
   CDialog::OnInitDialog();

   if (m_msg_for_password_only.GetLength() > 0)
   {  // Richiesta di conferma tramite autentificazione
      m_label_msg.ShowWindow(SW_SHOWNORMAL);
      m_login.ShowWindow(SW_HIDE);
      m_label_login.ShowWindow(SW_HIDE);
      m_label_msg.SetWindowText(m_msg_for_password_only);
      m_check_session.ShowWindow(SW_HIDE);
      if (GetDlgItem(IDHELP))
         ((CButton *) GetDlgItem(IDHELP))->EnableWindow(FALSE);
   }
   else
   {  // Login a GEOsim
      TCHAR LastLogin[MAX_LEN_LOGIN] = _T("");
      int   Value = GS_GOOD;

      if (gsc_getLastLogin(LastLogin) == GS_GOOD && wcslen(LastLogin) > 0)
         m_login.SetWindowText(LastLogin);

      gsc_getCheckSessionOnLogin(&Value);
      m_check_session.SetCheck((Value == GS_GOOD) ? BST_CHECKED : BST_UNCHECKED);
   }

   Path += _T("\\");
   Path += GEOUI_PATH;
   Path += _T("\\WORLD.AVI");
   m_animate.Open(Path.get_name());

   if (DefID != 0) SetDefID(DefID);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CClass_login::OnHelp() 
{
   gsc_help(IDH_ACCESSOALLEFUNZIONALITDIGEOSIM);
}

void CClass_login::OnOK() 
{
   HCURSOR PrevCrs;

   m_password.GetWindowText(_password);

   if (m_msg_for_password_only.GetLength() > 0)
   {
      // verifico che la password sia corretta
      if (get_CURRENT_USER() &&
          gsc_strcmp(_password, get_CURRENT_USER()->pwd) == 0)
         CDialog::OnOK();
      else
         gsui_alert(_T("Identificazione utente fallita."), m_hWnd);
      return;
   }

   PrevCrs = GetCursor();
   SetCursor(LoadCursor(NULL, IDC_WAIT));

   try
   {
      CString _login;
      int     FlagCheckSession;

      m_login.GetWindowText(_login);
      FlagCheckSession = (m_check_session.GetCheck() == 0) ? GS_BAD : GS_GOOD;

	   if (gsc_login(LPCTSTR(_login), LPCTSTR(_password), FlagCheckSession) == GS_BAD)
      {
         gsui_alert(_T("Identificazione utente fallita."), m_hWnd);
         AfxThrowUserException();
      }

      CDialog::OnOK();
   }

   catch (...) // any type of exception
   {}
   
   SetCursor(PrevCrs);
}

void CClass_login::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
   if (m_msg_for_password_only.GetLength() == 0)
   {
      CString _login;

      m_login.GetWindowText(_login);
      if (_login.GetLength() > 0) // login già impostata
         m_password.SetFocus();
   }


   /////////////////////////////////////////////////////

   HBITMAP   hBmp;
   COLORREF  crFrom;
	HINSTANCE Instance;

   crFrom   = RGB(255, 0, 0); // rosso
	// determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_KEYS), RT_BITMAP);
   hBmp     = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_KEYS));
   
   if (gsui_SetBmpColorToDlgBkColor(hBmp, crFrom))
      m_image.SetBitmap(hBmp);
}


/*************************************************************************/
/*.doc gsui_login                                                     */
/*+
  Comando per identificazione utente GEOsim.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_login(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   CClass_login class_login_dialog;

   acedRetVoid();
   if (get_GS_CURRENT_WRK_SESSION())
      { set_GS_ERR_COD(eGSSessionsFound); return GS_BAD; }

   //class_login_dialog.m_msg_for_password_only = "Inserire la propria password per confermare l'operazione.";
   class_login_dialog.DoModal();

   return GS_GOOD;  
}


///////////////////////////////////////////////////////////////////////////////
// FINE LOGIN
// INIZIO CREAZIONE UTENTE
///////////////////////////////////////////////////////////////////////////////


// finestra di dialogo C_CreateUsrDlg
IMPLEMENT_DYNAMIC(C_CreateUsrDlg, CDialog)

C_CreateUsrDlg::C_CreateUsrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(C_CreateUsrDlg::IDD, pParent)
   , m_UsrLevel(NORMALUSR)
{

}

C_CreateUsrDlg::~C_CreateUsrDlg()
{
}

void C_CreateUsrDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_USR_NAME, m_UsrName);
   DDX_Control(pDX, IDC_USR_PWD1, m_UsrPwd);
   DDX_Control(pDX, IDC_USR_PWD2, m_ConfirmUsrPwd);
   DDX_Control(pDX, IDC_PERMISSION_FROM_USR_CHECK, m_CopyFromUsr);
   DDX_Control(pDX, IDC_USRLIST, m_UsrList);
   DDX_Radio(pDX, IDC_SUPERUSER_RADIO, m_UsrLevel);
   DDX_Control(pDX, IDC_IMAGE, m_image);
}


BEGIN_MESSAGE_MAP(C_CreateUsrDlg, CDialog)
   ON_BN_CLICKED(IDC_SUPERUSER_RADIO, &C_CreateUsrDlg::OnBnClickedSuperuserRadio)
   ON_BN_CLICKED(IDC_USER_RADIO, &C_CreateUsrDlg::OnBnClickeduserRadio)
   ON_BN_CLICKED(IDC_PERMISSION_FROM_USR_CHECK, &C_CreateUsrDlg::OnBnClickedPermissionFromUsrCheck)
   ON_BN_CLICKED(IDOK, &C_CreateUsrDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &C_CreateUsrDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


// gestori di messaggi C_CreateUsrDlg

BOOL C_CreateUsrDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   if (gsc_getusrlist(&UsrList) == GS_GOOD)
   {
      C_INT_INT_STR *pUsr = (C_INT_INT_STR *) UsrList.get_head();
      
      while (pUsr)
      {
         if (pUsr->get_type() == 1) m_UsrList.AddString(pUsr->get_name()); // solo utenti normali
         pUsr = (C_INT_INT_STR *) UsrList.get_next();
      }
   }

   m_UsrLevel = NORMALUSR; // utente normale
   UpdateData(FALSE); // per aggiornare i radio button

   /////////////////////////////////////////////////////

   HBITMAP   hBmp;
   COLORREF  crFrom;
	HINSTANCE Instance;

   crFrom   = RGB(255, 0, 0); // rosso
	// determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_KEYS), RT_BITMAP);
   hBmp     = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_KEYS));
   
   if (gsui_SetBmpColorToDlgBkColor(hBmp, crFrom))
      m_image.SetBitmap(hBmp);

   m_UsrList.EnableWindow(FALSE);

   m_UsrName.SetFocus();

   //Specifies whether the application has set the input focus to one of the controls in the dialog box.
   //If OnInitDialog returns nonzero, Windows sets the input focus to the first control in the dialog box.
   //The application can return 0 only if it has explicitly set the input focus to one of the controls 
   //in the dialog box.
	return FALSE;
}


void C_CreateUsrDlg::OnBnClickedSuperuserRadio()
{
   m_CopyFromUsr.EnableWindow(FALSE);
   m_UsrList.EnableWindow(FALSE);
   m_UsrLevel = SUPERUSR;
}


void C_CreateUsrDlg::OnBnClickeduserRadio()
{
   m_CopyFromUsr.EnableWindow(TRUE);
   if (m_CopyFromUsr.GetCheck() == BST_CHECKED)
      m_UsrList.EnableWindow(TRUE);
   m_UsrLevel = NORMALUSR;
}


void C_CreateUsrDlg::OnBnClickedPermissionFromUsrCheck()
{
   if (m_CopyFromUsr.GetCheck() == BST_CHECKED)
      m_UsrList.EnableWindow(TRUE);
   else
      m_UsrList.EnableWindow(FALSE);
}


void C_CreateUsrDlg::OnBnClickedOk()
{
   CString _Login, _Pwd, _Pwd2;
   int     NewUserID;

   m_UsrName.GetWindowText(_Login);
   _Login.Trim();
   if (_Login.GetLength() == 0)
   {
      gsui_alert(_T("Nome utente non valido."));
      return;
   }

   m_UsrPwd.GetWindowText(_Pwd);
   if (_Pwd.GetLength() == 0)
   {
      gsui_alert(_T("Password non valida."));
      return;
   }

   m_ConfirmUsrPwd.GetWindowText(_Pwd2);
   if (_Pwd.Compare(_Pwd2) != 0)
   {
      gsui_alert(_T("Password di conferma non valida."));
      return;
   }

   if ((NewUserID = gsc_creausr(LPCTSTR(_Login), LPCTSTR(_Pwd), m_UsrLevel)) == 0)
   {
      gsui_alert(_T("Creazione utente fallita."));
      return;
   }

   if (m_CopyFromUsr.GetCheck() == BST_CHECKED)
   {
      m_UsrList.GetWindowText(_Login);
      if (_Login.GetLength() > 0)
      {
         C_INT_INT_STR *pUsr = (C_INT_INT_STR *) UsrList.search_name(LPCTSTR(_Login));

         if (!pUsr)
         {
            gsui_alert(_T("Utente da cui copiare le abilitazioni non trovato."));
            return;
         }

         if (gsc_ui_copyFromUserToUser(pUsr->get_key(), NewUserID) != GS_GOOD)
            return;
      }
   }

   CDialog::OnOK();
}


void C_CreateUsrDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Creazioneutente);
}


/*************************************************************************/
/*.doc gsui_createusr                                                    */
/*+
  Comando per creare un utente GEOsim.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_createusr(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   C_CreateUsrDlg            CreateUsrDlg;

   acedRetVoid();

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   if (CreateUsrDlg.DoModal() != IDOK)  return GS_CAN;
   
   return GS_GOOD;
}


///////////////////////////////////////////////////////////////////////////////
// FINE CREAZIONE UTENTE
// INIZIO MODIFICA PASSWORD UTENTE
///////////////////////////////////////////////////////////////////////////////


// finestra di dialogo C_ModiPwdUsrDlg
IMPLEMENT_DYNAMIC(C_ModiPwdUsrDlg, CDialog)

C_ModiPwdUsrDlg::C_ModiPwdUsrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(C_ModiPwdUsrDlg::IDD, pParent)
{

}

C_ModiPwdUsrDlg::~C_ModiPwdUsrDlg()
{
}

void C_ModiPwdUsrDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_USR_PWD1, m_UsrPwd);
   DDX_Control(pDX, IDC_USR_PWD2, m_ConfirmUsrPwd);
   DDX_Control(pDX, IDC_OLD_USR_PWD, m_OldUsrPwd);
   DDX_Control(pDX, IDC_USR_NAME, m_UsrNameStatic);
   DDX_Control(pDX, IDC_IMAGE, m_image);
}


BEGIN_MESSAGE_MAP(C_ModiPwdUsrDlg, CDialog)
   ON_BN_CLICKED(IDOK, &C_ModiPwdUsrDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &C_ModiPwdUsrDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


// gestori di messaggi C_ModiPwdUsrDlg

BOOL C_ModiPwdUsrDlg::OnInitDialog() 
{
   C_INT_INT_STR Usr;

   if (gsc_whoami(&Usr) == GS_BAD)
   {
      gsui_alert(_T("Utente corrente non esistente."));
      return FALSE;
   }

	CDialog::OnInitDialog();

   m_UsrNameStatic.SetWindowText(Usr.get_name());

   /////////////////////////////////////////////////////

   HBITMAP   hBmp;
   COLORREF  crFrom;
	HINSTANCE Instance;

   crFrom   = RGB(255, 0, 0); // rosso
	// determine location of the bitmap in resource fork
   Instance = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_KEYS), RT_BITMAP);
   hBmp     = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_KEYS));
   
   if (gsui_SetBmpColorToDlgBkColor(hBmp, crFrom))
      m_image.SetBitmap(hBmp);

	return TRUE;
}

void C_ModiPwdUsrDlg::OnBnClickedOk()
{
   CString _OldPwd, _Pwd, _Pwd2;

   m_OldUsrPwd.GetWindowText(_OldPwd);
   if (_OldPwd.GetLength() == 0)
   {
      gsui_alert(_T("Password non valida."));
      return;
   }

   m_UsrPwd.GetWindowText(_Pwd);
   if (_Pwd.GetLength() == 0)
   {
      gsui_alert(_T("Nuova password non valida."));
      return;
   }

   m_ConfirmUsrPwd.GetWindowText(_Pwd2);
   if (_Pwd.Compare(_Pwd2) != 0)
   {
      gsui_alert(_T("Password di conferma non valida."));
      return;
   }

   if (gsc_modpwd(LPCTSTR(_OldPwd), LPCTSTR(_Pwd)) == GS_BAD)
   {
      gsui_alert(_T("Modifica password fallita."));
      return;
   }   

   CDialog::OnOK();
}


void C_ModiPwdUsrDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Modificapassword);
}


/*************************************************************************/
/*.doc gsui_modipwdcurrusr                                               */
/*+
  Comando per modificare la password dell'utente corrente GEOsim.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_modipwdcurrusr(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   C_ModiPwdUsrDlg           ModiPwdUsrDlg;

   acedRetVoid();

   if (ModiPwdUsrDlg.DoModal() != IDOK)  return GS_CAN;
   
   return GS_GOOD;
}


///////////////////////////////////////////////////////////////////////////////
// FINE MODIFICA PASSWORD UTENTE
// INIZIO CANCELLAZIONE UTENTE
///////////////////////////////////////////////////////////////////////////////



/*************************************************************************/
/*.doc gsui_delusr                                                       */
/*+
  Comando per cancellare un utente GEOsim.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_delusr(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;

   acedRetVoid();
   CValuesListDlg ListDlg;
   C_INT_INT_STR_LIST UsrList;
   C_INT_INT_STR      *pUsr;
   C_STRING           Msg;
   int                i;

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   ListDlg.m_Title      = _T("GEOsim - Cancellazione utente");
   ListDlg.m_Msg        = _T("Scegli l'utente da cancellare:");
   ListDlg.m_ColsHeader = _T("Nome utente");
   ListDlg.m_SingleSel  = TRUE;
   ListDlg.m_OriginType = CValuesListDlg::RESBUF_LIST;

   ListDlg.RbList << acutBuildList(RTLB, 0);
   if (gsc_getusrlist(&UsrList) == GS_BAD) return GS_BAD;
   UsrList.sort_name();

   pUsr = (C_INT_INT_STR *) UsrList.get_head();
   while (pUsr)
   {
      // non considero l'utente corrente
      if (get_CURRENT_USER()->code != pUsr->get_key())
         ListDlg.RbList += acutBuildList(RTLB, RTSTR, pUsr->get_name(), RTLE, 0);
      pUsr = (C_INT_INT_STR *) UsrList.get_next();
   }
   ListDlg.RbList += acutBuildList(RTLE, 0);

   if (ListDlg.DoModal() != IDOK || ListDlg.m_ValueList.get_count() != 1) return GS_CAN;

   pUsr = (C_INT_INT_STR *) UsrList.get_head();
   i    = 0;
   while (pUsr)
   {
      // non considero l'utente corrente
      if (get_CURRENT_USER()->code != pUsr->get_key())
      {
         if (ListDlg.m_ValueList.get_head()->get_key() == i) break;
         i++;
      }

      pUsr = (C_INT_INT_STR *) UsrList.get_next();
   }

   Msg = _T("Confermare la cancellazione dell'utente ");
   Msg += pUsr->get_name();
   Msg += _T(".");
   if (gsui_confirm(Msg.get_name(), GS_BAD, TRUE , FALSE) == GS_GOOD)
      if (gsc_delusr(pUsr->get_key(), get_CURRENT_USER()->pwd) != GS_GOOD)
      {
         gsui_alert(_T("Cancellazione utente fallita."));
         return GS_BAD;
      }   
   
   return GS_GOOD;
}


///////////////////////////////////////////////////////////////////////////////
// FINE CANCELLAZIONE UTENTE
// INIZIO MODIFICA PASSWORD UTENTE
///////////////////////////////////////////////////////////////////////////////


// finestra di dialogo C_ModUsrDlg

IMPLEMENT_DYNAMIC(C_ModUsrDlg, CDialogEx)

C_ModUsrDlg::C_ModUsrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(C_ModUsrDlg::IDD, pParent)
{
   gsc_getusrlist(&UsrList);
   m_CurrUsrId = 0;
   Update = false;
}

C_ModUsrDlg::~C_ModUsrDlg()
{
   C_INT_VOIDPTR *p;

   p = (C_INT_VOIDPTR *) ClassesPermissionList.get_head();
   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) ClassesPermissionList.get_next();
   }

   p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_head();
   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_next();
   }
}


int C_ModUsrDlg::set_Usr(C_STRING &UsrName)
{
   C_INT_INT_STR *pUsr = (C_INT_INT_STR *) UsrList.search_name(UsrName.get_name());

   if (!pUsr) return GS_BAD;

   return set_Usr(pUsr->get_key());
}
int C_ModUsrDlg::set_Usr(int UsrId)
{
   C_INT_INT_STR *pUsr = (C_INT_INT_STR *) UsrList.search_key(UsrId);

   if (!pUsr || pUsr->get_type() != NORMALUSR) return GS_BAD;
   m_PrevUsrId = m_CurrUsrId;
   m_CurrUsrId = pUsr->get_key();

   return GS_GOOD;
}

void C_ModUsrDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialogEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_USRLIST, m_UsrName);
   DDX_Control(pDX, IDC_CMD_LIST, m_Command_Button);
   DDX_Control(pDX, IDC_DATA_LIST, m_Data_Button);
   DDX_Control(pDX, IDC_PERMISSION_FROM_USR_CHECK, m_CopyFromUsr);
   DDX_Control(pDX, IDC_USRLIST_COPYFROM, m_UsrList_CopyFrom);
   DDX_Control(pDX, IDOK, m_OK);
   DDX_Control(pDX, IDC_USERCODE, m_UsrCodeLbl);
}


BEGIN_MESSAGE_MAP(C_ModUsrDlg, CDialogEx)
   ON_BN_CLICKED(IDOK, &C_ModUsrDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDHELP, &C_ModUsrDlg::OnBnClickedHelp)
   ON_CBN_SELCHANGE(IDC_USRLIST, &C_ModUsrDlg::OnCbnSelchangeUsrlist)
   ON_BN_CLICKED(IDC_PERMISSION_FROM_USR_CHECK, &C_ModUsrDlg::OnBnClickedPermissionFromUsrCheck)
   ON_BN_CLICKED(IDC_CMD_LIST, &C_ModUsrDlg::OnBnClickedCmdList)
   ON_BN_CLICKED(IDC_DATA_LIST, &C_ModUsrDlg::OnBnClickedDataList)
END_MESSAGE_MAP()


// gestori di messaggi C_ModUsrDlg

BOOL C_ModUsrDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

   C_INT_INT_STR *pUsr = (C_INT_INT_STR *) UsrList.get_head();
   int           i = 0;
      
   while (pUsr)
   {
      if (pUsr->get_type() == 1)
      {
         m_UsrName.AddString(pUsr->get_name()); // solo utenti normali
         m_UsrName.SetItemDataPtr(m_UsrName.FindStringExact(-1, pUsr->get_name()), pUsr);
         // m_UsrName.SetItemDataPtr(i++, pUsr);
      }
      pUsr = (C_INT_INT_STR *) UsrList.get_next();
   }

   m_UsrList_CopyFrom.EnableWindow(FALSE);

   if (m_CurrUsrId > 0)
   {
      if ((pUsr = (C_INT_INT_STR *) UsrList.search_key(m_CurrUsrId)))
      m_UsrName.SetCurSel(m_UsrName.FindStringExact(-1, pUsr->get_name()));
      OnCbnSelchangeUsrlist();
   }
   else
   {
      m_UsrName.SetFocus();
      m_Command_Button.EnableWindow(FALSE);
      m_Data_Button.EnableWindow(FALSE);
      m_CopyFromUsr.EnableWindow(FALSE);
      m_OK.EnableWindow(FALSE);
   }

   return TRUE;
}


void C_ModUsrDlg::OnCbnSelchangeUsrlist()
{
   C_INT_INT_STR *pUsr;
   int           CurSel, i = 0;

   if (Update)
      if (gsui_confirm(_T("Cambiando utente saranno annullate le impostazioni correnti. Continuare ?")) != GS_GOOD)
      {
         pUsr = (C_INT_INT_STR *) UsrList.search_key(m_CurrUsrId);
         m_UsrName.SetCurSel(m_UsrName.FindStringExact(-1, pUsr->get_name()));
         return;
      }

   m_PrevUsrId = m_CurrUsrId;
   if ((CurSel = m_UsrName.GetCurSel()) == CB_ERR) return;
   pUsr = (C_INT_INT_STR *) m_UsrName.GetItemDataPtr(CurSel);
   m_CurrUsrId = pUsr->get_key();

   while (m_UsrList_CopyFrom.DeleteString(0) != CB_ERR); // svuoto la combobox

   pUsr = (C_INT_INT_STR *) UsrList.get_head();
   while (pUsr)
   {
      // solo utenti normali scartando quello corrente
      if (pUsr->get_type() == 1 && m_CurrUsrId != pUsr->get_key())
      {
         m_UsrList_CopyFrom.AddString(pUsr->get_name());
         m_UsrList_CopyFrom.SetItemDataPtr(m_UsrList_CopyFrom.FindStringExact(-1, pUsr->get_name()), pUsr);
      }
      pUsr = (C_INT_INT_STR *) UsrList.get_next();
   }
   m_UsrList_CopyFrom.SetCurSel(0);
   // carico abilitazioni
   if (LoadUsrPermission(m_CurrUsrId) == GS_BAD)
   {
      gsui_alert(_T("Abilitazioni dell'utente non caricate."));
      return;
   }

   C_STRING Msg(_T("Codice utente: "));
   Msg += m_CurrUsrId;
   m_UsrCodeLbl.SetWindowText(Msg.get_name());
   Update = false;

   m_CopyFromUsr.EnableWindow(TRUE);
   OnBnClickedPermissionFromUsrCheck();
   m_OK.EnableWindow(TRUE);
}

int C_ModUsrDlg::LoadUsrPermission(int UserId)
{
   C_INT_INT       *pPrj;
   C_INT_INT_LIST  *pClasses;
   C_4INT_STR_LIST *pSecTabs;
   C_INT_VOIDPTR   *p;

   // svuoto la lista dei permessi alle classi e alle tabelle secondarie
   p = (C_INT_VOIDPTR *) ClassesPermissionList.get_head();
   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) ClassesPermissionList.get_next();
   }
   ClassesPermissionList.remove_all();

   p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_head();
   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_next();
   }
   SecTabsPermissionList.remove_all();

   // carico abilitazione a comandi
   if (gsc_getusrcomm(UserId, &Commands) == GS_BAD) return GS_BAD;
   // carico abilitazione personale a progetti
   if (gsc_getPersonalPrjPermissions(UserId, &ProjectsPermissionList) == GS_BAD) return GS_BAD;

   // carico abilitazione personale a classi
   pPrj = (C_INT_INT *) ProjectsPermissionList.get_head();
   while (pPrj)
   {
      if ((pClasses = new C_INT_INT_LIST()) == NULL) return GS_BAD;
      if (gsc_getPersonalClassPermissions(UserId, pPrj->get_key(), pClasses) == GS_BAD)
         { delete pClasses; return GS_BAD; }
      ClassesPermissionList.add_tail_int_voidptr(pPrj->get_key(), pClasses);

      pPrj = (C_INT_INT *) ProjectsPermissionList.get_next();
   }

   // carico abilitazione personale alle tabelle secondarie
   pPrj = (C_INT_INT *) ProjectsPermissionList.get_head();
   while (pPrj)
   {
      if ((pSecTabs = new C_4INT_STR_LIST()) == NULL) return GS_BAD;
      if (gsc_getPersonalSecPermissions(UserId, pPrj->get_key(), pSecTabs) == GS_BAD)
         { delete pSecTabs; return GS_BAD; }
      SecTabsPermissionList.add_tail_int_voidptr(pPrj->get_key(), pSecTabs);

      pPrj = (C_INT_INT *) ProjectsPermissionList.get_next();
   }

   // carico i nomi degli utenti da cui ereditare le abilitazioni
   if (gsc_loadInheritanceUsers(UserId, InheritanceUserNames) == GS_BAD) return GS_BAD;

   return GS_GOOD;
}
int C_ModUsrDlg::SaveUsrPermission(int UserId)
{
   C_INT_VOIDPTR *p;

   // salvo abilitazione a comandi
   if (gsc_setusrcomm(UserId, &Commands) == GS_BAD) return GS_BAD;
   // salvo abilitazione personale a progetti
   if (gsc_setPersonalPrjPermissions(UserId, ProjectsPermissionList) == GS_BAD) return GS_BAD;
   // salvo abilitazione personale a classi
   p = (C_INT_VOIDPTR *) ClassesPermissionList.get_head();
   while (p)
   {
      if (gsc_setPersonalClassPermissions(UserId, p->get_key(), *((C_INT_INT_LIST *) p->get_VoidPtr())) == GS_BAD)
         return GS_BAD;
      p = (C_INT_VOIDPTR *) ClassesPermissionList.get_next();
   }
   // salvo abilitazione personale a tabelle secondarie
   p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_head();
   while (p)
   {
      if (gsc_setPersonalSecPermissions(UserId, p->get_key(), *((C_4INT_STR_LIST *) p->get_VoidPtr())) == GS_BAD)
         return GS_BAD;
      p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_next();
   }
   // salvo i nomi degli utenti da cui ereditare le abilitazioni
   if (gsc_saveInheritanceUsers(UserId, InheritanceUserNames) == GS_BAD)
      return GS_BAD;

   return GS_GOOD;
}


void C_ModUsrDlg::OnBnClickedOk()
{
   if (m_CopyFromUsr.GetCheck() == BST_CHECKED)
   {
      C_INT_INT_STR *pSourceUsr;
      int           CurSel;

      if ((CurSel = m_UsrList_CopyFrom.GetCurSel()) == CB_ERR)
      {
         gsui_alert(_T("Utente da cui copiare le abilitazioni non trovato."));
         return;
      }
      pSourceUsr = (C_INT_INT_STR *) m_UsrList_CopyFrom.GetItemDataPtr(CurSel);

      if (gsc_ui_copyFromUserToUser(pSourceUsr->get_key(), m_CurrUsrId) != GS_GOOD)
         return;
   }
   else
      if (Update)
      {
         if (SaveUsrPermission(m_CurrUsrId) == GS_BAD)
         {
            gsui_alert(_T("Salvataggio impostazioni correnti non riuscito."));
            return;
         }
      }

   Update = false;
   CDialogEx::OnOK();
}


void C_ModUsrDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Modificautente);
}


void C_ModUsrDlg::OnBnClickedPermissionFromUsrCheck()
{
   if (m_CopyFromUsr.GetCheck() == BST_CHECKED)
   {
      m_UsrList_CopyFrom.EnableWindow(TRUE);
      m_Command_Button.EnableWindow(FALSE);
      m_Data_Button.EnableWindow(FALSE);
   }
   else
   {
      m_UsrList_CopyFrom.EnableWindow(FALSE);
      m_Command_Button.EnableWindow(TRUE);
      m_Data_Button.EnableWindow(TRUE);
   }
}


void C_ModUsrDlg::OnBnClickedCmdList()
{
   CValuesListDlg ListDlg;
   C_INT_STR_LIST CmdNames;
   C_INT_STR      *pCmdName;
   int            i = 1;

   ListDlg.m_Title      = _T("GEOsim - Abilitazione comandi");
   ListDlg.m_Msg        = _T("Seleziona i comandi da abilitare.");
   ListDlg.m_ColsHeader = _T("Nome comando");
   ListDlg.m_SingleSel  = FALSE;
   ListDlg.m_OriginType = CValuesListDlg::RESBUF_LIST;

   ListDlg.RbList << acutBuildList(RTLB, 0);
   if (gsc_getgscomm(&CmdNames)==GS_BAD) return;

   pCmdName = (C_INT_STR *) CmdNames.get_head();
   while (pCmdName)
   {
      ListDlg.RbList += acutBuildList(RTLB, RTSTR, pCmdName->get_name(), RTLE, 0);
      if ((i = Commands.getpos_key(pCmdName->get_key())) > 0) // se comando abilitato
         ListDlg.m_ValueList.add_tail_int(i - 1);

      pCmdName = (C_INT_STR *) CmdNames.get_next();
   }
   ListDlg.RbList += acutBuildList(RTLE, 0);

   if (ListDlg.DoModal() == IDOK)
   {
      C_INT *pCmd = (C_INT *) ListDlg.m_ValueList.get_head();

      Commands.remove_all();
      while (pCmd)
      {
         if ((pCmdName = (C_INT_STR *) CmdNames.getptr_at(pCmd->get_key() + 1)))
            Commands.add_tail_int(pCmdName->get_key()); 
         pCmd = (C_INT *) pCmd->get_next();
      }

      Update = true;
   }
}


void C_ModUsrDlg::OnBnClickedDataList()
{
   C_ModDataUsrDlg ModDataUsrDlg(m_CurrUsrId);

   ModDataUsrDlg.SetProjects(ProjectsPermissionList);
   ModDataUsrDlg.SetClasses(ClassesPermissionList);
   ModDataUsrDlg.SetSecTabs(SecTabsPermissionList);
   ModDataUsrDlg.SetInheritanceUserNames(InheritanceUserNames);

   if (ModDataUsrDlg.DoModal() == IDOK)
   {
      ModDataUsrDlg.GetProjects(ProjectsPermissionList);
      ModDataUsrDlg.GetClasses(ClassesPermissionList);
      ModDataUsrDlg.GetSecTabs(SecTabsPermissionList);
      ModDataUsrDlg.GetInheritanceUserNames(InheritanceUserNames);
      Update = true;
   }
}


// finestra di dialogo C_ModDataUsrDlg

IMPLEMENT_DYNAMIC(C_ModDataUsrDlg, CDialogEx)

C_ModDataUsrDlg::C_ModDataUsrDlg(int _UsrId, CWnd* pParent /*=NULL*/)
	: CDialogEx(C_ModDataUsrDlg::IDD, pParent)
   , m_PrjPermission(1)
   , m_Permission(1)
{
   UsrId = _UsrId;
   pCurrPrjItem = NULL;
   pCurrClsItem = NULL;
}

C_ModDataUsrDlg::~C_ModDataUsrDlg()
{
   C_INT_VOIDPTR *p;

   p = (C_INT_VOIDPTR *) ClassesPermissionList.get_head();
   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) ClassesPermissionList.get_next();
   }

   p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_head();
   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_next();
   }
}

void C_ModDataUsrDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialogEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_PRJ_TREE, m_PrjTreeCtrl);
   DDX_Control(pDX, IDC_CLASS_TREE, m_ClassTreeCtrl);
   DDX_Control(pDX, IDC_SEC_TREE, m_SecTreeCtrl);
   DDX_Radio(pDX, IDC_PRJ_INH_RADIO, m_PrjPermission);
   DDX_Control(pDX, IDC_PRJ_INH_RADIO, m_PrjInhRadioCtrl);
   DDX_Control(pDX, IDC_PRJ_INV_RADIO, m_PrjInvRadioCtrl);
   DDX_Control(pDX, IDC_PRJ_VIS_RADIO, m_PrjVisRadioCtrl);
   DDX_Control(pDX, IDC_PRJ_MOD_RADIO, m_PrjModRadioCtrl);
   DDX_Radio(pDX, IDC_INH_RADIO, m_Permission);
   DDX_Control(pDX, IDC_INH_RADIO, m_InhRadioCtrl);
   DDX_Control(pDX, IDC_INV_RADIO, m_InvRadioCtrl);
   DDX_Control(pDX, IDC_VIS_RADIO, m_VisRadioCtrl);
   DDX_Control(pDX, IDC_MOD_RADIO, m_ModRadioCtrl);
   DDX_Control(pDX, IDC_UPDGRADE, m_UpdButton);
   DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
}


BEGIN_MESSAGE_MAP(C_ModDataUsrDlg, CDialogEx)
   ON_BN_CLICKED(IDC_PRJ_UPGRADE, &C_ModDataUsrDlg::OnBnClickedPrjUpd)
   ON_BN_CLICKED(IDHELP, &C_ModDataUsrDlg::OnBnClickedHelp)
   ON_NOTIFY(TVN_SELCHANGED, IDC_PRJ_TREE, &C_ModDataUsrDlg::OnTvnSelchangedPrjTree)
   ON_BN_CLICKED(IDC_UPDGRADE, &C_ModDataUsrDlg::OnBnClickedUpdgrade)
   ON_BN_CLICKED(IDOK, &C_ModDataUsrDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDC_INHERITANCE_BUTTON, &C_ModDataUsrDlg::OnBnClickedInheritanceButton)
   ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &C_ModDataUsrDlg::OnTcnSelchangeTab1)
   ON_NOTIFY(TVN_SELCHANGED, IDC_CLASS_TREE, &C_ModDataUsrDlg::OnTvnSelchangedClassTree)
END_MESSAGE_MAP()


int C_ModDataUsrDlg::SetProjects(C_INT_INT_LIST &_ProjectsPermissionList)
{
   return _ProjectsPermissionList.copy(&ProjectsPermissionList);
}
int C_ModDataUsrDlg::GetProjects(C_INT_INT_LIST &_ProjectsPermissionList)
{
   return ProjectsPermissionList.copy(&_ProjectsPermissionList);
}

int C_ModDataUsrDlg::SetClasses(C_INT_VOIDPTR_LIST &_ClassesPermissionList)
{
   C_INT_VOIDPTR  *p = (C_INT_VOIDPTR *) ClassesPermissionList.get_head();
   C_INT_INT_LIST *pClasses;

   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) p->get_next();
   }
   ClassesPermissionList.remove_all();

   p = (C_INT_VOIDPTR *) _ClassesPermissionList.get_head();
   while (p)
   {
      if ((pClasses = new C_INT_INT_LIST()) == NULL) return GS_BAD;
      if (((C_INT_INT_LIST *) p->get_VoidPtr())->copy(pClasses) == GS_BAD)
         { delete pClasses; return GS_BAD; }
      ClassesPermissionList.add_tail_int_voidptr(p->get_key(), pClasses);

      p = (C_INT_VOIDPTR *) p->get_next();
   }

   return GS_GOOD;
}
int C_ModDataUsrDlg::GetClasses(C_INT_VOIDPTR_LIST &_ClassesPermissionList)
{
   C_INT_VOIDPTR  *p = (C_INT_VOIDPTR *) _ClassesPermissionList.get_head();
   C_INT_INT_LIST *pClasses;

   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) p->get_next();
   }
   _ClassesPermissionList.remove_all();

   p = (C_INT_VOIDPTR *) ClassesPermissionList.get_head();
   while (p)
   {
      if ((pClasses = new C_INT_INT_LIST()) == NULL) return GS_BAD;
      if (((C_INT_INT_LIST *) p->get_VoidPtr())->copy(pClasses) == GS_BAD)
         { delete pClasses; return GS_BAD; }
      _ClassesPermissionList.add_tail_int_voidptr(p->get_key(), pClasses);

      p = (C_INT_VOIDPTR *) p->get_next();
   }

   return GS_GOOD;
}

int C_ModDataUsrDlg::SetSecTabs(C_INT_VOIDPTR_LIST &_SecTabsPermissionList)
{
   C_INT_VOIDPTR   *p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_head();
   C_4INT_STR_LIST *pSecTabs;

   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) p->get_next();
   }
   SecTabsPermissionList.remove_all();

   p = (C_INT_VOIDPTR *) _SecTabsPermissionList.get_head();
   while (p)
   {
      if ((pSecTabs = new C_4INT_STR_LIST()) == NULL) return GS_BAD;
      if (((C_4INT_STR_LIST *) p->get_VoidPtr())->copy(pSecTabs) == GS_BAD)
         { delete pSecTabs; return GS_BAD; }
      SecTabsPermissionList.add_tail_int_voidptr(p->get_key(), pSecTabs);

      p = (C_INT_VOIDPTR *) p->get_next();
   }

   return GS_GOOD;
}
int C_ModDataUsrDlg::GetSecTabs(C_INT_VOIDPTR_LIST &_SecTabsPermissionList)
{
   C_INT_VOIDPTR   *p = (C_INT_VOIDPTR *) _SecTabsPermissionList.get_head();
   C_4INT_STR_LIST *pSecTabs;

   while (p)
   {
      if (p->get_VoidPtr()) delete p->get_VoidPtr();
      p = (C_INT_VOIDPTR *) p->get_next();
   }
   _SecTabsPermissionList.remove_all();

   p = (C_INT_VOIDPTR *) SecTabsPermissionList.get_head();
   while (p)
   {
      if ((pSecTabs = new C_4INT_STR_LIST()) == NULL) return GS_BAD;
      if (((C_4INT_STR_LIST *) p->get_VoidPtr())->copy(pSecTabs) == GS_BAD)
         { delete pSecTabs; return GS_BAD; }
      _SecTabsPermissionList.add_tail_int_voidptr(p->get_key(), pSecTabs);

      p = (C_INT_VOIDPTR *) p->get_next();
   }

   return GS_GOOD;
}

int C_ModDataUsrDlg::SetInheritanceUserNames(C_STR_LIST &_InheritanceUserNames)
{
   return _InheritanceUserNames.copy(&InheritanceUserNames);
}
int C_ModDataUsrDlg::GetInheritanceUserNames(C_STR_LIST &_InheritanceUserNames)
{
   return InheritanceUserNames.copy(&_InheritanceUserNames);
}

GSDataPermissionTypeEnum C_ModDataUsrDlg::RadioNumberToPermission(int RadioNumber)
{
   switch (RadioNumber)
   {
      case 1:
         return GSInvisibleData;
      case 2:
         return GSReadOnlyData;
      case 3:
         return GSUpdateableData;
      default :
         return GSNonePermission;
   }
}

int C_ModDataUsrDlg::PermissionToRadioNumber(GSDataPermissionTypeEnum Permission)
{
   switch (Permission)
   {
      case GSInvisibleData:
         return 1;
      case GSReadOnlyData:
         return 2;
      case GSUpdateableData:
         return 3;
      default:
         return 0;
   }
}

BOOL C_ModDataUsrDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   CRect groupBoxRect, tabRect;
   int   x, y, cx, cy, OffSet = 10, OffSetYTop = 20 + OffSet;

   GetDlgItem(IDC_PERMISSION_GROUPBOX)->GetWindowRect(&groupBoxRect);
 	ScreenToClient(groupBoxRect);
 	m_TabCtrl.GetWindowRect(&tabRect);
 	ScreenToClient(tabRect);
   tabRect.bottom = groupBoxRect.bottom + OffSet;
   // porto il tab control in secondo piano
   ::SetWindowPos(m_TabCtrl.m_hWnd, HWND_BOTTOM, 0, 0, tabRect.Width(), tabRect.Height(), SWP_NOMOVE);

   x = tabRect.left + OffSet;
   y = tabRect.top + OffSetYTop;
   cx = tabRect.Width() - 2 * OffSet;
   cy = tabRect.Height() - OffSetYTop - groupBoxRect.Height() - 2 * OffSet;
	m_ClassTreeCtrl.SetWindowPos(&m_TabCtrl, x, y, cx, cy, SWP_NOZORDER);
	m_SecTreeCtrl.SetWindowPos(&m_TabCtrl, x, y, cx, cy, SWP_NOZORDER);

   m_PrjTreeCtrl.FinalObjectType = GSProject;
   m_PrjTreeCtrl.PrjPermissionVisibility = true;

   m_ClassTreeCtrl.FinalObjectType = GSClass;
   m_ClassTreeCtrl.ClassPermissionVisibility = true;
   m_ClassTreeCtrl.SelectedLinkedClass = true;
   m_ClassTreeCtrl.MultiSelect = TRUE;

   m_SecTreeCtrl.FinalObjectType = GSSecondaryTab;
   m_SecTreeCtrl.SecPermissionVisibility = true;
   m_SecTreeCtrl.MultiSelect = TRUE;

   m_PrjTreeCtrl.LoadFromDB(0, &ProjectsPermissionList, NULL, NULL, &InheritanceUserNames);
   m_PrjTreeCtrl.Refresh();

   OnSelchangedPrjTree();
   
   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// gestori di messaggi C_ModDataUsrDlg


void C_ModDataUsrDlg::OnTvnSelchangedPrjTree(NMHDR *pNMHDR, LRESULT *pResult)
{
   OnSelchangedPrjTree();
   *pResult = 0;
}

int C_ModDataUsrDlg::OnSelchangedPrjTree(void)
{
   BOOL          EnableCtrls;
   C_STRING      msg(_T("Classi di "));

   if ((pCurrPrjItem = m_PrjTreeCtrl.get_ItemList_ptr()->search_selected()))
   {
      EnableCtrls = TRUE;
      m_PrjPermission = PermissionToRadioNumber(pCurrPrjItem->updatable);

      // leggo lista delle composizioni di classi gruppo
      C_PROJECT *pPrj = (C_PROJECT *) get_GS_PROJECT()->search_key(pCurrPrjItem->get_key());
      if (pPrj)
      {
         pPrj->get_group_list(&GroupList);
         msg += pPrj->get_name();
      }

      UpdateData(FALSE); // per aggiornare i radio button

      m_ClassTreeCtrl.LoadFromDB(pCurrPrjItem->get_key(), NULL, &ClassesPermissionList, NULL, &InheritanceUserNames);
      m_ClassTreeCtrl.Refresh();
      m_ClassTreeCtrl.ExpandAll();

      m_SecTreeCtrl.LoadFromDB(pCurrPrjItem->get_key(), NULL, NULL, &SecTabsPermissionList, &InheritanceUserNames);


      EnableRadioCtrl(pCurrPrjItem->updatable);
   }
   else
   {
      m_InhRadioCtrl.EnableWindow(FALSE);
      m_InvRadioCtrl.EnableWindow(FALSE);
      m_VisRadioCtrl.EnableWindow(FALSE);
      m_ModRadioCtrl.EnableWindow(FALSE);

      EnableCtrls = FALSE;
   }

   m_PrjInhRadioCtrl.EnableWindow(EnableCtrls);
   m_PrjInvRadioCtrl.EnableWindow(EnableCtrls);
   m_PrjVisRadioCtrl.EnableWindow(EnableCtrls);
   m_PrjModRadioCtrl.EnableWindow(EnableCtrls);

   m_ClassTreeCtrl.EnableWindow(EnableCtrls);

   m_UpdButton.EnableWindow(EnableCtrls);

   m_TabCtrl.DeleteAllItems();
   m_TabCtrl.InsertItem(0, msg.get_name());
   m_TabCtrl.SetCurSel(0);
   OnChangeSelectionTabCtrl();

   return GS_GOOD;
}

void C_ModDataUsrDlg::OnBnClickedPrjUpd()
{
   HTREEITEM     hItem;

   if (pCurrPrjItem && (hItem = m_PrjTreeCtrl.GetSelectedItem()))
   {
      UpdateData(TRUE); // per aggiornare m_PrjPermission
      GSDataPermissionTypeEnum Permission = RadioNumberToPermission(m_PrjPermission);

      m_PrjTreeCtrl.UpdItem(pCurrPrjItem, pCurrPrjItem->get_name(),
                            pCurrPrjItem->Descr.get_name(), pCurrPrjItem->ImagePath.get_name(),
                            Permission, hItem);
      C_INT_INT *p = (C_INT_INT *) ProjectsPermissionList.search_key(pCurrPrjItem->get_key());
      if (!p) 
      {
         p = new C_INT_INT();
         ProjectsPermissionList.add_tail(p);
         p->set_key(pCurrPrjItem->get_key());
      }
      p->set_type(Permission);
      ClassDowngradeOnPrjPermission(Permission);
      SecDowngradeOnPermission(Permission);
      EnableRadioCtrl(Permission);
   }
}

void C_ModDataUsrDlg::ClassDowngradeOnPrjPermission(GSDataPermissionTypeEnum PrjPermission, CGEOTree_ItemList *pItemList)
{
   CGEOTree_Item *pItem;
   bool          Downgrade = false;
   int           Prj;

   if (!pCurrPrjItem) return;
   Prj = pCurrPrjItem->get_key();

   if (!pItemList) pItemList = m_ClassTreeCtrl.get_ItemList_ptr();

   pItem = (CGEOTree_Item *) pItemList->get_head();
   while (pItem)
   {
      Downgrade = false;

      if (pItem->ObjectType == GSProject) Downgrade = true;
      else // classe
         if (PrjPermission == GSNonePermission || PrjPermission == GSInvisibleData)
            Downgrade = true;
         else if (PrjPermission == GSReadOnlyData)
            if (pItem->updatable == GSUpdateableData) Downgrade = true;

      if (Downgrade)
      {
         HTREEITEM hItem = m_ClassTreeCtrl.FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);
         if (hItem)
            m_ClassTreeCtrl.UpdItem(pItem, pItem->get_name(),
                                    pItem->Descr.get_name(), pItem->ImagePath.get_name(),
                                    PrjPermission, hItem);

         SetClassPermission(Prj, pItem->get_key(), PrjPermission);
      }

      // Se ha figli
      ClassDowngradeOnPrjPermission(PrjPermission, &(pItem->ChildItemList));
      pItem = (CGEOTree_Item *) pItem->get_next();
   }
}

void C_ModDataUsrDlg::SecDowngradeOnPermission(GSDataPermissionTypeEnum Permission, CGEOTree_ItemList *pItemList, int _Sub)
{
   CGEOTree_Item *pItem;
   bool          Downgrade = false;
   int           Prj, Cls, Sub = _Sub;

   if (!pCurrPrjItem || !pCurrClsItem) return;
   Prj = pCurrPrjItem->get_key();
   Cls = pCurrClsItem->get_key();

   if (!pItemList) pItemList = m_SecTreeCtrl.get_ItemList_ptr();

   pItem = (CGEOTree_Item *) pItemList->get_head();
   while (pItem)
   {
      Downgrade = false;

      if (pItem->ObjectType == GSProject) Downgrade = true;
      else // classe o secondaria
      {
         if (pItem->ObjectType == GSSubClass) Sub = pItem->get_key();

         if (Permission == GSNonePermission || Permission == GSInvisibleData)
            Downgrade = true;
         else if (Permission == GSReadOnlyData)
            if (pItem->updatable == GSUpdateableData) Downgrade = true;
      }

      if (Downgrade)
      {
         HTREEITEM hItem = m_SecTreeCtrl.FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);
         if (hItem)
            m_SecTreeCtrl.UpdItem(pItem, pItem->get_name(),
                                  pItem->Descr.get_name(), pItem->ImagePath.get_name(),
                                  Permission, hItem);

         SetSecPermission(Prj, Cls, Sub, pItem->get_key(), Permission);
      }

      // Se ha figli
      SecDowngradeOnPermission(Permission, &(pItem->ChildItemList), Sub);
      pItem = (CGEOTree_Item *) pItem->get_next();
   }
}

int C_ModDataUsrDlg::SetClassPermission(int Prj, int Cls, GSDataPermissionTypeEnum Permission)
{
   C_INT_VOIDPTR  *p = (C_INT_VOIDPTR *) ClassesPermissionList.search_key(Prj);
   C_INT_INT_LIST *pClasses;
   C_INT_INT      *pCls;

   if (!p) 
   {
      if ((p = new C_INT_VOIDPTR(Prj, NULL)) == NULL) return GS_BAD;
      ClassesPermissionList.add_tail(p);
   }

   if (!(pClasses = (C_INT_INT_LIST *) p->get_VoidPtr()))
   {
      if ((pClasses = new C_INT_INT_LIST()) == NULL) return GS_BAD;
      p->set_VoidPtr(pClasses);
   }
   if (!(pCls = (C_INT_INT *) pClasses->search_key(Cls)))
   {
      pClasses->values_add_tail(Cls, (int) Permission); // abilitazione della tabella secondaria
   }
   else pCls->set_type((int) Permission);

   return GS_GOOD;
}

int C_ModDataUsrDlg::SetSecPermission(int Prj, int Cls, int Sub, int Sec, GSDataPermissionTypeEnum Permission)
{
   C_INT_VOIDPTR  *p = (C_INT_VOIDPTR *) SecTabsPermissionList.search_key(Prj);
   C_4INT_STR_LIST *pSecs;
   C_4INT_STR      *pSec;

   if (!p) 
   {
      if ((p = new C_INT_VOIDPTR(Prj, NULL)) == NULL) return GS_BAD;
      SecTabsPermissionList.add_tail(p);
   }

   if (!(pSecs = (C_4INT_STR_LIST *) p->get_VoidPtr()))
   {
      if ((pSecs = new C_4INT_STR_LIST()) == NULL) return GS_BAD;
      p->set_VoidPtr(pSecs);
   }
   if (!(pSec = (C_4INT_STR *) pSecs->search(Cls, Sub, Sec)))
   {
      // Uso una C_4INT_STR in cui:   
      // set_key() e get_key()           gestiscono il codice della classe
      // set_type() e get_type()         gestiscono il codice della sotto-classe
      // set_category() e get_category() gestiscono il codice della tabella secondaria
      // set_level() e get_level()       gestiscono l'abilitazione della tabella secondaria

      pSec = new C_4INT_STR;
      pSecs->add_tail(pSec);

      pSec->set_key(Cls);          // codice della classe
      pSec->set_type(Sub);         // codice della sotto-classe
      pSec->set_category(Sec);     // codice della tabella secondaria
      pSec->set_level(Permission); // abilitazione della tabella secondaria
   }
   else pSec->set_level(Permission); // abilitazione della tabella secondaria

   return GS_GOOD;
}

void C_ModDataUsrDlg::EnableRadioCtrl(GSDataPermissionTypeEnum Permission)
{
   switch (Permission)
   {
       case GSNonePermission:
         m_InhRadioCtrl.EnableWindow(TRUE);
         m_InvRadioCtrl.EnableWindow(FALSE);
         m_VisRadioCtrl.EnableWindow(FALSE);
         m_ModRadioCtrl.EnableWindow(FALSE);
         break;
      case GSInvisibleData:
         m_InhRadioCtrl.EnableWindow(FALSE);
         m_InvRadioCtrl.EnableWindow(TRUE);
         m_VisRadioCtrl.EnableWindow(FALSE);
         m_ModRadioCtrl.EnableWindow(FALSE);
         break;
      case GSReadOnlyData:
         m_InhRadioCtrl.EnableWindow(TRUE);
         m_InvRadioCtrl.EnableWindow(TRUE);
         m_VisRadioCtrl.EnableWindow(TRUE);
         m_ModRadioCtrl.EnableWindow(FALSE);
         break;
      case GSUpdateableData:
         m_InhRadioCtrl.EnableWindow(TRUE);
         m_InvRadioCtrl.EnableWindow(TRUE);
         m_VisRadioCtrl.EnableWindow(TRUE);
         m_ModRadioCtrl.EnableWindow(TRUE);
         break;
   }

   m_Permission = PermissionToRadioNumber(Permission);
   UpdateData(FALSE); // per aggiornare i radio button
}

void C_ModDataUsrDlg::OnBnClickedUpdgrade()
{
   int curSel = m_TabCtrl.GetCurSel();

   switch (curSel)
   {
      case 0: // classi
         return OnBnClickedClassUpd();
      case 1: // secondarie
         return OnBnClickedSecUpd();
   }

   return;
}

void C_ModDataUsrDlg::OnBnClickedClassUpd()
{
   int           Prj;
   CGEOTree_Item *pItem;
   C_INT_LIST    SelectedCodes;
   C_INT         *pSelectedCode;
   HTREEITEM     hItem;
   C_BIRELATION  *pGrp;

   if (!pCurrPrjItem) return;
   Prj = pCurrPrjItem->get_key();

   UpdateData(TRUE); // per aggiornare m_Permission
   GSDataPermissionTypeEnum Permission = RadioNumberToPermission(m_Permission);

   C_INT_INT *p = (C_INT_INT *) ProjectsPermissionList.search_key(Prj);
   if (!p) 
   {
      p = new C_INT_INT(Prj, (int) GSInvisibleData);
      ProjectsPermissionList.add_tail(p);
      p->set_key(Prj);
   }

   m_ClassTreeCtrl.GetSelectedCodes(SelectedCodes);
   pSelectedCode = (C_INT *) SelectedCodes.get_head();
   while (pSelectedCode)
   {
      if (!(pItem = m_ClassTreeCtrl.get_ItemList_ptr()->search_cls(Prj, pSelectedCode->get_key())))
         return;
      hItem = m_ClassTreeCtrl.FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);
      if (hItem)
         m_ClassTreeCtrl.UpdItem(pItem, pItem->get_name(),
                                 pItem->Descr.get_name(), pItem->ImagePath.get_name(),
                                 Permission, hItem);

      SetClassPermission(Prj, pItem->get_key(), Permission);

      // se è stata selezionata una classe appartenente ad un gruppo 
      // devo includere tutte le classi del gruppo
      pGrp = (C_BIRELATION *) GroupList.get_head();
      while (pGrp)
      {
         if (pGrp->get_key() == pItem->get_key() ||
             pGrp->relation.search_key(pItem->get_key()))
         {
            C_INT_INT *pMemberCls = (C_INT_INT *) pGrp->relation.get_head();

            // aggiungo la classe gruppo se non era selezionata
            if (!SelectedCodes.search_key(pGrp->get_key()))
               SelectedCodes.add_tail_int(pGrp->get_key());
            // aggiungo le classi membro del gruppo se non erano selezionate
            while (pMemberCls)
            {
               if (!SelectedCodes.search_key(pMemberCls->get_key()))
                  SelectedCodes.add_tail_int(pMemberCls->get_key());
               pMemberCls = (C_INT_INT *) pMemberCls->get_next();
            }
         }

         pGrp = (C_BIRELATION *) pGrp->get_next();
      }
      pSelectedCode = (C_INT *) pSelectedCode->get_next();
   }

   SecDowngradeOnPermission(Permission);
}


/*****************************************************************************/
/*.doc C_ModDataUsrDlg::GetSecSelectedCodes                       <external> */
/*+
  Questa funzione ricava le tabelle secondarie selezionate.
  Parametri:
  C_INT_INT_LIST &Codes;     lista di codici (sottoclasse-secondaria)
  HTREEITEM  hItem;           uso interno
  int int sub;  uso interno
-*/
/*****************************************************************************/
void C_ModDataUsrDlg::GetSecSelectedCodes(C_INT_INT_LIST &Codes, HTREEITEM hItem, int sub)
{
   CGEOTree_Item *pItem;
   HTREEITEM hChildItem;

   if (!hItem)
   {
      hItem = m_SecTreeCtrl.GetRootItem();
      Codes.remove_all();
   }

   do
   {
      if (!(pItem = (CGEOTree_Item *) m_SecTreeCtrl.GetItemData(hItem))) return;

      switch (pItem->ObjectType)
      {
         case GSSubClass:
            sub = pItem->get_key();
            break;
         case GSSecondaryTab: // se si devono cercare tabelle secondarie
            if (pItem->selected)
               Codes.values_add_tail(sub, pItem->get_key());
            break;
      }

      if ((hChildItem = m_SecTreeCtrl.GetChildItem(hItem)) != NULL) // Se ha figli
         GetSecSelectedCodes(Codes, hChildItem, sub); // cerco nei figli
   }
   // cerco nei fratelli
   while ((hItem = m_SecTreeCtrl.GetNextSiblingItem(hItem))); // fratello successivo

   return;
}


void C_ModDataUsrDlg::OnBnClickedSecUpd()
{
   int            Prj, Cls, Sub;
   CGEOTree_Item  *pItem;
   C_INT_INT_LIST SelectedCodes;
   C_INT_INT      *pSelectedCode;
   HTREEITEM      hItem;

   if (!pCurrPrjItem || !pCurrClsItem) return;
   Prj = pCurrPrjItem->get_key();
   Cls = pCurrClsItem->get_key();

   UpdateData(TRUE); // per aggiornare m_Permission
   GSDataPermissionTypeEnum Permission = RadioNumberToPermission(m_Permission);

   GetSecSelectedCodes(SelectedCodes);
   pSelectedCode = (C_INT_INT *) SelectedCodes.get_head();
   while (pSelectedCode)
   {
      Sub = pSelectedCode->get_key(); // codice sottoclasse
      if (!(pItem = m_SecTreeCtrl.get_ItemList_ptr()->search_sec(Prj, Cls, Sub,
                                                                 pSelectedCode->get_type()))) // codice tabella secondaria
         return;
      hItem = m_SecTreeCtrl.FindItem((HTREEITEM) NULL, (DWORD_PTR) pItem);
      if (hItem)
         m_SecTreeCtrl.UpdItem(pItem, pItem->get_name(),
                               pItem->Descr.get_name(), pItem->ImagePath.get_name(),
                               Permission, hItem);

      SetSecPermission(Prj, Cls, Sub, pItem->get_key(), Permission);

      pSelectedCode = (C_INT_INT *) pSelectedCode->get_next();
   }
}

void C_ModDataUsrDlg::OnBnClickedInheritanceButton()
{
   C_InheritanceDataUsrDlg InheritanceDataUsrDlg(UsrId);

   InheritanceUserNames.copy(&InheritanceDataUsrDlg.InheritanceUserNames);
   if (InheritanceDataUsrDlg.DoModal() == IDOK)
      InheritanceDataUsrDlg.InheritanceUserNames.copy(&InheritanceUserNames);
}

void C_ModDataUsrDlg::OnBnClickedOk()
{
   CDialogEx::OnOK();
}

void C_ModDataUsrDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Abilitazioneaidati);
}

void C_ModDataUsrDlg::OnTvnSelchangedClassTree(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

   while (m_TabCtrl.GetItemCount() > 1)
      m_TabCtrl.DeleteItem(1);

   if (pCurrPrjItem && (pCurrClsItem = getCurrClsItem()))
   {
      CGEOTree_Item *pItem;

      m_Permission = PermissionToRadioNumber(pCurrClsItem->updatable);
      UpdateData(FALSE); // per aggiornare i radio button

      // verifico se ci sono tabelle secondarie
      if ((pItem = m_SecTreeCtrl.get_ItemList_ptr()->search_cls(pCurrPrjItem->get_key(), pCurrClsItem->get_key())))
      {
         bool exist = false;

         if (pItem->category == CAT_SUBCLASS)
         {
            pItem = (CGEOTree_Item *) pItem->ChildItemList.get_head();
            while (pItem) // ciclo sulle sottoclassi
               if (pItem->ChildItemList.get_count() > 0)
                  { exist = true; break; }
               else
                  pItem = (CGEOTree_Item *) pItem->get_next();
         }
         else
            if (pItem->ChildItemList.get_count() > 0) exist = true;

         if (exist)
         {
            C_STRING msg(_T("Tabelle secondarie di "));

            // filtro per progetto e classe
            m_SecTreeCtrl.FilterOnCodes.add_tail_int(pCurrPrjItem->get_key());
            ((C_FAMILY *) m_SecTreeCtrl.FilterOnCodes.get_head())->relation.add_tail_int(pCurrClsItem->get_key());

            m_SecTreeCtrl.Refresh();
            m_SecTreeCtrl.ExpandAll();

            msg += pCurrClsItem->get_name();
            m_TabCtrl.InsertItem(1, msg.get_name());
         }
      }
   }

   *pResult = 0;
}

void C_ModDataUsrDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
   OnChangeSelectionTabCtrl();
   *pResult = 0;
}

CGEOTree_Item *C_ModDataUsrDlg::getCurrClsItem(void)
{
   HTREEITEM hItem;

   if ((hItem = m_ClassTreeCtrl.GetSelectedItem()))
   {
      TV_ITEM tvI;

      tvI.hItem = hItem;
      tvI.mask  = TVIF_PARAM | TVIF_HANDLE;

      if (m_ClassTreeCtrl.GetItem(&tvI) && tvI.lParam)
         return (CGEOTree_Item *) tvI.lParam;
   }

   return NULL;
}

void C_ModDataUsrDlg::OnChangeSelectionTabCtrl()
{
   int curSel = m_TabCtrl.GetCurSel();

   m_ClassTreeCtrl.ShowWindow(SW_HIDE);
   m_SecTreeCtrl.ShowWindow(SW_HIDE);

   switch (curSel)
   {
      case 0: // classi
         m_ClassTreeCtrl.ShowWindow(SW_SHOW);
         m_ClassTreeCtrl.BringWindowToTop();
         if (pCurrPrjItem) EnableRadioCtrl(pCurrPrjItem->updatable);
         break;
      case 1: // sottoclassi o secondarie
         if (pCurrClsItem)
         {
            m_SecTreeCtrl.ShowWindow(SW_SHOW);
            m_SecTreeCtrl.BringWindowToTop();
            EnableRadioCtrl(pCurrClsItem->updatable);
         }
         break;
   }

   GetDlgItem(IDC_PERMISSION_GROUPBOX)->BringWindowToTop();
   GetDlgItem(IDC_STATIC_GRP)->BringWindowToTop();
   //m_InhRadioCtrl.BringWindowToTop(); // se usato i radio non vanno più (baco di mfc)
   //m_InvRadioCtrl.BringWindowToTop();
   //m_VisRadioCtrl.BringWindowToTop();
   //m_ModRadioCtrl.BringWindowToTop();
   m_UpdButton.BringWindowToTop();

   RedrawWindow();
}


// finestra di dialogo C_InheritanceDataUsrDlg


IMPLEMENT_DYNAMIC(C_InheritanceDataUsrDlg, CDialogEx)

C_InheritanceDataUsrDlg::C_InheritanceDataUsrDlg(int _UsrId, CWnd* pParent /*=NULL*/)
	: CDialogEx(C_InheritanceDataUsrDlg::IDD, pParent)
{
   UsrId = _UsrId;
}

C_InheritanceDataUsrDlg::~C_InheritanceDataUsrDlg()
{
}

BOOL C_InheritanceDataUsrDlg::OnInitDialog() 
{ 
   CDialogEx::OnInitDialog();

   if (gsc_getusrlist(&UsrList) == GS_BAD) return GS_GOOD;
   RefreshListCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void C_InheritanceDataUsrDlg::RefreshListCtrl(void)
{
   C_INT_INT_STR *pUsr = (C_INT_INT_STR *) UsrList.get_head();

   m_UsrList.ResetContent();
   m_UsrInheritanceList.ResetContent();

   while (pUsr)
   {     
      if (pUsr->get_key() != UsrId && pUsr->get_type() == 1) // solo utenti normali escluso l'utente corrente
      {
         if (InheritanceUserNames.search_name(pUsr->get_name()))
            m_UsrInheritanceList.AddString(pUsr->get_name()); // lista utenti da cui ereditare
         else
            m_UsrList.AddString(pUsr->get_name()); // lista utenti da cui NON ereditare
      }

      pUsr = (C_INT_INT_STR *) UsrList.get_next();
   }
}

void C_InheritanceDataUsrDlg::MoveSelectedUser(bool ToInheritanceList)
{
   int     count, i;
   CString tmp;
   CArray< int,int > arrayListSel;

   if (ToInheritanceList)
   {
      count = m_UsrList.GetSelCount();
      arrayListSel.SetSize(count);     // make room in array
      m_UsrList.GetSelItems(count, arrayListSel.GetData());    // copy data to array
      for (i = 0; i < count; i++)
      {
         m_UsrList.GetText(arrayListSel[i], tmp);
         InheritanceUserNames.add_tail_str(tmp);
      }
   }
   else
   {
      count = m_UsrInheritanceList.GetSelCount();
      arrayListSel.SetSize(count);     // make room in array
      m_UsrInheritanceList.GetSelItems(count, arrayListSel.GetData());    // copy data to array
      for (i = 0; i < count; i++)
      {
         m_UsrInheritanceList.GetText(arrayListSel[i], tmp);
         InheritanceUserNames.remove_name(tmp);
      }
   }

   RefreshListCtrl();
}

void C_InheritanceDataUsrDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialogEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_USR_LIST, m_UsrList);
   DDX_Control(pDX, IDC_INHERITANCE_USR_LIST, m_UsrInheritanceList);
}


BEGIN_MESSAGE_MAP(C_InheritanceDataUsrDlg, CDialogEx)
   ON_LBN_DBLCLK(IDC_USR_LIST, &C_InheritanceDataUsrDlg::OnLbnDblclkUsrList)
   ON_LBN_DBLCLK(IDC_INHERITANCE_USR_LIST, &C_InheritanceDataUsrDlg::OnLbnDblclkInheritanceUsrList)
   ON_BN_CLICKED(IDC_ADD_INHERITANCE_BUTTON, &C_InheritanceDataUsrDlg::OnBnClickedAddInheritanceButton)
   ON_BN_CLICKED(IDC_DEL_INHERITANCE_BUTTON, &C_InheritanceDataUsrDlg::OnBnClickedDelInheritanceButton)
   ON_BN_CLICKED(IDHELP, &C_InheritanceDataUsrDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


// gestori di messaggi C_InheritanceDataUsrDlg


void C_InheritanceDataUsrDlg::OnLbnDblclkUsrList()
{
   MoveSelectedUser(true);
}

void C_InheritanceDataUsrDlg::OnLbnDblclkInheritanceUsrList()
{
   MoveSelectedUser(false);
}

void C_InheritanceDataUsrDlg::OnBnClickedAddInheritanceButton()
{
   MoveSelectedUser(true);
}

void C_InheritanceDataUsrDlg::OnBnClickedDelInheritanceButton()
{
   MoveSelectedUser(false);
}


void C_InheritanceDataUsrDlg::OnBnClickedHelp()
{
   gsc_help(IDH_Abilitazioneaidati);
}


/*************************************************************************/
/*.doc gsui_modusr                                                       */
/*+
  Comando per modificare le abilitazioni di un utente GEOsim.

  La funzione restituisce GS_GOOD in caso di successo altrimenti GS_BAD.  
-*/
/*************************************************************************/
int gsui_modusr(void)
{
   // When resource from this ARX app is needed, just
   // instantiate a local CAcModuleResourceOverride
   CAcModuleResourceOverride myResources;
   C_ModUsrDlg               ModUsrDlg;

   acedRetVoid();

   if (gsc_superuser() == GS_BAD)
   {
      gsui_alert(_T("Comando disponibile solo a superutente"));
      return GS_BAD;
   }

   if (ModUsrDlg.DoModal() != IDOK)  return GS_CAN;
   
   return GS_GOOD;
}
