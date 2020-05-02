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

    QVBoxLayout *v_layout = new QVBoxLayout();
    this->setLayout(v_layout);

    m_pages_selector = new PagesSelector(parent=this);
    v_layout->addWidget(m_pages_selector);

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

#ifdef FLATPAK_BUILD
    base_name_label->setEnabled(false);
    m_base_name.setEnabled(false);
    label->setEnabled(false);
    extract_individual_button->setEnabled(false);
#endif

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

void ExtractPages::pdf_info_changed()
{
    AbstractOperation::pdf_info_changed();

    QFileInfo file_info(QString::fromStdString(m_pdf_info->filename()));

    m_base_name.setText(file_info.completeBaseName());
}

void ExtractPages::extract_to_individual()
{
    QString selection = m_pages_selector->get_selection_as_text(
                m_pdf_info->n_pages());

    if (selection.isNull())
        return;

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
        parse_output_pages_string(selection.toStdString(),
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
    QString selection = m_pages_selector->get_selection_as_text(
                m_pdf_info->n_pages());

    if (selection.isNull())
        return;

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
        fileconf.ouput_pages = selection.toStdString();
        fileconf.multipage_enabled = false;
        fileconf.rotation = 0;
        fileconf.outline_entry = "";

        conf.files.push_back(fileconf);

        write_pdf(conf, progress);

        emit write_finished(m_save_filename);
    }
}
