// SerialCom.cpp: implementation of the CSerialCom class.
// For Vorager 2.0
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerialCom.h"
#include <afxwin.h>
#include <conio.h> //调用控制台

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//串口接收线程
UINT ComMonitor(LPVOID pParam)
{
	OVERLAPPED m_os={0};
	m_os.hEvent=NULL;//指定一个I/O操作完成后触发的事件
	CSerialCom * pCom=(CSerialCom *)pParam;
	DWORD dwMaskFlag;
	DWORD dwCommEvent;
	DWORD dwByteToRead;
	DWORD dwRes;
	BYTE rechar[1]={0xff};//用于保存读入数据的一个缓冲区
	bool bWaitingOnStatusHandle = false;

	dwMaskFlag=EV_RXCHAR | EV_CTS;//准备监视的串口事件掩码
	if (!SetCommMask(pCom->m_hCom,dwMaskFlag))//设置要通信事件的掩码,失败则返回
	{
		return 0;
	}
	m_os.hEvent=CreateEvent(NULL,true,false,NULL);//该函数创建一个Event同步对象,并返回该对象的Handle   
	if (m_os.hEvent==NULL)//创建失败则返回0
		return 0;

	while (true)
	{
		//为一个特指的通信设备等待一个事件发生,如果函数成功，返回非零值，否则返回0.
		if (!WaitCommEvent(pCom->m_hCom,&dwCommEvent,&m_os))  

		{
			if (GetLastError() == ERROR_IO_PENDING)//如果得到的错误信息相符，则设置为正在等待状态
			{
				bWaitingOnStatusHandle = true;
			}
            else
				break;
		}
		else
		{
			bWaitingOnStatusHandle=false;
		}
		if (bWaitingOnStatusHandle)
		{
			dwRes = WaitForSingleObject(m_os.hEvent,/*2000*/ INFINITE);//调用线程愿意永远等待下去（无限量时间）
			GetCommMask(pCom->m_hCom,&dwCommEvent);
			if ((dwCommEvent & EV_RXCHAR)==EV_RXCHAR)
			{
				do
				{
					COMSTAT comstat;
					DWORD dwError = 0;
					//此函数清除硬件的通讯错误以及获取通讯设备的当前状态，得到comstat的值
					ClearCommError(pCom->m_hCom,&dwError,&comstat);//检查串口接收缓冲区中的数据个数
					if (comstat.cbInQue==0)//若串口中当前含有的数据字节个数为0
						break;
					ReadFile(pCom->m_hCom,rechar,1,&dwByteToRead,&m_os);//从串口m_hCom中读取数据到缓冲区rechar中
					if (pCom!=NULL)
						pCom->Parse(*(BYTE*)rechar);//接收并且存储rechar数据
				}
				while (true);
			}
			if (dwRes==WAIT_OBJECT_0)
			{
				//该函数返回重叠操作的结果，用来判断异步操作是否完成，它是通过m_os结构中的hEvent是否被置位来判断
				if (!GetOverlappedResult(pCom->m_hCom, &m_os, &dwByteToRead, false))
				{
					do
					{
						COMSTAT comstat;
						DWORD dwError = 0;
						
						ClearCommError(pCom->m_hCom,&dwError,&comstat);
						if (comstat.cbInQue==0)
							break;
						ReadFile(pCom->m_hCom,rechar,1,&dwByteToRead,&m_os);
						if (dwByteToRead>0)
						{
							if (pCom=NULL)
								pCom->Parse(*(BYTE*)rechar);
						}
						else
						{
							CloseHandle(m_os.hEvent);
							return 0;
						}

					}
					while (true);
				}
			}
		}
	}
	CloseHandle(m_os.hEvent);
	return 0;
}

//串口发送线程
UINT SendingThread(LPVOID pParam)
{
	CSerialCom * pCom = (CSerialCom *) pParam;
	POSITION pos;

	while (pCom->IsRuning)
	{
		pos = pCom->m_cmdlist.GetHeadPosition();
		if (pos != NULL)
		{
			CMDBUF * pCbuf = (CMDBUF *)pCom->m_cmdlist.GetNext(pos);

			//如果正在发送，则等待
			while (pCom->bSending == TRUE)
			{
				;
			}

			//发送缓冲列表里的指令
			pCom->ComSend(pCbuf->pCmdBuf,pCbuf->nLen);

			//释放缓冲列表里已发送指令占用的资源
			pCom->m_cmdlist.RemoveHead();
			delete []pCbuf->pCmdBuf;
			delete pCbuf;	
		}
		Sleep(5);
	}
	return 0;
}



CSerialCom::CSerialCom() //构造函数
{
	//默认串口属性
	//m_baudrate=CBR_19200;
	m_baudrate=CBR_115200;
	m_bytesize=8;
	m_stopbits=ONESTOPBIT;
	m_parity=NOPARITY;
	
	bSending = FALSE;
	IsRuning = FALSE;

	//pDispList = NULL;
	//pMoveList = NULL;
}

CSerialCom::~CSerialCom() //析构函数
{
	IsRuning = FALSE;  

	//清除缓冲列表里的数据
	POSITION pos = m_cmdlist.GetHeadPosition();
	while (pos != NULL)
	{
		m_pTempCmd = (CMDBUF *)m_cmdlist.GetNext(pos);
		delete []m_pTempCmd->pCmdBuf;
		delete m_pTempCmd;
	}
	m_cmdlist.RemoveAll();

	Sleep(50);
}

BOOL CSerialCom::Create(int inCom) //根据串口号(inCom)打开指定串口
{
	
	if (TRUE == IsRuning)
	{
		if (inCom == m_com)
		{	//重复打开同一串口
			return FALSE;
		} 
		else
		{	//未关闭原来的串口
			CloseHandle(m_hCom);
		}		
	}
	m_com = inCom;

	//建立串口
	//定义CString 对象，相当于字符数组，MFC中把它封装成一个类了
	CString strCom;
	strCom.Format("COM%d",inCom); //生成一个串口号字符串
	
	//用串口号字符串打开串口并返回打开的串口句柄
	m_hCom=CreateFile(strCom,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);

	if (m_hCom==INVALID_HANDLE_VALUE)		//打开串口失败的处理
	{
	
		m_hCom=NULL; //m_hCom赋NULL,以后可以根据m_hCom是否等于NULL判断串口是否打开
		return false; 
	}

	DCB dcb;			          //设备配置对象，下面是对串口的一般配置

	COMMTIMEOUTS m_CommTimeouts;      //规定读/写操作的超时时间,单位ms
	m_CommTimeouts.ReadIntervalTimeout = 1000;           // 读间隔超时 
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;    // 读时间系数 
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000;      // 读时间常量
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;   // 写时间系数 
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;     // 写时间常量 


    //SetCommTimeouts windows系统利用此函数设定通讯设备读写时的超时参数
	if (!SetCommTimeouts(m_hCom, &m_CommTimeouts))
	{
		//SetCommTimeouts函数调用失败进入
		CloseHandle(m_hCom); //关闭已打开串口
		m_hCom=NULL;
		return false; //return 0;
	}

	GetCommState(m_hCom,&dcb);			//将配置对象设置到打开的串口
	dcb.BaudRate=m_baudrate;			//设置串口波特率
	dcb.ByteSize=m_bytesize;			//设置传输的字节数
	dcb.StopBits=m_stopbits;			//设置停止位
	dcb.Parity=m_parity;				//设置校验方式
	
	//将配置对象设置到打开的串口，如果该失败，清除句柄关闭串口
	if (!SetCommState(m_hCom,&dcb))		
	{
		CloseHandle(m_hCom);
		m_hCom=NULL;
		return false;
	}

	IsRuning = TRUE;

	CString info;
	info.Format("COM %d 打开成功！",inCom);
//	m_PrintToList(info);

	//开启串口接收线程
	AfxBeginThread(ComMonitor,(LPVOID)this,THREAD_PRIORITY_NORMAL);
	
	//开启串口发送线程
	AfxBeginThread(SendingThread,(LPVOID)this,THREAD_PRIORITY_NORMAL);

	


	return true;
}

void CSerialCom::Send(const void *pBuffer, const int iLength) //通过缓冲列表进行指令发送（标准接口）
{
	m_pTempCmd = new CMDBUF;
	m_pTempCmd->pCmdBuf = new UCHAR[iLength];
	memcpy(m_pTempCmd->pCmdBuf,pBuffer,iLength); //复制字符串
	m_pTempCmd->nLen = iLength;

	m_cmdlist.AddTail(m_pTempCmd); //在链表尾处插入新m_pTempCmd数据，链表数据个数加1，返回新的链表尾位置

}
int num;
void CSerialCom::ComSend(const void *pBuffer, const int iLength) //直接发送指令
{
	if (m_hCom==NULL)   //串口没有打开则返回
		return;

	bSending = TRUE;    //发送数据位置TRUE,就是1

	OVERLAPPED m_os={0};
	DWORD dwByteWrite;  //DWORD等于unsigned long，已经写入的字节数
	//_cprintf("send\n"); //向控制台写入
	num++;
	if (!WriteFile(m_hCom,pBuffer,iLength,&dwByteWrite,&m_os)) //从缓冲区向串口写数据
	{
		DWORD iErr=GetLastError();
	}
	Sleep(10);						//程序挂起100ms

	DWORD dwNumberOfBytesTransferred;//用于容纳传输字节数量的一个变量 
//	_cprintf("send %d\n",num); 
	GetOverlappedResult(m_hCom,&m_os,&dwNumberOfBytesTransferred,TRUE);//等待写数据结束，通过m_os判断
	//_cprintf("send ok\n");
	bSending = FALSE;
}

void CSerialCom::Close() //关闭串口函数
{
	//关闭串口
	if (m_hCom!=NULL)  //如果不为空则说明打开了串口，为NULL说明没有打开串口不用执行下面函数
		CloseHandle(m_hCom); //关闭m_hCom句柄对象。
	
	IsRuning = FALSE; 
	m_hCom=NULL;  //关闭后赋NULL值
}

void CSerialCom::SetBaudRate(int inBaud) //设置波特率
{
	m_baudrate = inBaud;
}


//void CSerialCom::m_PrintToList(CString inStr) //列表框pDispList,操作函数，更新显示字符串
//{
//	if (pDispList != NULL) //当列表框存在时
//	{
//		pDispList->AddString(inStr);   //添加一个字符串到列表框中
//		//GetCount函数返回字符数组中所包含的字符数
//		pDispList->SetCurSel(pDispList->GetCount()-1); ////使添加的字符串可见，本函数在组合框的列表框中选择一个字符串。必要时列表框会滚动，以使该字符串在列表的可视区内 
//	}
//}

//void CSerialCom::m_PrintToMoveList(CString inStr) //列表框pMoveList,操作函数，更新显示字符串
//{
//	if (pMoveList != NULL)
//	{
//		pMoveList->AddString(inStr);  //添加一个字符串到列表框中
//		pMoveList->SetCurSel(pMoveList->GetCount()-1);  //使添加的字符串可见
//		//pMoveList->GetCount()获取列表框中项的数目。
//		//SetCurSel()函数在组合框的列表框中选择一个字符串。
//        //必要时列表框会滚动，以使该字符串在列表的可视区内 （列表是可见的时 ）。
//
//	}
//}
