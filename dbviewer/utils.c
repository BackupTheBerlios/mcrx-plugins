#include "dbwiz.h"
#include "..\..\miranda32\ui\contactlist\m_clist.h"



extern MODULESTRUCT mod_dat;
extern CONTACTSTRUCT con_dat;



int EnumModules(const char *szModule,DWORD ofsModule,LPARAM lparam)
{
	mod_dat.module[mod_dat.modules++].pszModule=strdup(szModule);
	return 0;
}



int EnumSettings(const char *szSetting,LPARAM lparam)
{
	SETTINGSTRUCT *setting=(SETTINGSTRUCT *)lparam;

	free((char *)setting->setting[setting->settings].pszSetting);

	setting->setting[setting->settings++].pszSetting=strdup(szSetting);
	
	return 0;
}




int InitData(void)
{
	HANDLE hcontact;
	int loop=1;

	CallService(MS_DB_MODULES_ENUM,(LPARAM)0,(WPARAM)EnumModules);

	con_dat.contacts=(int)CallService(MS_DB_CONTACT_GETCOUNT,0,0)+1;

	con_dat.contact[0].hContact=NULL;
	con_dat.contact[0].pszContact=Translate("Main contact");

	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);

	while(hcontact!=NULL)
	{
		con_dat.contact[loop].hContact=hcontact;
		con_dat.contact[loop++].pszContact=strdup((const char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hcontact,0));

		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}

	return 0;
}



int ReInitUsers(HANDLE hdeleted)
{
	HANDLE hcontact;
	int loop=1;

	ZeroMemory(&con_dat,sizeof(con_dat));

	con_dat.contact[0].hContact=NULL;
	con_dat.contact[0].pszContact=Translate("Main contact");
	
	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);

	while(hcontact!=NULL)
	{
		if(hcontact!=hdeleted)
		{
			con_dat.contact[loop].hContact=hcontact;
			con_dat.contact[loop++].pszContact=strdup((const char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hcontact,0));
		}

		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}

	con_dat.contacts=loop;

	return 0;
}



int FillUserList(HWND hlist)
{
	int nItem,loop;
	
	for(loop=1;loop<con_dat.contacts;loop++)
	{
		nItem=ListBox_AddString(hlist,con_dat.contact[loop].pszContact);
		ListBox_SetItemData(hlist,nItem,con_dat.contact[loop].hContact);
	}

	nItem=ListBox_InsertString(hlist,con_dat.contact[0].pszContact);
	ListBox_SetItemData(hlist,nItem,NULL);

//	SendMessage(hlist,LB_SETCURSEL,0,0);
	return 0;
}



int ClearSettings(HWND htree)
{
	HWND hdlg=GetParent(htree);

	TreeView_DeleteAllItems(htree);
	SetWindowLong(GetDlgItem(hdlg,IDC_VALUE),GWL_USERDATA,0);
	SetDlgItemText(hdlg,IDC_TYPE,"");
	SetDlgItemText(hdlg,IDC_VALUE,"");
	return 0;
}



int SetModules(HWND htree,HANDLE hcontact)
{
	TVINSERTSTRUCT tvis;
	HTREEITEM htvi;
	SETTINGSTRUCT setting={0};
	DBCONTACTENUMSETTINGS dbces;
	int loop,doop;

	tvis.item.mask=TVIF_TEXT;
	dbces.pfnEnumProc=EnumSettings;
	dbces.lParam=(LPARAM)&setting;

	for(loop=0;loop<mod_dat.modules;loop++)
	{
		tvis.hParent=TVI_ROOT;
		tvis.hInsertAfter=TVI_SORT;
		tvis.item.pszText=(char *)mod_dat.module[loop].pszModule;
		tvis.item.cchTextMax=strlen(mod_dat.module[loop].pszModule);

		dbces.szModule=mod_dat.module[loop].pszModule;

		setting.settings=0;

		if(CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)hcontact,(LPARAM)&dbces)==-1)
			continue;

		htvi=TreeView_InsertItem(htree,&tvis);

		for(doop=0;doop<setting.settings;doop++)
		{
			tvis.hParent=htvi;
			tvis.item.mask|=TVIF_PARAM;
			tvis.item.pszText=strdup(setting.setting[doop].pszSetting);
			tvis.item.cchTextMax=strlen(setting.setting[doop].pszSetting);
			tvis.item.lParam=(LPARAM)strdup(mod_dat.module[loop].pszModule);
			TreeView_InsertItem(htree,&tvis);
		}
	}
	return 0;
}