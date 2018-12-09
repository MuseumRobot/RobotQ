#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "stdafx.h"
#include "ui_DashBoard.h"
#include "Overview.h"

class DashBoard : public QDialog{
	Q_OBJECT
private:
	int m_timerId;
public:
	DashBoard(QWidget *parent = 0);
	~DashBoard();
	Ui::DashBoard ui;
	Overview* m_Overview;
	QTime m_time;
	QDateTime m_day;
	void AppendMessage(QString strMsg);
	virtual void timerEvent(QTimerEvent *event);
private slots:
	void OnBtnSaveLog();
};

#endif // DASHBOARD_H
