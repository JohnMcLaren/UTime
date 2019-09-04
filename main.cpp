/*******************************************************
 * License GPLv3 2007
 * UTime 2019 (c) JML
*******************************************************/
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
