#pragma once
#include <list>
#include <QString>
#include <QPoint>

using namespace std;

class TaskDataType{
public:
	TaskDataType(): id(0) {}
	int id;
	char name[32];
	int taskType;
	int SpeakContentId;
	double x;
	double y;
};

typedef list<TaskDataType> TaskDataTypeList;
