#ifndef ROBOTQ_H
#define ROBOTQ_H

#define MSG_REFRESH_TIME 500		//消息更新到主界面上的时间间隔
#define IS_RECORDER_CONTINUE TRUE	//是否连续录音

#include "ui_robotq.h"
#include "stdafx.h"

using std::string;

typedef enum _tag_AsrRecogType{
	kRecogTypeUnkown = -1,      //未知类型
	kRecogTypeCloud = 0,        //云端识别
	kRecogTypeLocal,            //本地识别
}AsrRecogType;

typedef enum _tag_AsrRecogMode{
	kRecogModeUnkown = -1,      //未知类型
	kRecogModeFreetalk = 0,     //自由说
	kRecogModeGrammar,          //语法识别
}AsrRecogMode;

class RobotQ : public QMainWindow{
	Q_OBJECT

public:
	RobotQ(QWidget *parent = 0, Qt::WFlags flags = 0);
	~RobotQ();
	bool Init();
	bool Uninit(void);
	clock_t m_startClock;
	static QString GLOBAL_strMessage;
	static RECORDER_EVENT GLOBAL_eRecorderEvent;
	static bool GLOBAL_CommandValid;

private slots:
	int OnStartClicked(bool checked);
	int OnEndClicked(bool checked);
private:
	void OnShowStatus(RECORDER_EVENT eRecorderEvent, QString strMessage);

private:Ui::RobotQClass ui;

private:
	void EchoGrammarData(const string &grammarFile);
	bool CheckAndUpdataAuth();
	void GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode);
	static void HCIAPI RecordEventChange(RECORDER_EVENT eRecorderEvent, void *pUsrParam);
	static void HCIAPI RecorderRecogFinish(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult,void *pUsrParam);
	static void HCIAPI RecorderRecogProcess(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult,void *pUsrParam);
	static void HCIAPI RecorderErr(RECORDER_EVENT eRecorderEvent,HCI_ERR_CODE eErrorCode,void *pUsrParam);
	static void HCIAPI RecorderRecordingCallback(unsigned char * pVoiceData,unsigned int uiVoiceLen,void * pUsrParam);

public:
	void AppendMessage(QString strMsg);
	void RecorderRecording(unsigned char * pVoiceData, unsigned int uiVoiceLen);
	void PostRecorderEventAndMsg(RECORDER_EVENT eRecorderEvent, QString strMessage);
	virtual void timerEvent(QTimerEvent *event);
private:
	AsrRecogType m_RecogType;
	AsrRecogMode m_RecogMode;
	unsigned int m_GrammarId;

	bool m_recordingFlag;
	FILE * m_recordingFile;
	QString m_recordingFileName;

	int m_timerId;
};

#endif // ROBOTQ_H
