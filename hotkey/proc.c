#include "hotkey.h"
#include "..\..\miranda32\ui\contactlist\m_clist.h"
#include "..\..\miranda32\sendrecv\file\m_file.h"
#include "..\..\miranda32\sendrecv\url\m_url.h"
#include "..\..\miranda32\sendrecv\message\m_message.h"
#include "..\..\miranda32\ui\userinfo\m_userinfo.h"
#include "..\..\miranda32\ui\history\m_history.h"
#include "..\..\miranda32\random\skin\m_skin.h"
#include "..\..\miranda32\random\langpack\m_langpack.h"
#include "..\..\miranda32\random\utils\m_utils.h"
#include <stdio.h>



WNDPROC MainProc,StaticProc;
HACCEL hacct;



const char *tiptext[]={"Send message","Send file","Send URL","Open userinfo","Open history"};



typedef struct {
	HWND handle;
	RECT rc;
} WNDRECT;



extern HIMAGELIST himl;
extern CONTACTSTRUCT ns;
extern BYTE ProtoExtension;
extern HINSTANCE hinstance;
extern HWND hmain;
extern HHOOK hhook;



void CheckText(HWND,char *);
void RebuildArray(int);
char *GetProto(HANDLE);
void DrawList(HWND,BYTE);


// get array position from handle
int GetItemPos(HANDLE hcontact)
{
	int loop;

	for(loop=0;loop<ns.count;loop++)
	{
		if(hcontact==ns.contact[loop].hcontact)
			return loop;
	}
	return 0;
}



// this is the callback function for the icon-animation
LRESULT CALLBACK IconProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	RECT rcicon;
	POINT pt;

	switch(msg)
	{
		case WM_SETFOCUS:
			GetWindowRect(hdlg,&rcicon);
			pt.x=rcicon.left;
			pt.y=rcicon.top;
			ScreenToClient(GetParent(hdlg),&pt);
			MoveWindow(hdlg,pt.x-2,pt.y-2,16,16,TRUE);
			return 0;

		case WM_KILLFOCUS:
			GetWindowRect(hdlg,&rcicon);
			pt.x=rcicon.left;
			pt.y=rcicon.top;
			ScreenToClient(GetParent(hdlg),&pt);
			MoveWindow(hdlg,pt.x+2,pt.y+2,16,16,TRUE);
			return 0;

		case WM_KEYUP:
			if(wparam==VK_RETURN)
				SendMessage(GetParent(hdlg),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(hdlg),STN_CLICKED),0);

			if(wparam==VK_ESCAPE)
				ShowWindow(GetParent(hdlg),FALSE);

			return 0;

		default:
			break;
	}

	return CallWindowProc(StaticProc,hdlg,msg,wparam,lparam);
}



// callback function for edit-box of the listbox
// without this the autofill function isn't possible
// this was done like ie does it..as far as spy++ could tell ;)
LRESULT CALLBACK EditProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char sztext[120]="";
	int len;

	switch(msg)
	{
		case WM_CHAR:
			if(wparam<32) break;

			if(SendMessage(hdlg,EM_GETSEL,(WPARAM)NULL,(LPARAM)NULL)!=-1)
			{
				sztext[0]=wparam;sztext[1]=0;
				SendMessage(hdlg,EM_REPLACESEL,(WPARAM)0,(LPARAM)sztext);
			}

			SendMessage(hdlg,WM_GETTEXT,(WPARAM)sizeof sztext,(LPARAM)sztext);
			CheckText(hdlg,sztext);
			return 1;

		case WM_KEYUP:
			if(wparam==VK_RETURN)
			{
				switch(SendMessage(GetParent(hdlg),CB_GETDROPPEDSTATE,0,0))
				{
					case FALSE:
						SendMessage(GetParent(GetParent(hdlg)),WM_COMMAND,MAKEWPARAM(IDC_MESSAGE,STN_CLICKED),0);
						break;

					case TRUE:
						SendMessage(GetParent(hdlg),CB_SHOWDROPDOWN,(WPARAM)FALSE,0);
						break;
				}
			}

			if(wparam==VK_ESCAPE)
			{
				switch(SendMessage(GetParent(hdlg),CB_GETDROPPEDSTATE,0,0))
				{
					case FALSE:
						SendMessage(GetParent(GetParent(hdlg)),UM_SAVEHIDE,0,0);
						break;

					case TRUE:
						SendMessage(GetParent(hdlg),CB_SHOWDROPDOWN,(WPARAM)FALSE,0);
						break;
				}
			}
			return 0;

		case UM_UPDATE:
			len=SendMessage(hdlg,WM_GETTEXTLENGTH,0,0);
			SendMessage(hdlg,WM_SETTEXT,0,wparam);
			SendMessage(hdlg,EM_SETSEL,(WPARAM)len,(LPARAM)-1);
			return 0;

		case WM_GETDLGCODE:
			return DLGC_WANTCHARS|DLGC_WANTARROWS;

	}

	return CallWindowProc(MainProc,hdlg,msg,wparam,lparam);

}



// This function filters the message queue and translates
// the keyboard accelerators
LRESULT CALLBACK HookProc(int code,WPARAM wparam,LPARAM lparam)
{
	MSG *msg;
	HWND htemp;

	if(code!=MSGF_DIALOGBOX) return 0;

	msg=(MSG*)lparam;
	htemp=msg->hwnd;
	msg->hwnd=hmain;

	if(!TranslateAccelerator(msg->hwnd,hacct,msg))
		msg->hwnd=htemp;
		
	return 0;
}



// callback function for the main dialog
// i know gotos are bad style but it was the easiest way :)
BOOL CALLBACK DialogCallback(HWND hdlg,UINT message,WPARAM wparam,LPARAM lparam)
{
	char cname[120];
	HWND hedit,htool;
	LPDRAWITEMSTRUCT lpdis;
	LPMEASUREITEMSTRUCT lpmis;
	COLORREF clrfore,clrback;
	TEXTMETRIC tm;
	WORD lwp;
	int x,y,pos,*array,loop,iw=0,ih=0,clicked=0,len=0;
	WNDRECT wndrc[5];
	TOOLINFO ti;

	switch(message)
	{
		case WM_INITDIALOG:
			htool=CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,"",WS_POPUP|TTS_NOPREFIX,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hdlg,NULL,hinstance,NULL);

			ti.cbSize=sizeof(ti);
			ti.uFlags=TTF_IDISHWND|TTF_SUBCLASS;
			ti.hwnd=hdlg;
			ti.hinst=hinstance;

			hedit=GetWindow(GetWindow(hdlg,GW_CHILD),GW_CHILD);
			SendMessage(hedit,EM_LIMITTEXT,(WPARAM)119,0);
			wndrc[0].handle=GetDlgItem(hdlg,IDC_MESSAGE);
			wndrc[1].handle=GetDlgItem(hdlg,IDC_FILE);
			wndrc[2].handle=GetDlgItem(hdlg,IDC_URL);
			wndrc[3].handle=GetDlgItem(hdlg,IDC_USERINFO);
			wndrc[4].handle=GetDlgItem(hdlg,IDC_HISTORY);

			for(loop=0;loop<5;loop++)
			{
				GetWindowRect(wndrc[loop].handle,&wndrc[loop].rc);
				StaticProc=(WNDPROC)SetWindowLong(wndrc[loop].handle,GWL_WNDPROC,(LONG)IconProc);
				ti.uId=(UINT)wndrc[loop].handle;
				ti.lpszText=Translate(tiptext[loop]);
				SendMessage(htool,TTM_ADDTOOL,0,(LPARAM)&ti);
			}
			


			MainProc=(WNDPROC)SetWindowLong(hedit,GWL_WNDPROC,(LONG)EditProc);

			
			SendDlgItemMessage(hdlg,IDC_USERNAME,CB_SETEXTENDEDUI,(WPARAM)TRUE,0);
			{
				HICON hIcon;

				hIcon=LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
				hIcon=CopyImage(hIcon,IMAGE_ICON,16,16,LR_COPYFROMRESOURCE);
				SendMessage(wndrc[0].handle,STM_SETICON,(WPARAM)hIcon,0);

				hIcon=LoadSkinnedIcon(SKINICON_EVENT_FILE);
				hIcon=CopyImage(hIcon,IMAGE_ICON,16,16,LR_COPYFROMRESOURCE);
				SendMessage(wndrc[1].handle,STM_SETICON,(WPARAM)hIcon,0);

				hIcon=LoadSkinnedIcon(SKINICON_EVENT_URL);
				hIcon=CopyImage(hIcon,IMAGE_ICON,16,16,LR_COPYFROMRESOURCE);
				SendMessage(wndrc[2].handle,STM_SETICON,(WPARAM)hIcon,0);
			}

			SendMessage(wndrc[3].handle,STM_SETICON,(WPARAM)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(160),IMAGE_ICON,16,16,LR_DEFAULTCOLOR),0);
			SendMessage(wndrc[4].handle,STM_SETICON,(WPARAM)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(174),IMAGE_ICON,16,16,LR_DEFAULTCOLOR),0);
			Utils_RestoreWindowPosition(hdlg,NULL,HK_PLG,"window");
			ShowWindow(hdlg,SW_HIDE);

			hacct=LoadAccelerators(hinstance,MAKEINTRESOURCE(ACCEL_TABLE));

			hhook=SetWindowsHookEx(WH_MSGFILTER,HookProc,hinstance,GetCurrentThreadId());

			break;

		case WM_COMMAND:
			lwp=LOWORD(wparam);

			if((lwp==IDC_MESSAGE || lwp==IDC_URL || lwp==IDC_FILE || lwp==IDC_USERINFO || lwp==IDC_HISTORY) && HIWORD(wparam)==STN_CLICKED)
			{
				GetDlgItemText(hdlg,IDC_USERNAME,cname,sizeof cname);
						
				for(loop=0;loop<ns.count;loop++)
				{
					if(!lstrcmpi(cname,ns.contact[loop].szname))
						break;
						
					else if(loop==ns.count-1)
					{
						SetDlgItemText(hdlg,IDC_USERNAME,"");
						SetFocus(GetDlgItem(hdlg,IDC_USERNAME));
						return 0;
					}
				}



				
				switch(lwp)
				{
					case IDC_MESSAGE:
						// don't know why it doesn't work with MS_MSG_SENDMESSAGE
						// when convers is enabled
						array=(int *)CallService(MS_PLUGINS_GETDISABLEDEFAULTARRAY,0,0);
						if(CallService("SRMsg/LaunchMessageWindow",(WPARAM)ns.contact[loop].hcontact,0)==CALLSERVICE_NOTFOUND)
							CallService(MS_MSG_SENDMESSAGE,(WPARAM)ns.contact[loop].hcontact,0);
						SendMessage(hdlg,UM_SAVEHIDE,0,0);
						break;

					case IDC_FILE:
						CallService(MS_FILE_SENDFILE,(WPARAM)ns.contact[loop].hcontact,0);
						SendMessage(hdlg,UM_SAVEHIDE,0,0);
						break;

					case IDC_URL:
						CallService(MS_URL_SENDURL,(WPARAM)ns.contact[loop].hcontact,0);
						SendMessage(hdlg,UM_SAVEHIDE,0,0);
						break;

					case IDC_USERINFO:
						CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)ns.contact[loop].hcontact,0);
						SendMessage(hdlg,UM_SAVEHIDE,0,0);
						break;

					case IDC_HISTORY:
						CallService(MS_HISTORY_SHOWCONTACTHISTORY,(WPARAM)ns.contact[loop].hcontact,0);
						SendMessage(hdlg,UM_SAVEHIDE,0,0);
						break;

				}
				SetDlgItemText(hdlg,IDC_USERNAME,"");
				DBWriteContactSettingDword(NULL,HK_PLG,"LastSentTo",(DWORD)ns.contact[loop].hcontact);
				SetWindowLong(hdlg,GWL_USERDATA,(LONG)ns.contact[loop].hcontact);
				return 0;
			}

			switch(lwp){
				
				case HOTKEY_FILE:
					SendMessage(hdlg,WM_COMMAND,MAKEWPARAM(IDC_FILE,STN_CLICKED),0);
					break;

				case HOTKEY_URL:
					SendMessage(hdlg,WM_COMMAND,MAKEWPARAM(IDC_URL,STN_CLICKED),0);
					break;

				case HOTKEY_INFO:
					SendMessage(hdlg,WM_COMMAND,MAKEWPARAM(IDC_USERINFO,STN_CLICKED),0);
					break;

				case HOTKEY_HISTORY:
					SendMessage(hdlg,WM_COMMAND,MAKEWPARAM(IDC_HISTORY,STN_CLICKED),0);
					break;
			}



			
			return 0;


		case WM_DRAWITEM:
			// add icons and protocol to listbox
			lpdis=(LPDRAWITEMSTRUCT)lparam;

			if(lpdis->itemID==-1) return 0;

			GetTextMetrics(lpdis->hDC, &tm);
			ImageList_GetIconSize(himl,&iw,&ih);

            y=(lpdis->rcItem.bottom+lpdis->rcItem.top-tm.tmHeight)/2;
            x=(lpdis->rcItem.left+10+iw);

			clrfore=SetTextColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHTTEXT:COLOR_WINDOWTEXT));
			clrback=SetBkColor(lpdis->hDC,GetSysColor(lpdis->itemState & ODS_SELECTED?COLOR_HIGHLIGHT:COLOR_WINDOW));

			ExtTextOut(lpdis->hDC,x,y,ETO_OPAQUE,&lpdis->rcItem,ns.contact[lpdis->itemData].szname,strlen(ns.contact[lpdis->itemData].szname),NULL);

			if(ProtoExtension)
			{
				x=(lpdis->rcItem.right-(tm.tmMaxCharWidth*3)-2);
				TextOut(lpdis->hDC,x,y,ns.contact[lpdis->itemData].proto,3);
			}

			x=(lpdis->rcItem.left + 5);
			y=(lpdis->rcItem.bottom+lpdis->rcItem.top-ih)/2;
			ImageList_Draw(himl,CallService(MS_CLIST_GETCONTACTICON,(WPARAM)ns.contact[lpdis->itemData].hcontact,0),lpdis->hDC,x,y,ILD_NORMAL);
			

			SetTextColor(lpdis->hDC,clrfore);
			SetBkColor(lpdis->hDC,clrback);
			return 1;

		case WM_MEASUREITEM:
			lpmis=(LPMEASUREITEMSTRUCT)lparam;
			GetTextMetrics(GetDC(hdlg),&tm);
			ImageList_GetIconSize(himl,&iw,&ih);
			if(ih>=tm.tmHeight)
				lpmis->itemHeight=ih;
			else
				lpmis->itemHeight=tm.tmHeight;
				
			return 1;

		case UM_NAMECHANGE:
			// change the name of a listbox item
			pos=GetItemPos((HANDLE)wparam);

			if(lstrcmp(ns.contact[pos].proto,PROTO_ICQ) || strcmp(ns.contact[pos].proto,PROTO_MSN))
				lstrcpy(ns.contact[pos].proto,GetProto((HANDLE)wparam));

			lstrcpyn(ns.contact[pos].szname,(const char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,wparam,0),sizeof ns.contact[pos].szname);
			DrawList(hdlg,1);
			return 0;

		case UM_DELETECONTACT:
			pos=GetItemPos((HANDLE)wparam);
			RebuildArray(pos);
			DrawList(hdlg,0);
			return 0;

		case UM_SAVEHIDE:
			ShowWindow(hdlg,SW_HIDE);
			Utils_SaveWindowPosition(hdlg,NULL,HK_PLG,"window");
			break;

		case WM_KILLFOCUS:
			return 0;

		case WM_ACTIVATEAPP:
			if(LOWORD(wparam) || GetFocus()!=hdlg)
				SendMessage(hdlg,UM_SAVEHIDE,0,0);

			else ShowWindow(hdlg,SW_SHOW);
			return 0;

		case WM_SHOWWINDOW:
			SetFocus(GetDlgItem(hdlg,IDC_USERNAME));
			return 0;

		case WM_HOTKEY:
			if(wparam==HOTKEY_GENERAL)
			{
				Utils_RestoreWindowPositionNoSize(hdlg,NULL,HK_PLG,"window");
				ShowWindow(hdlg,SW_SHOW);
				SetForegroundWindow(hdlg);

				if(DBGetContactSettingByte(NULL,HK_PLG,"EnableLastSentTo",0))
					SendDlgItemMessage(hdlg,IDC_USERNAME,CB_SETCURSEL,(WPARAM)GetItemPos((HANDLE)GetWindowLong(hdlg,GWL_USERDATA)),0);
				else
					SetDlgItemText(hdlg,IDC_USERNAME,"");
				break;
			}

		case WM_CLOSE:
			SendMessage(hdlg,UM_SAVEHIDE,0,0);
			break;

		case WM_DESTROY:
			UnhookWindowsHookEx(hhook);
			break;

	}
	return 0;
}