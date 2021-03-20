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

#ifndef PDF_WRITER_H
#define PDF_WRITER_H

#include <functional>
#include <vector>

#include <qpdf/QPDFObjectHandle.hh>

#include "definitions.h"

struct Point {
    double top;
    double left;
};

// return coordinates for the destination in outlines so that the top left
// corner of the page is displayed in the top left corner of the window,
// independently on the rotation of the page
Point get_destination_coordinates(QPDFObjectHandle &page_obj);

void write_pdf(const Conf &conf, std::function<void(int)> &progress);

void write_alternate_mix(const std::vector<std::string> &input_filenames,
                         const std::string &output_filename,
                         const std::vector<bool> &reverse_order,
                         std::function<void (int)> &progress);

void write_booklet_pdf(const std::string &input_filename,
                       const std::string &output_filename,
                       bool right_side_binding,
                       bool back_cover,
                       std::function<void (int)> &progress);

void write_add_empty_pages(const std::string &input_filename,
                           const std::string &output_filename,
                           int count,
                           double page_width,
                           double page_height,
                           int location,
                           bool before,
                           std::function<void (int)> &progress);

void write_delete_pages(const std::string &input_filename,
                        const std::string &output_filename,
                        const std::vector<bool> &pages,
                        std::function<void (int)> &progress);

#endif // PDF_WRITER_H
