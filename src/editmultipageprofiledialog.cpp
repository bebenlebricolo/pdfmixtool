/* Copyright (C) 2017-2021 Marco Scarpetta
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

#include "editmultipageprofiledialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <cmath>

EditMultipageProfileDialog::EditMultipageProfileDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(tr("Edit multipage profile"));
    this->setModal(true);

    QGridLayout *layout = new QGridLayout();
    layout->setSpacing(10);
    this->setLayout(layout);

    int row = 1;
    layout->addWidget(new QLabel(tr("Name:"), this), row, 1);
    layout->addWidget(&m_name, row, 2, 1, 3);
    m_name.setClearButtonEnabled(true);

    layout->addWidget(&m_multipage_editor, ++row, 1, 1, 4);

    QPushButton *cancel_button = new QPushButton("Cancel", this);
    QPushButton *save_button = new QPushButton("Save", this);
    cancel_button->setDefault(true);
    save_button->setAutoDefault(true);

    layout->addItem(new QSpacerItem(0, 30), ++row, 1);

    layout->addWidget(cancel_button, ++row, 3);
    layout->addWidget(save_button, row, 4);

    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 1);
    layout->setColumnStretch(3, 0);
    layout->setColumnStretch(4, 0);

    connect(save_button, SIGNAL(pressed()), this, SLOT(accept()));
    connect(cancel_button, SIGNAL(pressed()), this, SLOT(reject()));
}

void EditMultipageProfileDialog::set_multipage(const Multipage &multipage)
{
    m_name.setText(QString::fromStdString(multipage.name));
    m_multipage_editor.set_multipage(multipage);
}

Multipage EditMultipageProfileDialog::get_multipage()
{
    Multipage multipage = m_multipage_editor.get_multipage();
    multipage.name = m_name.text().toStdString();
    return multipage;
}

void EditMultipageProfileDialog::set_index(int index)
{
    m_index = index;
}

int EditMultipageProfileDialog::get_index()
{
    return m_index;
}
