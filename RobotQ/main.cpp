#include "robotq.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	RobotQ w;
	w.show();
	return a.exec();
}
