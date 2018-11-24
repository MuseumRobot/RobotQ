#include "stdafx.h"
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

MainGUI::MainGUI(QWidget *parent): QDialog(parent){
	ui.setupUi(this);
	m_RobotQ = new RobotQ(this);						//初始化这些成员对象需要在connect前
	m_ManualControl = new ManualControl(this);
	m_DashBoard = new DashBoard(this);
	m_MuseumGUI = new MuseumGUI(this);
	m_popup_secondScreen_image = new PopupDialog(this);	//初始化第二屏幕弹出窗口
	m_timer_refresh_task = startTimer(INSTRUCTION_CYCLE);					//计数器查询分配任务
	m_timer_refresh_dashboard = startTimer(INFOREFRESH_CYCLE);			//计数器查询显示机器状态
	connect(ui.btnAutoGuide,SIGNAL(clicked()),this,SLOT(OnBtnAutoGuide()));
	connect(ui.btnRobotQ,SIGNAL(clicked()),this,SLOT(OnBtnRobotQ()));
	connect(ui.btnManualControl,SIGNAL(clicked()),this,SLOT(OnBtnManualControl()));
	connect(ui.btnDashBoard,SIGNAL(clicked()),this,SLOT(OnBtnDashBoard()));
	connect(m_ManualControl->ui.btnForward,SIGNAL(clicked()),this,SLOT(On_MC_BtnForward()));
	connect(m_ManualControl->ui.btnBackward,SIGNAL(clicked()),this,SLOT(On_MC_BtnBackward()));
	connect(m_ManualControl->ui.btnTurnleft,SIGNAL(clicked()),this,SLOT(On_MC_BtnTurnleft()));
	connect(m_ManualControl->ui.btnTurnright,SIGNAL(clicked()),this,SLOT(On_MC_BtnTurnright()));
	connect(m_ManualControl->ui.btnStopmove,SIGNAL(clicked()),this,SLOT(On_MC_BtnStopmove()));
	connect(m_ManualControl->ui.btn_MC_GoHome,SIGNAL(clicked()),this,SLOT(On_MC_BtnGoHome()));
	connect(m_ManualControl->ui.btnExeSelfTask,SIGNAL(clicked()),this,SLOT(On_MC_BtnExeSelfTask()));
	connect(m_ManualControl->ui.btn_MC_Path1,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath1()));
	connect(m_ManualControl->ui.btn_MC_Path2,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath2()));
	connect(m_ManualControl->ui.btn_MC_Path3,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath3()));
	connect(m_ManualControl->ui.btn_MC_Path4,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath4()));
	connect(m_ManualControl->ui.btn_MC_FastGuide,SIGNAL(clicked()),this,SLOT(OnBtnFastGuideMode()));
	connect(m_ManualControl->ui.btnStartSpeak,SIGNAL(clicked()),this,SLOT(On_MC_BtnRobotQSpeak()));
	connect(m_ManualControl->ui.btnStopSpeak,SIGNAL(clicked()),m_RobotQ,SLOT(OnStopSpeak()));	
	connect(m_MuseumGUI->ui.btnAutoGuide,SIGNAL(clicked()),this,SLOT(OnBtnAutoGuide()));
	connect(m_MuseumGUI->ui.btnRobotQ,SIGNAL(clicked()),this,SLOT(OnBtnRobotQ()));
	connect(m_MuseumGUI->ui.btnHome,SIGNAL(clicked()),this,SLOT(On_MC_BtnGoHome()));
	connect(m_MuseumGUI->ui.btnPath1,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath1()));
	connect(m_MuseumGUI->ui.btnPath2,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath2()));
	connect(m_MuseumGUI->ui.btnPath3,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath3()));
	connect(m_MuseumGUI->ui.btnPath4,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath4()));
	//connect(m_RobotQ,SIGNAL(TTS_Ready()),this,SLOT(check_TTS_Ready()));//在子窗口的初始化函数中发射信号无法被接受，而在初始化函数之外发射有效
	//因为初始化函数为静态函数
	Init();
}
MainGUI::~MainGUI(){
	delete m_RobotQ;
	delete m_ManualControl;
	delete m_DashBoard;
	delete m_MuseumGUI;
	m_cURG.SwitchOff();		//关闭激光
}
void MainGUI::Init(){
	//InitStarMark();				//LED标签数组赋值(实验室)
	InitStarMarkMuseum();			//LED标签数组赋值(博物馆)
	InitDataBase();					//数据库初始化
	InitCommMotorAndStar();			//串口初始化Motor/Star
	InitCommLaser();				//串口初始化URG
	InitDashBoardData();			//仪表盘数据初始化
	InitTaskAssignment(1);			//任务字符串初始化(默认分配路线一)
	InitAdjustGUI();				//调整界面适配使用者
}
void MainGUI::closeEvent(QCloseEvent *event){
	RobotQ::OnStopSpeak();
	QTest::qSleep(100);
}
void MainGUI::InitDataBase(){
	if(m_dataManager.loadTask() == 1){			//数据库管理员对象读取任务文件
		m_DashBoard->ui.ck_isTaskDataReady->setChecked(true);
	}else{
		m_DashBoard->ui.ck_isTaskDataReady->setChecked(false);
	}
	if(m_dataManager.loadSpeakContent() ==1){	//数据库管理员对象读取语料文件
		m_DashBoard->ui.ck_isSpeakDataReady->setChecked(true);
	}else{
		m_DashBoard->ui.ck_isSpeakDataReady->setChecked(false);
	}
}
int MainGUI::OnBtnAutoGuide(){
	if(is_Auto_Mode_Open == true){
		is_Auto_Mode_Open = false;
		if(MUSEUMMODE == 0){
			m_DashBoard->ui.ck_Auto->setChecked(false);
			ui.btnAutoGuide->setText("开启自动导航");
		}else{
			const QIcon pic_AutoGuide_MainGUI = QIcon("Resources/自动导航.bmp");
			m_MuseumGUI->ui.btnAutoGuide->setIcon(pic_AutoGuide_MainGUI);
		}
		RobotQ::OnStopSpeak();
		QTest::qSleep(100);
		RobotQ::RobotQSpeak("自动导航已关闭！");
	}else{
		currentTodoListId = 0;
		taskID=todoList[currentTodoListId];						//从todoList中获取当前任务代码
		is_project_accomplished = false;
		is_Auto_Mode_Open = true;
		is_advertisement_available=true;
		if(MUSEUMMODE == 0){
			m_DashBoard->ui.ck_Auto->setChecked(true);
			ui.btnAutoGuide->setText("关闭自动导航");
		}else{
			const QIcon pic_AutoGuide_MainGUI = QIcon("Resources/自动导航_激活.bmp");
			m_MuseumGUI->ui.btnAutoGuide->setIcon(pic_AutoGuide_MainGUI);
		}
		RobotQ::OnStopSpeak();
		QTest::qSleep(100);
		RobotQ::RobotQSpeak("自动导航已开启！");
		SpeakWaitCycle = 3000/INSTRUCTION_CYCLE+1;
	}
	return 0;
}
int MainGUI::On_MC_BtnForward(){
	m_motor.VectorMove(1000,0);
	return 0;
}
int MainGUI::On_MC_BtnBackward(){
	m_motor.VectorMove(-800,0);
	return 0;
}
int MainGUI::On_Auto_BtnForward(int speedlevel){
	float inLV = 0.0f;
	float inPS = 0.0f;
	if(is_far_path_clear){
		inLV = 1000;
	}else{
		inLV = 800;
	}
	m_motor.CompromisedVectorMove(inLV,inPS);
	return 0;
}
int MainGUI::On_Auto_BtnTurnleft(int level){
	float factor = 0.0f;
	float speed = 1.0f;
	switch(level){
	case 0:factor = 1;break;
	case 1:factor = 2;break;
	case 2:factor = 0.5;break;
	default: factor = 1.2;
	}
	float inLV = 0.0f;
	float inPS = factor * speed;
	m_motor.CompromisedVectorMove(inLV,inPS);
	return 0;
}
int MainGUI::On_Auto_BtnTurnright(int level){
	float factor = 0.0f;
	float speed = 1.0f;
	switch(level){
	case 0:factor = 1;break;
	case 1:factor = 2;break;
	case 2:factor = 0.5;break;
	default: factor = 1.2;
	}
	float inPS = -1* factor * speed;
	float inLV = 0.0f;
	m_motor.CompromisedVectorMove(inLV,inPS);
	return 0;
}
int MainGUI::On_MC_BtnTurnleft(){
	m_motor.VectorMove(0,2);
	return 0;
}
int MainGUI::On_MC_BtnTurnright(){
	m_motor.VectorMove(0,-2);
	return 0;
}
int MainGUI::On_MC_BtnStopmove(){
	m_motor.stop();
	return 0;
}
int MainGUI::On_MC_BtnRobotQSpeak(){
	SpeakContent=m_ManualControl->ui.comSpeaklist->currentText();
	RobotQ::RobotQSpeak(SpeakContent);
	return 0;
}
void MainGUI::timerEvent(QTimerEvent *event){
	if(event->timerId()==m_timer_refresh_dashboard){
		refreshDashboardData();			//刷新仪表盘数据
	}else if(event->timerId()==m_timer_refresh_task){
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

	m_MARK[15].markID = 546;
	m_MARK[15].mark_angle = 0.00;
	m_MARK[15].mark_x = 283;
	m_MARK[15].mark_y = 888;

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
			if(m_laser_data_postpro[k]>0&&m_laser_data_postpro[k]<FAR_OBSTACLE_DISTANCE){
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
	CalculateSectorDistance();		//计算扇区内障碍物距离
	JudgeForwardSituation();		//判断前方是否畅通无阻
	refreshDashboardSector();		//刷新障碍物分布图
	if(is_Comm_URG_Open)m_DashBoard->ui.ck_URG->setChecked(true);	//判断电机是否开启
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	for (int loop_mark = 0; loop_mark < MARKNUM - 1; loop_mark++){
		if (m_MARK[loop_mark].markID == m_StarGazer.starID){
			//采集原始Star1数据
			PosByStar1.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX);
			PosByStar1.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY);
			AngleByStar1 = m_StarGazer.starAngel;
			//Star1未失灵情况下机器人本体的世界坐标与朝向由Star1确定
			if(m_StarGazer.starID !=34 && m_StarGazer.starID !=64 && m_StarGazer.starID !=66){
				AngleByStar1>0?AngleSafe=AngleByStar1:AngleSafe=AngleByStar1+360.0;		//得到机器人本体的朝向(顺时针)
				float dx = Distance_Robot_forward_StarGazer * zTool_cos_angle(AngleSafe);
				float dy = Distance_Robot_forward_StarGazer * zTool_sin_angle(AngleSafe);
				PosSafe = QPointF(PosByStar1.x()+dx,PosByStar1.y()-dy);
				AngleSafe = 360.0 - AngleSafe;											//得到机器人本体朝向（逆时针）
			}
		}
		if (m_MARK[loop_mark].markID == m_StarGazer.starID2){
			//采集原始Star2数据
			PosByStar2.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX2);
			PosByStar2.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY2);
			AngleByStar2 = m_StarGazer.starAngel;
			//Star1失灵情况下机器人本体的世界坐标与朝向由Star2确定
			if(m_StarGazer.starID ==34 || m_StarGazer.starID ==64 || m_StarGazer.starID ==66){
				AngleByStar2>0?AngleSafe=AngleByStar2:AngleSafe=AngleByStar2+360.0;		//得到机器人本体的朝向(顺时针)
				float dx = Distance_Robot_forward_StarGazer * zTool_cos_angle(AngleSafe);
				float dy = Distance_Robot_forward_StarGazer * zTool_sin_angle(AngleSafe);
				PosSafe = QPointF(PosByStar2.x()+dx,PosByStar2.y()-dy);
				AngleSafe = 360.0 - AngleSafe;
			}
		}
	}

	Angle_face_Goal = zTool_vector_angle(PosGoal - PosSafe);
	QString str;
	str.sprintf("(%.2f,%.2f)-(%.2f°)-(%d)",PosByStar1.x(),PosByStar1.y(),AngleByStar1,m_StarGazer.starID);
	m_DashBoard->ui.posStar1->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f°)-(%d)",PosByStar2.x(),PosByStar2.y(),AngleByStar2,m_StarGazer.starID2);
	m_DashBoard->ui.posStar2->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f°)",PosSafe.x(),PosSafe.y(),AngleSafe);
	m_DashBoard->ui.posSafe->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f°)",PosGoal.x(),PosGoal.y(),Angle_face_Goal);
	m_DashBoard->ui.posGoal->setText(str);

	//更新任务显示
	if(is_project_accomplished == false){
		str = "任务序号:";str += QString("%2").arg(currentTodoListId + 1);str +="(代码:";str += QString("%2").arg(todoList[currentTodoListId]);str += ")";
	}else{
		str = "无";
	}
	m_DashBoard->ui.text_CurrentTaskId->setText(str);

	int i=0;
	str = "";
	while(todoList[i] != 0){
		QString a =QString("%2").arg(todoList[i]);str += a;str += ",";i++;
	}
	str.resize(str.length() - 1);		//修剪字符串尾部多余的","
	m_DashBoard->ui.text_Todolist->setText(str);
}
void MainGUI::InitDashBoardData(){
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	PosSafe=QPointF(0.00,0.00);
	PosGoal=QPointF(00.00,00.00);
	AngleByStar1 = 0.0;
	AngleByStar2 = 0.0;
	AngleSafe = 0.0;
	Angle_face_Goal = 0.0;
	Angle_face_Audiance = 0.0;
	is_Auto_Mode_Open = false;
	is_FastGuideMode = false;
	is_path_clear = true;
	is_far_path_clear = true;
	is_dodge_moment = false;
	is_advertisement_available=true;
	dodge_move_times = 0;
	dodge_mode = 0;
	SpeakWaitCycle = 0;		//默认发出说话指令后，机器人本体不等待
	if(m_RobotQ->isAuthReady)m_DashBoard->ui.ck_Auth->setChecked(true);
	if(m_RobotQ->isASRReady)m_DashBoard->ui.ck_ASR->setChecked(true);
	if(m_RobotQ->isTTSReady)m_DashBoard->ui.ck_TTS->setChecked(true);
}
void MainGUI::AssignInstruction(){
	if(is_project_accomplished == false){			//当项目未完成时（一个项目由若干个任务组成，而一个任务想要完成需要经过若干个指令周期）
		if(SpeakWaitCycle>1){
			//如果还需等待的指令周期大于1，则等它讲话，除非开启了快速模式
			if(is_FastGuideMode)SpeakWaitCycle/=2;
		}else{
			if (JudgeTaskType(taskID) == PATHTASKTYPE){				//如果该任务是位移任务
				if(is_advertisement_available){
					is_advertisement_available=false;
					ShowAdvertisement();
				}
				AssignGoalPos(taskID);								//分配目标坐标和到达目标后的朝向
				float errorRange_Distance = ERRORDISTANCE;			//抵达目标点的距离误差范围，单位cm
				QPointF d = PosGoal - PosSafe;
				float dDistance = sqrt(pow(d.x(),2)+pow(d.y(),2));	//机器人中心到目标点的距离，单位cm
				if(dDistance > errorRange_Distance){				//如果还没有抵达当前任务目标点就继续执行任务
					if(is_dodge_moment == true){					//一旦前路不通会进入闪避时刻，占用若干个指令周期，而闪避时刻有自己的退出条件
						DodgeMeasures();							//如果处于闪避时刻就闪避
					}else{
						CommonMeasures();							//如果本指令周期没有处于闪避时刻，则正常向目标运行
					}
				}else{												//如果已经抵达当前目标点，还需要转向观众
					if(abs(Angle_face_Audiance-AngleSafe)>ERRORANGLE){		
						Rotate_to_GoalAngle(Angle_face_Audiance);
					}else{											//如果已经转向观众，则结束本条位移任务
						PathTaskFinishedMeasures();
					}		
				}
			}else if(JudgeTaskType(taskID) == SPEAKTASKTYPE){		//如果该任务是语音任务
				ShowPicByTaskID(taskID);
				AssignSpeakContent(taskID);		//将对应任务的语料赋值给SpeakContent，将对应语料的等待时间赋给SpeakWaitCycle
				RobotQ::RobotQSpeak(SpeakContent);
				SpeakWaitCycle = SpeakContent.length()/SPEAKWORDSPERSECOND*1000/INSTRUCTION_CYCLE+10;
				m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "正在朗读:" + SpeakContent);
				SpeakTaskFinishedMeasures();	//完成语音任务的善后操作
			}else{
				is_advertisement_available=false;
				ShowAdvertisement();
				ProjectFinishedMeasures();		//如果查询任务代码发现既不是语音任务也不是位移任务，则认为是项目结束符
			}
		}
		SpeakWaitCycle--;	//每过一个指令周期，接下来就少等一个周期，直到还需等待次数少于1就正常发配任务
	}	
}
float MainGUI::zTool_cos_angle(float angle){
	return cos(angle/360.0*2*PI);
}
float MainGUI::zTool_sin_angle(float angle){
	return sin(angle/360.0*2*PI);
}
void MainGUI::Rotate_to_GoalAngle(float AngleGoal){
	float dAngle = zTool_mod_360f(AngleGoal-AngleSafe);		//目标朝向与当前朝向的角度差，取值范围为(0,360)
	if(dAngle<180){
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
	//近距离障碍探测
	int n=0;
	for(int i=9;i<27;i++){
		if(sectorObstacleDistance[i]>0 && sectorObstacleDistance[i] < OBSTACLE_DISTANCE){
			n++;
		}
	}
	if(n>0){
		is_path_clear = false;
		is_dodge_moment = true;				//如果前路不通畅，进入闪避时刻
		dodge_move_times = 0;				//清空闪避操作次数计数器
		m_DashBoard->ui.ck_isClear->setChecked(false);
		m_DashBoard->ui.ck_isDodgetime->setChecked(true);
	}else{
		is_path_clear = true;
		m_DashBoard->ui.ck_isClear->setChecked(true);
	}
	//远距离障碍探测
	n=0;
	for(int i=9;i<27;i++){
		if(sectorObstacleDistance[i]>0 && sectorObstacleDistance[i] < FAR_OBSTACLE_DISTANCE){
			n++;
		}
	}
	if(n>0){
		is_far_path_clear = false;
		m_DashBoard->ui.ck_isFarClear->setChecked(false);
	}else{
		is_far_path_clear = true;
		m_DashBoard->ui.ck_isFarClear->setChecked(true);
	}
}
void MainGUI::DodgeTurnRight(){
	if(is_path_clear){
		On_Auto_BtnForward(1);		//在向右转到前方无障碍时前进一步
		dodge_move_times++;		//只有在躲避时刻中进行push操作才是有效操作，原地转圈没啥用
	}else{
		On_Auto_BtnTurnright(1);
	}
}
void MainGUI::DodgeTurnLeft(){
	if(is_path_clear){
		On_Auto_BtnForward(1);		//在向左转到前方无障碍时前进一步
		dodge_move_times++;		//只有在躲避时刻中进行push操作才是有效操作，原地转圈没啥用
	}else{
		On_Auto_BtnTurnleft(1);
	}
}
void MainGUI::InitAdjustGUI(){
	if(MUSEUMMODE == 1){
		OnBtnMuseumGUI();
	}else{	//进入开发者模式
		OnBtnDashBoard();				//开启仪表盘面板
		OnBtnManualControl();			//开启遥控器面板
		//OnBtnRobotQ();				//开启语音对话面板
		ShowAdvertisement();			//播放广告
	}
}
void MainGUI::InitTaskAssignment(int n){
	//初始化任务分配（路线X）
	currentTodoListId = 0;	//初始化当前todolist的下标为0
	QString str;
	if(n == 1){				//路线一
		//str = "60,61,60,61,62";	//这里是实验室三个路径点
		str="1,22,2,23,24,3,25,26,4,27,5,28,29,6,30,7,31,76";
	}else if(n == 2){
		str="8,32,9,33,34,10,35,11,36,37,38,39,76";
	}else if(n == 3){
		str="12,40,41,42,43,13,44,45,14,46,47,48,49,15,50,51,53,63,64,16,54,55,56,57,58,59,60,17,61,62,18,65,70,71,72,73,74,75,19,66,20,67,68,69,76";
	}else if(n == 4){
		str = "1,22,2,23,24,3,25,26,4,27,5,28,6,29,30,7,31,32,9,33,34,10,35,11,37,38,39,12,40,41,42,43,13,44,45,14,46,47,48,49,15,50,51,53,63,64,16,54,55,56,57,58,59,60,17,61,62,18,65,70,71,72,73,74,75,19,66,20,67,68,69,76";
	}
	ParseTodoList(str,todoList);
	if(is_FastGuideMode){
		FastGuideTodolist();	//如果开启了快速模式则修剪任务队列
	}
	taskID = todoList[currentTodoListId];
}
void MainGUI::AssignGoalPos(int taskID){		//根据任务代码（位移任务）分配目标位置及抵达目的地后的朝向
	TaskDataType* task = m_dataManager.findTask(taskID);
	if(task != NULL){
		PosGoal = QPointF(task->x,task->y);
		Angle_face_Audiance = task->FacingAngle;
	}
}
void MainGUI::AssignSpeakContent(int taskID){	//根据任务代码（语音任务）分配语音内容和机器人等待时间
	TaskDataType* task = m_dataManager.findTask(taskID);
	if(task != NULL){
		int SpeakContentId = task->SpeakContentId;
		SpeakContentType* speakContent = m_dataManager.findSpeakContent(SpeakContentId);
		SpeakContent = speakContent->content;
	}
}
float MainGUI::zTool_mod_360f(float angle){
	if(angle>360.0){
		angle -=360;
	}else if(angle<0.0){
		angle +=360;
	}
	return angle;
}
void MainGUI::DodgeMeasures(){
	if((is_path_clear == true && dodge_move_times > DODGESTEPS * 0.6) || dodge_move_times > DODGESTEPS){
		is_dodge_moment = false;				//超出闪避有效步数后，如果当前前路通畅，则退出闪避时刻
		dodge_mode = 0;							//闪避模式为0时可以接受新的闪避倾向策略
		m_DashBoard->ui.ck_isDodgetime->setChecked(false);
	}else{
		float d = zTool_mod_360f(Angle_face_Goal-AngleSafe);
		if(dodge_mode == 0){
			if(d<180.0){
				dodge_mode = 1;	//目标在当前朝向的左手边则向左避障
			}else{
				dodge_mode = 2;	//目标在当前朝向的右手边则向右避障
			}
		}
		if(dodge_mode == 1){
			DodgeTurnLeft();	
		}else{
			DodgeTurnRight();	
		}
	}
}
void MainGUI::CommonMeasures(){
	float errorRange_Angle = ERRORANGLE;		//选择角度的误差范围，单位°
	if(abs(Angle_face_Goal-AngleSafe)>errorRange_Angle){		
		Rotate_to_GoalAngle(Angle_face_Goal);	//如果和目标角度差距大就先看一眼目标
	}else{
		On_Auto_BtnForward(1);						//转到想要的角度后就继续走
	}
}
void MainGUI::PathTaskFinishedMeasures(){
	QString TaskCode;
	TaskCode.sprintf("任务代码%d",taskID);
	m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "完成位移任务," + TaskCode);
	currentTodoListId++;									//一个任务完成后，准备执行下一个
	taskID=todoList[currentTodoListId];						//从todoList中获取当前任务代码
}
void MainGUI::SpeakTaskFinishedMeasures(){
	QString TaskCode;
	TaskCode.sprintf("任务代码%d",taskID);
	m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "完成语音任务," + TaskCode);
	currentTodoListId++;									//一个任务完成后，准备执行下一个
	taskID=todoList[currentTodoListId];						//从todoList中获取当前任务代码
}
void MainGUI::ProjectFinishedMeasures(){
	is_project_accomplished = true;
	m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "我已经完成整个项目！");
	RobotQ::RobotQSpeak("我已经完成整个项目！");
	QTest::qSleep(4000);
	is_Auto_Mode_Open = false;
	m_DashBoard->ui.ck_Auto->setChecked(false);
	ui.btnAutoGuide->setText("开启自动导航");
	RobotQ::RobotQSpeak("自动导航已关闭！");
}
float MainGUI::zTool_vector_angle(QPointF d){
	float angle;
	if(d.x() > 0 && d.y() > 0){
		angle =atan(d.y()/d.x())/PI/2*360.0;	//目标在第一象限
	}else if(d.x() < 0 && d.y() < 0){
		angle =180 + atan(d.y()/d.x())/PI/2*360.0;	//目标在第三象限
	}else if(d.x() <0 && d.y() > 0){
		angle =180 + atan(d.y()/d.x())/PI/2*360.0;	//目标在第二象限
	}else{
		angle =360 + atan(d.y()/d.x())/PI/2*360.0;	//目标在第四象限
	}
	return angle;
}
int MainGUI::JudgeTaskType(int taskID){
	//判断任务类型，位移任务类型为1，语音任务类型为0
	TaskDataType* task = m_dataManager.findTask(taskID);
	if(task != NULL){
		return task->taskType;
	}else{
		return -1;	//返回异常值作为todoList的休止符
	}
}
void MainGUI::ParseTodoList(QString str,int *todoList){
	char* s;
	QByteArray ba = str.toLatin1();
	s = ba.data();
	int currentNum = 0;
	QString tempNum;
	for(int i = 0;s[i] != '\0';i++){
		char a = s[i];
		if(a != ','){	//没有读到','就继续读数字添加到tempNum中暂存
			tempNum.append(a);
		}else{			//如果读到','就将暂存的数字导出到todoList中
			todoList[currentNum] = tempNum.toInt();
			currentNum++;
			tempNum = "";
		}
	}
	todoList[currentNum] = tempNum.toInt();
	todoList[currentNum+1] = 0;
}
int MainGUI::OnBtnSelectPath1(){
	InitTaskAssignment(1);
	RobotQ::RobotQSpeak("已切换至路线一！");
	return 0;
}
int MainGUI::OnBtnSelectPath2(){
	InitTaskAssignment(2);
	RobotQ::RobotQSpeak("已切换至路线二！");
	return 0;
}
int MainGUI::OnBtnSelectPath3(){
	InitTaskAssignment(3);
	RobotQ::RobotQSpeak("已切换至路线三！");
	return 0;
}
int MainGUI::OnBtnSelectPath4(){
	InitTaskAssignment(4);
	RobotQ::RobotQSpeak("已切换至路线四！");
	return 0;
}
int MainGUI::On_MC_BtnGoHome(){
	currentTodoListId = 0;	//初始化当前todolist的下标为0
	QString str = "8";
	ParseTodoList(str,todoList);
	taskID = todoList[currentTodoListId];
	RobotQ::OnStopSpeak();
	RobotQ::RobotQSpeak("准备回家！");
	QTest::qSleep(3000);
	SpeakWaitCycle = 3000/INSTRUCTION_CYCLE + 1;
	OnBtnAutoGuide();
	return 0;
}
int MainGUI::On_MC_BtnExeSelfTask(){
	//任务字符串分配（路线X）
	currentTodoListId = 0;	//初始化当前todolist的下标为0
	QString str = m_ManualControl->ui.text_SelfTask->text();
	ParseTodoList(str,todoList);
	taskID = todoList[currentTodoListId];
	return 0;
}
void MainGUI::FastGuideTodolist(){
	//修剪任务队列，剔除所有语音点
	int i=0;
	int n=0;
	int fastList[99];
	int currentfastindex = 0;	//快速导览队列的数组下标
	while(todoList[i++]!=0)n++;
	for(i=0;i<n;i++){
		if(JudgeTaskType(todoList[i])==0){
			fastList[currentfastindex++] = todoList[i];
		}
	}
	fastList[currentfastindex] = 0;	//加上任务队列休止符
	//接下来把fastList赋值给todoList
	int j=0;
	for(i=0;i<currentfastindex;i++){
		todoList[j++] = fastList[i];
	}
	todoList[j] = 0;	//休止符
}
int MainGUI::OnBtnFastGuideMode(){
	if(is_FastGuideMode){
		is_FastGuideMode = !is_FastGuideMode;
		m_ManualControl->ui.btn_MC_FastGuide->setText("开启快速模式");
	}else{
		is_FastGuideMode = !is_FastGuideMode;
		m_ManualControl->ui.btn_MC_FastGuide->setText("关闭快速模式");
	}
	return 0;
}
void MainGUI::ShowPic(QString str){				//显示图片
	QDesktopWidget* desktop = QApplication::desktop();
	int N = desktop->screenCount();
	m_popup_secondScreen_image->setGeometry(desktop->screenGeometry(0));
	m_popup_secondScreen_image->ui.popup_image->setPixmap(QPixmap(str));
	m_popup_secondScreen_image->show();
	m_popup_secondScreen_image->resize(800,600);
	is_advertisement_available=true;
}
void MainGUI::ShowPicByTaskID(int taskID){		//显示图片（参数为任务代码）
	switch(taskID){
	//路线1
	case 23:ShowPic("Resources/图片/1、铁甲片.jpg");break;
	case 24:ShowPic("Resources/图片/2、铜马蹬.jpg");break;
	case 25:ShowPic("Resources/图片/3 、三足铜锅、三足铁锅（拼一张图）.jpg");break;
	case 26:ShowPic("Resources/图片/4、胡里改路之印.jpg");break;
	case 27:ShowPic("Resources/图片/5、兽面瓦当.jpg");break;
	case 28:ShowPic("Resources/图片/6、金代铁刀.jpg");break;
	case 29:ShowPic("Resources/图片/7、金代铁锏.jpg");break;
	case 30:ShowPic("Resources/图片/8 油画：户部达岗战役.jpg");break;
	case 31:ShowPic("Resources/图片/9.jpg");break;
	


	//路线2
	case 33:ShowPic("Resources/图片/10.jpg");break;
	case 34:ShowPic("Resources/图片/11、卷云龙纹长方砖.jpg");break;
	case 35:ShowPic("Resources/图片/12、银骨朵.jpg");break;
	case 37:
	case 36:ShowPic("Resources/图片/13、铁铧、犁镜、铁锹、铁镰（拼一张图）.jpg");break;
	//	case 36:ShowPic("Resources/图片/14、鱼形铁铡刀.jpg");break;
	case 38:ShowPic("Resources/图片/15、承安宝货.jpg");break;
	case 39:ShowPic("Resources/图片/16、翟家记真花银.jpg");break;
	//路线3
	case 42:ShowPic("Resources/图片/17、铜熨斗.jpg");break;
	case 43:ShowPic("Resources/图片/18、鎏金边荷花盏.jpg");break;
	case 45:ShowPic("Resources/图片/19、清酒肥羊四系瓶.jpg");break;
	case 46:ShowPic("Resources/图片/20、盘花金带銙.jpg");break;
	case 47:ShowPic("Resources/图片/21、鎏金铜带銙.jpg");break;
	case 48:ShowPic("Resources/图片/22、人物故事镜.jpg");break;
	case 49:ShowPic("Resources/图片/23、双鱼大铜镜.jpg");break;
	//	case 40:ShowPic("Resources/图片/24、奔马飞禽镜.jpg");break;
	case 50:ShowPic("Resources/图片/25、石雕飞天.jpg");break;
	case 51:ShowPic("Resources/图片/26、“将”字象棋.jpg");break;
	case 52:ShowPic("Resources/图片/27、三孔器、多孔器（拼一张图）.jpg");break;
	case 53:ShowPic("Resources/图片/28、手印砖、脚印砖（拼一张图）.jpg");break;
	case 55:ShowPic("Resources/图片/29.jpg");break;
	case 56:ShowPic("Resources/图片/30、金握.jpg");break;
	case 57:ShowPic("Resources/图片/31、海船菱花铜镜.jpg");break;
	case 58:ShowPic("Resources/图片/32、玉具剑.jpg");break;
	case 59:ShowPic("Resources/图片/33、银质铭牌.jpg");break;
	case 60:ShowPic("Resources/图片/34、黄地朵花金锦大带.jpg");break;
	case 61:ShowPic("Resources/图片/35、黄地散搭花金锦绵六合靴.jpg");break;
	case 62:ShowPic("Resources/图片/36、素娟兜跟.jpg");break;
	case 63:ShowPic("Resources/图片/37.jpg");break;
	case 64:ShowPic("Resources/图片/38.jpg");break;
	case 65:ShowPic("Resources/图片/39.jpg");break;	
	case 66:ShowPic("Resources/图片/40、铜坐龙.jpg");break;
	case 67:ShowPic("Resources/图片/41、铜项圈.jpg");break;
	case 68:ShowPic("Resources/图片/42、牙雕鱼饰件.jpg");break;
	case 69:ShowPic("Resources/图片/43.jpg");break;
	case 71:ShowPic("Resources/图片/44、环形金圈饰、桃形金圈饰、金花饰、金帽顶饰（拼一张图）.jpg");break;
	case 72:ShowPic("Resources/图片/45.jpg");break;
	case 73:ShowPic("Resources/图片/46、双鹿纹玉佩.jpg");break;
	case 74:ShowPic("Resources/图片/47、玉天鹅.jpg");break;
	case 75:ShowPic("Resources/图片/48、玉雕绶带鸟.jpg");break;
	default:ShowAdvertisement();
	}
}				
void MainGUI::ShowAdvertisement(){
	QDesktopWidget* desktop = QApplication::desktop();
	int N = desktop->screenCount();
	QMovie *movie = new QMovie("Resources/图片/七寸屏幕 .gif");
	m_popup_secondScreen_image->setGeometry(desktop->screenGeometry(0));
	m_popup_secondScreen_image->ui.popup_image->setMovie(movie);
	m_popup_secondScreen_image->show();
	m_popup_secondScreen_image->resize(800,600);
	movie->start();
}