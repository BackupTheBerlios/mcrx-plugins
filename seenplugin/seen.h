#include <windows.h>
#include "resource.h"
#include "..\..\miranda32\random\plugins\newpluginapi.h"
#include "..\..\miranda32\database\m_database.h"
#include "..\..\miranda32\random\langpack\m_langpack.h"


#pragma optimize("gsy",on)

#define S_MOD "SeenModule"

#define UM_CHECKHOOKS (WM_USER+1)

#define debug(a) MessageBox(NULL,a,"Debug",MB_OK)

#define ICON_OFFLINE		13
#define ICON_ONLINE			14
#define ICON_AWAY			15
#define ICON_NA				16
#define ICON_OCC			17
#define ICON_DND			18
#define ICON_FREE			19
#define	ICON_INVIS			20

typedef struct{
	int count;
	WPARAM wpcontact[1024];
	BYTE times[1024];
} MISSEDCONTACTS;