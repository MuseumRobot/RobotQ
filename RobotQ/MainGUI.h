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
#include "include/Comm_data_motor3.h"
#include "include/Comm_data_star.h"
#include "include/UPURG.h"

#define COMM_MOTOR 3				//�ײ�������ں�
#define COMM_STAR 4					//�Ǳ궨λ����
#define COMM_LASER 5				//���⴫�������ں�
#define PI 3.141592653
#define MUSEUMMODE 0				//ֵΪ1���������ʹ�ý��棬ֵΪ0���������߽���
#define MARKNUM	31					//ȫ���Ǳ�����
#define DODGESTEPS 5				//����ʱ���������Ч����
#define EMERGENCY_TIMES 3			//�����ƶ�N�κ���ʱ����ƶ�
#define EMERGENCY_DISTANCE 300		//�����ƶ�Σ�վ��룬��λmm
#define EMERGENCY_RECOVER_CYCLE 6	//�����ƶ��������N��ָ�����ں�ָ�
#define INSTRUCTION_CYCLE 1500		//ָ�����ڣ���λms
#define INFOREFRESH_CYCLE 300		//����ˢ�����ڣ���λms
#define OBSTACLE_DISTANCE 400		//�ϰ���̽����룬��λmm
#define ERRORANGLE 12.0				//ѡ��Ƕȵ���Χ����λ��
#define ERRORDISTANCE 15.0			//�ִ�Ŀ���������뾶��Χ����λcm
#define Distance_Robot_forward_StarGazer 32.5		//���������ĵ����Ǳ궨λ�����ĵ�ǰ32.5cm��ʵ��ԭ����תһ���Դ���8cm����λ���޷��պϣ�

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
	QPointF PosByStar1;		//��ѡ�Ǳ�õ����Ǳ궨λ�����������꣬��λcm
	QPointF PosByStar2;		//��ѡ�Ǳ꣬��λcm
	QPointF PosByMotor;		//�����̼�ȷ�������꣬��λcm
	QPointF PosSafe;		//�ۺϿ��ǵ�����Ǳ�����ó��ɿ��Ļ��������ĵ�����������λcm
	QPointF PosGoal;		//Ŀ�����꣬��λcm
	float AngleByStar1;		//��ѡ�Ǳ�õ����Ǳ궨λ���ĽǶȣ���λ��
	float AngleByStar2;		//��ѡ�Ǳ�õ����Ǳ궨λ���ĽǶȣ���λ��
	float AngleSafe;		//ʵ�ʵĻ����˳�����x��������Ϊ0�㣬��ʱ��Ϊ������ȡֵΪ[0~360)�Ͼ��Ǹ�����������ô������
	float Angle_face_Goal;	//վ�ڵ�ǰ�Ļ����˱����������곯Ŀ���������꿴�ĽǶȣ���λ��
private:
	Ui::MainGUI ui;
	RobotQ* m_RobotQ;
	ManualControl* m_ManualControl;
	DashBoard* m_DashBoard;
	CMotor m_motor;
	Cstar m_StarGazer;
	CUPURG m_cURG;	
	bool is_Auto_Mode_Open;
	bool is_mission_accomplished;					//��ǰ�����Ƿ����
	bool is_project_accomplished;					//��ǰ��Ŀ�Ƿ����
	bool is_path_clear;								//��ǰ��Ұ��ǰ���Ƿ�ͨ��
	bool is_dodge_moment;							//�Ƿ��������ʱ��
	int dodge_move_times;							//��������ʱ�̺��Ҷ�ܶ����Ƿ���������
	int Emergency_times;							//���������ƶ�3�κ�ܾ������ƶ�ת������
	int m_EMERGENCY_DISTANCE;						//�����ƶ����룬��λmm
	int sectorObstacleDistance[36];					//ÿ��Ȼ���һ������
	int m_timer_refresh_dashboard;					//��������ѯ��������������ʾ���Ǳ�����
	int m_timer_refresh_task;						//��������ѯˢ�µ�ǰ����
	int m_timer_refresh_emergency_distance;			//������ˢ�½����ƶ�����
	int todoList[10];								//�����嵥
	int currentTodoListId;							//��ǰ�������嵥�е��±�
	StarMark m_MARK[100];							//LED��λ��ǩ���� - ÿ��߳�455
	void Init();									//��ʼ��
	void InitAdjustGUI();							//��ʼ���������棨ѡ�����������ģʽ�򿪷���ģʽ��
	void InitStarMark();							//ΪLED��λ��ǩ���鸳ֵ
	void InitStarMarkMuseum();						//LED��ǩ���鸳ֵ(�����)
	void InitTaskAssignment(int n);					//��ʼ����������·��
	void InitCommMotorAndStar();					//����Ǳ괮�ڳ�ʼ��
	void InitDashBoardData();						//�Ǳ������ݳ�ʼ��
	void CalculateSectorDistance();					//�����������ϰ���ľ���
	void JudgeForwardSituation();					//�ж�ǰ·�Ƿ�ͨ��
	void AssignInstruction();						//������һ��ָ��
	void AssignGoalPos(int taskID);					//����Ŀ��λ��
	void Rotate_to_GoalAngle(float AngleGoal);		//��ת��ָ���Ƕȣ�������λ��
	void DodgeTurnRight();							//����ʱ�̺���(��������׼��)
	void DodgeTurnLeft();							//����ʱ�̺���(��������׼��)
	void refreshDashboardSector();					//ˢ���Ǳ����ϵ��ϰ��ֲ�ͼ
	void refreshDashboardData();					//ˢ���Ǳ����ϵ���ͨ����
	float zTool_cos_angle(float angle);				//��������ֵ��������λΪ��
	float zTool_sin_angle(float angle);				//��������ֵ��������λΪ��
	virtual void timerEvent(QTimerEvent *event);

private slots:
	int OnBtnRobotQ();
	int OnBtnManualControl();
	int OnBtnDashBoard();
	int OnBtnAutoGuide();
	int On_MC_BtnForward();
	int On_MC_BtnBackward();
	int On_Auto_BtnTurnleft(int speedlevel);
	int On_Auto_BtnTurnright(int speedlevel);
	int On_MC_BtnTurnleft();
	int On_MC_BtnTurnright();
	int On_MC_BtnStopmove();
	int On_MC_BtnRobotQSpeak();
};

#endif // MAINGUI_H
