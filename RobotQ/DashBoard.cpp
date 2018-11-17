#include "stdafx.h"
#include "DashBoard.h"

DashBoard::DashBoard(QWidget *parent):QDialog(parent){
	ui.setupUi(this);
	connect(ui.btnSaveLog,SIGNAL(clicked()),this,SLOT(OnBtnSaveLog()));
	m_timerId=startTimer(1000);	//每秒钟更新一次即可(事实上由于处理也需要时间，可能会略过一秒)
}
DashBoard::~DashBoard(){

}
void DashBoard::AppendMessage(QString strMsg){
	QString strMessage = "";
	strMessage=ui.textStatus->toPlainText();
	int nMessageLenMax = 1024;		//状态文本框最大承受1024个字符
	if(strMessage.length() > nMessageLenMax){
		strMessage = strMessage.left(nMessageLenMax);	//如果实际字符长度超过预设最大值，则截取最新部分予以显示，旧的舍弃
	}
	QString strNewMessage = "";
	strNewMessage = strMsg;
	if(strMessage.length() > 0){
		strNewMessage += "\r\n";
		strNewMessage += strMessage;
	}
	ui.textStatus->setPlainText(strNewMessage);
}
void DashBoard::OnBtnSaveLog(){
	QString Filename = "RobotQLog";
	Filename = Filename + m_day.toString("yyyyMMdd") + m_time.toString("hhmmss") + ".txt";
	QString filepath=QFileDialog::getSaveFileName(
		this,	//父窗口
		GBK::ToUnicode("保存日志"),	//标题caption
		Filename,
		tr("LOG (*.txt)")
		);
	if(filepath.length()>0){
		QString text=ui.textStatus->toPlainText();
		string gbk_text=GBK::FromUnicode(text);
		string gbk_filename=GBK::FromUnicode(filepath);
		//打开文件
		FILE* fp = fopen(gbk_filename.c_str(),"wb");
		fwrite(gbk_text.c_str(),1,gbk_text.length(),fp);
		fclose(fp);
	}
}
void DashBoard::timerEvent(QTimerEvent *event){
	//可以有多个定时器，每个定时器有不同的处理
	if(event->timerId()==m_timerId){
		m_day=QDateTime::currentDateTime();
		m_time=QTime::currentTime();
	}
}