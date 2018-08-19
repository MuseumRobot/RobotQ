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

struct StarMark{
	int markID;
	float mark_angle;
	float mark_x;
	float mark_y;
};
extern struct StarMark MARK[100];

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
	CMotor motor;
	Cstar StarGazer;
	CUPURG m_cURG;
	bool Init();
	int m_timerId;		//��������ѯ��������������ʾ���Ǳ�����
	int m_laser_data_postpro[1000];		//������Զ����ֵ(��λcm)
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
