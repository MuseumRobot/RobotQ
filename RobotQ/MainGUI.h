#ifndef MAINGUI_H
#define MAINGUI_H

#include <QMainWindow>
#include <QtGui/QApplication>
#include <QCheckBox>
#include <QPointF>
#include "ui_MainGUI.h"
#include "robotq.h"
#include "ManualControl.h"
#include "DashBoard.h"
#include "DataManager.h"
#include "include/Comm_data_motor3.h"
#include "include/Comm_data_star.h"
#include "include/UPURG.h"

#define COMM_MOTOR 3				//底部电机串口号
#define COMM_STAR 4					//星标定位串口
#define COMM_LASER 5				//激光传感器串口号
#define PI 3.141592653
#define MUSEUMMODE 0				//值为1开启博物馆使用界面，值为0则开启开发者界面
#define MARKNUM	31					//全局星标总数
#define TODOLISTMAXNUM 99			//任务清单数目的上限
#define DODGESTEPS 5				//闪避时刻中最低有效步数
#define EMERGENCY_TIMES 3			//紧急制动N次后暂时解除制动
#define EMERGENCY_DISTANCE 300		//紧急制动危险距离，单位mm
#define EMERGENCY_RECOVER_CYCLE 6	//紧急制动解除后将于N个指令周期后恢复
#define INSTRUCTION_CYCLE 1500		//指令周期，单位ms
#define INFOREFRESH_CYCLE 300		//数据刷新周期，单位ms
#define OBSTACLE_DISTANCE 400		//障碍物探测距离，单位mm
#define ERRORANGLE 12.0				//选择角度的误差范围，单位°
#define ERRORDISTANCE 15.0			//抵达目标点距离误差半径范围，单位cm
#define SPEAKWORDSPERSECOND 4		//王静每秒钟阅读的字数
#define Distance_Robot_forward_StarGazer 32.5		//机器人中心点在星标定位器中心点前32.5cm，实测原地旋转一周仍存在8cm内误差（位置无法闭合）
#define PATHTASKTYPE 0				//位移任务点类型
#define SPEAKTASKTYPE 1				//语音任务点类型

typedef struct StarMark{
	int markID;
	float mark_angle;
	float mark_x;
	float mark_y;
}StarMark;

struct threadInfo_laser_data{
	double 	m_Laser_Data_Value[768];
	int	m_Laser_Data_Point;
};

class MainGUI : public QMainWindow{
	Q_OBJECT

public:
	MainGUI(QWidget *parent = 0);
	~MainGUI();
	QPushButton* btnAutoGuide_MUSEUM;
	QPushButton* btnPath1;
	QPushButton* btnPath2;
	QPushButton* btnPath3;
	QPointF PosByStar1;		//首选星标得到的星标定位器的世界坐标，单位cm
	QPointF PosByStar2;		//备选星标，单位cm
	QPointF PosSafe;		//综合考虑电机和星标进而得出可靠的机器人中心点坐标结果，单位cm
	QPointF PosGoal;		//目标坐标，单位cm
	float AngleByStar1;		//首选星标得到的星标定位器的角度，单位°
	float AngleByStar2;		//备选星标得到的星标定位器的角度，单位°
	float AngleSafe;		//实际的机器人朝向（以x轴正方向为0°，逆时针为正方向，取值为[0~360)毕竟是浮点数不能这么整啦）
	float Angle_face_Goal;	//站在当前的机器人本体世界坐标朝目标世界坐标看的角度，单位°
	QString SpeakContent;	//机器人将要说的内容
	int SpeakWaitCycle;		//发出说话指令后，机器人在若干个指令周期内不分配任务
private:
	Ui::MainGUI ui;
	RobotQ* m_RobotQ;
	ManualControl* m_ManualControl;
	DashBoard* m_DashBoard;
	CMotor m_motor;
	Cstar m_StarGazer;
	CUPURG m_cURG;	
	DataManager m_dataManager;
	StarMark m_MARK[MARKNUM];						//LED定位标签数组 - 每块边长455
	bool is_Auto_Mode_Open;
	bool is_project_accomplished;					//当前项目是否完成
	bool is_path_clear;								//当前视野下前方是否通畅
	bool is_dodge_moment;							//是否进入闪避时刻
	bool isEMERGENCY;								//是否是紧急制动时刻
	bool isBlockEMERGENCY;							//是否屏蔽紧急制动（为了避免语音点紧急制动打断讲解词，在讲解任务时屏蔽紧急制动功能，所以在语音任务开始时解除紧急制动，在位移任务开始时恢复紧急制动）
	int dodge_move_times;							//开启闪避时刻后，闪避动作已执行的次数
	int Emergency_times;							//连续紧急制动3次后拒绝紧急制动转而避障
	int m_EMERGENCY_DISTANCE;						//紧急制动距离，单位mm
	int sectorObstacleDistance[36];					//每五度划分一个扇区
	int m_timer_refresh_dashboard;					//计数器查询将机器人数据显示在仪表盘中
	int m_timer_refresh_task;						//计数器查询刷新当前任务
	int m_timer_refresh_emergency_distance;			//计数器刷新紧急制动距离
	int todoList[TODOLISTMAXNUM];					//任务清单
	int taskID;										//当前任务代码
	int currentTodoListId;							//当前任务在清单中的下标
	int JudgeTaskType(int taskID);					//根据任务代码判断当前任务是语音任务还是位移任务
	void Init();									//初始化
	void InitAdjustGUI();							//初始化调整界面（选择启动博物馆模式或开发者模式）
	void InitStarMark();							//为LED定位标签数组赋值
	void InitStarMarkMuseum();						//LED标签数组赋值(博物馆)
	void InitDataBase(int n);						//初始化数据库
	void InitTaskAssignment(int n);					//初始化分配任务路线
	void InitCommMotorAndStar();					//电机星标串口初始化
	void InitDashBoardData();						//仪表盘数据初始化
	void AssignInstruction();						//分配下一步指令
	void AssignGoalPos(int taskID);					//根据任务代码（位移任务）分配目标位置
	void AssignSpeakContent(int taskID);			//根据任务代码（语音任务）分配语音内容和机器人等待时间
	void CalculateSectorDistance();					//计算扇区内障碍物的距离
	void CommonMeasures();							//没有意外发生的周期执行常规动作
	void DodgeMeasures();							//触发闪避时刻后的动作（在向左和向右两个基础动作中选择合适的执行）
	void DodgeTurnRight();							//闪避时刻基础动作(向右闪避)
	void DodgeTurnLeft();							//闪避时刻基础动作(向左闪避)
	void EmergencyMeasures();						//触发紧急时刻后执行的紧急动作
	void Rotate_to_GoalAngle(float AngleGoal);		//旋转到指定角度，参数单位°
	void JudgeEmergency();							//判断当前指令周期是否触发了紧急制动时刻
	void JudgeForwardSituation();					//判断前路是否通畅
	void ProjectFinishedMeasures();					//完成当前项目后执行的动作
	void PathTaskFinishedMeasures();				//完成位移任务后执行的动作
	void ParseTodoList(QString str, int* todoList);	//解析QString格式的任务清单，赋值给todoList数组
	void SpeakTaskFinishedMeasures();				//完成语音任务后执行的动作
	void refreshDashboardSector();					//刷新仪表盘上的障碍分布图
	void refreshDashboardData();					//刷新仪表盘上的普通数据
	float zTool_cos_angle(float angle);				//计算余弦值，参数单位为°
	float zTool_sin_angle(float angle);				//计算正弦值，参数单位为°
	float zTool_mod_360f(float angle);				//将角度范围归为(0,360)
	float zTool_vector_angle(QPointF d);			//求矢量角（范围(0,360)，单位°），参数单位为cm
	virtual void timerEvent(QTimerEvent *event);

private slots:
	int OnBtnRobotQ();
	int OnBtnManualControl();
	int OnBtnDashBoard();
	int OnBtnAutoGuide();
	int OnBtnSelectPath1();
	int OnBtnSelectPath2();
	int OnBtnSelectPath3();
	int On_MC_BtnForward();
	int On_MC_BtnBackward();
	int On_Auto_BtnTurnleft(int speedlevel);
	int On_Auto_BtnTurnright(int speedlevel);
	int On_MC_BtnTurnleft();
	int On_MC_BtnTurnright();
	int On_MC_BtnStopmove();
	int On_MC_BtnRobotQSpeak();
	int On_MC_BtnGoHome();
	int On_MC_BtnExeSelfTask();
};

#endif // MAINGUI_H
