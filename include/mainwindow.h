#pragma once

#include <QWidget>
#include <QMainWindow>
#include "BIGFiles.h"

// defined in Main.cpp
extern std::vector<std::string> musicfiles;

namespace Ui { class MainWindow; };

class MainWidget : public QMainWindow
{
Q_OBJECT

public:
	explicit MainWidget(QWidget *parent = 0);
	virtual ~MainWidget();

	BigArchive *archive;

private slots:
	// Qt slots
	void on_actionLicense_triggered(bool checked);
	void on_actionNew_triggered(bool checked);
	void on_actionOpen_triggered(bool checked);
	void on_actionSave_As_triggered(bool checked);
	void on_actionExit_triggered(bool checked);


	void on_tableWidget_cellDoubleClicked(int, int);

private:
	Ui::MainWindow *ui;
};
