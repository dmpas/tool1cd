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
#include "table_data_window.h"
#include "ui_table_data_window.h"
#include "table_fields_window.h"
#include <Table.h>
#include "skobkatextwindow.h"
#include "models/table_data_model.h"
#include "export_table_to_xml_dialog.h"
#include <QDebug>
#include <QItemSelection>
#include "models/skobka_tree_model.h"
#include <QFileDialog>
#include "models/v8catalog_tree_model.h"

QString index_presentation(Index *index)
{
	QString presentation = QString::fromStdString(index->get_name());
	presentation += QString(": ");
	for (int i = 0; i < index->get_num_records(); i++) {
		if (i != 0) {
			presentation += QString(", ");
		}
		presentation += QString::fromStdString(index->get_records()[i].field->get_name());
	}
	return presentation;
}

TableDataWindow::TableDataWindow(QWidget *parent, Table *table)
    : QMainWindow(parent), table(table),
      ui(new Ui::TableDataWindow),
      tableWindow(nullptr)
{
	ui->setupUi(this);
	setWindowTitle(QString::fromStdString(table->get_name()));
	ui->dataView->setModel(new TableDataModel(table));

	QVector<Index*> indexes;
	indexes.push_back(nullptr); // PK

	for (int i = 0; i < table->get_num_indexes(); i++) {
		Index *index = table->get_index(i);
		if (index->is_primary() && indexes[0] == nullptr) {
			indexes[0] = index;
		} else {
			indexes.push_back(index);
		}
	}

	ui->indexChooseBox->addItem(tr("<без индекса>"));
	for (Index *index : indexes) {
		if (index == nullptr) {
			continue;
		}
		ui->indexChooseBox->addItem(index_presentation(index), QString::fromStdString(index->get_name()));
	}

	connect(ui->dataView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	        this, SLOT(dataView_selection_changed(QItemSelection)));

	if (indexes.size() > 1) {
		ui->indexChooseBox->setCurrentIndex(1);
		emit ui->indexChooseBox->activated(1);
	}

	ui->dataView->setFocus();
}

TableDataWindow::~TableDataWindow()
{

}


void TableDataWindow::on_descriptionButton_clicked()
{
	SkobkaTextWindow *w = new SkobkaTextWindow(this);
	w->setText(QString::fromStdString(table->get_description()),
	           QString::fromStdString(table->get_name()));
	w->show();
}

void TableDataWindow::on_fieldsButton_clicked()
{
	if (tableWindow == nullptr) {
		tableWindow = new TableFieldsWindow(this, table);
	}
	tableWindow->show();
	tableWindow->activateWindow();
}

void TableDataWindow::on_exportToXmlButton_clicked()
{
	ExportTableToXmlDialog *export_dialog = new ExportTableToXmlDialog(table, this);
	export_dialog->show_with_file_dialog();
}

void TableDataWindow::on_indexChooseBox_activated(int index)
{
	if (index == 0) {
		// без индекса
		ui->dataView->setModel(new TableDataModel(table));
		return;
	}
	QString index_name = ui->indexChooseBox->itemData(index).toString();
	Index *table_index = table->get_index(index_name.toStdString());
	ui->dataView->setModel(new TableDataModel(table, table_index));

	connect(ui->dataView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	        this, SLOT(dataView_selection_changed(QItemSelection)));
}

void TableDataWindow::on_dataView_activated(const QModelIndex &index)
{
}

std::unique_ptr<Tree> try_parse_tree(const QVariant &data)
{
	std::string string_data = data.toString().toStdString();
	if (string_data.substr(0, 1) != "{") {
		return nullptr;
	}
	try {
		auto data_tree = parse_1Ctext(string_data, "");
		return data_tree;
	} catch (...) {
		return nullptr;
	}
}

void TableDataWindow::dataView_selection_changed(const QItemSelection &selection)
{
	if (selection.empty()) {
		return;
	}
	auto range = selection.at(0);
	auto column = range.left();
	auto row = range.top();
	auto index = ui->dataView->model()->index(row, column);
	auto model = static_cast<TableDataModel*>(ui->dataView->model());

	auto data = ui->dataView->model()->data(index, Qt::EditRole);

	if (model->isBlobValue(index)) {

		QString catalog_name("{catalog}");

		if (table->find_field("FILENAME") != nullptr) {

			auto rec = model->getRecord(index);
			catalog_name = QString::fromStdString(rec->get_string("FILENAME"));

		} else if (table->find_field("EXTNAME") != nullptr) {

			auto rec = model->getRecord(index);
			catalog_name = QString::fromStdString(rec->get_string("EXTNAME"));

		}

		auto stream = model->getBlobStream(index);

		if (stream != nullptr) {
			ui->dataWidget->setStream(stream, catalog_name);
			return;
		}
	}

	ui->dataWidget->setText(data.toString());
}

void TableDataWindow::on_saveBlobButton_clicked()
{
	auto index = ui->dataView->currentIndex();
	if (!index.isValid()) {
		return;
	}
	QString filename = QFileDialog::getSaveFileName(this, tr("Сохранение..."), "");
	if (filename.isNull()) {
		return;
	}
	auto model = static_cast<TableDataModel*>(ui->dataView->model());
	model->dumpBlob(index, filename);
}
