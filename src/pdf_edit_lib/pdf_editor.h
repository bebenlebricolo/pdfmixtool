/* Copyright (C) 2020-2021 Marco Scarpetta
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
#include <vector>
#include <cmath>

#include <qpdf/QPDF.hh>
#include <qpdf/QUtil.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageLabelDocumentHelper.hh>
#include <qpdf/QPDFOutlineDocumentHelper.hh>

#include "definitions.h"

class PdfEditor
{
public:
    // origin of the frame of reference is in the lower-left corner of the page
    class Page {
    public:
        double x, y, width, height;
        double crop_top, crop_bottom, crop_left, crop_right;
        Multipage::Alignment h_alignment{Multipage::Alignment::Center};
        Multipage::Alignment v_alignment{Multipage::Alignment::Center};
    };

    class PageLayout {
    public:
        PageLayout() {};
        PageLayout(const Multipage &mp);
        double width, height;
        std::vector<Page> pages;
    };

    PdfEditor();

    ~PdfEditor();

    // returns the file_id to be used in add_pages
    unsigned int add_file(const std::string &filename);

    void add_blank_pages(double width, double height, int count);

    // add pages from pdf denoted by file_id.
    // layout is deleted by this function.
    // intervals pages numbering starts from 0.
    // [-1, -1] intervals mean "skip position" when there is a multipage layout
    void add_pages(unsigned int file_id,
                   int relative_rotation = 0,
                   const PageLayout *layout = nullptr,
                   const std::vector<std::pair<int, int>> &intervals = {},
                   const std::string &outline_entry = {});

    void write(const std::string &output_filename);

private:
    using Point = std::pair<double, double>;

    enum class Move {
        up,
        down,
        next,
    };

    struct Dest {
        int file_id{-1};
        int page_id{-1};
        double x{0};
        double y{0};
    };

    struct FlatOutline {
        Move next_move;
        std::string title;
        Dest dest;
    };

    // represent an annotation
    struct Annotation {
        QPDFObjectHandle ann_obj;
        int orig_page_id;
        Dest dest;
    };

    struct PageInfo {
        Point get_point_in_dest(double x, double y)
        {
            double r = - M_PI / 180. * rot;
            double xr = std::cos(r) * x - std::sin(r) * y;
            double yr = std::sin(r) * x + std::cos(r) * y;
            double xo = x0 + xr * scale;
            double yo = y0 + yr * scale;
            return {xo, yo};
        }
        int dest{-1};
        int rot{0};
        double x0{0.};
        double y0{0.};
        double scale{1.};
    };

    std::locale m_old_locale;
    int m_last_page;

    std::vector<std::string> m_input_filenames;
    std::vector<QPDF *> m_input_files;
    std::vector<std::vector<QPDFPageObjectHelper>> m_pages;
    std::vector<std::vector<FlatOutline>> m_flat_outlines;
    std::map<int, std::vector<Annotation>> m_annots;
    std::vector<std::map<int, PageInfo>> m_page_infos;

    QPDF *m_output_pdf;
    std::vector<FlatOutline> m_output_outlines;

    Dest m_find_destination(int file_id, QPDFObjectHandle &obj);

    void m_add_flat_outlines(
            int file_id,
            const std::vector<QPDFOutlineObjectHelper> &outlines);

    static QPDFObjectHandle m_create_blank_page(double width, double height);

    int m_build_outline_level(const std::vector<FlatOutline> &flat_outlines,
                            QPDFObjectHandle &parent,
                            unsigned int starting_index);

    void m_build_outlines();

    void m_build_annotations();

    void m_set_outline_destination(QPDFObjectHandle &obj,
                                   const Dest &dest);

    // impose the page on the outer page and update its PageInfo
    void m_impose_page(QPDFObjectHandle &outer_page_obj,
                       int file_id,
                       int page_id,
                       const Page &page_layout);

    static QPDFObjectHandle m_get_key_in_name_tree(QPDFObjectHandle &node,
                                                   const std::string &key);

    // return the size of the given page, considering its rotation
    static Point m_get_page_size(QPDFPageObjectHelper &page);

    void m_clear();
};

#endif // PDFEDITOR_H
