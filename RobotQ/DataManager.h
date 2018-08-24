#pragma once
#include "TaskDataType.h"
#include <QVector>

class DataManager{

public:
	DataManager(void);
	~DataManager(void);
	int loadTask();
	TaskDataType* findTask(int taskId);
	TaskDataTypeList m_TaskDataRecords;		//所有任务点记录
};

