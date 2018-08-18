// SerialCom.h: interface for the CSerialCom class.
// For Vorager 2.0
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALCOM_H__78D38E89_1031_495D_8967_84842B40DF86__INCLUDED_)
#define AFX_SERIALCOM_H__78D38E89_1031_495D_8967_84842B40DF86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afx.h>
#include <afxwin.h>
#include <WinSock2.h>
#include <QString>


using std::string;

//发送指令的缓冲列表成员
struct CMDBUF 
{
	UCHAR * pCmdBuf;	//指令数组指针
	UINT nLen;			//指令长度
};
/***************************************************/
//串口通信类
/***************************************************/
class CSerialCom{
public:
	CSerialCom();             //构造函数                                        
	virtual ~CSerialCom();	  //析构函数
public:
	//void m_PrintToMoveList(CString inStr);                  //列表框更新函数
	//void m_PrintToList(CString inStr);                      //列表框更新函数
	virtual void Parse(BYTE inData){};
	void SetBaudRate(int inBaud);                           //设置波特率
	BOOL Create(int inCom);									//打开一个串口端口
	void Close();											//关闭当前打开的端口
	void Send(const void *pBuffer, const int iLength);		//通过缓冲列表进行指令发送（标准接口）
	void ComSend(const void *pBuffer, const int iLength);	//直接发送指令


	CPtrList m_cmdlist;										//发送指令的缓冲列表
	CMDBUF * m_pTempCmd;									//发送指令缓冲成员临时指针
	HANDLE m_hCom;											//已打开的串口句柄
	BOOL bSending;											//发送标记
	BOOL IsRuning;                                          //判断串口是否打开标志

	//show //CListBox类提供Windows列表框的功能。
	//CListBox* pDispList; 
//	CListBox * pMoveList;


protected:
	//串口属性
	int m_com;			//串口号									
	int m_baudrate;     //波特率
	BYTE m_bytesize;   // 字节数
	BYTE m_parity;   //校验方式
	BYTE m_stopbits;  //停止位

private:
	
};

#endif // !defined(AFX_SERIALCOM_H__78D38E89_1031_495D_8967_84842B40DF86__INCLUDED_)
