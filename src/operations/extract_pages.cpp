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

#include "extract_pages.h"

#include <QGridLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QFileInfo>
#include <QFileDialog>

#include "../gui_utils.h"
#include "../pdf_edit_lib/pdf_editor.h"

ExtractPages::ExtractPages(QWidget *parent) :
      AbstractOperation(parent)
{
    m_name = tr("Extract pages");
    m_icon = QIcon(":/icons/extract.svg");

    QVBoxLayout *v_layout = new QVBoxLayout();
    this->setLayout(v_layout);

    m_pages_selector = new PagesSelector(true, true, this);
    v_layout->addWidget(m_pages_selector);

    connect(m_pages_selector, &PagesSelector::selection_changed,
            [=]() {emit output_pages_count_changed(output_pages_count());});

    // spacer
    v_layout->addWidget(new QWidget(this), 1);

    QLabel *label = new QLabel("<b>" +
                               tr("Extract to individual PDF files") +
                               "</b>", this);
    v_layout->addWidget(label);

    QLabel *base_name_label = new QLabel(tr("Output PDF base name:"), this);
    QHBoxLayout *h_layout = new QHBoxLayout();
    h_layout->addWidget(base_name_label);
    h_layout->addWidget(&m_base_name, 1);
    m_base_name.setClearButtonEnabled(true);
    v_layout->addLayout(h_layout);

    QPushButton *extract_individual_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Extract…"),
                this);
    connect(extract_individual_button, &QPushButton::pressed,
            this, &ExtractPages::extract_to_individual);

    h_layout = new QHBoxLayout();
    h_layout->addWidget(new QWidget(this), 1);
    h_layout->addWidget(extract_individual_button);
    v_layout->addLayout(h_layout);

    label = new QLabel("<b>" + tr("Extract to single PDF") + "</b>", this);
    v_layout->addWidget(label);

    QPushButton *extract_single_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Extract…"),
                this);
    connect(extract_single_button, &QPushButton::pressed,
            this, &ExtractPages::extract_to_single);

    h_layout = new QHBoxLayout();
    h_layout->addWidget(new QWidget(this), 1);
    h_layout->addWidget(extract_single_button);
    v_layout->addLayout(h_layout);
}

void ExtractPages::set_pdf_info(const PdfInfo &pdf_info)
{
    m_pages_selector->set_num_pages(pdf_info.n_pages());

    AbstractOperation::set_pdf_info(pdf_info);

    QFileInfo file_info(QString::fromStdString(m_pdf_info.filename()));

    m_base_name.setText(file_info.completeBaseName());
}

int ExtractPages::output_pages_count()
{
    return m_pages_selector->get_number_of_selected_pages();
}

void ExtractPages::extract_to_individual()
{
    if (m_pages_selector->has_error())
        return;

    std::vector<std::pair<int, int>> intervals =
            m_pages_selector->get_selected_intervals();

    QString dir_name = QFileDialog::getExistingDirectory(
                this,
                tr("Select save directory"),
                settings->value("save_directory",
                                  settings->value("open_directory", "")
                                  ).toString(),
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks);

    if (!dir_name.isNull())
    {
        emit write_started();

        settings->setValue(
                    "save_directory",
                    QFileInfo(dir_name).dir().absolutePath());

        QDir dir(dir_name);
        QString base_name = m_base_name.text();

        try
        {
            int  j{0};
            std::vector<std::pair<int, int>>::iterator it;
            for (it = intervals.begin(); it != intervals.end(); ++it)
            {
                ++j;
                for (int i = it->first; i <= it->second; i++)
                {
                    QString filename = base_name + QString("_%1.pdf").arg(i + 1);

                    // FIXME SLOW! A custom function that opens the input file once may be necessary
                    PdfEditor editor;
                    unsigned int id = editor.add_file(m_pdf_info.filename());
                    editor.add_pages(id, 0, nullptr, {{i, i}});
                    editor.write(dir.filePath(filename).toStdString());

                    emit progress_changed(100. * j / intervals.size());
                }
            }

            emit write_finished(dir_name);
        }
        catch (std::exception &e)
        {
            emit write_error(QString::fromStdString(e.what()));
        }
    }
}

void ExtractPages::extract_to_single()
{
    if (m_pages_selector->has_error())
        return;

    std::vector<std::pair<int, int>> intervals =
            m_pages_selector->get_selected_intervals();

    m_save_filename = QFileDialog::getSaveFileName(
                this,
                tr("Extract to single PDF"),
                settings->value("save_directory",
                                  settings->value("open_directory", "")
                                  ).toString(),
                tr("PDF files") + " (*.pdf *.PDF)");

    if (!m_save_filename.isNull())
    {
        emit write_started();

        settings->setValue(
                    "save_directory",
                    QFileInfo(m_save_filename).dir().absolutePath());

        emit progress_changed(20);

        try
        {
            PdfEditor editor;
            unsigned int id = editor.add_file(m_pdf_info.filename());
            editor.add_pages(id, 0, nullptr, intervals);
            emit progress_changed(70);

            editor.write(m_save_filename.toStdString());

            emit write_finished(m_save_filename);
        }
        catch (std::exception &e)
        {
            emit write_error(QString::fromStdString(e.what()));
        }

    }
}
