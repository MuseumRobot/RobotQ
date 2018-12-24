#include "stdafx.h"
#include "robotq.h"

//静态全局变量
QString RobotQ::GLOBAL_strMessage;
RECORDER_EVENT RobotQ::GLOBAL_eRecorderEvent;
bool RobotQ::GLOBAL_CommandValid;

RobotQ::RobotQ(QWidget *parent, Qt::WFlags flags):QDialog(parent, flags){
	ui.setupUi(this);
	m_popup_image = new PopupDialog(0);
	connect(ui.btnStart,SIGNAL(clicked()),this,SLOT(OnStartClicked()));
	connect(ui.btnEnd,SIGNAL(clicked()),this,SLOT(OnEndClicked()));
	connect(ui.btnQuery,SIGNAL(clicked()),this,SLOT(OnQueryClicked()));
	connect(ui.btnStopspeak,SIGNAL(clicked()),this,SLOT(OnStopSpeak()));
	Init();
	m_timerId=startTimer(MSG_REFRESH_TIME);	//计时器查询识别状态
}
RobotQ::~RobotQ(){
	Uninit();
}
int RobotQ::OnStartClicked(){
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
int RobotQ::OnEndClicked(){
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
int RobotQ::OnStopSpeak(){
	hci_tts_player_stop();
	return 0;
}
int RobotQ::OnQueryClicked(){
	QString question = ui.comSpeaklist->currentText();
	QString answer = SearchQuery(question);
	AppendMessage(answer);
	RobotQSpeak(answer);
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
	isAuthReady = FALSE;
	isASRReady = FALSE;
	isTTSReady = FALSE;
	AccountInfo *account_info = AccountInfo::GetInstance();	// 获取AccountInfo单例
	string account_info_file = "testdata/AccountInfo.txt";// 账号信息读取
	bool account_success = account_info->LoadFromFile(account_info_file);
	// SYS初始化
	HCI_ERR_CODE errCode = HCI_ERR_NONE;
	// 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	string init_config = "";
	init_config += "appKey=" + account_info->app_key();              //灵云应用序号
	init_config += ",developerKey=" + account_info->developer_key(); //灵云开发者密钥
	init_config += ",cloudUrl=" + account_info->cloud_url();         //灵云云服务的接口地址
	init_config += ",authpath=" + account_info->auth_path();         //授权文件所在路径，保证可写
	init_config += ",logfilesize=1024000,loglevel=5";	// 其他配置使用默认值，不再添加，如果想设置可以参考开发手册
	errCode = hci_init( init_config.c_str() );
	// 检测授权,必要时到云端下载授权。此处需要注意的是，这个函数只是通过检测授权是否过期来判断是否需要进行
	// 获取授权操作，如果在开发调试过程中，授权账号中新增了灵云sdk的能力，请到hci_init传入的authPath路径中
	// 删除HCI_AUTH文件。否则无法获取新的授权文件，从而无法使用新增的灵云能力。
	if (CheckAndUpdataAuth()){
		isAuthReady = TRUE;
	}else{
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
	if (eRet == RECORDER_ERR_NONE){
		isASRReady = TRUE;
	}else{
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
	if(eReti==RECORDER_ERR_NONE){
		isTTSReady = TRUE;
		QString str="你好呀!";
		RobotQSpeak(str);
	}else{
		return false;
	}
	return true;
}

bool RobotQ::Uninit(void){
	HCI_ERR_CODE eRet = HCI_ERR_NONE;
	RECORDER_ERR_CODE eRecRet;
	hci_tts_player_stop();
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
	{"RECORDER_EVENT_BEGIN_RECORD",         "开始聆听..."},
	{"RECORDER_EVENT_HAVING_VOICE",         "听到声音 检测到始端的时候会触发该事件"},
	{"RECORDER_EVENT_NO_VOICE_INPUT",       "没有听到声音"},
	{"RECORDER_EVENT_BUFF_FULL",            "缓冲区已填满"},
	{"RECORDER_EVENT_END_RECORD",           "聆听完毕！"},
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
	if(eRecorderEvent == RECORDER_EVENT_BEGIN_RECORD || eRecorderEvent == RECORDER_EVENT_END_RECORD){
		//当开始录音和结束录音时给予文字提示
		QString strMessage(g_sStatus[eRecorderEvent].pszComment);
		dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
	}
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
			//if(psAsrRecogResult->uiResultItemCount>0)
			//	dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);	//如果想显示识别时间则取消此行注释
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
			//cJSON提取有效内容
			QString QResult;
			QString QAnswer;
			Json_Explain(strMessage,QResult,QAnswer);
			if(QResult.contains("黑龙江省博物馆",Qt::CaseInsensitive)){
				QAnswer = "黑龙江省博物馆是省级综合性博物馆，2012年被评为国家一级博物馆，是黑龙江省收藏历史文物、艺术品和动、植物标本的中心，是地方史和自然生态的研究中心之一，也是宣传地方历史文化和自然资源的重要场所。黑龙江省博物馆主楼始建于1906年，是一座欧洲巴洛克式建筑，为原俄罗斯商场旧址，现为国家一级保护建筑，他就像一位历经沧桑的百岁老人，见证着黑龙江省博物馆的历史变迁。1923年6月12日，东省文物研究会陈列所在现址举行成立典礼并对外开放。此后的30年间，曾历经东省特别区文物研究会博物馆、伪北满特别区文物研究所、伪大陆科学院哈尔滨分院博物馆、哈尔滨工业大学常设运输经济陈列馆、松江省科学博物馆、松江省博物馆等几个阶段。1954年8月，随着松江省并入黑龙江省，松江省博物馆与黑龙江省博物馆筹备处合并，从此，黑龙江省博物馆正式宣告成立。1962年中国科学院院长郭沫若为本馆书写馆名。黑龙江省博物馆馆藏丰富，现有各类藏品62万余件。本馆设有“黑龙江历史文物陈列——以肃慎族系遗存为中心”、“黑龙江俄侨文化文物展”、“自然陈列”、“铁笔翰墨——邓散木艺术专题陈列”、“墨韵冰魂北国情——于志学艺术馆藏精品展”五大基本陈列，以及各种主题和形式的临时陈列。我们对大家的到来表示诚挚的欢迎，希望大家为黑龙江省博物馆多多提出宝贵的意见和建议，谢谢！";
			}else if(QResult.contains("哈尔滨",Qt::CaseInsensitive) && QResult.contains("介",Qt::CaseInsensitive)){
				QAnswer = "素有“冰城夏都”美誉的历史文化名城哈尔滨，有着众多著名学府，如：哈尔滨工业大学、哈尔滨工程大学、黑龙江大学、东北农业大学、哈尔滨师范大学等。其中，哈尔滨工业大学（简称哈工大），隶属于工业和信息化部，拥有哈尔滨、威海、深圳三个校区，是一所以理工为主，理、工、管、文、经、法、艺等多学科协调发展的国家重点大学。 学校始建于1920年，1951年被确定为全国学习国外高等教育办学模式的两所样板大学之一，1954年进入国家首批重点建设的6所高校行列（京外唯一一所），是新中国第一所本科五年制、研究生三年制、毕业生直接被授予工程师称号的理工科大学，被誉为工程师的摇篮。学校于1996年进入国家“211工程”首批重点建设高校，1999年被确定为国家首批按照世界知名高水平大学目标重点建设的9所大学之一，2017年入选“双一流”建设A类高校名单。";
			}
			strMessage="小灵的回答:" + QAnswer  + "\n" + "您的提问:"+ QResult;
			unsigned char* pszUTF8 = NULL;
			HciExampleComon::GBKToUTF8( (unsigned char*)QAnswer.toStdString().c_str(), &pszUTF8 );
			string startConfig = "property=cn_xiaokun_common,tagmode=none,capkey=tts.cloud.wangjing";
			PLAYER_ERR_CODE eRetk = hci_tts_player_start( (const char*)pszUTF8, startConfig.c_str() );
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

}
void RobotQ::PostRecorderEventAndMsg(RECORDER_EVENT eRecorderEvent, QString strMessage){
	GLOBAL_CommandValid=TRUE;
	GLOBAL_eRecorderEvent=eRecorderEvent;
	GLOBAL_strMessage=strMessage;
	QTest::qSleep(MSG_REFRESH_TIME);	//发送一段新消息时睡一个周期以便计时器能不遗漏信息
}
bool RobotQ::CheckAndUpdataAuth(){
	//获取过期时间
	int64 nExpireTime;
	int64 nCurTime = (int64)time( NULL );
	HCI_ERR_CODE errCode = hci_get_auth_expire_time( &nExpireTime );
	if( errCode == HCI_ERR_NONE ){
		//获取成功则判断是否过期
		if( nExpireTime > nCurTime ){	//没有过期
			return true;
		}
	}
	//获取过期时间失败或已经过期
	//手动调用更新授权
	errCode = hci_check_auth();
	if( errCode == HCI_ERR_NONE ){	//更新成功
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
void RobotQ::RobotQSpeak(QString str){
	unsigned char* pszUTF8 = NULL;
	HciExampleComon::GBKToUTF8( (unsigned char*)str.toStdString().c_str(), &pszUTF8 );
	string startConfig = "property=cn_xiaokun_common,tagmode=none,capkey=tts.cloud.wangjing";
	hci_tts_player_stop();	//后发的语音指令会将之前的语音指令覆盖
	QTest::qSleep(100);		//发送stop指令后留有短暂时间以供stop命令执行完成，否则无法start下一句
	PLAYER_ERR_CODE eRetk = hci_tts_player_start( (const char*)pszUTF8, startConfig.c_str() );
}

int RobotQ::Json_Explain (QString buf,QString& Qresult,QString& Qanswer){
	QString domain = Json_SearchKeyword(buf,"domain");
	if(domain == "weather"){
		Qanswer = "今天天气：" + Json_SearchKeyword(buf,"description");
	}else if(domain == "calendar"){
		Qanswer = "今天是" + Json_SearchKeyword(buf,"date_gongli") + "，" + Json_SearchKeyword(buf,"date_nongli");
	}else if(domain == "joke" || domain == "story"){
		Qanswer = Json_SearchKeyword(buf,"content\":{\"content");
	}else if(domain == "news"){
		Qanswer = Json_SearchKeyword(buf,"desc");
	}else{
		Qanswer = Json_SearchKeyword(buf,"text");
	}
	Qresult = Json_SearchKeyword(buf,"result");
	return 0;
}
QString RobotQ::Json_SearchKeyword(QString buf,QString keyword){
	QString QAnswer;
	int i=0,j=0,n=3;
	i=buf.indexOf(keyword);
	while(buf.at(i+keyword.length()+n+j) != '"'){
		QAnswer += buf.at(i+keyword.length()+n+j);
		j++;
	}
	return QAnswer;
}
QString RobotQ::SearchQuery(QString question){
	QString answer;
	int picFlag=0;
	if (question == "机器人简介"){
		answer = "大家好，我是博物馆导览服务机器人，是哈尔滨工业大学互动媒体设计与装备服务创新文化部重点实验室研发的。我可以进行展厅讲解，自动绕开前方的行人，还可以和观众朋友互动对话，您可以点击屏幕下方的开始对话按钮和我对话哦！";
	}
	if(question == "龙博概况"){
		answer = "黑龙江省博物馆是省级综合性博物馆，2012年被评为国家一级博物馆，是黑龙江省收藏历史文物、艺术品和动、植物标本的中心，是地方史和自然生态的研究中心之一，也是宣传地方历史文化和自然资源的重要场所。黑龙江省博物馆主楼始建于1906年，是一座欧洲巴洛克式建筑，为原俄罗斯商场旧址，现为国家一级保护建筑，他就像一位历经沧桑的百岁老人，见证着黑龙江省博物馆的历史变迁。1923年6月12日，东省文物研究会陈列所在现址举行成立典礼并对外开放。此后的30年间，曾历经东省特别区文物研究会博物馆、伪北满特别区文物研究所、伪大陆科学院哈尔滨分院博物馆、哈尔滨工业大学常设运输经济陈列馆、松江省科学博物馆、松江省博物馆等几个阶段。1954年8月，随着松江省并入黑龙江省，松江省博物馆与黑龙江省博物馆筹备处合并，从此，黑龙江省博物馆正式宣告成立。1962年中国科学院院长郭沫若为本馆书写馆名。黑龙江省博物馆馆藏丰富，现有各类藏品62万余件。本馆设有“黑龙江历史文物陈列——以肃慎族系遗存为中心”、“黑龙江俄侨文化文物展”、“自然陈列”、“铁笔翰墨——邓散木艺术专题陈列”、“墨韵冰魂北国情——于志学艺术馆藏精品展”五大基本陈列，以及各种主题和形式的临时陈列。我们对大家的到来表示诚挚的欢迎，希望大家为黑龙江省博物馆多多提出宝贵的意见和建议，谢谢！"; 
		ShowGif("Resources/query/1.gif");
		picFlag = 1;
	}
	if(question == "开放时间"){
		answer = "每周二至周日开馆，周一全天闭馆，节假日除外。每天9:00至16:30开馆，15:30停止发票、入场。";
	}
	if(question == "获取门票的方法"){
		answer = "黑龙江省博物馆全年实行免费开放，观众可凭身份证等有效证件原件在博物馆正门的发票处排队领取免费门票，凭门票通过安检后进入馆内参观；为合理安排团队参观，各级党政机关、企事业单位、部队、学校、社区、旅游团队等社会团体参观，须提前3天凭单位介绍信或旅行社任务派遣单到博物馆发票处预约登记，并按预约时间进行参观；60岁以上老人（凭老年优待证）、残疾人（凭残疾证）、孕妇及其他行动不便者可优先领票参观。";
	}
	if(question == "到达黑龙江省博物馆的方式/交通/地理位置"){
		answer = "黑龙江省博物馆地址：哈尔滨市南岗区红军街64号.乘坐公交车路线：乘101路、103路、108路、109路、115路；2路、6路、7 路、8路、13路、14路、16路、18路、21路、28路；32路、33路、64路、74路、89路；888路到博物馆站下车；乘104路、107路、110路、111路、112路、118路、119路、120路；10路、11路、16路；31路、55路、63路、81路、82路到省博览中心站下车。乘坐地铁路线：乘地铁1号线到博物馆站下车。";
	}
	if(question == "参观导览/陈列布局/楼层分布"){
		answer = "黑龙江省博物馆共有3层。二层展厅主要有“自然陈列”、“黑龙江历史文物陈列”；一层为“黑龙江俄侨文化文物展”、“ “铁笔翰墨——邓散木艺术专题陈列”、“墨韵冰魂北国情——于志学艺术馆藏精品展”、“每月一星”；负一层为“寒暑假特展”、“民俗展览”、“每月一县”三个临时展厅。特别提示：“黑龙江俄侨文化文物展”需从“黑龙江历史文物陈列”最后一个展厅的楼梯前往参观。";
		ShowGif("Resources/query/5.gif");
		picFlag = 1;
	}
	if(question == "免费讲解时间安排"){
		answer = "全天固定开展四场免费讲解，分别是上午9：30、10:30开讲，下午13：30、14:30开讲。开讲展厅地点为二楼自然陈列展厅。";
	}
	if(question == "免费服务/便民服务"){
		answer = "针对残障人士，在博物馆内凭身份证免费租借轮椅，方便参观；为观众提供免费寄存服务。如有需要，请到一楼大厅服务咨询台办理。";
	}
	if(question == "镇馆之宝展示/藏品欣赏/藏品展示/馆藏精品"){
		answer = "黑龙江省博物馆馆藏丰富，2014年举行了“十大镇馆之宝评选”活动，“十大镇馆之宝”分别是：金代铜坐龙、金代齐国王墓丝织品服饰、南宋《蚕织图》、唐代渤海天门军之印、披毛犀化石骨架、南宋《兰亭序》图卷、黑龙江满洲龙、金代山水人物故事镜、松花江猛犸象化石骨架、新石器时代桂叶形石器。";
		ShowGif("Resources/query/8.gif");
		picFlag = 1;
	}
	if(question == "黑龙江省博物馆志愿者基地"){
		answer = "为更好的发挥博物馆的社会教育功能，更好地为社会大众提供服务，也为各大热心于博物馆和社会服务事业的志愿者提供一个实现社会价值和个人价值的平台，黑龙江省博物馆于2010年成立了“黑龙江省文博志愿者基地”。志愿者服务分为导览志愿者及讲解志愿者，除此之外，他们的身影经常出现在各项活动当中，一直以来深受广大观众的好评。如您有意愿参与其中，可到黑龙江省博物馆官网下载“志愿者申请表“或拨打电话0451-53644151进行咨询。欢迎您的加入。";
		ShowGif("Resources/query/9.gif");
		picFlag = 1;
	}
	if(question == "文创产品/文创天地/纪念品"){
		answer = "黑龙江省博物馆设有“文化创意经营中心”，通过深入挖掘文物资源，融合社会力量众筹、众创，以此大力开发文创产品，认真贯彻让文物“活”起来的重要指示，满足广大群众把文物带回家的美好心愿。您可以在一楼“文化创意经营中心”及二楼大厅处选择您心意的产品。此外，黑龙江省博物馆在一楼还设有“龙博书苑”， “水吧”。";
		ShowGif("Resources/query/10.gif");
		picFlag = 1;
	}
	if(question == "黑龙江省博物馆休息区/座椅"){
		answer = "为了更好的服务广大观众，黑龙江省博物馆在多个区域设有休息座椅，如：二楼的“黑龙江历史文物陈列”、“自然精品展厅“，一楼的“黑龙江俄侨文化文物展”，负一层大厅。特别提示：“黑龙江俄侨文化文物展”需从“黑龙江历史文物陈列”最后一个展厅的楼梯前往参观。";
	}
	if(question == "黑龙江省博物馆厕所/洗手间/卫生间"){
		answer = "女士洗手间位于二楼楼梯口处，男士洗手间位于一楼楼梯口处。";
	}
	if(question == "社会服务项目"){
		answer = "文物及古动物化石的鉴定、修复、复制，您可到一楼大厅服务咨询台或拨打电话0451-53644151进行咨询。";
	}
	if(question == "黑龙江省博物馆活动"){
		answer = "省博举办诸多丰富多彩的活动内容，有“相约龙博”科普教育活动、“环球自然日——青少年自然科学知识挑战赛”、“青少年科普绘画大赛”、“流动博物馆”等。您可以扫描屏幕上方的二维码实时关注我们，工作人员会在“黑龙江省博物馆互动平台”上发布展陈信息及活动内容。";
		ShowGif("Resources/query/14.jpg");
		picFlag = 1;
	}
	if(question == "黑龙江省博物馆“相约龙博”课堂/科普教育活动"){
		answer = "“相约龙博”课堂，旨让青少年在课余时间能在愉快地氛围中收获知识，增长能力，培养青少年的综合素质。工作人员根据省博馆藏资源精心策划活动内容，其中包括：“历史的记忆”、“动物大联盟”、“走进传承”、“玩艺坊”、“小花匠的植物王国”、“物质世界的真相”六大系列。“相约龙博”课堂设在二楼大厅，每逢周末及节假日都会组织开展精彩的活动内容。";
		ShowGif("Resources/query/15.gif");
		picFlag = 1;
	}
	if(question == "环球自然日——青少年自然科学知识挑战赛"){
		answer = "“环球自然日——青少年自然科学知识挑战赛”，是由美国著名慈善家肯尼斯·尤金·贝林创办，环球健康与教育基金会发起，用以激发中小学生对于自然科学的兴趣，并提高其研究、分析和交往能力的课外科普教育活动。此项活动于2012年进入中国，2014年起黑龙江赛区启动，由省博承办。";
		ShowGif("Resources/query/16.gif");
		picFlag = 1;
	}
	if(question == "环球自然日——青少年科普绘画大赛"){
		answer = "“环球自然日——青少年科普绘画大赛”旨在带动更多的青少年走进博物馆，通过结合博物馆的资源优势，让他们近距离观察并研究相关自然科学知识，同时，将自然和艺术融合，使他们在活动过程中感受自然之美，激发自然科学的学习热情。";
		ShowGif("Resources/query/17.gif");
		picFlag = 1;
	}
	if(question == "黑龙江省博物馆流动博物馆"){
		answer = "黑龙江省博物馆自2014年起成立流动博物馆，将展览带到大众身边，足不出户就能够看到展览。截止目前，有“远离毒品、远离邪教、远离赌博、倡导绿色上网”、“昆虫世界中的铠甲勇士——锹甲”、“黑龙江省中药材特展”三个主题展览，曾走进多所院校、社区等，深受广大群众好评。";
		ShowGif("Resources/query/18.gif");
		picFlag = 1;
	}
	if(question == "黑龙江省博物馆分馆/枫叶小镇奥特莱斯分馆"){
		answer = "黑龙江省博物馆枫叶小镇分馆是以世界珍稀野生动物标本展、古生物化石展和熊展三大基本陈列为主的自然类博物馆，大部分展品是由世界著名企业家、慈善家肯尼斯.尤金.贝林先生向黑龙江省博物馆捐赠的。贝林先生捐赠的动物标本，有来自北美洲及非洲的大型珍稀动物标本，有产自美国的美洲貂、敏狐、浣熊、戴氏盘羊、北极狐等，还有产自南非的狞猫、非洲野猪等，这些都是黑龙江地区罕见的珍稀标本。展览的目的是在普及自然科学知识，宣传、保护野生动物的同时，唤醒公众对自然界的热爱及对保护环境重要意义的认识。";
		ShowGif("Resources/query/19.jpg");
		picFlag = 1;
	}
	if(question == "哈尔滨简介"){
		answer = "哈尔滨为黑龙江省省会、副省级市、特大城市、中国东北地区中心城市之一，是东北北部交通、政治、经济、文化、金融中心，也是中国省辖市中陆地管辖面积最大、户籍人口居第三位的特大城市，地处中国东北平原东北部地区、黑龙江省南部，国家重要的制造业基地。截至2015年，哈尔滨总面积53186平方千米，市辖区面积10198平方公里，辖9个市辖区、7个县，代管2个县级市，常住人口1066.5万人。哈尔滨地处东北亚中心地带，被誉为欧亚大陆桥的明珠，是第一条欧亚大陆桥和空中走廊的重要枢纽，也是中国历史文化名城、热点旅游城市和国际冰雪文化名城。是国家战略定位的“沿边开发开放中心城市”、“东北亚区域中心城市”及“对俄合作中心城市”。哈尔滨还具有“东方小巴黎”、“东方莫斯科“之称。";
	}
	if(question == "哈尔滨景区/景点介绍"){
		answer = "哈尔滨市内有多个旅游景点，如：中央大街、太阳岛风景区、冰雪大世界等。中央大街始建于1898年，初称“中国大街”。1925年改称“中央大街”，后来发展成为东北北部最繁华商业街。大街北起松花江防洪纪念塔，南至经纬街。全街建有欧式及仿欧式建筑71栋，并汇集了文艺复兴、巴洛克、折衷主义及现代多种风格市级保护建筑13栋，是国内罕见的一条建筑艺术长廊。它是亚洲最大最长的步行街之一。太阳岛风景区，中国唯一坐落于城市中心的江漫湿地型风景名胜区，是国家级风景名胜区、国家AAAAA级旅游景区、国家水利风景区。被国家建设部评为“中国人居环境范例奖”，被联合国友好理事会评为“联合国FOUN生态示范岛”。它以独特的北方自然风光和浪漫的欧陆风情著称。一年一度的“中国·哈尔滨国际雪雕艺术博览会”及许多国际国内的雪雕比赛都在这里举办。冰雪大世界始创于1999年，是由哈尔滨市政府为迎接千年庆典神州世纪游活动，充分发挥哈尔滨的冰雪资源优势，进一步开拓构思，而隆重推出规模空前的超大型冰雪艺术精品工程。哈尔滨冰雪大世界，向世人展示了北方名城哈尔滨冰雪文化和冰雪旅游的独特魅力。每年冬季来此游玩的国内外游人络绎不绝。哈尔滨冰雪大世界是世界上最大的室外以冰雪为主题的超大型娱乐工程。";
		ShowGif("Resources/query/21.gif");
		picFlag = 1;
	}
	if(question == "哈尔滨餐饮/饮食/特产推荐介绍"){
		answer = "哈尔滨市居民以汉族为主，其中多为山东与河北省移民。旧时的哈尔滨，外国侨民比例较多，其中俄侨占多数。外侨生活方式与习俗对哈尔滨人的文化、饮食习俗有一定影响。哈尔滨人旧时多以大饼子（玉米面贴饼子）为主食。原当地居民喜食大馇子、小米饭和炖菜；山东人喜吃面食和鱼；河北人喜食米饭，爱喝高粱米粥。哈尔滨人受少数民族饮食习俗影响，喜欢吃黏糕和黏豆包、“列巴”（一种大面包）、红肠等，喜欢喝“苏波汤”（俄式红汤）。大列巴，又叫大面包，被称为哈尔滨一绝，是哈尔滨独特的风味食品。哈尔滨秋林公司和华梅西餐厅生产的大面包都已有七、八十年历史。这种大面包为圆形，有5斤重，是面包之冠。味道也别具芳香，具有传统的欧洲风味。出炉后的大面包，外皮焦脆，内瓤松软，香味独特，又宜存放，是老少皆宜的方便食品。哈尔滨最经典的吃的就是红肠和干肠。红肠原本来自俄罗斯，最普通的，也是最著名最传统的红肠风味是“里道斯”风味，就是大蒜味的，下酒极佳，配上“格瓦斯”（俄语译音，俄式饮料，用面包干发酵酿制而成，颜色近似啤酒而略呈红色，酸甜适度，近似酸）。红肠可以夹在列巴里，是很主要的肉食品种。锅包肉，原名“锅爆肉”，是正宗冰城美食，出自哈尔滨道台府府尹杜学赢专用厨师，“滨江膳祖”——郑兴文之手。由于用急火快炒，把铁锅烧热，把汁淋到锅里，浸透到肉里，起名叫“锅爆肉”。";
	}
	if(question == "哈尔滨学府"){
		answer = "素有“冰城夏都”美誉的历史文化名城哈尔滨，有着众多著名学府，如：哈尔滨工业大学、哈尔滨工程大学、黑龙江大学、东北农业大学、哈尔滨师范大学等。其中，哈尔滨工业大学（简称哈工大），隶属于工业和信息化部，拥有哈尔滨、威海、深圳三个校区，是一所以理工为主，理、工、管、文、经、法、艺等多学科协调发展的国家重点大学。 学校始建于1920年，1951年被确定为全国学习国外高等教育办学模式的两所样板大学之一，1954年进入国家首批重点建设的6所高校行列（京外唯一一所），是新中国第一所本科五年制、研究生三年制、毕业生直接被授予工程师称号的理工科大学，被誉为工程师的摇篮。学校于1996年进入国家“211工程”首批重点建设高校，1999年被确定为国家首批按照世界知名高水平大学目标重点建设的9所大学之一，2017年入选“双一流”建设A类高校名单。";
	}
	if(picFlag == 0){
		ShowGif("Resources/图片/七寸屏幕.gif");
	}
	return answer;
}
void RobotQ::ShowGif(QString filepath){
	QDesktopWidget* desktop = QApplication::desktop();
	int N = desktop->screenCount();
	QMovie *movie = new QMovie(filepath);
	m_popup_image->setGeometry(desktop->screenGeometry(0));
	m_popup_image->ui.popup_image->setMovie(movie);
	m_popup_image->show();
	m_popup_image->resize(800,600);
	movie->start();
}