#ifndef OVERVIEW_H
#define OVERVIEW_H

#include "stdafx.h"

class Overview : public QFrame{
	Q_OBJECT

public:
	Overview(QWidget *parent);
	~Overview();

public:
	QPointF m_pos;

private:
	void paintEvent ( QPaintEvent * event );
	void timerEvent(QTimerEvent *event);

private:
	int m_timerId; // ÿ��Timer��һ��id
	int m_times; // ����
};

#endif // OVERVIEW_H
