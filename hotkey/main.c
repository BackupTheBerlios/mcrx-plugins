#include "hotkey.h"
#include "..\SDK\headers_c\m_system.h"
#include "..\SDK\headers_c\m_clist.h"
#include "..\SDK\headers_c\m_options.h"
#include "..\SDK\headers_c\m_langpack.h"
#include <string.h>



HINSTANCE hinstance;
HHOOK hhook;
HANDLE hevent[4];
HWND hmain;
BYTE ProtoExtension=0;
CONTACTSTRUCT ns;
HIMAGELIST himl;
PLUGINLINK *pluginLink;
PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
		"Hotkey Control",
		PLUGIN_MAKE_VERSION(3,2,1,0),
		"Open contact-specific windows by hotkey",
		"Heiko Schillinger",
		"micron@nexgo.de",
		"© 2001 Heiko Schillinger",
		"http://nortiq.com/miranda",
		0,
		0
};



BOOL CALLBACK DialogCallback(HWND,UINT,WPARAM,LPARAM);
int EventAdded(WPARAM,LPARAM);
int HotkeyChange(WPARAM,LPARAM);
void WordToModAndVk(WORD,UINT *,UINT *);
char *GetProto(HANDLE);


// simple sorting function to have
// the contact array in alphabetical order
void SortArray(void)
{
	int loop,doop;
	struct c_struct cs_temp;

	for(loop=0;loop<ns.count;loop++)
	{
		for(doop=loop+1;doop<ns.count;doop++)
		{
			if(lstrcmp(ns.contact[loop].szname,ns.contact[doop].szname)>0)
			{
				cs_temp=ns.contact[loop];
				ns.contact[loop]=ns.contact[doop];
				ns.contact[doop]=cs_temp;
			}
			else if(!lstrcmp(ns.contact[loop].szname,ns.contact[doop].szname))
			{
				if(lstrcmp(ns.contact[loop].proto,ns.contact[doop].proto)>0)
				{
					cs_temp=ns.contact[loop];
					ns.contact[loop]=ns.contact[doop];
					ns.contact[doop]=cs_temp;
				}
			}

		}
	}
}


// redraw the listbox and optionally resort the contact array
void DrawList(HWND hmain,BYTE sort)
{
	int loop;

	SendDlgItemMessage(hmain,IDC_USERNAME,CB_RESETCONTENT,0,0);

	if(sort) SortArray();

	for(loop=0;loop<ns.count;loop++)
		SendDlgItemMessage(hmain,IDC_USERNAME,CB_SETITEMDATA,(WPARAM)SendDlgItemMessage(hmain,IDC_USERNAME,CB_ADDSTRING,0,(LPARAM)ns.contact[loop].szname),(LPARAM)loop);
}


// remove deleted contacts from array
void RebuildArray(int pos)
{
	int loop;

	for(loop=pos;loop<ns.count-1;loop++)
	{
		ns.contact[loop]=ns.contact[loop+1];
	}
	ns.count--;
}


// check if the char(s) entered appears in a contacts name
int CheckText(HWND hdlg,char *sztext)
{
	int loop;

	if(!sztext[0])
		return 0;

	for(loop=0;loop<ns.count;loop++)
	{
		if(!strnicmp(sztext,ns.contact[loop].szname,lstrlen(sztext)))
		{
			SendMessage(hdlg,UM_UPDATE,(WPARAM)ns.contact[loop].szname,0);
			break;
		}
	}

	return 0;
}




// called when a db-setting is changed
int ContactRename(WPARAM wparam,LPARAM lparam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*) lparam;

	if((HANDLE)wparam==NULL) return 0;

	if(!lstrcmp(cws->szModule,"CList") && !lstrcmp(cws->szSetting,"MyHandle"))
		SendMessage(hmain,UM_NAMECHANGE,wparam,0);

	if((!lstrcmp(cws->szSetting,"UIN") || !lstrcmp(cws->szSetting,"Nick") || !lstrcmp(cws->szSetting,"FirstName") || !lstrcmp(cws->szSetting,"LastName") || !lstrcmp(cws->szSetting,"e-mail")))
		SendMessage(hmain,UM_NAMECHANGE,wparam,0);

	return 0;
}


// adds a contact to the array
int ContactAdd(WPARAM wparam,LPARAM lparam)
{
	ns.contact[ns.count++].hcontact=(HANDLE)wparam;
	return 0;
}


// contact was deleted from db
int ContactDelete(WPARAM wparam,LPARAM lparam)
{
	SendMessage(hmain,UM_DELETECONTACT,wparam,0);
	return 0;
}


// enumerate protocols (ICQ/MSN)
void GetProtocols(void)
{
	int loop;

	int pcount,protos=0;
	PROTOCOLDESCRIPTOR** pdesc;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)(int*)&pcount,(LPARAM)(PROTOCOLDESCRIPTOR***)&pdesc);

	for(loop=0;loop<pcount;loop++)
	{
		if(pdesc[loop]->type==PROTOTYPE_PROTOCOL)
			protos++;
	}

	if(protos>1)
		ProtoExtension=1;
}


// receive the protocol a contact is on
char *GetProto(HANDLE hcontact)
{
	char *pname;

	pname=strdup((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0));
	return pname?pname:"   ";
}


// called when all modules are loaded
int PluginInit(WPARAM wparam,LPARAM lparam)
{
	HANDLE hcontact,hlastsent;
	WORD hotkey;
	UINT mod,vk;

	// create main window and set the (translated) title
	hmain=CreateDialog(hinstance,MAKEINTRESOURCE(IDD_MAIN),NULL,(DLGPROC)DialogCallback);
	SetWindowText(hmain,Translate("Enter username:"));

	// read hotkey from db and register it with windows
	hotkey=DBGetContactSettingWord(NULL,HK_PLG,"Hotkey",MAKEWORD(0x4D,HOTKEYF_CONTROL));
	WordToModAndVk(hotkey,&mod,&vk);

	RegisterHotKey(hmain,HOTKEY_GENERAL,mod,vk);

	// hook some events
	hevent[0]=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,ContactRename);
	hevent[1]=HookEvent(ME_DB_CONTACT_ADDED,ContactAdd);
	hevent[2]=HookEvent(ME_DB_CONTACT_DELETED,ContactDelete);
	hevent[3]=HookEvent(ME_DB_EVENT_ADDED,EventAdded);

	HookEvent(ME_OPT_INITIALISE,HotkeyChange);

	// get the icons for the listbox
	himl=(HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST,0,0);

	// get available protocols
	GetProtocols();
	ns.count=0;

	// read last-sent-to contact from db and set handle as window-userdata
	hlastsent=(HANDLE)DBGetContactSettingDword(NULL,HK_PLG,"LastSentTo",-1);
	SetWindowLong(hmain,GWL_USERDATA,(LONG)hlastsent);

	// enumerate all contacts and write them to the array
	// item data of listbox-strings is the array position
	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hcontact!=NULL)
	{
		char *pszProto=GetProto(hcontact);
		if(pszProto!=NULL)
		{
			lstrcpyn(ns.contact[ns.count].szname,(const char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hcontact,0),sizeof ns.contact[ns.count].szname);
			strcpy(ns.contact[ns.count].proto,GetProto(hcontact));
			ns.contact[ns.count++].hcontact=hcontact;
		}

		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}

	// fill the listbox
	DrawList(hmain,1);

	DBWriteContactSettingString(NULL,"Uninstall","Hotkey",HK_PLG);

	return 0;
}


__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}


__declspec(dllexport)int Unload(void)
{
	int loop=0;

	DestroyWindow(hmain);
	UnregisterHotKey(hmain,HOTKEY_GENERAL);

	for(;loop<4;)
		UnhookEvent(hevent[loop++]);

	for(loop=0;loop<ns.count;loop++)
		free(ns.contact[loop].szname);

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