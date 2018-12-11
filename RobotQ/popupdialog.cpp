#include "stdafx.h"
#include "popupdialog.h"

PopupDialog::PopupDialog(QWidget *parent):QDialog(parent){
	ui.setupUi(this);
	this->setWindowFlags(Qt::FramelessWindowHint);
}

PopupDialog::~PopupDialog(){}
