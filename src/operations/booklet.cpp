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

#include "booklet.h"

#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>

#include "../gui_utils.h"
#include "../pdf_edit_lib/pdf_editor.h"

Booklet::Booklet(QWidget *parent) :
    AbstractOperation(parent)
{
    m_name = tr("Booklet");
    m_icon = QIcon(":/icons/booklet.svg");

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

int Booklet::output_pages_count()
{
    return ceil(m_pdf_info.n_pages() / 4.) * 2;
}

void Booklet::generate_booklet()
{
    QString m_save_filename = QFileDialog::getSaveFileName(
                this,
                tr("Save booklet PDF file"),
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

        std::vector<std::pair<int, int>> intervals;

        // define output pages layout
        PdfEditor::PageLayout *layout = new PdfEditor::PageLayout();
        layout->width = 2 * m_pdf_info.width() * cm;
        layout->height = m_pdf_info.height() * cm;

        PdfEditor::Page page1;
        page1.x = 0;
        page1.y = 0;
        page1.width = m_pdf_info.width() * cm;
        page1.height = m_pdf_info.height() * cm;

        PdfEditor::Page page2 = page1;
        page2.x = m_pdf_info.width() * cm;

        layout->pages.push_back(page1);
        layout->pages.push_back(page2);

        // compute vector of indices of pages in the output file
        int num_pages = m_pdf_info.n_pages() % 4 == 0 ?
                    m_pdf_info.n_pages() : (m_pdf_info.n_pages() / 4 + 1) * 4;

        int i = 0;
        int j = num_pages - 1;

        bool is_right_page = !m_binding.currentIndex();

        while (j > i)
        {
            std::vector<int> couple;
            couple.push_back(i);
            couple.push_back(j);

            intervals.push_back(std::pair<int, int>(-1, -1));
            intervals.push_back(std::pair<int, int>(-1, -1));

            for (int current_page : couple)
            {
                if (m_back_cover.isChecked())
                {
                    if (current_page == num_pages - 1)
                        current_page = m_pdf_info.n_pages() - 1;
                    else if (current_page == m_pdf_info.n_pages() - 1)
                        current_page = num_pages - 1;
                }

                if (current_page >= m_pdf_info.n_pages())
                {
                    is_right_page = !is_right_page;
                    continue;
                }

                if (is_right_page)
                    intervals.end()[-1] = {current_page, current_page};
                else
                    intervals.end()[-2] = {current_page, current_page};

                is_right_page = !is_right_page;
            }

            is_right_page = !is_right_page; // revert last one

            i++;
            j--;
        }

        try
        {
            PdfEditor editor;
            unsigned int id = editor.add_file(m_pdf_info.filename());

            emit progress_changed(20);
            editor.add_pages(id, 0, layout, intervals);
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
