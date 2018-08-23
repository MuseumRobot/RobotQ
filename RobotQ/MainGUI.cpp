#include "MainGUI.h"

//若干对象需要自始至终存在于主界面中
CUPURG m_cURG;									//激光对象
CWinThread* pThread_Read_Laser;					//读激光数据线程
UINT ThreadReadLaser_Data(LPVOID lpParam);		//读激光数据函数
bool is_Comm_URG_Open = false;					//初始化激光未开启
bool key_laser = false;							//激光数据存储位置开关
void InitCommLaser();							//激光串口初始化
int m_laser_data_postpro[768];					//激光最远返回值(单位cm)
CEvent wait_data;
CEvent wait_laserpose;
threadInfo_laser_data Info_laser_data;

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);						//初始化这些成员对象需要在connect前
	m_ManualControl=new ManualControl(this);
	m_DashBoard=new DashBoard(this);
	m_timer_refresh_task=startTimer(INSTRUCTION_CYCLE);					//计数器查询分配任务
	m_timer_refresh_dashboard=startTimer(INFOREFRESH_CYCLE);			//计数器查询显示机器状态
	m_timer_refresh_emergency_distance=0;								//依赖分配任务计数器查询刷新恢复制动距离，模20
	if(m_RobotQ->isAuthReady)m_DashBoard->ui.ck_Auth->setChecked(true);
	if(m_RobotQ->isASRReady)m_DashBoard->ui.ck_ASR->setChecked(true);
	if(m_RobotQ->isTTSReady)m_DashBoard->ui.ck_TTS->setChecked(true);
	connect(ui.btnAutoGuide,SIGNAL(clicked()),this,SLOT(OnBtnAutoGuide()));
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
	//因为初始化函数为静态函数
	Init();
}
MainGUI::~MainGUI(){
	delete m_RobotQ;
	delete m_ManualControl;
	delete m_DashBoard;
	m_cURG.SwitchOff();		//关闭激光
}
void MainGUI::Init(){
	//InitStarMark();				//LED标签数组赋值(实验室)
	InitStarMarkMuseum();			//LED标签数组赋值(博物馆)
	InitCommMotorAndStar();			//串口初始化Motor/Star
	InitCommLaser();				//串口初始化URG
	InitDashBoardData();			//仪表盘数据初始化
	InitAdjustGUI();				//调整界面适配使用者
	InitTaskAssignment(1);			//默认分配路线一
	
}
int MainGUI::OnBtnAutoGuide(){
	if(is_Auto_Mode_Open == true){
		is_Auto_Mode_Open = false;
		if(MUSEUMMODE == 0){
			m_DashBoard->ui.ck_Auto->setChecked(false);
			ui.btnAutoGuide->setText("开启自动导航");
		}
	}else{
		currentTodoListId = 0;
		is_mission_accomplished = false;
		is_project_accomplished = false;
		is_Auto_Mode_Open = true;
		if(MUSEUMMODE == 0){
			m_DashBoard->ui.ck_Auto->setChecked(true);
			ui.btnAutoGuide->setText("关闭自动导航");
		}

	}
	return 0;
}
int MainGUI::OnBtnRobotQ(){
	if(MUSEUMMODE ==0 ){
		m_RobotQ->move(800,460);
	}
	m_RobotQ->show();
	return 0;
}
int MainGUI::OnBtnManualControl(){
	m_ManualControl->move(300,20);
	m_ManualControl->show();
	return 0;
}
int MainGUI::OnBtnDashBoard(){
	m_DashBoard->move(750,20);
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
int MainGUI::On_Auto_BtnTurnleft(int level){
	if(level == 0){
		m_motor.VectorMove(0,0.8);
	}else if(level == 1){
		m_motor.VectorMove(0,1.5);
	}
	return 0;
}
int MainGUI::On_Auto_BtnTurnright(int level){
	if(level == 0){
		m_motor.VectorMove(0,-0.8);
	}else if(level == 1){
		m_motor.VectorMove(0,-1.5);
	}
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
		m_timer_refresh_emergency_distance++;
		if(m_timer_refresh_emergency_distance == 20){
			m_timer_refresh_emergency_distance = 0;
			if(m_EMERGENCY_DISTANCE == 0){
				m_EMERGENCY_DISTANCE = EMERGENCY_DISTANCE;
				//RobotQ::RobotQSpeak("紧急制动已恢复");
				//Sleep(3000);
			}
		}
		if(is_Auto_Mode_Open){
			AssignInstruction();		//分配下一步指令
		}
	}
}
UINT ThreadReadLaser_Data(LPVOID lpParam){
	bool laser_key = true;
	while (laser_key){
		m_cURG.GetDataByGD(0,768,1);//前两个参数决定扫描角度范围（384是正前方的线，288线为90度范围），最后一个参数决定了角度分辨率。获取激光测距起的数据;
		WaitForSingleObject(m_cURG.wait_laser,INFINITE);
		Info_laser_data.m_Laser_Data_Point=m_nValPoint_temp;
		key_laser = !m_cURG.key;
		for (int i=0;i<Info_laser_data.m_Laser_Data_Point;i++){
			Info_laser_data.m_Laser_Data_Value[i]=m_cURG.m_distVal_temp_test[key_laser][i];
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
void MainGUI::InitStarMarkMuseum(){
	m_MARK[0].markID = 2;
	m_MARK[0].mark_angle = 0.00;
	m_MARK[0].mark_x = 118;
	m_MARK[0].mark_y = 183;


	m_MARK[1].markID = 16;
	m_MARK[1].mark_angle = 0.00;
	m_MARK[1].mark_x = 283;
	m_MARK[1].mark_y = 243;

	m_MARK[2].markID = 32;
	m_MARK[2].mark_angle = 0.00;
	m_MARK[2].mark_x = 463;
	m_MARK[2].mark_y = 243;

	m_MARK[3].markID = 18;
	m_MARK[3].mark_angle = 0.00;
	m_MARK[3].mark_x = 658;
	m_MARK[3].mark_y = 168;

	m_MARK[4].markID = 624;
	m_MARK[4].mark_angle = 0.00;
	m_MARK[4].mark_x = 193;
	m_MARK[4].mark_y = 363;

	m_MARK[5].markID = 626;
	m_MARK[5].mark_angle = 0.00;
	m_MARK[5].mark_x = 388;
	m_MARK[5].mark_y = 363;


	m_MARK[6].markID = 34;
	m_MARK[6].mark_angle = 180.00;
	m_MARK[6].mark_x = 598;
	m_MARK[6].mark_y = 333;

	m_MARK[7].markID = 610;
	m_MARK[7].mark_angle = 0.00;
	m_MARK[7].mark_x = 178;
	m_MARK[7].mark_y = 528;

	m_MARK[8].markID = 608;
	m_MARK[8].mark_angle = 0.00;
	m_MARK[8].mark_x = 418;
	m_MARK[8].mark_y = 513;

	m_MARK[9].markID = 594;
	m_MARK[9].mark_angle = 0.00;
	m_MARK[9].mark_x = 658;
	m_MARK[9].mark_y = 498;

	m_MARK[10].markID = 578;
	m_MARK[10].mark_angle = 0.00;
	m_MARK[10].mark_x = 268;
	m_MARK[10].mark_y = 648;

	m_MARK[11].markID = 592;
	m_MARK[11].mark_angle = 0.00;
	m_MARK[11].mark_x = 523;
	m_MARK[11].mark_y = 618;

	m_MARK[12].markID = 576;
	m_MARK[12].mark_angle = 0.00;
	m_MARK[12].mark_x = 178;
	m_MARK[12].mark_y = 783;

	m_MARK[13].markID = 50;
	m_MARK[13].mark_angle = 0.00;
	m_MARK[13].mark_x = 403;
	m_MARK[13].mark_y = 738;

	m_MARK[14].markID = 562;
	m_MARK[14].mark_angle = 0.00;
	m_MARK[14].mark_x = 673;
	m_MARK[14].mark_y = 768;

	m_MARK[15].markID = 546,
		m_MARK[15].mark_angle = 0.00,
		m_MARK[15].mark_x = 283,
		m_MARK[15].mark_y = 888,

		m_MARK[16].markID = 560;
	m_MARK[16].mark_angle = 0.00;
	m_MARK[16].mark_x = 523;
	m_MARK[16].mark_y = 858;

	m_MARK[17].markID = 528;
	m_MARK[17].mark_angle = 0.00;
	m_MARK[17].mark_x = 178;
	m_MARK[17].mark_y = 963;

	m_MARK[18].markID = 544;
	m_MARK[18].mark_angle = 0.00;
	m_MARK[18].mark_x = 422.5;
	m_MARK[18].mark_y = 1008;

	m_MARK[19].markID = 530;
	m_MARK[19].mark_angle = 0.00;
	m_MARK[19].mark_x = 673;
	m_MARK[19].mark_y = 1008;

	m_MARK[20].markID = 514;
	m_MARK[20].mark_angle = 0.00;
	m_MARK[20].mark_x = 283;
	m_MARK[20].mark_y = 1083;

	m_MARK[21].markID = 512;
	m_MARK[21].mark_angle = 0.00;
	m_MARK[21].mark_x = 487;
	m_MARK[21].mark_y = 1083;

	m_MARK[22].markID = 80;
	m_MARK[22].mark_angle = 0.00;
	m_MARK[22].mark_x = 178;
	m_MARK[22].mark_y = 1218;

	m_MARK[23].markID = 68;
	m_MARK[23].mark_angle = 0.00;
	m_MARK[23].mark_x = 418;
	m_MARK[23].mark_y = 1188;

	m_MARK[24].markID = 114;
	m_MARK[24].mark_angle = 0.00;
	m_MARK[24].mark_x = 688;
	m_MARK[24].mark_y = 1203;

	m_MARK[25].markID = 82;
	m_MARK[25].mark_angle = 0.00;
	m_MARK[25].mark_x = 268;
	m_MARK[25].mark_y = 1338;

	m_MARK[26].markID = 112;
	m_MARK[26].mark_angle = 0.00;
	m_MARK[26].mark_x = 568;
	m_MARK[26].mark_y = 1353;

	m_MARK[27].markID = 96;
	m_MARK[27].mark_angle = 0.00;
	m_MARK[27].mark_x = 178;
	m_MARK[27].mark_y = 1473;

	m_MARK[28].markID = 48;
	m_MARK[28].mark_angle = 0.00;
	m_MARK[28].mark_x = 273;
	m_MARK[28].mark_y = 1473;

	m_MARK[29].markID = 98;
	m_MARK[29].mark_angle = 0.00;
	m_MARK[29].mark_x = 568;
	m_MARK[29].mark_y = 1473;

	m_MARK[30].markID = 64;
	m_MARK[30].mark_angle = 0.00;
	m_MARK[30].mark_x = 740.5;
	m_MARK[30].mark_y = 1503;
}
void MainGUI::InitCommMotorAndStar(){
	if(m_motor.open_com_motor(COMM_MOTOR)){
		m_DashBoard->ui.ck_Motor->setChecked(true);
	}
	if(m_StarGazer.open_com(COMM_STAR)){
		m_DashBoard->ui.ck_Star->setChecked(true);
	}
}
void InitCommLaser(){
	for(int i=0;i<768;i++)	m_laser_data_postpro[i] = 10000;	//初始化接受激光返回的数据为10000mm
	if (m_cURG.Create(COMM_LASER)){
		m_cURG.SwitchOn();
		m_cURG.SCIP20();	
		m_cURG.GetDataByGD(0,768,1);
		pThread_Read_Laser=AfxBeginThread(ThreadReadLaser_Data,&Info_laser_data);		//开启激光计算线程
		is_Comm_URG_Open = true;
	}	
}
void MainGUI::CalculateSectorDistance(){
	for(int i=0;i<36;i++)sectorObstacleDistance[i]=0;
	int sectorId = 0;
	while(sectorId < 36){	//从6到762这21*36条射线中结果分析
		int n = 0;			//扇区中有障碍物的射线数
		int sum = 0.0;	//障碍物距离总值
		for(int i=0;i<21;i++){
			int k=sectorId*21+i+6;
			if(m_laser_data_postpro[k]>0&&m_laser_data_postpro[k]<OBSTACLE_DISTANCE){
				sum += m_laser_data_postpro[k];
				n++;
			}
		}
		if(n>1){
			sectorObstacleDistance[sectorId] = sum/n;
		}
		sectorId++;
	}
}
void MainGUI::refreshDashboardSector(){
	int threshold = OBSTACLE_DISTANCE;	//激光射线返回的单位为mm
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
	if(sectorObstacleDistance[N-0]<threshold && sectorObstacleDistance[N-0] >0)m_DashBoard->ui.r5->setChecked(flag);
	if(sectorObstacleDistance[N-1]<threshold && sectorObstacleDistance[N-1] >0)m_DashBoard->ui.r10->setChecked(flag);
	if(sectorObstacleDistance[N-2]<threshold && sectorObstacleDistance[N-2] >0)m_DashBoard->ui.r15->setChecked(flag);
	if(sectorObstacleDistance[N-3]<threshold && sectorObstacleDistance[N-3] >0)m_DashBoard->ui.r20->setChecked(flag);
	if(sectorObstacleDistance[N-4]<threshold && sectorObstacleDistance[N-4] >0)m_DashBoard->ui.r25->setChecked(flag);
	if(sectorObstacleDistance[N-5]<threshold && sectorObstacleDistance[N-5] >0)m_DashBoard->ui.r30->setChecked(flag);
	if(sectorObstacleDistance[N-6]<threshold && sectorObstacleDistance[N-6] >0)m_DashBoard->ui.r35->setChecked(flag);
	if(sectorObstacleDistance[N-7]<threshold && sectorObstacleDistance[N-7] >0)m_DashBoard->ui.r40->setChecked(flag);
	if(sectorObstacleDistance[N-8]<threshold && sectorObstacleDistance[N-8] >0)m_DashBoard->ui.r45->setChecked(flag);
	if(sectorObstacleDistance[N-9]<threshold && sectorObstacleDistance[N-9] >0)m_DashBoard->ui.r50->setChecked(flag);
	if(sectorObstacleDistance[N-10]<threshold && sectorObstacleDistance[N-10] >0)m_DashBoard->ui.r55->setChecked(flag);
	if(sectorObstacleDistance[N-11]<threshold && sectorObstacleDistance[N-11] >0)m_DashBoard->ui.r60->setChecked(flag);
	if(sectorObstacleDistance[N-12]<threshold && sectorObstacleDistance[N-12] >0)m_DashBoard->ui.r65->setChecked(flag);
	if(sectorObstacleDistance[N-13]<threshold && sectorObstacleDistance[N-13] >0)m_DashBoard->ui.r70->setChecked(flag);
	if(sectorObstacleDistance[N-14]<threshold && sectorObstacleDistance[N-14] >0)m_DashBoard->ui.r75->setChecked(flag);
	if(sectorObstacleDistance[N-15]<threshold && sectorObstacleDistance[N-15] >0)m_DashBoard->ui.r80->setChecked(flag);
	if(sectorObstacleDistance[N-16]<threshold && sectorObstacleDistance[N-16] >0)m_DashBoard->ui.r85->setChecked(flag);
	if(sectorObstacleDistance[N-17]<threshold && sectorObstacleDistance[N-17] >0)m_DashBoard->ui.r90->setChecked(flag);
	if(sectorObstacleDistance[N-18]<threshold && sectorObstacleDistance[N-18] >0)m_DashBoard->ui.r95->setChecked(flag);
	if(sectorObstacleDistance[N-19]<threshold && sectorObstacleDistance[N-19] >0)m_DashBoard->ui.r100->setChecked(flag);
	if(sectorObstacleDistance[N-20]<threshold && sectorObstacleDistance[N-20] >0)m_DashBoard->ui.r105->setChecked(flag);
	if(sectorObstacleDistance[N-21]<threshold && sectorObstacleDistance[N-21] >0)m_DashBoard->ui.r110->setChecked(flag);
	if(sectorObstacleDistance[N-22]<threshold && sectorObstacleDistance[N-22] >0)m_DashBoard->ui.r115->setChecked(flag);
	if(sectorObstacleDistance[N-23]<threshold && sectorObstacleDistance[N-23] >0)m_DashBoard->ui.r120->setChecked(flag);
	if(sectorObstacleDistance[N-24]<threshold && sectorObstacleDistance[N-24] >0)m_DashBoard->ui.r125->setChecked(flag);
	if(sectorObstacleDistance[N-25]<threshold && sectorObstacleDistance[N-25] >0)m_DashBoard->ui.r130->setChecked(flag);
	if(sectorObstacleDistance[N-26]<threshold && sectorObstacleDistance[N-26] >0)m_DashBoard->ui.r135->setChecked(flag);
	if(sectorObstacleDistance[N-27]<threshold && sectorObstacleDistance[N-27] >0)m_DashBoard->ui.r140->setChecked(flag);
	if(sectorObstacleDistance[N-28]<threshold && sectorObstacleDistance[N-28] >0)m_DashBoard->ui.r145->setChecked(flag);
	if(sectorObstacleDistance[N-29]<threshold && sectorObstacleDistance[N-29] >0)m_DashBoard->ui.r150->setChecked(flag);
	if(sectorObstacleDistance[N-30]<threshold && sectorObstacleDistance[N-30] >0)m_DashBoard->ui.r155->setChecked(flag);
	if(sectorObstacleDistance[N-31]<threshold && sectorObstacleDistance[N-31] >0)m_DashBoard->ui.r160->setChecked(flag);
	if(sectorObstacleDistance[N-32]<threshold && sectorObstacleDistance[N-32] >0)m_DashBoard->ui.r165->setChecked(flag);
	if(sectorObstacleDistance[N-33]<threshold && sectorObstacleDistance[N-33] >0)m_DashBoard->ui.r170->setChecked(flag);
	if(sectorObstacleDistance[N-34]<threshold && sectorObstacleDistance[N-34] >0)m_DashBoard->ui.r175->setChecked(flag);
	if(sectorObstacleDistance[N-35]<threshold && sectorObstacleDistance[N-35] >0)m_DashBoard->ui.r180->setChecked(flag);
}
void MainGUI::refreshDashboardData(){
	if(is_Comm_URG_Open)m_DashBoard->ui.ck_URG->setChecked(true);	//判断电机是否开启
	JudgeForwardSituation();		//判断前方是否畅通无阻
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	for (int loop_mark = 0; loop_mark < MARKNUM - 1; loop_mark++){
		if (m_MARK[loop_mark].markID == m_StarGazer.starID){
			
			PosByStar1.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX);
			PosByStar1.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY);
			AngleByStar1 = m_StarGazer.starAngel;
			
			//if(m_MARK[loop_mark].markID !=34 && m_MARK[loop_mark].markID !=64 && m_MARK[loop_mark].markID !=66){
				AngleByStar1>0?AngleSafe=AngleByStar1:AngleSafe=AngleByStar1+360.0;		//得到机器人本体的朝向(顺时针)
				float dx = Distance_Robot_forward_StarGazer * zTool_cos_angle(AngleSafe);
				float dy = Distance_Robot_forward_StarGazer * zTool_sin_angle(AngleSafe);
				PosSafe = QPointF(PosByStar1.x()+dx,PosByStar1.y()+dy);
			//}
			AngleSafe = 360.0 - AngleSafe;											//得到机器人本体朝向（逆时针）
		}
		if (m_MARK[loop_mark].markID == m_StarGazer.starID2){
			PosByStar2.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX2);
			PosByStar2.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY2);
			AngleByStar2 = m_StarGazer.starAngel;
			//if(m_MARK[loop_mark].markID ==34 && m_MARK[loop_mark].markID ==64 && m_MARK[loop_mark].markID ==66){
			//	AngleByStar2>0?AngleSafe=AngleByStar2:AngleSafe=AngleByStar2+360.0;		//得到机器人本体的朝向(顺时针)
			//	float dx = Distance_Robot_forward_StarGazer * zTool_cos_angle(AngleSafe);
			//	float dy = Distance_Robot_forward_StarGazer * zTool_sin_angle(AngleSafe);
			//	PosSafe = QPointF(PosByStar2.x()+dx,PosByStar2.y()+dy);
			//}
		}
	}
	QPointF d = PosGoal - PosSafe;
	if(d.x() > 0 && d.y() > 0){
		Angle_face_Goal =atan((d.y())/d.x())/PI/2*360.0;	//目标在第一象限
	}else if(d.x() < 0 && d.y() < 0){
		Angle_face_Goal =180 + atan((d.y())/d.x())/PI/2*360.0;	//目标在第三象限
	}else if(d.x() <0 && d.y() > 0){
		Angle_face_Goal =180 + atan((d.y())/d.x())/PI/2*360.0;	//目标在第二象限
	}else{
		Angle_face_Goal =360 + atan((PosGoal.y()-PosSafe.y())/(PosGoal.x()-PosSafe.x()))/PI/2*360.0;	//目标在第四象限
	}

	QString str;
	str.sprintf("(%.2f,%.2f)-(%.2f°)-(%d)",PosByStar1.x(),PosByStar1.y(),AngleByStar1,m_StarGazer.starID);
	m_DashBoard->ui.posStar1->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f°)-(%d)",PosByStar2.x(),PosByStar2.y(),AngleByStar2,m_StarGazer.starID2);
	m_DashBoard->ui.posStar2->setText(str);
	str.sprintf("(%.2f,%.2f)",PosByMotor.x(),PosByMotor.y());
	m_DashBoard->ui.posMotor->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f°)",PosSafe.x(),PosSafe.y(),AngleSafe);
	m_DashBoard->ui.posSafe->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f°)",PosGoal.x(),PosGoal.y(),Angle_face_Goal);
	m_DashBoard->ui.posGoal->setText(str);

	//更新任务显示
	if(is_project_accomplished == false){
		str = "任务序号:";str += QString("%2").arg(currentTodoListId);str +="(代号:";str += QString("%2").arg(todoList[currentTodoListId]);str += ")";
		m_DashBoard->ui.text_CurrentTaskId->setText(str);
	}
	int i=0;
	str = "";
	while(todoList[i] != 0){
		QString a =QString("%2").arg(todoList[i]);str += a;str += " ";i++;
	}
	m_DashBoard->ui.text_Todolist->setText(str);
}
void MainGUI::InitDashBoardData(){
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	PosByMotor=QPointF(0.00,0.00);
	PosSafe=QPointF(0.00,0.00);
	PosGoal=QPointF(50.00,50.00);
	is_Auto_Mode_Open = false;
	is_mission_accomplished = false;
	is_project_accomplished = false;
	is_path_clear = true;
	is_dodge_moment = false;
	dodge_move_times = 0;
	m_EMERGENCY_DISTANCE = EMERGENCY_DISTANCE;
	Emergency_times = 0;
}
void MainGUI::AssignInstruction(){
	bool isEMERGENCY = false;
	for(int i=9;i<27;i++){
		if(sectorObstacleDistance[i]>0 && sectorObstacleDistance[i] < OBSTACLE_DISTANCE){
			if(sectorObstacleDistance[i] > 0 && sectorObstacleDistance[i] < m_EMERGENCY_DISTANCE){
				isEMERGENCY =true;
				break;
			}
		}
	}
	if(isEMERGENCY && is_Auto_Mode_Open && m_DashBoard->ui.ck_isEmergency->isChecked()){
		Emergency_times++;
		m_motor.stop();
		RobotQ::RobotQSpeak("紧急制动!您挡到小灵啦!");
		Sleep(5000);
		if(Emergency_times == 3){
			Emergency_times = 0;
			m_EMERGENCY_DISTANCE = 0;
			RobotQ::RobotQSpeak("紧急制动已解除，10秒后恢复");
			m_timer_refresh_emergency_distance = 0;
		}
	}else{	
		//非紧急情况下正常发配指令
		if(is_project_accomplished == false){
			if(is_mission_accomplished == false){
				int taskID=todoList[currentTodoListId];		
				if (taskID<10){		//taskID<10意味着是路径点
					AssignGoalPos(taskID);	//分配目标坐标
					float errorRange_Angle = ERRORANGLE;				//选择角度的误差范围，单位°
					float errorRange_Distance = ERRORDISTANCE;			//抵达目标点的距离误差范围，单位cm
					QPointF d = PosGoal - PosSafe;
					float dDistance = sqrt(pow(d.x(),2)+pow(d.y(),2));	//机器人中心到目标点的距离，单位cm
					if(dDistance > errorRange_Distance){
						if(is_dodge_moment == true){
							if((is_path_clear == true && dodge_move_times > DODGESTEPS-2) || dodge_move_times > DODGESTEPS){
								is_dodge_moment = false;				//超出闪避有效步数后，如果当前前路通畅，则退出闪避时刻
								m_DashBoard->ui.ck_isDodgetime->setChecked(false);
							}else{
								float d = AngleSafe-Angle_face_Goal;
								if(d > 0.0 && d< 90.0 || d < -270.0 ){
									DodgeTurnRight();
								}else{
									DodgeTurnRight();
								}

							}
						}else{
							//如果没有进入闪避时间，则正常运行
							if(abs(Angle_face_Goal-AngleSafe)>errorRange_Angle){		
								Rotate_to_GoalAngle(Angle_face_Goal);	//如果和目标角度差距大就先看一眼目标
							}else{
								if(is_path_clear == true){
									On_MC_BtnForward();					//转到想要的角度后如果前路通畅就继续走
								}else{
									//如果前路不通畅，进入闪避时刻
									is_dodge_moment = true;
									dodge_move_times = 0;				//清空闪避操作次数计数器
									m_DashBoard->ui.ck_isDodgetime->setChecked(true);
								}
							}
						}
					}else{
						is_mission_accomplished = true;
						QString TaskCode;
						TaskCode.sprintf("任务代号%d",taskID);
						m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "我已经到达目标点," + TaskCode);
						RobotQ::RobotQSpeak("我已经到达目标点," + TaskCode);
						Sleep(3000);
						currentTodoListId++;
						is_mission_accomplished = false;	//一个任务完成后，怒吼一下，休息3秒，又开始接新任务了
					}
				}else{
					//taskID>10意味着是语音播报点
					QString str;
					if(taskID == 11 ){
						str="大家好，我是哈尔滨工业大学智能导览机器人小灵，我正在测试，请不要靠近我哦!"; 
					}else{
						str="呵呵呵";
					}
					Sleep(3000);
					m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + str);
					RobotQ::RobotQSpeak(str);
					currentTodoListId++;
				}
			}
		}	
	}
}
float MainGUI::zTool_cos_angle(float angle){
	return cos(angle/360.0*2*PI);
}
float MainGUI::zTool_sin_angle(float angle){
	return sin(angle/360.0*2*PI);
}
void MainGUI::Rotate_to_GoalAngle(float AngleGoal){
	float dAngle=AngleGoal-AngleSafe;	//目标朝向与当前朝向的角度差
	if(dAngle>360.0){
		dAngle -=360;
	}else if(dAngle<0.0){
		dAngle +=360;
	}
	if(dAngle>0 && dAngle<180){
		if(dAngle<30){
			On_Auto_BtnTurnleft(0);
		}else{
			On_Auto_BtnTurnleft(1);
		 }
	}else{
		if(dAngle>330){
			On_Auto_BtnTurnright(0);
		}else{
			On_Auto_BtnTurnright(1);
		}
	}
}
void MainGUI::JudgeForwardSituation(){
	int n=0;
	for(int i=9;i<27;i++){
		if(sectorObstacleDistance[i]>0 && sectorObstacleDistance[i] < OBSTACLE_DISTANCE){
			n++;
		}
	}
	if(n>0){
		is_path_clear = false;
		m_DashBoard->ui.ck_isClear->setChecked(false);
	}else{
		is_path_clear = true;
		m_DashBoard->ui.ck_isClear->setChecked(true);
	}

}
void MainGUI::DodgeTurnRight(){
	if(is_path_clear){
		On_MC_BtnForward();		//在向右转到前方无障碍时前进一步
		dodge_move_times++;		//只有在躲避时刻中进行push操作才是有效操作，原地转圈没啥用
	}else{
		On_Auto_BtnTurnright(0);
	}
}
void MainGUI::DodgeTurnLeft(){
	if(is_path_clear){
		On_MC_BtnForward();		//在向左转到前方无障碍时前进一步
		dodge_move_times++;		//只有在躲避时刻中进行push操作才是有效操作，原地转圈没啥用
	}else{
		On_Auto_BtnTurnleft(0);
	}
}
void MainGUI::InitAdjustGUI(){
	if(MUSEUMMODE == 1){
		delete ui.centralWidget;		//清除所有开发者界面控件重新编写布局
		QSize MainGUISize(640,480);		//主界面尺寸
		QSize RobotQSize(540,380);		//语音交互截面尺寸
		QSize btnRobotQSize(150,50);	//语音交互按钮尺寸
		QSize btnPathSize(180,36);

		QLabel* MainBackGround = new QLabel(this);
		QPushButton* btnRobotQ = new QPushButton(this);
		QPushButton* btnAutoGuide = new QPushButton(this);
		QPushButton* btnPath1 = new QPushButton(this);
		QPushButton* btnPath2 = new QPushButton(this);
		QPushButton* btnPath3 = new QPushButton(this);
		
		const QIcon pic_RobotQ_MainGUI = QIcon("Resources/语音交互.bmp");
		const QIcon pic_AutoGuide_MainGUI = QIcon("Resources/自动导航.bmp");
		const QIcon pic_Path1_MainGUI = QIcon("Resources/路线1.bmp");
		const QIcon pic_Path2_MainGUI = QIcon("Resources/路线2.bmp");
		const QIcon pic_Path3_MainGUI = QIcon("Resources/路线3.bmp");
		const QPixmap pic_BG_MainGUI = QPixmap("Resources/博物馆背景.bmp");

		resize(MainGUISize);
		MainBackGround->resize(MainGUISize);
		MainBackGround->setPixmap(pic_BG_MainGUI);
		MainBackGround->setScaledContents(true);

		btnRobotQ->move(60,50);
		btnRobotQ->resize(btnRobotQSize);
		btnRobotQ->setIconSize(btnRobotQSize);
		btnRobotQ->setIcon(pic_RobotQ_MainGUI);

		btnAutoGuide->move(60,120);
		btnAutoGuide->resize(btnRobotQSize);
		btnAutoGuide->setIconSize(btnRobotQSize);
		btnAutoGuide->setIcon(pic_AutoGuide_MainGUI);

		btnPath1->move(360,280);
		btnPath1->resize(btnPathSize);
		btnPath1->setIconSize(btnPathSize);
		btnPath1->setIcon(pic_Path1_MainGUI);

		btnPath2->move(360,330);
		btnPath2->resize(btnPathSize);
		btnPath2->setIconSize(btnPathSize);
		btnPath2->setIcon(pic_Path2_MainGUI);

		btnPath3->move(360,380);
		btnPath3->resize(btnPathSize);
		btnPath3->setIconSize(btnPathSize);
		btnPath3->setIcon(pic_Path3_MainGUI);

		m_RobotQ->move(130,110);
		m_RobotQ->resize(RobotQSize);
		
		connect(btnRobotQ,SIGNAL(clicked()),this,SLOT(OnBtnRobotQ()));
		connect(btnAutoGuide,SIGNAL(clicked()),this,SLOT(OnBtnAutoGuide()));
		
	}else{	//进入开发者模式
		OnBtnDashBoard();				//开启仪表盘面板
		OnBtnManualControl();			//开启遥控器面板
		//OnBtnRobotQ();					//开启语音对话面板
	}
}
void MainGUI::InitTaskAssignment(int n){
	//初始化任务分配（路线X）
	if(n ==1 ){	//路线一
		currentTodoListId = 0;	//初始化当前todolist的下标为0
		todoList[0] = 1;	//(227,1514)
		todoList[1] = 11;
		todoList[2] = 2;	//(632,1418)
		todoList[3] = 11;
		todoList[4] = 3;	//(687,374)
		todoList[5] = 11;	//语音点
		todoList[6] = 4;	//(205,277)
		todoList[7] = 1;	
		todoList[8] = 0;	//休止符
	}
}
void MainGUI::AssignGoalPos(int taskID){
	//if(taskID == 1){
	//	PosGoal = QPointF(227.0,1514.0);
	//}else if(taskID == 2){
	//	PosGoal = QPointF(632.0,1418.0);
	//}else if(taskID == 5){
	//	PosGoal = QPointF(687.0,1450.0);
	//}else if(taskID == 0){
	//	is_project_accomplished = true;
	//	Sleep(3000);
	//	m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "我已经完成整个项目");
	//	RobotQ::RobotQSpeak("我已经完成整个项目");
	//	is_Auto_Mode_Open = false;
	//	m_DashBoard->ui.ck_Auto->setChecked(false);
	//	ui.btnAutoGuide->setText("开启自动导航");
	//}
	switch(taskID){
	case 1:PosGoal = QPointF(227.0,1514.0);break;
	case 2:PosGoal = QPointF(632.0,1418.0);break;
	case 3:PosGoal = QPointF(687.0,374.0);break;
	case 4:PosGoal = QPointF(205.0,277.0);break;
	default:{
				is_project_accomplished = true;
				Sleep(3000);
				m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "我已经完成整个项目");
				RobotQ::RobotQSpeak("我已经完成整个项目");
				is_Auto_Mode_Open = false;
				m_DashBoard->ui.ck_Auto->setChecked(false);
				ui.btnAutoGuide->setText("开启自动导航");
			}
	}
}