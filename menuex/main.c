#include "menuex.h"
#include "..\..\miranda32\core\m_system.h"
#include "..\..\miranda32\ui\contactlist\m_clist.h"
#include "..\..\miranda32\protocols\protocols\m_protosvc.h"
#include "..\..\miranda32\protocols\protocols\m_protomod.h"
#include "..\..\miranda32\ui\clui\m_clui.h"
#include "..\..\miranda32\random\ignore\m_ignore.h"



#define MS_SETINVIS		"MenuEx/SetInvis"
#define MS_SETVIS		"MenuEx/SetVis"
#define MS_HIDE			"MenuEx/Hide"
#define MS_IGNORE		"MenuEx/ShowIgnore"
#define MS_PROTO		"MenuEx/ShowProto"
#define MS_GROUP		"MenuEx/ShowGroup"
#define MS_ADDED		"MenuEx/SendAdded"
#define MS_AUTHREQ		"MenuEx/SendAuthReq"



HINSTANCE hinstance;
HANDLE hmenuVis,hmenuOff,hmenuHide,hmenuIgnore,hmenuProto,hmenuGroup,hmenuAdded,hmenuAuthReq;
HMENU hpopupIgnore,hpopupProto,hpopupGroup;
HWND hdummy;
char *szmainProto,*protodat[MAX_PROTOS],*groupdat[MAX_GROUPS];


int OptionsInit(WPARAM,LPARAM);
PLUGINLINK *pluginLink;
PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
		"MenuItemEx",
		PLUGIN_MAKE_VERSION(1,2,0,0),
		"Set user properties by contact menu",
		"Heiko Schillinger",
		"micron@nexgo.de",
		"© 2001-03 Heiko Schillinger",
		"http://nortiq.com/miranda",
		0,
		/*DEFMOD_UIVISIBILITY*/0		// your decision
};



BOOL CALLBACK AuthReqWndProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static HANDLE hcontact;

	switch(msg){

		case WM_INITDIALOG:
			hcontact=(HANDLE)lparam;
			break;

		case WM_COMMAND:
			switch(LOWORD(wparam)) {
				case IDOK:
				{
					char szReason[256];

					GetDlgItemText(hdlg,IDC_REASON,szReason,256);
					CallContactService(hcontact,PSS_AUTHREQUEST,0,(LPARAM)szReason);
				} // fall through
				case IDCANCEL:
					DestroyWindow(hdlg);
					break;
			}
			break;

	}

	return 0;
}


int SendAuthRequest(WPARAM wparam,LPARAM lparam)
{
	DWORD flags;
	char *szProto;

	szProto=(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0);
	
	flags=CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_4,0);
	if (flags&PF4_NOCUSTOMAUTH)	CallContactService((HANDLE)wparam,PSS_AUTHREQUEST,0,(LPARAM)"");
	else CreateDialogParam(hinstance,MAKEINTRESOURCE(IDD_AUTHREQ),(HWND)CallService(MS_CLUI_GETHWND,0,0),AuthReqWndProc,(LPARAM)wparam);

	return 0;
}



int SendAdded(WPARAM wparam,LPARAM lparam)
{
	CallContactService((HANDLE)wparam,PSS_ADDED,0,0);
	return 0;
}


// Window Proc to catch the Ignore-Popupmenu clicks
BOOL CALLBACK CatchMenuMsg(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg) {

		case WM_COMMAND:
			{
				LPARAM flag;
				int isIgnored,all,loop;
				HANDLE hcontact=(HANDLE)GetWindowLong(hwnd,GWL_USERDATA);
			
				switch(LOWORD(wparam)) {

					case IDM_MSG:
						flag=IGNOREEVENT_MESSAGE;
						break;
						
					case IDM_ALL:
						flag=IGNOREEVENT_ALL;
						break;

					case IDM_URL:
						flag=IGNOREEVENT_URL;
						break;

					case IDM_FILE:
						flag=IGNOREEVENT_FILE;
						break;

					case IDM_ONLINE:
						flag=IGNOREEVENT_USERONLINE;
						break;

					case IDM_AUTH:
						flag=IGNOREEVENT_AUTHORIZATION;
						break;

				}

				if(flag==IGNOREEVENT_ALL)
				{
					for(all=0,loop=1;loop<=5;loop++)
					{
						if(CallService(MS_IGNORE_ISIGNORED,(WPARAM)hcontact,(LPARAM)loop))
							all++;
					}
					isIgnored=((all==5)?1:0);
				}
				else
					isIgnored=CallService(MS_IGNORE_ISIGNORED,(WPARAM)hcontact,flag);

				CallService(isIgnored?MS_IGNORE_UNIGNORE:MS_IGNORE_IGNORE,(WPARAM)hcontact,flag);
			

				if(LOWORD(wparam)>=IDM_PROTOS && LOWORD(wparam)<IDM_GROUPS)
				{
					if(szmainProto)
						CallService(MS_PROTO_REMOVEFROMCONTACT,(WPARAM)hcontact,(LPARAM)szmainProto);
					CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hcontact,(LPARAM)protodat[LOWORD(wparam)-IDM_PROTOS]);
				}

				else if(LOWORD(wparam)>=IDM_GROUPS)
				{
					if(LOWORD(wparam)==IDM_GROUPS)
						CallService(MS_CLIST_CONTACTCHANGEGROUP,(WPARAM)hcontact,(LPARAM)NULL);
					else
						DBWriteContactSettingString(hcontact,"CList","Group",groupdat[LOWORD(wparam)-IDM_GROUPS]);
				}
			}

			break;
	}

	return 0;
}



int ShowGroup(WPARAM wparam,LPARAM lparam)
{
	POINT pos;
	int loop,offset=0;
	MENUITEMINFO mii;
	DBVARIANT dbvgrplst,dbvcntctgrp;
	char idstr[4];

	hpopupGroup=GetSubMenu(LoadMenu(hinstance,MAKEINTRESOURCE(IDR_MENU)),1);
	GetCursorPos(&pos);

	ZeroMemory(&mii,sizeof(mii));
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_STRING|MIIM_ID|MIIM_STATE;
	mii.fType=MFT_STRING;

	mii.fState=DBGetContactSetting((HANDLE)wparam,"CList","Group",&dbvcntctgrp)?MFS_CHECKED:MFS_UNCHECKED;
	mii.dwTypeData=Translate("<none>");
	mii.wID=IDM_GROUPS+offset;
	InsertMenuItem(hpopupGroup,offset,TRUE,&mii);
	groupdat[offset++]=Translate("<none>");

	for(loop=0;loop<MAX_GROUPS-1;loop++)
	{
		char *szgrp;
		itoa(loop,idstr,10);

		if(DBGetContactSetting(NULL,"CListGroups",idstr,&dbvgrplst)) break;
		DBGetContactSetting((HANDLE)wparam,"CList","Group",&dbvcntctgrp);
	
		mii.wID=IDM_GROUPS+offset;
		mii.fState=lstrcmp(dbvgrplst.pszVal+1,dbvcntctgrp.pszVal)?MFS_UNCHECKED:MFS_CHECKED;

		szgrp=strrchr(dbvgrplst.pszVal+1,'\\');

		if(szgrp==NULL)
			szgrp=dbvgrplst.pszVal+1;

		mii.dwTypeData=szgrp;
		InsertMenuItem(hpopupGroup,offset,TRUE,&mii);
		groupdat[offset++]=strdup(dbvgrplst.pszVal+1);
		DBFreeVariant(&dbvgrplst);
		DBFreeVariant(&dbvcntctgrp);
	}

	RemoveMenu(hpopupGroup,IDM_DUMMY,MF_BYCOMMAND);

	TrackPopupMenu(hpopupGroup,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,hdummy,NULL);
	DestroyMenu(hpopupGroup);
	SetWindowLong(hdummy,GWL_USERDATA,(LPARAM)wparam);

	return 0;
}


// Popup the Proto - menu
int ShowProto(WPARAM wparam,LPARAM lparam)
{
	POINT pos;
	int loop,offset=0,pcount;
	MENUITEMINFO mii;
	PROTOCOLDESCRIPTOR** pdesc;

	hpopupProto=GetSubMenu(LoadMenu(hinstance,MAKEINTRESOURCE(IDR_MENU)),1);
	GetCursorPos(&pos);

	ZeroMemory(&mii,sizeof(mii));
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_STRING|MIIM_ID|MIIM_STATE;
	mii.fType=MFT_STRING;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&pcount,(LPARAM)&pdesc);

	for(loop=0;loop<pcount;loop++)
	{
		if(pdesc[loop]->type==PROTOTYPE_PROTOCOL)
		{
			mii.wID=IDM_PROTOS+offset;
			mii.fState=CallService(MS_PROTO_ISPROTOONCONTACT,wparam,(LPARAM)pdesc[loop]->szName)?MFS_CHECKED:MFS_UNCHECKED;
			mii.dwTypeData=pdesc[loop]->szName;
			if(mii.fState==MFS_CHECKED)
				szmainProto=pdesc[loop]->szName;

			InsertMenuItem(hpopupProto,offset,TRUE,&mii);
			protodat[offset++]=pdesc[loop]->szName;
		}
	}

	RemoveMenu(hpopupProto,IDM_DUMMY,MF_BYCOMMAND);

	TrackPopupMenu(hpopupProto,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,hdummy,NULL);
	DestroyMenu(hpopupProto);
	SetWindowLong(hdummy,GWL_USERDATA,(LPARAM)wparam);
	return 0;
}


// Popup the Ignore - menu
int ShowIgnore(WPARAM wparam,LPARAM lparam)
{
	POINT pos;
	int loop,all;

	hpopupIgnore=GetSubMenu(LoadMenu(hinstance,MAKEINTRESOURCE(IDR_MENU)),0);
	GetCursorPos(&pos);

	for(all=0,loop=1;loop<=5;loop++)
	{
		if(CallService(MS_IGNORE_ISIGNORED,wparam,(LPARAM)loop))
		{
			all++;
			CheckMenuItem(hpopupIgnore,loop,MF_BYPOSITION|MF_CHECKED);
		}
	}
	if(all==5)
		CheckMenuItem(hpopupIgnore,0,MF_BYPOSITION|MF_CHECKED);

	TrackPopupMenu(hpopupIgnore,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,hdummy,NULL);
	DestroyMenu(hpopupIgnore);
	SetWindowLong(hdummy,GWL_USERDATA,(LPARAM)wparam);
	return 0;
}


// set the invisible-flag in db
int SetInvis(WPARAM wparam,LPARAM lparam)
{
	CallContactService((HANDLE)wparam,PSS_SETAPPARENTMODE,(DBGetContactSettingWord((HANDLE)wparam,(const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0),"ApparentMode",0)==ID_STATUS_OFFLINE)?0:ID_STATUS_OFFLINE,0);
	return 0;
}


// set visible-flag in db
int SetVis(WPARAM wparam,LPARAM lparam)
{
	CallContactService((HANDLE)wparam,PSS_SETAPPARENTMODE,(DBGetContactSettingWord((HANDLE)wparam,(const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0),"ApparentMode",0)==ID_STATUS_ONLINE)?0:ID_STATUS_ONLINE,0);
	return 0;
}

int Hide(WPARAM wparam,LPARAM lparam)
{
	DBWriteContactSettingByte((HANDLE)wparam,"CList","Hidden",1);
	CallService(MS_CLUI_SORTLIST,0,0);
	return 0;
}


// following 4 functions should be self-explanatory
void ModifyVisibleSet(CLISTMENUITEM *cli)
{
	cli->flags|=CMIF_CHECKED;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuVis,(LPARAM)cli);
}



void ModifyInvisSet(CLISTMENUITEM *cli)
{
	cli->flags|=CMIF_CHECKED;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuOff,(LPARAM)cli);
}



void ModifyHidden(CLISTMENUITEM *cli)
{
	cli->flags|=CMIF_CHECKED;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuHide,(LPARAM)cli);
}



void ShowItem(CLISTMENUITEM *cli,HMENU hmenu)
{
	cli->flags=CMIM_FLAGS;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenu,(LPARAM)cli);
}



void HideItem(CLISTMENUITEM *cli,HMENU hmenu)
{
	cli->flags|=CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenu,(LPARAM)cli);
}



// called when the contact-menu is built
int BuildMenu(WPARAM wparam,LPARAM lparam)
{
	CLISTMENUITEM cliAV={0},cliNV={0},cliHFL={0},cliIGN={0},cliPROTO={0},cliGRP={0},cliADD={0},cliREQ={0};
	BYTE flags=DBGetContactSettingByte(NULL,VISPLG,"flags",0);

	cliAV.cbSize=sizeof(CLISTMENUITEM);
	cliAV.flags=CMIM_FLAGS;
	cliAV.hIcon=NULL;
	cliAV.pszContactOwner=NULL;
	cliNV=cliHFL=cliIGN=cliPROTO=cliGRP=cliADD=cliREQ=cliAV;

	if(flags&VF_AV) ShowItem(&cliAV,hmenuVis);
	else HideItem(&cliAV,hmenuVis);

	if(flags&VF_NV) ShowItem(&cliNV,hmenuOff);
	else HideItem(&cliNV,hmenuOff);

	if(flags&VF_HFL) ShowItem(&cliHFL,hmenuHide);
	else HideItem(&cliHFL,hmenuHide);

	if(flags&VF_IGN) ShowItem(&cliIGN,hmenuIgnore);
	else HideItem(&cliIGN,hmenuIgnore);

	if(flags&VF_PROTO) ShowItem(&cliPROTO,hmenuProto);
	else HideItem(&cliPROTO,hmenuProto);

	if(flags&VF_GRP) ShowItem(&cliGRP,hmenuGroup);
	else HideItem(&cliGRP,hmenuGroup);

	if(flags&VF_ADD) ShowItem(&cliADD,hmenuAdded);
	else HideItem(&cliADD,hmenuAdded);

	if(flags&VF_REQ) ShowItem(&cliREQ,hmenuAuthReq);
	else HideItem(&cliREQ,hmenuAuthReq);

	if((flags&VF_AV) || (flags&VF_NV))
	{
		switch(DBGetContactSettingWord((HANDLE)wparam,(const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0),"ApparentMode",0))
		{
			case ID_STATUS_ONLINE:
				ModifyVisibleSet(&cliAV);
				//ShowItem(&cliNV,hmenuOff);
				break;

			case ID_STATUS_OFFLINE:
				ModifyInvisSet(&cliNV);
				//ShowItem(&cliAV,hmenuVis);
				break;

			default:
				break;
		}
	}

	if(DBGetContactSettingByte((HANDLE)wparam,"CList","Hidden",0))
		ModifyHidden(&cliHFL);

	return 0;
}



// called when all modules are loaded
static int PluginInit(WPARAM wparam,LPARAM lparam)
{
	CLISTMENUITEM cli;

	CreateServiceFunction(MS_SETINVIS,SetInvis);
	CreateServiceFunction(MS_SETVIS,SetVis);
	CreateServiceFunction(MS_HIDE,Hide);
	CreateServiceFunction(MS_IGNORE,ShowIgnore);
	CreateServiceFunction(MS_PROTO,ShowProto);
	CreateServiceFunction(MS_GROUP,ShowGroup);
	CreateServiceFunction(MS_ADDED,SendAdded);
	CreateServiceFunction(MS_AUTHREQ,SendAuthRequest);

	cli.cbSize=sizeof(CLISTMENUITEM);
	cli.flags=0;
	cli.hIcon=NULL;
	cli.pszContactOwner=NULL;

	cli.position=100000;
	cli.pszName=Translate("Always visible");
	cli.pszService=MS_SETVIS;
	hmenuVis=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

	cli.position=100010;
	cli.pszName=Translate("Never visible");
	cli.pszService=MS_SETINVIS;
	hmenuOff=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

	cli.position=100020;
	cli.pszName=Translate("Hide from list");
	cli.pszService=MS_HIDE;
	hmenuHide=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

	cli.position=100030;
	cli.pszName=Translate("Ignore...");
	cli.pszService=MS_IGNORE;
	hmenuIgnore=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

	cli.position=100040;
	cli.pszName=Translate("Protocol...");
	cli.pszService=MS_PROTO;
	hmenuProto=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

	cli.position=100050;
	cli.pszName=Translate("Group...");
	cli.pszService=MS_GROUP;
	hmenuGroup=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

	cli.position=100060;
	cli.pszName=Translate("Send 'You were added'");
	cli.pszService=MS_ADDED;
	hmenuAdded=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

	cli.position=100070;
	cli.pszName=Translate("Request authorization");
	cli.pszService=MS_AUTHREQ;
	hmenuAuthReq=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU,BuildMenu);

	HookEvent(ME_OPT_INITIALISE,OptionsInit);

	hdummy=CreateWindow("BUTTON","",0,0,0,0,0,NULL,NULL,hinstance,NULL);
	SetWindowLong(hdummy,GWL_WNDPROC,(LPARAM)CatchMenuMsg);
	return 0;
}


__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}


__declspec(dllexport)int Unload(void)
{
	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
	hinstance=hinst;
	return 1;
}


int __declspec(dllexport)Load(PLUGINLINK *link)
{
	pluginLink=link;
	HookEvent(ME_SYSTEM_MODULESLOADED,PluginInit);
	return 0;
}
