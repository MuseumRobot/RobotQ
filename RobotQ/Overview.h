#ifndef OVERVIEW_H
#define OVERVIEW_H

#include "stdafx.h"

class Overview : public QFrame{
	Q_OBJECT

public:
	Overview(QWidget *parent);
	~Overview();

public:
	QPointF m_posRobot;		//机器人本体坐标
	QPointF m_posGoal;		//目标坐标
	float m_angleRobot;		//机器人本体朝向

private:
	void paintEvent ( QPaintEvent * event );
	void timerEvent(QTimerEvent *event);

private:
	int m_timerId;					// 每个Timer有一个id
	int m_times;					// 次数
	QImage m_robotImg;				//机器人图片
	QImage m_goalImg;		//目标点图片
};

#endif // OVERVIEW_H
