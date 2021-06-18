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

#include "add_empty_pages.h"

#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>

#include "../gui_utils.h"
#include "../pdf_edit_lib/pdf_editor.h"

AddEmptyPages::AddEmptyPages(QWidget *parent) :
        AbstractOperation(parent)
{
    m_name = tr("Add empty pages");
    m_icon = QIcon(":/icons/add_empty_pages.svg");

    QVBoxLayout *v_layout = new QVBoxLayout();
    QGridLayout *grid_layout = new QGridLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(grid_layout);
    v_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Minimum,
                                      QSizePolicy::Expanding));
    v_layout->addLayout(h_layout);
    this->setLayout(v_layout);

    grid_layout->addWidget(new QLabel(tr("Count:"), this), 0, 0);
    grid_layout->addWidget(&m_count, 0, 1);
    m_count.setRange(1, 1000);
    connect(&m_count, QOverload<int>::of(&QSpinBox::valueChanged),
            [=](int value) {
        emit output_pages_count_changed(output_pages_count());});

    grid_layout->addWidget(new QLabel("<b>" + tr("Page size") + "</b>", this),
                           1, 0, 1, 4);

    m_page_size.addButton(new QRadioButton(tr("Same as document"), this),
                          static_cast<int>(m_pages_size_type::same));
    grid_layout->addWidget(m_page_size.button(0), 2, 0);

    m_page_size.addButton(new QRadioButton(tr("Custom:"), this),
                          static_cast<int>(m_pages_size_type::custom));
    grid_layout->addWidget(m_page_size.button(1), 3, 0);

    m_page_width.setSuffix(" cm");
    m_page_width.setDecimals(1);
    m_page_width.setSingleStep(0.1);
    m_page_width.setMinimum(1.0);
    m_page_width.setMaximum(1000.0);
    m_page_width.setValue(21.0);

    m_page_height.setSuffix(" cm");
    m_page_height.setDecimals(1);
    m_page_height.setSingleStep(0.1);
    m_page_height.setMinimum(1.0);
    m_page_height.setMaximum(1000.0);
    m_page_height.setValue(29.7);

    grid_layout->addWidget(&m_page_width, 3, 1);
    grid_layout->addWidget(new QLabel("×", this), 3, 2, Qt::AlignCenter);
    grid_layout->setColumnStretch(0, 1);
    grid_layout->setColumnStretch(1, 1);
    grid_layout->setColumnStretch(2, 0);
    grid_layout->setColumnStretch(3, 1);
    grid_layout->addWidget(&m_page_height, 3, 3);

    m_page_size.addButton(new QRadioButton(tr("Standard:"), this),
                          static_cast<int>(m_pages_size_type::standard));
    grid_layout->addWidget(m_page_size.button(2), 4, 0, 1, 2);
    int i = 0;
    for (PaperSize size : paper_sizes)
    {
        m_standard_page_size.addItem(QString::fromStdString(size.name), i);
        i++;
    }
    grid_layout->addWidget(&m_standard_page_size, 4, 1, 1, 3);
    m_orientation.addButton(new QRadioButton(tr("Portrait"), this), 0);
    grid_layout->addWidget(m_orientation.button(0), 5, 1);
    m_orientation.addButton(new QRadioButton(tr("Landscape"), this), 1);
    grid_layout->addWidget(m_orientation.button(1), 5, 3);

    grid_layout->addWidget(new QLabel("<b>" + tr("Location") + "</b>", this),
                           6, 0, 1, 4);

    m_before_after.addButton(new QRadioButton(tr("Before"), this),
                             static_cast<int>(m_position::before));
    grid_layout->addWidget(m_before_after.button(0), 7, 0);
    m_before_after.addButton(new QRadioButton(tr("After"), this),
                             static_cast<int>(m_position::after));
    grid_layout->addWidget(m_before_after.button(1), 7, 1);

    grid_layout->addWidget(new QLabel(tr("Page:"), this), 8, 0);
    grid_layout->addWidget(&m_page, 8, 1);
    m_page.setRange(1, 1000);

    m_page_size.button(0)->setChecked(true);
    m_orientation.button(0)->setChecked(true);
    m_before_after.button(0)->setChecked(true);

    h_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Minimum));

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
            [=]() {if (show_overwrite_dialog()) m_save();});
    connect(save_as_button, &QPushButton::pressed,
            [=]() {if (show_save_as_dialog()) m_save();});
}

void AddEmptyPages::set_pdf_info(const PdfInfo &pdf_info)
{
    AbstractOperation::set_pdf_info(pdf_info);

    m_page.setRange(1, m_pdf_info.n_pages());

    m_page_width.setValue(m_pdf_info.width());
    m_page_height.setValue(m_pdf_info.height());
}

int AddEmptyPages::output_pages_count()
{
    if (m_pdf_info.n_pages() == 0)
        return 0;
    else
        return m_pdf_info.n_pages() + m_count.value();
}

void AddEmptyPages::m_save()
{
    emit write_started();

    int count = m_count.value();

    m_pages_size_type type =
            static_cast<m_pages_size_type>(m_page_size.checkedId());
    double page_width, page_height;
    switch (type)
    {
    case m_pages_size_type::same:
    {
        page_width = m_pdf_info.width();
        page_height = m_pdf_info.height();
        break;
    }
    case m_pages_size_type::custom:
    {
        page_width = m_page_width.value();
        page_height = m_page_height.value();
        break;
    }
    case m_pages_size_type::standard:
    {
        PaperSize size = paper_sizes[
                m_standard_page_size.currentData().toUInt()];

        if (m_orientation.checkedId() == 0)
        {
            page_width = size.width;
            page_height = size.height;
        }
        else
        {
            page_width = size.height;
            page_height = size.width;
        }
        break;
    }
    }

    page_width = page_width * cm;
    page_height = page_height * cm;

    try
    {
        PdfEditor editor;
        unsigned int id = editor.add_file(m_pdf_info.filename());
        int location = m_page.value() - 1;

        m_position position = static_cast<m_position>(m_before_after.checkedId());
        switch (position)
        {
        case m_position::before:
        {
            if (location > 0)
            {
                editor.add_pages(id, 0, nullptr, {{0, location - 1}});
                emit progress_changed(20);
            }

            editor.add_blank_pages(page_width, page_height, count);
            emit progress_changed(40);

            editor.add_pages(id, 0, nullptr,
                             {{location, m_pdf_info.n_pages() - 1}});
            emit progress_changed(60);

            break;
        }
        case m_position::after:
        {
            editor.add_pages(id, 0, nullptr, {{0, location}});
            emit progress_changed(20);

            editor.add_blank_pages(page_width, page_height, count);
            emit progress_changed(40);

            if (location < m_pdf_info.n_pages() - 1)
            {
                editor.add_pages(id, 0, nullptr,
                                 {{location + 1, m_pdf_info.n_pages() - 1}});
                emit progress_changed(60);
            }
        }
        }

        editor.write(m_save_filename.toStdString());

        emit write_finished(m_save_filename);
    }
    catch (std::exception &e)
    {
        emit write_error(QString::fromStdString(e.what()));
    }
}
