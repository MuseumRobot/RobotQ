#include "stdafx.h"
#include "ASR_Recorder_Example.h"
#include "ASR_Recorder_ExampleDlg.h"
#include "hci_asr_recorder.h"
#include "common/CommonTool.h"
#include "common/AccountInfo.h"

#define WM_USER_SHOW_STATUS	WM_USER + 100

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD){
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX){
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CRecorder_ExampleDlg 对话框

CRecorder_ExampleDlg::CRecorder_ExampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecorder_ExampleDlg::IDD, pParent)
	, m_recordingFlag(FALSE){
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRecorder_ExampleDlg::DoDataExchange(CDataExchange* pDX){
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_BTN_SAVE_RECORDING, m_recordingFlag);
}

BEGIN_MESSAGE_MAP(CRecorder_ExampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_USER_SHOW_STATUS, &CRecorder_ExampleDlg::OnShowStatus) 
	ON_BN_CLICKED(IDC_BTN_START_RECORD, &CRecorder_ExampleDlg::OnBnClickedBtnStartRecord)
	ON_BN_CLICKED(IDC_BTN_CANCEL_RECORD, &CRecorder_ExampleDlg::OnBnClickedBtnCancelRecord)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &CRecorder_ExampleDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_BROWSER, &CRecorder_ExampleDlg::OnBnClickedBtnBrowser)
	ON_BN_CLICKED(IDC_BTN_SAVE_RECORDING, &CRecorder_ExampleDlg::OnBnClickedSaveRecording)
END_MESSAGE_MAP()

bool CheckAndUpdataAuth(){
    //获取过期时间
    int64 nExpireTime;
    int64 nCurTime = (int64)time( NULL );
    HCI_ERR_CODE errCode = hci_get_auth_expire_time( &nExpireTime );
    if( errCode == HCI_ERR_NONE ){
        //获取成功则判断是否过期
        if( nExpireTime > nCurTime ){
            //没有过期
            printf( "auth can use continue\n" );
            return true;
        }
    }

    //获取过期时间失败或已经过期
    //手动调用更新授权
    errCode = hci_check_auth();
    if( errCode == HCI_ERR_NONE ){
        //更新成功
        printf( "check auth success \n" );
        return true;
    }
    else{
        //更新失败
        printf( "check auth return (%d:%s)\n", errCode ,hci_get_error_info(errCode));
        return false;
    }
}

//获取capkey属性
void GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode){
    HCI_ERR_CODE errCode = HCI_ERR_NONE;
	CAPABILITY_ITEM *pItem = NULL;

	// 枚举所有的asr能力
	CAPABILITY_LIST list = {0};
	if ((errCode = hci_get_capability_list("asr", &list))!= HCI_ERR_NONE){
		// 没有找到相应的能力。
		return;
	}

	// 获取asr能力配置信息。
	for (int i = 0; i < list.uiItemCount; i++){
		if (list.pItemList[i].pszCapKey != NULL && stricmp(list.pItemList[i].pszCapKey, cap_key.c_str()) == 0){
			pItem = &list.pItemList[i];
			break;
		}
	}

	// 没有获取相应能力配置，返回。
	if (pItem == NULL || pItem->pszCapKey == NULL){
		hci_free_capability_list(&list);
		return;
	}

    
	if (strstr(pItem->pszCapKey, "cloud") != NULL){
		type = kRecogTypeCloud;
	}else{
		type = kRecogTypeLocal;
	}  

	if (strstr(pItem->pszCapKey, "freetalk") != NULL){
		mode = kRecogModeFreetalk;
	}else if (strstr(pItem->pszCapKey, "grammar") != NULL){
		mode = kRecogModeGrammar;
	}else{
		mode = kRecogModeUnkown;
	}

	hci_free_capability_list(&list);
       
    return;
};
// CRecorder_ExampleDlg 消息处理程序

BOOL CRecorder_ExampleDlg::OnInitDialog(){
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL){
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()){
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	((CButton *)GetDlgItem( IDC_CONTINUE ))->SetCheck(TRUE);
    if (Init() == false){
        return FALSE;
    }
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRecorder_ExampleDlg::OnSysCommand(UINT nID, LPARAM lParam){
	if ((nID & 0xFFF0) == IDM_ABOUTBOX){
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}else{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRecorder_ExampleDlg::OnPaint(){
	if (IsIconic()){
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}else{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRecorder_ExampleDlg::OnQueryDragIcon(){
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CRecorder_ExampleDlg::OnShowStatus( WPARAM wParam, LPARAM lParam ){
	CString * str = (CString *)lParam;
	AppendMessage(*str);
	delete str;

	RECORDER_EVENT eEvent = (RECORDER_EVENT)wParam;
	switch( eEvent ){
		// 若是开始录音、听到声音或者开始识别，则使按钮不可用
	case RECORDER_EVENT_BEGIN_RECORD:
	case RECORDER_EVENT_BEGIN_RECOGNIZE:		
	case RECORDER_EVENT_HAVING_VOICE:
		GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( FALSE );
		GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( TRUE );
		break;
		// 状态保持不变
	case RECORDER_EVENT_ENGINE_ERROR:
		break;
		// 录音结束、任务结束
	case RECORDER_EVENT_END_RECORD:
	case RECORDER_EVENT_TASK_FINISH:
		GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( TRUE );
		GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( FALSE );
		break;
		// 识别结束
	case RECORDER_EVENT_RECOGNIZE_COMPLETE:
		if (IsDlgButtonChecked( IDC_CONTINUE ) == FALSE){
			GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( TRUE );
			GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( FALSE );
		}
		break;
		// 其他状态，包括未听到声音或者发生错误等，则恢复按钮可用
	default:
		char buff[32];
		sprintf(buff, "Default Event:%d", eEvent);
		AppendMessage(CString(buff));

		GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( TRUE );
		GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( FALSE );
	}

	return 0;
}

void CRecorder_ExampleDlg::OnBnClickedBtnStartRecord(){	
	RECORDER_ERR_CODE eRet = RECORDER_ERR_NONE;
	
	GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( FALSE );
	GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( TRUE );	
	
	// 清空状态记录
	SetDlgItemText( IDC_EDIT_STATUS, "" );

    AccountInfo *account_info = AccountInfo::GetInstance();
    string startConfig = "";
	if (!IsDlgButtonChecked( IDC_ONLY_RECORDING )){
     	startConfig += "capkey=" + account_info->cap_key();
	}
	startConfig += ",audioformat=pcm16k16bit";
	//startConfig += ",domain=qwdz,intention=qwmap;music,needcontent=no";
	//startConfig     += ",realTime=rt";
    if (IsDlgButtonChecked( IDC_CONTINUE )){
        startConfig += ",continuous=yes";
    }
	if ( m_RecogMode == kRecogModeGrammar ){
		char chTmp[32] = {0};
		sprintf(chTmp,",grammarid=%d",m_GrammarId);
		startConfig += chTmp; 
	}

	eRet = hci_asr_recorder_start(startConfig.c_str(),"");
	if (RECORDER_ERR_NONE != eRet){
		CString strErrMessage;
		strErrMessage.Format( "开始录音失败,错误码%d", eRet );
		MessageBox( strErrMessage );
		GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( TRUE );
		return;
	}
}

bool CRecorder_ExampleDlg::Init(){	
    CString strErrorMessage;

	m_recordingFlag = FALSE;
	m_recordingFileName = "recording.pcm";
	m_recordingFile = NULL;
	SetDlgItemText( IDC_EDIT_SAVE_RECORDING_FILE, m_recordingFileName );
	UpdateData(FALSE);

    // 获取AccountInfo单例
    AccountInfo *account_info = AccountInfo::GetInstance();
    // 账号信息读取
    string account_info_file = "../../testdata/AccountInfo.txt";
    bool account_success = account_info->LoadFromFile(account_info_file);
    if (!account_success){
        strErrorMessage.Format("AccountInfo read from %s failed\n", account_info_file.c_str());
        MessageBox(strErrorMessage);
        return false;
    }

    // SYS初始化
    HCI_ERR_CODE errCode = HCI_ERR_NONE;
    // 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
    string init_config = "";
    init_config += "appKey=" + account_info->app_key();              //灵云应用序号
    init_config += ",developerKey=" + account_info->developer_key(); //灵云开发者密钥
    init_config += ",cloudUrl=" + account_info->cloud_url();         //灵云云服务的接口地址
	init_config += ",authpath=" + account_info->auth_path();         //授权文件所在路径，保证可写
    init_config += ",logfilepath=" + account_info->logfile_path();   //日志的路径
	init_config += ",logfilesize=1024000,loglevel=5";
    // 其他配置使用默认值，不再添加，如果想设置可以参考开发手册
    errCode = hci_init( init_config.c_str() );
    if( errCode != HCI_ERR_NONE ){
        strErrorMessage.Format( "hci_init return (%d:%s)\n", errCode, hci_get_error_info(errCode) );
        MessageBox(strErrorMessage);
        return false;
    }
    printf( "hci_init success\n" );


    // 检测授权,必要时到云端下载授权。此处需要注意的是，这个函数只是通过检测授权是否过期来判断是否需要进行
    // 获取授权操作，如果在开发调试过程中，授权账号中新增了灵云sdk的能力，请到hci_init传入的authPath路径中
    // 删除HCI_AUTH文件。否则无法获取新的授权文件，从而无法使用新增的灵云能力。
    if (!CheckAndUpdataAuth()){
        hci_release();
        strErrorMessage.Format("CheckAndUpdateAuth failed\n");
        MessageBox(strErrorMessage);
        return false;
    }

    // capkey属性获取
    m_RecogType = kRecogTypeUnkown;
    m_RecogMode = kRecogModeUnkown;
    GetCapkeyProperty(account_info->cap_key(),m_RecogType,m_RecogMode);

	if( m_RecogType == kRecogTypeCloud && m_RecogMode == kRecogModeGrammar ){
        // 云端语法暂时不支持实时识别
		// GetDlgItem( IDC_REALTIME )->EnableWindow(FALSE);
		hci_release();
        strErrorMessage.Format("Recorder not support cloud grammar, init failed\n");
        MessageBox(strErrorMessage);
        return false;
	}

    RECORDER_ERR_CODE eRet = RECORDER_ERR_UNKNOWN;
	RECORDER_CALLBACK_PARAM call_back;
	memset( &call_back, 0, sizeof(RECORDER_CALLBACK_PARAM) );
	call_back.pvStateChangeUsrParam		= this;
	call_back.pvRecogFinishUsrParam		= this;
	call_back.pvErrorUsrParam			= this;
	call_back.pvRecordingUsrParam		= this;
	call_back.pvRecogProcessParam		= this;
    call_back.pfnStateChange	= CRecorder_ExampleDlg::RecordEventChange;
	call_back.pfnRecogFinish	= CRecorder_ExampleDlg::RecorderRecogFinish;
	call_back.pfnError			= CRecorder_ExampleDlg::RecorderErr;
	call_back.pfnRecording		= CRecorder_ExampleDlg::RecorderRecordingCallback;
	call_back.pfnRecogProcess   = CRecorder_ExampleDlg::RecorderRecogProcess;


	string initConfig = "initCapkeys=" + account_info->cap_key();	
	initConfig        += ",dataPath=" + account_info->data_path();
	//string initConfig = "dataPath=" + account_info->data_path();
	//initConfig      += ",encode=speex";
	//initConfig		+= ",initCapkeys=asr.local.grammar";			      //初始化本地引擎

	eRet = hci_asr_recorder_init( initConfig.c_str(), &call_back);
	if (eRet != RECORDER_ERR_NONE){
		hci_release();
		strErrorMessage.Format( "录音机初始化失败,错误码%d", eRet);
		MessageBox( strErrorMessage );
		return false;
	}
    m_GrammarId = -1;
    if (m_RecogMode == kRecogModeGrammar){
        string grammarFile = account_info->test_data_path() + "/stock_10001.gram";
        if (m_RecogType == kRecogTypeLocal){
			string strLoadGrammarConfig = "grammarType=jsgf,isFile=yes,capkey=" + account_info->cap_key();
            eRet = hci_asr_recorder_load_grammar(strLoadGrammarConfig.c_str() , grammarFile.c_str(), &m_GrammarId );
            if( eRet != RECORDER_ERR_NONE ){
                hci_asr_recorder_release();
                hci_release();
                strErrorMessage.Format( "载入语法文件失败,错误码%d", eRet );
                MessageBox( strErrorMessage );
                return false;
            }
            EchoGrammarData(grammarFile);
        }else{
            // 如果是云端语法识别，需要开发者通过开发者社区自行上传语法文件，并获得可以使用的ID。
            // m_GrammarId = 2;
        }
    }

    return true;
}

void CRecorder_ExampleDlg::EchoGrammarData(const string &grammarFile){
    FILE* fp = fopen( grammarFile.c_str(), "rt" );
    if( fp == NULL ){
        GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( FALSE );
        CString strErrorMessage;
        strErrorMessage.Format("打开语法文件%s失败",grammarFile.c_str());
        MessageBox( strErrorMessage );
        return;
    }

    unsigned char szBom[3];
    fread( szBom, 3, 1, fp );
    // 若有bom头，则清除，没有则当前位置回到头部
    if( !( szBom[0] == 0xef && szBom[1] == 0xbb && szBom[2] == 0xbf ) ){
        fseek( fp, 0, SEEK_SET );
    }

    CString grammarData = "";
    char szData[1024] = {0};
    while( fgets( szData, 1024, fp ) != NULL ){
        unsigned char* pszGBK = NULL;
        HciExampleComon::UTF8ToGBK( (unsigned char*)szData, &pszGBK);
        grammarData += (char*)pszGBK;
        HciExampleComon::FreeConvertResult( pszGBK );
        grammarData += "\r\n";
    }

    fclose( fp );
    SetDlgItemText( IDC_EDIT_WORDLIST, grammarData );
    return;
}

bool CRecorder_ExampleDlg::Uninit(void){
	HCI_ERR_CODE eRet = HCI_ERR_NONE;	
	// 如果是本地语法识别，则需要释放语法资源
	if( m_RecogType == kRecogTypeLocal && m_RecogMode == kRecogModeGrammar ){
		hci_asr_recorder_unload_grammar( m_GrammarId );
	}
	RECORDER_ERR_CODE eRecRet;
	eRecRet = hci_asr_recorder_release();
	if(eRecRet != RECORDER_ERR_NONE){
		return false;
	}
	eRet = hci_release();

	AccountInfo::ReleaseInstance();

	return eRet == HCI_ERR_NONE;
}

void CRecorder_ExampleDlg::OnBnClickedBtnCancelRecord(){
	RECORDER_ERR_CODE eRet = hci_asr_recorder_cancel();
	if (RECORDER_ERR_NONE != eRet){
		CString str;
		str.Format( _T("终止录音失败,错误码%d"), eRet );
		MessageBox( str );
		return;
	}
	GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( TRUE );
	GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( FALSE );
}

void CRecorder_ExampleDlg::OnBnClickedOk(){
	Uninit(); 

	OnOK();
}

struct{
    char* pszName;
    char* pszComment;
}g_sStatus[] ={
    {"RECORDER_EVENT_BEGIN_RECORD",         "录音开始"},
    {"RECORDER_EVENT_HAVING_VOICE",         "听到声音 检测到始端的时候会触发该事件"},
    {"RECORDER_EVENT_NO_VOICE_INPUT",       "没有听到声音"},
    {"RECORDER_EVENT_BUFF_FULL",            "缓冲区已填满"},
    {"RECORDER_EVENT_END_RECORD",           "录音完毕（自动或手动结束）"},
    {"RECORDER_EVENT_BEGIN_RECOGNIZE",      "开始识别"},
    {"RECORDER_EVENT_RECOGNIZE_COMPLETE",   "识别完毕"},
    {"RECORDER_EVENT_ENGINE_ERROR",         "引擎出错"},
    {"RECORDER_EVENT_DEVICE_ERROR",         "设备出错"},
    {"RECORDER_EVENT_MALLOC_ERROR",         "分配空间失败"},
    {"RECORDER_EVENT_INTERRUPTED",          "内部错误"},
    {"RECORDER_EVENT_PERMISSION_DENIED",    "内部错误"},
    {"RECORDER_EVENT_TASK_FINISH",          "识别任务结束"},
    {"RECORDER_EVENT_RECOGNIZE_PROCESS",    "识别中间状态"}
};

void HCIAPI CRecorder_ExampleDlg::RecordEventChange(RECORDER_EVENT eRecorderEvent, void *pUsrParam){
    CRecorder_ExampleDlg *dlg = (CRecorder_ExampleDlg*)pUsrParam;
    	
	if(eRecorderEvent == RECORDER_EVENT_BEGIN_RECOGNIZE){
		dlg->m_startClock = clock();
	}
	if(eRecorderEvent == RECORDER_EVENT_END_RECORD){
		if(dlg->m_recordingFile != NULL){
			fclose(dlg->m_recordingFile);
			dlg->m_recordingFile = NULL;
		}
	}

	CString strMessage(g_sStatus[eRecorderEvent].pszComment);
	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}

void CRecorder_ExampleDlg::AppendMessage(CString & strMsg){
    CString strMessage = "";
    GetDlgItemText( IDC_EDIT_STATUS, strMessage );
    
	int nMessageLenMax = 1024;
	if(strMessage.GetLength() > nMessageLenMax){
		strMessage = strMessage.Left(nMessageLenMax);
	}
	CString strNewMessage = "";
	strNewMessage = strMsg;
	if(strMessage.GetLength() > 0){
		strNewMessage += "\r\n";
		strNewMessage += strMessage;
	}
    SetDlgItemText( IDC_EDIT_STATUS, strNewMessage );
}

void CRecorder_ExampleDlg::PostRecorderEventAndMsg(RECORDER_EVENT eRecorderEvent, const CString & strMessage){
	CString * msg = new CString(strMessage);
	::PostMessage(m_hWnd, WM_USER_SHOW_STATUS, eRecorderEvent, (LPARAM)msg);
}

void HCIAPI CRecorder_ExampleDlg::RecorderRecogFinish(
                                       RECORDER_EVENT eRecorderEvent,
                                       ASR_RECOG_RESULT *psAsrRecogResult,
                                       void *pUsrParam){
	CString strMessage = "";

    CRecorder_ExampleDlg *dlg = (CRecorder_ExampleDlg*)pUsrParam;
	if(eRecorderEvent == RECORDER_EVENT_RECOGNIZE_COMPLETE){
		char buff[32];
		clock_t endClock = clock();

		strMessage.AppendFormat( "识别时间:%d", (int)endClock - (int)dlg->m_startClock );
		
		dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
	}

    strMessage = "";
    if( psAsrRecogResult->uiResultItemCount > 0 ){
        unsigned char* pucUTF8 = NULL;
        HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
        strMessage.AppendFormat( "识别结果: %s", pucUTF8 );
        HciExampleComon::FreeConvertResult( pucUTF8 );
        pucUTF8 = NULL;
    }else{
        strMessage.AppendFormat( "*****无识别结果*****" );
    }
    
	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}

void HCIAPI CRecorder_ExampleDlg::RecorderRecogProcess(
                                        RECORDER_EVENT eRecorderEvent,
                                        ASR_RECOG_RESULT *psAsrRecogResult,
                                        void *pUsrParam){
    CRecorder_ExampleDlg *dlg = (CRecorder_ExampleDlg*)pUsrParam;
    CString strMessage = "";
    if( psAsrRecogResult->uiResultItemCount > 0 ){
        unsigned char* pucUTF8 = NULL;
        HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
        strMessage.AppendFormat( "识别中间结果: %s", pucUTF8 );
        HciExampleComon::FreeConvertResult( pucUTF8 );
        pucUTF8 = NULL;
    }else{
        strMessage.AppendFormat( "*****无识别结果*****" );
    }

	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);    
}

void HCIAPI CRecorder_ExampleDlg::RecorderErr(
                               RECORDER_EVENT eRecorderEvent,
                               HCI_ERR_CODE eErrorCode,
                               void *pUsrParam){
    CRecorder_ExampleDlg * dlg = (CRecorder_ExampleDlg*)pUsrParam;
    CString strMessage = "";
    strMessage.AppendFormat( "系统错误:%d", eErrorCode );

	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}

void HCIAPI CRecorder_ExampleDlg::RecorderRecordingCallback(
                                     unsigned char * pVoiceData,
                                     unsigned int uiVoiceLen,
                                     void * pUsrParam
                                     ){
	CRecorder_ExampleDlg * dlg = (CRecorder_ExampleDlg *)pUsrParam;
	dlg->RecorderRecording(pVoiceData, uiVoiceLen);
}

void CRecorder_ExampleDlg::RecorderRecording(unsigned char * pVoiceData, unsigned int uiVoiceLen){
	if(m_recordingFlag == FALSE){
		if(m_recordingFile != NULL){
			fclose(m_recordingFile);
			m_recordingFile = NULL;
		}
		return;
	}

	if(m_recordingFile == NULL){
		m_recordingFile = fopen( m_recordingFileName.GetBuffer(), "wb" );
		if( m_recordingFile == NULL ){
			return;
		}
	}

	fwrite(pVoiceData, sizeof(unsigned char), uiVoiceLen, m_recordingFile);
	fflush(m_recordingFile);
}

void CRecorder_ExampleDlg::OnBnClickedBtnBrowser(){
	CFileDialog dlgFile(TRUE, NULL, m_recordingFileName.GetBuffer(), OFN_HIDEREADONLY, _T("PCM Files (*.pcm)|*.pcm|All Files (*.*)|*.*||"), NULL);
    if (dlgFile.DoModal()){
        m_recordingFileName = dlgFile.GetPathName();
    }
    
	SetDlgItemText( IDC_EDIT_SAVE_RECORDING_FILE, m_recordingFileName );
}

void CRecorder_ExampleDlg::OnBnClickedSaveRecording(){
	UpdateData(TRUE);

	if(m_recordingFlag == FALSE){
		if(m_recordingFile != NULL){
			fclose(m_recordingFile);
			m_recordingFile = NULL;
		}
	}
}
