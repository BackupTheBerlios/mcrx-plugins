#include "dbwiz.h"
#include "..\SDK\headers_c\m_system.h"
#include "..\SDK\headers_c\m_clist.h"
#include "..\SDK\headers_c\m_skin.h"



HINSTANCE hinstance;
HANDLE hevent[3];
MODULESTRUCT mod_dat;
CONTACTSTRUCT con_dat;
PLUGINLINK *pluginLink;
PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
		"Database viewer",
		PLUGIN_MAKE_VERSION(1,5,1,0),
		"View, change and delete DB settings",
		"Heiko Schillinger",
		"micron@nexgo.de",
		"© 2001-2002 Heiko Schillinger",
		"http://nortiq.com/miranda",
		0,
		0
};



int StartDialog(WPARAM,LPARAM);



int InitMenu(WPARAM wparam,LPARAM lparam)
{
	CLISTMENUITEM cmi;

	CreateServiceFunction("DBwiz/Show",StartDialog);

	ZeroMemory(&cmi,sizeof(cmi));
	cmi.cbSize=sizeof(cmi);
	cmi.pszName=Translate("Database viewer");
	cmi.position=1900000010;
	cmi.pszPopupName=NULL;
	cmi.flags=0;
	cmi.pszService="DBwiz/Show";
	cmi.hIcon=LoadIcon(hinstance,MAKEINTRESOURCE(IsWinVerXPPlus()?IDI_32BIT:IDI_8BIT));
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&cmi);

	return 0;
}



__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}


__declspec(dllexport)int Unload(void)
{
	int loop;

	for(loop=0;loop<3;loop++)
		UnhookEvent(hevent[loop]);
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
	
	ZeroMemory(&mod_dat,sizeof(mod_dat));
	ZeroMemory(&con_dat,sizeof(con_dat));

	// this isn't required for most events
	// but the ME_USERINFO_INITIALISE
	// I decided to hook all events after
	// everything is loaded because it seems
	// to be safer in my opinion
	HookEvent(ME_SYSTEM_MODULESLOADED,InitMenu);
	return 0;
}