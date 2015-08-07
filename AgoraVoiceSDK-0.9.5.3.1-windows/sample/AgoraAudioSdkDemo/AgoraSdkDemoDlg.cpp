
// AgoraSdkDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AgoraSdkDemo.h"
#include "AgoraSdkDemoDlg.h"
#include <AgoraAudioSDK.h>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_AGORA_LOAD_ENGINE_SUCCESS WM_USER+1
#define WM_AGORA_JOIN_SUCCESS	WM_USER+2
#define WM_AGORA_ERROR			WM_USER+3
#define WM_AGORA_LOG_EVENT		WM_USER+4
#define WM_AGORA_QUALITY		WM_USER+5

class AgoraAudioEventHandler : public IAgoraAudioEventHandler
{
private:
	CWnd* m_pWnd;
	std::set<std::string*> m_lstPendingMessages;
	CRITICAL_SECTION m_cs;
public:
	AgoraAudioEventHandler(CWnd* wnd)
		:m_pWnd(wnd)
	{
		::InitializeCriticalSection(&m_cs);
	}
	~AgoraAudioEventHandler()
	{
		::DeleteCriticalSection(&m_cs);
		std::set<std::string*>::iterator it = m_lstPendingMessages.begin();
		for (; it != m_lstPendingMessages.end(); ++it)
			delete *it;
	}
	std::string* insertMessage(const char* msg)
	{
		if (!msg)
			return NULL;
		std::string* str = new std::string(msg);
		::EnterCriticalSection(&m_cs);
		m_lstPendingMessages.insert(str);
		::LeaveCriticalSection(&m_cs);
		return str;
	}
	void removeMessage(std::string* msg)
	{
		::EnterCriticalSection(&m_cs);
		m_lstPendingMessages.erase(msg);
		::LeaveCriticalSection(&m_cs);
		delete msg;
	}

	virtual void onLoadAudioEngineSuccess()
	{
		// dont block the callback thread
		if (!m_pWnd->GetSafeHwnd())
			return;
		m_pWnd->PostMessage(WM_AGORA_LOAD_ENGINE_SUCCESS, 0, 0);
	}
    virtual void onGetAudioSvrAddrSuccess()
	{
	}
    virtual void onJoinSuccess(unsigned int sid, unsigned int uid, int elapsed)
	{
		char buf[128];
		sprintf(buf, "Joined channel: sid %u, uid %u, elapsed %d ms", sid, uid, elapsed);
		onLogEvent(buf);
	}
    virtual void onRejoinSuccess(unsigned int sid, unsigned int uid, int elapsed)
	{
		char buf[128];
		sprintf(buf, "Rejoined channel: sid %u, uid %u, elapsed %d ms", sid, uid, elapsed);
		onLogEvent(buf);
	}
    virtual void onError(int rescode, const char* msg)
	{
		// dont block the callback thread
		if (!m_pWnd->GetSafeHwnd())
			return;
		m_pWnd->PostMessage(WM_AGORA_ERROR, rescode, (LPARAM)insertMessage(msg));
	}
    virtual void onLogEvent(const char* msg)
	{
		// dont block the callback thread
		if (!m_pWnd->GetSafeHwnd())
			return;
		m_pWnd->PostMessage(WM_AGORA_LOG_EVENT, 0, (LPARAM)insertMessage(msg));
	}
	static const char* getQualityDesc(int quality)
	{
		switch (quality)
		{
		case MEDIA_QUALITY_EXCELLENT:
			return "5";
		case MEDIA_QUALITY_GOOD:
			return "4";
		case MEDIA_QUALITY_POOR:
			return "3";
		case MEDIA_QUALITY_BAD:
			return "2";
		case MEDIA_QUALITY_VBAD:
			return "1";
		case MEDIA_QUALITY_UNKNOWN:
		default:
			return "unknown";
		}
	}
	virtual void onAudioQuality(unsigned int uid, int quality, unsigned short delay, unsigned short jitter, unsigned short lost, unsigned short lost2)
	{
		if (!m_pWnd->GetSafeHwnd())
			return;

		// dont block the callback thread
		char buf[256];
		sprintf(buf, "user %u quality %s delay %d jitter %d lost %d/%d", uid, getQualityDesc(quality), delay, jitter, lost, lost2);
		m_pWnd->PostMessage(WM_AGORA_QUALITY, 0, (LPARAM)insertMessage(buf));
	}
    virtual void onRecapStat(const char* recap_state, int length)
	{
	}
	virtual void onAudioEngineEvent(int evt)
	{
		switch (evt)
		{
		case AUDIO_ENGINE_RECORDING_ERROR:
			onLogEvent("Recording device error");
			break;
		case AUDIO_ENGINE_PLAYOUT_ERROR:
			onLogEvent("Playout device error");
			break;
		case AUDIO_ENGINE_RECORDING_WARNING:
			onLogEvent("Recording device warning");
			break;
		case AUDIO_ENGINE_PLAYOUT_WARNING:
			onLogEvent("Playout device warning");
			break;
		}
	}
	static const char* getDeviceType(int deviceType)
	{
		switch (deviceType)
		{
		case PLAYOUT_DEVICE:
			return "Playout";
		case RECORDING_DEVICE:
			return "Recording";
		default:
			return "Unkown";
		}
	}
	static const char* getDeviceState(int deviceState)
	{
		switch (deviceState)
		{
		case AUDIO_DEVICE_STATE_ACTIVE:
			return "active";
		case AUDIO_DEVICE_STATE_DISABLED:
			return "disabled";
		case AUDIO_DEVICE_STATE_NOT_PRESENT:
			return "not present";
		case AUDIO_DEVICE_STATE_UNPLUGGED:
			return "unplugged";
		default:
			return "";
		}
	}
	virtual void onAudioDeviceStateChanged(const char* deviceId, int deviceType, int deviceState)
	{
		char buf[256];
		sprintf(buf, "%s device %s: %s", getDeviceType(deviceType), getDeviceState(deviceState), deviceId);
		onLogEvent(buf);
	}
	virtual void onSpeakersReport(const SpeakerInfo* speakers, unsigned int speakerNumber, int mixVolume)
	{
	}
	virtual void onUpdateSessionStats(const SessionStat& stat)
	{
	}
	static std::string bytesToString(unsigned int bytes)
	{
		char buf[256];
		if (bytes >= 1024*1024)
			sprintf(buf, "%u.%u MB", bytes/(1024*1024), bytes%(1024*1024));
		else if (bytes >= 1024)
			sprintf(buf, "%u.%u KB", bytes/1024, bytes%1024);
		else
			sprintf(buf, "%u Bytes", bytes);
		return buf;
	}
	virtual void onLeaveChannel(const SessionStat& stat)
	{
		char buf[256];
		unsigned int total = stat.rxBytes+stat.txBytes;
		sprintf(buf, "call ended: duration %u secs, rx %s, tx %s, total %s, avg speed %s/sec",
			stat.duration,
			bytesToString(stat.rxBytes).c_str(),
			bytesToString(stat.txBytes).c_str(),
			bytesToString(total).c_str(),
			stat.duration?bytesToString((total/stat.duration)).c_str():"0"
			);
		onLogEvent(buf);
	}
    virtual void onNetworkQuality(int quality)
	{
	}
	virtual void onPeerConnected(int callSetupTime)
	{
		char buf[128];
		sprintf(buf, "peer connected, call setup time %d ms", callSetupTime);
		onLogEvent(buf);
	}
};

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAgoraSdkDemoDlg dialog




CAgoraSdkDemoDlg::CAgoraSdkDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAgoraSdkDemoDlg::IDD, pParent)
	, m_pAgoraAudio(NULL)
	, m_agoraEventHandler(new AgoraAudioEventHandler(this))
	, m_bMute(false)
	, m_strVendorKey(_T("6D7A26A1D3554A54A9F43BE6797FE3E2"))
	, m_strChannelName(_T(""))
	, m_nUserId(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CAgoraSdkDemoDlg::~CAgoraSdkDemoDlg()
{
	// Hwnd is not valid
	ReleaseAgoraAudio();
	if (m_agoraEventHandler)
	{
		delete m_agoraEventHandler;
		m_agoraEventHandler = NULL;
	}
	finalDestroyAgoraAudioInstance();
}

void CAgoraSdkDemoDlg::OnOK()
{
	ReleaseAgoraAudio();
	CDialog::OnOK();
}

void CAgoraSdkDemoDlg::OnCancel()
{
	ReleaseAgoraAudio();
	CDialog::OnCancel();
}

void CAgoraSdkDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_JOIN, m_btnJoin);
	DDX_Control(pDX, IDC_BTN_MUTE, m_btnMute);
	DDX_Text(pDX, IDC_EDIT_KEY, m_strVendorKey);
	DDX_Text(pDX, IDC_EDIT_CHANNEL, m_strChannelName);
	DDX_Text(pDX, IDC_EDIT_USER_ID, m_nUserId);
	DDX_Control(pDX, IDC_REDIT_LOG, m_editLog);
}

BEGIN_MESSAGE_MAP(CAgoraSdkDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_JOIN, &CAgoraSdkDemoDlg::OnBnClickedBtnJoin)
	ON_BN_CLICKED(IDC_BTN_MUTE, &CAgoraSdkDemoDlg::OnBnClickedBtnMute)
	ON_BN_CLICKED(IDOK, &CAgoraSdkDemoDlg::OnBnClickedOk)
	ON_MESSAGE(WM_AGORA_LOAD_ENGINE_SUCCESS, &CAgoraSdkDemoDlg::OnLoadEngineSuccess)
	ON_MESSAGE(WM_AGORA_JOIN_SUCCESS, &CAgoraSdkDemoDlg::OnJoinSuccess)
	ON_MESSAGE(WM_AGORA_ERROR, &CAgoraSdkDemoDlg::OnError)
	ON_MESSAGE(WM_AGORA_LOG_EVENT, &CAgoraSdkDemoDlg::OnLogEvent)
	ON_MESSAGE(WM_AGORA_QUALITY, &CAgoraSdkDemoDlg::OnQuality)
END_MESSAGE_MAP()


// CAgoraSdkDemoDlg message handlers

BOOL CAgoraSdkDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CWnd *pChannel = GetDlgItem(IDC_EDIT_CHANNEL);
	if (pChannel)
		pChannel->SetFocus();

	m_nUserId = 0;
	m_bMute = false;

	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CAgoraSdkDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAgoraSdkDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAgoraSdkDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAgoraSdkDemoDlg::OnBnClickedBtnJoin()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_strVendorKey.IsEmpty() || m_strChannelName.IsEmpty())
		return;

	if (!m_pAgoraAudio)
	{
		// Join channel
		m_btnJoin.SetWindowText(_T("Leave"));
		m_pAgoraAudio = createAgoraAudioInstance(m_agoraEventHandler);
		char workingDir[MAX_PATH];
		workingDir[0] = '\0';
		GetCurrentDirectoryA(MAX_PATH, workingDir);
		workingDir[MAX_PATH-1] = '\0';
		m_pAgoraAudio->initialize(NULL, workingDir, NULL);
		USES_CONVERSION;
		LPCSTR key = T2CA((LPCTSTR)m_strVendorKey);
		LPCSTR channelName = T2CA((LPCTSTR)m_strChannelName);
		m_pAgoraAudio->joinChannel(key, channelName, "", m_nUserId);
	}
	else
	{
		m_btnJoin.SetWindowText(_T("Join"));
		ReleaseAgoraAudio();
	}
}

void CAgoraSdkDemoDlg::OnBnClickedBtnMute()
{
	// TODO: Add your control notification handler code here
	if (!m_pAgoraAudio)
		return;
	if (m_bMute)
		m_btnMute.SetWindowText(_T("Mute"));
	else
		m_btnMute.SetWindowText(_T("Unmute"));
	m_bMute = !m_bMute;
	AgoraAudioParameters aap(m_pAgoraAudio);
	aap.mute(m_bMute);
}

void CAgoraSdkDemoDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	ReleaseAgoraAudio();
	OnOK();
}

void CAgoraSdkDemoDlg::ReleaseAgoraAudio()
{
	if (m_pAgoraAudio)
	{
		m_pAgoraAudio->leave();
		m_pAgoraAudio->release();
		m_pAgoraAudio = NULL;
	}
}

LRESULT CAgoraSdkDemoDlg::OnLoadEngineSuccess(WPARAM wParam, LPARAM lParam)
{
	AddLog("Agora audio engine loaded\n");
	return 0;
}

LRESULT CAgoraSdkDemoDlg::OnJoinSuccess(WPARAM wParam, LPARAM lParam)
{
	char buf[256];
	sprintf(buf, "Joined channel: sid %u uid %u\n", wParam, lParam);
	AddLog(buf);
	return 0;
}

LRESULT CAgoraSdkDemoDlg::OnError(WPARAM wParam, LPARAM lParam)
{
	std::string* msg = (std::string*)lParam;
	char buf[256];
	sprintf(buf, "Got error %d: %s\n", wParam, msg ? msg->c_str():"");
	AddLog(buf);
	return 0;
}

LRESULT CAgoraSdkDemoDlg::OnLogEvent(WPARAM wParam, LPARAM lParam)
{
	std::string* msg = (std::string*)lParam;
	if (msg)
	{
		AddLog(*msg+"\n");
		m_agoraEventHandler->removeMessage(msg);
	}
	return 0;
}

LRESULT CAgoraSdkDemoDlg::OnQuality(WPARAM wParam, LPARAM lParam)
{
	std::string* msg = (std::string*)lParam;
	if (msg)
	{
		AddLog(*msg+"\n");
		m_agoraEventHandler->removeMessage(msg);
	}
	return 0;
}

void CAgoraSdkDemoDlg::AddLog(const std::string& message)
{
	if (message.empty())
		return;

	int nLength = m_editLog.GetWindowTextLength();
   m_editLog.SetSel(nLength, nLength);
   USES_CONVERSION;
   LPCTSTR msg = A2CT(message.c_str());
   m_editLog.ReplaceSel(msg);
}
