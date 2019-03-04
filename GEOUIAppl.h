// GEOUIAppl.h: interface for the GEOUIAppl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEOUIAPPL_H__E20E3826_AB4A_11D5_856B_0060086FD147__INCLUDED_)
#define AFX_GEOUIAPPL_H__E20E3826_AB4A_11D5_856B_0060086FD147__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GEOUICMDLIST.h"      // classe C_GEOUICMDLIST
#include "gs_ui_organizer.h"      // classe GEOOrganizer

#define GEOUI_PATH           _T("GEOUI")



class GEOUIAppl  
{
public:
	GEOUIAppl();
	virtual ~GEOUIAppl();

public:
	static int init(void); // Inizializzazione applicazione
   static int terminate(void);                   // Termina applicazione
   
   static C_GEOUICMDLIST GEOUICMD_LIST;  // Lista dei comandi di GEOUI
};

#endif // !defined(AFX_GEOUIAPPL_H__E20E3826_AB4A_11D5_856B_0060086FD147__INCLUDED_)
