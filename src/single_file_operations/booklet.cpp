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
#include "../gui_utils.h"

#include <QFormLayout>
#include <QPushButton>

Booklet::Booklet(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *v_layout = new QVBoxLayout();
    QFormLayout *form_layout = new QFormLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(form_layout, 1);
    v_layout->addLayout(h_layout);
    this->setLayout(v_layout);

    booklet_binding.addItem(tr("Left"));
    booklet_binding.addItem(tr("Right"));
    form_layout->addRow(tr("Binding:"), &booklet_binding);

    h_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Minimum));
    QPushButton *button = new QPushButton(tr("Generate booklet"), this);
    button->setShortcut(QKeySequence::Save);
    button->setToolTip(
                QString(TOOLTIP_STRING)
                .arg(
                    button->text(),
                    button->shortcut().toString()));
    connect(button, &QPushButton::pressed,
            this, &Booklet::generate_booklet_pressed);
    h_layout->addWidget(button);
}
