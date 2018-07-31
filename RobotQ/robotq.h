#ifndef ROBOTQ_H
#define ROBOTQ_H

#include <QtGui/QMainWindow>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ui_robotq.h"
#include "stdafx.h"
#include "common/AccountInfo.h"
#include "common/CommonTool.h"
#include "common/FileReader.h"
#include "include/hci_asr_recorder.h"
#include <string>
#include <QMessageBox>
#include <QString>

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

signals:
	void sendRecorderAndMsg(RECORDER_EVENT eRecorderEvent, QString strMessage);
private slots:
	int OnStartClicked(bool checked);
	int OnEndClicked(bool checked);
	void OnShowStatus(RECORDER_EVENT eRecorderEvent, QString strMessage);

private:Ui::RobotQClass ui;

private:
	void EchoGrammarData(const string &grammarFile);
	bool CheckAndUpdataAuth();
	void GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode);
	static void HCIAPI RecordEventChange(RECORDER_EVENT eRecorderEvent, void *pUsrParam);

	static void HCIAPI RecorderRecogFinish(
		RECORDER_EVENT eRecorderEvent,
		ASR_RECOG_RESULT *psAsrRecogResult,
		void *pUsrParam);

	static void HCIAPI RecorderRecogProcess(
		RECORDER_EVENT eRecorderEvent,
		ASR_RECOG_RESULT *psAsrRecogResult,
		void *pUsrParam);

	static void HCIAPI RecorderErr(
		RECORDER_EVENT eRecorderEvent,
		HCI_ERR_CODE eErrorCode,
		void *pUsrParam);

	static void HCIAPI RecorderRecordingCallback(
		unsigned char * pVoiceData,
		unsigned int uiVoiceLen,
		void * pUsrParam
		);

public:
	void AppendMessage(QString strMsg);
	void RecorderRecording(unsigned char * pVoiceData, unsigned int uiVoiceLen);
	void PostRecorderEventAndMsg(RECORDER_EVENT eRecorderEvent, QString strMessage);

private:
	AsrRecogType m_RecogType;
	AsrRecogMode m_RecogMode;
	unsigned int m_GrammarId;

	bool m_recordingFlag;
	FILE * m_recordingFile;
	QString m_recordingFileName;


};

#endif // ROBOTQ_H
