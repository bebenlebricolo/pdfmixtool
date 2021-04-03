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

#include "gui_utils.h"

QDataStream &operator<<(QDataStream &out, const Multipage &multipage)
{
    out << 1; // version

    out << multipage.name.c_str();

    out << multipage.page_width;
    out << multipage.page_height;

    out << multipage.rows;
    out << multipage.columns;

    out << multipage.rtl;

    out << multipage.h_alignment;
    out << multipage.v_alignment;

    out << multipage.margin_left;
    out << multipage.margin_right;
    out << multipage.margin_top;
    out << multipage.margin_bottom;

    out << multipage.spacing;

    return out;
}

QDataStream &operator>>(QDataStream &in, Multipage &multipage)
{
    int version;
    in >> version;

    char *name;
    in >> name;
    multipage.name = name;

    in >> multipage.page_width;
    in >> multipage.page_height;

    in >> multipage.rows;
    in >> multipage.columns;

    if (version == 0)
    {
        int rotation;
        in >> rotation;
        multipage.rtl = false;
    }
    else if (version == 1)
    {
        in >> multipage.rtl;
    }

    int h_alignment;
    int v_alignment;
    in >> h_alignment;
    multipage.h_alignment = static_cast<Multipage::Alignment>(h_alignment);
    in >> v_alignment;
    multipage.v_alignment = static_cast<Multipage::Alignment>(v_alignment);

    in >> multipage.margin_left;
    in >> multipage.margin_right;
    in >> multipage.margin_top;
    in >> multipage.margin_bottom;

    in >> multipage.spacing;

    return in;
}

double draw_preview_page(QPainter *painter,
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

    painter->drawRect(dx, dy, w, h);

    if (text.size() > 0)
    {
        QFont font = painter->font();
        font.setFamily("Serif");
        font.setPixelSize(h / 5 * 4);
        painter->setFont(font);

        int x = dx + w / 2 -
                painter->fontMetrics().boundingRect(text).width() / 2;
        int y = dy + h / 2 + font.pixelSize() / 2;

        painter->drawText(x, y, text);
    }

    return scale;
}

void draw_preview(QPainter *painter, const QRect &rect,
                  double source_width, double source_height,
                  int rotation,
                  bool multipage_enabled, const Multipage &multipage)
{
    painter->save();

    painter->fillRect(rect, painter->background());
    painter->setPen(QColor(10, 10, 10));

    painter->translate(rect.x() + rect.width() / 2,
                       rect.y() + rect.height() / 2);
    painter->rotate(rotation);

    if (!multipage_enabled)
        draw_preview_page(painter, rect.width() - 4, rect.height() - 4,
                          source_width, source_height,
                          Multipage::Center, Multipage::Center,
                          "1");
    else
    {
        double scale = draw_preview_page(painter,
                                         rect.width() - 4,
                                         rect.height() - 4,
                                         multipage.page_width,
                                         multipage.page_height,
                                         Multipage::Center,
                                         Multipage::Center,
                                         "");

        int rows = multipage.rows;
        int columns = multipage.columns;
        int margin_left = static_cast<int>(multipage.margin_left * scale);
        int margin_right = static_cast<int>(multipage.margin_right * scale);
        int margin_top = static_cast<int>(multipage.margin_top * scale);
        int margin_bottom = static_cast<int>(multipage.margin_bottom * scale);
        int page_width = static_cast<int>(multipage.page_width * scale);
        int page_height = static_cast<int>(multipage.page_height * scale);
        int spacing = static_cast<int>(multipage.spacing * scale);

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
                if (multipage.rtl)
                    dx = margin_left - page_width / 2 \
                            + (columns - 1 - j) * (spacing + subpage_width) \
                            + subpage_width / 2;
                else
                    dx = margin_left - page_width / 2 +
                            j * (spacing + subpage_width) + subpage_width / 2;
                int dy = margin_top - page_height / 2 +
                        i * (spacing + subpage_height) + subpage_height / 2;

                painter->translate(dx, dy);

                draw_preview_page(painter, subpage_width, subpage_height,
                                  source_width, source_height,
                                  multipage.h_alignment, multipage.v_alignment,
                                  QString::number(page_number));

                painter->translate(-dx, -dy);

                page_number++;
            }
        }

    }

    painter->restore();
}

QSettings *settings = new QSettings("PDFMixTool", "pdfmixtool");
QMap<int, Multipage> multipages = QMap<int, Multipage>();
