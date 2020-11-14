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

#ifndef PDFEDITOR_H
#define PDFEDITOR_H

#include <string>
#include <functional>
#include <vector>

#include <qpdf/QPDF.hh>
#include <qpdf/QUtil.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageLabelDocumentHelper.hh>
#include <qpdf/QPDFOutlineDocumentHelper.hh>

struct Rect {
    double x, y, width, height;
};

struct PageLocation {
    Rect crop_box;
    Rect location;
};

class PdfEditor
{
public:
    static bool parse_output_pages_string(const std::string &str,
                                          int n_pages,
                                          std::vector<std::pair<int, int>> &intervals,
                                          int &output_pages_count);

    PdfEditor();

    unsigned int add_file(const std::string &filename);

    void add_pages(unsigned int file_id,
                   const std::vector<std::pair<int, int>> &intervals = {},
                   int relative_rotation = 0,
                   const std::string &outline_entry = {},
                   const std::vector<PageLocation> &layout = {});

    void add_blank_pages(double width, double height, int count);

    void write(const std::string &output_filename,
               std::function<void(int)> &progress);

private:
    enum Move {
        up,
        down,
        next,
    };

    struct FlatOutline {
        Move next_move;
        std::string title;
        int page;
        double top;
        double left;
    };

    static void add_flatten_outlines(const std::vector<QPDFPageObjectHelper> &pages,
                                     const std::vector<QPDFOutlineObjectHelper> &outlines,
                                     std::vector<FlatOutline> &flat_outlines);

    static QPDFObjectHandle m_create_blank_page(double width, double height);

    int build_outline_level(const std::vector<FlatOutline> &flat_outlines,
                            QPDFObjectHandle &parent,
                            unsigned int starting_index);

    void m_build_outlines(const std::vector<FlatOutline> &flat_outlines);

    void m_set_outline_destination(QPDFObjectHandle &outline,
                                   unsigned int page_index);

    struct BlankPages {
        double width;
        double height;
        int count;
    };

    struct Rotation {
        int first_page;
        int last_page;
        int relative_rotation;
    };

    struct FromFile {
        unsigned int id;
        std::vector<std::pair<int, int>> intervals;
        int relative_rotation;
        std::string outline_entry;
        std::vector<PageLocation> layout;
    };

    struct BlockPointer {
        enum Type {
            BlankPages,
            FromFile
        };

        Type type;
        void *p;
    };

    std::vector<BlockPointer> m_blocks;

    std::vector<std::string> m_input_filenames;

    QPDF m_output_pdf;

    QPDFObjectHandle m_last_outline;
    QPDFObjectHandle m_last_first_level_outline;
};

#endif // PDFEDITOR_H
