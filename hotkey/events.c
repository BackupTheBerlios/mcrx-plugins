#include "hotkey.h"



extern HWND hmain;



//int GetItemPos(HANDLE);


// called when a message/file/url was sent
// handle of contact is set as window-userdata
int EventAdded(WPARAM wparam,LPARAM lparam)
{
	DBEVENTINFO dbei;

    ZeroMemory(&dbei,sizeof(dbei));
    dbei.cbSize=sizeof(dbei);
    dbei.cbBlob=0;
    
    CallService(MS_DB_EVENT_GET,lparam,(LPARAM)&dbei);
    
	if(!(dbei.flags&DBEF_SENT) || dbei.flags&DBEF_READ || !DBGetContactSettingByte(NULL,HK_PLG,"EnableLastSentTo",0) || DBGetContactSettingWord(NULL,HK_PLG,"MsgTypeRec",IDC_GLOBAL)!=IDC_GLOBAL) return 0;

	DBWriteContactSettingDword(NULL,HK_PLG,"LastSentTo",(DWORD)(HANDLE)wparam);
	SetWindowLong(hmain,GWL_USERDATA,(LONG)(HANDLE)wparam);
	return 0;
}
