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

//ASR函数*******************************
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

bool RobotQ::Init(){	
	m_recordingFlag = FALSE;
	m_recordingFileName = "recording.pcm";
	m_recordingFile = NULL;
	//SetDlgItemText( IDC_EDIT_SAVE_RECORDING_FILE, m_recordingFileName );
	//UpdateData(FALSE);

	// 获取AccountInfo单例
	AccountInfo *account_info = AccountInfo::GetInstance();
	// 账号信息读取
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

	// SYS初始化
	HCI_ERR_CODE errCode = HCI_ERR_NONE;
	// 配置串是由"字段=值"的形式给出的一个字符串，多个字段之间以','隔开。字段名不分大小写。
	string init_config = "";
	init_config += "appKey=" + account_info->app_key();              //灵云应用序号
	init_config += ",developerKey=" + account_info->developer_key(); //灵云开发者密钥
	init_config += ",cloudUrl=" + account_info->cloud_url();         //灵云云服务的接口地址
	init_config += ",authpath=" + account_info->auth_path();         //授权文件所在路径，保证可写
	init_config += ",logfilepath=" + account_info->logfile_path();   //日志的路径（可以不要日志）
	init_config += ",logfilesize=1024000,loglevel=5";
	// 其他配置使用默认值，不再添加，如果想设置可以参考开发手册
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


	// 检测授权,必要时到云端下载授权。此处需要注意的是，这个函数只是通过检测授权是否过期来判断是否需要进行
	// 获取授权操作，如果在开发调试过程中，授权账号中新增了灵云sdk的能力，请到hci_init传入的authPath路径中
	// 删除HCI_AUTH文件。否则无法获取新的授权文件，从而无法使用新增的灵云能力。
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

	// capkey属性获取
	m_RecogType = kRecogTypeUnkown;
	m_RecogMode = kRecogModeUnkown;
	GetCapkeyProperty(account_info->cap_key(),m_RecogType,m_RecogMode);

	if( m_RecogType == kRecogTypeCloud && m_RecogMode == kRecogModeGrammar ){
		// 云端语法暂时不支持实时识别
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
	//initConfig		+= ",initCapkeys=asr.local.grammar";			      //初始化本地引擎

	eRet = hci_asr_recorder_init( initConfig.c_str(), &call_back);
	if (eRet != RECORDER_ERR_NONE){
		hci_release();
		//strErrorMessage.Format( "录音机初始化失败,错误码%d", eRet);
		//MessageBox( strErrorMessage );
		QString str;
		str.sprintf("录音机初始化失败，错误码%d\n",eRet);
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
				//strErrorMessage.Format( "载入语法文件失败,错误码%d", eRet );
				//MessageBox( strErrorMessage );
				QString str;
				str.sprintf("载入语法文件失败，错误码%d",eRet);
				QMessageBox msgBox;
				msgBox.setText(str);
				msgBox.exec();
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

void RobotQ::EchoGrammarData(const string &grammarFile){
	FILE* fp = fopen( grammarFile.c_str(), "rt" );
	if( fp == NULL ){
		//GetDlgItem( IDC_BTN_START_RECORD )->EnableWindow( FALSE );
		//CString strErrorMessage;
		//strErrorMessage.Format("打开语法文件%s失败",grammarFile.c_str());
		//MessageBox( strErrorMessage );
		QString str;
		str.sprintf("打开语法文件%s失败",grammarFile.c_str());
		QMessageBox msgBox;
		msgBox.setText(str);
		msgBox.exec();
		return;
	}

	unsigned char szBom[3];
	fread( szBom, 3, 1, fp );
	// 若有bom头，则清除，没有则当前位置回到头部
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

void HCIAPI RobotQ::RecorderRecogProcess(
	RECORDER_EVENT eRecorderEvent,
	ASR_RECOG_RESULT *psAsrRecogResult,
	void *pUsrParam){
		QString strMessage = "";
		QString add;
		if( psAsrRecogResult->uiResultItemCount > 0 ){
			unsigned char* pucUTF8 = NULL;
			HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
			add.sprintf( "识别中间结果: %s", pucUTF8 );
			strMessage+=add;
			HciExampleComon::FreeConvertResult( pucUTF8 );
			pucUTF8 = NULL;
		}else{
			strMessage.append( "*****无识别结果*****" );
		}
		//ui.textStatus->setPlainText(strMessage);	//此处显示结果应该由SIGNAL&SLOT强行实现
		    
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
		add.sprintf( "系统错误:%d", eErrorCode);
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

			//strMessage.AppendFormat( "识别时间:%d", (int)endClock - (int)dlg->m_startClock );

			//dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
		}

		strMessage = "";
		if( psAsrRecogResult->uiResultItemCount > 0 ){
			unsigned char* pucUTF8 = NULL;
			HciExampleComon::UTF8ToGBK( (unsigned char*)psAsrRecogResult->psResultItemList[0].pszResult, &pucUTF8 );
			//strMessage.AppendFormat( "识别结果: %s", pucUTF8 );
			HciExampleComon::FreeConvertResult( pucUTF8 );
			pucUTF8 = NULL;
		}else{
			//strMessage.AppendFormat( "*****无识别结果*****" );
		}

		//dlg->PostRecorderEventAndMsg(eRecorderEvent, strMessage);
}