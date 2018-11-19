#include "stdafx.h"
#include "MuseumGUI.h"

MuseumGUI::MuseumGUI(QWidget *parent):QDialog(parent){
	ui.setupUi(this);
}

MuseumGUI::~MuseumGUI(){}

void MuseumGUI::closeEvent(QCloseEvent *event){
	QApplication *app;
	app->quit();
}
