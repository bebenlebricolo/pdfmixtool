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

#include "delete_pages.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QMessageBox>

#include "../gui_utils.h"
#include "../pdf_edit_lib/pdf_writer.h"

DeletePages::DeletePages(const PdfInfo &pdf_info,
                         QProgressBar *progress_bar,
                         QWidget *parent) :
    AbstractOperation(pdf_info, progress_bar, parent)
{
    m_name = tr("Delete pages");

    m_pages_selector = new PagesSelector(false, false, this);

    QVBoxLayout *v_layout = new QVBoxLayout();
    this->setLayout(v_layout);

    v_layout->addWidget(m_pages_selector);

    // spacer
    v_layout->addWidget(new QWidget(this), 1);

    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(h_layout);

    // spacer
    h_layout->addWidget(new QWidget(this), 1);

    QPushButton *save_as_button = new QPushButton(
                QIcon::fromTheme("document-save-as"),
                tr("Save as…"),
                this);
    save_as_button->setShortcut(QKeySequence::SaveAs);
    save_as_button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    save_as_button->text(),
                    save_as_button->shortcut().toString()));

    h_layout->addWidget(&m_save_button);
    h_layout->addWidget(save_as_button);

    connect(&m_save_button, &QPushButton::pressed,
            [=]() {save(false);});
    connect(save_as_button, &QPushButton::pressed,
            [=]() {save(true);});
}

void DeletePages::save(bool save_as)
{
    QString selection = m_pages_selector->get_selection_as_text(
                m_pdf_info->n_pages());

    if (selection.isNull())
        return;

    if (save_as)
    {
        if (!show_save_as_dialog())
            return;
    }
    else
    {
        if (!show_overwrite_dialog())
            return;
    }

    emit write_started();

    std::vector<bool> pages;
    for (int i = 0; i < m_pdf_info->n_pages(); i++)
        pages.push_back(false);

    int output_pages_count;
    std::vector<std::pair<int, int>> intervals;

    parse_output_pages_string(selection.toStdString(),
                              m_pdf_info->n_pages(),
                              intervals,
                              output_pages_count);

    std::vector<std::pair<int, int>>::iterator it;
    for (it = intervals.begin(); it != intervals.end(); ++it)
        for (int i = it->first - 1; i < it->second; i++)
            pages[i] = true;

    QProgressBar *pb = m_progress_bar;
    std::function<void (int)> progress = [pb] (int p)
    {
        pb->setValue(p);
    };

    write_delete_pages(m_pdf_info->filename(),
                       m_save_filename.toStdString(),
                       pages,
                       progress);

    emit write_finished(m_save_filename);
}
