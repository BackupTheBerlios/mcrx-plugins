#include "menuex.h"


extern HINSTANCE hinstance;


BOOL CALLBACK OptionsProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	BYTE flags=DBGetContactSettingByte(NULL,VISPLG,"flags",0);
	
	switch(msg)
	{
		case WM_INITDIALOG:
			
			CheckDlgButton(hdlg,IDC_ALWAYSVIS,(flags&VF_AV)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_NEVERVIS,(flags&VF_NV)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_HIDE,(flags&VF_HFL)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_IGNORE,(flags&VF_IGN)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_PROTOS,(flags&VF_PROTO)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_GROUPS,(flags&VF_GRP)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_ADDED,(flags&VF_ADD)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hdlg,IDC_AUTHREQ,(flags&VF_REQ)?BST_CHECKED:BST_UNCHECKED);
			TranslateDialogDefault(hdlg);
			return 0;

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->code){
				case PSN_APPLY:
				{
					BYTE mod_flags=0;

					mod_flags|=IsDlgButtonChecked(hdlg,IDC_ALWAYSVIS)?VF_AV:0;
					mod_flags|=IsDlgButtonChecked(hdlg,IDC_NEVERVIS)?VF_NV:0;
					mod_flags|=IsDlgButtonChecked(hdlg,IDC_HIDE)?VF_HFL:0;
					mod_flags|=IsDlgButtonChecked(hdlg,IDC_IGNORE)?VF_IGN:0;
					mod_flags|=IsDlgButtonChecked(hdlg,IDC_PROTOS)?VF_PROTO:0;
					mod_flags|=IsDlgButtonChecked(hdlg,IDC_GROUPS)?VF_GRP:0;
					mod_flags|=IsDlgButtonChecked(hdlg,IDC_ADDED)?VF_ADD:0;
					mod_flags|=IsDlgButtonChecked(hdlg,IDC_AUTHREQ)?VF_REQ:0;
					DBWriteContactSettingByte(NULL,VISPLG,"flags",mod_flags);
					
					return 1;
				}
			}
			break;

		case WM_COMMAND:
			if(HIWORD(wparam)==BN_CLICKED && GetFocus()==(HWND)lparam)
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);

			return 0;

		case WM_CLOSE:
			EndDialog(hdlg,0);
			return 0;
	}
	return 0;
}

int OptionsInit(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE odp={0};

	odp.cbSize=sizeof(odp);
    odp.position=955000000;
    odp.hInstance=hinstance;
    odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
    odp.pszTitle=Translate("MenuItemEx");
    odp.pfnDlgProc=OptionsProc;
	odp.pszGroup=Translate("Plugins");
	odp.flags=ODPF_BOLDGROUPS;

    CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);
	return 0;
}
