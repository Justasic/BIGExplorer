#pragma once

#include <QWidget>
#include <QMainWindow>
#include <xmp.h>

// defined in Main.cpp
extern std::vector<std::string> musicfiles;

namespace Ui { class MainWindow; };

class MainWidget : public QMainWindow
{
Q_OBJECT

public:
	explicit MainWidget(QWidget *parent = 0);
	virtual ~MainWidget();

private slots:
	// Qt slots
	void on_pushButton_play_clicked();
	void on_pushButton_pause_clicked();
	void on_pushButton_next_clicked();
	void on_pushButton_previous_clicked();
	void on_tableWidget_cellDoubleClicked(int, int);

private:
	Ui::MainWindow *ui;
};
