#include "stdafx.h"
#include "Overview.h"

Overview::Overview(QWidget *parent):QFrame(parent){
	m_timerId = startTimer(100);
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

	painter.drawEllipse(m_pos.x(),m_pos.y(),40,40);


}
void Overview::timerEvent(QTimerEvent *event){
	if(event->timerId() == m_timerId){
		update();
	}
}