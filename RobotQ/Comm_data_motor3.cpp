#include "stdafx.h"
#include "include/Comm_data_motor3.h"

#define COS30f  0.86602540
#define SIN30f  0.5
//float R  =    220; // robot radius: 18cm
#define PIf 3.1415926
//float Wheel_radius = 50.00; ////mm

CMotor::CMotor(void){
	m_baudrate=115200;
	motor_str[0]=':';
	motor_str[1]='L';
	motor_str[2]='F';
	motor_str[3]='0';
	motor_str[4]='0';
	motor_str[5]='0';
	motor_str[6]='R';
	motor_str[7]='F';
	motor_str[8]='0';
	motor_str[9]='0';
	motor_str[10]='0';
	motor_str[11]='Z';
	motor_str[12]='F';
	motor_str[13]='0';
	motor_str[14]='0';
	motor_str[15]='0';
	motor_str[16]='G';
	motor_str[17]='.';

	Go_status = 0;
	Go_status_old = 0;
	m_lastChar = 0;
	m_ParHeader = '*'; 
	m_ParEnd = '#';
	Time_start='T';
	ldata_star = 'L';
	rdata_star = 'R';
	zdata_star = 'Z';

	m_databuf = new char[100]; //���ݴ洢
	m_nRecvindex = 0;       //����д��λ��m_recvbuf
	m_nFrameLen = 0;        //�洢���ݰ��ĳ���
	m_bFrameStart = false;  //���ݰ����տ�ʼ��־λ��TRUEΪ�ѿ�ʼ���գ�FLASEΪû�п�ʼ
	m_right=false;

	m_Last_inLV = 0.0f;		//Ĭ�����ٶ�Ϊ0
	m_Last_inPS = 0.0f;		//Ĭ�Ͻ��ٶ�Ϊ0
}


CMotor::~CMotor(void){
	delete m_databuf;
}

void CMotor::Parse(BYTE inData){
	if (m_bFrameStart == false){
		//��û�п�ʼ�������ݣ�����Ҫ����Ƿ������ݰ�ͷ���֣����ݰ����տ�ʼ��־λ��TRUEΪ�ѿ�ʼ���գ�FLASEΪû�п�ʼ
		//�ж��Ƿ�Ϊ��ͷ
		if (inData == Time_start && m_lastChar == m_ParHeader){
			//���ݰ�ͷ������ʼ��������
			m_bFrameStart = true;
			m_recvbuf[0]='*';
			m_recvbuf[1]='T';
			//д���ͷ���Ѱ�ͷд�뵽���ݰ��洢
			m_nRecvindex = 1;//����д��λ
		}
	}
	else{
		//���Ѿ���ʼ��������
		m_nRecvindex++;//����д��λ��1
		m_recvbuf[m_nRecvindex] = inData;
		//Ѱ�Ұ�β
		if (m_nRecvindex%25 == 0){
			//�����ݺ���һ�����յ����ַ���Ϊ���ݰ�������ʽ
			if (inData == m_ParEnd){
				//���ݳ����Ƿ���ȷ ��19���ֽ�����
				m_nFrameLen = m_nRecvindex + 1;//�洢���ݰ��ĳ��ȵ�������д��λ��1
				m_ParseFrame();//�Խ��յ����ݰ����н���
				encoder_l = lift_num;
				encoder_r = right_num;
				encoder_z = zhong_num;
				timer = motor_timer;
				memset(m_recvbuf,0x00,sizeof(m_recvbuf));
			}
				
			m_bFrameStart = false;
			m_nRecvindex = 0;
			m_nFrameLen = 0;
				
		}
			
	}
	m_lastChar = inData;//����һ�����յ����ַ����ĳ�inData
}

void CMotor::m_ParseFrame(void){
	//�������ݽ���
	for (int i = 0;i<m_nFrameLen-1;i++){
		if (m_recvbuf[i]==Time_start&&m_recvbuf[i+5]==';'){
		    motor_timer=m_recvbuf[i+1];
			motor_timer=(motor_timer<<8)+m_recvbuf[i+2];
			motor_timer=(motor_timer<<8)+m_recvbuf[i+3];
			motor_timer=(motor_timer<<8)+m_recvbuf[i+4];
		}
		if (m_recvbuf[i]==ldata_star&&m_recvbuf[i+5]==';'){
			lift_num=m_recvbuf[i+1]; 
			lift_num=(lift_num<<8)+m_recvbuf[i+2];
			lift_num=(lift_num<<8)+m_recvbuf[i+3];
			lift_num=(lift_num<<8)+m_recvbuf[i+4];
		}
		if (m_recvbuf[i]==rdata_star&&m_recvbuf[i+5]==';'){
			right_num=m_recvbuf[i+1];
			right_num=(right_num<<8)+m_recvbuf[i+2];
			right_num=(right_num<<8)+m_recvbuf[i+3];
			right_num=(right_num<<8)+m_recvbuf[i+4];
		}
		if (m_recvbuf[i]==zdata_star&&m_recvbuf[i+5]==';'){
			zhong_num=m_recvbuf[i+1];
			zhong_num=(zhong_num<<8)+m_recvbuf[i+2];
			zhong_num=(zhong_num<<8)+m_recvbuf[i+3];
			zhong_num=(zhong_num<<8)+m_recvbuf[i+4];
		}
		if (m_recvbuf[i]==0x0d&&m_recvbuf[i+1]==0x0a){
			return;
		}
	}
}

//�������˶�������ֱ��ͨ�������������ٿ��ƻ������˶�
bool CMotor::gomotor(int Lspeed, int Rspeed,int Zspeed){
	//Lspeed +Ϊ��ת-Ϊ��ת ����ֵΪ�ٶ� ��λcm/s
	motor_str[0]=':';
	motor_str[1]='L';
	motor_str[2]='F';
	motor_str[3]='0';
	motor_str[4]='0';
	motor_str[5]='0';
	motor_str[6]='R';
	motor_str[7]='F';
	motor_str[8]='0';
	motor_str[9]='0';
	motor_str[10]='0';
	motor_str[11]='Z';
	motor_str[12]='F';
	motor_str[13]='0';
	motor_str[14]='0';
	motor_str[15]='0';
	motor_str[16]='G';
	motor_str[17]='.';
	if (Lspeed<=0)	{
		motor_str[2]='B';
	}else{
		motor_str[2]='F';
	}
	if (Rspeed<=0){
		motor_str[7]='B';
	}else{
		motor_str[7]='F';
	}
	if (Zspeed<0){
		motor_str[12] = 'B';
	}else{
		motor_str[12] = 'F';
	}
	if (Lspeed>=0&&Rspeed>=0) Go_status=0;
	if (Lspeed>=0&&Rspeed<0) Go_status=1;
	if (Lspeed<0&&Rspeed>=0) Go_status=2;
	if (Lspeed<0&&Rspeed<0) Go_status=3;

	motor_str[16]='G';
	Lspeed=abs(Lspeed);
	Rspeed=abs(Rspeed);
	Zspeed = abs(Zspeed);

	motor_str[3]=Lspeed/100+0x30;
	motor_str[4]=(Lspeed%100)/10+0x30;
	motor_str[5]=Lspeed%10+0x30;

	motor_str[8]=Rspeed/100+0x30;
	motor_str[9]=(Rspeed%100)/10+0x30;
	motor_str[10]=Rspeed%10+0x30;

	motor_str[13]=Zspeed/100+0x30;
	motor_str[14]=(Zspeed%100)/10+0x30;
	motor_str[15]=Zspeed%10+0x30;

	ComSend(motor_str,18);
	memset(motor_str,0x00,sizeof(motor_str));
	return true;
}

bool CMotor::stop(){
	ComSend(":LF000RF000ZF000G.",18);
	return true;
}

void CMotor::VectorMove(float inLV, float inPSpeed){	
	//������������������˶������ٶȣ��������˶��Ľ��ٶ�
	float Vx,Vy;
	float Radius_robot =200.0;  // robot radius: 18cm
	float Wheel_radius = 50.00; ////mm
	double R_theta;
	R_theta=0.0;
	Vx = inLV * cos(R_theta);
	Vy = inLV * sin(R_theta);	
	move_lsp = int((sin(PIf/3 + R_theta) * (Vx) - cos(PIf/3 + R_theta) * (Vy) - Radius_robot * (inPSpeed))/Wheel_radius) ; 		
    move_zsp = int((-sin(R_theta) * (Vx) + cos(R_theta) * (Vy) -Radius_robot * (inPSpeed))/Wheel_radius) ;		
    move_rsp = int((-sin(PIf/3 - R_theta) * (Vx) - cos(PIf/3 - R_theta) * (Vy) - Radius_robot * (inPSpeed))/Wheel_radius); 
	gomotor(-move_lsp,-move_rsp,-move_zsp);//com345:��(�������ȫ���෴һ�������������)

}

int CMotor::m_CalAngle(int angle1, int angle2){
	//����Ƕ�
	int ret;
	ret=angle2-angle1;
	if (ret>180)
		ret=-360+ret;
	if(ret<-180)
		ret=360+ret;
	return ret;
}

bool CMotor::open_com_motor(int CCommport){
	//�򿪵������
	return Create(CCommport);
}

float CMotor::CompromisePS(float inPS){
	if(abs(inPS-m_Last_inPS)>0.5){
		inPS = inPS/6 + 5*m_Last_inPS/6;
		m_Last_inPS = inPS;
	}
	return inPS;
}
float CMotor::CompromiseLV(float inLV){
	if(abs(m_Last_inLV-inLV)>200.0){
		inLV = inLV/6 + 5*m_Last_inLV/6;
		m_Last_inLV = inLV;
	}
	return inLV;
}
void CMotor::CompromisedVectorMove(float inLV,float inPS){
	inLV = CompromiseLV(inLV);
	inPS = CompromisePS(inPS);
	/////
	FILE *alloutxhy;
	alloutxhy = fopen("alloutxhy.txt","a+");
	fprintf(alloutxhy," l:  %f  r:  %f  z:   %f   \n",speed_l, speed_r, speed_z);
	fprintf(alloutxhy,"++++++LV:%f  PS:  %f  \n",inLV, inPS);
	fclose(alloutxhy);
	//////
	VectorMove(inLV,inPS);
	m_Last_inLV = inLV;
	m_Last_inPS = inPS;
}