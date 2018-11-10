#pragma once
#include <list>

using namespace std;

class TaskDataType{
public:
	TaskDataType(): id(0) {}
	int id;				
	int taskType;			//任务类型（语音任务为1，位移任务为0）
	double x;				//目标点的x坐标（仅在当前任务是位移任务时有效）
	double y;				//目标点的y坐标（仅在当前任务是位移任务时有效）
	int FacingAngle;		//到达目标点后需要面朝的方向（仅在当前任务是位移任务时有效）
	int SpeakContentId;		//语音内容的索引号（仅在当前任务是语音任务时有效）
};

typedef list<TaskDataType> TaskDataTypeList;
