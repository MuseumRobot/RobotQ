#include "stdafx.h"
#include "ASR_Recorder_Example.h"
#include "ASR_Recorder_ExampleDlg.h"
#include "hci_asr_recorder.h"
#include "common/CommonTool.h"
#include "common/AccountInfo.h"

#define WM_USER_SHOW_STATUS	WM_USER + 100

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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

// CRecorder_ExampleDlg �Ի���

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
    //��ȡ����ʱ��
    int64 nExpireTime;
    int64 nCurTime = (int64)time( NULL );
    HCI_ERR_CODE errCode = hci_get_auth_expire_time( &nExpireTime );
    if( errCode == HCI_ERR_NONE ){
        //��ȡ�ɹ����ж��Ƿ����
        if( nExpireTime > nCurTime ){
            //û�й���
            printf( "auth can use continue\n" );
            return true;
        }
    }

    //��ȡ����ʱ��ʧ�ܻ��Ѿ�����
    //�ֶ����ø�����Ȩ
    errCode = hci_check_auth();
    if( errCode == HCI_ERR_NONE ){
        //���³ɹ�
        printf( "check auth success \n" );
        return true;
    }
    else{
        //����ʧ��
        printf( "check auth return (%d:%s)\n", errCode ,hci_get_error_info(errCode));
        return false;
    }
}

//��ȡcapkey����
void GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode){
    HCI_ERR_CODE errCode = HCI_ERR_NONE;
	CAPABILITY_ITEM *pItem = NULL;

	// ö�����е�asr����
	CAPABILITY_LIST list = {0};
	if ((errCode = hci_get_capability_list("asr", &list))!= HCI_ERR_NONE){
		// û���ҵ���Ӧ��������
		return;
	}

	// ��ȡasr����������Ϣ��
	for (int i = 0; i < list.uiItemCount; i++){
		if (list.pItemList[i].pszCapKey != NULL && stricmp(list.pItemList[i].pszCapKey, cap_key.c_str()) == 0){
			pItem = &list.pItemList[i];
			break;
		}
	}

	// û�л�ȡ��Ӧ�������ã����ء�
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
// CRecorder_ExampleDlg ��Ϣ�������

BOOL CRecorder_ExampleDlg::OnInitDialog(){
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	((CButton *)GetDlgItem( IDC_CONTINUE ))->SetCheck(TRUE);
    if (Init() == false){
        return FALSE;
    }
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CRecorder_ExampleDlg::OnSysCommand(UINT nID, LPARAM lParam){
	if ((nID & 0xFFF0) == IDM_ABOUTBOX){
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}else{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CRecorder_ExampleDlg::OnPaint(){
	if (IsIconic()){
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}else{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CRecorder_ExampleDlg::OnQueryDragIcon(){
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CRecorder_ExampleDlg::OnShowStatus( WPARAM wParam, LPARAM lParam ){
	CString * str = (CString *)lParam;
	AppendMessage(*str);
	delete str;

	RECORDER_EVENT eEvent = (RECORDER_EVENT)wParam;
	switch( eEvent ){
		// ���ǿ�ʼ¼���������������߿�ʼʶ����ʹ��ť������
	case RECORDER_EVENT_BEGIN_RECORD:
	case RECORDER_EVENT_BEGIN_RECOGNIZE:		
	case RECORDER_EVENT_HAVING_VOICE:
		GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( FALSE );
		GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( TRUE );
		break;
		// ״̬���ֲ���
	case RECORDER_EVENT_ENGINE_ERROR:
		break;
		// ¼���������������
	case RECORDER_EVENT_END_RECORD:
	case RECORDER_EVENT_TASK_FINISH:
		GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( TRUE );
		GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( FALSE );
		break;
		// ʶ�����
	case RECORDER_EVENT_RECOGNIZE_COMPLETE:
		if (IsDlgButtonChecked( IDC_CONTINUE ) == FALSE){
			GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( TRUE );
			GetDlgItem( IDC_BTN_CANCEL_RECORD )->EnableWindow( FALSE );
		}
		break;
		// ����״̬������δ�����������߷�������ȣ���ָ���ť����
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
	
	// ���״̬��¼
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
		strErrMessage.Format( "��ʼ¼��ʧ��,������%d", eRet );
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

    // ��ȡAccountInfo����
    AccountInfo *account_info = AccountInfo::GetInstance();
    // �˺���Ϣ��ȡ
    string account_info_file = "../../testdata/AccountInfo.txt";
    bool account_success = account_info->LoadFromFile(account_info_file);
    if (!account_success){
        strErrorMessage.Format("AccountInfo read from %s failed\n", account_info_file.c_str());
        MessageBox(strErrorMessage);
        return false;
    }

    // SYS��ʼ��
    HCI_ERR_CODE errCode = HCI_ERR_NONE;
    // ���ô�����"�ֶ�=ֵ"����ʽ������һ���ַ���������ֶ�֮����','�������ֶ������ִ�Сд��
    string init_config = "";
    init_config += "appKey=" + account_info->app_key();              //����Ӧ�����
    init_config += ",developerKey=" + account_info->developer_key(); //���ƿ�������Կ
    init_config += ",cloudUrl=" + account_info->cloud_url();         //�����Ʒ���Ľӿڵ�ַ
	init_config += ",authpath=" + account_info->auth_path();         //��Ȩ�ļ�����·������֤��д
    init_config += ",logfilepath=" + account_info->logfile_path();   //��־��·��
	init_config += ",logfilesize=1024000,loglevel=5";
    // ��������ʹ��Ĭ��ֵ��������ӣ���������ÿ��Բο������ֲ�
    errCode = hci_init( init_config.c_str() );
    if( errCode != HCI_ERR_NONE ){
        strErrorMessage.Format( "hci_init return (%d:%s)\n", errCode, hci_get_error_info(errCode) );
        MessageBox(strErrorMessage);
        return false;
    }
    printf( "hci_init success\n" );


    // �����Ȩ,��Ҫʱ���ƶ�������Ȩ���˴���Ҫע����ǣ��������ֻ��ͨ�������Ȩ�Ƿ�������ж��Ƿ���Ҫ����
    // ��ȡ��Ȩ����������ڿ������Թ����У���Ȩ�˺�������������sdk���������뵽hci_init�����authPath·����
    // ɾ��HCI_AUTH�ļ��������޷���ȡ�µ���Ȩ�ļ����Ӷ��޷�ʹ������������������
    if (!CheckAndUpdataAuth()){
        hci_release();
        strErrorMessage.Format("CheckAndUpdateAuth failed\n");
        MessageBox(strErrorMessage);
        return false;
    }

    // capkey���Ի�ȡ
    m_RecogType = kRecogTypeUnkown;
    m_RecogMode = kRecogModeUnkown;
    GetCapkeyProperty(account_info->cap_key(),m_RecogType,m_RecogMode);

	if( m_RecogType == kRecogTypeCloud && m_RecogMode == kRecogModeGrammar ){
        // �ƶ��﷨��ʱ��֧��ʵʱʶ��
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
	//initConfig		+= ",initCapkeys=asr.local.grammar";			      //��ʼ����������

	eRet = hci_asr_recorder_init( initConfig.c_str(), &call_back);
	if (eRet != RECORDER_ERR_NONE){
		hci_release();
		strErrorMessage.Format( "¼������ʼ��ʧ��,������%d", eRet);
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
                strErrorMessage.Format( "�����﷨�ļ�ʧ��,������%d", eRet );
                MessageBox( strErrorMessage );
                return false;
            }
            EchoGrammarData(grammarFile);
        }else{
            // ������ƶ��﷨ʶ����Ҫ������ͨ�����������������ϴ��﷨�ļ�������ÿ���ʹ�õ�ID��
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
        strErrorMessage.Format("���﷨�ļ�%sʧ��",grammarFile.c_str());
        MessageBox( strErrorMessage );
        return;
    }

    unsigned char szBom[3];
    fread( szBom, 3, 1, fp );
    // ����bomͷ���������û����ǰλ�ûص�ͷ��
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
	// ����Ǳ����﷨ʶ������Ҫ�ͷ��﷨��Դ
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
		str.Format( _T("��ֹ¼��ʧ��,������%d"), eRet );
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
    {"RECORDER_EVENT_BEGIN_RECORD",         "¼����ʼ"},
    {"RECORDER_EVENT_HAVING_VOICE",         "�������� ��⵽ʼ�˵�ʱ��ᴥ�����¼�"},
    {"RECORDER_EVENT_NO_VOICE_INPUT",       "û����������"},
    {"RECORDER_EVENT_BUFF_FULL",            "������������"},
    {"RECORDER_EVENT_END_RECORD",           "¼����ϣ��Զ����ֶ�������"},
    {"RECORDER_EVENT_BEGIN_RECOGNIZE",      "��ʼʶ��"},
    {"RECORDER_EVENT_RECOGNIZE_COMPLETE",   "ʶ�����"},
    {"RECORDER_EVENT_ENGINE_ERROR",         "�������"},
    {"RECORDER_EVENT_DEVICE_ERROR",         "�豸����"},
    {"RECORDER_EVENT_MALLOC_ERROR",         "����ռ�ʧ��"},
    {"RECORDER_EVENT_INTERRUPTED",          "�ڲ�����"},
    {"RECORDER_EVENT_PERMISSION_DENIED",    "�ڲ�����"},
    {"RECORDER_EVENT_TASK_FINISH",          "ʶ���������"},
    {"RECORDER_EVENT_RECOGNIZE_PROCESS",    "ʶ���м�״̬"}
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

		strMessage.AppendFormat( "ʶ��ʱ��:%d", (int)endClock - (int)dlg->m_startClock );
		
		dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
	}

    strMessage = "";
    if( psAsrRecogResult->uiResultItemCount > 0 ){
        unsigned char* pucUTF8 = NULL;
        HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
        strMessage.AppendFormat( "ʶ����: %s", pucUTF8 );
        HciExampleComon::FreeConvertResult( pucUTF8 );
        pucUTF8 = NULL;
    }else{
        strMessage.AppendFormat( "*****��ʶ����*****" );
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
        strMessage.AppendFormat( "ʶ���м���: %s", pucUTF8 );
        HciExampleComon::FreeConvertResult( pucUTF8 );
        pucUTF8 = NULL;
    }else{
        strMessage.AppendFormat( "*****��ʶ����*****" );
    }

	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);    
}

void HCIAPI CRecorder_ExampleDlg::RecorderErr(
                               RECORDER_EVENT eRecorderEvent,
                               HCI_ERR_CODE eErrorCode,
                               void *pUsrParam){
    CRecorder_ExampleDlg * dlg = (CRecorder_ExampleDlg*)pUsrParam;
    CString strMessage = "";
    strMessage.AppendFormat( "ϵͳ����:%d", eErrorCode );

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
