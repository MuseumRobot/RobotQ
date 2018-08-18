#include "DashBoard.h"

DashBoard::DashBoard(QWidget *parent):QDialog(parent){
	ui.setupUi(this);
	connect(ui.btnSaveLog,SIGNAL(clicked()),this,SLOT(OnBtnSaveLog()));
	m_timerId=startTimer(1000);	//每秒钟更新一次即可(事实上由于处理也需要时间，可能会略过一秒)
}
DashBoard::~DashBoard(){

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