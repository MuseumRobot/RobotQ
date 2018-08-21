#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QDialog>
#include <QCheckBox>
#include <QFileDialog>
#include <GBK.h>
#include <QString>
#include <QTime>
#include <QDateTime>
#include <QPlainTextEdit>
#include "ui_DashBoard.h"

class DashBoard : public QDialog{
	Q_OBJECT
private:
	int m_timerId;
	QTime m_time;
	QDateTime m_day;
public:
	DashBoard(QWidget *parent = 0);
	~DashBoard();
	Ui::DashBoard ui;
	void AppendMessage(QString strMsg);
	virtual void timerEvent(QTimerEvent *event);
private slots:
	void OnBtnSaveLog();
};

#endif // DASHBOARD_H
