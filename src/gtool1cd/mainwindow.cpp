/*
    GTool1CD provides GUI front end to Tool1CD library
    Copyright © 2009-2017 awa
    Copyright © 2017-2018 E8 Tools contributors

    This file is part of GTool1CD.

    GTool1CD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GTool1CD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GTool1CD.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QStringListModel>
#include "starter.h"
#include "cache.h"
#include "table_fields_window.h"
#include "table_data_window.h"
#include "models/tables_list_model.h"
#include "configurations_window.h"
#include "about_dialog.h"

void MainWindow::AddDetailedMessage(
        const std::string &description,
        const MessageState mstate,
        const TStringList *param)
{
	this->addLogMessage(QString(description.c_str()));
}

void MainWindow::Status(const std::string& message)
{

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    configurationsWindow(nullptr)
{
	ui->setupUi(this);
	ui->logList->setModel(new QStringListModel(logData));
	ui->logList->setVisible(false);
	
	ui->tableListView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableListView, SIGNAL(customContextMenuRequested(QPoint)),
			this, SLOT(show_table_context_menu(QPoint)));
}

void MainWindow::show_table_context_menu(const QPoint &pos) 
{
	QMenu contextMenu(tr("Context menu"), this);
	
	QAction action_export_blob(tr("Экспорт BLOB"), this);
	connect(&action_export_blob, SIGNAL(triggered()), this, SLOT(export_blob_file()));
	contextMenu.addAction(&action_export_blob);
	
	QAction action_export_xml(tr("Экспорт XML"), this);
	connect(&action_export_xml, SIGNAL(triggered()), this, SLOT(export_xml_file()));
	contextMenu.addAction(&action_export_xml);

	QAction action_import_blob(tr("Импорт BLOB"), this);
	connect(&action_import_blob, SIGNAL(triggered()), this, SLOT(import_blob_file()));
	contextMenu.addAction(&action_import_blob);
	
	QAction action_delete_table(tr("Удалить таблицу"), this);
	connect(&action_delete_table, SIGNAL(triggered()), this, SLOT(delete_table()));
	contextMenu.addAction(&action_delete_table);
	
	contextMenu.exec(mapToGlobal(pos));
}

void MainWindow::delete_table()
{
	auto indexes = ui->tableListView->selectionModel()->selectedIndexes();
	if (indexes.empty()) {
		return;
	}
	std::set<int> done;
	for (auto &index : indexes) {
		if (done.count(index.row()) != 0) {
			continue;
		}
		done.insert(index.row());
		Table *t = db->get_table(index.row());
		qDebug() << QString::fromStdString(t->get_name());
		try {
			db->delete_table(t);
		} catch (std::exception &exc) {
			qDebug() << QString(exc.what());
		}
		db->garbage(true);
	}
}

void MainWindow::export_blob_file()
{
	auto indexes = ui->tableListView->selectionModel()->selectedIndexes();
	if (indexes.empty()) {
		return;
	}
	QString dir = QFileDialog::getExistingDirectory(this);
	if (dir.isNull()) {
		return;
	}
	
	std::set<int> done;
	for (auto &index : indexes) {
		if (done.count(index.row()) != 0) {
			continue;
		}
		done.insert(index.row());
		Table *t = db->get_table(index.row());
		qDebug() << QString::fromStdString(t->get_name());
		try {
			t->export_table(boost::filesystem::path(dir.toStdWString()));
		} catch (std::exception &exc) {
			qDebug() << QString(exc.what());
		}
		db->garbage(true);
	}
}


void MainWindow::export_xml_file()
{
	auto indexes = ui->tableListView->selectionModel()->selectedIndexes();
	if (indexes.empty()) {
		return;
	}
	QString dir = QFileDialog::getExistingDirectory(this);
	if (dir.isNull()) {
		return;
	}

	std::set<int> done;
	for (auto &index : indexes) {
		if (done.count(index.row()) != 0) {
			continue;
		}
		done.insert(index.row());
		Table *t = db->get_table(index.row());
		if (QString::fromStdString(t->get_name()).startsWith("_Enum")) {
			continue;
		}

		t->fill_records_index();
		{
			TableIterator checker(t);
			if (checker.eof()) {
				// нет данных - пропускаем
				// TODO: вынести в настройку
				continue;
			}
		}
		qDebug() << QString::fromStdString(t->get_name());
		boost::filesystem::path filepath = boost::filesystem::path(dir.toStdWString())
				/ t->get_name();
		filepath.replace_extension("xml");
		try {
			if (!t->export_to_xml(filepath.string(), false, true)) {
				if (boost::filesystem::exists(filepath)) {
					boost::filesystem::remove(filepath);
				}
			}
		} catch (std::exception &exc) {
			qDebug() << QString(exc.what());
		}
		db->garbage(true);
	}
}


void MainWindow::import_blob_file()
{
	auto indexes = ui->tableListView->selectionModel()->selectedIndexes();
	if (indexes.empty()) {
		return;
	}
	QString dir = QFileDialog::getExistingDirectory(this);
	if (dir.isNull()) {
		return;
	}
	
	boost::filesystem::path rootpath(dir.toStdWString());
	std::set<int> done;
	for (auto &index : indexes) {
		if (done.count(index.row()) != 0) {
			continue;
		}
		done.insert(index.row());
		Table *t = db->get_table(index.row());
		qDebug() << QString::fromStdString(t->get_name());
		
		try {
			t->import_table(rootpath);
		} catch (std::exception &exc) {
			qDebug() << QString(exc.what());
		}
		db->garbage(true);
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::open(T_1CD *database)
{
	db = database;
	ui->tableListView->setModel(new TablesListModel(db));
	setWindowTitle(QString::fromStdWString(db->get_filepath().wstring()));
	// refresh data
}

void MainWindow::addLogMessage(const QString &message)
{
	logData.append(message);
}

void MainWindow::on_exitAction_triggered()
{
	for (auto w : table_windows) {
		delete w;
	}
	close();
}

void MainWindow::on_openDatabaseFileAction_triggered()
{
	StarterWindow *w = new StarterWindow();
	w->setCache(new Cache());
	w->show();
}

void MainWindow::on_tableListView_doubleClicked(const QModelIndex &index)
{
	Table *t = db->get_table(index.row());
	if (table_windows.find(t) == table_windows.end()) {
		table_windows[t] = new TableDataWindow(this, t);
	}
	table_windows[t]->show();
	table_windows[t]->activateWindow();
}

void MainWindow::on_configurationsButton_clicked()
{
	if (configurationsWindow == nullptr) {
		configurationsWindow = new ConfigurationsWindow(db, this);
	}
	configurationsWindow->show();
	configurationsWindow->activateWindow();
}

void MainWindow::on_tableListView_activated(const QModelIndex &index)
{
	emit ui->tableListView->doubleClicked(index);
}

void MainWindow::on_aboutAction_triggered()
{
	if (about_window == nullptr) {
		about_window = new AboutDialog(this);
	}
	about_window->show();
}
