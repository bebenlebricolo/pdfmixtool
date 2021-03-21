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

#include "pages_selector.h"

#include <QGridLayout>
#include <QRadioButton>
#include <QMessageBox>

#include "../pdf_edit_lib/definitions.h"

LineEdit::LineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    this->setClearButtonEnabled(true);
}

void LineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    emit(focusIn());
}


PagesSelector::PagesSelector(bool show_all_pages,
                             bool all_pages_first,
                             QWidget *parent) : QWidget(parent)
{
    m_type.addButton(new QRadioButton(tr("Pages:"), this), 0);
    m_type.addButton(new QRadioButton(tr("Even pages"), this), 1);
    m_type.addButton(new QRadioButton(tr("Odd pages"), this), 2);
    if (show_all_pages)
        m_type.addButton(new QRadioButton(tr("All pages"), this), 3);

    QGridLayout *grid_layout = new QGridLayout();
    grid_layout->setColumnStretch(0, 0);
    grid_layout->setColumnStretch(1, 1);
    this->setLayout(grid_layout);

    int row = 0;

    if (show_all_pages)
    {
        if (all_pages_first)
        {
            grid_layout->addWidget(m_type.button(3), row++, 0);
            m_type.button(3)->setChecked(true);
            grid_layout->addWidget(m_type.button(1), row++, 0);
            grid_layout->addWidget(m_type.button(2), row++, 0);
            grid_layout->addWidget(m_type.button(0), row, 0);
            grid_layout->addWidget(&m_selection, row, 1);
        }
        else
        {
            grid_layout->addWidget(m_type.button(0), row, 0);
            m_type.button(0)->setChecked(true);
            grid_layout->addWidget(&m_selection, row++, 1);
            grid_layout->addWidget(m_type.button(1), row++, 0);
            grid_layout->addWidget(m_type.button(2), row++, 0);
            grid_layout->addWidget(m_type.button(3), row, 0);
        }
    }
    else
    {
        grid_layout->addWidget(m_type.button(0), row, 0);
        m_type.button(0)->setChecked(true);
        grid_layout->addWidget(&m_selection, row++, 1);
        grid_layout->addWidget(m_type.button(1), row++, 0);
        grid_layout->addWidget(m_type.button(2), row, 0);
    }

    connect(&m_selection, &LineEdit::focusIn,
            [=]() {m_type.button(0)->setChecked(true);});
}

QString PagesSelector::get_selection_as_text(int num_pages)
{
    switch (m_type.checkedId())
    {
    case 0: {
        int output_pages_count;
        std::vector<std::pair<int, int>> intervals;

        bool result = parse_output_pages_string(
                    m_selection.text().toStdString(),
                    num_pages,
                    intervals,
                    output_pages_count);

        if (m_selection.text().isEmpty() || !result)
        {
            QString error_message(
                        tr("<p>Page intervals are badly formatted. "
                           "Please make sure you complied with the following "
                           "rules:</p><ul>"
                           "<li>intervals of pages must be written indicating "
                           "the first page and the last page separated by a "
                           "dash (e.g. \"1-5\");</li>"
                           "<li>single pages and intervals of pages must be "
                           "separated by spaces, commas or both "
                           "(e.g. \"1, 2, 3, 5-10\" or \"1 2 3 5-10\");</li>"
                           "<li>all pages and intervals of pages must be "
                           "between 1 and the number of pages of the PDF file;"
                           "</li>"
                           "<li>only numbers, spaces, commas and dashes can be "
                           "used. All other characters are not allowed.</li>"
                           "</ul>"));
            QMessageBox::critical(this,
                                  tr("Error"),
                                  error_message);

            return QString();
        }
        else
            return m_selection.text();
    }
    case 1: {
        QString s;

        for (int i = 1; i <= num_pages; i++)
            if (i % 2 == 0)
                s += QString::number(i) + ",";

        return s;
    }
    case 2: {
        QString s;

        for (int i = 1; i <= num_pages; i++)
            if (i % 2 == 1)
                s += QString::number(i) + ",";

        return s;
    }
    case 3: {
        return "";
    }
    }

    return QString();
}
