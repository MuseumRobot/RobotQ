#include "robotq.h"

//静态全局变量
QString RobotQ::GLOBAL_strMessage;
RECORDER_EVENT RobotQ::GLOBAL_eRecorderEvent;
bool RobotQ::GLOBAL_CommandValid;

RobotQ::RobotQ(QWidget *parent, Qt::WFlags flags):QMainWindow(parent, flags){
	ui.setupUi(this);
	connect(ui.btnStart,SIGNAL(clicked(bool)),this,SLOT(OnStartClicked(bool)));
	connect(ui.btnEnd,SIGNAL(clicked(bool)),this,SLOT(OnEndClicked(bool)));
	Init();
	m_timerId=startTimer(MSG_REFRESH_TIME);	//计时器查询识别状态
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
	if(IS_RECORDER_CONTINUE)	//是否连续录音
		startConfig += ",continuous=yes";	
	if ( m_RecogMode == kRecogModeDialog ){
		startConfig +=",intention=weather;joke;story;baike;calendar;translation;news"; 
	}
	eRet = hci_asr_recorder_start(startConfig.c_str(),"");
	if (RECORDER_ERR_NONE != eRet){
		QString strErrMessage;
		strErrMessage.sprintf( "开始录音失败,错误码%d", eRet );
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
		strErrMessage.sprintf( "终止录音失败,错误码%d", eRet );
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
	GLOBAL_CommandValid = FALSE;	//初始化执行步骤不附加到显示框中
	m_recordingFlag = FALSE;
	m_recordingFileName = "recording.pcm";
	m_recordingFile = NULL;
	// 获取AccountInfo单例
	AccountInfo *account_info = AccountInfo::GetInstance();
	// 账号信息读取
	string account_info_file = "testdata/AccountInfo.txt";
	bool account_success = account_info->LoadFromFile(account_info_file);
	if (!account_success){
		QString str=sprintf("AccountInfo read from %s failed\n",account_info_file.c_str());
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
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
	//init_config += ",logfilepath=" + account_info->logfile_path();   //日志的路径（可以不要日志）
	init_config += ",logfilesize=1024000,loglevel=5";	// 其他配置使用默认值，不再添加，如果想设置可以参考开发手册
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
	// 检测授权,必要时到云端下载授权。此处需要注意的是，这个函数只是通过检测授权是否过期来判断是否需要进行
	// 获取授权操作，如果在开发调试过程中，授权账号中新增了灵云sdk的能力，请到hci_init传入的authPath路径中
	// 删除HCI_AUTH文件。否则无法获取新的授权文件，从而无法使用新增的灵云能力。
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
	//asr_recorder初始化
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
		str.sprintf("录音机初始化失败，错误码%d\n",eRet);
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
		return false;
	}
	//tts_player初始化
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
	QString str="你好呀!";
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

void HCIAPI RobotQ::RecorderRecogProcess(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult,void *pUsrParam){
		RobotQ *dlg=(RobotQ *)pUsrParam;
		QString strMessage = "";
		QString add;
		if( psAsrRecogResult->uiResultItemCount > 0 ){
			unsigned char* pucUTF8 = NULL;
			HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
			add.sprintf( "识别中间结果: %s", pucUTF8 );
			add=QString::fromLocal8Bit(add.toStdString().c_str());
			strMessage+=add;
			HciExampleComon::FreeConvertResult( pucUTF8 );
			pucUTF8 = NULL;
		}else{
			strMessage.append( "*****无识别结果*****" );
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
		add.sprintf( "系统错误:%d", eErrorCode);
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
			add.sprintf("识别时间:%dms", (int)endClock - (int)dlg->m_startClock);
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
			strMessage+="识别结果:";
			strMessage+=add;
			HciExampleComon::FreeConvertResult( pucUTF8 );
			pucUTF8 = NULL;
		}else{
			strMessage.append( "*****无识别结果*****" );
		}
		dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}

//自定义槽函数
void RobotQ::OnShowStatus(RECORDER_EVENT eRecorderEvent, QString strMessage){
	AppendMessage(strMessage);	//在文本框中附加显示信息
	RECORDER_EVENT eEvent = eRecorderEvent;
	switch( eEvent ){
		// 若是开始录音、听到声音或者开始识别，则使按钮不可用
	case RECORDER_EVENT_BEGIN_RECORD:
	case RECORDER_EVENT_BEGIN_RECOGNIZE:		
	case RECORDER_EVENT_HAVING_VOICE:
		ui.btnStart->setEnabled(false);
		ui.btnEnd->setEnabled(true);
		break;
		// 状态保持不变
	case RECORDER_EVENT_ENGINE_ERROR:
		break;
		// 录音结束、任务结束
	case RECORDER_EVENT_END_RECORD:
	case RECORDER_EVENT_TASK_FINISH:
		ui.btnStart->setEnabled(true);
		ui.btnEnd->setEnabled(false);
		break;
		// 识别结束
	case RECORDER_EVENT_RECOGNIZE_COMPLETE:
		if(IS_RECORDER_CONTINUE==FALSE){
			ui.btnStart->setEnabled(true);
			ui.btnEnd->setEnabled(false);
		}
		break;
		// 其他状态，包括未听到声音或者发生错误等，则恢复按钮可用
	default:
		char buff[32];
		sprintf(buff, "Default Event:%d", eEvent);
		AppendMessage(QString(buff));
		ui.btnStart->setEnabled(true);
		ui.btnEnd->setEnabled(false);
	}
}

//自定义工具函数
void RobotQ::AppendMessage(QString strMsg){	//AppendMessage函数目的是将新加的文字补充填写到状态文本框中(传0长度字符则清空)
	QString strMessage = "";
	strMessage=ui.textStatus->toPlainText();
	int nMessageLenMax = 1024;		//状态文本框最大承受1024个字符
	if(strMessage.length() > nMessageLenMax){
		strMessage = strMessage.left(nMessageLenMax);	//如果实际字符长度超过预设最大值，则截取最新部分予以显示，旧的舍弃
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
	fflush(m_recordingFile);	//更新缓冲区，将缓冲区数据强制写入文件
}
void RobotQ::PostRecorderEventAndMsg(RECORDER_EVENT eRecorderEvent, QString strMessage){
	GLOBAL_CommandValid=TRUE;
	GLOBAL_eRecorderEvent=eRecorderEvent;
	GLOBAL_strMessage=strMessage;
	Sleep(MSG_REFRESH_TIME);	//发送一段新消息时睡一个周期以便计时器能不遗漏信息
}
bool RobotQ::CheckAndUpdataAuth(){
	//获取过期时间
	int64 nExpireTime;
	int64 nCurTime = (int64)time( NULL );
	HCI_ERR_CODE errCode = hci_get_auth_expire_time( &nExpireTime );
	if( errCode == HCI_ERR_NONE ){
		//获取成功则判断是否过期
		if( nExpireTime > nCurTime ){
			//没有过期
			qDebug()<<"auth can use continue";
			return true;
		}
	}
	//获取过期时间失败或已经过期
	//手动调用更新授权
	errCode = hci_check_auth();
	if( errCode == HCI_ERR_NONE ){	//更新成功
		qDebug()<<"check auth success";
		return true;
	}
	else{	//更新失败
		qDebug()<<"check auth return("<<errCode<<":"<<hci_get_error_info(errCode)<<")";
		return false;
	}
}
//获取capkey属性
void RobotQ::GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode){
	HCI_ERR_CODE errCode = HCI_ERR_NONE;
	CAPABILITY_ITEM *pItem = NULL;
	// 枚举所有的asr能力
	CAPABILITY_LIST list = {0};
	if ((errCode = hci_get_capability_list("asr", &list))!= HCI_ERR_NONE){		// 没有找到相应的能力。
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
	case PLAYER_EVENT_BEGIN:strEvent = "开始播放";break;
	case PLAYER_EVENT_PAUSE:strEvent = "暂停播放"; break;
	case PLAYER_EVENT_RESUME:strEvent = "恢复播放";break;
	case PLAYER_EVENT_PROGRESS:strEvent = "播放进度";break;
	case PLAYER_EVENT_BUFFERING:strEvent = "播放缓冲";break;
	case PLAYER_EVENT_END:strEvent = "播放完毕";break;
	case PLAYER_EVENT_ENGINE_ERROR:strEvent = "引擎出错";break;
	case PLAYER_EVENT_DEVICE_ERROR:strEvent = "设备出错";break;
	}
}
void HCIAPI RobotQ::CB_ProgressChange (_MUST_ _IN_ PLAYER_EVENT ePlayerEvent,_MUST_ _IN_ int nStart,_MUST_ _IN_ int nStop,_OPT_ _IN_ void * pUsrParam){
	string strEvent;
	char szData[256] = {0};
	switch ( ePlayerEvent ){
	case PLAYER_EVENT_BEGIN:strEvent = "开始播放";break;
	case PLAYER_EVENT_PAUSE:strEvent = "暂停播放";break;
	case PLAYER_EVENT_RESUME:strEvent = "恢复播放";break;
	case PLAYER_EVENT_PROGRESS:sprintf( szData, "播放进度：起始=%d,终点=%d", nStart, nStop );strEvent = szData;break;
	case PLAYER_EVENT_BUFFERING:strEvent = "播放缓冲";break;
	case PLAYER_EVENT_END:strEvent = "播放完毕";break;
	case PLAYER_EVENT_ENGINE_ERROR:strEvent = "引擎出错";break;
	case PLAYER_EVENT_DEVICE_ERROR:strEvent = "设备出错";break;
	}
}
void HCIAPI RobotQ::CB_SdkErr( _MUST_ _IN_ PLAYER_EVENT ePlayerEvent,_MUST_ _IN_ HCI_ERR_CODE eErrorCode,_OPT_ _IN_ void * pUsrParam ){
	string strEvent;
	switch ( ePlayerEvent ){
	case PLAYER_EVENT_BEGIN:strEvent = "开始播放";break;
	case PLAYER_EVENT_PAUSE:strEvent = "暂停播放";break;
	case PLAYER_EVENT_RESUME:strEvent = "恢复播放";break;
	case PLAYER_EVENT_PROGRESS:strEvent = "播放进度";break;
	case PLAYER_EVENT_BUFFERING:strEvent = "播放缓冲";break;
	case PLAYER_EVENT_END:strEvent = "播放完毕";break;
	case PLAYER_EVENT_ENGINE_ERROR:strEvent = "引擎出错";break;
	case PLAYER_EVENT_DEVICE_ERROR:strEvent = "设备出错";break;
	}
}