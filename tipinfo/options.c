#include "tooltip.h"
#include "resource.h"





extern HINSTANCE hinstance;



BOOL CALLBACK OptDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	DBVARIANT dbv;
	char szstamp[512];
	
	switch(msg){
		
		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);

			CheckDlgButton(hdlg,IDC_FOCUSED,DBGetContactSettingByte(NULL,TIP_MOD,"Focused",1));
			CheckDlgButton(hdlg,IDC_CONTACT,DBGetContactSettingByte(NULL,TIP_MOD,"Contact",1));
			CheckDlgButton(hdlg,IDC_GROUP,DBGetContactSettingByte(NULL,TIP_MOD,"Group",0));
			CheckDlgButton(hdlg,IDC_REMOVE,DBGetContactSettingByte(NULL,TIP_MOD,"RemoveBreak",1));

			EnableWindow(GetDlgItem(hdlg,IDC_TEMPLATE),IsDlgButtonChecked(hdlg,IDC_CONTACT));

			SendDlgItemMessage(hdlg,IDC_TEMPLATE,EM_LIMITTEXT,(WPARAM)511,0);
			SetDlgItemText(hdlg,IDC_TEMPLATE,!DBGetContactSetting(NULL,TIP_MOD,"Template",&dbv)?dbv.pszVal:"%id\r\n%mail\r\n%smsg");
			SetDlgItemInt(hdlg,IDC_DELAY,(int)CallService(MS_CLC_GETINFOTIPHOVERTIME,0,0),0);
			
			DBFreeVariant(&dbv);
			break;

		case WM_COMMAND:
			if((HIWORD(wparam)==BN_CLICKED || HIWORD(wparam)==EN_CHANGE) && GetFocus()==(HWND)lparam)
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);

			if(HIWORD(wparam)==BN_CLICKED && LOWORD(wparam)==IDC_CONTACT)
				EnableWindow(GetDlgItem(hdlg,IDC_TEMPLATE),IsDlgButtonChecked(hdlg,IDC_CONTACT));

			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->code){
				case PSN_APPLY:
					GetDlgItemText(hdlg,IDC_TEMPLATE,szstamp,512);
					DBWriteContactSettingString(NULL,TIP_MOD,"Template",szstamp);

					CallService(MS_CLC_SETINFOTIPHOVERTIME,(WPARAM)GetDlgItemInt(hdlg,IDC_DELAY,NULL,0),0);
										
					DBWriteContactSettingByte(NULL,TIP_MOD,"Focused",(BYTE)IsDlgButtonChecked(hdlg,IDC_FOCUSED));
					DBWriteContactSettingByte(NULL,TIP_MOD,"Contact",(BYTE)IsDlgButtonChecked(hdlg,IDC_CONTACT));
					DBWriteContactSettingByte(NULL,TIP_MOD,"Group",(BYTE)IsDlgButtonChecked(hdlg,IDC_GROUP));
					DBWriteContactSettingByte(NULL,TIP_MOD,"RemoveBreak",(BYTE)IsDlgButtonChecked(hdlg,IDC_REMOVE));
					break;
			}
			break;

	
	}

	return 0;
}



int OptionsInit(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=150000000;
	odp.pszGroup=Translate("Plugins");
	odp.groupPosition=910000000;
	odp.hInstance=hinstance;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszTitle=Translate("Tool Tip");
	odp.pfnDlgProc=OptDlgProc;
	odp.flags=ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);
	return 0;
}