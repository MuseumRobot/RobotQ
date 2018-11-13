#pragma once
#include "TaskDataType.h"
#include "SpeakContentType.h"

class DataManager{

public:
	DataManager(void);
	~DataManager(void);
	int loadTask();
	int loadSpeakContent();
	TaskDataType* findTask(int taskId);
	SpeakContentType* findSpeakContent(int speakContentId);
	TaskDataTypeList m_TaskDataRecords;				//����������¼
	SpeakContentTypeList m_SpeakContentRecords;		//�������ϼ�¼
};

