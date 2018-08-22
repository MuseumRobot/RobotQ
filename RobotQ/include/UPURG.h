// UPURG.h: interface for the CUPURG class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPURG_H__450564AC_462A_4B7B_8206_579EB9A55BD8__INCLUDED_)
#define AFX_UPURG_H__450564AC_462A_4B7B_8206_579EB9A55BD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "include/SerialCom.h"
#include <afxmt.h>

extern int m_distVal_temp_temp[768];
extern int m_nValPoint_temp;//m_distVal的长度

#define URG_RECVBUF_LEN 3072

struct Beam{
	double left_angle, right_angle;		
	double width_of_angle;				//right_angle-left_angle
	double left_dis,right_dis,min_distance;
	double width;
	int m_Obstacle;						
};

/***                 ***/
class CUPURG : public CSerialCom{
public:
	void SwitchOff(); //断开指令
	void SwitchOn();  //握手指令
	void SCIP20();
	void GetDataByGD(int inStart,int inEnd,int inClusterCnt);
	void Parse(BYTE inData); //接收数据并存储
public:
	CUPURG();              //构造函数
	virtual ~CUPURG();     //析构函数
protected:
	long urg_decode(const char* data, int data_byte); //数据解析函数之一
public:
	int m_DirFlag; 
	int m_nTurnRightFlag;	//转向标志位
	int m_nTurnLeftFlag;	//转向标志位
	int m_nObstacleOnLeft;	//最近障碍物在左侧标志
	int m_nObstacleOnRight;	//最近障碍物在右侧标志
	int m_nDangerDir;		//判断障碍类型依据之一
	CEvent wait_laser;		//激光线程中断开关
	bool key;				//数据存储控制开关,false--m_distVal_temp_test[0]接收;ture--m_distVal_temp_test[1]接收
	void fileter(int * p);
	int *m_distVal_temp;
	int m_distVal_temp_test[2][768];
	double robotPosition_x;
	double robotPosition_y;
	int target_angle;		//the angle between robot and target 0-72 ,correspond to m_vfh[72]
	int target_flag;		//标记目标角度在自由区域的哪个区间
	double robotsize;
protected:
	void m_ParseFrame();	//对接收的数据包进行解析
	char m_cmdToSend[512];	//send
	char* m_recvbuf;		//数据包储存
	//parse
	char m_ParHeader[3];	//数据包的开头格式
	char m_ParEnd[2];		//数据包结束格式
	char m_lastChar;		//存储上一个接收到的字符
	bool m_bFrameStart;		//数据包接收开始标志位，TRUE为已开始接收，FLASE为没有开始
	int m_nFrameLen;		//存储数据包的长度
	int m_nRecvindex;		//数据写入位，m_recvbuf
	char* m_databuf;		//数据存储
	int* m_distVal;			//数据解析结果存储
	int * m_disPreVal;		//*****暂时没用到
	int m_nValPoint;		//m_distVal的长度

	//display
	CString m_strTmp; //
	CRect m_FrameRect;  //CRect类为一个矩形，相当于定义一个矩形
};

#endif // !defined(AFX_UPURG_H__450564AC_462A_4B7B_8206_579EB9A55BD8__INCLUDED_)
