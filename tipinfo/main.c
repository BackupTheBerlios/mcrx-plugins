#include "tooltip.h"
#include "..\SDK\headers_c\m_system.h"
#include "..\SDK\headers_c\m_clist.h"
#include "..\SDK\headers_c\m_protocols.h"
#include "..\SDK\headers_c\m_protosvc.h"
#include "..\SDK\headers_c\m_utils.h"
#include "..\SDK\headers_c\m_contacts.h"



HINSTANCE hinstance;
PLUGINLINK *pluginLink;
HWND htt;
UINT nTimer;
int replace_pos;
char *pszReplacement,*template_str=NULL;
HANDLE hProcess=(HANDLE)ID_STATUS_OFFLINE,hevent=NULL;
PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
		"Tip info",
		PLUGIN_MAKE_VERSION(2,1,2,0),
		"Show various user/group infos as tooltips",
		"Heiko Schillinger",
		"micron@nexgo.de",
		"© 2002-03 Heiko Schillinger",
		"http://nortiq.com/miranda",
		0,
		0
};


int OptionsInit(WPARAM,LPARAM);



VOID CALLBACK TimerProc(HWND hwnd,UINT msg,UINT Id,DWORD time)
{
	TOOLINFO ti;
	int len;

	ti.cbSize=sizeof(TOOLINFO);
	ti.hwnd=NULL;
	ti.uId=TIP_ID;

	KillTimer(NULL,nTimer);
	
	if(!strncmp(template_str+replace_pos,Translate("Retrieving mode message..."),6))
	{
		len=lstrlen(Translate("Retrieving mode message..."));

		MoveMemory(template_str+replace_pos,template_str+replace_pos+len,lstrlen(template_str)+1-replace_pos-len);
		template_str=(char *)realloc(template_str,lstrlen(template_str)+1);
		ti.lpszText=template_str;
		SendMessage(htt,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
	}
}


int RemoveMessage(char *template_str)
{
	if(!strncmp(template_str+replace_pos,Translate("Retrieving mode message..."),6))
	{
		int len=lstrlen(Translate("Retrieving mode message..."));

		MoveMemory(template_str+replace_pos,template_str+replace_pos+len,lstrlen(template_str)+1-replace_pos-len);
		template_str=(char *)realloc(template_str,lstrlen(template_str)+1);
	}

	
	return 0;
}


int ProtoAck(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *pAck=(ACKDATA *)lparam;
	
	if(pAck->hProcess!=hProcess) return 0;
	hProcess=(HANDLE)ID_STATUS_OFFLINE;
	if(pAck->type!=ACKTYPE_AWAYMSG) return 0;
	{
		TOOLINFO ti;

		switch(pAck->result){

			case ACKRESULT_SUCCESS:
				{
					int len;
					char *smsg=strdup((char *)pAck->lParam);

					ti.cbSize=sizeof(TOOLINFO);
					ti.hwnd=NULL;
					ti.uId=TIP_ID;

					KillTimer(NULL,nTimer);
					
					RemoveMessage(template_str);
					len=lstrlen(template_str)+1+lstrlen(smsg);
					template_str=(char *)realloc(template_str,len);

					MoveMemory(template_str+replace_pos+lstrlen(smsg),template_str+replace_pos,lstrlen(template_str)+1-replace_pos);
					CopyMemory(template_str+replace_pos,smsg,lstrlen(smsg));
					ti.lpszText=template_str;
					free(smsg);
				}
				break;
			case ACKRESULT_FAILED:
				return 0;
		}

		SendMessage(htt,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
	}

	return 0;
}


DWORD StatusModeToProtoFlag(int status)
{
	switch(status) {
		case ID_STATUS_ONLINE:
			return PF2_ONLINE;
		case ID_STATUS_OFFLINE:
			break;
		case ID_STATUS_INVISIBLE:
			return PF2_INVISIBLE;
		case ID_STATUS_OUTTOLUNCH:
			return PF2_OUTTOLUNCH;
		case ID_STATUS_ONTHEPHONE:
			return PF2_ONTHEPHONE;
		case ID_STATUS_AWAY:
			return PF2_SHORTAWAY;
		case ID_STATUS_NA:
			return PF2_LONGAWAY;
		case ID_STATUS_OCCUPIED:
			return PF2_LIGHTDND;
		case ID_STATUS_DND:
			return PF2_HEAVYDND;
		case ID_STATUS_FREECHAT:
			return PF2_FREECHAT;
	}
	return 0;
}


void remove_breaks(char *sub_str,int *pos,int *where,int token_len)
{
	get_sub("");

	if(DBGetContactSettingByte(NULL,TIP_MOD,"RemoveBreak",1) && *pos>1)
	{
		while(!strncmp((template_str+(*pos)-2),"\r\n",2) && (!strncmp((template_str+(*pos)+token_len),"\r\n",2) || !*(template_str+(*pos)+token_len)))
		{
			MoveMemory(template_str+(*pos)-2,template_str+(*pos),lstrlen(template_str+(*pos))+1);
			*pos-=2;
			*where-=2;
			template_str=(char *)realloc(template_str,lstrlen(template_str)+1);
		}
	}
	if(*pos<=0 && !strncmp(template_str+token_len,"\r\n",2))
	{
		MoveMemory(template_str+token_len,template_str+2+token_len,lstrlen(template_str+2+token_len)+1);
		*pos-=1;
	}

	if(*pos>1)
		*pos-=2;
}


char *GetTipText(HANDLE hItem,int isGroup,int isFocused,HWND hlist)
{
	int loop;

	if(!isGroup)
	{
		static char szProto[14],sub_str[256];
		DBVARIANT dbv;
		BOOL mustreplace=FALSE;
		int status,token_len,pos;
		CONTACTINFO ci;

		if(!DBGetContactSettingByte(NULL,TIP_MOD,"Contact",0))
			return "";

		lstrcpyn(szProto,(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hItem,0),14);

		if(!szProto) return Translate("(Unknown Contact)");

		status=DBGetContactSettingWord(hItem,szProto,"Status",ID_STATUS_OFFLINE);

		if(DBGetContactSetting(NULL,TIP_MOD,"Template",&dbv))
			dbv.pszVal="%id\r\n%mail\r\n%smsg";

		template_str=(char *)realloc(template_str,lstrlen(dbv.pszVal)+1);
		if(!template_str) debug("Memory could not be allocated");
		CopyMemory(template_str,dbv.pszVal,lstrlen(dbv.pszVal)+1);
		clean_up;
		ZeroMemory(&ci,sizeof(CONTACTINFO));
		ci.cbSize=sizeof(CONTACTINFO);
		ci.hContact=hItem;
		ci.szProto=szProto;


		for(pos=loop=token_len=0;template_str[loop];loop++,get_sub(""))
		{
			if(template_str[loop]!='%') continue;
			pos=loop;
			
			if(search_token("%lname",6))
			{
				ci.dwFlag=CNF_CUSTOMNICK;
				if(get_ci)
					ci.pszVal="";
				set_pszinfo;
				token_len=6;
				break_remover;
			}

			else if(search_token("%first",6))
			{
				ci.dwFlag=CNF_FIRSTNAME;
				if(get_ci)
					ci.pszVal="";
				set_pszinfo;
				token_len=6;
				break_remover;
			}

			else if(search_token("%last",5))
			{
				ci.dwFlag=CNF_LASTNAME;
				if(get_ci)
					ci.pszVal="";
				set_pszinfo;
				token_len=5;
				break_remover;
			}

			else if(search_token("%city",5))
			{
				ci.dwFlag=CNF_CITY;
				if(get_ci)
					ci.pszVal="";
				set_pszinfo;
				token_len=5;
				break_remover;
			}

			else if(search_token("%age",4))
			{
				ci.dwFlag=CNF_AGE;
				get_ci;
				itoa(ci.bVal,sub_str,10);
				token_len=4;
				if(sub_str[0]=='0')
					remove_breaks(sub_str,&loop,&pos,token_len);
			}

			else if(search_token("%gender",7))
			{
				ci.dwFlag=CNF_GENDER;
				get_ci;
				get_sub(ci.bVal?(ci.bVal=='M'?Translate("Male"):Translate("Female")):"");
				token_len=7;
				break_remover;
			}

			else if(search_token("%country",8))
			{
				ci.dwFlag=CNF_COUNTRY;
				if(get_ci)
					ci.pszVal="";
				set_pszinfo;
				token_len=8;
				break_remover;
			}

			else if(search_token("%mail",5))
			{
				ci.dwFlag=CNF_EMAIL;
				if(get_ci)
					ci.pszVal="";
				set_pszinfo;
				token_len=5;
				break_remover;
			}

			else if(search_token("%phone",6))
			{
				ci.dwFlag=CNF_PHONE;
				if(get_ci)
					ci.pszVal="";
				set_pszinfo;
				token_len=6;
				break_remover;
			}

			else if(search_token("%cell",5))
			{
				get_sub(proto_setting("Cellular"));
				clean_up;
				token_len=5;
				break_remover;
			}

			else if(search_token("%iname",6))
			{
				ci.dwFlag=CNF_NICK;
				if(get_ci)
					ci.pszVal="";
				set_pszinfo;
				token_len=6;
				break_remover;
			}

			else if(search_token("%lang1",6))
			{
				get_sub(proto_setting("Language1"));
				clean_up;
				token_len=6;
				break_remover;
			}

			else if(search_token("%lang2",6))
			{
				get_sub(proto_setting("Language2"));
				clean_up;
				token_len=6;
				break_remover;
			}

			else if(search_token("%lang3",6))
			{
				get_sub(proto_setting("Language3"));
				clean_up;
				token_len=6;
				break_remover;
			}

			else if(search_token("%ver",4))
			{
				int ver=DBGetContactSettingWord(hItem,szProto,"Version",0);
				static char *szVersionDescr[]={"", "ICQ 1.x", "ICQ 2.x", "Unknown", "ICQ98", "Unknown", "ICQ99 / licq", "ICQ2000", "ICQ2001-2003, Miranda or Trillian", "ICQ Lite"};
				wsprintf(sub_str,"%d: %s",ver,ver>sizeof(szVersionDescr)/sizeof(szVersionDescr[0])?Translate("Unknown"):Translate(szVersionDescr[ver]));
				token_len=4;
			}

			else if(search_token("%mver",5))
			{
				get_sub(proto_setting("MirVer"));
				clean_up;
				token_len=5;
				break_remover;
			}

			else if(search_token("%id",3))
			{
				ci.dwFlag=CNF_UNIQUEID;
				if(get_ci)
					ci.pszVal="";

				switch(ci.type){

					case CNFT_BYTE:
						ltoa(ci.bVal,sub_str,10);
						break;
					case CNFT_WORD:
						ltoa(ci.wVal,sub_str,10);
						break;
					case CNFT_DWORD:
						ltoa(ci.dVal,sub_str,10);
						break;
					case CNFT_ASCIIZ:
						set_pszinfo;
						break;
				}

				token_len=3;
				break_remover;
			}

			else if(search_token("%ip",3))
			{
				struct in_addr ia;
				DWORD ip=DBGetContactSettingDword(hItem,szProto,"IP",0);
				if(ip)
				{
					ia.S_un.S_addr=htonl(ip);
					get_sub(inet_ntoa(ia));
				}
				else
					get_sub("");
				token_len=3;
				break_remover;
			}

			else if(search_token("%realip",7))
			{
				struct in_addr ia;
				DWORD ip=DBGetContactSettingDword(hItem,szProto,"RealIP",0);
				if(ip)
				{
					ia.S_un.S_addr=htonl(ip);
					get_sub(inet_ntoa(ia));
				}
				else
					get_sub("");
				token_len=7;
				break_remover;
			}

			else if(search_token("%bday",5))
			{
				int bday=DBGetContactSettingByte(hItem,szProto,"BirthDay",0);
				if(bday)
					itoa(bday,sub_str,10);
				else
					get_sub("");
				token_len=5;
				break_remover;
			}

			else if(search_token("%byear",6))
			{
				int bday=DBGetContactSettingWord(hItem,szProto,"BirthYear",0);
				if(bday)
					itoa(bday,sub_str,10);
				else
					get_sub("");
				token_len=6;
				break_remover;
			}

			else if(search_token("%bmonth",7))
			{
				int bmonth=DBGetContactSettingByte(hItem,szProto,"BirthMonth",0);
				if(bmonth)
					GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SABBREVMONTHNAME1+bmonth-1,sub_str,sizeof sub_str);
				else
					get_sub("");
				token_len=7;
				break_remover;
			}

			else if(search_token("%status",7))
			{
				char *sts=(char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)DBGetContactSettingWord(hItem,szProto,"Status",0),0);
				if(sts)
					get_sub(sts);
				token_len=7;
				break_remover;
			}

			else if(search_token("%smsg",5))
			{
				if(CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_1,0)&PF1_MODEMSGRECV)
				{
					if(CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_3,0)&StatusModeToProtoFlag(status))
					{
						hProcess=(HANDLE)CallContactService(hItem,PSS_GETAWAYMSG,0,0);
						nTimer=SetTimer(NULL,0,2000,TimerProc);
						get_sub(Translate("Retrieving mode message..."));
						mustreplace=TRUE;
						replace_pos=loop;
					}
				}
				token_len=5;
				break_remover;
			}

			else if(search_token("%tab",4))
			{
				get_sub("\t");
				token_len=4;
			}

			else if(search_token("%notes",6))
			{
				get_sub(DBGetContactSetting(hItem,"UserInfo","MyNotes",&dbv)?"":dbv.pszVal);
				clean_up;
				token_len=6;
				break_remover;
			}

			else continue;

			if(lstrlen(sub_str)>token_len)
				template_str=(char *)realloc(template_str,lstrlen(template_str)+1+lstrlen(sub_str)-token_len);
			MoveMemory(template_str+pos+lstrlen(sub_str),template_str+pos+token_len,lstrlen(template_str)+1-pos-token_len);
			if(sub_str)
				CopyMemory(template_str+pos,sub_str,lstrlen(sub_str));
		}

		free(szProto);

		if(!template_str[0])
			return Translate("No information available");

		return template_str;
	}

	else
	{
		DBVARIANT dbv;
		char szItem[4]="";

		if(!DBGetContactSettingByte(NULL,TIP_MOD,"Group",0))
			return "";

		wsprintf(szItem,"%i",(int)hItem-1);

		DBGetContactSetting(NULL,"CListGroups",szItem,&dbv);
		return (char *)CallService(MS_CLIST_GROUPGETNAME,(WPARAM)hItem,(LPARAM)NULL);
	}
}


int ShowTip(WPARAM wparam,LPARAM lparam)
{
	TOOLINFO ti={0};
	CLCINFOTIP *pcit=(CLCINFOTIP *)lparam;

	if(!pcit->isTreeFocused && !DBGetContactSettingByte(NULL,TIP_MOD,"Focused",1))
		return 0;

	ti.cbSize=sizeof(TOOLINFO);
	ti.hwnd=NULL;
	ti.uId=TIP_ID;
	
	ti.lpszText=GetTipText(pcit->hItem,pcit->isGroup,pcit->isTreeFocused,WindowFromPoint(pcit->ptCursor));

/* Miranda 0.1.2.1 fix */
	GetCursorPos(&pcit->ptCursor);
/* ------------------- */
/* misplacement fix */	
	ti.rect=pcit->rcItem;
	ti.rect.left=ti.rect.right=pcit->ptCursor.x;
	SendMessage(htt,TTM_SETTOOLINFO,0,(LPARAM)&ti);
/* ---------------- */
	SendMessage(htt,TTM_TRACKPOSITION,0,(LPARAM)MAKELPARAM(pcit->ptCursor.x+10,pcit->ptCursor.y+20));
	SendMessage(htt,TTM_TRACKACTIVATE,1,(LPARAM)&ti);
	return 1;
}

int HideTip(WPARAM wparam,LPARAM lparam)
{
	TOOLINFO ti;

	ti.cbSize=sizeof(TOOLINFO);
	ti.hwnd=NULL;
	ti.uId=TIP_ID;

	KillTimer(NULL,nTimer);
	hProcess=NULL;

	SendMessage(htt,TTM_TRACKACTIVATE,0,(LPARAM)&ti);
	return 0;
}


int MainInit(WPARAM wparam,LPARAM lparam)
{
	TOOLINFO ti={0};

	htt=CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,"",TTS_NOPREFIX|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,hinstance,0);
	SendMessage(htt,TTM_ACTIVATE,1,0);
	HookEvent(ME_CLC_SHOWINFOTIP,ShowTip);
	HookEvent(ME_CLC_HIDEINFOTIP,HideTip);
	HookEvent(ME_OPT_INITIALISE,OptionsInit);

	ti.cbSize=sizeof(TOOLINFO);
	ti.uFlags=TTF_TRACK;
	ti.hwnd=NULL;
	ti.uId=TIP_ID;
	ti.hinst=hinstance;
	ti.lpszText="";
	SendMessage(htt,TTM_ADDTOOL,0,(LPARAM)&ti);
	ti.rect.top=3;
	ti.rect.left=ti.rect.bottom=ti.rect.right=5;
	SendMessage(htt,TTM_SETMAXTIPWIDTH,0,(LPARAM)200);
	SendMessage(htt,TTM_SETMARGIN,0,(LPARAM)&ti.rect);
	hevent=HookEvent(ME_PROTO_ACK,ProtoAck);
	return 0;
}



__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}


__declspec(dllexport)int Unload(void)
{
	EndDialog(htt,0);
	KillTimer(NULL,nTimer);
	UnhookEvent(hevent);
	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
	hinstance=hinst;
	return 1;
}


int __declspec(dllexport)Load(PLUGINLINK *link)
{
	pluginLink=link;
	// this isn't required for most events
	// but the ME_USERINFO_INITIALISE
	// I decided to hook all events after
	// everything is loaded because it seems
	// to be safer in my opinion
	HookEvent(ME_SYSTEM_MODULESLOADED,MainInit);
	return 0;
}