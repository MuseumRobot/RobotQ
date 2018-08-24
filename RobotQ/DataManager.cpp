#include "DataManager.h"

DataManager::DataManager(void){}
DataManager::~DataManager(void){}
int DataManager::loadTask(){
	FILE* fp = fopen("task.data", "rb");
	if(!fp) return 0;
	m_TaskDataRecords.clear(); // 清空
	// 加载数据	
	while( !feof(fp)){
		TaskDataType record;
		int n = fread(&record, 1, sizeof(TaskDataType), fp);
		if( n < 0 ) break;
		if( n == 0) continue;
		m_TaskDataRecords.push_back(record);
	}
	fclose(fp);
	return 1;
}
TaskDataType* DataManager::findTask(int taskId){
	for(TaskDataTypeList::iterator iter = m_TaskDataRecords.begin();iter != m_TaskDataRecords.end(); iter ++){
		TaskDataType& record = *iter;
		if(record.id == taskId){
			return &record;
		}
	}
	return NULL;
}