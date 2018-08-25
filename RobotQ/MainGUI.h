#ifndef MAINGUI_H
#define MAINGUI_H

#include <QMainWindow>
#include <QtGui/QApplication>
#include <QCheckBox>
#include <QPointF>
#include "ui_MainGUI.h"
#include "robotq.h"
#include "ManualControl.h"
#include "DashBoard.h"
#include "DataManager.h"
#include "include/Comm_data_motor3.h"
#include "include/Comm_data_star.h"
#include "include/UPURG.h"

#define COMM_MOTOR 3				//�ײ�������ں�
#define COMM_STAR 4					//�Ǳ궨λ����
#define COMM_LASER 5				//���⴫�������ں�
#define PI 3.141592653
#define MUSEUMMODE 0				//ֵΪ1���������ʹ�ý��棬ֵΪ0���������߽���
#define MARKNUM	31					//ȫ���Ǳ�����
#define TODOLISTMAXNUM 99			//�����嵥��Ŀ������
#define DODGESTEPS 5				//����ʱ���������Ч����
#define EMERGENCY_TIMES 3			//�����ƶ�N�κ���ʱ����ƶ�
#define EMERGENCY_DISTANCE 300		//�����ƶ�Σ�վ��룬��λmm
#define EMERGENCY_RECOVER_CYCLE 6	//�����ƶ��������N��ָ�����ں�ָ�
#define INSTRUCTION_CYCLE 1500		//ָ�����ڣ���λms
#define INFOREFRESH_CYCLE 300		//����ˢ�����ڣ���λms
#define OBSTACLE_DISTANCE 400		//�ϰ���̽����룬��λmm
#define ERRORANGLE 12.0				//ѡ��Ƕȵ���Χ����λ��
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

class MainGUI : public QMainWindow{
	Q_OBJECT

public:
	MainGUI(QWidget *parent = 0);
	~MainGUI();
	QPushButton* btnAutoGuide_MUSEUM;
	QPushButton* btnPath1;
	QPushButton* btnPath2;
	QPushButton* btnPath3;
	QPointF PosByStar1;		//��ѡ�Ǳ�õ����Ǳ궨λ�����������꣬��λcm
	QPointF PosByStar2;		//��ѡ�Ǳ꣬��λcm
	QPointF PosSafe;		//�ۺϿ��ǵ�����Ǳ�����ó��ɿ��Ļ��������ĵ�����������λcm
	QPointF PosGoal;		//Ŀ�����꣬��λcm
	float AngleByStar1;		//��ѡ�Ǳ�õ����Ǳ궨λ���ĽǶȣ���λ��
	float AngleByStar2;		//��ѡ�Ǳ�õ����Ǳ궨λ���ĽǶȣ���λ��
	float AngleSafe;		//ʵ�ʵĻ����˳�����x��������Ϊ0�㣬��ʱ��Ϊ������ȡֵΪ[0~360)�Ͼ��Ǹ�����������ô������
	float Angle_face_Goal;	//վ�ڵ�ǰ�Ļ����˱����������곯Ŀ���������꿴�ĽǶȣ���λ��
	QString SpeakContent;	//�����˽�Ҫ˵������
	int SpeakWaitCycle;		//����˵��ָ��󣬻����������ɸ�ָ�������ڲ���������
private:
	Ui::MainGUI ui;
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
	bool is_path_clear;								//��ǰ��Ұ��ǰ���Ƿ�ͨ��
	bool is_dodge_moment;							//�Ƿ��������ʱ��
	bool isEMERGENCY;								//�Ƿ��ǽ����ƶ�ʱ��
	bool isBlockEMERGENCY;							//�Ƿ����ν����ƶ���Ϊ�˱�������������ƶ���Ͻ���ʣ��ڽ�������ʱ���ν����ƶ����ܣ���������������ʼʱ��������ƶ�����λ������ʼʱ�ָ������ƶ���
	int dodge_move_times;							//��������ʱ�̺����ܶ�����ִ�еĴ���
	int Emergency_times;							//���������ƶ�3�κ�ܾ������ƶ�ת������
	int m_EMERGENCY_DISTANCE;						//�����ƶ����룬��λmm
	int sectorObstacleDistance[36];					//ÿ��Ȼ���һ������
	int m_timer_refresh_dashboard;					//��������ѯ��������������ʾ���Ǳ�����
	int m_timer_refresh_task;						//��������ѯˢ�µ�ǰ����
	int m_timer_refresh_emergency_distance;			//������ˢ�½����ƶ�����
	int todoList[TODOLISTMAXNUM];					//�����嵥
	int taskID;										//��ǰ�������
	int currentTodoListId;							//��ǰ�������嵥�е��±�
	int JudgeTaskType(int taskID);					//������������жϵ�ǰ����������������λ������
	void Init();									//��ʼ��
	void InitAdjustGUI();							//��ʼ���������棨ѡ�����������ģʽ�򿪷���ģʽ��
	void InitStarMark();							//ΪLED��λ��ǩ���鸳ֵ
	void InitStarMarkMuseum();						//LED��ǩ���鸳ֵ(�����)
	void InitDataBase(int n);						//��ʼ�����ݿ�
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
	void EmergencyMeasures();						//��������ʱ�̺�ִ�еĽ�������
	void Rotate_to_GoalAngle(float AngleGoal);		//��ת��ָ���Ƕȣ�������λ��
	void JudgeEmergency();							//�жϵ�ǰָ�������Ƿ񴥷��˽����ƶ�ʱ��
	void JudgeForwardSituation();					//�ж�ǰ·�Ƿ�ͨ��
	void ProjectFinishedMeasures();					//��ɵ�ǰ��Ŀ��ִ�еĶ���
	void PathTaskFinishedMeasures();				//���λ�������ִ�еĶ���
	void ParseTodoList(QString str, int* todoList);	//����QString��ʽ�������嵥����ֵ��todoList����
	void SpeakTaskFinishedMeasures();				//������������ִ�еĶ���
	void refreshDashboardSector();					//ˢ���Ǳ����ϵ��ϰ��ֲ�ͼ
	void refreshDashboardData();					//ˢ���Ǳ����ϵ���ͨ����
	float zTool_cos_angle(float angle);				//��������ֵ��������λΪ��
	float zTool_sin_angle(float angle);				//��������ֵ��������λΪ��
	float zTool_mod_360f(float angle);				//���Ƕȷ�Χ��Ϊ(0,360)
	float zTool_vector_angle(QPointF d);			//��ʸ���ǣ���Χ(0,360)����λ�㣩��������λΪcm
	virtual void timerEvent(QTimerEvent *event);

private slots:
	int OnBtnRobotQ();
	int OnBtnManualControl();
	int OnBtnDashBoard();
	int OnBtnAutoGuide();
	int OnBtnSelectPath1();
	int OnBtnSelectPath2();
	int OnBtnSelectPath3();
	int On_MC_BtnForward();
	int On_MC_BtnBackward();
	int On_Auto_BtnTurnleft(int speedlevel);
	int On_Auto_BtnTurnright(int speedlevel);
	int On_MC_BtnTurnleft();
	int On_MC_BtnTurnright();
	int On_MC_BtnStopmove();
	int On_MC_BtnRobotQSpeak();
	int On_MC_BtnGoHome();
	int On_MC_BtnExeSelfTask();
};

#endif // MAINGUI_H
