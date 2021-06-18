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

#include "delete_pages.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QMessageBox>

#include "../gui_utils.h"
#include "../pdf_edit_lib/pdf_editor.h"

DeletePages::DeletePages(QWidget *parent) :
    AbstractOperation(parent)
{
    m_name = tr("Delete pages");
    m_icon = QIcon(":/icons/delete.svg");

    m_pages_selector = new PagesSelector(false, false, this);

    QVBoxLayout *v_layout = new QVBoxLayout();
    this->setLayout(v_layout);

    v_layout->addWidget(m_pages_selector);

    connect(m_pages_selector, &PagesSelector::selection_changed,
            [=]() {emit output_pages_count_changed(output_pages_count());});

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

void DeletePages::set_pdf_info(const PdfInfo &pdf_info)
{
    m_pages_selector->set_num_pages(pdf_info.n_pages());

    AbstractOperation::set_pdf_info(pdf_info);
}

int DeletePages::output_pages_count()
{
    return m_pdf_info.n_pages() -
            m_pages_selector->get_number_of_unique_selected_pages();
}

void DeletePages::save(bool save_as)
{
    if (m_pages_selector->has_error())
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

    std::vector<std::pair<int, int>> delete_intervals =
            m_pages_selector->get_selected_intervals();

    std::vector<bool> keep_pages(m_pdf_info.n_pages(), true);

    std::vector<std::pair<int, int>>::iterator it;
    for (it = delete_intervals.begin(); it != delete_intervals.end(); ++it)
        for (int i = it->first; i <= it->second; i++)
            keep_pages[i] = false;

    bool prev = false;
    int first;
    std::vector<std::pair<int, int>> keep_intervals;
    for (unsigned int i = 0; i < keep_pages.size(); i++)
    {
        if (!prev && keep_pages[i])
            first = i;

        if (!keep_pages[i] && prev)
            keep_intervals.push_back(std::pair<int, int>(first, i - 1));

        if (keep_pages[i] && i == keep_pages.size() - 1)
            keep_intervals.push_back(std::pair<int, int>(first, i));

        prev = keep_pages[i];
    }
    emit progress_changed(20);

    try
    {
        PdfEditor editor;
        unsigned int id = editor.add_file(m_pdf_info.filename());
        editor.add_pages(id, 0, nullptr, keep_intervals);
        emit progress_changed(70);

        editor.write(m_save_filename.toStdString());

        emit write_finished(m_save_filename);
    }
    catch (std::exception &e)
    {
        emit write_error(QString::fromStdString(e.what()));
    }
}
