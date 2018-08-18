#include "MainGUI.h"

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);
	m_ManualControl=new ManualControl(this);
	connect(ui.btnRobotQ,SIGNAL(clicked()),this,SLOT(OnBtnRobotQ()));
	connect(ui.btnManualControl,SIGNAL(clicked()),this,SLOT(OnBtnManualControl()));
	Init();
}
MainGUI::~MainGUI(){

}
bool MainGUI::Init(){
	isMotorOpen = false;
	// 电机串口初始化
	CMotor motor;
	if(motor.open_com_motor(COMM_MOTOR)) 
		isMotorOpen = true;
	else
		return false;
	motor.VectorMove(0,1200,1.2);
	return 0;
}

int MainGUI::OnBtnRobotQ(){
	m_RobotQ->show();
	return 0;
}

int MainGUI::OnBtnManualControl(){
	m_ManualControl->show();
	return 0;
}

