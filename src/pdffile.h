/* Copyright (C) 2017 Marco Scarpetta
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

#ifndef PDFFILE_H
#define PDFFILE_H

#include <podofo/podofo.h>
#include <string>
#include <list>

enum class ErrorType {
    invalid_interval,
    invalid_char,
    page_out_of_range
};

struct Error {
    Error(ErrorType type, const std::string &data) : type(type), data(data) {}
    ErrorType type;
    std::string data;
};

struct PoDoFoFile {
    PoDoFo::PdfMemDocument *file;
    int ref_count;
};

class PdfFile
{
public:
    PdfFile();
    PdfFile(const std::string &filename);
    PdfFile(PdfFile &pdf_file);
    ~PdfFile();

    const std::string &filename();

    int page_count();

    std::list<Error *> *set_pages_filter_from_string(const std::string &str);

    Error *add_pages_filter(int from, int to);

    void set_rotation(int rotation);

    void run(PoDoFo::PdfMemDocument *output_file);

private:
    std::string m_filename;
    PoDoFoFile *m_podofo_file;

    std::list<std::pair<int, int>> m_filters;
    int m_rotation;
};

#endif // PDFFILE_H