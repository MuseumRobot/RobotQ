#include "stdafx.h"
#include "Overview.h"

Overview::Overview(QWidget *parent):QFrame(parent){
	m_timerId = startTimer(100);
	this->setFixedSize(500,1000);
	m_robotImg.load("Resources/yellowCar.png");
	m_goalImg.load("Resources/redFlag.png");
	m_angleRobot = 0;
}
Overview::~Overview(){}
void Overview::paintEvent (QPaintEvent* event){
	QPainter painter(this);
	int width = this->width();
	int height = this->height();

	// 控制如何绘制外围的线条
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(QColor(0,0,0)));
	painter.drawRect(QRect(0,0,width,height));
	// 控制了如何填充
	if(m_times == 0){
		QBrush brush(QColor(0x00, 0xFF, 0));
		painter.setBrush(brush);
		m_times ++;
	}
	else{
		QBrush brush(QColor(0x88, 0x88, 0x88));
		painter.setBrush(brush);
		m_times = 0;
	}
	painter.drawEllipse(0,0,40,40);

	QMatrix matrix;
	matrix.rotate(180-m_angleRobot);			//rotate默认顺时针旋转
	painter.drawImage(m_posRobot*0.5,m_robotImg.scaled(40,40).transformed(matrix,Qt::FastTransformation));
	painter.drawImage(m_posGoal*0.5,m_goalImg);

}
void Overview::timerEvent(QTimerEvent *event){
	if(event->timerId() == m_timerId){
		update();
	}
}