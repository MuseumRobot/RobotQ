#include "stdafx.h"
#include "DashBoard.h"

DashBoard::DashBoard(QWidget *parent):QDialog(parent){
	ui.setupUi(this);
	connect(ui.btnSaveLog,SIGNAL(clicked()),this,SLOT(OnBtnSaveLog()));
	m_timerId=startTimer(1000);	//ÿ���Ӹ���һ�μ���(��ʵ�����ڴ���Ҳ��Ҫʱ�䣬���ܻ��Թ�һ��)
}
DashBoard::~DashBoard(){

}
void DashBoard::AppendMessage(QString strMsg){
	QString strMessage = "";
	strMessage=ui.textStatus->toPlainText();
	int nMessageLenMax = 1024;		//״̬�ı���������1024���ַ�
	if(strMessage.length() > nMessageLenMax){
		strMessage = strMessage.left(nMessageLenMax);	//���ʵ���ַ����ȳ���Ԥ�����ֵ�����ȡ���²���������ʾ���ɵ�����
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
		this,	//������
		GBK::ToUnicode("������־"),	//����caption
		Filename,
		tr("LOG (*.txt)")
		);
	if(filepath.length()>0){
		QString text=ui.textStatus->toPlainText();
		string gbk_text=GBK::FromUnicode(text);
		string gbk_filename=GBK::FromUnicode(filepath);
		//���ļ�
		FILE* fp = fopen(gbk_filename.c_str(),"wb");
		fwrite(gbk_text.c_str(),1,gbk_text.length(),fp);
		fclose(fp);
	}
}
void DashBoard::timerEvent(QTimerEvent *event){
	//�����ж����ʱ����ÿ����ʱ���в�ͬ�Ĵ���
	if(event->timerId()==m_timerId){
		m_day=QDateTime::currentDateTime();
		m_time=QTime::currentTime();
	}
}