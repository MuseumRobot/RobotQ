#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <QDialog>
#include "ui_popupdialog.h"

class PopupDialog : public QDialog{
	Q_OBJECT

public:
	PopupDialog(QWidget *parent = 0);
	~PopupDialog();

public:
	Ui::PopupDialog ui;
};

#endif // POPUPDIALOG_H
