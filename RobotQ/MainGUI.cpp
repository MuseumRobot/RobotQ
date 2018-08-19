#include "MainGUI.h"

struct StarMark MARK[100]={ //LED定位标签数组
	//每块边长455
	MARK[0].markID = 624,
	MARK[0].mark_angle = 0.00,
	MARK[0].mark_x = -427,
	MARK[0].mark_y = 10,


	MARK[1].markID = 610,
	MARK[1].mark_angle = 0.00,
	MARK[1].mark_x = -344,
	MARK[1].mark_y = 132,

	MARK[2].markID = 594, 
	MARK[2].mark_angle = 0.00,
	MARK[2].mark_x = -212,
	MARK[2].mark_y = 0,

	MARK[3].markID = 608,
	MARK[3].mark_angle = 0.00,
	MARK[3].mark_x = -20,
	MARK[3].mark_y = 30,

	MARK[4].markID = 626,
	MARK[4].mark_angle = 0.00,
	MARK[4].mark_x = -195,
	MARK[4].mark_y = 179,

	MARK[5].markID = 592,
	MARK[5].mark_angle = 0.00,
	MARK[5].mark_x = -6,
	MARK[5].mark_y = 177,


	MARK[6].markID = 560,
	MARK[6].mark_angle = 0.00,
	MARK[6].mark_x = 160,
	MARK[6].mark_y = 12,

	MARK[7].markID = 546,
	MARK[7].mark_angle = 0.00,
	MARK[7].mark_x = 150,
	MARK[7].mark_y = 183,

	MARK[8].markID = 578,
	MARK[8].mark_angle = 0.00,
	MARK[8].mark_x = 140,
	MARK[8].mark_y = 377,

	MARK[9].markID = 544,
	MARK[9].mark_angle = 0.00,
	MARK[9].mark_x = 309,
	MARK[9].mark_y = 358,

	MARK[10].markID = 576,
	MARK[10].mark_angle = 0.00,
	MARK[10].mark_x = 138,
	MARK[10].mark_y = 540,

	MARK[11].markID = 16,
	MARK[11].mark_angle = 0.00,
	MARK[11].mark_x = 308,
	MARK[11].mark_y = 168,

	MARK[12].markID = 562,
	MARK[12].mark_angle = 0.00,
	MARK[12].mark_x = 311,
	MARK[12].mark_y = 528,

	MARK[13].markID = 530,
	MARK[13].mark_angle = 0.00,
	MARK[13].mark_x = 309,
	MARK[13].mark_y = 1,
};

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);	//初始化这些成员对象需要在connect前
	m_ManualControl=new ManualControl(this);
	m_DashBoard=new DashBoard(this);
	m_timerId=startTimer(1000);		//计数器查询显示机器状态
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
	// 串口初始化Motor/Star/Urg
	if(motor.open_com_motor(COMM_MOTOR)){
		m_DashBoard->ui.ck_Motor->setChecked(true);
		motor.VectorMove(1200,2);	//启动电机后漂移示意
	}
	if(StarGazer.open_com(COMM_STAR)){
		m_DashBoard->ui.ck_Star->setChecked(true);
	}
	for(int loop=0;loop<1000;loop++){
		m_laser_data_postpro[loop] = 50000;
	}
	if (m_cURG.Create(COMM_LASER)){
		m_cURG.SwitchOn();
		m_cURG.SCIP20();	
		m_cURG.GetDataByGD(0,768,1);
		//pThread_Read_Laser=AfxBeginThread(ThreadReadLaser_Data,&Info_laser_data);
		m_DashBoard->ui.ck_URG->setChecked(true);
	}
	//仪表盘数据初始化
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	PosByMotor=QPointF(0.00,0.00);
	PosSafe=QPointF(0.00,0.00);
	PosGoal=QPointF(0.00,0.00);
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
void MainGUI::timerEvent(QTimerEvent *event){
	//刷新仪表盘数据
	if(event->timerId()==m_timerId){
		PosByStar1=QPointF(0.00,0.00);
		PosByStar2=QPointF(0.00,0.00);
		for (int loop_mark = 0; loop_mark < 14; loop_mark++){
			if (MARK[loop_mark].markID == StarGazer.starID){
				PosByStar1.setX(MARK[loop_mark].mark_x + StarGazer.starX);
				PosByStar1.setY(MARK[loop_mark].mark_y + StarGazer.starY);
			}
			if (MARK[loop_mark].markID == StarGazer.starID2){
				PosByStar2.setX(MARK[loop_mark].mark_x + StarGazer.starX2);
				PosByStar2.setY(MARK[loop_mark].mark_y + StarGazer.starY2);
			}
		}
		QString str;
		str.sprintf("(%.2f,%.2f) - (%d)",PosByStar1.x(),PosByStar1.y(),StarGazer.starID);
		m_DashBoard->ui.posStar1->setText(str);
		str.sprintf("(%.2f,%.2f) - (%d)",PosByStar2.x(),PosByStar2.y(),StarGazer.starID2);
		m_DashBoard->ui.posStar2->setText(str);
		str.sprintf("(%.2f,%.2f)",PosByMotor.x(),PosByMotor.y());
		m_DashBoard->ui.posMotor->setText(str);
		str.sprintf("(%.2f,%.2f)",PosSafe.x(),PosSafe.y());
		m_DashBoard->ui.posSafe->setText(str);
		str.sprintf("(%.2f,%.2f)",PosGoal.x(),PosGoal.y());
		m_DashBoard->ui.posGoal->setText(str);
	}
}
UINT MainGUI::ThreadReadLaser_Data(LPVOID lpParam){
	//FILE *allout;
	//allout = fopen("allout2laser.txt","a+");
	//while (lase_key){
	//	m_cURG.GetDataByGD(0,768,1);
	//	WaitForSingleObject(m_cURG.wait_laser,INFINITE);

	//	//	m_cURG.GetDataByGD(0,768,1);//前两个参数决定扫描角度范围（384是正前方的线，288线为90度范围），最后一个参数决定了角度分辨率。获取激光测距起的数据;
	//	//////////////////////////////////////////
	//	Info_laser_data.m_Laser_Data_Point=m_nValPoint_temp;

	//	m_Laser_Data_Point_PostPro=m_nValPoint_temp;
	//	//	pReadThread_postpro->m_Laser_Point=m_nValPoint_temp;

	//	key_laser = !m_cURG.key;
	//	for (int i=0;i<Info_laser_data.m_Laser_Data_Point;i++){
	//		Info_laser_data.m_Laser_Data_Value[i]=m_cURG.m_distVal_temp_test[key_laser][i];
	//		m_laser_data_raw[i]=m_cURG.m_distVal_temp_test[key_laser][i];
	//		m_laser_data_postpro[i] = m_cURG.m_distVal_temp_test[key_laser][i];
	//		if(i%10==0){
	//			fprintf(allout," 0000===%d   \n",m_laser_data_postpro[i]);
	//		}
	//	}
	//	wait_data.SetEvent();
	//	m_cURG.wait_laser.ResetEvent();
	//	wait_laserpose.SetEvent();
	//}
	return 0;
}