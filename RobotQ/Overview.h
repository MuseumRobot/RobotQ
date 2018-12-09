#ifndef OVERVIEW_H
#define OVERVIEW_H

#include "stdafx.h"

class Overview : public QFrame{
	Q_OBJECT

public:
	Overview(QWidget *parent);
	~Overview();

public:
	QPointF m_posRobot;		//�����˱�������
	QPointF m_posGoal;		//Ŀ������
	float m_angleRobot;		//�����˱��峯��

private:
	void paintEvent ( QPaintEvent * event );
	void timerEvent(QTimerEvent *event);

private:
	int m_timerId;					// ÿ��Timer��һ��id
	int m_times;					// ����
	QImage m_robotImg;				//������ͼƬ
	QImage m_goalImg;		//Ŀ���ͼƬ
};

#endif // OVERVIEW_H
