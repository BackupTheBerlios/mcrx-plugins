#include "hotkey.h"
#include "..\SDK\headers_c\m_system.h"
#include "..\SDK\headers_c\m_options.h"
#include "..\SDK\headers_c\m_langpack.h"

#define PSN_APPLY_MOD -201

#define ENABLE(a,b)	EnableWindow(GetDlgItem(hdlg,a),b)
#define CHECK(a)	CheckRadioButton(hdlg,IDC_GLOBAL,IDC_LOCAL,a)



extern HINSTANCE hinstance;
extern HWND hmain;


// copied from miranda-source
void WordToModAndVk(WORD w,UINT *mod,UINT *vk)
{
        *mod=0;
        if(HIBYTE(w)&HOTKEYF_CONTROL) *mod|=MOD_CONTROL;
        if(HIBYTE(w)&HOTKEYF_SHIFT) *mod|=MOD_SHIFT;
        if(HIBYTE(w)&HOTKEYF_ALT) *mod|=MOD_ALT;
        if(HIBYTE(w)&HOTKEYF_EXT) *mod|=MOD_WIN;
        *vk=LOBYTE(w);
}


// options-page callback function
BOOL CALLBACK HotkeyChangeProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	WORD hotkey;
	UINT mod,vk;
	BYTE last;
	
	switch(msg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessage(hdlg,IDC_HOTKEY,HKM_SETRULES,(WPARAM)HKCOMB_NONE,MAKELPARAM(HOTKEYF_CONTROL,0));
			SendDlgItemMessage(hdlg,IDC_HOTKEY,HKM_SETHOTKEY,(WPARAM)(WORD)DBGetContactSettingWord(NULL,HK_PLG,"Hotkey",MAKEWORD(0x4D,HOTKEYF_CONTROL)),0);
			SetFocus(GetDlgItem(hdlg,IDC_HOTKEY));
			CheckDlgButton(hdlg,IDC_LASTSENTTO,(last=DBGetContactSettingByte(NULL,HK_PLG,"EnableLastSentTo",0)?BST_CHECKED:BST_UNCHECKED));
			CHECK(DBGetContactSettingWord(NULL,HK_PLG,"MsgTypeRec",IDC_GLOBAL));
			if(last)
			{
				ENABLE(IDC_GLOBAL,1);
				ENABLE(IDC_LOCAL,1);
			}
			TranslateDialogDefault(hdlg);
			return 0;

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->code)
			{
				case PSN_APPLY_MOD:
					hotkey=(WORD)SendDlgItemMessage(hdlg,IDC_HOTKEY,HKM_GETHOTKEY,0,0);
					DBWriteContactSettingWord(NULL,HK_PLG,"Hotkey",hotkey);
					WordToModAndVk(hotkey,&mod,&vk);
					UnregisterHotKey(hmain,HOTKEY_GENERAL);
					RegisterHotKey(hmain,HOTKEY_GENERAL,mod,vk);

					DBWriteContactSettingByte(NULL,HK_PLG,"EnableLastSentTo",(BYTE)IsDlgButtonChecked(hdlg,IDC_LASTSENTTO));

					DBWriteContactSettingWord(NULL,HK_PLG,"MsgTypeRec",(WORD)(IsDlgButtonChecked(hdlg,IDC_GLOBAL)?IDC_GLOBAL:IDC_LOCAL));
					return 1;
			}
			break;

		case WM_COMMAND:
			if((HIWORD(wparam)==BN_CLICKED || HIWORD(wparam)==EN_CHANGE) && GetFocus()==(HWND)lparam)
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);

			if(LOWORD(wparam)==IDC_LASTSENTTO)
			{
				last=IsDlgButtonChecked(hdlg,IDC_LASTSENTTO);
				ENABLE(IDC_GLOBAL,!last);
				ENABLE(IDC_LOCAL,!last);
			}

			return 0;

		case WM_DESTROY:
		case WM_CLOSE:
			EndDialog(hdlg,0);
			return 0;
	}
	return 0;
}


// add options-page
int HotkeyChange(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
    odp.cbSize=sizeof(odp);
    odp.position=945000000;
    odp.hInstance=hinstance;
    odp.pszTemplate=MAKEINTRESOURCE(IDD_HOTKEYSELECT);
    odp.pszTitle="Hotkey";
    odp.pfnDlgProc=HotkeyChangeProc;
	odp.pszGroup=Translate("Plugins");
	odp.flags=ODPF_BOLDGROUPS;
    CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);

	return 0;
}
