#include "StdAfx.h"
#include "include/Comm_data_star.h"

#include <string>
using namespace std;

Cstar::Cstar(void){
	m_bFrameStart=false;
	m_nRecvindex=0;
	m_nFrameLen=0;
	key = false;
	times = 0;
	sumID = 0.00;
	starID = 0.00;
	starID2 = 0.00;
}


Cstar::~Cstar(void){

}

void Cstar::Parse(BYTE inData){
	//~^I624|+42.37|+76.83|+31.75|206.22`
	//½ÓÊÕº¯Êý
	if (m_bFrameStart==false){
		if ((inData=='~')){
			m_bFrameStart=true;
			m_nRecvindex = 0;
			m_recvbuf[m_nRecvindex]=inData;
			if (!key){
				m_pInBuffer1 += inData;
			}else{
				m_pInBuffer1 += inData;
			}
			
		}
	}else{
		m_nRecvindex++;
		m_recvbuf[m_nRecvindex]=inData;
		if (m_recvbuf[2] == '1'){
			m_buffer_maxnum = 38;
			LandmarkNum = 1;
		}else if (m_recvbuf[2] == '2'){
			m_buffer_maxnum = 80;
			LandmarkNum = 2;
		}else if (m_recvbuf[2] == 'I'){
			m_buffer_maxnum = 38;
			LandmarkNum = 1;
		}else{
			m_buffer_maxnum = 10;
		}
		if (m_nRecvindex>m_buffer_maxnum){
			m_nRecvindex=0;
			m_bFrameStart=false;
		}
		if (!key){
			m_pInBuffer1 += inData;
		}else{
			m_pInBuffer1 += inData;
		}
		if (inData=='`'){
			m_bFrameStart=false;
			key = !key;
			m_ParseFrame();
		}
	}
}

#define MAX_MARK_NUM 3

void Cstar::m_ParseFrame(void){
	int num_ETX	= m_pInBuffer1.Find('`');
	int len	= m_pInBuffer1.GetLength();
	int count=0;
	int data_len;
	int num_of_Landmark;
	CString strGetData;
	CString strToken;
	CString strCommand, strData;
	CString strLogData;
	CString strReceiveData[80];
	char temp;

	while (num_ETX!=-1){
		m_pBuffer	=  m_pInBuffer1.Left(num_ETX+1);
		m_pInBuffer1 =  m_pInBuffer1.Right(len-num_ETX-1);
		num_ETX	=  m_pInBuffer1.Find('.');
		len		=  m_pInBuffer1.GetLength();

		if ( m_pBuffer.Left(2)=="~!") m_pRecvAck =  m_pBuffer;
		else if ( m_pBuffer.Left(2)=="~$"){
			m_pRecvGet =  m_pBuffer;
			strGetData =  m_pBuffer;
			data_len = strGetData.GetLength();
			strGetData = strGetData.Left(data_len-1);
			data_len = strGetData.GetLength();
			strGetData = strGetData.Right(data_len-2);
			AfxExtractSubString(strToken, strGetData, 0, '|');
			strCommand = strToken;
			AfxExtractSubString(strToken, strGetData, 1, '|');
			strData = strToken;
			if (strCommand=="Version")			m_pVersion_Get = strData;
			else if (strCommand=="MarkType")	m_pMarkType_Get = strData;
			else if (strCommand=="MarkMode")	m_pMarkMode_Get = strData;
			else if (strCommand=="HeightFix")	m_pMarkHeightFix_Get = strData;
			else if (strCommand=="MarkHeight")	m_pMarkHeight_Get = strData;
			else if (strCommand=="IDNum")		m_pIDNum_Get = strData;
			else if (strCommand=="RefID")		m_pRefID_Get = strData;
			else if (strCommand=="BaudRate")	m_pBaudRate_Get = strData;
		}else if ( m_pBuffer.Left(2)=="~^"){
			m_pRecvData =  m_pBuffer;
			strGetData =  m_pBuffer;
			temp = strGetData[2];
			if(temp == 'I'){
				num_of_Landmark = 1;
			}else{
				num_of_Landmark = temp - '0';
				if (num_of_Landmark<=0 || num_of_Landmark>35) return ;
			}
			data_len = strGetData.GetLength();
			strGetData = strGetData.Left(data_len-1);
			data_len = strGetData.GetLength();
			strGetData = strGetData.Right(data_len-3);
			while (AfxExtractSubString(strToken, strGetData, count, '|')!=NULL){
				strReceiveData[count++] = strToken;
			}
			if (count!=num_of_Landmark*5) return ;
			for (int i=0; i<num_of_Landmark; i++){
				m_pData_IDNum[i] = strReceiveData[i*5+0];
				m_pData_Angle[i] = strReceiveData[i*5+1];
				m_pData_X[i] = strReceiveData[i*5+2];
				m_pData_Y[i] = strReceiveData[i*5+3];
				m_pData_Z[i] = strReceiveData[i*5+4];
				strLogData = strLogData + m_pData_IDNum[i] + " " + m_pData_Angle[i] + " " + m_pData_X[i] + " " + m_pData_Y[i] + " " + m_pData_Z[i] + "\n";
			}
			times++;
			newID = _ttof(m_pData_IDNum[0]);
			sumID += newID;
			sumAngle +=  _ttof(m_pData_Angle[0]);
			sumX +=  _ttof(m_pData_X[0]);
			sumY +=  _ttof(m_pData_Y[0]);
			if (times == 4){
				if(sumID/times == newID){
					starID = newID;					
					oldID = newID;
					starAngel = sumAngle/times;
					starX = sumX/times;
					starY = sumY/times;
					oldAngle = starAngel;
					oldX = starX;
					oldY = starY;
				}else{
					starID = oldID;
					starAngel = oldAngle;
					starX = oldX;
					starY = oldY;
				}
				times = 0;
				sumID = 0.00;
				sumAngle = 0.00;
				sumX = 0.00;
				sumY = 0.00;
			}
			if(LandmarkNum == 2){
				times2++;
				newID2 = _ttof(m_pData_IDNum[1]);
				sumID2 += newID2;
				sumAngle2 +=  _ttof(m_pData_Angle[1]);
				sumX2 +=  _ttof(m_pData_X[1]);
				sumY2 +=  _ttof(m_pData_Y[1]);
				if (times2 == 4){
					if(sumID2/times2 == newID2){
						starID2 = newID2;
						oldID2 = newID2;
						starAngel2 = sumAngle2/times2;
						starX2 = sumX2/times2;
						starY2 = sumY2/times2;
						oldAngle2 = starAngel2;
						oldX2 = starX2;
						oldY2 = starY2;
					}else{
						starID2 = oldID2;
						starAngel2 = oldAngle2;
						starX2 = oldX2;
						starY2 = oldY2;
					}
					times2 = 0;
					sumID2 = 0.00;
					sumAngle2 = 0.00;
					sumX2 = 0.00;
					sumY2 = 0.00;
				}
			}else if (LandmarkNum == 1){
				times2 = 0;
				sumID2 = 0.00;
				sumAngle2 = 0.00;
				sumX2 = 0.00;
				sumY2 = 0.00;
			}
			for (int i=num_of_Landmark; i<MAX_MARK_NUM; i++){
				m_pData_IDNum[i] = "";
				m_pData_Angle[i] = "";
				m_pData_X[i] = "";
				m_pData_Y[i] = "";
				m_pData_Z[i] = "";
			}
		}
	}
}
bool  Cstar::open_com(int CCommport){
	return Create(CCommport);
}

