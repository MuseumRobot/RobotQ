#include "stdafx.h"
#include "MuseumGUI.h"

MuseumGUI::MuseumGUI(QWidget *parent):QDialog(parent){
	ui.setupUi(this);
	connect(ui.btnLock,SIGNAL(clicked()),this,SLOT(OnBtnLock()));
	isLocked = false;
	ui.block1->hide();
	ui.block2->hide();
}

MuseumGUI::~MuseumGUI(){}

void MuseumGUI::closeEvent(QCloseEvent *event){
	QApplication *app;
	app->quit();
}
void MuseumGUI::OnBtnLock(){
	if(isLocked){
		ui.btnLock->setIcon(QIcon("Resources/unlocked.png"));
		ui.block1->hide();
		ui.block2->hide();
	}else{
		ui.btnLock->setIcon(QIcon("Resources/locked.png"));
		ui.block1->show();
		ui.block2->show();
	}
	ui.btnLock->setIconSize(QSize(40,40));
	isLocked = !isLocked;
}
