#include "seen.h"
#include "..\..\miranda32\ui\contactlist\m_clist.h"
#include "..\..\miranda32\protocols\protocols\m_protocols.h"
#include "..\..\miranda32\protocols\protocols\m_protosvc.h"



void FileWrite(HANDLE);
void SetOffline(void);



char *ParseString(char *szstring,HANDLE hcontact,BYTE isfile)
{
	char sztemp[1024]="",szdbsetting[128]="";
	UINT loop=0;
	DBVARIANT dbv;
	int isetting=0;
	DWORD dwsetting=0;
	struct in_addr ia;
	char *weekdays[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
	char *wdays_short[]={"Sun.","Mon.","Tue.","Wed.","Thu.","Fri.","Sat."};
	char *monthnames[]={"January","February","March","April","May","June","July","August","September","October","November","December"};
	char *mnames_short[]={"Jan.","Feb.","Mar.","Apr.","May","Jun.","Jul.","Aug.","Sep.","Oct.","Nov.","Dec."};
	
	for(;loop<strlen(szstring);loop++)
	{
		if(szstring[loop]!='%')
		{
			strncat(sztemp,szstring+loop,1);
			continue;
		}

		else
		{
			switch(szstring[++loop]){
				case 'Y':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Year",0)))
						return Translate("<unknown>");
					wsprintf(szdbsetting,"%04i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'y':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Year",0)))
						return Translate("<unknown>");
					wsprintf(szdbsetting,"%04i",isetting);
					strcat(sztemp,szdbsetting+2);
					break;

				case 'm':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0)))
						return Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'd':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Day",0)))
						return Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'W':
					isetting=DBGetContactSettingWord(hcontact,S_MOD,"WeekDay",-1);
					if(isetting==-1)
						break;
					strcat(sztemp,Translate(weekdays[isetting]));
					break;

				case 'w':
					isetting=DBGetContactSettingWord(hcontact,S_MOD,"WeekDay",-1);
					if(isetting==-1)
						break;
					strcat(sztemp,Translate(wdays_short[isetting]));
					break;

				case 'E':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0)))
						return Translate("<unknown>");
					strcat(sztemp,Translate(monthnames[isetting-1]));
					break;

				case 'e':
					if(!(isetting=DBGetContactSettingWord(hcontact,S_MOD,"Month",0)))
						return Translate("<unknown>");
					strcat(sztemp,Translate(mnames_short[isetting-1]));
					break;

				case 'H':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)
						return Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'h':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)
						return Translate("<unknown>");

					if(!isetting) isetting=12;
					
					wsprintf(szdbsetting,"%i",(isetting-((isetting>12)?12:0)));
					strcat(sztemp,szdbsetting);
					break;

				case 'p':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Hours",-1))==-1)
						return Translate("<unknown>");
					if(isetting>12)
						strcat(sztemp,"PM");
					else strcat(sztemp,"AM");
					break;

				case 'M':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Minutes",-1))==-1)
						return Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'S':
					if((isetting=DBGetContactSettingWord(hcontact,S_MOD,"Seconds",-1))==-1)
						return Translate("<unknown>");
					wsprintf(szdbsetting,"%02i",isetting);
					strcat(sztemp,szdbsetting);
					break;

				case 'n':
					strcat(sztemp,(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hcontact,0));
					break;

				case 'u':
/*					dwsetting=DBGetContactSettingDword(hcontact,"ICQ","UIN",0);
					if(!dwsetting)
					{
						DBGetContactSetting(hcontact,"MSN","e-mail",&dbv);
						strcat(sztemp,dbv.pszVal);
					}

					else
					{
						wsprintf(szdbsetting,"%i",dwsetting);
						strcat(sztemp,szdbsetting);
					}*/

					// YAHOO: e-mail
/*					szproto=strdup(CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0));
					
					if((dwsetting=DBGetContactSettingDword(hcontact,"ICQ","UIN",0))
						wsprintf(szdbsetting,"%i",dwsetting);

					else if((dwsetting=DBGetContactSettingDword(hcontact,"EM_LAN_PROTO","ipaddr",0)))
						wsprintf(szdbsetting,"%i",dwsetting);
					
					else if((dwsetting=DBGetContactSettingDword(hcontact,"EM_LAN_PROTO","ipaddr",0)))
						wsprintf(szdbsetting,"%i",dwsetting);

					else if((dwsetting=DBGetContactSettingDword(hcontact,"GG","UIN",0)))
						wsprintf(szdbsetting,"%i",dwsetting);
*/
					PFLAG_UNIQUEIDTEXT

					break;

				case 's':
					isetting=DBGetContactSettingWord(hcontact,S_MOD,"Status",ID_STATUS_OFFLINE);
					strcpy(szdbsetting,(const char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)isetting,0));
					strcat(sztemp,Translate(szdbsetting));
					break;

				case 'i':
				case 'r':
					dwsetting=DBGetContactSettingDword(hcontact,S_MOD,szstring[loop]=='i'?"IP":"RealIP",0);
					if(!dwsetting)
						strcat(sztemp,Translate("<unknown>"));
					else
					{
						ia.S_un.S_addr=htonl(dwsetting);
						strcat(sztemp,inet_ntoa(ia));
					}
					break;

				case 'b':
					strcat(sztemp,/*"\n"*/"\x0D\x0A");
					break;

				case 't':
					strcat(sztemp,"\t");
					break;

				default:
					strncat(sztemp,szstring+loop-1,2);
					break;
			}
		}
	}
	DBFreeVariant(&dbv);
	return &sztemp[0];
}



void DBWriteTime(SYSTEMTIME *st,HANDLE hcontact)
{
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Day",st->wDay);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Month",st->wMonth);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Year",st->wYear);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Hours",st->wHour);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Minutes",st->wMinute);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"Seconds",st->wSecond);
	DBWriteContactSettingWord((HANDLE)hcontact,S_MOD,"WeekDay",st->wDayOfWeek);

}



int UpdateValues(WPARAM wparam,LPARAM lparam)
{
	DBCONTACTWRITESETTING *cws;
	SYSTEMTIME time;

	cws=(DBCONTACTWRITESETTING *)lparam;

	if(strcmp(cws->szSetting,"Status") || (strcmp(cws->szModule,"ICQ") && strcmpi(cws->szModule,"MSN") && strcmpi(cws->szModule,"YAHOO") && strcmp(cws->szModule,"Tlen") && strcmp(cws->szModule,"GG") && strcmp(cws->szModule,"Jabber") && strcmp(cws->szModule,"EM_LAN_PROTO")) || (HANDLE)wparam==NULL) return 0;

	if(cws->value.wVal==ID_STATUS_OFFLINE)
	{
		DBWriteContactSettingByte((HANDLE)wparam,S_MOD,"Offline",1);
		GetLocalTime(&time);
		DBWriteTime(&time,(HANDLE)wparam);
		if(!DBGetContactSettingByte(NULL,S_MOD,"IgnoreOffline",1))
		{
			DBWriteContactSettingWord((HANDLE)wparam,S_MOD,"Status",ID_STATUS_OFFLINE);

			if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))
				FileWrite((HANDLE)wparam);
		}
		return 0;
	}
	if(cws->value.wVal==DBGetContactSettingWord((HANDLE)wparam,S_MOD,"Status",ID_STATUS_OFFLINE) && !DBGetContactSettingByte((HANDLE)wparam,S_MOD,"Offline",0)) return 0;

	GetLocalTime(&time);

	DBWriteTime(&time,(HANDLE)wparam);

	DBWriteContactSettingWord((HANDLE)wparam,S_MOD,"Status",(WORD)cws->value.wVal);

	if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))
		FileWrite((HANDLE)wparam);

//	CallContactService((HANDLE)wparam,PSS_GETINFO,0,0);

	DBWriteContactSettingByte((HANDLE)wparam,S_MOD,"Offline",0);

	return 0;
}



int ModeChange(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *ack;
	int isetting=0;
	SYSTEMTIME time;

	ack=(ACKDATA *)lparam;

	if(ack->type!=ACKTYPE_STATUS || ack->result!=ACKRESULT_SUCCESS || ack->hContact!=NULL) return 0;
	
	GetLocalTime(&time);

	DBWriteTime(&time,NULL);

	isetting=CallProtoService(ack->szModule,PS_GETSTATUS,0,0);
	DBWriteContactSettingWord(NULL,S_MOD,"Status",(WORD)isetting);

	if(DBGetContactSettingByte(NULL,S_MOD,"FileOutput",0))
		FileWrite(NULL);

	if(isetting==ID_STATUS_OFFLINE)
		SetOffline();

	return 0;
}



int GetInfoAck(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *ack;
	DWORD dwsetting=0;

	ack=(ACKDATA *)lparam;

	if(ack->type!=ACKTYPE_GETINFO || ack->hContact==NULL) return 0;
	if(((int)ack->hProcess-1)!=(int)ack->lParam) return 0;
	
	dwsetting=DBGetContactSettingDword(ack->hContact,ack->szModule,"IP",0);
	if(dwsetting)
		DBWriteContactSettingDword(ack->hContact,S_MOD,"IP",dwsetting);

	dwsetting=DBGetContactSettingDword(ack->hContact,ack->szModule,"RealIP",0);
	if(dwsetting)
		DBWriteContactSettingDword(ack->hContact,S_MOD,"RealIP",dwsetting);

	return 0;
}



void SetOffline(void)
{
	HANDLE hcontact=NULL;

	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hcontact!=NULL)
	{
		DBWriteContactSettingByte(hcontact,S_MOD,"Offline",1);
		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}
}