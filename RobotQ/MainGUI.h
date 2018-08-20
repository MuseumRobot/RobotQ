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

#define COMM_MOTOR 3	//�ײ�������ں�
#define COMM_STAR 4		//�Ǳ궨λ����
#define COMM_LASER 5	//���⴫�������ں�

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

//�����������ݶ�ȡ�ṹ����
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
	QPointF PosByStar1;		//��ѡ�Ǳ�õ��Ļ�������������
	QPointF PosByStar2;		//��ѡ�Ǳ�
	QPointF PosByMotor;		//�����̼�ȷ��������
	QPointF PosSafe;		//�ۺϿ��ǵ�����Ǳ�����ó��ɿ���������
	QPointF PosGoal;		//Ŀ������

private:
	Ui::MainGUI ui;
	RobotQ* m_RobotQ;
	ManualControl* m_ManualControl;
	DashBoard* m_DashBoard;
	CMotor m_motor;
	Cstar m_StarGazer;
	CUPURG m_cURG;	
	bool Init();
	float sectorObstacleDistance[36];				//ÿ��Ȼ���һ������
	int m_timer_refresh_dashboard;					//��������ѯ��������������ʾ���Ǳ�����
	int m_timer_refresh_task;						//��������ѯˢ�µ�ǰ����
	StarMark m_MARK[100];							//LED��λ��ǩ���� - ÿ��߳�455
	void InitStarMark();							//ΪLED��λ��ǩ���鸳ֵ
	void InitCommMotorAndStar();					//����Ǳ괮�ڳ�ʼ��
	void CalculateSectorDistance();					//�����������ϰ���ľ���
	void refreshDashboardSector();					//ˢ���Ǳ����ϵ��ϰ��ֲ�ͼ
	virtual void timerEvent(QTimerEvent *event);

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
