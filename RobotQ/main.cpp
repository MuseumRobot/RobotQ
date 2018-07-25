#include "robotq.h"
#include <QtGui/QApplication>
#include <QtWebKit/QtWebKit>
#include <QtWebKit/QWebView>
#include <QUrl>

int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	RobotQ w;
	w.show();
	return a.exec();
}
