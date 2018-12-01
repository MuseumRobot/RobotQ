#ifndef MAINGUI_H
#define MAINGUI_H

#include "stdafx.h"
#include "ui_MainGUI.h"
#include "MuseumGUI.h"
#include "robotq.h"
#include "ManualControl.h"
#include "DashBoard.h"
#include "DataManager.h"

#define COMM_MOTOR 3				//底部电机串口号
#define COMM_STAR 4					//星标定位串口
#define COMM_LASER 5				//激光传感器串口号
#define PI 3.141592653
#define MUSEUMMODE 1				//值为1开启博物馆使用界面，值为0则开启开发者界面，值为2则开启带有虚拟机的混合现实开发者界面
#define MARKNUM	31					//全局星标总数
#define TODOLISTMAXNUM 99			//任务清单数目的上限
#define DODGESTEPS 50				//闪避时刻中最低有效步数
#define INSTRUCTION_CYCLE 200		//指令周期，单位ms
#define INFOREFRESH_CYCLE 100		//数据刷新周期，单位ms
#define OBSTACLE_DISTANCE 400		//近处障碍物探测距离，单位mm，用以判断前方道路是否通畅
#define FAR_OBSTACLE_DISTANCE 800	//远处障碍物探测距离，单位mm，用以判断前方较远处是否通畅，从而控制前进速度
#define ERRORANGLE 10.0				//选择角度的误差范围，单位°
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

class MainGUI : public QDialog{
	Q_OBJECT

public:
	MainGUI(QWidget *parent = 0);
	~MainGUI();
	PopupDialog* m_popup_secondScreen_image;	//第二显示器弹出窗口
	QPointF PosByStar1;		//首选星标得到的星标定位器的世界坐标，单位cm
	QPointF PosByStar2;		//备选星标，单位cm
	QPointF PosSafe;		//综合考虑电机和星标进而得出可靠的机器人中心点坐标结果，单位cm
	QPointF PosGoal;		//目标坐标，单位cm
	float AngleByStar1;		//首选星标得到的星标定位器的角度，单位°
	float AngleByStar2;		//备选星标得到的星标定位器的角度，单位°
	float AngleSafe;		//实际的机器人朝向（以x轴正方向为0°，逆时针为正方向，取值为[0~360)毕竟是浮点数不能这么整啦）
	float Angle_face_Goal;	//站在当前的机器人本体世界坐标朝目标世界坐标看的角度，单位°
	float Angle_face_Audiance;	//抵达目的地后看向观众的角度
	QString SpeakContent;	//机器人将要说的内容
	int SpeakWaitCycle;		//发出说话指令后，机器人在若干个指令周期内不分配任务
private:
	Ui::MainGUI ui;
	MuseumGUI* m_MuseumGUI;
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
	bool is_FastGuideMode;							//当前是否是快速导览模式（无语音模式）
	bool is_path_clear;								//当前视野下前方是否通畅
	bool is_far_path_clear;							//判断当前视野下远处是否通畅
	bool is_dodge_moment;							//是否进入闪避时刻
	bool is_advertisement_available;				//是否该显示广告
	int dodge_mode;									//为了使每次进入闪避时刻只会向一个方向闪避，需要在进入时给定闪避方向，1为左，2为右，退出闪避时为0，可供分配新的值
	int dodge_move_times;							//开启闪避时刻后，闪避动作已执行的次数
	int sectorObstacleDistance[36];					//每五度划分一个扇区
	int m_timer_refresh_dashboard;					//计数器查询将机器人数据显示在仪表盘中
	int m_timer_refresh_task;						//计数器查询刷新当前任务
	float m_Last_inLV;								//上一个动作的线速度
	float m_Last_inPS;								//上一个动作的角速度
	int todoList[TODOLISTMAXNUM];					//任务清单
	int taskID;										//当前任务代码
	int currentTodoListId;							//当前任务在清单中的下标
	int JudgeTaskType(int taskID);					//根据任务代码判断当前任务是语音任务还是位移任务
	void Init();									//初始化
	void InitAdjustGUI();							//初始化调整界面（选择启动博物馆模式或开发者模式）
	void InitStarMark();							//为LED定位标签数组赋值
	void InitStarMarkMuseum();						//LED标签数组赋值(博物馆)
	void InitDataBase();							//初始化数据库
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
	void FastGuideTodolist();						//修剪任务队列，剔除所有语音点
	void JudgeForwardSituation();					//判断前路是否通畅
	void ProjectFinishedMeasures();					//完成当前项目后执行的动作
	void PathTaskFinishedMeasures();				//完成位移任务后执行的动作
	void ParseTodoList(QString str, int* todoList);	//解析QString格式的任务清单，赋值给todoList数组
	void Rotate_to_GoalAngle(float AngleGoal);		//旋转到指定角度，参数单位°
	void SpeakTaskFinishedMeasures();				//完成语音任务后执行的动作
	void ShowPic(QString str);						//显示图片（参数为图片路径）
	void ShowPicByTaskID(int taskID);				//显示图片（参数为任务代码）
	void ShowAdvertisement();						//播放广告
	void refreshDashboardSector();					//刷新仪表盘上的障碍分布图
	void refreshDashboardData();					//刷新仪表盘上的普通数据
	float zTool_cos_angle(float angle);				//计算余弦值，参数单位为°
	float zTool_sin_angle(float angle);				//计算正弦值，参数单位为°
	float zTool_mod_360f(float angle);				//将角度范围归为(0,360)
	float zTool_vector_angle(QPointF d);			//求矢量角（范围(0,360)，单位°），参数单位为cm
	virtual void timerEvent(QTimerEvent *event);
	void closeEvent(QCloseEvent *event);			//关闭主界面后的动作

private slots:
	inline int OnBtnRobotQ(){m_RobotQ->show();return 0;}
	inline int OnBtnManualControl(){m_ManualControl->move(300,20);m_ManualControl->show();return 0;}
	inline int OnBtnMuseumGUI(){m_MuseumGUI->move(80,50);m_MuseumGUI->show();return 0;}
	inline int OnBtnDashBoard(){m_DashBoard->move(750,20);m_DashBoard->show();return 0;}
	int OnBtnAutoGuide();
	int OnBtnSelectPath1();
	int OnBtnSelectPath2();
	int OnBtnSelectPath3();
	int OnBtnSelectPath4();
	int OnBtnFastGuideMode();
	int On_MC_BtnForward();
	int On_MC_BtnBackward();
	int On_Auto_BtnTurnleft(int speedlevel);
	int On_Auto_BtnTurnright(int speedlevel);
	int On_Auto_BtnForward(int speedlevel);
	int On_MC_BtnTurnleft();
	int On_MC_BtnTurnright();
	int On_MC_BtnStopmove();
	int On_MC_BtnRobotQSpeak();
	int On_MC_BtnGoHome();
	int On_MC_BtnExit();
	int On_MC_BtnExeSelfTask();
};

#endif // MAINGUI_H
