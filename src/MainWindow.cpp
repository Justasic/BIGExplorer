// Qt5 and system includes required by the project
#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QDesktopWidget>
#include <QTableWidgetItem>
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

enum
{
		MW_FILE, // this corresponds to the "file" column
 		MW_TYPE, // this corresponds to the "Type" column
		MW_SIZE, // this corresponds to the "Size" column
};

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

void MainWidget::OpenArchiveFile(const QString &fileName)
{
	this->ui->statusbar->showMessage(_("Opening archive \"%s\"...", fileName.toStdString()).c_str());

	// free the previous archive if it was not already freed
	if (this->archive)
		delete this->archive;

	// Attempt to open the archive.
	try
	{
		this->archive = new BigArchive(fileName.toStdString());
	}
	catch (std::system_error se)
	{
		QMessageBox::critical(this, "Error",
							_("Error opening \"%s\": %s (%d)", fileName.toStdString(), se.what(), se.code()).c_str());

		if (this->archive)
			delete this->archive;

		this->archive = nullptr;
		return;
	}

	printf("%zu entries loaded\n", this->archive->FileEntries.size());

	if (this->archive->GetArchiveCorruptFlag())
		QMessageBox::warning(this, "Warning: Corrupt Archive", "The size of this archive differs from when it was originally created, this archive may be corrupt!");

	// Populate the list with files.
	int row = 0;
	for (auto file : this->archive->FileEntries)
	{
		printf("populating table item %d: %s\n", row, file->filename);
		this->ui->tableWidget->setItem(row, 0, new QTableWidgetItem(file->filename));
		this->ui->tableWidget->setItem(row, 1, new QTableWidgetItem("Unknown."));
		this->ui->tableWidget->setItem(row, 2, new QTableWidgetItem(GetHighestSize(file->size).c_str()));
		row++;
	}

	// we're done! :D
	this->ui->statusbar->showMessage(_("Opened \"%s\" which is %s big and contains %d files in %s format.",
									this->archive->GetFileName(), GetHighestSize(this->archive->GetFileSize()),
									this->archive->GetFileCount(), this->archive->GetArchiveType()).c_str());
}

void MainWidget::on_actionLicense_triggered(bool checked)
{
	QMessageBox::about(this, "License Information", license);
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


	// Open the archive.
	this->OpenArchiveFile(fileName);
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
