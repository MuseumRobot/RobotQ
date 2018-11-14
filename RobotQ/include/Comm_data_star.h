#pragma once

#include "stdafx.h"
#include <afxwin.h>
#include <math.h>
#include <WinSock2.h>
#include "include/SerialCom.h"
class Cstar : public CSerialCom{
public:
	Cstar(void);
	~Cstar(void);
	bool m_bFrameStart;
	BYTE m_recvbuf[90];
	BYTE m_recvbuf_temp[2][90];
	bool key;		//数据存储控制开关 false--m_pInBuffer1接收;ture--m_pInBuffer2接收
	int m_nRecvindex;
	int m_nFrameLen;
	int m_buffer_maxnum;
	int LandmarkNum;

	CString m_pInBuffer1;
	CString m_pInBuffer2;
	CString m_pBuffer;
	CString	m_pRecvData;
	CString	m_pData_Angle[10];
	CString	m_pData_IDNum[10];
	CString	m_pData_X[10];
	CString	m_pData_Y[10];
	CString	m_pData_Z[10];

	CString	m_pRecvAck; //指令回应存储
	CString	m_pRecvGet; //数据
	CString	m_pVersion_Get;
	CString	m_pMarkType_Get;
	CString	m_pMarkMode_Get;
	CString	m_pMarkHeightFix_Get;
	CString	m_pMarkHeight_Get;
	CString	m_pIDNum_Get;
	CString	m_pRefID_Get;
	CString	m_pBaudRate_Get;

	int times;
	int newID;
	int sumID;
	int oldID;
	int starID ;

	float starAngel ;
	float sumAngle;
	float oldAngle;

	float starX ;
	float sumX;
	float oldX;

	float starY ;
	float sumY;
	float oldY;

	int times2;
	int newID2;
	int sumID2;
	int oldID2;
	int starID2;

	float starAngel2 ;
	float sumAngle2;
	float oldAngle2;

	float starX2 ;
	float sumX2;
	float oldX2;

	float starY2 ;
	float sumY2;
	float oldY2;

	void Parse(BYTE inData);//串口接收并解析线程
	bool open_com(int CCommport);
	void m_ParseFrame(void);
};
