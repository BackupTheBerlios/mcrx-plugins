#include "seen.h"
#include "..\..\miranda32\ui\contactlist\m_clist.h"
#include "..\..\miranda32\random\skin\m_skin.h"



HANDLE hmenuitem=NULL;



char *ParseString(char *,HANDLE,BYTE);



int MenuitemClicked(WPARAM wparam,LPARAM lparam)
{
	return 0;
}



int BuildContactMenu(WPARAM wparam,LPARAM lparam)
{
	CLISTMENUITEM cmi;
	DBVARIANT dbv;
	int id=-1,isetting;

	ZeroMemory(&cmi,sizeof(cmi));
	cmi.cbSize=sizeof(cmi);

	if(!DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1))
		cmi.flags=CMIM_FLAGS|CMIF_HIDDEN;
	
	else
	{
		cmi.flags=CMIM_NAME|CMIM_FLAGS|CMIM_ICON;
		cmi.hIcon=NULL;
		cmi.pszName=ParseString(!DBGetContactSetting(NULL,S_MOD,"MenuStamp",&dbv)?dbv.pszVal:"%d.%m.%Y - %H:%M [%s]",(HANDLE)wparam,0);
		
		if(!strcmp(cmi.pszName,Translate("<unknown>")))
			cmi.flags|=CMIF_GRAYED;

		if(DBGetContactSettingByte(NULL,S_MOD,"ShowIcon",1))
		{
			isetting=DBGetContactSettingWord((HANDLE)wparam,S_MOD,"Status",-1);
			
			switch(isetting){
			
				default:
					id=isetting-ID_STATUS_OFFLINE;
					break;

				case ID_STATUS_DND:
					id=SKINICON_STATUS_DND;
					break;

				case ID_STATUS_NA:
					id=SKINICON_STATUS_NA;
					break;

				case ID_STATUS_OCCUPIED:
					id=SKINICON_STATUS_OCCUPIED;
					break;

				case -1:
					break;
			}

			cmi.hIcon=LoadSkinnedIcon(id);
		}
	}
	
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuitem,(LPARAM)&cmi);

	DBFreeVariant(&dbv);

	return 0;
}



void InitMenuitem(void)
{
	CLISTMENUITEM cmi;

	CreateServiceFunction("Foo",MenuitemClicked);

	ZeroMemory(&cmi,sizeof(cmi));
	cmi.cbSize=sizeof(cmi);
	cmi.flags=0;
	cmi.hIcon=NULL;
	cmi.hotKey=0;
	cmi.position=-0x7FFFFFFF;
	cmi.pszContactOwner=NULL;
	cmi.pszName="Test";
	cmi.pszService="Foo";
	
	hmenuitem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cmi);
	
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU,BuildContactMenu);
}