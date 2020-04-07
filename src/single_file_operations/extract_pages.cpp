/* Copyright (C) 2020 Marco Scarpetta
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
#include "../pdf_edit_lib/pdf_writer.h"

ExtractPages::ExtractPages(const PdfInfo &pdf_info,
                           QProgressBar *progress_bar,
                           QWidget *parent) :
      AbstractOperation(pdf_info, progress_bar, parent)
{
    m_name = tr("Extract pages");

    QGridLayout *grid_layout = new QGridLayout();
    grid_layout->setColumnStretch(0, 0);
    grid_layout->setColumnStretch(1, 1);
    this->setLayout(grid_layout);

    m_extraction_type.addButton(
                new QRadioButton(tr("Extract pages:"), this), 0);
    grid_layout->addWidget(m_extraction_type.button(0), 0, 0);
    m_extraction_type.addButton(
                new QRadioButton(tr("Extract all pages"), this), 1);
    grid_layout->addWidget(m_extraction_type.button(1), 1, 0);
    m_extraction_type.addButton(
                new QRadioButton(tr("Extract even pages"), this), 2);
    grid_layout->addWidget(m_extraction_type.button(2), 2, 0);
    m_extraction_type.addButton(
                new QRadioButton(tr("Extract odd pages"), this), 3);
    grid_layout->addWidget(m_extraction_type.button(3), 3, 0);
    m_extraction_type.button(0)->setChecked(true);

    grid_layout->addWidget(&m_selection, 0, 1);
    m_selection.setClearButtonEnabled(true);

    grid_layout->addItem(new QSpacerItem(0, 0,
                                         QSizePolicy::Minimum,
                                         QSizePolicy::Expanding), 4, 0, 1, 2);

    QLabel *label = new QLabel("<b>" +
                               tr("Extract to individual PDF files") +
                               "</b>", this);
    grid_layout->addWidget(label, 5, 0, 1, 2);

    QLabel *base_name_label = new QLabel(tr("Output PDF base name:"), this);
    grid_layout->addWidget(base_name_label, 6, 0);
    grid_layout->addWidget(&m_base_name, 6, 1);
    m_selection.setClearButtonEnabled(true);

    QPushButton *extract_individual_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Extract…"),
                this);
    connect(extract_individual_button, &QPushButton::pressed,
            [=]() {if (check_selection()) extract_to_individual();});

    QHBoxLayout *h_layout = new QHBoxLayout();
    h_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Minimum));
    h_layout->addWidget(extract_individual_button);
    grid_layout->addLayout(h_layout, 7, 0, 1, 2);

#ifdef FLATPAK_BUILD
    base_name_label->setEnabled(false);
    m_base_name.setEnabled(false);
    label->setEnabled(false);
    extract_individual_button->setEnabled(false);
#endif

    label = new QLabel("<b>" + tr("Extract to single PDF") + "</b>", this);
    grid_layout->addWidget(label, 8, 0, 1, 2);

    QPushButton *extract_single_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Extract…"),
                this);
    connect(extract_single_button, &QPushButton::pressed,
            [=]() {if (check_selection()) extract_to_single();});

    h_layout = new QHBoxLayout();
    h_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Minimum));
    h_layout->addWidget(extract_single_button);
    grid_layout->addLayout(h_layout, 9, 0, 1, 2);
}

void ExtractPages::pdf_info_changed()
{
    AbstractOperation::pdf_info_changed();

    QFileInfo file_info(QString::fromStdString(m_pdf_info->filename()));

    m_base_name.setText(file_info.completeBaseName());
}

bool ExtractPages::check_selection()
{
    if (m_extraction_type.checkedId() > 0)
        return true;

    int output_pages_count;
    std::vector<std::pair<int, int>> intervals;
    if (m_selection.text().toStdString().empty() ||
            !parse_output_pages_string(m_selection.text().toStdString(),
                                       m_pdf_info->n_pages(),
                                       intervals,
                                       output_pages_count))
    {
        QString error_message(
                    tr("<p>Pages to be extracted are badly formatted. "
                       "Please make sure you complied with the following "
                       "rules:</p><ul>"
                       "<li>intervals of pages must be written indicating the "
                       "first page and the last page separated by a dash "
                       "(e.g. \"1-5\");</li>"
                       "<li>single pages and intervals of pages must be "
                       "separated by spaces, commas or both "
                       "(e.g. \"1, 2, 3, 5-10\" or \"1 2 3 5-10\");</li>"
                       "<li>all pages and intervals of pages must be between "
                       "1 and the number of pages of the PDF file;</li>"
                       "<li>only numbers, spaces, commas and dashes can be "
                       "used. All other characters are not allowed.</li>"
                       "</ul>"));
        QMessageBox::critical(this,
                              tr("Error"),
                              error_message);

        return false;
    }

    return true;
}

QString ExtractPages::get_selection()
{
    QString s;

    switch (m_extraction_type.checkedId()) {
    case 0: {
        return m_selection.text();
    }
    case 1: {
        return "";
    }
    case 2: {
        for (int i = 1; i <= m_pdf_info->n_pages(); i++)
            if (i % 2 == 0)
                s += QString::number(i) + ",";
        break;
    }
    case 3: {
        for (int i = 1; i <= m_pdf_info->n_pages(); i++)
            if (i % 2 == 1)
                s += QString::number(i) + ",";
        break;
    }
    }

    return s;
}

void ExtractPages::extract_to_individual()
{
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

#ifdef FLATPAK_BUILD
        settings->setValue("save_directory", "");
#else
        settings->setValue(
                    "save_directory",
                    QFileInfo(dir_name).dir().absolutePath());
#endif

        QProgressBar *pb = m_progress_bar;
        std::function<void (int)> progress = [pb] (int p)
        {
            pb->setValue(p);
        };

        m_progress_bar->setValue(0);
        m_progress_bar->show();

        QDir dir(dir_name);
        QString base_name = m_base_name.text();

        int output_pages_count;
        std::vector<std::pair<int, int>> intervals;
        parse_output_pages_string(
                    get_selection().toStdString(),
                    m_pdf_info->n_pages(),
                    intervals,
                    output_pages_count);

        std::vector<std::pair<int, int>>::iterator it;
        for (it = intervals.begin(); it != intervals.end(); ++it)
        {
            for (int i = it->first; i <= it->second; i++)
            {
                QString filename = base_name + QString("_%1.pdf").arg(i);

                Conf conf;

                conf.output_path = dir.filePath(filename).toStdString();

                FileConf fileconf;
                fileconf.path = m_pdf_info->filename();
                fileconf.ouput_pages = std::to_string(i);
                fileconf.multipage_enabled = false;
                fileconf.rotation = 0;
                fileconf.outline_entry = "";

                conf.files.push_back(fileconf);

                write_pdf(conf, progress);
            }
        }

        emit write_finished(dir_name);
    }
}

void ExtractPages::extract_to_single()
{
    m_save_filename = QFileDialog::getSaveFileName(
                this,
                tr("Extract to single PDF"),
                settings->value("save_directory",
                                  settings->value("open_directory", "")
                                  ).toString(),
                tr("PDF files (*.pdf)"));

    if (!m_save_filename.isNull())
    {
        emit write_started();

#ifdef FLATPAK_BUILD
        settings->setValue("save_directory", "");
#else
        settings->setValue(
                    "save_directory",
                    QFileInfo(m_save_filename).dir().absolutePath());
#endif

        QProgressBar *pb = m_progress_bar;
        std::function<void (int)> progress = [pb] (int p)
        {
            pb->setValue(p);
        };

        m_progress_bar->setValue(0);
        m_progress_bar->show();

        Conf conf;

        conf.output_path = m_save_filename.toStdString();

        FileConf fileconf;
        fileconf.path = m_pdf_info->filename();
        fileconf.ouput_pages = get_selection().toStdString();
        fileconf.multipage_enabled = false;
        fileconf.rotation = 0;
        fileconf.outline_entry = "";

        conf.files.push_back(fileconf);

        write_pdf(conf, progress);

        emit write_finished(m_save_filename);
    }
}
