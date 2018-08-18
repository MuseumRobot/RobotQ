#ifndef MAINGUI_H
#define MAINGUI_H

#include <QMainWindow>
#include <QtGui/QApplication>
#include "ui_MainGUI.h"
#include "robotq.h"
#include "ManualControl.h"
#include "include/Comm_data_motor3.h"

#define COMM_MOTOR 3 //�ײ�������ں�
#define COMM_STAR 4//�Ǳ궨λ����
#define COMM_LASER 5 //���⴫�������ں�

class MainGUI : public QMainWindow{
	Q_OBJECT

public:
	MainGUI(QWidget *parent = 0);
	~MainGUI();

private:
	Ui::MainGUI ui;
	RobotQ* m_RobotQ;
	ManualControl* m_ManualControl;
	CMotor motor;
	bool Init();
	bool isMotorOpen;	//����˿��Ƿ���

private slots:
	int OnBtnRobotQ();
	int OnBtnManualControl();
};

#endif // MAINGUI_H
