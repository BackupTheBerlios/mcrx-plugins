#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "..\miranda\include\newpluginapi.h"
#include "..\miranda\include\m_database.h"
#include "..\miranda\include\m_clist.h"
#include "..\miranda\include\m_clui.h"
#include "..\miranda\include\m_langpack.h"
#include "..\miranda\include\m_skin.h"



#pragma optimize("gsy",on)



#define MODULE						"EventNotify"

//#define DEBUG



#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED 0x00080000
#endif

#define LWA_ALPHA               0x00000002



//#define debug(s) MessageBox(NULL,s,"DBWIZ",MB_OK)
#define ListBox_AddString(a,b,c) SendDlgItemMessage((a),(b),CB_ADDSTRING,0,(LPARAM)(LPCSTR)c)
#define ListBox_GetItemData(a,b,c) (LPARAM)SendDlgItemMessage((a),(b),CB_GETITEMDATA,(WPARAM)c,0)
#define ListBox_SetItemData(a,b,c,d) SendDlgItemMessage((a),(b),CB_SETITEMDATA,c,(LPARAM)d)
#define ListBox_GetSel(a,b) (int)SendDlgItemMessage((a),(b),CB_GETCURSEL,0,0)
#define ListBox_SetSel(a,b,c) (int)SendDlgItemMessage((a),(b),CB_SETCURSEL,(WPARAM)(c),0)



#define STYLE_FLAT					0x1
#define STYLE_3D					0x2
#define STYLE_MINIMAL				0x3

#define POS_TOPLEFT					0x1
#define POS_TOPRIGHT				0x2
#define POS_CENTER					0x3
#define POS_BOTTOMLEFT				0x4
#define POS_BOTTOMRIGHT				0x5

#define SPREAD_VERT					0x1
#define SPREAD_HORZ					0x2

#define BKSTYLE_SOLID				0x1
#define BKSTYLE_TRANSPARENT			0x2



#define DISMISS_ONLINE				0x0001
#define DISMISS_AWAY				0x0002
#define DISMISS_NA					0x0004
#define DISMISS_OCCUPIED			0x0008
#define DISMISS_DND					0x0010
#define DISMISS_FREE				0x0020
#define DISMISS_INVIS				0x0040
#define DISMISS_LUNCH				0x0080
#define DISMISS_PHONE				0x0100

#define NOTIFY_MSG					0x0200
#define NOTIFY_FILE					0x0400
#define NOTIFY_URL					0x0800
#define NOTIFY_ALWAYS				0x1000
#define NOTIFY_AUTH					0x2000
#define NOTIFY_ADDED				0x4000

#define PREVIEW_EVENT				-1



#define POPUP_TIMER					0x1



#define MAX_STYLES					3
#define SPACING						3

#define FLAT_WIDTH					142
#define FLAT_HEIGHT					35

#define S3D_WIDTH					146
#define S3D_HEIGHT					39

#define MINIMAL_WIDTH				131
#define MINIMAL_HEIGHT				22



#define UM_SETPOS					(WM_USER+1)



typedef struct{
	BYTE bStyle;
	BYTE bPos;
	BYTE bSpreading;
	BYTE bBKStyle;
} FLAGSTRUCT,*PFLAGS;



struct EXTENDSTRUCT{
	int width;
	int height;
};



typedef struct{
	HANDLE hContact;
	HANDLE hDBEvent;
	UINT uEvent;
} DATASTRUCT,*PDATA;
