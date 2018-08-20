#include "MainGUI.h"

//若干对象需要自始至终存在于主界面中
CUPURG m_cURG;									//激光对象
CWinThread* pThread_Read_Laser;					//读激光数据线程
//CWinThread* pThread_Cal_Obstacle_Dist;			//计算障碍物距离线程
UINT ThreadReadLaser_Data(LPVOID lpParam);		//读激光数据函数
bool is_Comm_URG_Open = false;					//初始化激光未开启
bool key_laser = false;							//激光数据存储位置开关
void InitCommLaser();							//激光串口初始化
//float sectorObstacleDistance[36];				//每五度划分一个扇区
int m_laser_data_postpro[768];					//激光最远返回值(单位cm)
int m_laser_data_raw[768];
int m_Laser_Data_Point_PostPro;
CEvent wait_data;
CEvent wait_laserpose;
threadInfo_laser_data Info_laser_data;

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);	//初始化这些成员对象需要在connect前
	m_ManualControl=new ManualControl(this);
	m_DashBoard=new DashBoard(this);
	m_timer_refresh_task=startTimer(1000);
	m_timer_refresh_dashboard=startTimer(600);		//计数器查询显示机器状态
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
	
	InitStarMark();		//LED标签数组赋值
	InitCommMotorAndStar();			//串口初始化Motor/Star
	InitCommLaser();	//串口初始化URG

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
	m_motor.VectorMove(800,0);
	return 0;
}
int MainGUI::On_MC_BtnBackward(){
	m_motor.VectorMove(-800,0);
	return 0;
}
int MainGUI::On_MC_BtnTurnleft(){
	m_motor.VectorMove(0,1);
	return 0;
}
int MainGUI::On_MC_BtnTurnright(){
	m_motor.VectorMove(0,-1);
	return 0;
}
int MainGUI::On_MC_BtnStopmove(){
	m_motor.stop();
	return 0;
}
int MainGUI::On_MC_BtnRobotQSpeak(){
	QString str;
	str=m_ManualControl->ui.comSpeaklist->currentText();
	RobotQ::RobotQSpeak(str);
	return 0;
}
void MainGUI::timerEvent(QTimerEvent *event){
	if(event->timerId()==m_timer_refresh_dashboard){
		CalculateSectorDistance();		//计算扇区内障碍物距离
		refreshDashboardSector();		//刷新障碍物分布图
		refreshDashboardData();			//刷新仪表盘数据
	}else if(event->timerId()==m_timer_refresh_task){
		int randomTask=rand()%6;
		switch (randomTask){
			case 0:m_motor.VectorMove(800,0);break;
			case 1:m_motor.VectorMove(-800,0);break;
			case 2:m_motor.VectorMove(0,100);break;
			case 3:m_motor.VectorMove(0,-100);break;
			case 4:m_motor.stop();
			default:RobotQ::RobotQSpeak("呵呵呵");
		}
		//m_motor.VectorMove(800,0);
	}
}
UINT ThreadReadLaser_Data(LPVOID lpParam){
	bool laser_key = true;
	while (laser_key){
		m_cURG.GetDataByGD(0,768,1);//前两个参数决定扫描角度范围（384是正前方的线，288线为90度范围），最后一个参数决定了角度分辨率。获取激光测距起的数据;
		WaitForSingleObject(m_cURG.wait_laser,INFINITE);
		Info_laser_data.m_Laser_Data_Point=m_nValPoint_temp;
		m_Laser_Data_Point_PostPro=m_nValPoint_temp;
		key_laser = !m_cURG.key;
		for (int i=0;i<Info_laser_data.m_Laser_Data_Point;i++){
			Info_laser_data.m_Laser_Data_Value[i]=m_cURG.m_distVal_temp_test[key_laser][i];
			m_laser_data_raw[i]=m_cURG.m_distVal_temp_test[key_laser][i];
			m_laser_data_postpro[i] = m_cURG.m_distVal_temp_test[key_laser][i];
		}
		wait_data.SetEvent();
		m_cURG.wait_laser.ResetEvent();
		wait_laserpose.SetEvent();
	}
	return 0;
}
void MainGUI::InitStarMark(){
	m_MARK[0].markID = 624;
	m_MARK[0].mark_angle = 0.00;
	m_MARK[0].mark_x = -427;
	m_MARK[0].mark_y = 10;

	m_MARK[1].markID = 610;
	m_MARK[1].mark_angle = 0.00;
	m_MARK[1].mark_x = -344;
	m_MARK[1].mark_y = 132;

	m_MARK[2].markID = 594;
	m_MARK[2].mark_angle = 0.00;
	m_MARK[2].mark_x = -212;
	m_MARK[2].mark_y = 0;

	m_MARK[3].markID = 608;
	m_MARK[3].mark_angle = 0.00;
	m_MARK[3].mark_x = -20;
	m_MARK[3].mark_y = 30;

	m_MARK[4].markID = 626;
	m_MARK[4].mark_angle = 0.00;
	m_MARK[4].mark_x = -195;
	m_MARK[4].mark_y = 179;

	m_MARK[5].markID = 592;
	m_MARK[5].mark_angle = 0.00;
	m_MARK[5].mark_x = -6;
	m_MARK[5].mark_y = 177;

	m_MARK[6].markID = 560;
	m_MARK[6].mark_angle = 0.00;
	m_MARK[6].mark_x = 160;
	m_MARK[6].mark_y = 12;

	m_MARK[7].markID = 546;
	m_MARK[7].mark_angle = 0.00;
	m_MARK[7].mark_x = 150;
	m_MARK[7].mark_y = 183;

	m_MARK[8].markID = 578;
	m_MARK[8].mark_angle = 0.00;
	m_MARK[8].mark_x = 140;
	m_MARK[8].mark_y = 377;

	m_MARK[9].markID = 544;
	m_MARK[9].mark_angle = 0.00;
	m_MARK[9].mark_x = 309;
	m_MARK[9].mark_y = 358;

	m_MARK[10].markID = 576;
	m_MARK[10].mark_angle = 0.00;
	m_MARK[10].mark_x = 138;
	m_MARK[10].mark_y = 540;

	m_MARK[11].markID = 16;
	m_MARK[11].mark_angle = 0.00;
	m_MARK[11].mark_x = 308;
	m_MARK[11].mark_y = 168;

	m_MARK[12].markID = 562;
	m_MARK[12].mark_angle = 0.00;
	m_MARK[12].mark_x = 311;
	m_MARK[12].mark_y = 528;

	m_MARK[13].markID = 530;
	m_MARK[13].mark_angle = 0.00;
	m_MARK[13].mark_x = 309;
	m_MARK[13].mark_y = 1;
}
void MainGUI::InitCommMotorAndStar(){
	if(m_motor.open_com_motor(COMM_MOTOR)){
		m_DashBoard->ui.ck_Motor->setChecked(true);
		m_motor.VectorMove(800,0);	//启动电机后漂移示意
	}
	if(m_StarGazer.open_com(COMM_STAR)){
		m_DashBoard->ui.ck_Star->setChecked(true);
	}
}
void InitCommLaser(){
	for(int loop=0;loop<1000;loop++){
		m_laser_data_postpro[loop] = 50000;
	}
	if (m_cURG.Create(COMM_LASER)){
		m_cURG.SwitchOn();
		m_cURG.SCIP20();	
		m_cURG.GetDataByGD(0,768,1);
		pThread_Read_Laser=AfxBeginThread(ThreadReadLaser_Data,&Info_laser_data);
		//pThread_Cal_Obstacle_Dist=AfxBeginThread(CalculateSectorDistance,&sectorObstacleDistance);
		is_Comm_URG_Open = true;
	}	
}
void MainGUI::CalculateSectorDistance(){
	for(int i=0;i<36;i++)sectorObstacleDistance[i]=0.0;
	int sectorId = 0;
	while(sectorId < 36){	//从6到762这21*36条射线中结果分析
		int n = 0;	//扇区中有障碍物的射线数
		float sum = 0.0;	//障碍物距离总值
		for(int i=0;i<21;i++){
			int k=sectorId*21+i;
			if(m_laser_data_postpro[k]>100&&m_laser_data_postpro[k]<50000){
				sum += m_laser_data_postpro[k];
				n++;
			}
		}
		if(n>0){
			sectorObstacleDistance[sectorId] = sum/n;
		}
		sectorId++;
	}
}
void MainGUI::refreshDashboardSector(){
	float threshold = 2000.0;
	bool flag = false;
	m_DashBoard->ui.r5->setChecked(flag);
	m_DashBoard->ui.r10->setChecked(flag);
	m_DashBoard->ui.r15->setChecked(flag);
	m_DashBoard->ui.r20->setChecked(flag);
	m_DashBoard->ui.r25->setChecked(flag);
	m_DashBoard->ui.r30->setChecked(flag);
	m_DashBoard->ui.r35->setChecked(flag);
	m_DashBoard->ui.r40->setChecked(flag);
	m_DashBoard->ui.r45->setChecked(flag);
	m_DashBoard->ui.r50->setChecked(flag);
	m_DashBoard->ui.r55->setChecked(flag);
	m_DashBoard->ui.r60->setChecked(flag);
	m_DashBoard->ui.r65->setChecked(flag);
	m_DashBoard->ui.r70->setChecked(flag);
	m_DashBoard->ui.r75->setChecked(flag);
	m_DashBoard->ui.r80->setChecked(flag);
	m_DashBoard->ui.r85->setChecked(flag);
	m_DashBoard->ui.r90->setChecked(flag);
	m_DashBoard->ui.r95->setChecked(flag);
	m_DashBoard->ui.r100->setChecked(flag);
	m_DashBoard->ui.r105->setChecked(flag);
	m_DashBoard->ui.r110->setChecked(flag);
	m_DashBoard->ui.r115->setChecked(flag);
	m_DashBoard->ui.r120->setChecked(flag);
	m_DashBoard->ui.r125->setChecked(flag);
	m_DashBoard->ui.r130->setChecked(flag);
	m_DashBoard->ui.r135->setChecked(flag);
	m_DashBoard->ui.r140->setChecked(flag);
	m_DashBoard->ui.r145->setChecked(flag);
	m_DashBoard->ui.r150->setChecked(flag);
	m_DashBoard->ui.r155->setChecked(flag);
	m_DashBoard->ui.r160->setChecked(flag);
	m_DashBoard->ui.r165->setChecked(flag);
	m_DashBoard->ui.r170->setChecked(flag);
	m_DashBoard->ui.r175->setChecked(flag);
	m_DashBoard->ui.r180->setChecked(flag);
	flag = true;
	int N = 35;
	if(sectorObstacleDistance[N-0]<threshold)m_DashBoard->ui.r5->setChecked(flag);
	if(sectorObstacleDistance[N-1]<threshold)m_DashBoard->ui.r10->setChecked(flag);
	if(sectorObstacleDistance[N-2]<threshold)m_DashBoard->ui.r15->setChecked(flag);
	if(sectorObstacleDistance[N-3]<threshold)m_DashBoard->ui.r20->setChecked(flag);
	if(sectorObstacleDistance[N-4]<threshold)m_DashBoard->ui.r25->setChecked(flag);
	if(sectorObstacleDistance[N-5]<threshold)m_DashBoard->ui.r30->setChecked(flag);
	if(sectorObstacleDistance[N-6]<threshold)m_DashBoard->ui.r35->setChecked(flag);
	if(sectorObstacleDistance[N-7]<threshold)m_DashBoard->ui.r40->setChecked(flag);
	if(sectorObstacleDistance[N-8]<threshold)m_DashBoard->ui.r45->setChecked(flag);
	if(sectorObstacleDistance[N-9]<threshold)m_DashBoard->ui.r50->setChecked(flag);
	if(sectorObstacleDistance[N-10]<threshold)m_DashBoard->ui.r55->setChecked(flag);
	if(sectorObstacleDistance[N-11]<threshold)m_DashBoard->ui.r60->setChecked(flag);
	if(sectorObstacleDistance[N-12]<threshold)m_DashBoard->ui.r65->setChecked(flag);
	if(sectorObstacleDistance[N-13]<threshold)m_DashBoard->ui.r70->setChecked(flag);
	if(sectorObstacleDistance[N-14]<threshold)m_DashBoard->ui.r75->setChecked(flag);
	if(sectorObstacleDistance[N-15]<threshold)m_DashBoard->ui.r80->setChecked(flag);
	if(sectorObstacleDistance[N-16]<threshold)m_DashBoard->ui.r85->setChecked(flag);
	if(sectorObstacleDistance[N-17]<threshold)m_DashBoard->ui.r90->setChecked(flag);
	if(sectorObstacleDistance[N-18]<threshold)m_DashBoard->ui.r95->setChecked(flag);
	if(sectorObstacleDistance[N-19]<threshold)m_DashBoard->ui.r100->setChecked(flag);
	if(sectorObstacleDistance[N-20]<threshold)m_DashBoard->ui.r105->setChecked(flag);
	if(sectorObstacleDistance[N-21]<threshold)m_DashBoard->ui.r110->setChecked(flag);
	if(sectorObstacleDistance[N-22]<threshold)m_DashBoard->ui.r115->setChecked(flag);
	if(sectorObstacleDistance[N-23]<threshold)m_DashBoard->ui.r120->setChecked(flag);
	if(sectorObstacleDistance[N-24]<threshold)m_DashBoard->ui.r125->setChecked(flag);
	if(sectorObstacleDistance[N-25]<threshold)m_DashBoard->ui.r130->setChecked(flag);
	if(sectorObstacleDistance[N-26]<threshold)m_DashBoard->ui.r135->setChecked(flag);
	if(sectorObstacleDistance[N-27]<threshold)m_DashBoard->ui.r140->setChecked(flag);
	if(sectorObstacleDistance[N-28]<threshold)m_DashBoard->ui.r145->setChecked(flag);
	if(sectorObstacleDistance[N-29]<threshold)m_DashBoard->ui.r150->setChecked(flag);
	if(sectorObstacleDistance[N-30]<threshold)m_DashBoard->ui.r155->setChecked(flag);
	if(sectorObstacleDistance[N-31]<threshold)m_DashBoard->ui.r160->setChecked(flag);
	if(sectorObstacleDistance[N-32]<threshold)m_DashBoard->ui.r165->setChecked(flag);
	if(sectorObstacleDistance[N-33]<threshold)m_DashBoard->ui.r170->setChecked(flag);
	if(sectorObstacleDistance[N-34]<threshold)m_DashBoard->ui.r175->setChecked(flag);
	if(sectorObstacleDistance[N-35]<threshold)m_DashBoard->ui.r180->setChecked(flag);
}
void MainGUI::refreshDashboardData(){
	if(is_Comm_URG_Open)m_DashBoard->ui.ck_URG->setChecked(true);	//判断电机是否开启
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	for (int loop_mark = 0; loop_mark < 14; loop_mark++){
		if (m_MARK[loop_mark].markID == m_StarGazer.starID){
			PosByStar1.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX);
			PosByStar1.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY);
		}
		if (m_MARK[loop_mark].markID == m_StarGazer.starID2){
			PosByStar2.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX2);
			PosByStar2.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY2);
		}
	}
	QString str;
	str.sprintf("(%.2f,%.2f) - (%d)",PosByStar1.x(),PosByStar1.y(),m_StarGazer.starID);
	m_DashBoard->ui.posStar1->setText(str);
	str.sprintf("(%.2f,%.2f) - (%d)",PosByStar2.x(),PosByStar2.y(),m_StarGazer.starID2);
	m_DashBoard->ui.posStar2->setText(str);
	str.sprintf("(%.2f,%.2f)",PosByMotor.x(),PosByMotor.y());
	m_DashBoard->ui.posMotor->setText(str);
	str.sprintf("(%.2f,%.2f)",PosSafe.x(),PosSafe.y());
	m_DashBoard->ui.posSafe->setText(str);
	str.sprintf("(%.2f,%.2f)",PosGoal.x(),PosGoal.y());
	m_DashBoard->ui.posGoal->setText(str);
}