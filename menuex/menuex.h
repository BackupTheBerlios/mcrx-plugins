#include <windows.h>
#include "resource.h"
#include "..\SDK\headers_c\newpluginapi.h"
#include "..\SDK\headers_c\m_options.h"
#include "..\SDK\headers_c\m_database.h"
#include "..\SDK\headers_c\m_langpack.h"


#pragma optimize("gsy",on)


#define VISPLG "MenuItemEx"

#define MIIM_STRING	0x00000040

#define MAX_PROTOS	32
#define MAX_GROUPS	32

#define VF_AV		0x0001
#define	VF_NV		0x0002
#define	VF_HFL		0x0004
#define VF_IGN		0x0008
#define VF_PROTO	0x0010
#define VF_GRP		0x0020
#define VF_ADD		0x0040
#define VF_REQ		0x0080
