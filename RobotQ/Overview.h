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
	int m_timerId; // 每个Timer有一个id
	int m_times; // 次数
};

#endif // OVERVIEW_H
