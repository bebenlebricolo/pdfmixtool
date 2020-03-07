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

#include "edit_page_layout.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <cmath>

#include "../gui_utils.h"

EditPageLayout::EditPageLayout(QWidget *parent) : QWidget(parent)
{
    m_new_profile_triggered = false;

    QVBoxLayout *v_layout = new QVBoxLayout();
    QFormLayout *form_layout = new QFormLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(form_layout);
    v_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Expanding));
    v_layout->addWidget(&preview_image, 0, Qt::AlignCenter);
    v_layout->addWidget(&paper_size_label, 0, Qt::AlignCenter);
    v_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Expanding));
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

    scale.setRange(1, 1000);
    scale.setValue(100);
    scale.setSuffix("%");
    connect(&scale,
            SIGNAL(valueChanged(int)),
            SLOT(update_preview_image()));
    form_layout->addRow(tr("Scale page:"), &scale);

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
            this, &EditPageLayout::save_button_pressed);

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
            this, &EditPageLayout::save_as_button_pressed);

    h_layout->addWidget(save_button);
    h_layout->addWidget(save_as_button);
}

void EditPageLayout::update_multipage_profiles()
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

void EditPageLayout::update_preview_image()
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
        double page_width = opened_pdf_info.width();
        double page_height = opened_pdf_info.height();

        int rotation_value = rotation.currentData().toInt();
        int mp_index = multipage.currentData().toInt();
        Multipage mp;
        if (mp_index >= 0)
        {
            mp = multipages[mp_index];
            page_width = mp.page_width;
            page_height = mp.page_height;
        }

        draw_preview(&painter,
                     QRect(0, 0, size, size),
                     opened_pdf_info.width(),
                     opened_pdf_info.height(),
                     rotation_value,
                     mp_index >= 0,
                     mp);

        if (rotation_value == 90 || rotation_value == 270)
        {
            double tmp = page_width;
            page_width = page_height;
            page_height = tmp;
        }

        page_width = std::round(page_width * scale.value() / 10) / 10;
        page_height = std::round(page_height * scale.value() / 10) / 10;

        QString text = QString::number(page_width) +
                QString(" cm \u00D7 %1 cm").arg(page_height);
        paper_size_label.setText(text);
    }

    preview_image.setPixmap(pixmap);
}

void EditPageLayout::multipage_activated(int index)
{
    if (index == multipage.count() - 1)
    {
        multipage.setCurrentIndex(0);
        m_new_profile_triggered = true;
        emit trigger_new_profile();
    }
}

void EditPageLayout::profile_created(int index)
{
    if (m_new_profile_triggered)
    {
        m_new_profile_triggered = false;

        if (index != -1)
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
    }
}
