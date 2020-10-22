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

#include "booklet.h"

#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>

#include "../gui_utils.h"
#include "../pdf_edit_lib/pdf_writer.h"

Booklet::Booklet(const PdfInfo &pdf_info,
                 QProgressBar *progress_bar,
                 QWidget *parent) :
AbstractOperation(pdf_info, progress_bar, parent)
{
    m_name = tr("Booklet");

    QVBoxLayout *v_layout = new QVBoxLayout();
    QFormLayout *form_layout = new QFormLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(form_layout, 1);
    v_layout->addLayout(h_layout);
    this->setLayout(v_layout);

    m_binding.addItem(tr("Left"));
    m_binding.addItem(tr("Right"));
    form_layout->addRow(tr("Binding:"), &m_binding);
    form_layout->addRow(tr("Use last page as back cover:"), &m_back_cover);
    form_layout->setAlignment(&m_back_cover, Qt::AlignVCenter);

    h_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Minimum));
    QPushButton *button = new QPushButton(QIcon::fromTheme("document-save-as"),
                                          tr("Generate booklet"), this);
    button->setShortcut(QKeySequence::Save);
    button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    button->text(),
                    button->shortcut().toString()));
    connect(button, &QPushButton::pressed,
            this, &Booklet::generate_booklet);
    h_layout->addWidget(button);
}

void Booklet::generate_booklet()
{
    QString m_save_filename = QFileDialog::getSaveFileName(
                this,
                tr("Save booklet PDF file"),
                settings->value("save_directory",
                                  settings->value("open_directory", "")
                                  ).toString(),
                tr("PDF files (*.pdf)"));

    if (!m_save_filename.isNull())
    {
        emit write_started();

        settings->setValue(
                    "save_directory",
                    QFileInfo(m_save_filename).dir().absolutePath());

        QProgressBar *pb = m_progress_bar;
        std::function<void (int)> progress = [pb] (int p)
        {
            pb->setValue(p);
        };

        m_progress_bar->setValue(0);
        m_progress_bar->show();

        write_booklet_pdf(m_pdf_info->filename(),
                          m_save_filename.toStdString(),
                          m_binding.currentIndex(),
                          m_back_cover.isChecked(),
                          progress);

        emit write_finished(m_save_filename);
    }
}
