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
    m_has_page_size{false},
    m_multipage_enabled{false},
    m_multipage{},
    m_rotation{0}
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void OutputPreview::set_page_size(double width, double height)
{
    m_page_width = width;
    m_page_height = height;
    m_has_page_size = true;

    update();
}

void OutputPreview::set_pdf_info(const PdfInfo &pdf_info)
{
    m_page_width = pdf_info.width();
    m_page_height = pdf_info.height();
    m_has_page_size = true;

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
    if (!m_has_page_size)
        return;

    QPainter painter(this);

    QRect rect = this->rect();

    painter.save();
    painter.setPen(QColor(10, 10, 10));
    painter.translate(rect.x() + rect.width() / 2,
                       rect.y() + rect.height() / 2);
    painter.rotate(m_rotation);

    int size = std::min(rect.width(), rect.height()) - 4;

    if (!m_multipage_enabled)
        draw_preview_page(painter, size, size,
                          m_page_width, m_page_height,
                          Multipage::Center, Multipage::Center,
                          "1");
    else
    {
        double scale = draw_preview_page(painter,
                                         size,
                                         size,
                                         m_multipage.page_width,
                                         m_multipage.page_height,
                                         Multipage::Center,
                                         Multipage::Center,
                                         "");

        int rows = m_multipage.rows;
        int columns = m_multipage.columns;
        int margin_left = static_cast<int>(m_multipage.margin_left * scale);
        int margin_right = static_cast<int>(m_multipage.margin_right * scale);
        int margin_top = static_cast<int>(m_multipage.margin_top * scale);
        int margin_bottom = static_cast<int>(m_multipage.margin_bottom * scale);
        int page_width = static_cast<int>(m_multipage.page_width * scale);
        int page_height = static_cast<int>(m_multipage.page_height * scale);
        int spacing = static_cast<int>(m_multipage.spacing * scale);

        int subpage_width = (page_width - margin_left - margin_right -
                             (columns - 1) * spacing) / columns;
        int subpage_height = (page_height - margin_top - margin_bottom -
                              (rows - 1) * spacing) / rows;

        int page_number = 1;

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                int dx;
                if (m_multipage.rtl)
                    dx = margin_left - page_width / 2 \
                            + (columns - 1 - j) * (spacing + subpage_width) \
                            + subpage_width / 2;
                else
                    dx = margin_left - page_width / 2 +
                            j * (spacing + subpage_width) + subpage_width / 2;
                int dy = margin_top - page_height / 2 +
                        i * (spacing + subpage_height) + subpage_height / 2;

                painter.translate(dx, dy);

                draw_preview_page(painter, subpage_width, subpage_height,
                                  m_page_width, m_page_height,
                                  m_multipage.h_alignment, m_multipage.v_alignment,
                                  QString::number(page_number));

                painter.translate(-dx, -dy);

                page_number++;
            }
        }

    }

    painter.restore();
}

double OutputPreview::draw_preview_page(QPainter &painter,
                         int max_width, int max_height,
                         double page_width, double page_height,
                         Multipage::Alignment h_alignment,
                         Multipage::Alignment v_alignment,
                         const QString &text)
{
    double scale = std::min(max_width / page_width, max_height / page_height);

    int w = static_cast<int>(page_width * scale);
    int h = static_cast<int>(page_height * scale);
    int dx, dy;

    switch (h_alignment)
    {
    case Multipage::Left:
        dx = - max_width / 2;
        break;
    case Multipage::Center:
        dx = - w / 2;
        break;
    case Multipage::Right:
        dx = max_width / 2 - w;
        break;
    default:
        dx = - max_width / 2;
    }

    switch (v_alignment)
    {
    case Multipage::Top:
        dy = - max_height / 2;
        break;
    case Multipage::Center:
        dy = - h / 2;
        break;
    case Multipage::Bottom:
        dy = max_height / 2 - h;
        break;
    default:
        dy = - max_height / 2;
    }

    painter.fillRect(dx, dy, w, h, Qt::white);
    painter.drawRect(dx, dy, w, h);

    if (text.size() > 0)
    {
        QFont font = painter.font();
        font.setFamily("Serif");
        font.setPixelSize(h / 5 * 4);
        painter.setFont(font);

        int x = dx + w / 2 -
                painter.fontMetrics().boundingRect(text).width() / 2;
        int y = dy + h / 2 + font.pixelSize() / 2;

        painter.drawText(x, y, text);
    }

    return scale;
}
