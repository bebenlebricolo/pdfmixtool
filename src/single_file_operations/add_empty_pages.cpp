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

AddEmptyPages::AddEmptyPages(QWidget *parent) : QWidget(parent)
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

    grid_layout->addWidget(new QLabel(tr("Count:"), this), 0, 0);
    grid_layout->addWidget(&count, 0, 1);
    count.setRange(1, 1000);

    grid_layout->addWidget(new QLabel("<b>" + tr("Page size") + "</b>", this),
                           1, 0, 1, 4);

    page_size.addButton(new QRadioButton(tr("Same as document"), this), 0);
    grid_layout->addWidget(page_size.button(0), 2, 0);

    page_size.addButton(new QRadioButton(tr("Custom:"), this), 1);
    grid_layout->addWidget(page_size.button(1), 3, 0);

    page_width.setSuffix(" cm");
    page_width.setDecimals(1);
    page_width.setSingleStep(0.1);
    page_width.setMinimum(1.0);
    page_width.setMaximum(1000.0);
    page_width.setValue(21.0);

    page_height.setSuffix(" cm");
    page_height.setDecimals(1);
    page_height.setSingleStep(0.1);
    page_height.setMinimum(1.0);
    page_height.setMaximum(1000.0);
    page_height.setValue(29.7);

    grid_layout->addWidget(&page_width, 3, 1);
    grid_layout->addWidget(new QLabel("×", this), 3, 2, Qt::AlignCenter);
    grid_layout->setColumnStretch(0, 1);
    grid_layout->setColumnStretch(1, 1);
    grid_layout->setColumnStretch(2, 0);
    grid_layout->setColumnStretch(3, 1);
    grid_layout->addWidget(&page_height, 3, 3);

    page_size.addButton(new QRadioButton(tr("Standard:"), this), 2);
    grid_layout->addWidget(page_size.button(2), 4, 0, 1, 2);
    int i = 0;
    for (PaperSize size : paper_sizes)
    {
        standard_page_size.addItem(QString::fromStdString(size.name), i);
        i++;
    }
    grid_layout->addWidget(&standard_page_size, 4, 1, 1, 3);
    orientation.addButton(new QRadioButton(tr("Portrait"), this), 0);
    grid_layout->addWidget(orientation.button(0), 5, 1);
    orientation.addButton(new QRadioButton(tr("Landscape"), this), 1);
    grid_layout->addWidget(orientation.button(1), 5, 3);

    grid_layout->addWidget(new QLabel("<b>" + tr("Location") + "</b>", this),
                           6, 0, 1, 4);

    before_after.addButton(new QRadioButton(tr("Before"), this), 0);
    grid_layout->addWidget(before_after.button(0), 7, 0);
    before_after.addButton(new QRadioButton(tr("After"), this), 1);
    grid_layout->addWidget(before_after.button(1), 7, 1);

    grid_layout->addWidget(new QLabel(tr("Page:"), this), 8, 0);
    grid_layout->addWidget(&page, 8, 1);
    page.setRange(1, 1000);

    page_size.button(0)->setChecked(true);
    orientation.button(0)->setChecked(true);
    before_after.button(0)->setChecked(true);

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
            this, &AddEmptyPages::save_button_pressed);
    connect(save_as_button, &QPushButton::pressed,
            this, &AddEmptyPages::save_as_button_pressed);
}
