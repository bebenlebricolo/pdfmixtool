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

#include "add_empty_pages.h"

#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>

#include "../gui_utils.h"
#include "../pdf_edit_lib/pdf_writer.h"

AddEmptyPages::AddEmptyPages(const PdfInfo &pdf_info,
                             QProgressBar *progress_bar,
                             QWidget *parent) :
        AbstractOperation(pdf_info, progress_bar, parent)
{
    m_name = tr("Add empty pages");

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

    grid_layout->addWidget(new QLabel("<b>" + tr("Page size") + "</b>", this),
                           1, 0, 1, 4);

    m_page_size.addButton(new QRadioButton(tr("Same as document"), this), 0);
    grid_layout->addWidget(m_page_size.button(0), 2, 0);

    m_page_size.addButton(new QRadioButton(tr("Custom:"), this), 1);
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

    m_page_size.addButton(new QRadioButton(tr("Standard:"), this), 2);
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

    m_before_after.addButton(new QRadioButton(tr("Before"), this), 0);
    grid_layout->addWidget(m_before_after.button(0), 7, 0);
    m_before_after.addButton(new QRadioButton(tr("After"), this), 1);
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
            [=]() {if (show_overwrite_dialog()) save();});
    connect(save_as_button, &QPushButton::pressed,
            [=]() {if (show_save_as_dialog()) save();});
}

void AddEmptyPages::pdf_info_changed()
{
    AbstractOperation::pdf_info_changed();

    m_page.setRange(1, m_pdf_info->n_pages());
}

void AddEmptyPages::save()
{
    emit write_started();

    int count = m_count.value();

    double page_width, page_height;
    switch (m_page_size.checkedId()) {
    case 0: {
        page_width = m_pdf_info->width();
        page_height = m_pdf_info->height();
        break;
    }
    case 1: {
        page_width = m_page_width.value();
        page_height = m_page_height.value();
        break;
    }
    case 2: {
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

    int location = m_page.value();
    bool before = !m_before_after.checkedId();

    QProgressBar *pb = m_progress_bar;
    std::function<void (int)> progress = [pb] (int p)
    {
        pb->setValue(p);
    };

    write_add_empty_pages(m_pdf_info->filename(),
                          m_save_filename.toStdString(),
                          count,
                          page_width,
                          page_height,
                          location,
                          before,
                          progress);

    emit write_finished(m_save_filename);
}
