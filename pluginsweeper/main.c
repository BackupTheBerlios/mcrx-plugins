#include "uninstall.h"
#include "..\..\miranda32\core\m_system.h"
#include "..\..\miranda32\ui\options\m_options.h"



HINSTANCE hinstance;
DWORD dwmirver;
PLUGINLINK *pluginLink;
PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
		"Plugin sweeper",
		PLUGIN_MAKE_VERSION(0,4,1,0),
		"Remove all DB entries of a plugin",
		"Heiko Schillinger",
		"micron@nexgo.de",
		"© 2002 Heiko Schillinger",
		"http://nortiq.com/miranda",
		0,
		0
};



int StartDialog(WPARAM,LPARAM);



int AddPlugin(WPARAM wparam,LPARAM lparam)
{
	DBWriteContactSettingString(NULL,"Uninstall",(const char *)wparam,(const char *)lparam);
	return 0;
}



int MainInit(WPARAM wparam,LPARAM lparam)
{
	CreateServiceFunction("PluginSweep/Add",AddPlugin);

	HookEvent(ME_OPT_INITIALISE,StartDialog);

	return 0;
}



__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	dwmirver=mirandaVersion;
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
	// this isn't required for most events
	// but the ME_USERINFO_INITIALISE
	// I decided to hook all events after
	// everything is loaded because it seems
	// to be safer in my opinion
	HookEvent(ME_SYSTEM_MODULESLOADED,MainInit);
	return 0;
}