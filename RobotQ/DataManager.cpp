#include "DataManager.h"

DataManager::DataManager(void){}
DataManager::~DataManager(void){}
int DataManager::loadTask(int n){
	FILE* fp;
	if(n == 1){
		fp = fopen("task1.data", "rb");
	}else if(n == 2){
		fp = fopen("task2.data", "rb");
	}else{
		fp = fopen("task3.data", "rb");
	}
	if(!fp) return 0;
	m_TaskDataRecords.clear(); // ���
	// ��������	
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
int DataManager::loadSpeakContent(){
	FILE* fp = fopen("speakContent.data", "rb");
	if(!fp) return 0;
	m_SpeakContentRecords.clear(); // ���
	// ��������	
	while( !feof(fp)){
		SpeakContentType record;
		int n = fread(&record, 1, sizeof(SpeakContentType), fp);
		if( n < 0 ) break;
		if( n == 0) continue;
		m_SpeakContentRecords.push_back(record);
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
SpeakContentType* DataManager::findSpeakContent(int speakContentId){
	for(SpeakContentTypeList::iterator iter = m_SpeakContentRecords.begin();iter != m_SpeakContentRecords.end(); iter ++){
		SpeakContentType& record = *iter;
		if(record.id == speakContentId){
			return &record;
		}
	}
	return NULL;
}