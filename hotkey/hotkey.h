#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "..\..\miranda32\random\plugins\newpluginapi.h"
#include "..\..\miranda32\database\m_database.h"
#include "..\..\miranda32\protocols\protocols\m_protocols.h"

#pragma optimize("gsy",on)


#define MAX_CONTACTS	512

// array where the contacts are put into
typedef struct {
	struct c_struct{
		char szname[120];
		HANDLE hcontact;
		char proto[4];
	}contact[MAX_CONTACTS];
	short int count;
} CONTACTSTRUCT;


#define UM_UPDATE			(WM_USER+129)
#define UM_NAMECHANGE		(WM_USER+130)
#define UM_DELETECONTACT	(WM_USER+131)
#define UM_REDRAW			(WM_USER+132)
#define UM_SAVEHIDE			(WM_USER+133)
#define HOTKEY_GENERAL		0xC001
#define PROTO_ICQ			"ICQ\0"
#define PROTO_MSN			"MSN\0"
#define HK_PLG				"HotkeyPlugin"

//#define debug(a)			MessageBox(NULL,a,"",MB_OK)
