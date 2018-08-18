#include "MainGUI.h"

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);	//初始化这些成员对象需要在connect前
	m_ManualControl=new ManualControl(this);
	m_DashBoard=new DashBoard(this);
	if(m_RobotQ->isAuthReady)m_DashBoard->ui.ck_Auth->setChecked(true);
	if(m_RobotQ->isASRReady)m_DashBoard->ui.ck_ASR->setChecked(true);
	if(m_RobotQ->isTTSReady)m_DashBoard->ui.ck_TTS->setChecked(true);
	connect(ui.btnRobotQ,SIGNAL(clicked()),this,SLOT(OnBtnRobotQ()));
	connect(ui.btnManualControl,SIGNAL(clicked()),this,SLOT(OnBtnManualControl()));
	connect(ui.btnDashBoard,SIGNAL(clicked()),this,SLOT(OnBtnDashBoard()));
	connect(m_ManualControl->ui.btnForward,SIGNAL(clicked()),this,SLOT(On_MC_BtnForward()));
	connect(m_ManualControl->ui.btnBackward,SIGNAL(clicked()),this,SLOT(On_MC_BtnBackward()));
	connect(m_ManualControl->ui.btnTurnleft,SIGNAL(clicked()),this,SLOT(On_MC_BtnTurnleft()));
	connect(m_ManualControl->ui.btnTurnright,SIGNAL(clicked()),this,SLOT(On_MC_BtnTurnright()));
	connect(m_ManualControl->ui.btnStopmove,SIGNAL(clicked()),this,SLOT(On_MC_BtnStopmove()));
	connect(m_ManualControl->ui.btnStartSpeak,SIGNAL(clicked()),this,SLOT(On_MC_BtnRobotQSpeak()));
	connect(m_ManualControl->ui.btnStopSpeak,SIGNAL(clicked()),m_RobotQ,SLOT(OnStopSpeak()));	
	//connect(m_RobotQ,SIGNAL(TTS_Ready()),this,SLOT(check_TTS_Ready()));//在子窗口的初始化函数中发射信号无法被接受，而在初始化函数之外发射有效
	Init();
}
MainGUI::~MainGUI(){
	delete m_RobotQ;
	delete m_ManualControl;
	delete m_DashBoard;
}
bool MainGUI::Init(){
	isMotorOpen = false;
	// 电机串口初始化
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
int MainGUI::OnBtnDashBoard(){
	m_DashBoard->show();
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
	motor.stop();
	return 0;
}
int MainGUI::On_MC_BtnRobotQSpeak(){
	QString str;
	str=m_ManualControl->ui.comSpeaklist->currentText();
	RobotQ::RobotQSpeak(str);
	return 0;
}