#include <windows.h>
#include <commctrl.h>
#include "..\SDK\headers_c\newpluginapi.h"
#include "..\SDK\headers_c\m_database.h"
#include "resource.h"
#include "..\SDK\headers_c\m_langpack.h"

#pragma optimize ("gsy",on) 

typedef struct{
	int modules;
	struct{
		const char *pszModule;
	}module[128];
} MODULESTRUCT,*PMOD;

typedef struct{
	int contacts;
	struct{
		HANDLE hContact;
		const char *pszContact;
	}contact[1024];
} CONTACTSTRUCT,*PCON;

typedef struct{
	int settings;
	struct{
		const char *pszSetting;
	}setting[128];
} SETTINGSTRUCT;

typedef struct{
	HANDLE hContact;
	const char *pszContact;
	const char *pszModule;
	const char *pszSetting;
	DBVARIANT dbv;
	int nContact;
} HANDOVERSTRUCT;



#define debug(s)					MessageBox(NULL,s,"DBWIZ",MB_OK)
#define ListBox_AddString(a,c)		SendMessage((a),LB_ADDSTRING,0,(LPARAM)(LPCSTR)c)
#define ListBox_InsertString(a,c)	SendMessage((a),LB_INSERTSTRING,0,(LPARAM)(LPCSTR)c)
#define ListBox_GetItemData(a,c)	(LPARAM)SendMessage((a),LB_GETITEMDATA,(WPARAM)c,0)
#define ListBox_SetItemData(a,c,d)	SendMessage((a),LB_SETITEMDATA,c,(LPARAM)d)
#define BASE						IsDlgButtonChecked(hdlg,IDC_DECIMAL)?10:16
#define Store_Val(a)				SetWindowLong(GetDlgItem(hdlg,IDC_VALUE),GWL_USERDATA,(LONG)a)

#define UM_REINIT			(WM_USER+1)
#define UM_CLISTREBUILD		(WM_USER+2)
#define UM_CONTACTRENAME	(WM_USER+3)
#define UM_BASECHANGE		(WM_USER+4)

// from win2k.h
#define WinVerMajor()      LOBYTE(LOWORD(GetVersion()))
#define WinVerMinor()      HIBYTE(LOWORD(GetVersion()))
#define IsWinVerXPPlus()   (WinVerMajor()>=5 && LOWORD(GetVersion())!=5)