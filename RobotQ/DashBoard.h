#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QDialog>
#include "ui_DashBoard.h"

class DashBoard : public QDialog
{
	Q_OBJECT

public:
	DashBoard(QWidget *parent = 0);
	~DashBoard();

private:
	Ui::DashBoard ui;
};

#endif // DASHBOARD_H
