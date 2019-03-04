// GEOUICMDLIST.h: interface for the C_GEOUICMDLIST class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEOUICMDLIST_H__A3F23B73_AB76_11D5_856B_0060086FD147__INCLUDED_)
#define AFX_GEOUICMDLIST_H__A3F23B73_AB76_11D5_856B_0060086FD147__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define INITGUID
#import "msado15.dll" no_namespace rename ("EOF", "EndOfFile") rename ("EOS", "ADOEOS")

#include "gs_list.h"          // per le liste
#include "gs_cmd.h"           // per le tabelle di funzioni
#include "gs_utily.h"         
#include "gs_error.h"         
#include "gs_user.h"         

#include "GEOUIacad_cmd.h"    //comandi di Autocad ridefiniti da GEOUI
#include "GEOUIlisp_cmd.h"  //funzioni lisp


class C_GEOUICMDLIST  
{
public:
	C_GEOUICMDLIST();
	virtual ~C_GEOUICMDLIST();

   //int UndefineAcadCmds(void);
   //int RedefineAcadCmds(void);
   int funcLoad();
   int funcUnload();
   FunTable *get_ptr_fun(int val);
   int dofun();
  
private:
   FunTable    *exfun;
   int         LenExfun;
   C_STR_LIST  AcadCmds;

};

#endif // !defined(AFX_GEOUICMDLIST_H__A3F23B73_AB76_11D5_856B_0060086FD147__INCLUDED_)
