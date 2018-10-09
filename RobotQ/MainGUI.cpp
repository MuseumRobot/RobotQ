#include "MainGUI.h"

//���ɶ�����Ҫ��ʼ���մ�������������
CUPURG m_cURG;									//�������
CWinThread* pThread_Read_Laser;					//�����������߳�
UINT ThreadReadLaser_Data(LPVOID lpParam);		//���������ݺ���
bool is_Comm_URG_Open = false;					//��ʼ������δ����
bool key_laser = false;							//�������ݴ洢λ�ÿ���
void InitCommLaser();							//���⴮�ڳ�ʼ��
int m_laser_data_postpro[768];					//������Զ����ֵ(��λcm)
CEvent wait_data;
CEvent wait_laserpose;
threadInfo_laser_data Info_laser_data;

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);						//��ʼ����Щ��Ա������Ҫ��connectǰ
	m_ManualControl=new ManualControl(this);
	m_DashBoard=new DashBoard(this);
	m_popup_secondScreen_image = new PopupDialog(this);	//��ʼ���ڶ���Ļ��������
	m_timer_refresh_task=startTimer(INSTRUCTION_CYCLE);					//��������ѯ��������
	m_timer_refresh_dashboard=startTimer(INFOREFRESH_CYCLE);			//��������ѯ��ʾ����״̬
	m_counter_refresh_emergency_distance=0;								//�������������������ѯˢ�»ָ��ƶ����룬ģ20
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
	connect(m_ManualControl->ui.btn_MC_GoHome,SIGNAL(clicked()),this,SLOT(On_MC_BtnGoHome()));
	connect(m_ManualControl->ui.btnExeSelfTask,SIGNAL(clicked()),this,SLOT(On_MC_BtnExeSelfTask()));
	connect(m_ManualControl->ui.btn_MC_Path1,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath1()));
	connect(m_ManualControl->ui.btn_MC_Path2,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath2()));
	connect(m_ManualControl->ui.btn_MC_Path3,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath3()));
	connect(m_ManualControl->ui.btn_MC_Path4,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath4()));
	connect(m_ManualControl->ui.btn_MC_FastGuide,SIGNAL(clicked()),this,SLOT(OnBtnFastGuideMode()));
	connect(m_ManualControl->ui.btnStartSpeak,SIGNAL(clicked()),this,SLOT(On_MC_BtnRobotQSpeak()));
	connect(m_ManualControl->ui.btnStopSpeak,SIGNAL(clicked()),m_RobotQ,SLOT(OnStopSpeak()));	
	//connect(m_RobotQ,SIGNAL(TTS_Ready()),this,SLOT(check_TTS_Ready()));//���Ӵ��ڵĳ�ʼ�������з����ź��޷������ܣ����ڳ�ʼ������֮�ⷢ����Ч
	//��Ϊ��ʼ������Ϊ��̬����
	Init();
}
MainGUI::~MainGUI(){
	delete m_RobotQ;
	delete m_ManualControl;
	delete m_DashBoard;
	m_cURG.SwitchOff();		//�رռ���
}
void MainGUI::Init(){
	InitStarMark();					//LED��ǩ���鸳ֵ(ʵ����)
	//InitStarMarkMuseum();			//LED��ǩ���鸳ֵ(�����)
	InitDataBase(1);				//���ݿ��ʼ��
	InitTaskAssignment(1);			//Ĭ�Ϸ���·��һ(�����ַ�����ʼ��)
	InitCommMotorAndStar();			//���ڳ�ʼ��Motor/Star
	InitCommLaser();				//���ڳ�ʼ��URG
	InitDashBoardData();			//�Ǳ������ݳ�ʼ��
	InitAdjustGUI();				//������������ʹ����

}
void MainGUI::InitDataBase(int n){
	switch(n){
	case 1:
		if(m_dataManager.loadTask(1) == 1){			//���ݿ����Ա�����ȡ�����ļ�
			m_DashBoard->ui.ck_isTaskDataReady->setChecked(true);
		}else{
			m_DashBoard->ui.ck_isTaskDataReady->setChecked(false);
		}
		break;
	case 2:
		if(m_dataManager.loadTask(2) == 1){			//���ݿ����Ա�����ȡ�����ļ�
			m_DashBoard->ui.ck_isTaskDataReady->setChecked(true);
		}else{
			m_DashBoard->ui.ck_isTaskDataReady->setChecked(false);
		}
		break;
	default:
		if(m_dataManager.loadTask(3) == 1){			//���ݿ����Ա�����ȡ�����ļ�
			m_DashBoard->ui.ck_isTaskDataReady->setChecked(true);
		}else{
			m_DashBoard->ui.ck_isTaskDataReady->setChecked(false);
		}
	}
	if(m_dataManager.loadSpeakContent() ==1){	//���ݿ����Ա�����ȡ�����ļ�
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
			ui.btnAutoGuide->setText("�����Զ�����");
		}else{
			const QIcon pic_AutoGuide_MainGUI = QIcon("Resources/�Զ�����.bmp");
			btnAutoGuide_MUSEUM->setIcon(pic_AutoGuide_MainGUI);
		}
		RobotQ::OnStopSpeak();
		QTest::qSleep(1000);
		RobotQ::RobotQSpeak("�Զ������ѹرգ�");
	}else{
		currentTodoListId = 0;
		taskID=todoList[currentTodoListId];						//��todoList�л�ȡ��ǰ�������
		is_project_accomplished = false;
		is_Auto_Mode_Open = true;
		if(MUSEUMMODE == 0){
			m_DashBoard->ui.ck_Auto->setChecked(true);
			ui.btnAutoGuide->setText("�ر��Զ�����");
		}else{
			const QIcon pic_AutoGuide_MainGUI = QIcon("Resources/�Զ�����_����.bmp");
			btnAutoGuide_MUSEUM->setIcon(pic_AutoGuide_MainGUI);
		}
		RobotQ::OnStopSpeak();
		QTest::qSleep(200);
		RobotQ::RobotQSpeak("�Զ������ѿ�����");
		SpeakWaitCycle = 3000/INSTRUCTION_CYCLE+1;
	}
	return 0;
}
int MainGUI::OnBtnRobotQ(){
	//QDesktopWidget* desktop = QApplication::desktop();
	//int N = desktop->screenCount();
	//m_RobotQ->setGeometry(desktop->screenGeometry(1));
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
	m_motor.VectorMove(1600,0);
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
		inLV = 1200;
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
	case 1:factor = 4;break;
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
	case 1:factor = 4;break;
	case 2:factor = 0.5;break;
	default: factor = 1.2;
	}
	float inPS = -1* factor * speed;
	float inLV = 0.0f;
	m_motor.CompromisedVectorMove(inLV,inPS);
	return 0;
}
int MainGUI::On_MC_BtnTurnleft(){
	m_motor.VectorMove(0,4);
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
		refreshDashboardData();			//ˢ���Ǳ�������
	}else if(event->timerId()==m_timer_refresh_task){
		if(m_counter_refresh_emergency_distance == EMERGENCY_RECOVER_CYCLE){
			m_counter_refresh_emergency_distance = 0;
			if(m_EMERGENCY_DISTANCE == 0){
				m_EMERGENCY_DISTANCE = EMERGENCY_DISTANCE;	//�����ƶ��ָ�
				m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":�����ƶ��ѻָ�");
				isBlockEMERGENCY = false;
				m_DashBoard->ui.ck_isBlockEmergency->setChecked(false);
			}
		}
		if(is_Auto_Mode_Open){
			AssignInstruction();		//������һ��ָ��
		}
	}
}
UINT ThreadReadLaser_Data(LPVOID lpParam){
	bool laser_key = true;
	while (laser_key){
		m_cURG.GetDataByGD(0,768,1);//ǰ������������ɨ��Ƕȷ�Χ��384����ǰ�����ߣ�288��Ϊ90�ȷ�Χ�������һ�����������˽Ƕȷֱ��ʡ���ȡ�������������;
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
	for(int i=0;i<768;i++)	m_laser_data_postpro[i] = 10000;	//��ʼ�����ܼ��ⷵ�ص�����Ϊ10000mm
	if (m_cURG.Create(COMM_LASER)){
		m_cURG.SwitchOn();
		m_cURG.SCIP20();	
		m_cURG.GetDataByGD(0,768,1);
		pThread_Read_Laser=AfxBeginThread(ThreadReadLaser_Data,&Info_laser_data);		//������������߳�
		is_Comm_URG_Open = true;
	}	
}
void MainGUI::CalculateSectorDistance(){
	for(int i=0;i<36;i++)sectorObstacleDistance[i]=0;
	int sectorId = 0;
	while(sectorId < 36){	//��6��762��21*36�������н������
		int n = 0;			//���������ϰ����������
		int sum = 0.0;	//�ϰ��������ֵ
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
	int threshold = OBSTACLE_DISTANCE;	//�������߷��صĵ�λΪmm
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
	CalculateSectorDistance();		//�����������ϰ������
	JudgeForwardSituation();		//�ж�ǰ���Ƿ�ͨ����
	refreshDashboardSector();		//ˢ���ϰ���ֲ�ͼ
	if(is_Comm_URG_Open)m_DashBoard->ui.ck_URG->setChecked(true);	//�жϵ���Ƿ���
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	for (int loop_mark = 0; loop_mark < MARKNUM - 1; loop_mark++){
		if (m_MARK[loop_mark].markID == m_StarGazer.starID){
			//�ɼ�ԭʼStar1����
			PosByStar1.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX);
			PosByStar1.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY);
			AngleByStar1 = m_StarGazer.starAngel;
			//Star1δʧ������»����˱�������������볯����Star1ȷ��
			if(m_StarGazer.starID !=34 && m_StarGazer.starID !=64 && m_StarGazer.starID !=66){
				AngleByStar1>0?AngleSafe=AngleByStar1:AngleSafe=AngleByStar1+360.0;		//�õ������˱���ĳ���(˳ʱ��)
				float dx = Distance_Robot_forward_StarGazer * zTool_cos_angle(AngleSafe);
				float dy = Distance_Robot_forward_StarGazer * zTool_sin_angle(AngleSafe);
				PosSafe = QPointF(PosByStar1.x()+dx,PosByStar1.y()-dy);
				AngleSafe = 360.0 - AngleSafe;											//�õ������˱��峯����ʱ�룩
			}
		}
		if (m_MARK[loop_mark].markID == m_StarGazer.starID2){
			//�ɼ�ԭʼStar2����
			PosByStar2.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX2);
			PosByStar2.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY2);
			AngleByStar2 = m_StarGazer.starAngel;
			//Star1ʧ������»����˱�������������볯����Star2ȷ��
			if(m_StarGazer.starID ==34 || m_StarGazer.starID ==64 || m_StarGazer.starID ==66){
				AngleByStar2>0?AngleSafe=AngleByStar2:AngleSafe=AngleByStar2+360.0;		//�õ������˱���ĳ���(˳ʱ��)
				float dx = Distance_Robot_forward_StarGazer * zTool_cos_angle(AngleSafe);
				float dy = Distance_Robot_forward_StarGazer * zTool_sin_angle(AngleSafe);
				PosSafe = QPointF(PosByStar2.x()+dx,PosByStar2.y()-dy);
				AngleSafe = 360.0 - AngleSafe;
			}
		}
	}

	Angle_face_Goal = zTool_vector_angle(PosGoal - PosSafe);
	QString str;
	str.sprintf("(%.2f,%.2f)-(%.2f��)-(%d)",PosByStar1.x(),PosByStar1.y(),AngleByStar1,m_StarGazer.starID);
	m_DashBoard->ui.posStar1->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f��)-(%d)",PosByStar2.x(),PosByStar2.y(),AngleByStar2,m_StarGazer.starID2);
	m_DashBoard->ui.posStar2->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f��)",PosSafe.x(),PosSafe.y(),AngleSafe);
	m_DashBoard->ui.posSafe->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f��)",PosGoal.x(),PosGoal.y(),Angle_face_Goal);
	m_DashBoard->ui.posGoal->setText(str);

	//����������ʾ
	if(is_project_accomplished == false){
		str = "�������:";str += QString("%2").arg(currentTodoListId + 1);str +="(����:";str += QString("%2").arg(todoList[currentTodoListId]);str += ")";
	}else{
		str = "��";
	}
	m_DashBoard->ui.text_CurrentTaskId->setText(str);

	int i=0;
	str = "";
	while(todoList[i] != 0){
		QString a =QString("%2").arg(todoList[i]);str += a;str += ",";i++;
	}
	str.resize(str.length() - 1);		//�޼��ַ���β�������","
	m_DashBoard->ui.text_Todolist->setText(str);
}
void MainGUI::InitDashBoardData(){
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	PosSafe=QPointF(0.00,0.00);
	PosGoal=QPointF(00.00,00.00);
	is_Auto_Mode_Open = false;
	is_FastGuideMode = false;
	is_path_clear = true;
	is_far_path_clear = true;
	is_dodge_moment = false;
	isEMERGENCY = false;
	isBlockEMERGENCY = false;
	dodge_move_times = 0;
	dodge_mode = 0;
	m_EMERGENCY_DISTANCE = EMERGENCY_DISTANCE;
	Emergency_times = 0;
	SpeakWaitCycle = 0;		//Ĭ�Ϸ���˵��ָ��󣬻����˱��岻�ȴ�
}
void MainGUI::AssignInstruction(){
	JudgeEmergency();			//�жϵ�ǰָ�������Ƿ񴥷��˽����ƶ�ʱ��
	if(isEMERGENCY){			//��ǰָ������Ϊ����״̬
		EmergencyMeasures();	//��������µĴ�ʩ
	}else{						//�ǽ����������������ָ��
		if(is_project_accomplished == false){			//����Ŀδ���ʱ��һ����Ŀ�����ɸ�������ɣ���һ��������Ҫ�����Ҫ�������ɸ�ָ�����ڣ�
			if(SpeakWaitCycle>1){
				//�������ȴ���ָ�����ڴ���1�����������
			}else{
				if (JudgeTaskType(taskID) == PATHTASKTYPE){				//�����������λ������
					m_counter_refresh_emergency_distance++;				//��λ�������в��ۼӽ����ƶ��ָ�������
					AssignGoalPos(taskID);								//����Ŀ������
					float errorRange_Distance = ERRORDISTANCE;			//�ִ�Ŀ���ľ�����Χ����λcm
					QPointF d = PosGoal - PosSafe;
					float dDistance = sqrt(pow(d.x(),2)+pow(d.y(),2));	//���������ĵ�Ŀ���ľ��룬��λcm
					if(dDistance > errorRange_Distance){				//�����û�еִﵱǰ����Ŀ���ͼ���ִ������
						if(is_dodge_moment == true){					//һ��ǰ·��ͨ���������ʱ�̣�ռ�����ɸ�ָ�����ڣ�������ʱ�����Լ����˳�����
							DodgeMeasures();							//�����������ʱ�̾�����
						}else{
							CommonMeasures();							//�����ָ������û�д�������ʱ�̣���������Ŀ������
						}
					}else{												//����Ѿ��ִﵱǰĿ���
						PathTaskFinishedMeasures();
						JudgeTaskType(taskID) == SPEAKTASKTYPE ? isBlockEMERGENCY = true:isBlockEMERGENCY = false;	//���������ƶ�
					}
				}else if(JudgeTaskType(taskID) == SPEAKTASKTYPE){		//�������������������
					if(taskID == 123){		//�������Ҫ����ͼƬ��������
						QDesktopWidget* desktop = QApplication::desktop();
						int N = desktop->screenCount();
						m_popup_secondScreen_image->setGeometry(desktop->screenGeometry(1));
						m_popup_secondScreen_image->ui.popup_image->setPixmap(QPixmap("Resources/������ˮӡ���ά��.jpg"));
						m_popup_secondScreen_image->show();
						m_popup_secondScreen_image->resize(600,400);
					}
					AssignSpeakContent(taskID);		//����Ӧ��������ϸ�ֵ��SpeakContent������Ӧ���ϵĵȴ�ʱ�丳��SpeakWaitCycle
					RobotQ::RobotQSpeak(SpeakContent);
					SpeakWaitCycle = SpeakContent.length()/SPEAKWORDSPERSECOND*1000/INSTRUCTION_CYCLE+1;
					m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "�����ʶ�:" + SpeakContent);
					SpeakTaskFinishedMeasures();	//�������������ƺ����
					JudgeTaskType(taskID) == SPEAKTASKTYPE ? isBlockEMERGENCY = true:isBlockEMERGENCY = false;	//���������ƶ�
				}else{		
					ProjectFinishedMeasures();		//�����ѯ������뷢�ּȲ�����������Ҳ����λ����������Ϊ����Ŀ������
				}
			}
			SpeakWaitCycle--;	//ÿ��һ��ָ�����ڣ����������ٵ�һ�����ڣ�ֱ������ȴ���������1��������������
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
	float dAngle = zTool_mod_360f(AngleGoal-AngleSafe);		//Ŀ�곯���뵱ǰ����ĽǶȲȡֵ��ΧΪ(0,360)
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
	//�������ϰ�̽��
	int n=0;
	for(int i=9;i<27;i++){
		if(sectorObstacleDistance[i]>0 && sectorObstacleDistance[i] < OBSTACLE_DISTANCE){
			n++;
		}
	}
	if(n>0){
		is_path_clear = false;
		is_dodge_moment = true;				//���ǰ·��ͨ������������ʱ��
		dodge_move_times = 0;				//������ܲ�������������
		m_DashBoard->ui.ck_isClear->setChecked(false);
		m_DashBoard->ui.ck_isDodgetime->setChecked(true);
	}else{
		is_path_clear = true;
		m_DashBoard->ui.ck_isClear->setChecked(true);
	}
	//Զ�����ϰ�̽��
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
		On_Auto_BtnForward(1);		//������ת��ǰ�����ϰ�ʱǰ��һ��
		dodge_move_times++;		//ֻ���ڶ��ʱ���н���push����������Ч������ԭ��תȦûɶ��
	}else{
		On_Auto_BtnTurnright(1);
	}
}
void MainGUI::DodgeTurnLeft(){
	if(is_path_clear){
		On_Auto_BtnForward(1);		//������ת��ǰ�����ϰ�ʱǰ��һ��
		dodge_move_times++;		//ֻ���ڶ��ʱ���н���push����������Ч������ԭ��תȦûɶ��
	}else{
		On_Auto_BtnTurnleft(1);
	}
}
void MainGUI::InitAdjustGUI(){
	if(MUSEUMMODE == 1){
		delete ui.centralWidget;		//������п����߽���ؼ����±�д����
		QSize MainGUISize(640,480);		//������ߴ�
		QSize RobotQSize(540,380);		//������������ߴ�
		QSize btnRobotQSize(150,50);	//����������ť�ߴ�
		QSize btnPathSize(180,36);

		QLabel* MainBackGround = new QLabel(this);
		QPushButton* btnRobotQ = new QPushButton(this);
		btnAutoGuide_MUSEUM = new QPushButton(this);
		btnPath1 = new QPushButton(this);
		btnPath2 = new QPushButton(this);
		btnPath3 = new QPushButton(this);
		
		const QIcon pic_RobotQ_MainGUI = QIcon("Resources/��������.bmp");
		const QIcon pic_AutoGuide_MainGUI = QIcon("Resources/�Զ�����.bmp");
		const QIcon pic_Path1_MainGUI = QIcon("Resources/·��1.bmp");
		const QIcon pic_Path2_MainGUI = QIcon("Resources/·��2.bmp");
		const QIcon pic_Path3_MainGUI = QIcon("Resources/·��3.bmp");
		const QPixmap pic_BG_MainGUI = QPixmap("Resources/����ݱ���.bmp");

		resize(MainGUISize);
		MainBackGround->resize(MainGUISize);
		MainBackGround->setPixmap(pic_BG_MainGUI);
		MainBackGround->setScaledContents(true);

		btnRobotQ->move(60,50);
		btnRobotQ->resize(btnRobotQSize);
		btnRobotQ->setIconSize(btnRobotQSize);
		btnRobotQ->setIcon(pic_RobotQ_MainGUI);

		btnAutoGuide_MUSEUM->move(60,120);
		btnAutoGuide_MUSEUM->resize(btnRobotQSize);
		btnAutoGuide_MUSEUM->setIconSize(btnRobotQSize);
		btnAutoGuide_MUSEUM->setIcon(pic_AutoGuide_MainGUI);

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
		connect(btnAutoGuide_MUSEUM,SIGNAL(clicked()),this,SLOT(OnBtnAutoGuide()));
		connect(btnPath1,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath1()));
		connect(btnPath2,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath2()));
		connect(btnPath3,SIGNAL(clicked()),this,SLOT(OnBtnSelectPath3()));
		
	}else{	//���뿪����ģʽ
		OnBtnDashBoard();				//�����Ǳ������
		OnBtnManualControl();			//����ң�������
		//OnBtnRobotQ();				//���������Ի����
	}
}
void MainGUI::InitTaskAssignment(int n){
	//��ʼ��������䣨·��X��
	currentTodoListId = 0;	//��ʼ����ǰtodolist���±�Ϊ0
	QString str;
	if(n == 1){				//·��һ
		//str = "1,2,3,4,5,17,6,19,20,7,8,9,10,11,12,13,16,14,15";
		str = "60,61,62";	//������ʵ��������·���㣬������ڲ�������У��ǵø�����Ӧ��task1.data
	}else if(n == 2){
		str = "1,2,3,20,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19";
	}else if(n == 3){
		str = "1,21,2,22,23,24,3,25,26,27,4,28,5,29,30,6,54,7,31,32,8,33,34,35,36,9,37,38,39,40,41,42,10,43,44,45,46,11,12,13,47,14,48,15,49,16,50,17,51,52,18,53";
	}else if(n == 4){
		str = "1,21,2";		//ȫ·��
	}
	ParseTodoList(str,todoList);
	if(is_FastGuideMode){
		FastGuideTodolist();	//��������˿���ģʽ���޼��������
	}
	taskID = todoList[currentTodoListId];
}
void MainGUI::AssignGoalPos(int taskID){		//����������루λ�����񣩷���Ŀ��λ��
	TaskDataType* task = m_dataManager.findTask(taskID);
	if(task != NULL){
		PosGoal = QPointF(task->x,task->y);
	}
}
void MainGUI::AssignSpeakContent(int taskID){	//����������루�������񣩷����������ݺͻ����˵ȴ�ʱ��
	TaskDataType* task = m_dataManager.findTask(taskID);
	if(task != NULL){
		int SpeakContentId = task->SpeakContentId;
		SpeakContentType* speakContent = m_dataManager.findSpeakContent(SpeakContentId);
		SpeakContent = speakContent->content;
	}
}
void MainGUI::JudgeEmergency(){
	isEMERGENCY = false;
	for(int i=9;i<27;i++){
		if(sectorObstacleDistance[i] > 0 && sectorObstacleDistance[i] < m_EMERGENCY_DISTANCE){
			if(isBlockEMERGENCY == false){	//�����ƶ�δ������
				isEMERGENCY =true;
				break;
			}
		}
	}
	m_DashBoard->ui.ck_isEmergency->setChecked(isEMERGENCY);
}
void MainGUI::EmergencyMeasures(){
	Emergency_times++;
	m_motor.stop();
	RobotQ::RobotQSpeak("�����ƶ�!������С����!");
	SpeakWaitCycle = 5000/INSTRUCTION_CYCLE+1;
	if(Emergency_times == 3){
		Emergency_times = 0;
		m_EMERGENCY_DISTANCE = 0;
		RobotQ::RobotQSpeak("�����ƶ��ѽ����10���ָ�");
		m_counter_refresh_emergency_distance = 0;
		isBlockEMERGENCY = true;
		m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":�����ƶ��ѽ��");
		m_DashBoard->ui.ck_isBlockEmergency->setChecked(true);
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
		is_dodge_moment = false;				//����������Ч�����������ǰǰ·ͨ�������˳�����ʱ��
		dodge_mode = 0;							//����ģʽΪ0ʱ���Խ����µ������������
		m_DashBoard->ui.ck_isDodgetime->setChecked(false);
	}else{
		float d = zTool_mod_360f(Angle_face_Goal-AngleSafe);
		if(dodge_mode == 0){
			if(d<180.0){
				dodge_mode = 1;	//Ŀ���ڵ�ǰ��������ֱ����������
			}else{
				dodge_mode = 2;	//Ŀ���ڵ�ǰ��������ֱ������ұ���
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
	float errorRange_Angle = ERRORANGLE;		//ѡ��Ƕȵ���Χ����λ��
	if(abs(Angle_face_Goal-AngleSafe)>errorRange_Angle){		
		Rotate_to_GoalAngle(Angle_face_Goal);	//�����Ŀ��ǶȲ�����ȿ�һ��Ŀ��
	}else{
		On_Auto_BtnForward(1);						//ת����Ҫ�ĽǶȺ�ͼ�����
	}
}
void MainGUI::PathTaskFinishedMeasures(){
	QString TaskCode;
	TaskCode.sprintf("�������%d",taskID);
	m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "���λ������," + TaskCode);
	currentTodoListId++;									//һ��������ɺ�׼��ִ����һ��
	taskID=todoList[currentTodoListId];						//��todoList�л�ȡ��ǰ�������
}
void MainGUI::SpeakTaskFinishedMeasures(){
	QString TaskCode;
	TaskCode.sprintf("�������%d",taskID);
	m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "�����������," + TaskCode);
	currentTodoListId++;									//һ��������ɺ�׼��ִ����һ��
	taskID=todoList[currentTodoListId];						//��todoList�л�ȡ��ǰ�������
}
void MainGUI::ProjectFinishedMeasures(){
	is_project_accomplished = true;
	m_DashBoard->AppendMessage(m_DashBoard->m_time.toString("hh:mm:ss")+ ":" + "���Ѿ����������Ŀ��");
	RobotQ::RobotQSpeak("���Ѿ����������Ŀ��");
	QTest::qSleep(4000);
	is_Auto_Mode_Open = false;
	m_DashBoard->ui.ck_Auto->setChecked(false);
	ui.btnAutoGuide->setText("�����Զ�����");
	RobotQ::RobotQSpeak("�Զ������ѹرգ�");
}
float MainGUI::zTool_vector_angle(QPointF d){
	float angle;
	if(d.x() > 0 && d.y() > 0){
		angle =atan(d.y()/d.x())/PI/2*360.0;	//Ŀ���ڵ�һ����
	}else if(d.x() < 0 && d.y() < 0){
		angle =180 + atan(d.y()/d.x())/PI/2*360.0;	//Ŀ���ڵ�������
	}else if(d.x() <0 && d.y() > 0){
		angle =180 + atan(d.y()/d.x())/PI/2*360.0;	//Ŀ���ڵڶ�����
	}else{
		angle =360 + atan(d.y()/d.x())/PI/2*360.0;	//Ŀ���ڵ�������
	}
	return angle;
}
int MainGUI::JudgeTaskType(int taskID){
	//�ж��������ͣ�λ����������Ϊ1��������������Ϊ0
	TaskDataType* task = m_dataManager.findTask(taskID);
	if(task != NULL){
		return task->taskType;
	}else{
		return -1;	//�����쳣ֵ��ΪtodoList����ֹ��
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
		if(a != ','){	//û�ж���','�ͼ�����������ӵ�tempNum���ݴ�
			tempNum.append(a);
		}else{			//�������','�ͽ��ݴ�����ֵ�����todoList��
			todoList[currentNum] = tempNum.toInt();
			currentNum++;
			tempNum = "";
		}
	}
	todoList[currentNum] = tempNum.toInt();
	todoList[currentNum+1] = 0;
}
int MainGUI::OnBtnSelectPath1(){
	InitDataBase(1);
	InitTaskAssignment(1);
	RobotQ::RobotQSpeak("���л���·��һ��");
	return 0;
}
int MainGUI::OnBtnSelectPath2(){
	InitDataBase(2);
	InitTaskAssignment(2);
	RobotQ::RobotQSpeak("���л���·�߶���");
	return 0;
}
int MainGUI::OnBtnSelectPath3(){
	InitDataBase(3);
	InitTaskAssignment(3);
	RobotQ::RobotQSpeak("���л���·������");
	return 0;
}
int MainGUI::OnBtnSelectPath4(){
	InitDataBase(4);
	InitTaskAssignment(4);
	RobotQ::RobotQSpeak("���л���·���ģ�");
	return 0;
}
int MainGUI::On_MC_BtnGoHome(){
	currentTodoListId = 0;	//��ʼ����ǰtodolist���±�Ϊ0
	QString str = "0";
	ParseTodoList(str,todoList);
	taskID = todoList[currentTodoListId];
	RobotQ::OnStopSpeak();
	RobotQ::RobotQSpeak("׼���ؼң�");
	QTest::qSleep(3000);
	SpeakWaitCycle = 3000/INSTRUCTION_CYCLE + 1;
	OnBtnAutoGuide();
	return 0;
}
int MainGUI::On_MC_BtnExeSelfTask(){
	//�����ַ������䣨·��X��
	currentTodoListId = 0;	//��ʼ����ǰtodolist���±�Ϊ0
	QString str = m_ManualControl->ui.text_SelfTask->text();
	ParseTodoList(str,todoList);
	taskID = todoList[currentTodoListId];
	return 0;
}
void MainGUI::FastGuideTodolist(){
	//�޼�������У��޳�����������
	int i=0;
	int n=0;
	int fastList[99];
	int currentfastindex = 0;	//���ٵ������е������±�
	while(todoList[i++]!=0)n++;
	for(i=0;i<n;i++){
		if(JudgeTaskType(todoList[i])==0){
			fastList[currentfastindex++] = todoList[i];
		}
	}
	fastList[currentfastindex] = 0;	//�������������ֹ��
	//��������fastList��ֵ��todoList
	int j=0;
	for(i=0;i<currentfastindex;i++){
		todoList[j++] = fastList[i];
	}
	todoList[j] = 0;	//��ֹ��
}
int MainGUI::OnBtnFastGuideMode(){
	if(is_FastGuideMode){
		is_FastGuideMode = !is_FastGuideMode;
		m_ManualControl->ui.btn_MC_FastGuide->setText("��������ģʽ");
	}else{
		is_FastGuideMode = !is_FastGuideMode;
		m_ManualControl->ui.btn_MC_FastGuide->setText("�رտ���ģʽ");
	}
	return 0;
}