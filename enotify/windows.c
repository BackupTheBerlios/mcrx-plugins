#include "msgpopup.h"


// number of open popups
BYTE count=0;
// hwnd's of opened popups
HWND popuparray[32]={NULL};


// main.c
extern HINSTANCE hinstance;
extern RECT screen;
extern struct EXTENDSTRUCT extend[MAX_STYLES];


//main.c
BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
// windows.c
int RemovePopup(HWND);
// utils.c
int CheckMsgWnd(WPARAM);


// get the message,url or file data from DB
char *GetBlob(PDATA pdata)
{
	DBEVENTINFO dbei;
	static char *pszPreview;
	char *pCurBlob;

	if(pdata->hDBEvent==NULL && pdata->hContact==NULL)
		return 0;

	dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)pdata->hDBEvent,0);
	dbei.pBlob=(PBYTE)malloc(dbei.cbBlob);
	dbei.cbSize=sizeof(dbei);
	CallService(MS_DB_EVENT_GET,(WPARAM)pdata->hDBEvent,(LPARAM)&dbei);

	pszPreview=(char *)malloc(dbei.cbBlob+4);

	switch(dbei.eventType){

		case EVENTTYPE_MESSAGE:
			strcpy(pszPreview,dbei.pBlob);
			pszPreview=realloc(pszPreview,strlen(pszPreview)+1);
			break;

		case EVENTTYPE_URL:
			strcpy(pszPreview,dbei.pBlob);
			strcat(pszPreview,"\n\n");
			strcat(pszPreview,dbei.pBlob+strlen(dbei.pBlob)+1);
			pszPreview=realloc(pszPreview,strlen(pszPreview)+1);
			break;

		case EVENTTYPE_FILE:
			strcpy(pszPreview,dbei.pBlob+4);
			strcat(pszPreview,"\n\n");
			strcat(pszPreview,dbei.pBlob+4+strlen(dbei.pBlob+4)+1);
			pszPreview=realloc(pszPreview,strlen(pszPreview)+1);
			break;

		case EVENTTYPE_AUTHREQUEST:
			pCurBlob=dbei.pBlob+sizeof(DWORD)+sizeof(HANDLE);
			pCurBlob+=strlen(pCurBlob)+1;pCurBlob+=strlen(pCurBlob)+1;
			pCurBlob+=strlen(pCurBlob)+1;pCurBlob+=strlen(pCurBlob)+1;
			strcpy(pszPreview,pCurBlob);
			pszPreview=realloc(pszPreview,strlen(pszPreview)+1);
			break;

		case EVENTTYPE_ADDED:
		default:
			free(pszPreview);
			pszPreview="";
			break;
	}

	free(dbei.pBlob);


	return pszPreview;
}
	

// check for right-mouse-button clicks into popup window
// meeded because of help.dll
LRESULT CALLBACK GetRightClick(int code,WPARAM wparam,LPARAM lparam)
{
	CWPSTRUCT *cwp=(CWPSTRUCT *)lparam;

	if(code!=HC_ACTION) return 0;

	if(cwp->message!=WM_RBUTTONUP) return 0;

	SendMessage(cwp->hwnd,cwp->message,cwp->wParam,cwp->lParam);

	return 0;
}


// DlgProc for popup window
LRESULT CALLBACK PopupProc(HWND hpopup,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static HBRUSH hbrush;
	static HHOOK hhook;
	DATASTRUCT *pdata;
	static char *pszPreview;

	pdata=(DATASTRUCT *)GetWindowLong(hpopup,GWL_USERDATA);
	switch(msg){

		case WM_INITDIALOG:
			{
			LOGFONT lf={0};
			DBVARIANT dbv;
			TOOLINFO ti;
			char *szFooter;
			HWND htool;
			RECT client;
			BYTE time;
			
			pdata=(DATASTRUCT *)malloc(sizeof(DATASTRUCT));

			SetWindowLong(hpopup,GWL_USERDATA,(LONG)pdata);
			*pdata=*(DATASTRUCT *)lparam;

			if(!(pszPreview=GetBlob(pdata)))
				pszPreview=Translate("Here will be the preview of the message/URL/file/auth request/\"You were added\" event.");

			htool=CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,"",TTS_ALWAYSTIP|WS_POPUP|TTS_NOPREFIX,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hpopup,NULL,hinstance,NULL);
			GetClientRect(hpopup,&client);

			ti.cbSize=sizeof(TOOLINFO);
			ti.uFlags=TTF_SUBCLASS;
			ti.hwnd=hpopup;
			ti.hinst=hinstance;
			ti.uId=0;
			ti.lpszText=pszPreview;
		    ti.rect.left=client.left;    
			ti.rect.top=client.top;
			ti.rect.right=client.right;
			ti.rect.bottom=client.bottom;

			SendMessage(htool,TTM_ADDTOOL,0,(LPARAM)&ti);
			SendMessage(htool, TTM_SETMAXTIPWIDTH, 0, 200);

			// ti.rect contains the spacing between text and border of tooltip
			ti.rect.left=5;
			ti.rect.right=5;
			ti.rect.top=5;
			ti.rect.bottom=5;

			SendMessage(htool,TTM_SETMARGIN,0,(LPARAM)&ti.rect);

			hhook=SetWindowsHookEx(WH_CALLWNDPROC,GetRightClick,hinstance,GetCurrentThreadId());

			if(!DBGetContactSetting(NULL,MODULE,"Font",&dbv))
				lf=*(LOGFONT*)dbv.pbVal;

			else
				GetObject((HFONT)SendDlgItemMessage(hpopup,IDC_NICKNAME,WM_GETFONT,0,0),sizeof(lf),&lf);

			SendDlgItemMessage(hpopup,IDC_FOOTER,WM_SETFONT,(WPARAM)CreateFontIndirect(&lf),MAKELPARAM(TRUE,0));
			lf.lfWeight=FW_BOLD;
			
			SendDlgItemMessage(hpopup,IDC_NICKNAME,WM_SETFONT,(WPARAM)CreateFontIndirect(&lf),MAKELPARAM(TRUE,0));

			DBFreeVariant(&dbv);

			switch(pdata->uEvent){

				case EVENTTYPE_MESSAGE:
					szFooter=Translate("New message");
					break;

				case EVENTTYPE_URL:
					szFooter=Translate("New URL");
					break;

				case EVENTTYPE_FILE:
					szFooter=Translate("New file");
					break;

				case EVENTTYPE_AUTHREQUEST:
					szFooter=Translate("Auth request");
					break;

				case EVENTTYPE_ADDED:
					szFooter=Translate("You were added");
					break;

				case PREVIEW_EVENT:
					szFooter=Translate("This is a preview");
					break;
			}

			SetDlgItemText(hpopup,IDC_FOOTER,(LPCSTR)szFooter);

			// transparency
			if((MySetLayeredWindowAttributes) && !DBGetContactSetting(NULL,MODULE,"Flags",&dbv) && ((PFLAGS)dbv.pbVal)->bBKStyle==BKSTYLE_TRANSPARENT)
			{
				DBFreeVariant(&dbv);
				SetWindowLong(hpopup, GWL_EXSTYLE, GetWindowLong(hpopup, GWL_EXSTYLE) | WS_EX_LAYERED);
				MySetLayeredWindowAttributes(hpopup, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,MODULE,"TransVal",0), LWA_ALPHA);
			}

			if(time=DBGetContactSettingByte(NULL,MODULE,"Time",5))
				SetTimer(hpopup,POPUP_TIMER,time*1000,NULL);
			}
			break;

		case WM_CTLCOLORSTATIC:
			{
			COLORREF fgcol,bkcol;

			fgcol=DBGetContactSettingDword(NULL,MODULE,"FGCol",GetSysColor(COLOR_WINDOWTEXT));
			bkcol=DBGetContactSettingDword(NULL,MODULE,"BKCol",GetSysColor(COLOR_BTNFACE));

			hbrush=CreateSolidBrush(bkcol);
			SetBkColor((HDC)wparam,bkcol);
			SetBkMode((HDC)wparam,TRANSPARENT);
			SetTextColor((HDC)wparam,fgcol);
			}
			return (BOOL)hbrush;

		case WM_CTLCOLORDLG:
			{
			COLORREF bkcol;

			bkcol=DBGetContactSettingDword(NULL,MODULE,"BKCol",GetSysColor(COLOR_BTNFACE));

			hbrush=CreateSolidBrush(bkcol);
			SetBkColor((HDC)wparam,bkcol);
			}
			return (BOOL)hbrush;

		case WM_TIMER:
			KillTimer(hpopup,POPUP_TIMER);
			SendMessage(hpopup,WM_CLOSE,0,0);
			break;

		case WM_LBUTTONUP:
			if(pdata->uEvent!=PREVIEW_EVENT && !(pdata->uEvent==EVENTTYPE_MESSAGE && CheckMsgWnd((WPARAM)pdata->hContact)==1))
			{
				CLISTEVENT *cle;

				cle=(CLISTEVENT *)CallService(MS_CLIST_GETEVENT,(WPARAM)(pdata->hContact?pdata->hContact:INVALID_HANDLE_VALUE),0);
				if(cle)
				{
					CallService(cle->pszService,(WPARAM)(HWND)CallService(MS_CLUI_GETHWND,0,0),(LPARAM)cle);
					CallService(MS_CLIST_REMOVEEVENT,(WPARAM)cle->hContact,(LPARAM)pdata->hDBEvent);
				}
			}
			SendMessage(hpopup,WM_CLOSE,0,0);
			break;

		case WM_RBUTTONUP:
			SendMessage(hpopup,WM_CLOSE,0,0);
			break;

		case UM_SETPOS:
			SetWindowPos(hpopup,HWND_TOPMOST,LOWORD(wparam),HIWORD(wparam),0,0,SWP_NOSIZE|SWP_NOACTIVATE);
			ShowWindow(hpopup,SW_SHOWNOACTIVATE);
			break;

		case WM_CLOSE:
			free(pdata);
			free(pszPreview);
			UnhookWindowsHookEx(hhook);
			RemovePopup(hpopup);
			break;

		case WM_DESTROY:
			DeleteObject(hbrush);
			break;
	}

	return 0;
}


// get the style flags from db
PFLAGS GetFlags(void)
{
	DBVARIANT dbv;
	PFLAGS pflags;

	pflags=(PFLAGS)malloc(sizeof(FLAGSTRUCT));

	ZeroMemory(pflags,sizeof(FLAGSTRUCT));

	if(!DBGetContactSetting(NULL,MODULE,"Flags",&dbv))
		*pflags=*(PFLAGS)dbv.pbVal;
	else 
	{
		pflags->bStyle=STYLE_FLAT;
		pflags->bPos=POS_CENTER;
		pflags->bSpreading=SPREAD_VERT;
	}

	DBFreeVariant(&dbv);

	return pflags;
}


// show popups at non-centered positions
int ShowNormal(int x,int y,int dx,int dy,int xmod,int ymod)
{
	int loop;

	for(loop=0;loop<count;loop++,y+=ymod*dy,x+=xmod*dx)
	{
		if(x>=screen.right || x<=screen.left || y>=screen.bottom-dy || y<=screen.top)
				break;

		SendMessage(popuparray[loop],UM_SETPOS,MAKEWPARAM(x,y),0);
	}

	return 0;
}


// show popups at centered position
int ShowCentered(int x,int y,int dx,int dy,int xjump,int yjump,int xmod,int ymod)
{
	int loop;
	int ystart,ypos,xstart,xpos;

	ypos=ystart=y-(ymod*(count-1)*dy);
	xpos=xstart=x-(xmod*(count-1)*dx);

	for(loop=0;loop<count;loop++,ypos=ystart+ymod*(loop*yjump),xpos=xstart+xmod*(loop*xjump))
	{
		if(count==1)
			SendMessage(popuparray[loop],UM_SETPOS,MAKEWPARAM(x,y),0);

		else
		{
			if(xpos>=screen.right || xpos<=screen.left || ypos>=screen.bottom || ypos<=screen.top)
				break;

			SendMessage(popuparray[loop],UM_SETPOS,MAKEWPARAM(xpos,ypos),0);
		}
	}

	return 0;
}


// get direction of popup spreading
int GetSpreading(PFLAGS pflags,int *xmod,int *ymod)
{
	switch(pflags->bSpreading){

		case SPREAD_VERT:
			*xmod=0;
			*ymod=1;
			break;

		case SPREAD_HORZ:
			*xmod=1;
			*ymod=0;
			break;
	}

	return 0;
}


// update position of all popups
// x:		starting position (x-axis)
// y:		starting position (y-axis)
// dx:		value to add for each popup (x-axis)
// dy:		value to add for each popup (y-axis)
// xmod:	spreading - horizontal
// ymod:	spreading - vertical
// xjump,yjump:	 (centered only) popup height+spacing
int ShowPopups(void)
{
	PFLAGS pflags;
	int x,y,dy,dx,xmod,ymod,xjump,yjump;

	pflags=GetFlags();
	GetSpreading(pflags,&xmod,&ymod);

	switch(pflags->bPos){

		case POS_TOPLEFT:
			x=SPACING;
			y=SPACING;
			dy=(SPACING+extend[pflags->bStyle-1].height);
			dx=(SPACING+extend[pflags->bStyle-1].width);
			ShowNormal(x,y,dx,dy,xmod,ymod);
			break;

		case POS_TOPRIGHT:
			x=screen.right-SPACING-extend[pflags->bStyle-1].width;
			y=SPACING;
			dy=(SPACING+extend[pflags->bStyle-1].height);
			dx=-(SPACING+extend[pflags->bStyle-1].width);
			ShowNormal(x,y,dx,dy,xmod,ymod);
			break;

		case POS_BOTTOMLEFT:
			x=SPACING;
			y=screen.bottom-SPACING-extend[pflags->bStyle-1].height;
			dy=-(SPACING+extend[pflags->bStyle-1].height);
			dx=(SPACING+extend[pflags->bStyle-1].width);
			ShowNormal(x,y,dx,dy,xmod,ymod);
			break;

		case POS_BOTTOMRIGHT:
			x=screen.right-SPACING-extend[pflags->bStyle-1].width;
			y=screen.bottom-SPACING-extend[pflags->bStyle-1].height;
			dy=-(SPACING+extend[pflags->bStyle-1].height);
			dx=-(SPACING+extend[pflags->bStyle-1].width);
			ShowNormal(x,y,dx,dy,xmod,ymod);
			break;

		case POS_CENTER:
			x=(screen.right-extend[pflags->bStyle-1].width)/2;
			y=(screen.bottom-extend[pflags->bStyle-1].height)/2;
			dy=(extend[pflags->bStyle-1].height+SPACING)/2;
			dx=(extend[pflags->bStyle-1].width+SPACING)/2;
			xjump=extend[pflags->bStyle-1].width+SPACING;
			yjump=extend[pflags->bStyle-1].height+SPACING;
			ShowCentered(x,y,dx,dy,xjump,yjump,xmod,ymod);
			break;
	}

	free(pflags);

	return 0;
}


// remove a popup from list and update the positions
// of remaining popups
int RemovePopup(HWND hpopup)
{
	int loop;

	for(loop=0;loop<count;loop++)
	{
		if(popuparray[loop]==hpopup)
		{
			DestroyWindow(hpopup);
			popuparray[loop]=NULL;
			break;
		}
	}

	for(;loop<count-1;loop++)
		popuparray[loop]=popuparray[loop+1];

	count--;

	ShowPopups();

	return 0;
}
		

// retrieve resource identifiers from style flags
int GetStyle(UINT *pstyle)
{
	PFLAGS pflags;

	pflags=GetFlags();

	switch(pflags->bStyle){

		case STYLE_FLAT:
			*pstyle=IDD_FLAT;
			break;

		case STYLE_3D:
			*pstyle=IDD_3D;
			break;

		case STYLE_MINIMAL:
			*pstyle=IDD_MINIMAL;
			break;
	}

	free(pflags);

	return 0;
}


// add a popup window to the list and update
// positions of the windows
int WindowRequest(HANDLE hcontact,UINT event,int iconID,HANDLE hevent)
{
	HWND hpopup;
	UINT style;
	PDATA pdata;
	HICON hIcon;

	// fall-thru
	if(count==32) return 0;

	pdata=(PDATA)malloc(sizeof(DATASTRUCT));
	pdata->uEvent=event;
	pdata->hContact=hcontact;
	pdata->hDBEvent=hevent;

	GetStyle(&style);

	hpopup=CreateDialogParam(hinstance,MAKEINTRESOURCE(style),NULL,PopupProc,(LPARAM)pdata);

	SetDlgItemText(hpopup,IDC_NICKNAME,(LPCSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hcontact,0));

	hIcon=LoadSkinnedIcon(iconID);
	hIcon=CopyImage(hIcon,IMAGE_ICON,16,16,LR_COPYFROMRESOURCE);
	SendDlgItemMessage(hpopup,IDC_CICON,STM_SETICON,(WPARAM)hIcon,0);

	popuparray[count++]=hpopup;

	ShowPopups();

	free(pdata);

	return 0;
}