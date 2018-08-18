#ifndef MAINGUI_H
#define MAINGUI_H

#include <QMainWindow>
#include <QtGui/QApplication>
#include "ui_MainGUI.h"
#include "robotq.h"
#include "ManualControl.h"
#include "include/Comm_data_motor3.h"

#define COMM_MOTOR 3 //底部电机串口号
#define COMM_STAR 4//星标定位串口
#define COMM_LASER 5 //激光传感器串口号

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
	bool isMotorOpen;	//电机端口是否开启

private slots:
	int OnBtnRobotQ();
	int OnBtnManualControl();
};

#endif // MAINGUI_H
