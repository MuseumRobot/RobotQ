#include "MainGUI.h"

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);
	m_ManualControl=new ManualControl(this);
	connect(ui.btnRobotQ,SIGNAL(clicked()),this,SLOT(OnBtnRobotQ()));
	connect(ui.btnManualControl,SIGNAL(clicked()),this,SLOT(OnBtnManualControl()));
	connect(m_ManualControl->ui.btnForward,SIGNAL(clicked()),this,SLOT(On_MC_BtnForward()));
	connect(m_ManualControl->ui.btnBackward,SIGNAL(clicked()),this,SLOT(On_MC_BtnBackward()));
	connect(m_ManualControl->ui.btnTurnleft,SIGNAL(clicked()),this,SLOT(On_MC_BtnTurnleft()));
	connect(m_ManualControl->ui.btnTurnright,SIGNAL(clicked()),this,SLOT(On_MC_BtnTurnright()));
	connect(m_ManualControl->ui.btnStopmove,SIGNAL(clicked()),this,SLOT(On_MC_BtnStopmove()));
	connect(m_ManualControl->ui.btnStartSpeak,SIGNAL(clicked()),this,SLOT(On_MC_BtnRobotQSpeak()));
	connect(m_ManualControl->ui.btnStopSpeak,SIGNAL(clicked()),m_RobotQ,SLOT(OnStopSpeak()));
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
	motor.VectorMove(1200,2);
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

int MainGUI::On_MC_BtnForward(){
	motor.VectorMove(1200,0);
	return 0;
}
int MainGUI::On_MC_BtnBackward(){
	motor.VectorMove(-1200,0);
	return 0;
}
int MainGUI::On_MC_BtnTurnleft(){
	motor.VectorMove(200,2);
	return 0;
}
int MainGUI::On_MC_BtnTurnright(){
	motor.VectorMove(200,-2);
	return 0;
}
int MainGUI::On_MC_BtnStopmove(){

	return 0;
}
int MainGUI::On_MC_BtnRobotQSpeak(){
	QString str;
	str=m_ManualControl->ui.comSpeaklist->currentText();
	RobotQ::RobotQSpeak(str);
	return 0;
}