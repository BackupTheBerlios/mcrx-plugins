#include "dbwiz.h"
#include "..\..\miranda32\random\skin\m_skin.h"



extern HINSTANCE hinstance;
extern MODULESTRUCT mod_dat;
extern CONTACTSTRUCT con_dat;
extern HANDLE hevent[3];



int InitData(void);
int FillUserList(HWND);
int ClearSettings(HWND);
int SetModules(HWND,HANDLE);
LRESULT CALLBACK ChangeDialogProc(HWND,UINT,WPARAM,LPARAM);
int ReInitUsers(HANDLE);



LRESULT CALLBACK DialogProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	int selection;
	HANDLE hcontact;
	BOOL show;
	HWND hChange,hTree;
	DBVARIANT dbv;
	DBCONTACTWRITESETTING *dbws;
	char szSetting[256],szModule[256],szBuffer[256],szTemp[32];
	HANDOVERSTRUCT chdata;
	TVITEM tvi;

	switch(msg){

		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);
			SendMessage(hdlg,WM_SETICON,ICON_SMALL,(LPARAM)LoadImage(hinstance,MAKEINTRESOURCE(IDI_8BIT),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
			SendMessage(hdlg,WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(hinstance,MAKEINTRESOURCE(IDI_8BIT)));
			SendDlgItemMessage(hdlg,IDC_LOGO,STM_SETICON,(WPARAM)LoadIcon(hinstance,MAKEINTRESOURCE(IsWinVerXPPlus()?IDI_32BIT:IDI_8BIT)),0);

			InitData();

			FillUserList(GetDlgItem(hdlg,IDC_CONTACTLIST));
			CheckRadioButton(hdlg,IDC_DECIMAL,IDC_HEX,IDC_DECIMAL);

			break;

		case WM_COMMAND:

			switch(HIWORD(wparam)){
				
				case LBN_SELCHANGE:
					{
						char szhandle[9]="";
						HANDLE handle;

						if(LOWORD(wparam)!=IDC_CONTACTLIST) break;
						ClearSettings(GetDlgItem(hdlg,IDC_MODULETREE));
						selection=SendDlgItemMessage(hdlg,IDC_CONTACTLIST,LB_GETCURSEL,0,0);
						handle=(HANDLE)ListBox_GetItemData(GetDlgItem(hdlg,IDC_CONTACTLIST),selection);
						SetModules(GetDlgItem(hdlg,IDC_MODULETREE),handle);
						itoa((int)handle,szhandle,BASE);
						SetDlgItemText(hdlg,IDC_HANDLE,szhandle);
						SetWindowLong(GetDlgItem(hdlg,IDC_HANDLE),GWL_USERDATA,(LONG)handle);
						break;
					}

				case BN_CLICKED:
					switch(LOWORD(wparam)){
					
						case IDC_CHANGE:
							selection=SendDlgItemMessage(hdlg,IDC_CONTACTLIST,LB_GETCURSEL,0,0);
							chdata.nContact=selection;
							chdata.hContact=(HANDLE)ListBox_GetItemData(GetDlgItem(hdlg,IDC_CONTACTLIST),selection);

							tvi.mask=TVIF_TEXT|TVIF_PARAM;
							tvi.hItem=TreeView_GetSelection(GetDlgItem(hdlg,IDC_MODULETREE));
							tvi.pszText=szSetting;
							tvi.cchTextMax=sizeof(szSetting);

							TreeView_GetItem(GetDlgItem(hdlg,IDC_MODULETREE),&tvi);

							chdata.pszModule=(const char *)tvi.lParam;
							chdata.pszSetting=szSetting;

							DBGetContactSetting(chdata.hContact,chdata.pszModule,chdata.pszSetting,&chdata.dbv);

							DialogBoxParam(hinstance,MAKEINTRESOURCE(IDD_CHANGE),hdlg,ChangeDialogProc,(LPARAM)&chdata);
							break;

						case IDC_CLOSE:
							SendMessage(hdlg,WM_CLOSE,0,0);
							break;

						case IDC_DECIMAL:
						case IDC_HEX:
							SendMessage(hdlg,UM_BASECHANGE,0,0);
							break;
					}

					break;
			}

			break;

		case WM_NOTIFY:
			switch(wparam)
			{
				case IDC_MODULETREE:
					switch(((LPNMHDR)lparam)->code)
					{
						case TVN_SELCHANGED:
							hTree=GetDlgItem(hdlg,IDC_MODULETREE);
							hChange=GetDlgItem(hdlg,IDC_CHANGE);
							tvi.mask=TVIF_TEXT|TVIF_CHILDREN;
							tvi.hItem=TreeView_GetSelection(hTree);
							tvi.pszText=szSetting;
							tvi.cchTextMax=sizeof(szSetting);
							TreeView_GetItem(hTree,&tvi);
							tvi.hItem=TreeView_GetParent(hTree,tvi.hItem);
							
							if(tvi.cChildren || tvi.hItem==NULL)
							{
								SetDlgItemText(hdlg,IDC_TYPE,"");
								SetDlgItemText(hdlg,IDC_VALUE,"");
								EnableWindow(hChange,FALSE);
								break;
							}

							tvi.pszText=szModule;
							TreeView_GetItem(hTree,&tvi);

							selection=SendDlgItemMessage(hdlg,IDC_CONTACTLIST,LB_GETCURSEL,0,0);
							hcontact=(HANDLE)ListBox_GetItemData(GetDlgItem(hdlg,IDC_CONTACTLIST),selection);

							DBGetContactSetting(hcontact,szModule,szSetting,&dbv);

							switch(dbv.type){
								
								case DBVT_BYTE:
									{
										BYTE bval=DBGetContactSettingByte(hcontact,szModule,szSetting,0);
										itoa((int)bval,szBuffer,BASE);
										SetDlgItemText(hdlg,IDC_TYPE,Translate("byte"));
										Store_Val(bval);
										EnableWindow(hChange,TRUE);
										break;
									}
								case DBVT_WORD:
									{
										int ival=DBGetContactSettingWord(hcontact,szModule,szSetting,0);
										itoa(ival,szBuffer,BASE);
										SetDlgItemText(hdlg,IDC_TYPE,Translate("word"));
										Store_Val(ival);
										EnableWindow(hChange,TRUE);
										break;
									}
								case DBVT_DWORD:
									{
										unsigned long ulval=DBGetContactSettingDword(hcontact,szModule,szSetting,0);
										ltoa(ulval,szBuffer,BASE);
										SetDlgItemText(hdlg,IDC_TYPE,Translate("double word"));
										Store_Val(ulval);
										EnableWindow(hChange,TRUE);
										break;
									}
								case DBVT_ASCIIZ:
									show=TRUE;
									if(!lstrcmp(szSetting,"Password") || !lstrcmp(szSetting,"ProxyPassword"))
									{
										show=FALSE;
										lstrcpyn(szBuffer,Translate("(password)"),sizeof szBuffer);
									}
										
									else if(!lstrcmp(dbv.pszVal,""))
										lstrcpyn(szBuffer,Translate("(empty)"),sizeof szBuffer);
									
									else
										lstrcpyn(szBuffer,dbv.pszVal,sizeof szBuffer);

									SetDlgItemText(hdlg,IDC_TYPE,Translate("string"));
									EnableWindow(hChange,show);
									break;
									Store_Val(0);
								case DBVT_BLOB:
									lstrcpyn(szBuffer,Translate("size of blob:"),sizeof szBuffer);
									wsprintf(szTemp,"\t%i ",dbv.cpbVal);
									lstrcat(szBuffer,szTemp);
									lstrcat(szBuffer,Translate("bytes"));
									lstrcat(szBuffer,"\x0D\x0A");

									lstrcat(szBuffer,Translate("adress of blob:"));
									wsprintf(szTemp,"\t%p\x0D\x0A",dbv.pbVal);
									lstrcat(szBuffer,szTemp);

									SetDlgItemText(hdlg,IDC_TYPE,Translate("blob"));
									EnableWindow(hChange,FALSE);
									Store_Val(0);
									break;
							}

							SetDlgItemText(hdlg,IDC_VALUE,szBuffer);
							break;
					}

					break;
			}

			break;

		case UM_REINIT:
			SendMessage(hdlg,WM_COMMAND,MAKEWPARAM(IDC_CONTACTLIST,LBN_SELCHANGE),0);
			break;

		case UM_CLISTREBUILD:
			ReInitUsers((HANDLE)wparam);
			SendDlgItemMessage(hdlg,IDC_CONTACTLIST,LB_RESETCONTENT,0,0);
			FillUserList(GetDlgItem(hdlg,IDC_CONTACTLIST));
			ClearSettings(GetDlgItem(hdlg,IDC_MODULETREE));
			break;

		case UM_CONTACTRENAME:
			dbws=(DBCONTACTWRITESETTING *)lparam;
			if(lstrcmp(dbws->szSetting,"MyHandle") && lstrcmp(dbws->szSetting,"Nick")) break;

			SendMessage(hdlg,UM_CLISTREBUILD,(WPARAM)NULL,0);
			break;

		case UM_BASECHANGE:
			{
				char string[32]="";
				unsigned long val=GetWindowLong(GetDlgItem(hdlg,IDC_HANDLE),GWL_USERDATA);

				itoa((int)val,string,BASE);
				SetDlgItemText(hdlg,IDC_HANDLE,string);

				if(val=GetWindowLong(GetDlgItem(hdlg,IDC_VALUE),GWL_USERDATA))
				{
					itoa((int)val,string,BASE);
					SetDlgItemText(hdlg,IDC_VALUE,string);
				}
				break;
			}

		case WM_CLOSE:
			EndDialog(hdlg,0);
			mod_dat.modules=0;
			con_dat.contacts=0;
			break;
	}

	return 0;
}



int StartDialog(WPARAM wparam,LPARAM lparam)
{
	HWND hdlg;

	hdlg=CreateDialogParam(hinstance,MAKEINTRESOURCE(IDD_MAIN),NULL,DialogProc,(LPARAM)0);

	hevent[0]=HookEventMessage(ME_DB_CONTACT_ADDED,hdlg,UM_CLISTREBUILD);
	hevent[1]=HookEventMessage(ME_DB_CONTACT_DELETED,hdlg,UM_CLISTREBUILD);
	hevent[2]=HookEventMessage(ME_DB_CONTACT_SETTINGCHANGED,hdlg,UM_CONTACTRENAME);

	return 0;
}