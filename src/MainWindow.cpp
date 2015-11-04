// Qt5 and system includes required by the project
#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <cstdlib>

// Project includes
#include "mainwindow.h"
#include "ui_explorer.h"

const char license[] = "Copyright (c) 2015, Justin Crawford \
All rights reserved.\n\n \
Redistribution and use in source and binary forms, with or without \
modification, are permitted provided that the following conditions are met:\n\n \
* Redistributions of source code must retain the above copyright notice, this \
  list of conditions and the following disclaimer.\n\n \
* Redistributions in binary form must reproduce the above copyright notice, \
  this list of conditions and the following disclaimer in the documentation \
  and/or other materials provided with the distribution.\n\n \
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" \
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE \
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE \
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE \
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL \
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR \
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER \
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, \
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE \
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";

MainWidget::MainWidget(QWidget *parent) : QMainWindow(parent), archive(nullptr), ui(new Ui::MainWindow)
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

void MainWidget::on_actionLicense_triggered(bool checked)
{
	QMessageBox::information(this, "License Information", license);
}

void MainWidget::on_actionNew_triggered(bool checked)
{
	// TODO: Clear all dialogs, tables, etc. and force new files to be created
	// when you click "Save" or "Save as"
}

void MainWidget::on_actionOpen_triggered(bool checked)
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                QDir::home().absolutePath(),
                                                tr("BIG archives (*.big)"));

	// Print a simple message
	if (!fileName.isEmpty())
		printf("User entered location \"%s\"\n", fileName.toStdString().c_str());

	// free the previous archive if it was not already freed
	if (this->archive)
		delete this->archive;

	this->archive = new BigArchive(fileName.toStdString());
}

void MainWidget::on_actionSave_As_triggered(bool checked)
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           QDir::home().absolutePath() ,
                           tr("BIG archives (*.big)"));

	if (!fileName.isEmpty())
		printf("Saved file to \"%s\"\n", fileName.toStdString().c_str());
}

void MainWidget::on_actionExit_triggered(bool checked)
{
	// Bye!
	QApplication::quit();
}

void MainWidget::on_tableWidget_cellDoubleClicked(int, int)
{
	// if it's text data, copy it directly into the plainTextEdit
	// if it's binary data, print a text hex table of the data (within reasonable size) <-- this may be version 2.0

}
