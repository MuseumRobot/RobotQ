#include "stdafx.h"
#include "MainGUI.h"
#include <QtGui/QApplication>
#include <QTextCodec>


int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
	MainGUI w;
	if(MUSEUMMODE == 0){
		w.move(20,20);
	}
	w.show();
	return a.exec();
}
