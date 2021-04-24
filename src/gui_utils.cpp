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

QSettings *settings = new QSettings("PDFMixTool", "pdfmixtool");
QMap<int, Multipage> multipages = QMap<int, Multipage>();
