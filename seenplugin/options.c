#include "seen.h"
#include "..\..\miranda32\ui\options\m_options.h"
#include "..\..\miranda32\ui\contactlist\m_clist.h"
#include "..\..\miranda32\ui\userinfo\m_userinfo.h"
#include "..\..\miranda32\protocols\protocols\m_protocols.h"



char szout1[512]="",szout2[512]="",szout3[512]="";



extern HINSTANCE hinstance;
extern HANDLE ehuserinfo,hmenuitem,ehmissed_proto;
void BuildInfo(char *,char *,char *);
int BuildContactMenu(WPARAM,LPARAM);
int UserinfoInit(WPARAM,LPARAM);
int InitFileOutput(void);
void ShutdownFileOutput(void);
void InitMenuitem(void);
int ModeChange_mo(WPARAM,LPARAM);
int CheckIfOnline(void);
int ResetMissed(void);



BOOL CALLBACK OptDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	DBVARIANT dbv;
	char szstamp[256];
	BYTE bchecked=0;
	WPARAM wpsend=0;
	
	switch(msg){
		
		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);

			wsprintf(szout1,"%%Y: %s\n%%d: %s\n%%p: %s\n%%n: %s\n%%i: %s\n%%t: %s\n%%E: %s",Translate("year (4 digits)"),Translate("day"),Translate("AM/PM"),Translate("username"),Translate("external IP"),Translate("tabulator"),Translate("name of month"));
			wsprintf(szout2,"%%y: %s\n%%H: %s\n%%M: %s\n%%u: %s\n%%r: %s\n%%W: %s\n%%e: %s",Translate("year (2 digits)"),Translate("hours (24)"),Translate("minutes"),Translate("UIN/handle"),Translate("internal IP"),Translate("weekday (full)"),Translate("short name of month"));
			wsprintf(szout3,"%%m: %s\n%%h: %s\n%%S: %s\n%%s: %s\n%%b: %s\n%%w: %s",Translate("month"),Translate("hours (12)"),Translate("seconds"),Translate("status"),Translate("line break"),Translate("weekday (abbreviated)"));

			SetDlgItemText(hdlg,IDC_VARIABLES1,szout1);
			SetDlgItemText(hdlg,IDC_VARIABLES2,szout2);
			SetDlgItemText(hdlg,IDC_VARIABLES3,szout3);

			free(szout1);free(szout2);free(szout3);

			CheckDlgButton(hdlg,IDC_MENUITEM,DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1));
			CheckDlgButton(hdlg,IDC_USERINFO,DBGetContactSettingByte(NULL,S_MOD,"UserinfoTab",1));
			CheckDlgButton(hdlg,IDC_FILE,DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0));
			CheckDlgButton(hdlg,IDC_IGNOREOFFLINE,DBGetContactSettingByte(NULL,S_MOD,"IgnoreOffline",1));
			CheckDlgButton(hdlg,IDC_MISSEDONES,DBGetContactSettingByte(NULL,S_MOD,"MissedOnes",0));
			CheckDlgButton(hdlg,IDC_SHOWICON,DBGetContactSettingByte(NULL,S_MOD,"ShowIcon",1));
			CheckDlgButton(hdlg,IDC_COUNT,DBGetContactSettingByte(NULL,S_MOD,"MissedOnes_Count",0));

			EnableWindow(GetDlgItem(hdlg,IDC_MENUSTAMP),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
			EnableWindow(GetDlgItem(hdlg,IDC_SHOWICON),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
			EnableWindow(GetDlgItem(hdlg,IDC_USERSTAMP),IsDlgButtonChecked(hdlg,IDC_USERINFO));
			EnableWindow(GetDlgItem(hdlg,IDC_FILESTAMP),IsDlgButtonChecked(hdlg,IDC_FILE));
			EnableWindow(GetDlgItem(hdlg,IDC_FILENAME),IsDlgButtonChecked(hdlg,IDC_FILE));
			EnableWindow(GetDlgItem(hdlg,IDC_COUNT),IsDlgButtonChecked(hdlg,IDC_MISSEDONES));

			SetDlgItemText(hdlg,IDC_MENUSTAMP,!DBGetContactSetting(NULL,S_MOD,"MenuStamp",&dbv)?dbv.pszVal:"%d.%m.%Y - %H:%M [%s]");
			SetDlgItemText(hdlg,IDC_USERSTAMP,!DBGetContactSetting(NULL,S_MOD,"UserStamp",&dbv)?dbv.pszVal:"Date: %d.%m.%Y%b%bTime: %H:%M:%S%b%bStatus: %s");
			SetDlgItemText(hdlg,IDC_FILESTAMP,!DBGetContactSetting(NULL,S_MOD,"FileStamp",&dbv)?dbv.pszVal:"%d.%m.%Y @ %H:%M:%S   %n[%u | %i] new status: %s");
			SetDlgItemText(hdlg,IDC_FILENAME,!DBGetContactSetting(NULL,S_MOD,"FileName",&dbv)?dbv.pszVal:"logs\\seen.txt");

			DBFreeVariant(&dbv);
			break;

		case WM_COMMAND:
			if((HIWORD(wparam)==BN_CLICKED || HIWORD(wparam)==EN_CHANGE) && GetFocus()==(HWND)lparam)
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);

			if(HIWORD(wparam)==BN_CLICKED)
			{
				switch(LOWORD(wparam)){
					case IDC_MENUITEM:
						EnableWindow(GetDlgItem(hdlg,IDC_MENUSTAMP),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
						EnableWindow(GetDlgItem(hdlg,IDC_SHOWICON),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
						break;
					case IDC_USERINFO:
						EnableWindow(GetDlgItem(hdlg,IDC_USERSTAMP),IsDlgButtonChecked(hdlg,IDC_USERINFO));
						break;
					case IDC_FILE:
						EnableWindow(GetDlgItem(hdlg,IDC_FILESTAMP),IsDlgButtonChecked(hdlg,IDC_FILE));
						EnableWindow(GetDlgItem(hdlg,IDC_FILENAME),IsDlgButtonChecked(hdlg,IDC_FILE));
						break;
					case IDC_MISSEDONES:
						EnableWindow(GetDlgItem(hdlg,IDC_COUNT),IsDlgButtonChecked(hdlg,IDC_MISSEDONES));
						break;
				}
			}
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->code){
				case PSN_APPLY:
					GetDlgItemText(hdlg,IDC_MENUSTAMP,szstamp,256);
					DBWriteContactSettingString(NULL,S_MOD,"MenuStamp",szstamp);

					GetDlgItemText(hdlg,IDC_USERSTAMP,szstamp,256);
					DBWriteContactSettingString(NULL,S_MOD,"UserStamp",szstamp);

					GetDlgItemText(hdlg,IDC_FILESTAMP,szstamp,256);
					DBWriteContactSettingString(NULL,S_MOD,"FileStamp",szstamp);

					GetDlgItemText(hdlg,IDC_FILENAME,szstamp,256);
					DBWriteContactSettingString(NULL,S_MOD,"FileName",szstamp);

					wpsend=MAKEWPARAM((BYTE)DBGetContactSettingByte(NULL,S_MOD,"UserinfoTab",1),(BYTE)DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0));
										
					DBWriteContactSettingByte(NULL,S_MOD,"MenuItem",(BYTE)IsDlgButtonChecked(hdlg,IDC_MENUITEM));
					DBWriteContactSettingByte(NULL,S_MOD,"UserinfoTab",(BYTE)IsDlgButtonChecked(hdlg,IDC_USERINFO));
					DBWriteContactSettingByte(NULL,S_MOD,"FileOutput",(BYTE)IsDlgButtonChecked(hdlg,IDC_FILE));
					DBWriteContactSettingByte(NULL,S_MOD,"IgnoreOffline",(BYTE)IsDlgButtonChecked(hdlg,IDC_IGNOREOFFLINE));
					DBWriteContactSettingByte(NULL,S_MOD,"MissedOnes",(BYTE)IsDlgButtonChecked(hdlg,IDC_MISSEDONES));
					DBWriteContactSettingByte(NULL,S_MOD,"ShowIcon",(BYTE)IsDlgButtonChecked(hdlg,IDC_SHOWICON));
					DBWriteContactSettingByte(NULL,S_MOD,"MissedOnes_Count",(BYTE)IsDlgButtonChecked(hdlg,IDC_COUNT));

					SendMessage(hdlg,UM_CHECKHOOKS,wpsend,0);

					break;
			}
			break;

		case UM_CHECKHOOKS:
			if((bchecked=(BYTE)DBGetContactSettingByte(NULL,S_MOD,"UserinfoTab",1))!=LOWORD(wparam))
			{
				switch(bchecked){
					case 1:
						ehuserinfo=HookEvent(ME_USERINFO_INITIALISE,UserinfoInit);
						break;
					case 0:
						UnhookEvent(ehuserinfo);
						break;
				}
			}

			if((bchecked=(BYTE)DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))!=HIWORD(wparam))
			{
				switch(bchecked){
					case 1:
						InitFileOutput();
						break;
					case 0:
						break;
				}
			}

			if(hmenuitem==NULL && DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1))
				InitMenuitem();

			if((bchecked=(BYTE)DBGetContactSettingByte(NULL,S_MOD,"MissedOnes",1))!=LOWORD(wparam))
			{
				switch(bchecked){
					case 1:
						ehmissed_proto=HookEvent(ME_PROTO_ACK,ModeChange_mo);
						break;
					case 0:
						UnhookEvent(ehmissed_proto);
						break;
				}
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
	odp.pszTitle=Translate("Last seen");
	odp.pfnDlgProc=OptDlgProc;
	odp.flags=ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);
	return 0;
}