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

#include <QDebug>

#include "output_preview.h"

OutputPreview::OutputPreview(QWidget *parent) :
    QWidget(parent),
    m_has_pdf_info{false},
    m_multipage_enabled{false},
    m_rotation{0}
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void OutputPreview::set_pdf_info(const PdfInfo &pdf_info)
{
    m_pdf_info = pdf_info;
    m_has_pdf_info = true;

    update();
}

void OutputPreview::set_multipage_enabled(bool enabled)
{
    m_multipage_enabled = enabled;

    update();
}

void OutputPreview::set_multipage(const Multipage &multipage)
{
    m_multipage_enabled = true;
    m_multipage = multipage;

    update();
}

void OutputPreview::set_rotation(int rotation)
{
    m_rotation = rotation;

    update();
}

void OutputPreview::paintEvent(QPaintEvent *)
{
    if (m_has_pdf_info)
    {
        QPainter painter(this);

        draw_preview(&painter, rect(), m_pdf_info.width(), m_pdf_info.height(),
                     m_rotation, m_multipage_enabled, m_multipage);
    }
}
