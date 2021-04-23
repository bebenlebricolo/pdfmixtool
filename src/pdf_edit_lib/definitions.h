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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <map>
#include <iostream>
#include <vector>

const double inch = 72;
const double cm = inch / 2.54;
const double mm = cm / 10;

struct PaperSize {
    double width;
    double height;
    std::string name;
    bool portrait;
};

const PaperSize paper_sizes[] = {
    {21.6, 27.9, "US letter", true},
    {84.1, 118.9, "A0", true},
    {59.4, 84.1, "A1", true},
    {42.0, 59.4, "A2", true},
    {29.7, 42.0, "A3", true},
    {21.0, 29.7, "A4", true},
    {14.8, 21.0, "A5", true},
    {10.5, 14.8, "A6", true},
    {7.4, 10.5, "A7", true},
    {5.2, 7.4, "A8", true},
    {3.7, 5.2, "A9", true},
    {2.6, 3.7, "A10", true},
};

struct Multipage {
    enum Alignment {
        Left=0,
        Right=1,
        Top=2,
        Bottom=3,
        Center=4
    };

    Multipage(const std::string &name="multipage profile",
              double page_width=21., double page_height=29.7,
              int rows=1, int columns=1, bool rtl=false,
              Alignment h_alignment=Multipage::Center,
              Alignment v_alignment=Multipage::Center,
              double margin_left=0., double margin_right=0.,
              double margin_top=0., double margin_bottom=0.,
              double spacing=0.) :
        name{name},
        page_width{page_width}, page_height{page_height},
        rows{rows}, columns{columns}, rtl{rtl},
        h_alignment{h_alignment}, v_alignment{v_alignment},
        margin_left{margin_left}, margin_right{margin_right},
        margin_top{margin_top}, margin_bottom{margin_bottom},
        spacing{spacing} {};

    std::string name;

    double  page_width;
    double  page_height;

    int rows;
    int columns;
    bool rtl;

    Alignment h_alignment;
    Alignment v_alignment;

    double margin_left;
    double margin_right;
    double margin_top;
    double margin_bottom;

    double spacing;
};

const Multipage multipage_defaults[] = {
    {
        "2x1, A4 portrait",
        21.0, 29.7,
        2, 1, false,
        Multipage::Center, Multipage::Center,
        0, 0, 0, 0, 0
    },
    {
        "1x2, A4 landscape",
        29.7, 21.0,
        1, 2, false,
        Multipage::Center, Multipage::Center,
        0, 0, 0, 0, 0
    },
    {
        "2x2, A4 portrait",
        21.0, 29.7,
        2, 2, false,
        Multipage::Center, Multipage::Center,
        0, 0, 0, 0, 0
    },
    {
        "2x2, A4 landscape",
        29.7, 21.0,
        2, 2, false,
        Multipage::Center, Multipage::Center,
        0, 0, 0, 0, 0
    }
};

// return true if the input string is valid
bool parse_output_pages_string(const std::string &str,
                               int n_pages,
                               std::vector<std::pair<int, int>> &intervals,
                               int &output_pages_count);

#endif // DEFINITIONS_H
