#include "uninstall.h"
#include "..\..\miranda32\ui\contactlist\m_clist.h"
#include "..\..\miranda32\random\langpack\m_langpack.h"



int EnumPlugins(const char *szSetting,LPARAM lparam)
{
	HWND hlist=(HWND)lparam;

	ListBox_AddString(hlist,strdup(szSetting));

	return 0;
}



int EnumSettings(const char *szSetting,LPARAM lparam)
{
	TVINSERTSTRUCT tvis;
	HANDOVER *phndvr=(HANDOVER *)lparam;

	tvis.hParent=phndvr->htvi;
	tvis.hInsertAfter=TVI_LAST;

	tvis.item.mask=TVIF_TEXT;
	tvis.item.pszText=(char *)szSetting;
	tvis.item.cchTextMax=strlen(szSetting);

	TreeView_InsertItem(phndvr->htree,&tvis);
	phndvr->valid=1;

	return 0;
}



int SweepSettings(const char *szSetting,LPARAM lparam)
{
	DELETESTRUCT *pdel=(DELETESTRUCT *)lparam;

	pdel->pszSetting[pdel->count++]=strdup(szSetting);

//	DBDeleteContactSetting(pdel->hcontact,pdel->pszModule,szSetting);
	return 0;
}



int FillPluginList(HWND hlist)
{
	DBCONTACTENUMSETTINGS dbces;

	dbces.pfnEnumProc=EnumPlugins;
	dbces.szModule="Uninstall";
	dbces.lParam=(LPARAM)hlist;

	if(CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)NULL,(LPARAM)&dbces)==-1)
		return 1;

	return 0;
}



int FillSettingsBox(HWND htree,const char *szModule)
{
	DBCONTACTENUMSETTINGS dbces;
	HANDOVER hndvr;
	HANDLE hcontact;
	TVINSERTSTRUCT tvis;
	HTREEITEM htvi;
	char *pszContact;

	hndvr.htree=htree;
	hndvr.valid=0;
	dbces.lParam=(LPARAM)&hndvr;
	dbces.pfnEnumProc=EnumSettings;
	dbces.szModule=szModule;

	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hcontact!=NULL)
	{
		pszContact=strdup((const char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hcontact,0));

		tvis.hParent=TVI_ROOT;
		tvis.hInsertAfter=TVI_SORT;
		tvis.item.mask=TVIF_TEXT|TVIF_PARAM;
		tvis.item.pszText=pszContact;
		tvis.item.cchTextMax=strlen(pszContact);
		tvis.item.lParam=(LPARAM)hcontact;

		htvi=TreeView_InsertItem(htree,&tvis);

		hndvr.htvi=htvi;

		if(CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)hcontact,(LPARAM)&dbces)==-1)
			TreeView_DeleteItem(htree,htvi);

		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}

	hcontact=NULL;
	pszContact=Translate("Main contact");

	tvis.hParent=TVI_ROOT;
	tvis.hInsertAfter=TVI_FIRST;
	tvis.item.mask=TVIF_TEXT|TVIF_PARAM;
	tvis.item.pszText=pszContact;
	tvis.item.cchTextMax=strlen(pszContact);
	tvis.item.lParam=(LPARAM)NULL;

	htvi=TreeView_InsertItem(htree,&tvis);

	hndvr.htvi=htvi;

	if(CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)hcontact,(LPARAM)&dbces)==-1)
		TreeView_DeleteItem(htree,htvi);

	return hndvr.valid;
}



int SweepPlugin(const char *szModule,const char *szPlugin)
{
	HANDLE hcontact;
	DBCONTACTENUMSETTINGS dbces;
	DELETESTRUCT del;

	dbces.szModule=szModule;
	dbces.pfnEnumProc=SweepSettings;
	dbces.lParam=(LPARAM)&del;

	del.pszModule=szModule;
	
	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hcontact!=NULL)
	{
		del.hcontact=hcontact;
		del.count=0;
		CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)hcontact,(LPARAM)&dbces);
		for(;del.count;del.count--)
		{
			DBDeleteContactSetting(del.hcontact,del.pszModule,del.pszSetting[del.count-1]);
			free((char *)del.pszSetting[del.count-1]);
		}
			
		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}

	del.hcontact=NULL;
	del.count=0;

	CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)del.hcontact,(LPARAM)&dbces);

	for(;del.count;del.count--)
	{
		DBDeleteContactSetting(del.hcontact,del.pszModule,del.pszSetting[del.count-1]);
		free((char *)del.pszSetting[del.count-1]);
	}

	DBDeleteContactSetting(NULL,"Uninstall",szPlugin);

	return 0;
}



int SweepContactSettings(HANDLE hcontact,const char *szModule)
{
	DBCONTACTENUMSETTINGS dbces;
	DELETESTRUCT del;

	dbces.szModule=szModule;
	dbces.pfnEnumProc=SweepSettings;
	dbces.lParam=(LPARAM)&del;

	del.pszModule=szModule;

	del.count=0;
	
	CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)hcontact,(LPARAM)&dbces);
	for(;del.count;del.count--)
	{
		DBDeleteContactSetting(hcontact,del.pszModule,del.pszSetting[del.count-1]);
		free((char *)del.pszSetting[del.count-1]);
	}

	return 0;
}