#include <windows.h>
#include <commctrl.h>

#include "seen.h"



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

	switch(msg)
	{
		
		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);

			CheckDlgButton(hdlg,IDC_MENUITEM,DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1));
			CheckDlgButton(hdlg,IDC_USERINFO,DBGetContactSettingByte(NULL,S_MOD,"UserinfoTab",1));
			CheckDlgButton(hdlg,IDC_FILE,DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0));
			CheckDlgButton(hdlg,IDC_HISTORY,DBGetContactSettingByte(NULL,S_MOD,"KeepHistory",0));
			CheckDlgButton(hdlg,IDC_IGNOREOFFLINE,DBGetContactSettingByte(NULL,S_MOD,"IgnoreOffline",1));
			CheckDlgButton(hdlg,IDC_MISSEDONES,DBGetContactSettingByte(NULL,S_MOD,"MissedOnes",0));
			CheckDlgButton(hdlg,IDC_SHOWICON,DBGetContactSettingByte(NULL,S_MOD,"ShowIcon",1));
			CheckDlgButton(hdlg,IDC_COUNT,DBGetContactSettingByte(NULL,S_MOD,"MissedOnes_Count",0));

			EnableWindow(GetDlgItem(hdlg,IDC_MENUSTAMP),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
			EnableWindow(GetDlgItem(hdlg,IDC_SHOWICON),IsDlgButtonChecked(hdlg,IDC_MENUITEM));
			EnableWindow(GetDlgItem(hdlg,IDC_USERSTAMP),IsDlgButtonChecked(hdlg,IDC_USERINFO));
			EnableWindow(GetDlgItem(hdlg,IDC_FILESTAMP),IsDlgButtonChecked(hdlg,IDC_FILE));
			EnableWindow(GetDlgItem(hdlg,IDC_FILENAME),IsDlgButtonChecked(hdlg,IDC_FILE));
			EnableWindow(GetDlgItem(hdlg,IDC_HISTORYSIZE),IsDlgButtonChecked(hdlg,IDC_HISTORY));
			EnableWindow(GetDlgItem(hdlg,IDC_HISTORYSTAMP),IsDlgButtonChecked(hdlg,IDC_HISTORY));
			EnableWindow(GetDlgItem(hdlg,IDC_COUNT),IsDlgButtonChecked(hdlg,IDC_MISSEDONES));

			SetDlgItemText(hdlg,IDC_MENUSTAMP,!DBGetContactSetting(NULL,S_MOD,"MenuStamp",&dbv)?dbv.pszVal:DEFAULT_MENUSTAMP);
			DBFreeVariant(&dbv);
			SetDlgItemText(hdlg,IDC_USERSTAMP,!DBGetContactSetting(NULL,S_MOD,"UserStamp",&dbv)?dbv.pszVal:DEFAULT_USERSTAMP);
			DBFreeVariant(&dbv);
			SetDlgItemText(hdlg,IDC_FILESTAMP,!DBGetContactSetting(NULL,S_MOD,"FileStamp",&dbv)?dbv.pszVal:DEFAULT_FILESTAMP);
			DBFreeVariant(&dbv);
			SetDlgItemText(hdlg,IDC_FILENAME,!DBGetContactSetting(NULL,S_MOD,"FileName",&dbv)?dbv.pszVal:DEFAULT_FILENAME);
			DBFreeVariant(&dbv);
			SetDlgItemInt(hdlg,IDC_HISTORYSIZE,DBGetContactSettingWord(NULL,S_MOD,"HistoryMax",10-1)-1,FALSE);
			SetDlgItemText(hdlg,IDC_HISTORYSTAMP,!DBGetContactSetting(NULL,S_MOD,"HistoryStamp",&dbv)?dbv.pszVal:DEFAULT_HISTORYSTAMP);
			DBFreeVariant(&dbv);

			// load protocol list
			SetWindowLong(GetDlgItem(hdlg,IDC_PROTOCOLLIST),GWL_STYLE,GetWindowLong(GetDlgItem(hdlg,IDC_PROTOCOLLIST),GWL_STYLE)|TVS_CHECKBOXES);
			{	
				TVINSERTSTRUCT tvis;
				int numberOfProtocols,i;
				PROTOCOLDESCRIPTOR** ppProtocolDescriptors;

				tvis.hParent=NULL;
				tvis.hInsertAfter=TVI_LAST;
				tvis.item.mask=TVIF_TEXT | TVIF_HANDLE | TVIF_STATE;
				tvis.item.stateMask = TVIS_STATEIMAGEMASK;

				CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&numberOfProtocols,(LPARAM)&ppProtocolDescriptors);
				for (i=0; i<numberOfProtocols; i++) {
					if(ppProtocolDescriptors[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(ppProtocolDescriptors[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) continue;
					tvis.item.pszText=ppProtocolDescriptors[i]->szName;
					tvis.item.state = INDEXTOSTATEIMAGEMASK(IsWatchedProtocol(tvis.item.pszText)+1);
					TreeView_InsertItem(GetDlgItem(hdlg,IDC_PROTOCOLLIST),&tvis);

				}
			}

			break; //case WM_INITDIALOG

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
					case IDC_HISTORY:
						EnableWindow(GetDlgItem(hdlg,IDC_HISTORYSTAMP),IsDlgButtonChecked(hdlg,IDC_HISTORY));
						EnableWindow(GetDlgItem(hdlg,IDC_HISTORYSIZE),IsDlgButtonChecked(hdlg,IDC_HISTORY));
						break;
					case IDC_MISSEDONES:
						EnableWindow(GetDlgItem(hdlg,IDC_COUNT),IsDlgButtonChecked(hdlg,IDC_MISSEDONES));
						break;
				}
			}
			
			if (LOWORD(wparam)==IDC_VARIABLES)
			{
				char szout[2048]="";
				wsprintf(szout,VARIABLE_LIST);
				MessageBox(NULL,szout,"Last Seen Variables",MB_OK|MB_TOPMOST);
			}

			break; //case WM_COMMAND

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->idFrom) 
			{
				case 0: 
					switch (((LPNMHDR)lparam)->code)
					{
						BYTE checkValue;

						case PSN_APPLY:

							GetDlgItemText(hdlg,IDC_MENUSTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"MenuStamp",szstamp);

							GetDlgItemText(hdlg,IDC_USERSTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"UserStamp",szstamp);

							GetDlgItemText(hdlg,IDC_FILESTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"FileStamp",szstamp);

							GetDlgItemText(hdlg,IDC_FILENAME,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"FileName",szstamp);

							GetDlgItemText(hdlg,IDC_HISTORYSTAMP,szstamp,256);
							DBWriteContactSettingString(NULL,S_MOD,"HistoryStamp",szstamp);
							
							DBWriteContactSettingWord(NULL,S_MOD,"HistoryMax",(WORD)(GetDlgItemInt(hdlg,IDC_HISTORYSIZE,NULL,FALSE)+1));

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_MENUITEM);
							if (DBGetContactSettingByte(NULL,S_MOD,"MenuItem",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"MenuItem",checkValue);
								if(hmenuitem==NULL && checkValue) {
									InitMenuitem();
								}
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_USERINFO);
							if (DBGetContactSettingByte(NULL,S_MOD,"UserinfoTab",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"UserinfoTab",checkValue);
								if(checkValue) {
									ehuserinfo=HookEvent(ME_USERINFO_INITIALISE,UserinfoInit);
								} else {
									UnhookEvent(ehuserinfo);
								}
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_FILE);
							if (DBGetContactSettingByte(NULL,S_MOD,"FileOutput",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"FileOutput",checkValue);
								if(checkValue) {
									InitFileOutput();
								}
							}


							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_HISTORY);
							if (DBGetContactSettingByte(NULL,S_MOD,"KeepHistory",0) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"KeepHistory",checkValue);
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_IGNOREOFFLINE);
							if (DBGetContactSettingByte(NULL,S_MOD,"IgnoreOffline",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"IgnoreOffline",checkValue);
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_MISSEDONES);
							if (DBGetContactSettingByte(NULL,S_MOD,"MissedOnes",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"MissedOnes",checkValue);
								if(checkValue) {
									ehmissed_proto=HookEvent(ME_PROTO_ACK,ModeChange_mo);
								} else {
									UnhookEvent(ehmissed_proto);
								}
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_SHOWICON);
							if (DBGetContactSettingByte(NULL,S_MOD,"ShowIcon",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"ShowIcon",checkValue);
							}

							checkValue = (BYTE)IsDlgButtonChecked(hdlg,IDC_COUNT);
							if (DBGetContactSettingByte(NULL,S_MOD,"MissedOnes_Count",1) != checkValue) {
								DBWriteContactSettingByte(NULL,S_MOD,"MissedOnes_Count",checkValue);
							}

							// save protocol list
							{
								HWND hwndTreeView = GetDlgItem(hdlg,IDC_PROTOCOLLIST);
								HTREEITEM hItem;
								TVITEM tvItem;
								char *watchedProtocols, protocol[20]; //TODO: remove magic number
								int size=1;

								watchedProtocols = (char *)malloc(sizeof(char));
								*watchedProtocols = '\0';
								hItem = TreeView_GetRoot(hwndTreeView);
								tvItem.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_STATE;
								tvItem.stateMask = TVIS_STATEIMAGEMASK;
								tvItem.pszText = protocol;
								tvItem.cchTextMax = 20; //magic

								while (hItem != NULL) 
								{
									tvItem.hItem = hItem;
									TreeView_GetItem(hwndTreeView, &tvItem);
									if ((BOOL)(tvItem.state >> 12) -1)
									{
										size = (size + strlen(protocol)+2) * sizeof(char);
										watchedProtocols = (char *)realloc(watchedProtocols, size);
										strcat(watchedProtocols, protocol); 
										strcat(watchedProtocols, " "); 
									}
									hItem = TreeView_GetNextSibling(hwndTreeView, hItem);
								}
								DBWriteContactSettingString(NULL,S_MOD,"WatchedProtocols",watchedProtocols);
								free(watchedProtocols);
							}

							break; //case PSN_APPLY
					}
					break; //case 0

				case IDC_PROTOCOLLIST:
					switch (((LPNMHDR)lparam)->code) 
					{
						case NM_CLICK:
							{
								HWND hTree=((LPNMHDR)lparam)->hwndFrom;
								TVHITTESTINFO hti;
								HTREEITEM hItem;

								hti.pt.x=(short)LOWORD(GetMessagePos());
								hti.pt.y=(short)HIWORD(GetMessagePos());
								ScreenToClient(hTree,&hti.pt);
								if(hItem=TreeView_HitTest(hTree,&hti))
								{
									if (hti.flags & TVHT_ONITEM) 
										TreeView_SelectItem(hTree,hItem);
									if (hti.flags & TVHT_ONITEMSTATEICON) 
										SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
									
								}
							}
							break; 
					}
					break; //case IDC_PROTOCOLLIST
			}
			break;//case WM_NOTIFY
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