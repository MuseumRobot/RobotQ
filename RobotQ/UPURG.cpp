// UPURG.cpp: implementation of the CUPURG class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "include/UPURG.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

int m_distVal_temp_temp[1000];
int m_nValPoint_temp;//m_distVal的长度

CUPURG::CUPURG(){
	m_lastChar = 0;
	m_ParHeader[0] = 0x47; //G
	m_ParHeader[1] = 0x44; //D
	m_ParHeader[2] = '\0';
	m_ParEnd[0] = '\n';
	m_ParEnd[1] = '\n';
	m_recvbuf = new char[URG_RECVBUF_LEN]; //new动态分配内存,数据包储存
	m_databuf = new char[URG_RECVBUF_LEN]; //数据存储
	m_distVal = new int[URG_RECVBUF_LEN/3];//数据解析结果存储
	m_disPreVal = new int[URG_RECVBUF_LEN/3];//
	m_nRecvindex = 0;       //数据写入位，m_recvbuf
	m_nFrameLen = 0;        //存储数据包的长度
	m_bFrameStart = false;  //数据包接收开始标志位，TRUE为已开始接收，FLASE为没有开始
	m_nValPoint = 0;        //m_distVal的长度
	key = false;
	for (int i = 0;i<1024;i++){
		//URG_RECVBUF_LEN/3=1024
		m_disPreVal[i] = 0;
	}
	m_nTurnLeftFlag = 0;
	m_nTurnRightFlag = 0;
	m_DirFlag = 0;
	for (int i=0;i<1000;i++){
		m_distVal_temp_temp[i]=100;
	}	
}

CUPURG::~CUPURG(){
	delete []m_recvbuf; //释放new分配空间
	delete []m_databuf;
	delete []m_distVal; 
	delete []m_disPreVal;
}

void CUPURG::Parse(BYTE inData){
	//接收数据并存储
	if (m_bFrameStart == false){
		//若没有开始接收数据，则需要检测是否有数据包头出现，数据包接收开始标志位，TRUE为已开始接收，FLASE为没有开始
		//判断是否为包头
		if (inData == m_ParHeader[1] && m_lastChar == m_ParHeader[0]){
			//数据包头出现则开始接收数据
			m_bFrameStart = true;
			memcpy(m_recvbuf,m_ParHeader,2);//写入包头，把包头写入到数据包存储
			m_nRecvindex = 1;//数据写入位
		}
	}else{
		//若已经开始接收数据
		//解析中
		m_nRecvindex ++;//数据写入位加1
		m_recvbuf[m_nRecvindex] = inData;
		//寻找包尾
		if (inData == m_ParEnd[1] && m_lastChar == m_ParEnd[0]){
			//若数据和上一个接收到的字符均为数据包结束格式
			m_nFrameLen = m_nRecvindex + 1;//存储数据包的长度等于数据写入位加1
			m_ParseFrame();//对接收的数据包进行解析
			m_bFrameStart = false;
			m_nRecvindex = 0;
		}
	}
	m_lastChar = inData;//把上一个接收到的字符更改成inData
}

long CUPURG::urg_decode(const char *data, int data_byte){
	//数据解析	
	long value = 0;
	for (int i = 0; i < data_byte; ++i){
		value <<= 6;
		value &= ~0x3f;
		value |= data[i] - 0x30;
	}
	return value;
}

void CUPURG::GetDataByGD(int inStart, int inEnd, int inClusterCnt){
	//当传感器接收到 GDGS 这个命令会反馈最新的测量数据到主机
	sprintf(m_cmdToSend, "GD%04d%04d%02d\n", inStart, inEnd, inClusterCnt);//把格式化的数据写入缓冲区m_cmdToSend
	Send(m_cmdToSend,13);//通过缓冲列表进行指令发送（标准接口）
}

void CUPURG::SCIP20(){
	//使用以下SCIP2.0切换命令改变传感器模式
	sprintf(m_cmdToSend, "SCIP2.0\n");
	Send(m_cmdToSend,8);
}

void CUPURG::SwitchOn(){
	//这个 BM 命令会使激光亮起，使能测量
	sprintf(m_cmdToSend, "BM\n");
	Send(m_cmdToSend,3);
}

void CUPURG::SwitchOff(){
	//这个QT命令将关闭激光，禁用测量状态。
	sprintf(m_cmdToSend, "QT\n");
	Send(m_cmdToSend,3);
}

void CUPURG::m_ParseFrame(){
	//对接收的数据包进行解析
	//计算数据块长度
	int nData = m_nFrameLen - 23/*cmd*/ - 1/*最后一个LF*/;
	int nBlock = nData/66;
	int nLeft = nData%66 - 2/*sum+LF*/;
	char* pDataBuf = m_recvbuf+23; //读取数据指针，跳过23个固定的cmd字符
	//data block
	for (int i=0;i<nBlock;i++){
		memcpy((m_databuf+i*64),pDataBuf,64); //将数据存储复制到变量中保存
		pDataBuf += 66;
	}
	//left
	if (nLeft>0){
		memcpy((m_databuf+nBlock*64),pDataBuf,nLeft);
	}
	//换算成距离值 //数据解析
	m_nValPoint = (nBlock*64+nLeft)/3;
	pDataBuf = m_databuf;
	CString str;
	//for (int i = 0; i <1000;i++)
	m_distVal[0] = 100; 
	m_nValPoint_temp=m_nValPoint;
	for (int i=1;i<m_nValPoint;i++){
		int temp;
		int j = 0;
		temp = urg_decode(pDataBuf,3); //数据解析
		m_distVal[i] = temp;//激光测距器所获得的每个角度的距离
		pDataBuf += 3;
	}
	for (int i=0;i<m_nValPoint;i++){
		for (int j=0;j<100;j++){
				m_distVal[j]=100;//////////////////////////100mm，表示被阻塞，都是障碍物
		}
		for (int j=666;j<769;j++){
			m_distVal[j]=100;
		}
		for (int j=100;j<666;j++){
			if (m_distVal[j]<50){
				m_distVal[j]=10000;/////////////////////////////表明激光器没有测量到障碍物，设定其返回结果为10000mm
			}
	
		}
		m_distVal_temp_temp[i]=m_distVal[i];
		m_distVal_temp_test[key][i]=(double)m_distVal[i];
	}
	key = !key;
	wait_laser.SetEvent();
}



void CUPURG::fileter(int * p){
	for (int i=0;i<100;i++){
		p[i]=100;//////////////////////////100mm，表示被阻塞，都是障碍物
	}
	for (int i=666;i<769;i++){
		p[i]=100;
	}
	for (int i=100;i<666;i++){
		if (p[i]<50){
			p[i]=10000;/////////////////////////////表明激光器没有测量到障碍物，设定其返回结果为10000mm
		}
	}
}