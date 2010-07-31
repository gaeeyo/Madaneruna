// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlfile.h>
#include <atltime.h>
#include <locale.h>
#include <atlcoll.h>
#include <atlframe.h>

class CMainDlg : public CDialogImpl<CMainDlg>, public CDialogResize<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_DLGRESIZE_MAP(CMainDlg)
		DLGRESIZE_CONTROL(IDC_TEST, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_WINDOWTITLE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_LOG, DLSZ_SIZE_Y | DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_POWERBROADCAST, OnPowerBroadcast)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_TEST, OnTest)
		CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	CListBox	m_Log;


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		setlocale( LC_ALL, "" ); 

		CString	conditionFilePath = GetOptionFilePath(_T(".txt"));
		if(!PathFileExists(conditionFilePath)) {
			// サンプルテキストを出力");
			SaveTextFile(conditionFilePath, _T("出力中*残り*\r\n検索中*残り*"));
		}

		CString	condition = LoadTextFile(conditionFilePath);
		
		SetDlgItemText(IDC_WINDOWTITLE, condition);

		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

		m_Log.Attach(GetDlgItem(IDC_LOG));
		
		DlgResize_Init(FALSE);

		return TRUE;
	}


	CString GetOptionFilePath(LPCTSTR ext) {
		// iniファイルのパス
		TCHAR	exe[MAX_PATH+1] = {0};
		GetModuleFileName(_AtlBaseModule.GetModuleInstance(), exe, MAX_PATH);
		PathRenameExtension(exe, ext);
		return exe;
	}

	CString LoadTextFile(LPCTSTR path) {
		CString		text;
		CAtlFile	f;
		if(SUCCEEDED(f.Create(path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))) {
			ULONGLONG	llsize;
			f.GetSize(llsize);
			int size = static_cast<int>(llsize);
			if(size > 0) {
				BYTE	*buf = new BYTE [size+1];
				f.Read(buf, size);
				buf[size] = 0;
				text = buf;
				delete buf;
			}
		}
		return text;
	}

	void SaveTextFile(LPCTSTR path, LPCTSTR t) {
		CStringA	a = t;
		CAtlFile	f;

		if(SUCCEEDED(f.Create(path, GENERIC_WRITE, FILE_SHARE_READ, CREATE_NEW))) {
			f.Write(a, a.GetLength());
		}
	}

	class CCheckWindowsProcData {
	public:
		BOOL					bMatch;
		CString					message;
		CSimpleArray<CString>	titles;

		CCheckWindowsProcData() : bMatch(FALSE) {
		}
	};

	static BOOL CALLBACK CheckWindowProc(HWND hWnd, LPARAM lParam) {
		CCheckWindowsProcData	*pData = (CCheckWindowsProcData*)lParam;

		CWindow		wnd(hWnd);
		CString		title;
		wnd.GetWindowText(title);
		if(!title.IsEmpty()) {
			for(int j=0; j<pData->titles.GetSize(); j++) {
				if(PathMatchSpec(title, pData->titles[j])) {
					pData->message.Format(_T("寝なせない理由 %s == %s"), pData->titles[j], title);
					pData->bMatch = TRUE;
					return FALSE;
				}
			}
		}
		return TRUE;
	}

	// 寝ても良いときに
	BOOL IsSleepOK(CString &message) {
		message.Empty();

		if(IsDlgButtonChecked(IDC_DONTSLEEP) == BST_CHECKED) {	// 「寝ない」にチェックが入ってる
			GetDlgItemText(IDC_DONTSLEEP, message);
			return FALSE;
		}

		CCheckWindowsProcData	data;

		{
			CString titles;
			GetDlgItemText(IDC_WINDOWTITLE, titles);
			titles.Replace(_T("\r\n"), _T("\n"));

			int	pos = 0;
			CString	token;
			while(!(token = titles.Tokenize(_T("\n"), pos)).IsEmpty()) {
				data.titles.Add(token);
			}
		}
		EnumWindows(CheckWindowProc, (LPARAM)&data);
		if(data.bMatch) {
			message = data.message;
			return FALSE;
		}

		message = _T("寝てよし");
		return TRUE;
	}


	void SetStatusText(LPCTSTR message, BOOL send)
	{
		CString		line;
		CTime		t = CTime::GetCurrentTime();

		line.Format(_T("%s %s%s"), t.Format(_T("%c")),
				(send ? _T(" >> ") : _T("")),
				message);
		int idx = m_Log.InsertString(-1, line);
		m_Log.SetCurSel(idx);
		m_Log.SetTopIndex(idx);
	}

	LRESULT OnPowerBroadcast(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		switch(wParam) {
		case PBT_APMQUERYSUSPEND:	// サスペンドの問い合わせ
			{
				SetStatusText(_T("寝て良い？ (PBT_APMQUERYSUSPEND)"), FALSE);
				CString	message;
				if(IsSleepOK(message)) {
					// スリープOK
					SetStatusText(message, TRUE);
				}
				else {
					// スリープしない
					SetStatusText(message, TRUE);

					bHandled = TRUE;
					return BROADCAST_QUERY_DENY;
				}
			}
			break;
		case PBT_APMPOWERSTATUSCHANGE:
			SetStatusText(_T("PBT_APMPOWERSTATUSCHANGE"), FALSE);
			break;
		case PBT_APMRESUMEAUTOMATIC:
			SetStatusText(_T("復帰した (PBT_APMRESUMEAUTOMATIC)"), FALSE);
			SetStatusText(_T("ディスプレイつけた SetThreadExecutionState(ES_DISPLAY_REQUIRED)"), TRUE);
			SetThreadExecutionState(ES_DISPLAY_REQUIRED);

			break;
		case PBT_APMSUSPEND:
			SetStatusText(_T("サスペンド入った (PBT_APMSUSPEND)"), FALSE);
			break;
		case 0x8013: //PBT_POWERSETTINGCHANGE:
			SetStatusText(_T("設定変わった (PBT_POWERSETTINGCHANGE)"), FALSE);
			break;
		case PBT_APMBATTERYLOW:
			SetStatusText(_T("バッテリ無い (PBT_APMBATTERYLOW)"), FALSE);
			break;
		case PBT_APMOEMEVENT:
			SetStatusText(_T("PBT_APMOEMEVENT"), FALSE);
			break;
		case PBT_APMQUERYSUSPENDFAILED:
			SetStatusText(_T("サスペンド入れんかった (PBT_APMQUERYSUSPENDFAILED)"), FALSE);
			break;
		case PBT_APMRESUMECRITICAL:
			SetStatusText(_T("PBT_APMRESUMECRITICAL"), FALSE);
			break;
		}
		return 0;
	}

	LRESULT OnTest(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString	message;
		IsSleepOK(message);

		message.Insert(0, _T("テスト: "));
		SetStatusText(message, TRUE);

		SetStatusText(_T("ずっとBUSY SetThreadExecutionState(ES_SYSTEM_REQUIRED|ES_CONTINUOUS)"), TRUE);
		SetThreadExecutionState(ES_SYSTEM_REQUIRED|ES_CONTINUOUS);

		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		LPCTSTR title = _T("終了確認");
		LPCTSTR message = _T("終了しますか?");
		if(MessageBox(message, title, MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
			EndDialog(wID);
		}
		return 0;
	}
};
