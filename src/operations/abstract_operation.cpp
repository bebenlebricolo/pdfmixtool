/* Copyright (C) 2020-2021 Marco Scarpetta
 *
 * This file is part of PDF Mix Tool.
 *
 * PDF Mix Tool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PDF Mix Tool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PDF Mix Tool. If not, see <http://www.gnu.org/licenses/>.
 */

#include "abstract_operation.h"

#include <atomic>
#include <thread>
#include <chrono>

#include <QFileInfo>
#include <QMessageBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QApplication>

#include "../gui_utils.h"

AbstractOperation::AbstractOperation(QWidget *parent) :
    QWidget(parent),
    m_is_single_file_operation{true},
    m_active{false}
{
    m_icon = QIcon(":/icons/icon.svg");

    m_save_button.setIcon(QIcon::fromTheme("document-save"));
    m_save_button.setText(tr("Save"));
    m_save_button.setShortcut(QKeySequence::Save);
    m_save_button.setToolTip(
                QString(TOOLTIP_STRING).arg(
                    m_save_button.text(),
                    m_save_button.shortcut().toString()));
}

const QString &AbstractOperation::name()
{
    return m_name;
}

const QIcon &AbstractOperation::icon()
{
    return m_icon;
}

bool AbstractOperation::is_single_file_operation()
{
    return m_is_single_file_operation;
}

void AbstractOperation::set_active(bool active)
{
    m_active = active;
}

void AbstractOperation::set_pdf_info(const PdfInfo &pdf_info)
{
    m_pdf_info = pdf_info;
    QFileInfo info(QString::fromStdString(m_pdf_info.filename()));
    m_save_button.setEnabled(info.isWritable());

    emit output_pages_count_changed(output_pages_count());
}

int AbstractOperation::output_pages_count()
{
    return 0;
}

void AbstractOperation::update_multipage_profiles()
{

}

void AbstractOperation::profile_created(int index)
{

}

bool AbstractOperation::show_overwrite_dialog()
{
    m_save_filename = QString::fromStdString(m_pdf_info.filename());

    QString filename = QFileInfo(m_save_filename).fileName();

    if (settings->value("show_overwrite_warning", true).toBool())
    {
        QMessageBox warning(QMessageBox::Warning,
                            tr("Overwrite File?"),
                            tr("A file called «%1» already exists. "
                               "Do you want to overwrite it?")
                            .arg(filename),
                            QMessageBox::Yes | QMessageBox::No);
        warning.setDefaultButton(QMessageBox::No);

        warning.setCheckBox(new QCheckBox(tr("Always overwrite")));

        int button = warning.exec();

        if (warning.checkBox()->isChecked())
            settings->setValue("show_overwrite_warning", false);

        if (button == QMessageBox::No)
            return false;
    }

    return true;
}

bool AbstractOperation::show_save_as_dialog()
{
    QString selected_file = QFileDialog::getSaveFileName(
                this,
                tr("Save PDF file"),
                settings->value("save_directory",
                                  settings->value("open_directory", "")
                                  ).toString(),
                tr("PDF files") + " (*.pdf *.PDF)");

    if (!selected_file.isNull())
    {
        m_save_filename = selected_file;

        settings->setValue(
                    "save_directory",
                    QFileInfo(selected_file).dir().absolutePath());

        return true;
    }

    return false;
}
