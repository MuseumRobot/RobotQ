
// Recorder_ExampleDlg.h : 头文件
//

#pragma once

#include "common/FileReader.h"
#include "hci_asr_recorder.h"
#include <string>
#include "afxwin.h"
using std::string;


typedef enum _tag_AsrRecogType
{
    kRecogTypeUnkown = -1,      //未知类型
    kRecogTypeCloud = 0,        //云端识别
    kRecogTypeLocal,            //本地识别
}AsrRecogType;

typedef enum _tag_AsrRecogMode
{
    kRecogModeUnkown = -1,      //未知类型
    kRecogModeFreetalk = 0,     //自由说
    kRecogModeGrammar,          //语法识别
}AsrRecogMode;

// CRecorder_ExampleDlg 对话框
class CRecorder_ExampleDlg : public CDialog
{
// 构造
public:
	CRecorder_ExampleDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CRecorder_ExampleDlg()
	{ 
		// 由于在录音机的回调中，使用了Windows窗口，不能在析构函数中反初始化
		// 必须在窗口有效时，进行反初始化
		//Uninit(); 
	}

// 对话框数据
	enum { IDD = IDD_RECORDER_EXAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
    HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnShowStatus( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStartRecord();
	afx_msg void OnBnClickedBtnCancelRecord();
	bool Init();
	bool Uninit(void);
	afx_msg void OnBnClickedOk();

private:
    void EchoGrammarData(const string &grammarFile);

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
	void AppendMessage(CString &strMsg);
	void RecorderRecording(unsigned char * pVoiceData, unsigned int uiVoiceLen);
	void PostRecorderEventAndMsg(RECORDER_EVENT eRecorderEvent, const CString & strMessage);

private:
    AsrRecogType m_RecogType;
    AsrRecogMode m_RecogMode;
    unsigned int m_GrammarId;

	BOOL m_recordingFlag;
	FILE * m_recordingFile;
	CString m_recordingFileName;
public:
	afx_msg void OnBnClickedBtnBrowser();
	afx_msg void OnBnClickedSaveRecording();

	clock_t m_startClock;
};
