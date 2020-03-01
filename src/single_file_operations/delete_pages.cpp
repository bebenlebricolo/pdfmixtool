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

DeletePages::DeletePages(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *v_layout = new QVBoxLayout();
    QGridLayout *grid_layout = new QGridLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(grid_layout);
    v_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Minimum,
                                      QSizePolicy::Expanding));
    v_layout->addLayout(h_layout);
    this->setLayout(v_layout);

    m_selection_type.addButton(new QRadioButton(tr("Delete pages:"), this), 0);
    grid_layout->addWidget(m_selection_type.button(0), 0, 0);
    m_selection_type.addButton(new QRadioButton(tr("Delete even pages"), this), 1);
    grid_layout->addWidget(m_selection_type.button(1), 1, 0);
    m_selection_type.addButton(new QRadioButton(tr("Delete odd pages"), this), 2);
    grid_layout->addWidget(m_selection_type.button(2), 2, 0);
    m_selection_type.button(0)->setChecked(true);

    grid_layout->addWidget(&m_selection, 0, 1);
    m_selection.setClearButtonEnabled(true);

    grid_layout->setColumnStretch(0, 0);
    grid_layout->setColumnStretch(1, 1);

    h_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Minimum));

    QPushButton *save_button = new QPushButton(
                QIcon::fromTheme("document-save"),
                tr("Save"),
                this);
    save_button->setShortcut(QKeySequence::Save);
    save_button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    save_button->text(),
                    save_button->shortcut().toString()));

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

    h_layout->addWidget(save_button);
    h_layout->addWidget(save_as_button);

    connect(save_button, &QPushButton::pressed,
            [=]() {if (check_selection()) save_button_pressed();});
    connect(save_as_button, &QPushButton::pressed,
            [=]() {if (check_selection()) save_as_button_pressed();});
}

void DeletePages::set_num_pages(int num_pages)
{
    m_num_pages = num_pages;
}

QString DeletePages::get_selection()
{
    QString s;

    switch (m_selection_type.checkedId()) {
    case 0: {
        int output_pages_count;
        std::vector<std::pair<int, int>> intervals;

        parse_output_pages_string(m_selection.text().toStdString(),
                                  m_num_pages,
                                  intervals,
                                  output_pages_count);

        for (int i = 1; i <= m_num_pages; i++)
        {
            std::vector<std::pair<int, int>>::iterator it;
            for (it = intervals.begin(); it != intervals.end(); ++it)
                if (i >= it->first && i <= it->second)
                    goto cnt;

            s += QString::number(i) + ",";

            cnt:;
        }
        break;
    }
    case 1:{
        for (int i = 1; i <= m_num_pages; i++)
            if (i % 2 == 1)
                s += QString::number(i) + ",";
        break;
    }
    case 2:{
        for (int i = 1; i <= m_num_pages; i++)
            if (i % 2 == 0)
                s += QString::number(i) + ",";
        break;
    }
    }

    return s;
}

bool DeletePages::check_selection()
{
    if (m_selection_type.checkedId() > 0)
        return true;

    int output_pages_count;
    std::vector<std::pair<int, int>> intervals;
    if (m_selection.text().toStdString().empty() ||
            !parse_output_pages_string(m_selection.text().toStdString(),
                                       m_num_pages,
                                       intervals,
                                       output_pages_count))
    {
        QString error_message(
                    tr("<p>Pages to be deleted are badly formatted. "
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
