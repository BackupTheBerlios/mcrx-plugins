#include "msgpopup.h"
#include "..\miranda\include\m_options.h"
#include "..\miranda\include\m_utils.h"


// main.c
extern HINSTANCE hinstance;


// windows.c
int WindowRequest(HANDLE,UINT,int,HANDLE);


// fill the combo boxes on the appearance-soptions page
// with the available settings
void FillCombos(HWND hdlg)
{
	int nPos;
	DBVARIANT dbv;

	nPos=ListBox_AddString(hdlg,IDC_STYLE,Translate("Flat"));
	ListBox_SetItemData(hdlg,IDC_STYLE,nPos,STYLE_FLAT);
	nPos=ListBox_AddString(hdlg,IDC_STYLE,Translate("Beveled"));
	ListBox_SetItemData(hdlg,IDC_STYLE,nPos,STYLE_3D);
	nPos=ListBox_AddString(hdlg,IDC_STYLE,Translate("Minimal"));
	ListBox_SetItemData(hdlg,IDC_STYLE,nPos,STYLE_MINIMAL);

	nPos=ListBox_AddString(hdlg,IDC_POSITION,Translate("Upper left corner"));
	ListBox_SetItemData(hdlg,IDC_POSITION,nPos,POS_TOPLEFT);
	nPos=ListBox_AddString(hdlg,IDC_POSITION,Translate("Upper right corner"));
	ListBox_SetItemData(hdlg,IDC_POSITION,nPos,POS_TOPRIGHT);
	nPos=ListBox_AddString(hdlg,IDC_POSITION,Translate("Centered"));
	ListBox_SetItemData(hdlg,IDC_POSITION,nPos,POS_CENTER);
	nPos=ListBox_AddString(hdlg,IDC_POSITION,Translate("Lower left corner"));
	ListBox_SetItemData(hdlg,IDC_POSITION,nPos,POS_BOTTOMLEFT);
	nPos=ListBox_AddString(hdlg,IDC_POSITION,Translate("Lower right corner"));
	ListBox_SetItemData(hdlg,IDC_POSITION,nPos,POS_BOTTOMRIGHT);

	nPos=ListBox_AddString(hdlg,IDC_SPREADING,Translate("Vertical"));
	ListBox_SetItemData(hdlg,IDC_SPREADING,nPos,SPREAD_VERT);
	nPos=ListBox_AddString(hdlg,IDC_SPREADING,Translate("Horizontal"));
	ListBox_SetItemData(hdlg,IDC_SPREADING,nPos,SPREAD_HORZ);

	nPos=ListBox_AddString(hdlg,IDC_BKSTYLE,Translate("Solid"));
	ListBox_SetItemData(hdlg,IDC_BKSTYLE,nPos,BKSTYLE_SOLID);
	// transparency only if windows-version>=2000
	if(LOBYTE(LOWORD(GetVersion()))>=5)
	{
		nPos=ListBox_AddString(hdlg,IDC_BKSTYLE,Translate("Transparent"));
		ListBox_SetItemData(hdlg,IDC_BKSTYLE,nPos,BKSTYLE_TRANSPARENT);
	}

	dbv.cpbVal=sizeof(FLAGSTRUCT);
	dbv.pbVal=(PBYTE)malloc(dbv.cpbVal);
	
	if(!DBGetContactSetting(NULL,MODULE,"Flags",&dbv))
	{
		ListBox_SetSel(hdlg,IDC_STYLE,((PFLAGS)dbv.pbVal)->bStyle-1);
		ListBox_SetSel(hdlg,IDC_POSITION,((PFLAGS)dbv.pbVal)->bPos-1);
		ListBox_SetSel(hdlg,IDC_SPREADING,((PFLAGS)dbv.pbVal)->bSpreading-1);
		ListBox_SetSel(hdlg,IDC_BKSTYLE,((PFLAGS)dbv.pbVal)->bBKStyle-1);
	}

	else
	{
		ListBox_SetSel(hdlg,IDC_STYLE,STYLE_FLAT-1);
		ListBox_SetSel(hdlg,IDC_POSITION,POS_CENTER-1);
		ListBox_SetSel(hdlg,IDC_SPREADING,SPREAD_VERT-1);
		ListBox_SetSel(hdlg,IDC_BKSTYLE,BKSTYLE_SOLID-1);
	}

	DBFreeVariant(&dbv);

}


// enables/disables all transparency related controls
void EnableTrans(BOOL bShow,HWND hdlg)
{
	EnableWindow(GetDlgItem(hdlg,IDC_TGROUP),bShow);
	EnableWindow(GetDlgItem(hdlg,IDC_TRANSPARENCY),(bShow && !IsDlgButtonChecked(hdlg,IDC_SYNC)));
	EnableWindow(GetDlgItem(hdlg,IDC_PERCENT),(bShow && !IsDlgButtonChecked(hdlg,IDC_SYNC)));
	EnableWindow(GetDlgItem(hdlg,IDC_SYNC),bShow);
}


// check the boxes specified in the db
WORD CheckAllBoxes(HWND hdlg)
{
	WORD wmode=DBGetContactSettingWord(NULL,MODULE,"ModeFlag",NOTIFY_MSG|NOTIFY_URL|NOTIFY_FILE|DISMISS_AWAY|DISMISS_NA|DISMISS_DND);

	CheckDlgButton(hdlg,IDC_MESSAGE,wmode&NOTIFY_MSG);
	SetWindowLong(GetDlgItem(hdlg,IDC_MESSAGE),GWL_USERDATA,(LONG)NOTIFY_MSG);
	CheckDlgButton(hdlg,IDC_URL,wmode&NOTIFY_URL);
	SetWindowLong(GetDlgItem(hdlg,IDC_URL),GWL_USERDATA,(LONG)NOTIFY_URL);
	CheckDlgButton(hdlg,IDC_FILE,wmode&NOTIFY_FILE);
	SetWindowLong(GetDlgItem(hdlg,IDC_FILE),GWL_USERDATA,(LONG)NOTIFY_FILE);
	CheckDlgButton(hdlg,IDC_AUTH,wmode&NOTIFY_AUTH);
	SetWindowLong(GetDlgItem(hdlg,IDC_AUTH),GWL_USERDATA,(LONG)NOTIFY_AUTH);
	CheckDlgButton(hdlg,IDC_ADDED,wmode&NOTIFY_ADDED);
	SetWindowLong(GetDlgItem(hdlg,IDC_ADDED),GWL_USERDATA,(LONG)NOTIFY_ADDED);
	CheckDlgButton(hdlg,IDC_ALWAYS,wmode&NOTIFY_ALWAYS);
	SetWindowLong(GetDlgItem(hdlg,IDC_ALWAYS),GWL_USERDATA,(LONG)NOTIFY_ALWAYS);

	SetDlgItemText(hdlg,IDC_ONLINE,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_ONLINE,0));
	CheckDlgButton(hdlg,IDC_ONLINE,wmode&DISMISS_ONLINE);
	SetWindowLong(GetDlgItem(hdlg,IDC_ONLINE),GWL_USERDATA,(LONG)DISMISS_ONLINE);

	SetDlgItemText(hdlg,IDC_AWAY,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_AWAY,0));
	CheckDlgButton(hdlg,IDC_AWAY,wmode&DISMISS_AWAY);
	SetWindowLong(GetDlgItem(hdlg,IDC_AWAY),GWL_USERDATA,(LONG)DISMISS_AWAY);

	SetDlgItemText(hdlg,IDC_NA,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_NA,0));
	CheckDlgButton(hdlg,IDC_NA,wmode&DISMISS_NA);
	SetWindowLong(GetDlgItem(hdlg,IDC_NA),GWL_USERDATA,(LONG)DISMISS_NA);

	SetDlgItemText(hdlg,IDC_OCCUPIED,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_OCCUPIED,0));
	CheckDlgButton(hdlg,IDC_OCCUPIED,wmode&DISMISS_OCCUPIED);
	SetWindowLong(GetDlgItem(hdlg,IDC_OCCUPIED),GWL_USERDATA,(LONG)DISMISS_OCCUPIED);

	SetDlgItemText(hdlg,IDC_DND,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_DND,0));
	CheckDlgButton(hdlg,IDC_DND,wmode&DISMISS_DND);
	SetWindowLong(GetDlgItem(hdlg,IDC_DND),GWL_USERDATA,(LONG)DISMISS_DND);

	SetDlgItemText(hdlg,IDC_FREE,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_FREECHAT,0));
	CheckDlgButton(hdlg,IDC_FREE,wmode&DISMISS_FREE);
	SetWindowLong(GetDlgItem(hdlg,IDC_FREE),GWL_USERDATA,(LONG)DISMISS_FREE);

	SetDlgItemText(hdlg,IDC_INVIS,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_INVISIBLE,0));
	CheckDlgButton(hdlg,IDC_INVIS,wmode&DISMISS_INVIS);
	SetWindowLong(GetDlgItem(hdlg,IDC_INVIS),GWL_USERDATA,(LONG)DISMISS_INVIS);

	SetDlgItemText(hdlg,IDC_LUNCH,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_OUTTOLUNCH,0));
	CheckDlgButton(hdlg,IDC_LUNCH,wmode&DISMISS_LUNCH);
	SetWindowLong(GetDlgItem(hdlg,IDC_LUNCH),GWL_USERDATA,(LONG)DISMISS_LUNCH);

	SetDlgItemText(hdlg,IDC_PHONE,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,ID_STATUS_ONTHEPHONE,0));
	CheckDlgButton(hdlg,IDC_PHONE,wmode&DISMISS_PHONE);
	SetWindowLong(GetDlgItem(hdlg,IDC_PHONE),GWL_USERDATA,(LONG)DISMISS_PHONE);

	return wmode;
}


// DlgProc for Notifying options-page
LRESULT CALLBACK GeneralDialogProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static WORD wmode;

	switch(msg){
		
		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);
			wmode=CheckAllBoxes(hdlg);
			break;

		case WM_COMMAND:
			SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			wmode^=(WORD)GetWindowLong((HWND)lparam,GWL_USERDATA);
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->idFrom){
				
				case 0:
					switch(((LPNMHDR)lparam)->code){
						
						case PSN_APPLY:
							DBWriteContactSettingWord(NULL,MODULE,"ModeFlag",wmode);
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


// DlgProc for Appearance options-page
LRESULT CALLBACK AppearanceDialogProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static HBRUSH hbrush;
	CHOOSEFONT cf={0};
	LOGFONT lf={0};
	static COLORREF fgcol,bkcol;
	DBVARIANT dbv;

	switch(msg){

		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);

			FillCombos(hdlg);

			fgcol=DBGetContactSettingDword(NULL,MODULE,"FGCol",GetSysColor(COLOR_WINDOWTEXT));
			bkcol=DBGetContactSettingDword(NULL,MODULE,"BKCol",GetSysColor(COLOR_BTNFACE));

			SendDlgItemMessage(hdlg,IDC_BKCOLOR,CPM_SETCOLOUR,0,(LPARAM)bkcol);
			SendDlgItemMessage(hdlg,IDC_FGCOLOR,CPM_SETCOLOUR,0,(LPARAM)fgcol);

			SendDlgItemMessage(hdlg,IDC_TRANSPARENCY,TBM_SETRANGE,(WPARAM)TRUE,MAKELONG(0,255));
			SetWindowLong(GetDlgItem(hdlg,IDC_TRANSPARENCY),GWL_USERDATA,(LONG)DBGetContactSettingByte(NULL,MODULE,"TransVal",128));

			hbrush=CreateSolidBrush(bkcol);
			
			if(!DBGetContactSetting(NULL,MODULE,"Font",&dbv))
				lf=*(LOGFONT*)dbv.pbVal;

			else
				GetObject((HFONT)SendDlgItemMessage(hdlg,IDC_SAMPLE,WM_GETFONT,0,0),sizeof(lf),&lf);

			SendDlgItemMessage(hdlg,IDC_SAMPLE,WM_SETFONT,(WPARAM)CreateFontIndirect(&lf),MAKELPARAM(TRUE,0));
			DBFreeVariant(&dbv);

			SetDlgItemInt(hdlg,IDC_TIMEOUT,DBGetContactSettingByte(NULL,MODULE,"Time",5),FALSE);

			CheckDlgButton(hdlg,IDC_SYNC,DBGetContactSettingByte(NULL,MODULE,"Sync",0)?BST_CHECKED:BST_UNCHECKED);
			SendMessage(hdlg,WM_COMMAND,MAKEWPARAM(IDC_SYNC,BN_CLICKED),(LPARAM)0x884422);

			EnableTrans(ListBox_GetItemData(hdlg,IDC_BKSTYLE,ListBox_GetSel(hdlg,IDC_BKSTYLE))==BKSTYLE_TRANSPARENT,hdlg);
			break;

		case WM_HSCROLL:
			{
			char szPos[5]="";
			wsprintf(szPos,"%d%%",(BYTE)(SendDlgItemMessage(hdlg,IDC_TRANSPARENCY,TBM_GETPOS,0,0)*100/255));
			SetDlgItemText(hdlg,IDC_PERCENT,szPos);
			}

			if(lparam!=0x884422)
			{
				SetWindowLong(GetDlgItem(hdlg,IDC_TRANSPARENCY),GWL_USERDATA,(LONG)SendDlgItemMessage(hdlg,IDC_TRANSPARENCY,TBM_GETPOS,0,0));
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			}
			break;

		case WM_CTLCOLORSTATIC:
			if((HWND)lparam!=GetDlgItem(hdlg,IDC_SAMPLE))
				return 0;
			SetBkColor((HDC)wparam,bkcol);
			SetTextColor((HDC)wparam,fgcol);
			return (BOOL)hbrush;

		case WM_COMMAND:
			switch(LOWORD(wparam)){
				
				case IDC_BKCOLOR:
					DeleteObject(hbrush);
					bkcol=SendDlgItemMessage(hdlg,IDC_BKCOLOR,CPM_GETCOLOUR,0,0);
					hbrush=CreateSolidBrush(bkcol);
					InvalidateRect(GetDlgItem(hdlg,IDC_SAMPLE),NULL,TRUE);
					SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
					break;

				case IDC_FGCOLOR:
					fgcol=SendDlgItemMessage(hdlg,IDC_FGCOLOR,CPM_GETCOLOUR,0,0);
					InvalidateRect(GetDlgItem(hdlg,IDC_SAMPLE),NULL,TRUE);
					SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
					break;

				case IDC_FONT:
					cf.hwndOwner=hdlg;
					cf.lpLogFont=&lf;
					cf.lStructSize=sizeof(cf);
					cf.nSizeMax=10;
					cf.nSizeMin=6;
					cf.Flags=CF_FORCEFONTEXIST|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_LIMITSIZE;
					GetObject((HFONT)SendDlgItemMessage(hdlg,IDC_SAMPLE,WM_GETFONT,0,0),sizeof(lf),&lf);
					
					if(!ChooseFont(&cf))
						break;

					SendDlgItemMessage(hdlg,IDC_SAMPLE,WM_SETFONT,(WPARAM)CreateFontIndirect(&lf),MAKELPARAM(TRUE,0));
					SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
					break;

				case IDC_BKSTYLE:
					if(HIWORD(wparam)!=LBN_SELCHANGE) break;
					if(ListBox_GetItemData(hdlg,IDC_BKSTYLE,ListBox_GetSel(hdlg,IDC_BKSTYLE))==BKSTYLE_TRANSPARENT)
						EnableTrans(TRUE,hdlg);
					else
						EnableTrans(FALSE,hdlg);

					SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
					break;

				case IDC_SYNC:
					if(IsDlgButtonChecked(hdlg,IDC_SYNC))
						SendDlgItemMessage(hdlg,IDC_TRANSPARENCY,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)DBGetContactSettingByte(NULL,"CList","Alpha",255));

					else
						SendDlgItemMessage(hdlg,IDC_TRANSPARENCY,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)GetWindowLong(GetDlgItem(hdlg,IDC_TRANSPARENCY),GWL_USERDATA));

					EnableTrans(TRUE,hdlg);

					SendMessage(hdlg,WM_HSCROLL,0,lparam);
					if(lparam!=0x884422)
						SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
					break;

				case IDC_TIMEOUT:
					if(HIWORD(wparam)!=EN_CHANGE || (HWND)lparam!=GetFocus()) break;
					
					SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
					break;

				case IDC_STYLE:
				case IDC_POSITION:
				case IDC_SPREADING:
					if((HWND)lparam!=GetFocus()) break;

					SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
					break;

				case IDC_PREVIEW:
					WindowRequest(NULL,PREVIEW_EVENT,SKINICON_EVENT_URL,NULL);
					break;

			}
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lparam)->idFrom){
				
				case 0:
					switch(((LPNMHDR)lparam)->code){
						
						case PSN_APPLY:
							{
							PFLAGS pflags=(PFLAGS)malloc(sizeof(FLAGSTRUCT));
							DBCONTACTWRITESETTING cws;

							GetObject((HFONT)SendDlgItemMessage(hdlg,IDC_SAMPLE,WM_GETFONT,0,0),sizeof(lf),&lf);
							cws.szModule=MODULE;
							cws.szSetting="Font";
							cws.value.type=DBVT_BLOB;
							cws.value.cpbVal=sizeof(lf);
							cws.value.pbVal=(PBYTE)&lf;
							CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)NULL,(LPARAM)&cws);

							pflags->bStyle=(BYTE)ListBox_GetItemData(hdlg,IDC_STYLE,ListBox_GetSel(hdlg,IDC_STYLE));
							pflags->bPos=(BYTE)ListBox_GetItemData(hdlg,IDC_POSITION,ListBox_GetSel(hdlg,IDC_POSITION));
							pflags->bSpreading=(BYTE)ListBox_GetItemData(hdlg,IDC_SPREADING,ListBox_GetSel(hdlg,IDC_SPREADING));
							pflags->bBKStyle=(BYTE)ListBox_GetItemData(hdlg,IDC_BKSTYLE,ListBox_GetSel(hdlg,IDC_BKSTYLE));

							cws.szSetting="Flags";
							cws.value.cpbVal=sizeof(FLAGSTRUCT);
							cws.value.pbVal=(PBYTE)pflags;
							CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)NULL,(LPARAM)&cws);

							DBWriteContactSettingDword(NULL,MODULE,"FGCol",SendDlgItemMessage(hdlg,IDC_FGCOLOR,CPM_GETCOLOUR,0,0));
							DBWriteContactSettingDword(NULL,MODULE,"BKCol",SendDlgItemMessage(hdlg,IDC_BKCOLOR,CPM_GETCOLOUR,0,0));

							DBWriteContactSettingByte(NULL,MODULE,"Time",(BYTE)GetDlgItemInt(hdlg,IDC_TIMEOUT,NULL,FALSE));

							if(IsDlgButtonChecked(hdlg,IDC_SYNC))
								DBWriteContactSettingByte(NULL,MODULE,"Sync",1);
							else
							{
								DBWriteContactSettingByte(NULL,MODULE,"TransVal",(BYTE)SendDlgItemMessage(hdlg,IDC_TRANSPARENCY,TBM_GETPOS,0,0));
								DBWriteContactSettingByte(NULL,MODULE,"Sync",0);
							}

							free(pflags);

							}
							break;
					}
					break;
			}
			break;

		case WM_CLOSE:
			DeleteObject(hbrush);
			break;
	}

	return 0;
}


// add the two options pages
int InitOptions(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE odp={0};

	odp.cbSize=sizeof(odp);
	odp.flags=ODPF_BOLDGROUPS;
	odp.hInstance=hinstance;
	odp.position=-1;
	odp.pszGroup=Translate("Events");

	odp.pfnDlgProc=AppearanceDialogProc;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_APPEARANCE);
	odp.pszTitle=Translate("Popup appearance");
	CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);

	odp.pfnDlgProc=GeneralDialogProc;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_MODES);
	odp.pszTitle=Translate("Notifying");
	CallService(MS_OPT_ADDPAGE,wparam,(LPARAM)&odp);

	return 0;
}