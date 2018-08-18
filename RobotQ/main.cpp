#include "MainGUI.h"
#include <QtGui/QApplication>
#include <QTextCodec>


int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
	MainGUI w;
	w.show();
	return a.exec();
}
