#include "seen.h"


extern HINSTANCE hinstance;

static HANDLE hWindowList;

char* BuildSetting(historyLast) {
	static char setting[15];
	static char sztemp[15];
	*setting = '\0';
	strcat(setting, "History_");
	strcat(setting, itoa(historyLast, sztemp, 10));
	return setting;
}

void HistoryWrite(HANDLE hContact)
{
	short historyFirst, historyLast, historyMax;
	DBVARIANT dbv;

	historyMax = DBGetContactSettingWord(NULL,S_MOD,"HistoryMax",10);
	if (historyMax < 0) historyMax; else if (historyMax > 99) historyMax = 99;
	if (historyMax == 0) return;
	historyFirst = DBGetContactSettingWord(hContact,S_MOD,"HistoryFirst",0);
	if (historyFirst >=  historyMax) historyFirst = 0;
	historyLast = DBGetContactSettingWord(hContact,S_MOD,"HistoryLast",0);
	if (historyLast >= historyMax) historyLast = historyMax-1;

	DBWriteContactSettingString(hContact,S_MOD,BuildSetting(historyLast),
			ParseString(!DBGetContactSetting(NULL,S_MOD,"HistoryStamp",&dbv)?dbv.pszVal:DEFAULT_HISTORYSTAMP,hContact,0));
	DBFreeVariant(&dbv);

	historyLast = (historyLast+1) % historyMax;
	DBWriteContactSettingWord(hContact,S_MOD,"HistoryLast",historyLast);
	if (historyLast == historyFirst) {
		DBWriteContactSettingWord(hContact,S_MOD,"HistoryFirst",(short) ((historyFirst+1) % historyMax));
	}

}

void LoadHistoryList(HANDLE hContact, HWND hwnd, int nList) {
	short historyFirst, historyLast, historyMax;
	short i;
	DBVARIANT dbv;


	SendDlgItemMessage(hwnd, nList, LB_RESETCONTENT, 0, 0);
	historyMax = DBGetContactSettingWord(NULL,S_MOD,"HistoryMax",10);
	if (historyMax < 0) historyMax = 0; else if (historyMax > 99) historyMax = 99;
	if (historyMax == 0) return;
	historyFirst = DBGetContactSettingWord(hContact,S_MOD,"HistoryFirst",0);
	if (historyFirst >=  historyMax) historyFirst = 0;
	historyLast = DBGetContactSettingWord(hContact,S_MOD,"HistoryLast",0);
	if (historyLast >= historyMax) historyLast = historyMax-1;
	
	i = historyLast;
	while (i != historyFirst) {
		i = (i-1+historyMax) % historyMax;
		SendDlgItemMessage(hwnd, nList, LB_ADDSTRING, 0, 
				(LPARAM)(!DBGetContactSetting(hContact,S_MOD,BuildSetting(i),&dbv)?dbv.pszVal:""));
		DBFreeVariant(&dbv);
	}

}


HDWP MyResizeWindow (HDWP hDwp, HWND hwndDlg, HWND hwndControl,
				 int nHorizontalOffset, int nVerticalOffset, 
				 int nWidthOffset, int nHeightOffset)
{
	POINT pt;
	RECT rcinit;

	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	
	// get current top left point
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);

	// resize control
/*	MoveWindow(hwndControl, 
			pt.x + nHorizontalOffset, 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left + nWidthOffset, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			FALSE);
*/
	return DeferWindowPos(hDwp, hwndControl, NULL,
			pt.x + nHorizontalOffset, 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left + nWidthOffset, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			SWP_NOZORDER);


}

HDWP MyHorizCenterWindow (HDWP hDwp, HWND hwndDlg, HWND hwndControl,
				 int nClientWidth, int nVerticalOffset, 
				 int nHeightOffset)
{
	POINT pt;
	RECT rcinit;

	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	
	// get current top left point
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);

	// resize control
/*	MoveWindow(hwndControl, 
			(int) ((nClientWidth - (rcinit.right - rcinit.left))/2), 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			TRUE);
*/
	return DeferWindowPos(hDwp, hwndControl, NULL,
			(int) ((nClientWidth - (rcinit.right - rcinit.left))/2), 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			SWP_NOZORDER);

}
void MyResizeGetOffset (HWND hwndDlg, HWND hwndControl, 
				 int nWidth, int nHeight,
				 int* nDx, int* nDy)
{
	RECT rcinit;

	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	
	// calculate offsets
	*nDx = nWidth - (rcinit.right - rcinit.left);
	*nDy = nHeight - (rcinit.bottom - rcinit.top);
}

BOOL CALLBACK HistoryDlgProc(HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam)
{
	HANDLE hContact;
	char sztemp[1024]="";
	
	switch(Message)
    {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwnd);
			hContact = (HANDLE)lparam;
			SetWindowLong(hwnd,GWL_USERDATA,lparam);
			strcpy(sztemp,(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
			strcat(sztemp, " - ");
			strcat(sztemp, Translate("last seen history"));
			SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)sztemp);
			SendMessage(hwnd, WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM) LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

//			LoadHistoryList(hContact, hwnd, IDC_HISTORYLIST);

			if (DBGetContactSettingByte(hContact,S_MOD,"OnlineAlert",0))
				SendDlgItemMessage(hwnd, IDC_STATUSCHANGE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			
			SendDlgItemMessage(hwnd,IDC_DETAILS,BM_SETIMAGE,IMAGE_ICON,
					(LPARAM)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_USERDETAILS),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0));
			SendDlgItemMessage(hwnd,IDC_SENDMSG,BM_SETIMAGE,IMAGE_ICON,
					(LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
			{	
				LOGFONT lf;
				HFONT hFont;
				hFont=(HFONT)SendDlgItemMessage(hwnd,IDC_USERMENU,WM_GETFONT,0,0);
				GetObject(hFont,sizeof(lf),&lf);
				hFont=CreateFont(lf.lfHeight*3/2,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Marlett");
				SendDlgItemMessage(hwnd,IDC_USERMENU,WM_SETFONT,(WPARAM)hFont,0);
			}
			Utils_RestoreWindowPositionNoMove(hwnd,NULL,S_MOD,"History_");
			ShowWindow(hwnd, SW_SHOW);
			break;

		case WM_MEASUREITEM:
			return CallService(MS_CLIST_MENUMEASUREITEM,wparam,lparam);
		case WM_DRAWITEM:
			return CallService(MS_CLIST_MENUDRAWITEM,wparam,lparam);
        case WM_COMMAND:
			hContact=(HANDLE)GetWindowLong(hwnd,GWL_USERDATA);
			if(CallService(MS_CLIST_MENUPROCESSCOMMAND,MAKEWPARAM(LOWORD(wparam),MPCF_CONTACTMENU),(LPARAM)hContact))
				break;
            switch(LOWORD(wparam))
            {
				case IDCANCEL:
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				case IDOK:
					if (SendDlgItemMessage(hwnd, IDC_STATUSCHANGE, BM_GETCHECK, 0, 0) == BST_CHECKED)
						DBWriteContactSettingByte(hContact,S_MOD,"OnlineAlert",1);
					else
						DBWriteContactSettingByte(hContact,S_MOD,"OnlineAlert",0);
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				case IDC_USERMENU:
					{	
						RECT rc;
						HMENU hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,(WPARAM)hContact,0);
						GetWindowRect(GetDlgItem(hwnd,IDC_USERMENU),&rc);
						TrackPopupMenu(hMenu,0,rc.left,rc.bottom,0,hwnd,NULL);
						DestroyMenu(hMenu);
					}
					break;
				case IDC_DETAILS:
					CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)hContact,0);
					break;
				case IDC_SENDMSG:
					CallService(MS_MSG_SENDMESSAGE,(WPARAM)hContact,0);
					break;
                case IDC_TEST:
					debug(ParseString("Date: %d.%m.%y(%Y) \n Date desc: %W - %w - %E - %e \n Time: %H:%M:%S (%h-%p) \n user: %n - %u \n Status: %s \n IP: %i - %r",hContact,0));
					break;
            }
			break;
		case WM_SIZE:
			{
				int dx, dy;
				HDWP hDwp;

				hDwp = BeginDeferWindowPos(6);
				MyResizeGetOffset(hwnd, GetDlgItem(hwnd, IDC_HISTORYLIST), 
						LOWORD(lparam)-15, HIWORD(lparam)-99, &dx, &dy);
				hDwp = MyResizeWindow(hDwp, hwnd, GetDlgItem(hwnd, IDC_USERMENU), 
						dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwnd, GetDlgItem(hwnd, IDC_DETAILS), 
						dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwnd, GetDlgItem(hwnd, IDC_SENDMSG), 
						dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwnd, GetDlgItem(hwnd, IDC_HISTORYLIST), 
						0, 0, dx, dy);
				hDwp = MyResizeWindow(hDwp, hwnd, GetDlgItem(hwnd, IDC_STATUSCHANGE), 
						0, dy, dx, 0);
				hDwp = MyHorizCenterWindow(hDwp, hwnd, GetDlgItem(hwnd, IDOK), 
						LOWORD(lparam), dy, 0);
				EndDeferWindowPos(hDwp);
			}				
			break;
		case WM_GETMINMAXINFO:
			{
				MINMAXINFO mmi;
				CopyMemory (&mmi, (LPMINMAXINFO) lparam, sizeof (MINMAXINFO));

				/* The minimum width in points*/
				mmi.ptMinTrackSize.x = 200;
				/* The minimum height in points*/
				mmi.ptMinTrackSize.y = 190;

				CopyMemory ((LPMINMAXINFO) lparam, &mmi, sizeof (MINMAXINFO));
			}
			break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
			WindowList_Remove(hWindowList,hwnd);
			break;
		case WM_DESTROY:
			Utils_SaveWindowPosition(hwnd,NULL,S_MOD,"History_");
			{	
				HFONT hFont;
				hFont=(HFONT)SendDlgItemMessage(hwnd,IDC_USERMENU,WM_GETFONT,0,0);
				DeleteObject(hFont);
			}
			break;
        default:
            return FALSE;
    }
	return TRUE;
}

void ShowHistory(HANDLE hContact, BYTE isAlert)
{
	HWND hHistoryDlg;
	
	hHistoryDlg = WindowList_Find(hWindowList,hContact);
	if (hHistoryDlg == NULL)
	{
		hHistoryDlg = CreateDialogParam(hinstance,MAKEINTRESOURCE(IDD_HISTORY),NULL,HistoryDlgProc,(LPARAM)hContact);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		WindowList_Add(hWindowList,hHistoryDlg,hContact);
	} 
	else 
	{
		SetForegroundWindow(hHistoryDlg);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		SetFocus(hHistoryDlg);
	}
	
	if (isAlert) 
	{
		SkinPlaySound("LastSeenTrackedStatusChange");
	}
}


void InitHistoryDialog(void)
{
	hWindowList=(HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
}
