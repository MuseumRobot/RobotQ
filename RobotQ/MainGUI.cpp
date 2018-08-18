#include "MainGUI.h"

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);
	m_ManualControl=new ManualControl(this);
	connect(ui.btnRobotQ,SIGNAL(clicked()),this,SLOT(OnBtnRobotQ()));
	connect(ui.btnManualControl,SIGNAL(clicked()),this,SLOT(OnBtnManualControl()));

}
MainGUI::~MainGUI(){

}
bool MainGUI::Init(){
	//#ifdef COMM_MOTOR
	//	// 电机串口初始化
	//	if(motor.open_com_motor(COMM_MOTOR)){
	//		//电机数据计算
	//		pThread_Motor_Comput = AfxBeginThread(ThreadComput_MotorData,NULL);
	//		int a= 1;
	//	}else{
	//		int a=0;
	//	}
	//#endif
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

