#include "MainGUI.h"

//���ɶ�����Ҫ��ʼ���մ�������������
CUPURG m_cURG;									//�������
CWinThread* pThread_Read_Laser;					//�����������߳�
UINT ThreadReadLaser_Data(LPVOID lpParam);		//���������ݺ���
bool is_Comm_URG_Open = false;					//��ʼ������δ����
bool key_laser = false;							//�������ݴ洢λ�ÿ���
void InitCommLaser();							//���⴮�ڳ�ʼ��
int m_laser_data_postpro[768];					//������Զ����ֵ(��λcm)
int m_laser_data_raw[768];
int m_Laser_Data_Point_PostPro;
CEvent wait_data;
CEvent wait_laserpose;
threadInfo_laser_data Info_laser_data;

MainGUI::MainGUI(QWidget *parent): QMainWindow(parent){
	ui.setupUi(this);
	m_RobotQ=new RobotQ(this);						//��ʼ����Щ��Ա������Ҫ��connectǰ
	m_ManualControl=new ManualControl(this);
	m_DashBoard=new DashBoard(this);
	m_timer_refresh_task=startTimer(1000);			//��������ѯ��������
	m_timer_refresh_dashboard=startTimer(600);		//��������ѯ��ʾ����״̬
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
	OnBtnDashBoard();				//�����Ǳ������
	OnBtnManualControl();			//����ң�������
	OnBtnRobotQ();					//���������Ի����
	
	InitStarMark();					//LED��ǩ���鸳ֵ
	InitCommMotorAndStar();			//���ڳ�ʼ��Motor/Star
	InitCommLaser();				//���ڳ�ʼ��URG
	InitDashBoardData();			//�Ǳ������ݳ�ʼ��


}
int MainGUI::OnBtnAutoGuide(){
	if(is_Auto_Mode_Open == true){
		is_Auto_Mode_Open = false;
		m_DashBoard->ui.ck_Auto->setChecked(false);
		ui.btnAutoGuide->setText("�����Զ�����");
		is_mission_accomplished = false;
	}else{
		is_Auto_Mode_Open = true;
		m_DashBoard->ui.ck_Auto->setChecked(true);
		ui.btnAutoGuide->setText("�ر��Զ�����");
	}
	return 0;
}
int MainGUI::OnBtnRobotQ(){
	m_RobotQ->move(800,450);
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
		CalculateSectorDistance();		//�����������ϰ������
		refreshDashboardSector();		//ˢ���ϰ���ֲ�ͼ
		refreshDashboardData();			//ˢ���Ǳ�������
	}else if(event->timerId()==m_timer_refresh_task){
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
		m_motor.VectorMove(800,0);	//���������Ư��ʾ��
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
	while(sectorId < 36){	//��6��762��21*36�������н������
		int n = 0;	//���������ϰ����������
		float sum = 0.0;	//�ϰ��������ֵ
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
	if(is_Comm_URG_Open)m_DashBoard->ui.ck_URG->setChecked(true);	//�жϵ���Ƿ���
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	for (int loop_mark = 0; loop_mark < 14; loop_mark++){
		if (m_MARK[loop_mark].markID == m_StarGazer.starID){
			PosByStar1.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX);
			PosByStar1.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY);
			AngleByStar1 = m_StarGazer.starAngel;
			AngleByStar1>0?AngleSafe=AngleByStar1:AngleSafe=AngleByStar1+360.0;		//�õ������˱���ĳ���(˳ʱ��)
			AngleSafe = 360.0 - AngleSafe;		//�õ������˱��峯����ʱ�룩
			float dx = Distance_Robot_forward_StarGazer * zTool_cos_angle(AngleSafe);
			float dy = Distance_Robot_forward_StarGazer * zTool_sin_angle(AngleSafe);
			PosSafe = QPointF(PosByStar1.x()+dx,PosByStar1.y()+dy);
		}
		if (m_MARK[loop_mark].markID == m_StarGazer.starID2){
			PosByStar2.setX(m_MARK[loop_mark].mark_x + m_StarGazer.starX2);
			PosByStar2.setY(m_MARK[loop_mark].mark_y + m_StarGazer.starY2);
			AngleByStar2 = m_StarGazer.starAngel;
		}
	}
	Angle_face_Goal = 180.0 + atan((PosGoal.y()-PosSafe.y())/(PosGoal.x()-PosSafe.x()))/PI/2*360.0;
	QString str;
	str.sprintf("(%.2f,%.2f)-(%.2f��)-(%d)",PosByStar1.x(),PosByStar1.y(),AngleByStar1,m_StarGazer.starID);
	m_DashBoard->ui.posStar1->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f��)-(%d)",PosByStar2.x(),PosByStar2.y(),AngleByStar2,m_StarGazer.starID2);
	m_DashBoard->ui.posStar2->setText(str);
	str.sprintf("(%.2f,%.2f)",PosByMotor.x(),PosByMotor.y());
	m_DashBoard->ui.posMotor->setText(str);
	str.sprintf("(%.2f,%.2f)-(%.2f��)",PosSafe.x(),PosSafe.y(),AngleSafe);
	m_DashBoard->ui.posSafe->setText(str);
	str.sprintf("(%.2f,%.2f)",PosGoal.x(),PosGoal.y());
	m_DashBoard->ui.posGoal->setText(str);
}
void MainGUI::InitDashBoardData(){
	PosByStar1=QPointF(0.00,0.00);
	PosByStar2=QPointF(0.00,0.00);
	PosByMotor=QPointF(0.00,0.00);
	PosSafe=QPointF(0.00,0.00);
	PosGoal=QPointF(0.00,0.00);
	is_Auto_Mode_Open = false;
	is_mission_accomplished = false;
}
void MainGUI::AssignInstruction(){
	if(is_mission_accomplished == false){
		float errorRange_Angle = 10.0;		//ѡ��Ƕȵ���Χ����λ��
		float errorRange_Distance = 30.0;	//�ִ�Ŀ���ľ�����Χ����λcm
		QPointF d = PosGoal - PosSafe;
		float dDistance = sqrt(pow(d.x(),2)+pow(d.y(),2));	//����Ŀ���ľ��룬��λcm
		if(dDistance > errorRange_Distance){
			if(abs(Angle_face_Goal-AngleSafe)>errorRange_Angle){		
				Rotate_to_GoalAngle(Angle_face_Goal);
			}else{
				On_MC_BtnForward();
			}
		}else{
			is_mission_accomplished = true;
			m_DashBoard->AppendMessage("���Ѿ�����Ŀ���");
			RobotQ::RobotQSpeak("���Ѿ�����Ŀ���");
		}
	}
	//int randomTask=rand()%6;
	//switch (randomTask){
	//case 0:m_motor.VectorMove(800,0);break;
	//case 1:m_motor.VectorMove(-800,0);break;
	//case 2:m_motor.VectorMove(0,100);break;
	//case 3:m_motor.VectorMove(0,-100);break;
	//case 4:m_motor.stop();
	//default:RobotQ::RobotQSpeak("�ǺǺ�");
	//}
}
float MainGUI::zTool_cos_angle(float angle){
	return cos(angle/360.0*2*PI);
}
float MainGUI::zTool_sin_angle(float angle){
	return sin(angle/360.0*2*PI);
}
void MainGUI::Rotate_to_GoalAngle(float AngleGoal){
	float dAngle=AngleGoal-AngleSafe;	//Ŀ�곯���뵱ǰ����ĽǶȲ�
	if(dAngle>0){
		On_MC_BtnTurnleft();
	}else{
		On_MC_BtnTurnright();
	}
}