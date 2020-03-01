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

#include "rotation_multipage.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QPushButton>

#include "../gui_utils.h"

RotationMultipage::RotationMultipage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *v_layout = new QVBoxLayout();
    QFormLayout *form_layout = new QFormLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(form_layout);
    v_layout->addWidget(&preview_image, 1, Qt::AlignCenter);
    v_layout->addLayout(h_layout);
    this->setLayout(v_layout);

    preview_image.setMinimumSize(200, 200);

    rotation.addItem(tr("No rotation"), 0);
    rotation.addItem("90°", 90);
    rotation.addItem("180°", 180);
    rotation.addItem("270°", 270);
    connect(&rotation,
            SIGNAL(currentIndexChanged(int)),
            SLOT(update_preview_image()));
    form_layout->addRow(tr("Rotation:"), &rotation);

    multipage.addItem(tr("Disabled"), -1);
    QMap<int, Multipage>::const_iterator it;
    for (it = multipages.constBegin();
         it != multipages.constEnd();
         ++it)
        multipage.addItem(
                    QString::fromStdString(it.value().name),
                    it.key());
    multipage.addItem(tr("New custom profile…"), -2);
    connect(&multipage,
            SIGNAL(currentIndexChanged(int)),
            SLOT(update_preview_image()));
    connect(&multipage, SIGNAL(activated(int)),
            SLOT(multipage_activated(int)));
    form_layout->addRow(tr("Multipage:"), &multipage);

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
    connect(save_button, &QPushButton::pressed,
            this, &RotationMultipage::save_button_pressed);

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
    connect(save_as_button, &QPushButton::pressed,
            this, &RotationMultipage::save_as_button_pressed);

    h_layout->addWidget(save_button);
    h_layout->addWidget(save_as_button);
}

void RotationMultipage::update_multipage_profiles()
{
    int current_data = multipage.currentData().toInt();
    int current_index = 0;
    multipage.clear();
    multipage.addItem(tr("Disabled"), -1);
    QMap<int, Multipage>::const_iterator it;
    for (it = multipages.constBegin();
         it != multipages.constEnd();
         ++it)
    {
        multipage.addItem(
                    QString::fromStdString(it.value().name),
                    it.key());
        if (it.key() == current_data)
            current_index = multipage.count() - 1;
    }
    multipage.addItem(tr("New custom profile…"), -2);
    multipage.setCurrentIndex(current_index);
}

void RotationMultipage::update_preview_image()
{
    int size = preview_image.minimumWidth();

    QPixmap pixmap(size, size);
    QPainter painter(&pixmap);

    if (opened_pdf_info.filename().empty())
    {
        painter.fillRect(0, 0, size, size, Qt::white);
    }
    else
    {
        int rotation_value = rotation.currentData().toInt();
        int mp_index = multipage.currentData().toInt();
        Multipage mp;
        if (mp_index >= 0)
            mp = multipages[mp_index];

        draw_preview(&painter,
                     QRect(0, 0, size, size),
                     opened_pdf_info.width(),
                     opened_pdf_info.height(),
                     rotation_value,
                     mp_index >= 0,
                     mp);
    }

    preview_image.setPixmap(pixmap);
}

void RotationMultipage::multipage_activated(int index)
{
    if (index == multipage.count() - 1)
    {
        multipage.setCurrentIndex(0);
        emit trigger_new_profile();
    }
}

void RotationMultipage::profile_created(int index)
{
    multipage.clear();
    multipage.addItem(tr("Disabled"), -1);
    QMap<int, Multipage>::const_iterator it;
    for (it = multipages.constBegin();
         it != multipages.constEnd();
         ++it)
        multipage.addItem(
                    QString::fromStdString(it.value().name),
                    it.key());
    multipage.addItem(tr("New custom profile…"), -2);
    multipage.setCurrentIndex(index + 1);
}
