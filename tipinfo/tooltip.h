#include <windows.h>
#include <commctrl.h>
#include "..\..\miranda32\random\plugins\newpluginapi.h"
#include "..\..\miranda32\random\langpack\m_langpack.h"
#include "..\..\miranda32\\database\m_database.h"
#include "..\..\miranda32\ui\options\m_options.h"
#include "..\..\miranda32\ui\clc\m_clc.h"


#define debug(a) MessageBox(NULL,a," ",MB_OK)
#define TIP_ID 1
#pragma optimize ("gsy",on)


#define TIP_MOD		"ToolTipPlugin"


#define search_token(token,chars)	!_strnicmp(template_str+loop,token,chars)

#define get_sub(sub)				lstrcpyn(sub_str,sub,128)

#define clean_up					DBFreeVariant(&dbv)

#define proto_setting(setting)		DBGetContactSetting(hItem,szProto,setting,&dbv)?"":dbv.pszVal

#define break_remover				if(!sub_str[0]) remove_breaks(sub_str,&loop,&pos,token_len)

#define get_ci						CallService(MS_CONTACT_GETCONTACTINFO,(WPARAM)0,(LPARAM)&ci)

#define set_pszinfo					get_sub(ci.pszVal);