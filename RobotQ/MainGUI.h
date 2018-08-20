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
	int m_timerId;									//��������ѯ��������������ʾ���Ǳ�����
	StarMark m_MARK[100];							//LED��λ��ǩ���� - ÿ��߳�455
	void InitStarMark();							//ΪLED��λ��ǩ���鸳ֵ
	void InitComm();								//����Ǳ꼤�⴮�ڳ�ʼ��
	int m_laser_data_postpro[1000];					//������Զ����ֵ(��λcm)
	CWinThread* pThread_Read_Laser;					//�����������߳�
	virtual void timerEvent(QTimerEvent *event);
	UINT ThreadReadLaser_Data(LPVOID lpParam);		//���������ݺ���

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
