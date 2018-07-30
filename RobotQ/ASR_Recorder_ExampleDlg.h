
// Recorder_ExampleDlg.h : ͷ�ļ�
//

#pragma once

#include "common/FileReader.h"
#include "hci_asr_recorder.h"
#include <string>
#include "afxwin.h"
using std::string;


typedef enum _tag_AsrRecogType
{
    kRecogTypeUnkown = -1,      //δ֪����
    kRecogTypeCloud = 0,        //�ƶ�ʶ��
    kRecogTypeLocal,            //����ʶ��
}AsrRecogType;

typedef enum _tag_AsrRecogMode
{
    kRecogModeUnkown = -1,      //δ֪����
    kRecogModeFreetalk = 0,     //����˵
    kRecogModeGrammar,          //�﷨ʶ��
}AsrRecogMode;

// CRecorder_ExampleDlg �Ի���
class CRecorder_ExampleDlg : public CDialog
{
// ����
public:
	CRecorder_ExampleDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CRecorder_ExampleDlg()
	{ 
		// ������¼�����Ļص��У�ʹ����Windows���ڣ����������������з���ʼ��
		// �����ڴ�����Чʱ�����з���ʼ��
		//Uninit(); 
	}

// �Ի�������
	enum { IDD = IDD_RECORDER_EXAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
    HICON m_hIcon;
	// ���ɵ���Ϣӳ�亯��
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
