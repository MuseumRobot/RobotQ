#include "robotq.h"

//��̬ȫ�ֱ���
QString RobotQ::GLOBAL_strMessage;
RECORDER_EVENT RobotQ::GLOBAL_eRecorderEvent;
bool RobotQ::GLOBAL_CommandValid;

RobotQ::RobotQ(QWidget *parent, Qt::WFlags flags):QMainWindow(parent, flags){
	ui.setupUi(this);
	connect(ui.btnStart,SIGNAL(clicked(bool)),this,SLOT(OnStartClicked(bool)));
	connect(ui.btnEnd,SIGNAL(clicked(bool)),this,SLOT(OnEndClicked(bool)));
	Init();
	m_timerId=startTimer(MSG_REFRESH_TIME);	//��ʱ����ѯʶ��״̬
}
RobotQ::~RobotQ(){
	Uninit();
}
int RobotQ::OnStartClicked(bool checked){
	RECORDER_ERR_CODE eRet = RECORDER_ERR_NONE;
	ui.btnStart->setEnabled(false);
	ui.btnEnd->setEnabled(true);
	AccountInfo *account_info = AccountInfo::GetInstance();
	string startConfig = "";
	startConfig += "capkey=" + account_info->cap_key();
	startConfig += ",audioformat=pcm16k16bit";
	if(IS_RECORDER_CONTINUE)	//�Ƿ�����¼��
		startConfig += ",continuous=yes";	
	if ( m_RecogMode == kRecogModeDialog ){
		startConfig +=",intention=weather;joke;story;baike;calendar;translation;news"; 
	}
	eRet = hci_asr_recorder_start(startConfig.c_str(),"");
	if (RECORDER_ERR_NONE != eRet){
		QString strErrMessage;
		strErrMessage.sprintf( "��ʼ¼��ʧ��,������%d", eRet );
		QMessageBox msgBox;
		msgBox.setText(strErrMessage);
		msgBox.exec();
		return 1;
	}
	return 0;
}
int RobotQ::OnEndClicked(bool checked){
	RECORDER_ERR_CODE eRet = hci_asr_recorder_cancel();
	if (RECORDER_ERR_NONE != eRet){
		QString strErrMessage;
		strErrMessage.sprintf( "��ֹ¼��ʧ��,������%d", eRet );
		QMessageBox msgBox;
		msgBox.setText(strErrMessage);
		msgBox.exec();
		return 1;
	}
	ui.btnEnd->setEnabled(false);
	ui.btnStart->setEnabled(true);
	return 0;
}
void RobotQ::timerEvent(QTimerEvent *event){
	if(event->timerId()==m_timerId&&GLOBAL_CommandValid==TRUE){
		OnShowStatus(GLOBAL_eRecorderEvent,GLOBAL_strMessage);
		GLOBAL_CommandValid = FALSE;
	}
}
bool RobotQ::Init(){	
	GLOBAL_CommandValid = FALSE;	//��ʼ��ִ�в��費���ӵ���ʾ����
	m_recordingFlag = FALSE;
	m_recordingFileName = "recording.pcm";
	m_recordingFile = NULL;
	// ��ȡAccountInfo����
	AccountInfo *account_info = AccountInfo::GetInstance();
	// �˺���Ϣ��ȡ
	string account_info_file = "testdata/AccountInfo.txt";
	bool account_success = account_info->LoadFromFile(account_info_file);
	if (!account_success){
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
	//init_config += ",logfilepath=" + account_info->logfile_path();   //��־��·�������Բ�Ҫ��־��
	init_config += ",logfilesize=1024000,loglevel=5";	// ��������ʹ��Ĭ��ֵ��������ӣ���������ÿ��Բο������ֲ�
	errCode = hci_init( init_config.c_str() );
	if( errCode != HCI_ERR_NONE ){
		QString str;
		str.sprintf("hci_init return %s\n",  hci_get_error_info(errCode));
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
		return false;
	}
	qDebug()<<"hci_init success";
	// �����Ȩ,��Ҫʱ���ƶ�������Ȩ���˴���Ҫע����ǣ��������ֻ��ͨ�������Ȩ�Ƿ�������ж��Ƿ���Ҫ����
	// ��ȡ��Ȩ����������ڿ������Թ����У���Ȩ�˺�������������sdk���������뵽hci_init�����authPath·����
	// ɾ��HCI_AUTH�ļ��������޷���ȡ�µ���Ȩ�ļ����Ӷ��޷�ʹ������������������
	if (!CheckAndUpdataAuth()){
		hci_release();
		QString str;
		str.sprintf("CheckAndUpdateAuth failed\n");
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
		return false;
	}
	m_RecogType = kRecogTypeUnkown;
	m_RecogMode = kRecogModeUnkown;
	GetCapkeyProperty(account_info->cap_key(),m_RecogType,m_RecogMode);
	//asr_recorder��ʼ��
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
	eRet = hci_asr_recorder_init( initConfig.c_str(), &call_back);
	if (eRet != RECORDER_ERR_NONE){
		hci_release();
		QString str;
		str.sprintf("¼������ʼ��ʧ�ܣ�������%d\n",eRet);
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
		return false;
	}
	//tts_player��ʼ��
	PLAYER_CALLBACK_PARAM cb;
	cb.pvStateChangeUsrParam			= this;
	cb.pvProgressChangeUsrParam			= this;
	cb.pvPlayerErrorUsrParam			= this;
	cb.pfnProgressChange			= RobotQ::CB_ProgressChange;
	cb.pfnStateChange				= RobotQ::CB_EventChange;
	cb.pfnPlayerError				= RobotQ::CB_SdkErr;

	PLAYER_ERR_CODE eReti = PLAYER_ERR_NONE;
	string initConfigtts = "InitCapkeys=tts.cloud.wangjing";
	initConfigtts += ",dataPath="+account_info->data_path();
	eReti = hci_tts_player_init( initConfigtts.c_str(), &cb );
	QString str="���ѽ!";
	unsigned char* pszUTF8 = NULL;
	HciExampleComon::GBKToUTF8( (unsigned char*)str.toStdString().c_str(), &pszUTF8 );
	string startConfig = "property=cn_xiaokun_common,tagmode=none,capkey=tts.cloud.wangjing";
	PLAYER_ERR_CODE eRetk = hci_tts_player_start( (const char*)pszUTF8, startConfig.c_str() );



	return true;
}

bool RobotQ::Uninit(void){
	HCI_ERR_CODE eRet = HCI_ERR_NONE;
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

void HCIAPI RobotQ::RecorderRecogProcess(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult,void *pUsrParam){
		RobotQ *dlg=(RobotQ *)pUsrParam;
		QString strMessage = "";
		QString add;
		if( psAsrRecogResult->uiResultItemCount > 0 ){
			unsigned char* pucUTF8 = NULL;
			HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
			add.sprintf( "ʶ���м���: %s", pucUTF8 );
			add=QString::fromLocal8Bit(add.toStdString().c_str());
			strMessage+=add;
			HciExampleComon::FreeConvertResult( pucUTF8 );
			pucUTF8 = NULL;
		}else{
			strMessage.append( "*****��ʶ����*****" );
		}
		dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);	
}
void HCIAPI RobotQ::RecordEventChange(RECORDER_EVENT eRecorderEvent, void *pUsrParam){
	RobotQ *dlg=(RobotQ *)pUsrParam;
	if(eRecorderEvent == RECORDER_EVENT_BEGIN_RECOGNIZE){
		dlg->m_startClock = clock();
	}
	if(eRecorderEvent == RECORDER_EVENT_END_RECORD){
		if(dlg->m_recordingFile != NULL){
			fclose(dlg->m_recordingFile);
			dlg->m_recordingFile = NULL;
		}
	}
	QString strMessage(g_sStatus[eRecorderEvent].pszComment);
	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}
void HCIAPI RobotQ::RecorderErr(RECORDER_EVENT eRecorderEvent,HCI_ERR_CODE eErrorCode,void *pUsrParam){
		RobotQ *dlg=(RobotQ *)pUsrParam;
		QString strMessage = "";
		QString add;
		add.sprintf( "ϵͳ����:%d", eErrorCode);
		strMessage+=add;
		dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}
void HCIAPI RobotQ::RecorderRecordingCallback(unsigned char * pVoiceData,unsigned int uiVoiceLen,void * pUsrParam){
		RobotQ *dlg=(RobotQ *)pUsrParam;
		dlg->RecorderRecording(pVoiceData, uiVoiceLen);
}
void HCIAPI RobotQ::RecorderRecogFinish(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult,void *pUsrParam){
		RobotQ *dlg=(RobotQ *)pUsrParam;
		QString strMessage = ""; 
		if(eRecorderEvent == RECORDER_EVENT_RECOGNIZE_COMPLETE){
			char buff[32];
			clock_t endClock = clock();
			QString add;
			add.sprintf("ʶ��ʱ��:%dms", (int)endClock - (int)dlg->m_startClock);
			strMessage+=add;
			dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
		}
		strMessage = "";
		if( psAsrRecogResult->uiResultItemCount > 0 ){
			unsigned char* pucUTF8 = NULL;
			HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
			QString add;
			string str=(char*)pucUTF8;
			add=QString::fromStdString(str);
			strMessage+="ʶ����:";
			strMessage+=add;
			HciExampleComon::FreeConvertResult( pucUTF8 );
			pucUTF8 = NULL;
		}else{
			strMessage.append( "*****��ʶ����*****" );
		}
		dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}

//�Զ���ۺ���
void RobotQ::OnShowStatus(RECORDER_EVENT eRecorderEvent, QString strMessage){
	AppendMessage(strMessage);	//���ı����и�����ʾ��Ϣ
	RECORDER_EVENT eEvent = eRecorderEvent;
	switch( eEvent ){
		// ���ǿ�ʼ¼���������������߿�ʼʶ����ʹ��ť������
	case RECORDER_EVENT_BEGIN_RECORD:
	case RECORDER_EVENT_BEGIN_RECOGNIZE:		
	case RECORDER_EVENT_HAVING_VOICE:
		ui.btnStart->setEnabled(false);
		ui.btnEnd->setEnabled(true);
		break;
		// ״̬���ֲ���
	case RECORDER_EVENT_ENGINE_ERROR:
		break;
		// ¼���������������
	case RECORDER_EVENT_END_RECORD:
	case RECORDER_EVENT_TASK_FINISH:
		ui.btnStart->setEnabled(true);
		ui.btnEnd->setEnabled(false);
		break;
		// ʶ�����
	case RECORDER_EVENT_RECOGNIZE_COMPLETE:
		if(IS_RECORDER_CONTINUE==FALSE){
			ui.btnStart->setEnabled(true);
			ui.btnEnd->setEnabled(false);
		}
		break;
		// ����״̬������δ�����������߷�������ȣ���ָ���ť����
	default:
		char buff[32];
		sprintf(buff, "Default Event:%d", eEvent);
		AppendMessage(QString(buff));
		ui.btnStart->setEnabled(true);
		ui.btnEnd->setEnabled(false);
	}
}

//�Զ��幤�ߺ���
void RobotQ::AppendMessage(QString strMsg){	//AppendMessage����Ŀ���ǽ��¼ӵ����ֲ�����д��״̬�ı�����(��0�����ַ������)
	QString strMessage = "";
	strMessage=ui.textStatus->toPlainText();
	int nMessageLenMax = 1024;		//״̬�ı���������1024���ַ�
	if(strMessage.length() > nMessageLenMax){
		strMessage = strMessage.left(nMessageLenMax);	//���ʵ���ַ����ȳ���Ԥ�����ֵ�����ȡ���²���������ʾ���ɵ�����
	}
	QString strNewMessage = "";
	strNewMessage = strMsg;
	if(strMessage.length() > 0){
		strNewMessage += "\r\n";
		strNewMessage += strMessage;
	}
	ui.textStatus->setPlainText(strNewMessage);
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
	fflush(m_recordingFile);	//���»�������������������ǿ��д���ļ�
}
void RobotQ::PostRecorderEventAndMsg(RECORDER_EVENT eRecorderEvent, QString strMessage){
	GLOBAL_CommandValid=TRUE;
	GLOBAL_eRecorderEvent=eRecorderEvent;
	GLOBAL_strMessage=strMessage;
	Sleep(MSG_REFRESH_TIME);	//����һ������Ϣʱ˯һ�������Ա��ʱ���ܲ���©��Ϣ
}
bool RobotQ::CheckAndUpdataAuth(){
	//��ȡ����ʱ��
	int64 nExpireTime;
	int64 nCurTime = (int64)time( NULL );
	HCI_ERR_CODE errCode = hci_get_auth_expire_time( &nExpireTime );
	if( errCode == HCI_ERR_NONE ){
		//��ȡ�ɹ����ж��Ƿ����
		if( nExpireTime > nCurTime ){
			//û�й���
			qDebug()<<"auth can use continue";
			return true;
		}
	}
	//��ȡ����ʱ��ʧ�ܻ��Ѿ�����
	//�ֶ����ø�����Ȩ
	errCode = hci_check_auth();
	if( errCode == HCI_ERR_NONE ){	//���³ɹ�
		qDebug()<<"check auth success";
		return true;
	}
	else{	//����ʧ��
		qDebug()<<"check auth return("<<errCode<<":"<<hci_get_error_info(errCode)<<")";
		return false;
	}
}
//��ȡcapkey����
void RobotQ::GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode){
	HCI_ERR_CODE errCode = HCI_ERR_NONE;
	CAPABILITY_ITEM *pItem = NULL;
	// ö�����е�asr����
	CAPABILITY_LIST list = {0};
	if ((errCode = hci_get_capability_list("asr", &list))!= HCI_ERR_NONE){		// û���ҵ���Ӧ��������
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
	}else if(strstr(pItem->pszCapKey, "dialog")!= NULL){
		mode = kRecogModeDialog;
	}else{
		mode = kRecogModeUnkown;
	}
	hci_free_capability_list(&list);
	return;
}

void HCIAPI RobotQ::CB_EventChange(_MUST_ _IN_ PLAYER_EVENT ePlayerEvent,_OPT_ _IN_ void * pUsrParam){
	string strEvent;
	switch ( ePlayerEvent ){
	case PLAYER_EVENT_BEGIN:strEvent = "��ʼ����";break;
	case PLAYER_EVENT_PAUSE:strEvent = "��ͣ����"; break;
	case PLAYER_EVENT_RESUME:strEvent = "�ָ�����";break;
	case PLAYER_EVENT_PROGRESS:strEvent = "���Ž���";break;
	case PLAYER_EVENT_BUFFERING:strEvent = "���Ż���";break;
	case PLAYER_EVENT_END:strEvent = "�������";break;
	case PLAYER_EVENT_ENGINE_ERROR:strEvent = "�������";break;
	case PLAYER_EVENT_DEVICE_ERROR:strEvent = "�豸����";break;
	}
}
void HCIAPI RobotQ::CB_ProgressChange (_MUST_ _IN_ PLAYER_EVENT ePlayerEvent,_MUST_ _IN_ int nStart,_MUST_ _IN_ int nStop,_OPT_ _IN_ void * pUsrParam){
	string strEvent;
	char szData[256] = {0};
	switch ( ePlayerEvent ){
	case PLAYER_EVENT_BEGIN:strEvent = "��ʼ����";break;
	case PLAYER_EVENT_PAUSE:strEvent = "��ͣ����";break;
	case PLAYER_EVENT_RESUME:strEvent = "�ָ�����";break;
	case PLAYER_EVENT_PROGRESS:sprintf( szData, "���Ž��ȣ���ʼ=%d,�յ�=%d", nStart, nStop );strEvent = szData;break;
	case PLAYER_EVENT_BUFFERING:strEvent = "���Ż���";break;
	case PLAYER_EVENT_END:strEvent = "�������";break;
	case PLAYER_EVENT_ENGINE_ERROR:strEvent = "�������";break;
	case PLAYER_EVENT_DEVICE_ERROR:strEvent = "�豸����";break;
	}
}
void HCIAPI RobotQ::CB_SdkErr( _MUST_ _IN_ PLAYER_EVENT ePlayerEvent,_MUST_ _IN_ HCI_ERR_CODE eErrorCode,_OPT_ _IN_ void * pUsrParam ){
	string strEvent;
	switch ( ePlayerEvent ){
	case PLAYER_EVENT_BEGIN:strEvent = "��ʼ����";break;
	case PLAYER_EVENT_PAUSE:strEvent = "��ͣ����";break;
	case PLAYER_EVENT_RESUME:strEvent = "�ָ�����";break;
	case PLAYER_EVENT_PROGRESS:strEvent = "���Ž���";break;
	case PLAYER_EVENT_BUFFERING:strEvent = "���Ż���";break;
	case PLAYER_EVENT_END:strEvent = "�������";break;
	case PLAYER_EVENT_ENGINE_ERROR:strEvent = "�������";break;
	case PLAYER_EVENT_DEVICE_ERROR:strEvent = "�豸����";break;
	}
}