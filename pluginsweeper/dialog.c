#include "uninstall.h"
#include "..\..\miranda32\random\langpack\m_langpack.h"
#include "..\..\miranda32\ui\options\m_options.h"



extern HINSTANCE hinstance;
extern DWORD dwmirver;



WNDPROC MainProc;
static UINT controls[]={IDC_ADD,IDC_REMOVE};



int FillPluginList(HWND);
int FillSettingsBox(HWND,const char *);
int SweepPlugin(const char *,const char *);
int SweepContactSettings(HANDLE,const char *);



LRESULT CALLBACK RemoveDialogProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char szPlugin[256];
	switch(msg){

		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);
			SetWindowLong(hdlg,GWL_USERDATA,(LONG)lparam);

			if(FillPluginList(GetDlgItem(hdlg,IDC_PLUGINLIST)))
			{
				ListBox_AddString(GetDlgItem(hdlg,IDC_PLUGINLIST),Translate("No uninstallable plugins found"));
				EnableWindow(GetDlgItem(hdlg,IDC_PLUGINLIST),FALSE);
			}
			break;

		case WM_COMMAND:
			if(HIWORD(wparam)!=BN_CLICKED) break;

			if(LOWORD(wparam)==IDC_REMOVE)
			{
				ListBox_GetString(GetDlgItem(hdlg,IDC_PLUGINLIST),ListBox_GetSel(GetDlgItem(hdlg,IDC_PLUGINLIST)),szPlugin);

				if(MessageBox(hdlg,Translate("Do you really want to remove this plugin from the list?"),Translate("Plugin sweeper"),MB_YESNO|MB_ICONQUESTION)==IDYES)
				{
					DBDeleteContactSetting(NULL,"Uninstall",szPlugin);
					SendMessage((HWND)GetWindowLong(hdlg,GWL_USERDATA),UM_STARTUP,0,0);
				}
			}

			SendMessage(hdlg,WM_CLOSE,0,0);
			break;

		case WM_CLOSE:
			EndDialog(hdlg,0);
			break;
	}

	return 0;
}



LRESULT CALLBACK AddDialogProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char szPlugin[256],szModule[256];
	switch(msg){

		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);
			SetWindowLong(hdlg,GWL_USERDATA,(LONG)lparam);
			break;

		case WM_COMMAND:
			if(HIWORD(wparam)!=BN_CLICKED) break;

			if(LOWORD(wparam)==IDC_ADD)
			{
				GetDlgItemText(hdlg,IDC_PLUGINNAME,szPlugin,sizeof(szPlugin));
				if(!strcmp(szPlugin,"")) break;

				GetDlgItemText(hdlg,IDC_SZMODULE,szModule,sizeof(szModule));
				if(!strcmp(szModule,"")) break;

				DBWriteContactSettingString(NULL,"Uninstall",szPlugin,szModule);

				SendMessage((HWND)GetWindowLong(hdlg,GWL_USERDATA),UM_STARTUP,0,0);
			}

			SendMessage(hdlg,WM_CLOSE,0,0);
			break;

		case WM_CLOSE:
			EndDialog(hdlg,0);
			break;
	}

	return 0;
}



LRESULT CALLBACK TreeProc(HWND htree,UINT msg,WPARAM wparam,LPARAM lparam)
{
	TVITEM tvi;

	switch(msg){
		case WM_KILLFOCUS:
			if((HWND)wparam==GetDlgItem(GetParent(htree),IDC_REMOVESELECTED)) break;
			
			EnableWindow(GetDlgItem(GetParent(htree),IDC_REMOVESELECTED),FALSE);
			break;

		case WM_SETFOCUS:
			tvi.mask=TVIF_CHILDREN;
			tvi.hItem=TreeView_GetSelection(htree);
			TreeView_GetItem(htree,&tvi);
							
			EnableWindow(GetDlgItem(GetParent(htree),IDC_REMOVESELECTED),tvi.cChildren?TRUE:FALSE);
			break;
	}

	return CallWindowProc(MainProc,htree,msg,wparam,lparam);
}



LRESULT CALLBACK DialogProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char szPlugin[256];
	DBVARIANT dbv;
	TVITEM tvi;

	switch(msg){

		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);
			
			SendMessage(hdlg,UM_STARTUP,0,0);
			MainProc=(WNDPROC)SetWindowLong(GetDlgItem(hdlg,IDC_RMSETTINGS),GWL_WNDPROC,(LONG)(WNDPROC)TreeProc);
			break;

		case WM_COMMAND:
			switch(HIWORD(wparam)){
				
			case LBN_SELCHANGE:
				switch(LOWORD(wparam)){

					case IDC_PLUGINLIST:
						SendDlgItemMessage(hdlg,IDC_RMSETTINGS,TVM_DELETEITEM,0,(LPARAM)TVI_ROOT);
						ListBox_GetString((HWND)lparam,ListBox_GetSel((HWND)lparam),szPlugin);
						DBGetContactSetting(NULL,"Uninstall",szPlugin,&dbv);

						if(dbv.type!=DBVT_ASCIIZ)
							break;

						if(FillSettingsBox(GetDlgItem(hdlg,IDC_RMSETTINGS),dbv.pszVal))
						{
							EnableWindow(GetDlgItem(hdlg,IDC_REMOVEALL),TRUE);
							EnableWindow(GetDlgItem(hdlg,IDC_RMSETTINGS),TRUE);
						}

						else
						{
							EnableWindow(GetDlgItem(hdlg,IDC_RMSETTINGS),FALSE);
							EnableWindow(GetDlgItem(hdlg,IDC_REMOVEALL),FALSE);
						}

						EnableWindow(GetDlgItem(hdlg,IDC_REMOVESELECTED),FALSE);
						break;
				}

				break;

			case BN_CLICKED:
				switch(LOWORD(wparam)){

					case IDC_REMOVEALL:
						if(MessageBox(hdlg,Translate("Are you sure you want to remove all DB settings of this plugin?"),Translate("Plugin sweeper"),MB_YESNO|MB_ICONQUESTION)==IDYES)
						{
							ListBox_GetString(GetDlgItem(hdlg,IDC_PLUGINLIST),ListBox_GetSel(GetDlgItem(hdlg,IDC_PLUGINLIST)),szPlugin);
							DBGetContactSetting(NULL,"Uninstall",szPlugin,&dbv);

							SweepPlugin(dbv.pszVal,szPlugin);
							SendMessage(hdlg,UM_STARTUP,0,0);
							EnableWindow(GetDlgItem(hdlg,IDC_REMOVEALL),FALSE);
							EnableWindow(GetDlgItem(hdlg,IDC_REMOVESELECTED),FALSE);
						}
						break;

					case IDC_REMOVESELECTED:
						if(MessageBox(hdlg,Translate("Are you sure you want to remove all DB settings of this plugin from the selected contact?"),Translate("Plugin sweeper"),MB_YESNO|MB_ICONQUESTION)==IDYES)
						{
							tvi.mask=TVIF_PARAM;
							tvi.hItem=TreeView_GetSelection(GetDlgItem(hdlg,IDC_RMSETTINGS));
							TreeView_GetItem(GetDlgItem(hdlg,IDC_RMSETTINGS),&tvi);

							ListBox_GetString(GetDlgItem(hdlg,IDC_PLUGINLIST),ListBox_GetSel(GetDlgItem(hdlg,IDC_PLUGINLIST)),szPlugin);
							DBGetContactSetting(NULL,"Uninstall",szPlugin,&dbv);
								
							SweepContactSettings((HANDLE)tvi.lParam,dbv.pszVal);

							SendMessage(hdlg,UM_STARTUP,0,0);
							EnableWindow(GetDlgItem(hdlg,IDC_REMOVEALL),FALSE);
							EnableWindow(GetDlgItem(hdlg,IDC_REMOVESELECTED),FALSE);
						}
						break;

					case IDC_ADD:
						DialogBoxParam(hinstance,MAKEINTRESOURCE(IDD_ADD),hdlg,AddDialogProc,(LPARAM)hdlg);
						break;

					case IDC_REMOVE:
						DialogBoxParam(hinstance,MAKEINTRESOURCE(IDD_REMOVE),hdlg,RemoveDialogProc,(LPARAM)hdlg);
						break;
				}
				break;
			}

			break;

		case UM_STARTUP:
			SendDlgItemMessage(hdlg,IDC_PLUGINLIST,LB_RESETCONTENT,0,0);
			SendDlgItemMessage(hdlg,IDC_RMSETTINGS,TVM_DELETEITEM,0,(LPARAM)TVI_ROOT);
			if(FillPluginList(GetDlgItem(hdlg,IDC_PLUGINLIST)))
			{
				ListBox_AddString(GetDlgItem(hdlg,IDC_PLUGINLIST),Translate("No uninstallable plugins found"));
				EnableWindow(GetDlgItem(hdlg,IDC_PLUGINLIST),FALSE);
			}
			break;

		case WM_NOTIFY:
			switch(wparam)
			{
				case IDC_RMSETTINGS:
					switch(((LPNMHDR)lparam)->code)
					{
						case TVN_SELCHANGED:
							tvi.mask=TVIF_CHILDREN;
							tvi.hItem=TreeView_GetSelection(GetDlgItem(hdlg,IDC_RMSETTINGS));
							TreeView_GetItem(GetDlgItem(hdlg,IDC_RMSETTINGS),&tvi);
							
							EnableWindow(GetDlgItem(hdlg,IDC_REMOVESELECTED),tvi.cChildren?TRUE:FALSE);
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



int StartDialog(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE odp={0};
	struct {
		int cbSize;
		int position;
		char *pszTitle;
		DLGPROC pfnDlgProc;
		char *pszTemplate;
		HINSTANCE hInstance;
		HICON hIcon;
		char *pszGroup;
		int groupPosition;
		HICON hGroupIcon;
	}odpo={0};
	BYTE bversion;

	bversion=(BYTE)(dwmirver&0xFF);

	if(bversion)
		odp.cbSize=sizeof(odp);
	else
		odpo.cbSize=sizeof(odpo);
	odp.expertOnlyControls=controls;
	odp.nExpertOnlyControls=2;
	odp.flags=ODPF_BOLDGROUPS;
	odp.hInstance=odpo.hInstance=hinstance;
	odp.pfnDlgProc=odpo.pfnDlgProc=DialogProc;
	odp.pszTitle=odpo.pszTitle=Translate("Uninstaller");
	odp.pszTemplate=odpo.pszTemplate=MAKEINTRESOURCE(IDD_MAIN);
	odp.position=odpo.position=910000000;
	odp.pszGroup=odpo.pszGroup=Translate("Plugins");
	CallService(MS_OPT_ADDPAGE,wparam,bversion?(LPARAM)&odp:(LPARAM)&odpo);

	
	return 0;
}