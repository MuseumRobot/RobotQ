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
	TaskDataTypeList m_TaskDataRecords;				//所有任务点记录
	SpeakContentTypeList m_SpeakContentRecords;		//所有语料记录
};

