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

#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <locale>

#include <podofo/podofo.h>
#include <qpdf/QUtil.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageLabelDocumentHelper.hh>
#include <qpdf/QPDFAcroFormDocumentHelper.hh>

#include "pdf_writer.h"
#include "pdf_info.h"

bool parse_output_pages_string(
        const std::string &str,
        int n_pages,
        std::vector<std::pair<int, int>> &intervals,
        int &output_pages_count)
{
    intervals.clear();
    output_pages_count = 0;

    // Invalid characters
    if (str.find_first_not_of("0123456789- ,") != std::string::npos)
        return false;

    // Void string
    if (str.find_first_not_of("- ,") == std::string::npos)
    {
        output_pages_count = n_pages;
        intervals.push_back(std::pair<int, int>(1, n_pages));
        return true;
    }

    // Parse string
    std::string::size_type cursor = str.find_first_not_of(" ,-");
    std::string::size_type interval_end = str.find_first_of(" ,", cursor);

    while (cursor < str.length())
    {
        // Single number
        if (str.find_first_of('-', cursor) >= interval_end)
        {
            std::string page_number = str.substr(cursor, interval_end - cursor);
            int num = std::stoi(page_number);
            intervals.push_back(std::pair<int, int>(num, num));
            output_pages_count++;

            cursor = str.find_first_not_of(" ,-", interval_end);
            interval_end = str.find_first_of(" ,", cursor);
        }
        // Interval
        else
        {
            std::string::size_type first_number_end =
                    str.find_first_of('-', cursor);
            std::string::size_type second_number_start =
                    str.find_first_not_of('-', first_number_end);
            if (
                    // Syntax error: no second number
                    second_number_start >= interval_end ||
                    // Syntax error: more '-' in one interval
                    str.find_first_of('-', second_number_start) < interval_end
                    )
            {
                return false;
            }
            else
            {
                int from = std::stoi(
                            str.substr(cursor, first_number_end - cursor));
                int to = std::stoi(
                            str.substr(second_number_start, interval_end));
                if (from > to || from < 1 || to > n_pages)
                    return false;
                else
                {
                    intervals.push_back(std::pair<int, int>(from, to));
                    output_pages_count += to - from + 1;

                    cursor = str.find_first_not_of(" ,-", interval_end);
                    interval_end = str.find_first_of(" ,", cursor);
                }
            }
        }
    }

    if (intervals.size() == 0)
        intervals.push_back(std::pair<int, int>(1, n_pages));

    return true;
}

void write_pdf(const Conf &conf, std::function<void (int)>& progress)
{
    std::locale old_locale = std::locale::global(std::locale::classic());

    QPDF output_file;
    output_file.emptyPDF();

    std::map<int, const std::string &> outlines;
    int outline_page = 0;

    for (std::vector<FileConf>::size_type i = 0; i < conf.files.size(); i++)
    {
        if (conf.files.at(i).outline_entry.size() > 0)
            outlines.insert(std::pair<int, const std::string &>(
                                outline_page,
                                conf.files.at(i).outline_entry));

        const Multipage *mp = conf.files.at(i).multipage;

        QPDF tmp_file;
        tmp_file.emptyPDF();
        tmp_file.setImmediateCopyFrom(true);

        QPDF input_file;
        input_file.processFile(conf.files.at(i).path.c_str());
        input_file.setImmediateCopyFrom(true);

        int output_pages_count;
        std::vector<std::pair<int, int>> intervals;
        parse_output_pages_string(conf.files.at(i).ouput_pages,
                                  input_file.getAllPages().size(),
                                  intervals,
                                  output_pages_count);
        if (mp == nullptr)
            outline_page += output_pages_count;

        // Add pages to the output file
        std::vector<std::pair<int, int>>::iterator it;
        for (it = intervals.begin(); it != intervals.end(); ++it)
        {
            std::vector<QPDFPageObjectHelper> const &pages =
                    QPDFPageDocumentHelper(input_file).getAllPages();

            for (unsigned int j = it->first - 1; j < it->second; j++)
            {
                QPDFPageObjectHelper page = pages.at(j);
                page = page.shallowCopyPage();

                if (mp == nullptr)
                {
                    // Rotate page
                    page.rotatePage(conf.files.at(i).rotation, true);
                    // Add page
                    QPDFPageDocumentHelper(output_file).addPage(page, false);
                }
                else
                {
                    // Rotate page
                    page.rotatePage(mp->rotation, true);
                    // Add page
                    QPDFPageDocumentHelper(tmp_file).addPage(page, false);
                }
            }
        }

        // Handle multipage options
        if (mp != nullptr)
        {
            // Get size of the first page
            double source_width;
            double source_height;

            std::vector<QPDFPageObjectHelper> pages =
                    QPDFPageDocumentHelper(tmp_file).getAllPages();

            int n_pages = pages.size();

            // Get rotation of the first page
            long long page_rotation = 0;
            if (pages.at(0).getAttribute("/Rotate", true).isInteger())
                page_rotation = pages.at(0).getAttribute("/Rotate", true)
                        .getIntValue();

            QPDFObjectHandle::Rectangle rect = pages.at(0)
                    .getAttribute("/MediaBox", true).getArrayAsRectangle();

            if (page_rotation % 180 == 90)
            {
                source_width = std::abs(rect.lly - rect.ury);
                source_height = std::abs(rect.llx - rect.urx);
            }
            else
            {
                source_height = std::abs(rect.lly - rect.ury);
                source_width = std::abs(rect.llx - rect.urx);
            }

            int plate_page_count = mp->rows * mp->columns;

            double dest_width = mp->page_width * cm;
            double dest_height = mp->page_height * cm;
            double margin_top = mp->margin_top * cm;
            double margin_bottom = mp->margin_bottom * cm;
            double margin_left = mp->margin_left * cm;
            double margin_right = mp->margin_right * cm;
            double spacing = mp->spacing * cm;

            double available_width = (dest_width -
                                      margin_left - margin_right -
                                      spacing * (mp->columns - 1)
                                      ) / mp->columns;

            double available_height = (dest_height -
                                       margin_top - margin_bottom -
                                       spacing * (mp->rows - 1)
                                       ) / mp->rows;

            double scale_factor = std::min(
                        available_width / source_width,
                        available_height / source_height
                        );

            double page_width = source_width * scale_factor;
            double page_height = source_height * scale_factor;

            // Alignment settings
            double delta_x = 0, delta_y = 0;

            if (mp->h_alignment == Multipage::Center)
                delta_x = (available_width - page_width) / 2;
            else if (mp->h_alignment == Multipage::Right)
                delta_x = available_width - page_width;

            if (mp->v_alignment == Multipage::Center)
                delta_y = (available_height - page_height) / 2;
            else if (mp->v_alignment == Multipage::Top)
                delta_y = available_height - page_height;

            // Create a blank page with podofo
            PoDoFo::PdfMemDocument podofo_blank_file;
            podofo_blank_file.CreatePage(
                        PoDoFo::PdfRect (0.0, 0.0, dest_width, dest_height));
            std::stringstream blank_file_buf;
            PoDoFo::PdfOutputDevice output_device(&blank_file_buf);
            podofo_blank_file.Write(&output_device);

            // Load the blank page with QPDF
            QPDF blank_file;
            blank_file.processMemoryFile("blank file",
                                         blank_file_buf.str().c_str(),
                                         blank_file_buf.str().size());
            blank_file.setImmediateCopyFrom(true);

            // Add pages
            int current_page = 0;

            while (current_page < n_pages)
            {
                QPDFPageDocumentHelper blank_helper(blank_file);
                QPDFPageObjectHelper blank_page =
                        blank_helper.getAllPages().at(0);
                blank_page = blank_page.shallowCopyPage();
                QPDFPageDocumentHelper(output_file).addPage(blank_page, false);

                QPDFPageObjectHelper out_page_helper =
                        QPDFPageDocumentHelper(output_file)
                        .getAllPages().back();

                for (int j = 0; j < plate_page_count; j++)
                {
                    if(current_page < n_pages)
                    {
                        // Calculate page position
                        int matrix_row, matrix_col;
                        if (mp->rotation % 180 == 0)
                        {
                            matrix_row = j / mp->columns;
                            matrix_col = j % mp->columns;
                        }
                        else
                        {
                            matrix_row = j % mp->rows;
                            matrix_col = mp->columns - 1 - (j / mp->rows);
                        }

                        double x = margin_left +
                                (available_width + spacing) * matrix_col +
                                delta_x;
                        double y = dest_height - margin_top -
                                (available_height + spacing) * matrix_row -
                                available_height + delta_y;

                        // Get form xobject for input page
                        QPDFPageObjectHelper in_page_helper =
                                QPDFPageDocumentHelper(tmp_file)
                                .getAllPages().at(current_page);

                        QPDFObjectHandle page_xobject =
                                output_file.copyForeignObject(
                                    in_page_helper.getFormXObjectForPage());

                        // Find a unique resource name for the new form XObject
                        QPDFObjectHandle resources = out_page_helper
                                .getAttribute("/Resources", true);

                        int min_suffix = current_page + 1;
                        std::string name = resources.getUniqueResourceName(
                                    "/Fx", min_suffix);

                        std::string content = out_page_helper.placeFormXObject(
                                    page_xobject,
                                    name,
                                    QPDFObjectHandle::Rectangle(
                                        x, y,
                                        x + page_width, y + page_height),
                                    false);

                        if (! content.empty())
                        {
                            // Append the content to the page's content.
                            // Surround the original content with q...Q to the
                            // new content from the page's original content.

                            resources.mergeResources(
                                        QPDFObjectHandle::parse(
                                            "<< /XObject << >> >>"));
                            resources.getKey("/XObject")
                                    .replaceKey(name, page_xobject);
                            out_page_helper.addPageContents(
                                        QPDFObjectHandle::newStream(
                                            &output_file, "q\n"),
                                        true);
                            out_page_helper.addPageContents(
                                        QPDFObjectHandle::newStream(
                                            &output_file, "\nQ\n" + content),
                                        false);
                        }
                    }

                    out_page_helper.rotatePage(conf.files.at(i).rotation,
                                               false);

                    ++current_page;
                }

                ++outline_page;
            }
        }

        progress(100 * i / (conf.files.size() + 1));
    }

    QPDFWriter writer(output_file);
    writer.setOutputMemory();
    writer.write();
    Buffer *buffer = writer.getBuffer();

    // Add outline with podofo
    PoDoFo::PdfMemDocument podofo_file;

    const char *buf = reinterpret_cast<const char *>(buffer->getBuffer());
    podofo_file.LoadFromBuffer(buf, buffer->getSize());

    PoDoFo::PdfOutlineItem *last =
            podofo_file.GetOutlines(true)->CreateRoot("");

    for (std::pair<int, const std::string &> entry : outlines)
        last = last->CreateNext(
                    entry.second,
                    PoDoFo::PdfDestination(podofo_file.GetPage(entry.first))
                    );

    podofo_file.Write(conf.output_path.c_str());

    delete buffer;

    progress(100);

    std::locale::global(old_locale);
}
