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

#include "inputpdffilewidget.h"

#include <QGridLayout>
#include <QFocusEvent>
#include <QPainter>

#include "gui_utils.h"
#include "multipageprofilesmanager.h"

InputPdfFileWidget::InputPdfFileWidget(
        const QModelIndex &index,
        const QMap<int, Multipage> &custom_multipages,
        int preview_size,
        bool alternate_mix,
        QWidget *parent) :
    QWidget(parent),
    m_multipages(custom_multipages),
    m_alternate_mix(alternate_mix),
    m_output_preview(new OutputPreview(this)),
    m_new_profile_triggered(false)
{
    m_index = index.row();
    m_page_width = index.data(PAGE_WIDTH_ROLE).toDouble();
    m_page_height = index.data(PAGE_HEIGHT_ROLE).toDouble();

    this->setBackgroundRole(QPalette::AlternateBase);
    this->setAutoFillBackground(true);

    QGridLayout *layout = new QGridLayout();
    this->setLayout(layout);

    layout->addWidget(m_output_preview, 1, 1, 2, 1);
    double size = preview_size - layout->contentsMargins().top() * 2;
    m_output_preview->setFixedSize(size, size);
    layout->setColumnMinimumWidth(1, preview_size + 10);
    layout->setAlignment(m_output_preview, Qt::AlignLeft);

    if (alternate_mix)
    {
        m_reverse_order_checkbox = new QCheckBox(this);

        layout->addWidget(new QLabel(tr("Reverse page order:"), this), 1, 2);
        layout->addWidget(m_reverse_order_checkbox, 1, 3);
        layout->setColumnStretch(3, 1);
    }
    else
    {
        m_pages_filter_lineedit = new QLineEdit(this);
        m_multipage_combobox = new QComboBox(this);
        m_rotation_combobox = new QComboBox(this);
        m_outline_entry_lineedit = new QLineEdit(this);

        m_pages_filter_lineedit->setClearButtonEnabled(true);
        m_outline_entry_lineedit->setClearButtonEnabled(true);

        m_multipage_combobox->addItem(tr("Disabled"), -1);
        QMap<int, Multipage>::const_iterator it;
        for (it = m_multipages.constBegin();
             it != m_multipages.constEnd();
             ++it)
            m_multipage_combobox->addItem(
                        QString::fromStdString(it.value().name),
                        it.key());
        m_multipage_combobox->addItem(tr("New custom profile…"), -2);

        m_rotation_combobox->addItem(tr("No rotation"), 0);
        m_rotation_combobox->addItem("90°", 90);
        m_rotation_combobox->addItem("180°", 180);
        m_rotation_combobox->addItem("270°", 270);

        layout->addWidget(new QLabel(tr("Pages:"), this), 1, 2);
        layout->addWidget(m_pages_filter_lineedit, 1, 3);
        layout->addWidget(new QLabel(tr("Multipage:"), this), 1, 4);
        layout->addWidget(m_multipage_combobox, 1, 5);
        layout->addWidget(new QLabel(tr("Rotation:"), this), 2, 2);
        layout->addWidget(m_rotation_combobox, 2, 3);
        layout->addWidget(new QLabel(tr("Outline entry:"), this), 2, 4);
        layout->addWidget(m_outline_entry_lineedit, 2, 5);

        layout->addItem(new QSpacerItem(0, 0), 1, 6);
        layout->setColumnStretch(3, 10);
        layout->setColumnStretch(5, 10);
        layout->setColumnStretch(6, 90);

        connect(m_multipage_combobox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(update_preview()));
        connect(m_multipage_combobox, SIGNAL(activated(int)),
                this, SLOT(multipage_activated(int)));
        connect(m_rotation_combobox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(update_preview()));
    }

    update_preview();
}

void InputPdfFileWidget::set_editor_data(const QModelIndex &index)
{
    if (m_alternate_mix)
    {
        m_reverse_order_checkbox->setChecked(
                    index.data(REVERSE_ORDER_ROLE).toBool());
    }
    else
    {
        m_pages_filter_lineedit->setText(
                    index.data(OUTPUT_PAGES_ROLE).toString());
        m_multipage_combobox->setCurrentIndex(
                m_multipage_combobox->findData(
                        index.data(MULTIPAGE_ROLE).toInt()));
        m_rotation_combobox->setCurrentIndex(
                m_rotation_combobox->findData(
                        index.data(ROTATION_ROLE).toInt()));
        m_outline_entry_lineedit->setText(
                    index.data(OUTLINE_ENTRY_ROLE).toString());
    }
}

void InputPdfFileWidget::set_model_data(QStandardItem *item)
{
    if (m_alternate_mix)
    {
        item->setData(m_reverse_order_checkbox->isChecked(),
                      REVERSE_ORDER_ROLE);
    }
    else
    {
        m_last_item = item;
        QString current_output_pages = m_pages_filter_lineedit->text();
        int current_mp_index = m_multipage_combobox->currentData().toInt();
        int current_rotation = m_rotation_combobox->currentData().toInt();
        QString current_outline_entry = m_outline_entry_lineedit->text();

        item->setData(current_output_pages, OUTPUT_PAGES_ROLE);
        item->setData(current_mp_index, MULTIPAGE_ROLE);
        item->setData(current_rotation, ROTATION_ROLE);
        item->setData(current_outline_entry, OUTLINE_ENTRY_ROLE);
    }
}

void InputPdfFileWidget::mouse_button_pressed(QMouseEvent *event)
{
    if (m_alternate_mix)
    {
        if (!this->rect().contains(
                    this->mapFromGlobal(event->globalPos())))
            emit focus_out(this);
    }
    else
    {
        QRect mp_rect = m_multipage_combobox->rect();
        mp_rect.setHeight((m_multipage_combobox->count() + 1) *
                          mp_rect.height());
        mp_rect.setTop(mp_rect.top() - m_multipage_combobox->count() *
                       mp_rect.height());

        QRect rotation_rect = m_rotation_combobox->rect();
        rotation_rect.setHeight((m_rotation_combobox->count() + 1) *
                                rotation_rect.height());
        rotation_rect.setTop(rotation_rect.top() -
                             m_rotation_combobox->count() *
                             rotation_rect.height());

        if (!this->rect().contains(
                    this->mapFromGlobal(event->globalPos())) &&
                !mp_rect.contains(
                    m_multipage_combobox->mapFromGlobal(event->globalPos())) &&
                !rotation_rect.contains(
                    m_rotation_combobox->mapFromGlobal(event->globalPos()))
                )
            emit focus_out(this);
    }
}

void InputPdfFileWidget::update_preview()
{

    if (m_alternate_mix)
    {
        m_output_preview->set_page_size(m_page_width, m_page_height);
        m_output_preview->set_rotation(0);
        m_output_preview->set_multipage_enabled(false);
    }
    else
    {
        bool ok;
        int mp_index = m_multipage_combobox->currentData().toInt(&ok);
        if (!ok)
            mp_index = -1;
        bool mp_enabled;
        Multipage mp;
        if (mp_index < 0)
            mp_enabled = false;
        else
        {
            mp_enabled = true;
            mp = m_multipages[mp_index];
        }

        m_output_preview->set_page_size(m_page_width, m_page_height);
        m_output_preview->set_rotation(m_rotation_combobox->currentData().toInt());
        m_output_preview->set_multipage(mp);
        m_output_preview->set_multipage_enabled(mp_enabled);
    }
}

void InputPdfFileWidget::multipage_activated(int index)
{
    if (index == m_multipage_combobox->count() - 1)
    {
        m_multipage_combobox->setCurrentIndex(0);
        m_new_profile_triggered = true;
        emit trigger_new_profile(m_index);
    }
}

void InputPdfFileWidget::profile_created(int index)
{
    if (m_new_profile_triggered)
    {
        m_new_profile_triggered = false;

        if (index != -1)
            m_last_item->setData(index, MULTIPAGE_ROLE);
    }
}
