#ifndef ROBOTQ_H
#define ROBOTQ_H

#include <QtGui/QMainWindow>
#include "ui_robotq.h"

class RobotQ : public QMainWindow{
	Q_OBJECT

public:
	RobotQ(QWidget *parent = 0, Qt::WFlags flags = 0);
	~RobotQ();

private slots:
	int OnStartClicked(bool checked);
	int OnEndClicked(bool checked);

private:
	Ui::RobotQClass ui;
};

#endif // ROBOTQ_H
