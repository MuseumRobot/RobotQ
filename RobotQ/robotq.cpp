#include "robotq.h"

RobotQ::RobotQ(QWidget *parent, Qt::WFlags flags):QMainWindow(parent, flags){
	ui.setupUi(this);
	connect(ui.btnStart,SIGNAL(clicked(bool)),this,SLOT(OnStartClicked(bool)));
	connect(ui.btnEnd,SIGNAL(clicked(bool)),this,SLOT(OnEndClicked(bool)));
}
RobotQ::~RobotQ(){

}

int RobotQ::OnStartClicked(bool checked){
	ui.textStatus->setPlainText("Start");

	return 0;
}
int RobotQ::OnEndClicked(bool checked){
	ui.textStatus->setPlainText("End");

	return 0;
}