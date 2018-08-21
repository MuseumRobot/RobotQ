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
#include "include/Comm_data_motor3.h"
#include "include/Comm_data_star.h"
#include "include/UPURG.h"

#define COMM_MOTOR 3			//底部电机串口号
#define COMM_STAR 4				//星标定位串口
#define COMM_LASER 5			//激光传感器串口号
#define PI 3.141592653
#define EMERGENCY_TIMES 3
#define EMERGENCY_DISTANCE 300
#define OBSTACLE_DISTANCE 800	//距离低于这个值将认为是障碍物
#define DODGESTEPS 15
#define Distance_Robot_forward_StarGazer 32.5		//机器人中心点在星标定位器中心点前32.5cm，实测原地旋转一周仍存在8cm内误差（位置无法闭合）

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

//激光测距器数据读取结构参数
struct threadInfo_laser_data_postpro{
	double 	m_Laser_Data_Value_PostPro[768];
	int	m_Laser_Data_Point_PostPro;
	double rx;
	double ry;   //robot odometry pos
	double  th;   //robot orientation 
	double  x[10][769];//[cm]
	double  y[10][769];//[cm]
	int bad[10][769];// 0 if OK
	int seg[10][769];
};

class MainGUI : public QMainWindow{
	Q_OBJECT

public:
	MainGUI(QWidget *parent = 0);
	~MainGUI();
	QPointF PosByStar1;		//首选星标得到的星标定位器的世界坐标，单位cm
	QPointF PosByStar2;		//备选星标，单位cm
	QPointF PosByMotor;		//电机里程计确定的坐标，单位cm
	QPointF PosSafe;		//综合考虑电机和星标进而得出可靠的机器人中心点坐标结果，单位cm
	QPointF PosGoal;		//目标坐标，单位cm
	float AngleByStar1;		//首选星标得到的星标定位器的角度，单位°
	float AngleByStar2;		//备选星标得到的星标定位器的角度，单位°
	float AngleSafe;		//实际的机器人朝向（以x轴正方向为0°，逆时针为正方向，取值为[0~360)毕竟是浮点数不能这么整啦）
	float Angle_face_Goal;	//站在当前的机器人本体世界坐标朝目标世界坐标看的角度，单位°
private:
	Ui::MainGUI ui;
	RobotQ* m_RobotQ;
	ManualControl* m_ManualControl;
	DashBoard* m_DashBoard;
	CMotor m_motor;
	Cstar m_StarGazer;
	CUPURG m_cURG;	
	float m_EMERGENCY_DISTANCE;						//紧急制动距离，单位mm
	bool is_Auto_Mode_Open;
	bool is_mission_accomplished;					//当前任务是否完成
	bool is_project_accomplished;					//当前项目是否完成
	bool is_path_clear;								//当前视野下前方是否通畅
	bool is_dodge_moment;							//是否进入闪避时刻
	bool is_time_to_dodge_right;					//是时候向右闪避次了吗
	bool is_time_to_dodge_left;						//是时候向左转向修正闪避了吗
	int dodge_right_times;							//开启闪避时刻后右躲避动作是否经历了许多次
	int Emergency_times;							//连续紧急制动3次后拒绝紧急制动转而避障
	float sectorObstacleDistance[36];				//每五度划分一个扇区
	int m_timer_refresh_dashboard;					//计数器查询将机器人数据显示在仪表盘中
	int m_timer_refresh_task;						//计数器查询刷新当前任务
	int m_timer_refresh_emergency_distance;			//计数器刷新紧急制动距离
	int todoList[10];								//任务清单
	int currentTodoListId;							//当前任务在清单中的下标
	StarMark m_MARK[100];							//LED定位标签数组 - 每块边长455
	void Init();									//初始化
	void InitStarMark();							//为LED定位标签数组赋值
	void InitCommMotorAndStar();					//电机星标串口初始化
	void InitDashBoardData();						//仪表盘数据初始化
	void CalculateSectorDistance();					//计算扇区内障碍物的距离
	void JudgeForwardSituation();					//判断前路是否通畅
	void AssignInstruction();						//分配下一步指令
	void Rotate_to_GoalAngle(float AngleGoal);		//旋转到指定角度，参数单位°
	void DodgeTurnRight();							//闪避时刻第一步(向右闪避准则)
	void DodgeTurnLeft();							//闪避时刻第二步(向右闪避准则)
	void refreshDashboardSector();					//刷新仪表盘上的障碍分布图
	void refreshDashboardData();					//刷新仪表盘上的普通数据
	float zTool_cos_angle(float angle);					//计算余弦值，参数单位为°
	float zTool_sin_angle(float angle);					//计算正弦值，参数单位为°
	virtual void timerEvent(QTimerEvent *event);

private slots:
	int OnBtnRobotQ();
	int OnBtnManualControl();
	int OnBtnDashBoard();
	int OnBtnAutoGuide();
	int On_MC_BtnForward();
	int On_MC_BtnBackward();
	int On_MC_BtnTurnleft();
	int On_MC_BtnTurnright();
	int On_MC_BtnStopmove();
	int On_MC_BtnRobotQSpeak();
};

#endif // MAINGUI_H
