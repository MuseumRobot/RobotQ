#pragma once
#include "stdafx.h"
#include "include/SerialCom.h"
#include <math.h>

#define Drobot_wheel_spacing 359//轮间距

class CMotor:public CSerialCom{
public:

	volatile int lift_num ;//左轮数据存储
	volatile int right_num ;//右轮数据存储
	volatile int zhong_num;//中轮数据存储
	volatile int motor_timer ;//时间数据存储

	int encoder_l; //左编码器光栅数
	int encoder_r; //右编码器光栅数
	int encoder_z; //中编码器光栅数
	//extern int timer; //时间


	int timer ;  //时间记录
	int timerold ; //上一次时间
	int timer_dif ; //时间差

	int time_hour; //小时
	int time_minute; //分钟
	int time_second; //秒
	int time_msec; //毫秒

	int errornum ;
	int speed_stated; //定速
	float speed_l; //左轮实时速度
	float speed_r ; //右轮实时速度
	float speed_z ; //右轮实时速度

public:

	void Parse(BYTE inData);//串口接收并解析线程
	char m_cmdToSend[512];//数据缓冲区

	//recv
	BYTE m_recvbuf[50]; //数据包储存

	//parse
	char m_ParHeader; //数据包的开头格式
	char m_ParEnd;  //数据包结束格式
	char ldata_star;
	char rdata_star;
	char zdata_star;
	char Time_start;

	char m_lastChar;   //存储上一个接收到的字符
	bool m_bFrameStart; //数据包接收开始标志位，TRUE为已开始接收，FLASE为没有开始
	bool m_right;//数据包开始正确
	int m_nFrameLen;    //存储数据包的长度
	int m_nRecvindex; //数据写入位，m_recvbuf

	char* m_databuf; //数据存储
	int* m_distVal;  //数据解析结果存储
	CMotor(void);
	~CMotor(void);
	char motor_str[20];//
	int Go_status;//前进状态
	int Go_status_old;//前进状态
	int move_lsp;
	int move_rsp;
	int move_zsp;

	
	bool gomotor(int Lspeed, int Rspeed, int Zspeed);	//前进函数 Lspeed 左轮速度 Rspeed 右轮速度  单位cm/s
	bool stop();	//停止
	void VectorMove(double inAngle,float inLV,float inPSpeed); //左右前后移动控制，三个参数分别代表机器人坐标系与全局坐标系在水平方向的夹角，机器人运动的线速度，机器人运动的角速度
	int CMotor::m_CalAngle(int angle1, int angle2);//计算角度
	//打开电机串口
	bool open_com_motor(int CCommport);
	void m_ParseFrame(void);//串口数据解析

};