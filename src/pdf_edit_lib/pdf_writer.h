/* Copyright (C) 2019 Marco Scarpetta
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

#ifndef PDF_WRITER_H
#define PDF_WRITER_H

#include <functional>
#include <vector>

#include "definitions.h"

// return true if the input string is valid
bool parse_output_pages_string(
        const std::string &str,
        int n_pages,
        std::vector<std::pair<int, int>> &intervals,
        int &output_pages_count);

void write_pdf(const Conf &conf, std::function<void(int)>& progress);

#endif // PDF_WRITER_H
