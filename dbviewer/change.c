#include "dbwiz.h"
#include "..\SDK\headers_c\m_clist.h"




LRESULT CALLBACK ChangeDialogProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HANDOVERSTRUCT *phdat;
	char szVal[256];
	BYTE bVal;
	WORD wVal;
	DWORD dwVal;

	switch(msg){

		case WM_INITDIALOG:
			phdat=(HANDOVERSTRUCT*)lparam;
			SetWindowLong(hdlg,GWL_USERDATA,(LONG)phdat);

			TranslateDialogDefault(hdlg);

			SetDlgItemText(hdlg,IDC_USER,(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)phdat->hContact,0));
			SetDlgItemInt(hdlg,IDC_HANDLE,(UINT)phdat->hContact,FALSE);
			SetDlgItemText(hdlg,IDC_MODULE,phdat->pszModule);
			SetDlgItemText(hdlg,IDC_SETTING,phdat->pszSetting);

			switch(phdat->dbv.type){

				case DBVT_BYTE:
					SetDlgItemInt(hdlg,IDC_CURRENT,(UINT)DBGetContactSettingByte(phdat->hContact,phdat->pszModule,phdat->pszSetting,0),FALSE);
					SetDlgItemText(hdlg,IDC_TYPE,Translate("byte"));
					break;
				
				case DBVT_WORD:
					SetDlgItemInt(hdlg,IDC_CURRENT,(UINT)DBGetContactSettingWord(phdat->hContact,phdat->pszModule,phdat->pszSetting,0),TRUE);
					SetDlgItemText(hdlg,IDC_TYPE,Translate("word"));
					break;

				case DBVT_DWORD:
					SetDlgItemInt(hdlg,IDC_CURRENT,(UINT)DBGetContactSettingDword(phdat->hContact,phdat->pszModule,phdat->pszSetting,0),TRUE);
					SetDlgItemText(hdlg,IDC_TYPE,Translate("double word"));
					break;

				case DBVT_ASCIIZ:
					if(!lstrcmp(phdat->dbv.pszVal,""))
						SetDlgItemText(hdlg,IDC_CURRENT,Translate("(empty)"));
					else
						SetDlgItemText(hdlg,IDC_CURRENT,phdat->dbv.pszVal);

					SetDlgItemText(hdlg,IDC_TYPE,Translate("string"));

					SetWindowLong(GetDlgItem(hdlg,IDC_NEW),GWL_STYLE,(LONG)(GetWindowLong(GetDlgItem(hdlg,IDC_NEW),GWL_STYLE)^ES_NUMBER));
					break;
			}


			break;

		case WM_COMMAND:
			if(HIWORD(wparam)==BN_CLICKED)
			{
				switch(LOWORD(wparam)){

					case IDC_CANCEL:
						SendMessage(hdlg,WM_CLOSE,0,0);
						break;

					case IDC_APPLY:
						GetDlgItemText(hdlg,IDC_NEW,szVal,256);
						if(!lstrcmp(szVal,"")) break;

						phdat=(HANDOVERSTRUCT *)GetWindowLong(hdlg,GWL_USERDATA);

						switch(phdat->dbv.type){

							case DBVT_BYTE:
								bVal=(BYTE)GetDlgItemInt(hdlg,IDC_NEW,NULL,FALSE);
								DBWriteContactSettingByte(phdat->hContact,phdat->pszModule,phdat->pszSetting,bVal);
								break;

							case DBVT_WORD:
								wVal=(WORD)GetDlgItemInt(hdlg,IDC_NEW,NULL,TRUE);
								DBWriteContactSettingWord(phdat->hContact,phdat->pszModule,phdat->pszSetting,wVal);
								break;

							case DBVT_DWORD:
								dwVal=(DWORD)GetDlgItemInt(hdlg,IDC_NEW,NULL,TRUE);
								DBWriteContactSettingDword(phdat->hContact,phdat->pszModule,phdat->pszSetting,dwVal);
								break;

							case DBVT_ASCIIZ:
								GetDlgItemText(hdlg,IDC_NEW,szVal,255);
								DBWriteContactSettingString(phdat->hContact,phdat->pszModule,phdat->pszSetting,szVal);
								break;

						}

						SendMessage(hdlg,WM_CLOSE,0,0);
						SendMessage(GetParent(hdlg),UM_REINIT,0,0);
						SendDlgItemMessage(GetParent(hdlg),IDC_CONTACTLIST,LB_SETCURSEL,(WPARAM)phdat->nContact,0);
						break;

					case IDC_DELETE:
						phdat=(HANDOVERSTRUCT *)GetWindowLong(hdlg,GWL_USERDATA);

						if(MessageBox(hdlg,Translate("Are you sure you want to delete this setting?"),Translate("Database viewer"),MB_YESNO|MB_APPLMODAL|MB_ICONWARNING)==IDNO)
							break;

						DBDeleteContactSetting(phdat->hContact,phdat->pszModule,phdat->pszSetting);
						SendMessage(hdlg,WM_CLOSE,0,0);
						SendMessage(GetParent(hdlg),UM_REINIT,0,0);
						SendDlgItemMessage(GetParent(hdlg),IDC_CONTACTLIST,LB_SETCURSEL,(WPARAM)phdat->nContact,0);
						break;

				}
				break;
			}
			break;

		case WM_CLOSE:
			EndDialog(hdlg,0);
			break;

	}

	return 0;
}
