#ifndef MAINGUI_H
#define MAINGUI_H

#include "stdafx.h"
#include "ui_MainGUI.h"
#include "MuseumGUI.h"
#include "robotq.h"
#include "ManualControl.h"
#include "DashBoard.h"
#include "DataManager.h"

#define COMM_MOTOR 3				//�ײ�������ں�
#define COMM_STAR 4					//�Ǳ궨λ����
#define COMM_LASER 5				//���⴫�������ں�
#define PI 3.141592653
#define MUSEUMMODE 1				//ֵΪ1���������ʹ�ý��棬ֵΪ0���������߽��棬ֵΪ2��������������Ļ����ʵ�����߽���
#define MARKNUM	31					//ȫ���Ǳ�����
#define TODOLISTMAXNUM 99			//�����嵥��Ŀ������
#define DODGESTEPS 50				//����ʱ���������Ч����
#define INSTRUCTION_CYCLE 200		//ָ�����ڣ���λms
#define INFOREFRESH_CYCLE 100		//����ˢ�����ڣ���λms
#define OBSTACLE_DISTANCE 400		//�����ϰ���̽����룬��λmm�������ж�ǰ����·�Ƿ�ͨ��
#define FAR_OBSTACLE_DISTANCE 800	//Զ���ϰ���̽����룬��λmm�������ж�ǰ����Զ���Ƿ�ͨ�����Ӷ�����ǰ���ٶ�
#define ERRORANGLE 10.0				//ѡ��Ƕȵ���Χ����λ��
#define ERRORDISTANCE 15.0			//�ִ�Ŀ���������뾶��Χ����λcm
#define SPEAKWORDSPERSECOND 4		//����ÿ�����Ķ�������
#define Distance_Robot_forward_StarGazer 32.5		//���������ĵ����Ǳ궨λ�����ĵ�ǰ32.5cm��ʵ��ԭ����תһ���Դ���8cm����λ���޷��պϣ�
#define PATHTASKTYPE 0				//λ�����������
#define SPEAKTASKTYPE 1				//�������������

typedef struct StarMark{
	int markID;
	float mark_angle;
	float mark_x;
	float mark_y;
}StarMark;

struct threadInfo_laser_data{
	double 	m_Laser_Data_Value[768];
	int	m_Laser_Data_Point;
};

class MainGUI : public QDialog{
	Q_OBJECT

public:
	MainGUI(QWidget *parent = 0);
	~MainGUI();
	PopupDialog* m_popup_secondScreen_image;	//�ڶ���ʾ����������
	QPointF PosByStar1;		//��ѡ�Ǳ�õ����Ǳ궨λ�����������꣬��λcm
	QPointF PosByStar2;		//��ѡ�Ǳ꣬��λcm
	QPointF PosSafe;		//�ۺϿ��ǵ�����Ǳ�����ó��ɿ��Ļ��������ĵ�����������λcm
	QPointF PosGoal;		//Ŀ�����꣬��λcm
	float AngleByStar1;		//��ѡ�Ǳ�õ����Ǳ궨λ���ĽǶȣ���λ��
	float AngleByStar2;		//��ѡ�Ǳ�õ����Ǳ궨λ���ĽǶȣ���λ��
	float AngleSafe;		//ʵ�ʵĻ����˳�����x��������Ϊ0�㣬��ʱ��Ϊ������ȡֵΪ[0~360)�Ͼ��Ǹ�����������ô������
	float Angle_face_Goal;	//վ�ڵ�ǰ�Ļ����˱����������곯Ŀ���������꿴�ĽǶȣ���λ��
	float Angle_face_Audiance;	//�ִ�Ŀ�ĵغ�����ڵĽǶ�
	QString SpeakContent;	//�����˽�Ҫ˵������
	int SpeakWaitCycle;		//����˵��ָ��󣬻����������ɸ�ָ�������ڲ���������
private:
	Ui::MainGUI ui;
	MuseumGUI* m_MuseumGUI;
	RobotQ* m_RobotQ;
	ManualControl* m_ManualControl;
	DashBoard* m_DashBoard;
	CMotor m_motor;
	Cstar m_StarGazer;
	CUPURG m_cURG;	
	DataManager m_dataManager;
	StarMark m_MARK[MARKNUM];						//LED��λ��ǩ���� - ÿ��߳�455
	bool is_Auto_Mode_Open;
	bool is_project_accomplished;					//��ǰ��Ŀ�Ƿ����
	bool is_FastGuideMode;							//��ǰ�Ƿ��ǿ��ٵ���ģʽ��������ģʽ��
	bool is_path_clear;								//��ǰ��Ұ��ǰ���Ƿ�ͨ��
	bool is_far_path_clear;							//�жϵ�ǰ��Ұ��Զ���Ƿ�ͨ��
	bool is_dodge_moment;							//�Ƿ��������ʱ��
	bool is_advertisement_available;				//�Ƿ����ʾ���
	int dodge_mode;									//Ϊ��ʹÿ�ν�������ʱ��ֻ����һ���������ܣ���Ҫ�ڽ���ʱ�������ܷ���1Ϊ��2Ϊ�ң��˳�����ʱΪ0���ɹ������µ�ֵ
	int dodge_move_times;							//��������ʱ�̺����ܶ�����ִ�еĴ���
	int sectorObstacleDistance[36];					//ÿ��Ȼ���һ������
	int m_timer_refresh_dashboard;					//��������ѯ��������������ʾ���Ǳ�����
	int m_timer_refresh_task;						//��������ѯˢ�µ�ǰ����
	float m_Last_inLV;								//��һ�����������ٶ�
	float m_Last_inPS;								//��һ�������Ľ��ٶ�
	int todoList[TODOLISTMAXNUM];					//�����嵥
	int taskID;										//��ǰ�������
	int currentTodoListId;							//��ǰ�������嵥�е��±�
	int JudgeTaskType(int taskID);					//������������жϵ�ǰ����������������λ������
	void Init();									//��ʼ��
	void InitAdjustGUI();							//��ʼ���������棨ѡ�����������ģʽ�򿪷���ģʽ��
	void InitStarMark();							//ΪLED��λ��ǩ���鸳ֵ
	void InitStarMarkMuseum();						//LED��ǩ���鸳ֵ(�����)
	void InitDataBase();							//��ʼ�����ݿ�
	void InitTaskAssignment(int n);					//��ʼ����������·��
	void InitCommMotorAndStar();					//����Ǳ괮�ڳ�ʼ��
	void InitDashBoardData();						//�Ǳ������ݳ�ʼ��
	void AssignInstruction();						//������һ��ָ��
	void AssignGoalPos(int taskID);					//����������루λ�����񣩷���Ŀ��λ��
	void AssignSpeakContent(int taskID);			//����������루�������񣩷����������ݺͻ����˵ȴ�ʱ��
	void CalculateSectorDistance();					//�����������ϰ���ľ���
	void CommonMeasures();							//û�����ⷢ��������ִ�г��涯��
	void DodgeMeasures();							//��������ʱ�̺�Ķ������������������������������ѡ����ʵ�ִ�У�
	void DodgeTurnRight();							//����ʱ�̻�������(��������)
	void DodgeTurnLeft();							//����ʱ�̻�������(��������)
	void FastGuideTodolist();						//�޼�������У��޳�����������
	void JudgeForwardSituation();					//�ж�ǰ·�Ƿ�ͨ��
	void ProjectFinishedMeasures();					//��ɵ�ǰ��Ŀ��ִ�еĶ���
	void PathTaskFinishedMeasures();				//���λ�������ִ�еĶ���
	void ParseTodoList(QString str, int* todoList);	//����QString��ʽ�������嵥����ֵ��todoList����
	void Rotate_to_GoalAngle(float AngleGoal);		//��ת��ָ���Ƕȣ�������λ��
	void SpeakTaskFinishedMeasures();				//������������ִ�еĶ���
	void ShowPic(QString str);						//��ʾͼƬ������ΪͼƬ·����
	void ShowPicByTaskID(int taskID);				//��ʾͼƬ������Ϊ������룩
	void ShowAdvertisement();						//���Ź��
	void refreshDashboardSector();					//ˢ���Ǳ����ϵ��ϰ��ֲ�ͼ
	void refreshDashboardData();					//ˢ���Ǳ����ϵ���ͨ����
	float zTool_cos_angle(float angle);				//��������ֵ��������λΪ��
	float zTool_sin_angle(float angle);				//��������ֵ��������λΪ��
	float zTool_mod_360f(float angle);				//���Ƕȷ�Χ��Ϊ(0,360)
	float zTool_vector_angle(QPointF d);			//��ʸ���ǣ���Χ(0,360)����λ�㣩��������λΪcm
	virtual void timerEvent(QTimerEvent *event);
	void closeEvent(QCloseEvent *event);			//�ر��������Ķ���

private slots:
	inline int OnBtnRobotQ(){m_RobotQ->show();return 0;}
	inline int OnBtnManualControl(){m_ManualControl->move(300,20);m_ManualControl->show();return 0;}
	inline int OnBtnMuseumGUI(){m_MuseumGUI->move(80,50);m_MuseumGUI->show();return 0;}
	inline int OnBtnDashBoard(){m_DashBoard->move(750,20);m_DashBoard->show();return 0;}
	int OnBtnAutoGuide();
	int OnBtnSelectPath1();
	int OnBtnSelectPath2();
	int OnBtnSelectPath3();
	int OnBtnSelectPath4();
	int OnBtnFastGuideMode();
	int On_MC_BtnForward();
	int On_MC_BtnBackward();
	int On_Auto_BtnTurnleft(int speedlevel);
	int On_Auto_BtnTurnright(int speedlevel);
	int On_Auto_BtnForward(int speedlevel);
	int On_MC_BtnTurnleft();
	int On_MC_BtnTurnright();
	int On_MC_BtnStopmove();
	int On_MC_BtnRobotQSpeak();
	int On_MC_BtnGoHome();
	int On_MC_BtnExit();
	int On_MC_BtnExeSelfTask();
};

#endif // MAINGUI_H
