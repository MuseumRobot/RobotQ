#ifndef MANUALCONTROL_H
#define MANUALCONTROL_H

#include <QDialog>
#include "ui_ManualControl.h"

class ManualControl : public QDialog{
	Q_OBJECT

public:
	ManualControl(QWidget *parent = 0);
	~ManualControl();

private:
	Ui::ManualControl ui;
};

#endif // MANUALCONTROL_H
