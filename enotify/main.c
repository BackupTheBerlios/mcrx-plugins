#include "msgpopup.h"
#include "..\SDK\headers_c\m_system.h"
#include "..\SDK\headers_c\m_options.h"
#include "..\SDK\headers_c\m_utils.h"



struct EXTENDSTRUCT extend[MAX_STYLES]={FLAT_WIDTH,FLAT_HEIGHT,S3D_WIDTH,S3D_HEIGHT,MINIMAL_WIDTH,MINIMAL_HEIGHT};
RECT screen;
BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
HINSTANCE hinstance;
PLUGINLINK *pluginLink;

PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
		"Message popup",
		PLUGIN_MAKE_VERSION(0,1,5,4),
		"Pops up a window on various events",
		"Heiko Schillinger",
		"micron@nexgo.de",
		"© 2002 Heiko Schillinger",
		"http://nortiq.com/miranda",
		0,
		0
};


// utils.c
int CheckNewEvents(WPARAM,LPARAM);
// options.c
int InitOptions(WPARAM,LPARAM);



int MainInit(WPARAM wparam,LPARAM lparam)
{

#ifdef DEBUG
	MessageBox(NULL,"MainInit() was called","DEBUG",MB_OK);
#endif

	HookEvent(ME_DB_EVENT_ADDED,CheckNewEvents);
	HookEvent(ME_OPT_INITIALISE,InitOptions);

	// Plugin sweeper support
	DBWriteContactSettingString(NULL,"Uninstall","Event Notify",MODULE);

	return 0;
}



__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
#ifdef DEBUG
	MessageBox(NULL,"MirandaPluginInfo() was called","DEBUG",MB_OK);
#endif
	return &pluginInfo;
}



__declspec(dllexport)int Unload(void)
{
	return 0;
}



BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
	hinstance=hinst;

#ifdef DEBUG
	MessageBox(NULL,"DllMain() was called","DEBUG",MB_OK);
#endif
	// get the working area rect (needed for popup-placement
	SystemParametersInfo(SPI_GETWORKAREA,0,&screen,0);
	return TRUE;
}



int __declspec(dllexport)Load(PLUGINLINK *link)
{
	HMODULE hUserDll;
	pluginLink=link;

#ifdef DEBUG
	MessageBox(NULL,"Load() was called","DEBUG",MB_OK);
#endif

	// check if transparency is available
	hUserDll=LoadLibrary("user32.dll");
	if(hUserDll)
		MySetLayeredWindowAttributes=(BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");

	HookEvent(ME_SYSTEM_MODULESLOADED,MainInit);
	return 0;
}