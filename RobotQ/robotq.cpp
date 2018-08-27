﻿#include "robotq.h"

//静态全局变量
QString RobotQ::GLOBAL_strMessage;
RECORDER_EVENT RobotQ::GLOBAL_eRecorderEvent;
bool RobotQ::GLOBAL_CommandValid;

RobotQ::RobotQ(QWidget *parent, Qt::WFlags flags):QDialog(parent, flags){
	ui.setupUi(this);
	m_popup_image = new PopupDialog(this);
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
			char buf[10000] = {NULL};
			char result[10000]={NULL};
			char answer[10000]={NULL};
			strcpy(buf,strMessage.toStdString().c_str());
			Json_Explain(buf,result,answer);
			QString QResult(result);
			QString QAnswer(answer);
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
int RobotQ::Json_Explain (char buf[],char result[],char answer[]){  
	int n=strlen(buf);
	if(n>0){
		for(int i=0;i<n;i++){
			if(buf[i]=='u'&&buf[i+1]=='l'&&buf[i+2]=='t'){
				int j=0;
				while(buf[i+6+j]!='"')
					result[j++]=buf[i+6+j];
			}
			if(buf[i]=='n'&&buf[i+1]=='t'&&buf[i+2]=='"'){
				int j=0;
				while(buf[i+13+j]!='"')
					answer[j++]=buf[i+13+j];
				break;
			}

		}
	}	
	return 0;  
}  
QString RobotQ::SearchQuery(QString question){
	QString answer;
	if(question == "博物馆简介")answer = "黑龙江省博物馆是省级综合性博物馆，2012年被评为国家一级博物馆，是黑龙江省收藏历史文物、艺术品和动、植物标本的中心，是地方史和自然生态的研究中心之一，也是宣传地方历史文化和自然资源的重要场所。"; 
	if(question == "黑龙江省文博志愿者基地")answer = "为更好的发挥博物馆的社会教育功能，更好地为社会大众提供服务，也为各大热心于博物馆和社会服务事业的志愿者提供一个实现社会价值和个人价值的平台，黑龙江省博物馆于2010年成立了“黑龙江省文博志愿者基地”。志愿者服务分为导览志愿者及讲解志愿者，除此之外，他们的身影经常出现在各项活动当中，一直以来深受广大观众的好评。"; 
	if(question == "黑龙江省博物馆流动博物馆")answer = "黑龙江省博物馆自2014年起成立流动博物馆，将展览带到大众身边，足不出户就能够看到展览。截止目前，有“远离毒品、远离邪教、远离赌博、倡导绿色上网”、“昆虫世界中的铠甲勇士——锹甲”、“黑龙江省中药材特展”三个主题展览，曾走进多所院校、社区等，深受广大群众好评。"; 
	if(question == "环球自然日——青少年科普绘画大赛")answer = "“环球自然日——青少年科普绘画大赛”旨在带动更多的青少年走进博物馆，通过结合博物馆的资源优势，让他们近距离观察并研究相关自然科学知识，同时，将自然和艺术融合，使他们在活动过程中感受自然之美，激发自然科学的学习热情。"; 
	if(question == "环球自然日——青少年自然科学知识挑战赛")answer = "“环球自然日——青少年自然科学知识挑战赛”，是由美国著名慈善家肯尼斯尤金贝林创办，环球健康与教育基金会发起，用以激发中小学生对于自然科学的兴趣，并提高其研究、分析和交往能力的课外科普教育活动。此项活动于2012年进入中国，2014年起黑龙江赛区启动，由省博承办。"; 
	if(question == "黑龙江省博物馆“相约龙博”课堂 ")answer = "“相约龙博”课堂，旨让青少年在课余时间能在愉快地氛围中收获知识，增长能力，培养青少年的综合素质。工作人员根据省博馆藏资源精心策划活动内容，其中包括：“历史的记忆”、“动物大联盟”、“走进传承”、“玩艺坊”、“小花匠的植物王国”、“物质世界的真相”六大系列。“相约龙博”课堂设在二楼大厅，每逢周末及节假日都会组织开展精彩的活动内容。"; 
	if(question == "黑龙江省博物馆有哪些活动？"){
		answer = "省博举办诸多丰富多彩的活动内容，有“相约龙博”科普教育活动、“环球自然日——青少年自然科学知识挑战赛”、“青少年科普绘画大赛”、“流动博物馆”等。您可以扫描屏幕上方的二维码实时关注我们，工作人员会在“黑龙江省博物馆互动平台”上发布展陈信息及活动内容。"; 
		m_popup_image->ui.popup_image->setPixmap(QPixmap("Resources/最新无水印版二维码.jpg"));
		m_popup_image->show();
		m_popup_image->resize(681,322);
	}
	if(question == "社会服务项目")answer = "文物及古动物化石的鉴定、修复、复制、咨询。"; 
	if(question == "文创天地")answer = "黑龙江省博物馆还设有“龙博书苑”，“文化创意经营中心”、“水吧”等。"; 
	if(question == "便民服务")answer = "针对残障人士，在博物馆内凭身份证免费租借轮椅，方便参观；为观众提供免费寄存服务。"; 
	if(question == "洗手间在哪")answer = "女士洗手间位于二楼楼梯口处，男士洗手间位于一楼楼梯口处。"; 
	if(question == "黑龙江省博物馆镇馆之宝有哪些？"){
		answer = "黑龙江省博物馆馆藏丰富，2014年举行了“十大镇馆之宝评选”活动，“十大镇馆之宝”分别是：金代铜坐龙、金代齐国王墓丝织品服饰、南宋《蚕织图》、唐代渤海天门军之印、披毛犀化石骨架、南宋《兰亭序》图卷、黑龙江满洲龙、金代山水人物故事镜、松花江猛犸象化石骨架、新石器时代桂叶形石器。"; 
		m_popup_image->ui.popup_image->setPixmap(QPixmap("Resources/十大镇馆之宝之一：金代铜坐龙.jpg"));
		m_popup_image->show();
		m_popup_image->resize(399,600);
	}
	if(question == "免费讲解")answer = "上午9：30开讲，下午14:30开讲。开讲展厅地点为二楼自然陈列展厅。"; 
	if(question == "黑龙江省博物馆有几层，都有哪些展厅")answer = "黑龙江省博物馆共有3层。二层展厅主要有“自然陈列”、“黑龙江历史文物陈列”；一层为“黑龙江俄侨文化文物展”、“邓散木艺术专题陈列”、“每月一星”；负一层为“寒暑假特展”、“民俗展览”、“每月一县”三个临时展厅。"; 
	if(question == "黑龙江省博物馆开闭馆时间")answer = "每周二至周日开馆，周一全天闭馆，节假日除外。冬令时：10月8日-3月31日，9:00至16:00，15:00停止发票。夏令时：4月1日-10月7日，9:00至16:30，15:30停止发票。"; 
	//if(question == )answer = ; 
	return answer;
}