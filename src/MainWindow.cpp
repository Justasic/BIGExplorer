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


MainWidget::MainWidget(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->statusbar->showMessage("Ready");

	// We have 3 columns
	ui->tableWidget->setColumnCount(3);
	QStringList m_columnLabels;
	m_columnLabels << "File" << "Type" << "Size";
	// Set column names.
	ui->tableWidget->setHorizontalHeaderLabels(m_columnLabels);
	// Make column header visible (show column names)
	ui->tableWidget->verticalHeader()->setVisible(false);
	// Remove the grid-style since I want a list.
	ui->tableWidget->setShowGrid(false);
	// Don't allow editing of the cells.
	ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// Select the entire row instead of a specific cell (since this is a list)
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	// Turn off horizontal scroll bar (it looks bad and most OTHER UIs I have seen remove them as well)
	ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	// Stretch the horizontal cells to fit the text they contain.
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

MainWidget::~MainWidget()
{
	delete ui;
}

// Qt slots
void MainWidget::on_pushButton_play_clicked() {}
void MainWidget::on_pushButton_pause_clicked() {}
void MainWidget::on_pushButton_next_clicked() {}
void MainWidget::on_pushButton_previous_clicked() {}
void MainWidget::on_tableWidget_cellDoubleClicked(int, int) {}
