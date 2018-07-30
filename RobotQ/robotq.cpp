#include "robotq.h"
#include "stdafx.h"
#include "common/AccountInfo.h"
#include "common/CommonTool.h"

#define WM_USER_SHOW_STATUS	WM_USER + 100

RobotQ::RobotQ(QWidget *parent, Qt::WFlags flags):QMainWindow(parent, flags){
	ui.setupUi(this);
	connect(ui.btnStart,SIGNAL(clicked(bool)),this,SLOT(OnStartClicked(bool)));
	connect(ui.btnEnd,SIGNAL(clicked(bool)),this,SLOT(OnEndClicked(bool)));
	Init();
}
RobotQ::~RobotQ(){
	Uninit();
}

int RobotQ::OnStartClicked(bool checked){
	ui.textStatus->setPlainText("Start");

	return 0;
}
int RobotQ::OnEndClicked(bool checked){
	ui.textStatus->setPlainText("End");

	return 0;
}

//ASR����*******************************
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

bool RobotQ::Init(){	
	m_recordingFlag = FALSE;
	m_recordingFileName = "recording.pcm";
	m_recordingFile = NULL;
	//SetDlgItemText( IDC_EDIT_SAVE_RECORDING_FILE, m_recordingFileName );
	//UpdateData(FALSE);

	// ��ȡAccountInfo����
	AccountInfo *account_info = AccountInfo::GetInstance();
	// �˺���Ϣ��ȡ
	string account_info_file = "testdata/AccountInfo.txt";
	bool account_success = account_info->LoadFromFile(account_info_file);
	if (!account_success){
		//strErrorMessage.Format("AccountInfo read from %s failed\n", account_info_file.c_str());
		//MessageBox(strErrorMessage);
		QString str=sprintf("AccountInfo read from %s failed\n",account_info_file.c_str());
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
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
	init_config += ",logfilepath=" + account_info->logfile_path();   //��־��·�������Բ�Ҫ��־��
	init_config += ",logfilesize=1024000,loglevel=5";
	// ��������ʹ��Ĭ��ֵ��������ӣ���������ÿ��Բο������ֲ�
	errCode = hci_init( init_config.c_str() );
	if( errCode != HCI_ERR_NONE ){
		//strErrorMessage.Format( "hci_init return (%d:%s)\n", errCode, hci_get_error_info(errCode) );
		//MessageBox(strErrorMessage);
		QString str;
		str.sprintf("hci_init return %s\n",  hci_get_error_info(errCode));
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
		return false;
	}
	printf( "hci_init success\n" );


	// �����Ȩ,��Ҫʱ���ƶ�������Ȩ���˴���Ҫע����ǣ��������ֻ��ͨ�������Ȩ�Ƿ�������ж��Ƿ���Ҫ����
	// ��ȡ��Ȩ����������ڿ������Թ����У���Ȩ�˺�������������sdk���������뵽hci_init�����authPath·����
	// ɾ��HCI_AUTH�ļ��������޷���ȡ�µ���Ȩ�ļ����Ӷ��޷�ʹ������������������
	if (!CheckAndUpdataAuth()){
		hci_release();
		//strErrorMessage.Format("CheckAndUpdateAuth failed\n");
		//MessageBox(strErrorMessage);
		QString str;
		str.sprintf("CheckAndUpdateAuth failed\n");
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
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
		//strErrorMessage.Format("Recorder not support cloud grammar, init failed\n");
		//MessageBox(strErrorMessage);
		QString str;
		str.sprintf("Recorder not support cloud grammar, init failed\n");
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
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
	call_back.pfnStateChange	= RobotQ::RecordEventChange;
	call_back.pfnRecogFinish	= RobotQ::RecorderRecogFinish;
	call_back.pfnError			= RobotQ::RecorderErr;
	call_back.pfnRecording		= RobotQ::RecorderRecordingCallback;
	call_back.pfnRecogProcess   = RobotQ::RecorderRecogProcess;


	string initConfig = "initCapkeys=" + account_info->cap_key();	
	initConfig        += ",dataPath=" + account_info->data_path();
	//string initConfig = "dataPath=" + account_info->data_path();
	//initConfig      += ",encode=speex";
	//initConfig		+= ",initCapkeys=asr.local.grammar";			      //��ʼ����������

	eRet = hci_asr_recorder_init( initConfig.c_str(), &call_back);
	if (eRet != RECORDER_ERR_NONE){
		hci_release();
		//strErrorMessage.Format( "¼������ʼ��ʧ��,������%d", eRet);
		//MessageBox( strErrorMessage );
		QString str;
		str.sprintf("¼������ʼ��ʧ�ܣ�������%d\n",eRet);
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
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
				//strErrorMessage.Format( "�����﷨�ļ�ʧ��,������%d", eRet );
				//MessageBox( strErrorMessage );
				QString str;
				str.sprintf("�����﷨�ļ�ʧ�ܣ�������%d",eRet);
				QMessageBox msgBox;
				msgBox.setText(str);
				msgBox.exec();
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

void RobotQ::EchoGrammarData(const string &grammarFile){
	FILE* fp = fopen( grammarFile.c_str(), "rt" );
	if( fp == NULL ){
		//GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( FALSE );
		//CString strErrorMessage;
		//strErrorMessage.Format("���﷨�ļ�%sʧ��",grammarFile.c_str());
		//MessageBox( strErrorMessage );
		QString str;
		str.sprintf("���﷨�ļ�%sʧ��",grammarFile.c_str());
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
		return;
	}

	unsigned char szBom[3];
	fread( szBom, 3, 1, fp );
	// ����bomͷ���������û����ǰλ�ûص�ͷ��
	if( !( szBom[0] == 0xef && szBom[1] == 0xbb && szBom[2] == 0xbf ) ){
		fseek( fp, 0, SEEK_SET );
	}

	QString grammarData = "";
	char szData[1024] = {0};
	while( fgets( szData, 1024, fp ) != NULL ){
		unsigned char* pszGBK = NULL;
		HciExampleComon::UTF8ToGBK( (unsigned char*)szData, &pszGBK);
		grammarData += (char*)pszGBK;
		HciExampleComon::FreeConvertResult( pszGBK );
		grammarData += "\r\n";
	}

	fclose( fp );
	//SetDlgItemText( IDC_EDIT_WORDLIST, grammarData );
	return;
}

bool RobotQ::Uninit(void){
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

void HCIAPI RobotQ::RecorderRecogProcess(
	RECORDER_EVENT eRecorderEvent,
	ASR_RECOG_RESULT *psAsrRecogResult,
	void *pUsrParam){
		QString strMessage = "";
		QString add;
		if( psAsrRecogResult->uiResultItemCount > 0 ){
			unsigned char* pucUTF8 = NULL;
			HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
			add.sprintf( "ʶ���м���: %s", pucUTF8 );
			strMessage+=add;
			HciExampleComon::FreeConvertResult( pucUTF8 );
			pucUTF8 = NULL;
		}else{
			strMessage.append( "*****��ʶ����*****" );
		}
		//ui.textStatus->setPlainText(strMessage);	//�˴���ʾ���Ӧ����SIGNAL&SLOTǿ��ʵ��
		    
}
void HCIAPI RobotQ::RecordEventChange(RECORDER_EVENT eRecorderEvent, void *pUsrParam){
	//if(eRecorderEvent == RECORDER_EVENT_BEGIN_RECOGNIZE){
	//	dlg->m_startClock = clock();
	//}
	//if(eRecorderEvent == RECORDER_EVENT_END_RECORD){
	//	if(dlg->m_recordingFile != NULL){
	//		fclose(dlg->m_recordingFile);
	//		dlg->m_recordingFile = NULL;
	//	}
	//}

	//QString strMessage(g_sStatus[eRecorderEvent].pszComment);
	//dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}
void HCIAPI RobotQ::RecorderErr(
	RECORDER_EVENT eRecorderEvent,
	HCI_ERR_CODE eErrorCode,
	void *pUsrParam){
		QString strMessage = "";
		QString add;
		add.sprintf( "ϵͳ����:%d", eErrorCode);
		strMessage+=add;

		/*dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);*/
}

void HCIAPI RobotQ::RecorderRecordingCallback(
	unsigned char * pVoiceData,
	unsigned int uiVoiceLen,
	void * pUsrParam
	){
		//dlg->RecorderRecording(pVoiceData, uiVoiceLen);
}

void RobotQ::RecorderRecording(unsigned char * pVoiceData, unsigned int uiVoiceLen){
	if(m_recordingFlag == FALSE){
		if(m_recordingFile != NULL){
			fclose(m_recordingFile);
			m_recordingFile = NULL;
		}
		return;
	}

	if(m_recordingFile == NULL){
		m_recordingFile = fopen( m_recordingFileName.toStdString().c_str(), "wb" );
		if( m_recordingFile == NULL ){
			return;
		}
	}

	fwrite(pVoiceData, sizeof(unsigned char), uiVoiceLen, m_recordingFile);
	fflush(m_recordingFile);
}
void HCIAPI RobotQ::RecorderRecogFinish(
	RECORDER_EVENT eRecorderEvent,
	ASR_RECOG_RESULT *psAsrRecogResult,
	void *pUsrParam){
		QString strMessage = "";
		if(eRecorderEvent == RECORDER_EVENT_RECOGNIZE_COMPLETE){
			char buff[32];
			clock_t endClock = clock();

			//strMessage.AppendFormat( "ʶ��ʱ��:%d", (int)endClock - (int)dlg->m_startClock );

			//dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
		}

		strMessage = "";
		if( psAsrRecogResult->uiResultItemCount > 0 ){
			unsigned char* pucUTF8 = NULL;
			HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
			//strMessage.AppendFormat( "ʶ����: %s", pucUTF8 );
			HciExampleComon::FreeConvertResult( pucUTF8 );
			pucUTF8 = NULL;
		}else{
			//strMessage.AppendFormat( "*****��ʶ����*****" );
		}

		//dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}