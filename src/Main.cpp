#include <cstdio>
#include <cstdint>
#include <QApplication>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

// Include our interface.
#include "ui_explorer.h"
#include "mainwindow.h"

std::vector<std::string> Arguments;

int main(int argc, char **argv)
{
	Arguments = std::vector<std::string>(argv, argv+argc);
	QApplication application(argc, argv);

	MainWidget w;

	w.show();
	
	return application.exec();
}
