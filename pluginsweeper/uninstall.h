#include <windows.h>
#include <commctrl.h>
#include "..\..\miranda32\random\plugins\newpluginapi.h"
#include "resource.h"
#include "..\..\miranda32\database\m_database.h"

#define debug(s) MessageBox(NULL,s,"DBWIZ",MB_OK)
#define ListBox_AddString(a,c) SendMessage((a),LB_ADDSTRING,0,(LPARAM)(LPCSTR)c)
//#define ListBox_InsertString(a,c) SendMessage((a),LB_INSERTSTRING,0,(LPARAM)(LPCSTR)c)
#define ListBox_GetString(a,b,c) SendMessage((a),LB_GETTEXT,(WPARAM)b,(LPARAM)(LPCSTR)c)
//#define ListBox_GetItemData(a,c) (LPARAM)SendMessage((a),LB_GETITEMDATA,(WPARAM)c,0)
//#define ListBox_SetItemData(a,c,d) SendMessage((a),LB_SETITEMDATA,c,(LPARAM)d)
#define ListBox_GetSel(a) SendMessage((a),LB_GETCURSEL,0,0)

typedef struct{
	HTREEITEM htvi;
	HWND htree;
	int valid;
} HANDOVER;

typedef struct{
	HANDLE hcontact;
	const char *pszModule;
	const char *pszSetting[64];
	int count;
}DELETESTRUCT;


#define UM_STARTUP (WM_USER+1)
