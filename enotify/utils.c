#include "msgpopup.h"
#include <stdio.h>
#include "..\SDK\headers_c\m_protosvc.h"


// windows.c
int WindowRequest(HANDLE,UINT,int,HANDLE);


// checks if the message-dialog window is already opened
// return values:
//	0 - No window found
//	1 - Split-mode window found
//	2 - Single-mode window found
int CheckMsgWnd(WPARAM contact)
{
	char newtitle[256];
	char *szStatus,*contactName;
	char *szProto;

	szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,contact,0);
	contactName=(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,contact,0);
	szStatus=(char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,szProto==NULL?ID_STATUS_OFFLINE:DBGetContactSettingWord((HANDLE)contact,szProto,"Status",ID_STATUS_OFFLINE),0);
	_snprintf(newtitle,sizeof(newtitle),"%s (%s): %s",contactName,szStatus,Translate("Message Session"));

	if(FindWindow("#32770",newtitle))
		return 1;

	_snprintf(newtitle,sizeof(newtitle),"%s (%s): %s",contactName,szStatus,Translate("Message Received"));

	if(FindWindow("#32770",newtitle))
		return 2;

	return 0;
}


// called when an event is added to the DB
int CheckNewEvents(WPARAM wparam,LPARAM lparam)
{
	DBEVENTINFO dbei={0};
	WORD mode,status;

	dbei.cbBlob=0;
	dbei.cbSize=sizeof(dbei);

	CallService(MS_DB_EVENT_GET,(WPARAM)lparam,(LPARAM)&dbei);

	if(dbei.eventType!=EVENTTYPE_MESSAGE && dbei.eventType!=EVENTTYPE_FILE && dbei.eventType!=EVENTTYPE_URL && dbei.eventType!=EVENTTYPE_ADDED && dbei.eventType!=EVENTTYPE_AUTHREQUEST)
		return 0;

	if(dbei.flags&DBEF_SENT)
		return 0;

	mode=DBGetContactSettingWord(NULL,MODULE,"ModeFlag",NOTIFY_MSG|NOTIFY_URL|NOTIFY_FILE|NOTIFY_AUTH|NOTIFY_ADDED|DISMISS_AWAY|DISMISS_NA|DISMISS_DND);

	if(dbei.eventType==EVENTTYPE_MESSAGE && !(mode&NOTIFY_MSG))
		return 0;

	if(dbei.eventType==EVENTTYPE_URL && !(mode&NOTIFY_URL))
		return 0;

	if(dbei.eventType==EVENTTYPE_FILE && !(mode&NOTIFY_FILE))
		return 0;

	if(dbei.eventType==EVENTTYPE_AUTHREQUEST && !(mode&NOTIFY_AUTH))
		return 0;

	if(dbei.eventType==EVENTTYPE_ADDED && !(mode&NOTIFY_ADDED))
		return 0;

	status=(WORD)CallProtoService(dbei.szModule,PS_GETSTATUS,0,0);

	switch(status){

		case ID_STATUS_ONLINE:
			if(mode&DISMISS_ONLINE) return 0;
			break;

		case ID_STATUS_AWAY:
			if(mode&DISMISS_AWAY) return 0;
			break;

		case ID_STATUS_NA:
			if(mode&DISMISS_NA) return 0;
			break;

		case ID_STATUS_OCCUPIED:
			if(mode&DISMISS_OCCUPIED) return 0;
			break;

		case ID_STATUS_DND:
			if(mode&DISMISS_DND) return 0;
			break;

		case ID_STATUS_FREECHAT:
			if(mode&DISMISS_FREE) return 0;
			break;

		case ID_STATUS_INVISIBLE:
			if(mode&DISMISS_INVIS) return 0;
			break;

		case ID_STATUS_ONTHEPHONE:
			if(mode&DISMISS_PHONE) return 0;
			break;

		case ID_STATUS_OUTTOLUNCH:
			if(mode&DISMISS_LUNCH) return 0;
			break;

		default:
			return 0;
	}

	dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)lparam,0);
	dbei.pBlob=(PBYTE)malloc(dbei.cbBlob);
	CallService(MS_DB_EVENT_GET,(WPARAM)lparam,(LPARAM)&dbei);

	switch(dbei.eventType){

		case EVENTTYPE_MESSAGE:
			if(DBGetContactSettingByte(NULL,"SRMsg","AutoPopup",0) && !(mode&NOTIFY_ALWAYS)) break;

			if(CheckMsgWnd(wparam) && !(mode&NOTIFY_ALWAYS)) break;

			WindowRequest((HANDLE)wparam,EVENTTYPE_MESSAGE,SKINICON_EVENT_MESSAGE,(HANDLE)lparam);
			break;

		case EVENTTYPE_URL:
			WindowRequest((HANDLE)wparam,EVENTTYPE_URL,SKINICON_EVENT_URL,(HANDLE)lparam);
			break;

		case EVENTTYPE_FILE:
			WindowRequest((HANDLE)wparam,EVENTTYPE_FILE,SKINICON_EVENT_FILE,(HANDLE)lparam);
			break;

		case EVENTTYPE_AUTHREQUEST:
			WindowRequest((HANDLE)wparam,EVENTTYPE_AUTHREQUEST,SKINICON_OTHER_MIRANDA,(HANDLE)lparam);
			break;

		case EVENTTYPE_ADDED:
			WindowRequest((HANDLE)wparam,EVENTTYPE_ADDED,SKINICON_OTHER_MIRANDA,(HANDLE)lparam);
			break;
	}

	free(dbei.pBlob);
			
	return 0;
}
