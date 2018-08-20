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

#define COMM_MOTOR 3	//底部电机串口号
#define COMM_STAR 4		//星标定位串口
#define COMM_LASER 5	//激光传感器串口号

typedef struct StarMark{
	int markID;
	float mark_angle;
	float mark_x;
	float mark_y;
}StarMark;

class MainGUI : public QMainWindow{
	Q_OBJECT

public:
	MainGUI(QWidget *parent = 0);
	~MainGUI();
	QPointF PosByStar1;		//首选星标得到的机器人世界坐标
	QPointF PosByStar2;		//备选星标
	QPointF PosByMotor;		//电机里程计确定的坐标
	QPointF PosSafe;		//综合考虑电机和星标进而得出可靠的坐标结果
	QPointF PosGoal;		//目标坐标

private:
	Ui::MainGUI ui;
	RobotQ* m_RobotQ;
	ManualControl* m_ManualControl;
	DashBoard* m_DashBoard;
	CMotor m_motor;
	Cstar m_StarGazer;
	CUPURG m_cURG;
	bool Init();
	int m_timerId;									//计数器查询将机器人数据显示在仪表盘中
	StarMark m_MARK[100];							//LED定位标签数组 - 每块边长455
	void InitStarMark();							//为LED定位标签数组赋值
	void InitComm();								//电机星标激光串口初始化
	int m_laser_data_postpro[1000];					//激光最远返回值(单位cm)
	CWinThread* pThread_Read_Laser;					//读激光数据线程
	virtual void timerEvent(QTimerEvent *event);
	UINT ThreadReadLaser_Data(LPVOID lpParam);		//读激光数据函数

private slots:
	int OnBtnRobotQ();
	int OnBtnManualControl();
	int OnBtnDashBoard();
	int On_MC_BtnForward();
	int On_MC_BtnBackward();
	int On_MC_BtnTurnleft();
	int On_MC_BtnTurnright();
	int On_MC_BtnStopmove();
	int On_MC_BtnRobotQSpeak();
};

#endif // MAINGUI_H
