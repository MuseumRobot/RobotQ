#ifndef ROBOTQ_H
#define ROBOTQ_H

#define MSG_REFRESH_TIME 200		//消息更新到主界面上的时间间隔
#define IS_RECORDER_CONTINUE FALSE	//是否连续录音

#include "ui_robotq.h"
#include "stdafx.h"

using std::string;

typedef enum _tag_AsrRecogType{
	kRecogTypeUnkown = -1,      //未知类型
	kRecogTypeCloud = 0,        //云端识别
	kRecogTypeLocal = 1,            //本地识别
}AsrRecogType;

typedef enum _tag_AsrRecogMode{
	kRecogModeUnkown = -1,      //未知类型
	kRecogModeFreetalk = 0,     //自由说
	kRecogModeGrammar = 1,          //语法识别
	kRecogModeDialog = 2,
}AsrRecogMode;

class RobotQ : public QDialog{
	Q_OBJECT
public:
	RobotQ(QWidget *parent = 0, Qt::WFlags flags = 0);
	~RobotQ();
	Ui::RobotQClass ui;
	bool isAuthReady;
	bool isASRReady;
	bool isTTSReady;
private slots:
	int OnStartClicked();
	int OnEndClicked();
	int OnQueryClicked();
public slots:
	static int OnStopSpeak();
private:
	bool Init();
	bool Uninit(void);
	clock_t m_startClock;
	static QString GLOBAL_strMessage;
	static RECORDER_EVENT GLOBAL_eRecorderEvent;
	static bool GLOBAL_CommandValid;
	void OnShowStatus(RECORDER_EVENT eRecorderEvent, QString strMessage);
private:
	bool CheckAndUpdataAuth();
	void GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode);
	static void HCIAPI RecordEventChange(RECORDER_EVENT eRecorderEvent, void *pUsrParam);
	static void HCIAPI RecorderRecogFinish(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult,void *pUsrParam);
	static void HCIAPI RecorderRecogProcess(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult,void *pUsrParam);
	static void HCIAPI RecorderErr(RECORDER_EVENT eRecorderEvent,HCI_ERR_CODE eErrorCode,void *pUsrParam);
	static void HCIAPI RecorderRecordingCallback(unsigned char * pVoiceData,unsigned int uiVoiceLen,void * pUsrParam);

	static void HCIAPI CB_EventChange(_MUST_ _IN_ PLAYER_EVENT ePlayerEvent,_OPT_ _IN_ void * pUsrParam);
	static void HCIAPI CB_ProgressChange (_MUST_ _IN_ PLAYER_EVENT ePlayerEvent,_MUST_ _IN_ int nStart,
										  _MUST_ _IN_ int nStop,_OPT_ _IN_ void * pUsrParam);
	static void HCIAPI CB_SdkErr( _MUST_ _IN_ PLAYER_EVENT ePlayerEvent,_MUST_ _IN_ HCI_ERR_CODE eErrorCode,_OPT_ _IN_ void * pUsrParam );
public:
	void AppendMessage(QString strMsg);
	void RecorderRecording(unsigned char * pVoiceData, unsigned int uiVoiceLen);
	void PostRecorderEventAndMsg(RECORDER_EVENT eRecorderEvent, QString strMessage);
	virtual void timerEvent(QTimerEvent *event);
	static void RobotQSpeak(QString str);
	static int Json_Explain (char buf[],char result[],char answer[]);
private:
	AsrRecogType m_RecogType;
	AsrRecogMode m_RecogMode;
	int m_timerId;
};

#endif // ROBOTQ_H
