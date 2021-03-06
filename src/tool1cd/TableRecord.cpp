/*
    Tool1CD library provides access to 1CD database files.
    Copyright © 2009-2017 awa
    Copyright © 2017-2018 E8 Tools contributors

    This file is part of Tool1CD Library.

    Tool1CD Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Tool1CD Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Tool1CD Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "TableRecord.h"
#include "Field.h"
#include "Table.h"
#include "TempStream.h"
#include "UZLib.h"

NullValueException::NullValueException(const Field *field)
	: DetailedException("Запрошенное значение равно NULL")
{
	add_detail("Поле", field->get_name());
	add_detail("Таблица", field->get_parent()->get_name());
}

FieldCannotBeNullException::FieldCannotBeNullException(const Field *field)
	: DetailedException("Поле не может быть NULL")
{
	add_detail("Поле", field->get_name());
	add_detail("Таблица", field->get_parent()->get_name());
}

TableRecord::TableRecord(const Table *parent, char *data, int data_size)
	: data(data == nullptr ? new char [parent->get_recordlen()] : data),
	  table(parent),
	  data_size(data_size == -1 ? parent->get_recordlen() : data_size)
{

}

TableRecord::TableRecord(const TableRecord &another)
: data(new char [another.get_table()->get_recordlen()]),
  table(another.get_table()),
  data_size(another.data_size)
{
	auto i = data_size;
	while (i--) {
		data[i] = another.data[i];
	}
}

TableRecord::~TableRecord()
{
	if (data) {
		// delete[] data; // TODO: утечка данных
		data = nullptr;
	}
}

std::string TableRecord::get_string(const Field *field) const
{
	if (is_null_value(field)) {
		throw NullValueException(field);
	}
	return field->get_presentation(data);
}

std::string TableRecord::get_string(const std::string &field_name) const
{
	return get_string(table->get_field(field_name));
}

bool TableRecord::is_null_value(const Field *field) const
{
	if (!field->get_null_exists()) {
		return false;
	}
	return data[field->get_offset()] == '\0';
}

bool TableRecord::is_null_value(const std::string &field_name) const
{
	return is_null_value(table->get_field(field_name));
}

bool TableRecord::is_removed() const
{
	return data[0] != '\0';
}

const char *TableRecord::get_raw(const Field *field) const
{
	return &data[field->get_offset()];
}

const char *TableRecord::get_raw(const std::string &field_name) const
{
	return get_raw(table->get_field(field_name));
}


const char *TableRecord::get_data(const Field *field) const
{
	return &data[field->get_offset() + (field->get_null_exists() ? 1 : 0)];
}

char *TableRecord::__get_data(const Field *field)
{
	return &data[field->get_offset() + (field->get_null_exists() ? 1 : 0)];
}


const char *TableRecord::get_data(const std::string &field_name) const
{
	return get_data(table->get_field(field_name));
}

void TableRecord::Assign(const TableRecord *another_record)
{
	if (table != nullptr) {
		if (another_record->data_size != data_size || another_record->table != table) {
			throw DetailedException("Попытка передать данные между записями разных таблиц!")
					.add_detail("Таблица-приёмник", table->get_name())
					.add_detail("Таблица-источник", another_record->table->get_name());
		}
	}
	auto i = data_size;
	while (i--) {
		data[i] = another_record->data[i];
	}
}

void TableRecord::set_null(const Field *field)
{
	if (!field->get_null_exists()) {
		throw FieldCannotBeNullException(field);
	}
	data[field->get_offset()] = '\0';
}

void TableRecord::set_data(const Field *field, const void *new_data)
{
	char *data_start = &data[field->get_offset()];
	if (field->get_null_exists()) {
		data_start[0] = '\001';
		data_start++;
	}
	memcpy(data_start, new_data, field->get_size());
}

std::string TableRecord::get_xml_string(const Field *field) const
{
	if (is_null_value(field)) {
		throw NullValueException(field);
	}
	return field->get_XML_presentation(data);
}

std::string TableRecord::get_xml_string(const std::string &field_name) const
{
	return get_xml_string(table->get_field(field_name));
}

bool TableRecord::try_store_blob_data(const Field *field, TStream *&out, bool inflate_stream) const
{
	if (is_null_value(field)) {
		return false;
	}
	auto b = get<table_blob_file>(field);
	if (b.blob_start == 0 && b.blob_length == 0) {
		return false;
	}

	out = new TTempStream;
	if (inflate_stream) {
		TMemoryStream in;
		table->readBlob(&in, b.blob_start, b.blob_length);
		in.Seek(0, soFromBeginning);
		try {
			ZInflateStream(&in, out);
		} catch (ZError) {
			out->CopyFrom(&in, 0);
		}
	}
	else {
		table->readBlob(out, b.blob_start, b.blob_length);
	}
	out->Close();
	return true;
}

const Field *TableRecord::get_field(const std::string &field_name) const
{
	return table->get_field(field_name);
}

void RecordConverters::convert(const char *data, BinaryGuid &result)
{
	result = BinaryGuid(data);
}
