// Qt5 and system includes required by the project
#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QDesktopWidget>
#include <cstdlib>

// Project includes
#include "mainwindow.h"
#include "ui_explorer.h"


explicit MainWidget::MainWidget(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->statusbar->showMessage()
}

MainWidget::~MainWidget()
{
	delete ui;
}

// Qt slots
void on_pushButton_play_clicked() {}
void on_pushButton_pause_clicked() {}
void on_pushButton_next_clicked() {}
void on_pushButton_previous_clicked() {}
void on_tableWidget_cellDoubleClicked(int, int) {}
