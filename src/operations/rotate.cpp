/* Copyright (C) 2021 Marco Scarpetta
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

#include "rotate.h"

#include <QVBoxLayout>
#include <QPushButton>

#include "../gui_utils.h"

Rotate::Rotate(QWidget *parent) :
    AbstractOperation(parent),
    m_rotation{0},
    m_output_preview{new OutputPreview{this}},
    m_pages_selector{new PagesSelector{true, true, this}}
{
    m_name = tr("Rotate");
    m_icon = QIcon(":/icons/rotate.svg");

    QVBoxLayout *v_layout = new QVBoxLayout{};
    this->setLayout(v_layout);

    QHBoxLayout *h_layout = new QHBoxLayout{};
    QPushButton *rotate_counterclockwise = new QPushButton{this};
    rotate_counterclockwise->setIcon(QIcon::fromTheme("object-rotate-left"));
    rotate_counterclockwise->setFixedSize(60, 60);
    rotate_counterclockwise->setIconSize(QSize(50, 50));
    QPushButton *rotate_clockwise = new QPushButton{this};
    rotate_clockwise->setIcon(QIcon::fromTheme("object-rotate-right"));
    rotate_clockwise->setFixedSize(60, 60);
    rotate_clockwise->setIconSize(QSize(50, 50));
    h_layout->addWidget(rotate_counterclockwise, 0, Qt::AlignRight);
    h_layout->addWidget(rotate_clockwise, 0, Qt::AlignLeft);
    v_layout->addLayout(h_layout);

    connect(rotate_counterclockwise, &QPushButton::clicked,
            [=]() {rotate(false);});
    connect(rotate_clockwise, &QPushButton::clicked,
            [=]() {rotate(true);});

    m_output_preview->setMinimumHeight(100);
    m_output_preview->setMaximumHeight(200);
    v_layout->addWidget(m_output_preview, 10);

    v_layout->addWidget(m_pages_selector);

    // spacer
    v_layout->addWidget(new QWidget(this), 1);

    h_layout = new QHBoxLayout();
    v_layout->addLayout(h_layout);

    // spacer
    h_layout->addWidget(new QWidget(this), 1);

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
            [=]() {save(false);});
    connect(save_as_button, &QPushButton::pressed,
            [=]() {save(true);});
}

void Rotate::set_pdf_info(const PdfInfo &pdf_info)
{
    m_pages_selector->set_num_pages(pdf_info.n_pages());
    AbstractOperation::set_pdf_info(pdf_info);
    m_rotation = 0;
    m_output_preview->set_rotation(m_rotation);
    m_output_preview->set_pdf_info(m_pdf_info);
}

int Rotate::output_pages_count()
{
    return m_pdf_info.n_pages();
}

void Rotate::rotate(bool clockwise)
{
    if (clockwise)
        m_rotation += 90;
    else
        m_rotation -= 90;

    m_output_preview->set_rotation(m_rotation);
}

void Rotate::save(bool save_as)
{
    if (m_pages_selector->has_error())
        return;

    std::vector<std::pair<int, int>> rotate_intervals =
            m_pages_selector->get_selected_intervals();

    if (save_as)
    {
        if (!show_save_as_dialog())
            return;
    }
    else
    {
        if (!show_overwrite_dialog())
            return;
    }

    emit write_started();

    try
    {
        PdfEditor editor;
        unsigned int id = editor.add_file(m_pdf_info.filename());

        std::vector<bool> rotate_pages(m_pdf_info.n_pages(), false);

        std::vector<std::pair<int, int>>::iterator it;
        for (it = rotate_intervals.begin(); it != rotate_intervals.end(); ++it)
            for (int i = it->first; i <= it->second; i++)
                rotate_pages[i] = true;

        // add dummy last element
        rotate_pages.push_back(!rotate_pages.back());

        int first{0};
        std::vector<std::pair<int, int>> intervals;
        for (unsigned int i{1}; i < rotate_pages.size(); i++)
        {
            if (rotate_pages[i - 1] != rotate_pages[i])
            {
                if (rotate_pages[i - 1])
                    editor.add_pages(id, m_rotation, nullptr,
                                     {std::pair<int, int>{first, i - 1}});
                else
                    editor.add_pages(id, 0, nullptr,
                                     {std::pair<int, int>{first, i - 1}});

                first = i;
            }
        }
        emit progress_changed(70);

        editor.write(m_save_filename.toStdString());

        emit write_finished(m_save_filename);
    }
    catch (std::exception &e)
    {
        emit write_error(QString::fromStdString(e.what()));
    }
}
