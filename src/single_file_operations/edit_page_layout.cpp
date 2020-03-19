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
#include "../pdf_edit_lib/pdf_writer.h"

EditPageLayout::EditPageLayout(const PdfInfo &pdf_info,
                               QProgressBar *progress_bar,
                               QWidget *parent) :
          AbstractOperation(pdf_info, progress_bar, parent)
{
    m_name = tr("Edit page layout");

    m_new_profile_triggered = false;

    QVBoxLayout *v_layout = new QVBoxLayout();
    QFormLayout *form_layout = new QFormLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    v_layout->addLayout(form_layout);
    v_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Expanding));
    v_layout->addWidget(&m_preview_image, 0, Qt::AlignCenter);
    v_layout->addWidget(&m_paper_size_label, 0, Qt::AlignCenter);
    v_layout->addItem(new QSpacerItem(0, 0,
                                      QSizePolicy::Expanding,
                                      QSizePolicy::Expanding));
    v_layout->addLayout(h_layout);
    this->setLayout(v_layout);

    m_preview_image.setMinimumSize(200, 200);

    m_rotation.addItem(tr("No rotation"), 0);
    m_rotation.addItem("90°", 90);
    m_rotation.addItem("180°", 180);
    m_rotation.addItem("270°", 270);
    connect(&m_rotation,
            SIGNAL(currentIndexChanged(int)),
            SLOT(update_preview_image()));
    form_layout->addRow(tr("Rotation:"), &m_rotation);

    m_multipage.addItem(tr("Disabled"), -1);
    QMap<int, Multipage>::const_iterator it;
    for (it = multipages.constBegin();
         it != multipages.constEnd();
         ++it)
        m_multipage.addItem(
                    QString::fromStdString(it.value().name),
                    it.key());
    m_multipage.addItem(tr("New custom profile…"), -2);
    connect(&m_multipage,
            SIGNAL(currentIndexChanged(int)),
            SLOT(update_preview_image()));
    connect(&m_multipage, SIGNAL(activated(int)),
            SLOT(multipage_activated(int)));
    form_layout->addRow(tr("Multipage:"), &m_multipage);

    m_scale.setRange(1, 100);
    m_scale.setValue(100);
    m_scale.setSuffix("%");
    connect(&m_scale,
            SIGNAL(valueChanged(int)),
            SLOT(update_preview_image()));
    form_layout->addRow(tr("Scale page:"), &m_scale);

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
            [=]() {if (show_overwrite_dialog()) save();});
    connect(save_as_button, &QPushButton::pressed,
            [=]() {if (show_save_as_dialog()) save();});
}

void EditPageLayout::pdf_info_changed()
{
    AbstractOperation::pdf_info_changed();

    update_preview_image();
}

void EditPageLayout::update_multipage_profiles()
{
    int current_data = m_multipage.currentData().toInt();
    int current_index = 0;
    m_multipage.clear();
    m_multipage.addItem(tr("Disabled"), -1);
    QMap<int, Multipage>::const_iterator it;
    for (it = multipages.constBegin();
         it != multipages.constEnd();
         ++it)
    {
        m_multipage.addItem(
                    QString::fromStdString(it.value().name),
                    it.key());
        if (it.key() == current_data)
            current_index = m_multipage.count() - 1;
    }
    m_multipage.addItem(tr("New custom profile…"), -2);
    m_multipage.setCurrentIndex(current_index);
}

void EditPageLayout::update_preview_image()
{
    int size = m_preview_image.minimumWidth();

    QPixmap pixmap(size, size);
    QPainter painter(&pixmap);

    if (m_pdf_info->filename().empty())
    {
        painter.fillRect(0, 0, size, size, Qt::white);
    }
    else
    {
        double page_width = m_pdf_info->width();
        double page_height = m_pdf_info->height();

        int rotation_value = m_rotation.currentData().toInt();
        int mp_index = m_multipage.currentData().toInt();
        Multipage mp;
        if (mp_index >= 0)
        {
            mp = multipages[mp_index];
            page_width = mp.page_width;
            page_height = mp.page_height;
        }

        draw_preview(&painter,
                     QRect(0, 0, size, size),
                     m_pdf_info->width(),
                     m_pdf_info->height(),
                     rotation_value,
                     mp_index >= 0,
                     mp);

        if (rotation_value == 90 || rotation_value == 270)
        {
            double tmp = page_width;
            page_width = page_height;
            page_height = tmp;
        }

        page_width = std::round(page_width * m_scale.value() / 10) / 10;
        page_height = std::round(page_height * m_scale.value() / 10) / 10;

        QString text = QString::number(page_width) +
                QString(" cm \u00D7 %1 cm").arg(page_height);
        m_paper_size_label.setText(text);
    }

    m_preview_image.setPixmap(pixmap);
}

void EditPageLayout::multipage_activated(int index)
{
    if (index == m_multipage.count() - 1)
    {
        m_multipage.setCurrentIndex(0);
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
            m_multipage.clear();
            m_multipage.addItem(tr("Disabled"), -1);
            QMap<int, Multipage>::const_iterator it;
            for (it = multipages.constBegin();
                 it != multipages.constEnd();
                 ++it)
                m_multipage.addItem(
                            QString::fromStdString(it.value().name),
                            it.key());
            m_multipage.addItem(tr("New custom profile…"), -2);
            m_multipage.setCurrentIndex(index + 1);
        }
    }
}

void EditPageLayout::save()
{
    emit write_started();
    Conf conf;

    conf.output_path = m_save_filename.toStdString();
    conf.alternate_mix = false;

    FileConf fileconf;
    fileconf.path = m_pdf_info->filename();
    fileconf.ouput_pages = "";
    int mp_index = m_multipage.currentData().toInt();
    if (mp_index < 0)
        fileconf.multipage_enabled = false;
    else
    {
        fileconf.multipage_enabled = true;
        fileconf.multipage = &multipages[mp_index];
    }
    fileconf.rotation = m_rotation.currentData().toInt();
    fileconf.scale = m_scale.value();
    fileconf.outline_entry = "";
    fileconf.reverse_order = false;

    conf.files.push_back(fileconf);

    QProgressBar *pb = m_progress_bar;
    std::function<void (int)> progress = [pb] (int p)
    {
        pb->setValue(p);
    };

    write_pdf(conf, progress);

    emit write_finished(m_save_filename);
}
