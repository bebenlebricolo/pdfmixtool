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

#include "pdf_editor.h"

#include <fstream>

#include <qpdf/QUtil.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageLabelDocumentHelper.hh>
#include <qpdf/QPDFAcroFormDocumentHelper.hh>
#include <qpdf/QPDFOutlineDocumentHelper.hh>

QPDFOutlineDocumentHelper::~QPDFOutlineDocumentHelper()
{

}

PdfEditor::PdfEditor() :
    m_old_locale{std::locale::global(std::locale::classic())},
    m_last_page{-1}
{
    // create empty PDF
    m_output_pdf = QPDF();
    m_output_pdf.emptyPDF();
}

unsigned int PdfEditor::add_file(const std::string &filename)
{
    for (unsigned int i = 0; i < m_input_filenames.size(); ++i)
        if (m_input_filenames[i] == filename)
            return i;

    m_input_filenames.push_back(filename);

    unsigned int file_id = m_input_files.size();

    m_input_files.push_back(QPDF());
    m_input_files[file_id].processFile(filename.c_str());
    // FIXME not necessary if input files are kept until write
    m_input_files[file_id].setImmediateCopyFrom(true);

    m_pages.push_back(
                QPDFPageDocumentHelper(m_input_files[file_id]).getAllPages());
    m_page_infos.push_back(std::map<int, PageInfo>{});

    QPDFOutlineDocumentHelper outline_helper(m_input_files[file_id]);
    QPDFObjectHandle root = m_input_files[file_id].getRoot();

    // flatten outlines tree
    m_flat_outlines.push_back(std::vector<FlatOutline>());
    m_add_flat_outlines(file_id, root, outline_helper.getTopLevelOutlines());

    // add document's info from the first input file
    if (file_id == 0 && m_input_files[file_id].getTrailer().hasKey("/Info"))
    {
        QPDFObjectHandle info = m_output_pdf.makeIndirectObject(
                    m_input_files[file_id].getTrailer().getKey("/Info"));
        m_output_pdf.getTrailer().replaceKey("/Info", info);
    }

    return file_id;
}

void PdfEditor::add_blank_pages(double width, double height, int count)
{
    QPDFPageDocumentHelper output_helper(m_output_pdf);

    for (int i = 0; i < count; ++i)
        output_helper.addPage(m_create_blank_page(width, height), false);

    m_last_page += count;
}

void PdfEditor::add_pages(unsigned int file_id,
                          int relative_rotation,
                          const PageLayout *layout,
                          const std::vector<std::pair<int, int>> &intervals,
                          const std::string &outline_entry)
{
    // if no intervals are provided, call this function with one interval
    // containing all the pages of the file_id pdf
    if (intervals.empty())
    {
        add_pages(file_id, relative_rotation, layout,
                  {{0, m_pages[file_id].size() - 1}}, outline_entry);
        return;
    }

    QPDFPageDocumentHelper output_helper(m_output_pdf);

    // add pages to the output file and add outlines to m_output_outlines

    // add parent outline for the block, if any
    unsigned int parent_outline_index;
    if (!outline_entry.empty())
    {        
        QPDFPageObjectHelper &page = m_pages[file_id][intervals[0].first];
        QPDFObjectHandle::Rectangle rect =
                page.getAttribute("/MediaBox", true).getArrayAsRectangle();

        FlatOutline outline;
        outline.title = outline_entry;
        outline.top = std::abs(rect.lly - rect.ury);
        outline.left = 0;
        outline.file_id = file_id;
        outline.page = intervals[0].first;

        parent_outline_index = m_output_outlines.size();
        m_output_outlines.push_back(outline);
    }

    int num_pages_per_paper;
    int sub_page_count{0};
    QPDFObjectHandle outer_page_obj;

    if (layout)
    {
        num_pages_per_paper = layout->pages.size();
        outer_page_obj = m_create_blank_page(layout->width, layout->height);

        output_helper.addPage(QPDFPageObjectHelper(outer_page_obj), false);

        QPDFPageObjectHelper(outer_page_obj).rotatePage(relative_rotation,
                                                        true);

        ++m_last_page;
    }

    // add pages from each interval and outlines poinintg to those pages
    for (unsigned int i = 0; i < intervals.size(); ++i)
    {
        // add pages
        for (int j = intervals[i].first; j <= intervals[i].second; ++j)
        {
            if (layout)
            {
                if (sub_page_count >= num_pages_per_paper)
                {
                    sub_page_count = 0;

                    outer_page_obj = m_create_blank_page(layout->width,
                                                         layout->height);

                    output_helper.addPage(QPDFPageObjectHelper(outer_page_obj), false);

                    QPDFPageObjectHelper(outer_page_obj).rotatePage(relative_rotation,
                                                                    true);

                    ++m_last_page;
                }

                if (j > -1)
                {
                    PageInfo pi;
                    pi.dest = m_last_page;
                    m_page_infos[file_id][j] = pi;

                    m_impose_page(outer_page_obj, file_id, j,
                                  layout->pages[sub_page_count]);
                }

                ++sub_page_count;
            }
            else
            {
                QPDFPageObjectHelper page = m_pages[file_id][j].shallowCopyPage();

                if (relative_rotation != 0)
                    page.rotatePage(relative_rotation, true);

                output_helper.addPage(page, false);

                ++m_last_page;

                PageInfo pi;
                pi.dest = m_last_page;
                m_page_infos[file_id][j] = pi;
            }
        }

        // add outlines
        unsigned int j = 0;
        while (j < m_flat_outlines[file_id].size())
        {
            if (m_flat_outlines[file_id][j].page >= intervals[i].first)
            {
                int depth = 0;

                while (j < m_flat_outlines[file_id].size() &&
                       (m_flat_outlines[file_id][j].page <= intervals[i].second
                        || m_flat_outlines[file_id][j].page == -1))
                {
                    FlatOutline outline = m_flat_outlines[file_id][j++];

                    if (outline.next_move == Move::up)
                    {
                        // add move-ups only if depth > 0
                        if (depth == 0)
                            continue;
                        else
                            --depth;
                    }
                    else if (outline.next_move == Move::down)
                        ++depth;

                    m_output_outlines.push_back(outline);
                }

                // go back to the starting level
                while (depth > 0)
                {
                    FlatOutline dummy_outline;
                    dummy_outline.page = -1;
                    dummy_outline.next_move = Move::up;
                    m_output_outlines.push_back(dummy_outline);

                    --depth;
                }
            }

            ++j;
        }
    }

    // set parent outline next_move based on whether any child outline was added
    if (!outline_entry.empty())
    {
        if (parent_outline_index == m_output_outlines.size() - 1)
            m_output_outlines[parent_outline_index].next_move = Move::next;
        else
        {
            m_output_outlines[parent_outline_index].next_move = Move::down;

            FlatOutline dummy_outline;
            dummy_outline.page = -1;
            dummy_outline.next_move = Move::up;
            m_output_outlines.push_back(dummy_outline);
        }
    }

    delete layout;
}

void PdfEditor::write(const std::string &output_filename)
{
    // add outlines to the output file
    m_build_outlines(m_output_outlines);

    // write the PDF file to memory first to prevent problems when saving to
    // one of the input files
    QPDFWriter writer(m_output_pdf);
    writer.setOutputMemory();
    writer.write();
    Buffer *buffer = writer.getBuffer();

    const char *buf = reinterpret_cast<const char *>(buffer->getBuffer());

    // write the PDF file to disk
    std::ofstream output_file_stream;
    output_file_stream.open(output_filename);
    output_file_stream.write(buf, buffer->getSize());
    output_file_stream.close();
    delete buffer;

    std::locale::global(m_old_locale);

    // FIXME clear object
}

void PdfEditor::m_add_flat_outlines(
        int file_id,
        QPDFObjectHandle &root,
        const std::vector<QPDFOutlineObjectHelper> &outlines)
{
    const std::vector<QPDFPageObjectHelper> &pages = m_pages[file_id];
    std::vector<FlatOutline> &flat_outlines = m_flat_outlines[file_id];

    int count = 0;

    for (QPDFOutlineObjectHelper outline : outlines)
    {
        FlatOutline flat_outline;
        flat_outline.file_id = file_id;
        flat_outline.title = outline.getTitle();

        flat_outline.top = 0;
        flat_outline.left = 0;

        // find destination page object and coordinates
        // FIXME submit a patch for QPDFOutlineObjectHelper::getDestPage()
        QPDFObjectHandle outline_obj = outline.getObjectHandle();
        QPDFObjectHandle dest_page = QPDFObjectHandle::newNull();
        QPDFObjectHandle dest = QPDFObjectHandle::newNull();
        if (outline_obj.hasKey("/Dest"))
        {
            dest = outline_obj.getKey("/Dest");
        }
        else if (outline_obj.hasKey("/A"))
        {
            QPDFObjectHandle action = outline_obj.getKey("/A");
            if (action.getKey("/S").getName() == "/GoTo" && action.hasKey("/D"))
            {
                dest = action.getKey("/D");

                // destination is a name: retrieve real destination
                if (dest.isString())
                {
                    QPDFObjectHandle tmp;
                    // PDF 1.1
                    if (root.hasKey("/Dests"))
                    {
                         tmp = root.getKey("/Dests")
                                 .getKey(dest.getUTF8Value());
                    }
                    // PDF 1.2
                    else if (root.hasKey("/Names"))
                    {
                        QPDFObjectHandle dests_root = root.getKey("/Names")
                                .getKey("/Dests");

                        // traverse nodes tree to find the named destination
                        tmp = m_get_key_in_name_tree(dests_root,
                                                     dest.getUTF8Value());
                    }
                    if (tmp.isArray())
                        dest = tmp;
                    else if (tmp.isDictionary())
                        dest = tmp.getKey("/D");
                }
            }
        }
        if (!dest.isNull())
        {
            if (dest.isArray())
            {
                dest_page = dest.getArrayItem(0);

                // find coordinates
                if (dest.getArrayItem(1).getName() == "/XYZ")
                {
                    if (!dest.getArrayItem(2).isNull())
                        flat_outline.left =
                                dest.getArrayItem(2).getNumericValue();
                    if (!dest.getArrayItem(3).isNull())
                        flat_outline.top =
                                dest.getArrayItem(3).getNumericValue();
                }
                else if (dest.getArrayItem(1).getName() == "/FitH")
                {
                    if (!dest.getArrayItem(2).isNull())
                        flat_outline.top =
                                dest.getArrayItem(2).getNumericValue();
                }
                else if (dest.getArrayItem(1).getName() == "/FitV")
                {
                    if (!dest.getArrayItem(2).isNull())
                        flat_outline.left =
                                dest.getArrayItem(2).getNumericValue();
                }
                else if (dest.getArrayItem(1).getName() == "/FitR")
                {
                    if (!dest.getArrayItem(2).isNull())
                        flat_outline.left =
                                dest.getArrayItem(2).getNumericValue();
                    if (!dest.getArrayItem(5).isNull())
                        flat_outline.top =
                                dest.getArrayItem(5).getNumericValue();
                }
                else if (dest.getArrayItem(1).getName() == "/FitBH")
                {
                    if (!dest.getArrayItem(2).isNull())
                        flat_outline.top =
                                dest.getArrayItem(2).getNumericValue();
                }
            }
        }

        // last try
        if (dest_page.isNull())
            dest_page = outline.getDestPage();

        // find destination page number
        flat_outline.page = -1;
        if (!dest_page.isNull())
        {
            QPDFObjGen og = dest_page.getObjGen();
            for (unsigned int j = 0; j < pages.size(); ++j)
            {
                if (pages[j].getObjectHandle().getObjGen() == og)
                {
                    flat_outline.page = j;
                    break;
                }
            }
        }

        if (outline.getKids().empty())
            flat_outline.next_move = Move::next;
        else
            flat_outline.next_move = Move::down;

        flat_outlines.push_back(flat_outline);

        m_add_flat_outlines(file_id, root, outline.getKids());

        ++count;
    }

    if (count > 0)
    {
        // add a dummy outline to mark the end of the level
        FlatOutline dummy_outline;
        dummy_outline.page = -1;
        dummy_outline.next_move = Move::up;
        flat_outlines.push_back(dummy_outline);
    }
}

int PdfEditor::m_build_outline_level(const std::vector<FlatOutline> &flat_outlines,
                                   QPDFObjectHandle &parent,
                                   unsigned int starting_index)
{
    unsigned int i = starting_index;

    QPDFObjectHandle last = QPDFObjectHandle::newNull();

    while (i < flat_outlines.size())
    {
        if (flat_outlines[i].next_move == Move::up)
        {
            ++i;
            break;
        }

        QPDFObjectHandle outline = QPDFObjectHandle::newDictionary();
        outline = m_output_pdf.makeIndirectObject(outline);
        outline.replaceKey("/Title", QPDFObjectHandle::newUnicodeString(flat_outlines[i].title));
        outline.replaceKey("/Parent", parent);
        m_set_outline_destination(outline, flat_outlines[i]);

        if (last.isNull())
        {
            parent.replaceKey("/First", outline);
        }
        else
        {
            last.replaceKey("/Next", outline);
            outline.replaceKey("/Prev", last);
        }

        last = outline;

        if (flat_outlines[i].next_move == Move::down)
            i = m_build_outline_level(flat_outlines, outline, i + 1);
        else if (flat_outlines[i].next_move == Move::next)
            ++i;
    }

    parent.replaceKey("/Last", last);

    return i;
}

void PdfEditor::m_build_outlines(const std::vector<FlatOutline> &flat_outlines)
{
    // initialize outlines dictionary
    QPDFObjectHandle outlines_root = QPDFObjectHandle::newDictionary();
    outlines_root.replaceKey("/Type", QPDFObjectHandle::newName("/Outlines"));
    outlines_root = m_output_pdf.makeIndirectObject(outlines_root);

    m_build_outline_level(flat_outlines, outlines_root, 0);

    if (outlines_root.hasKey("/First"))
        m_output_pdf.getRoot().replaceKey("/Outlines", outlines_root);
}

QPDFObjectHandle PdfEditor::m_create_blank_page(double width, double height)
{
    QPDFObjectHandle page = QPDFObjectHandle::newDictionary();
    page.replaceKey("/Type", QPDFObjectHandle::newName("/Page"));

    QPDFObjectHandle media_box = QPDFObjectHandle::newArray();
    media_box.appendItem(QPDFObjectHandle::newInteger(0));
    media_box.appendItem(QPDFObjectHandle::newInteger(0));
    media_box.appendItem(QPDFObjectHandle::newReal(width));
    media_box.appendItem(QPDFObjectHandle::newReal(height));
    page.replaceKey("/MediaBox", media_box);

    QPDFObjectHandle resources = QPDFObjectHandle::newDictionary();
    QPDFObjectHandle proc_set = QPDFObjectHandle::newArray();
    proc_set.appendItem(QPDFObjectHandle::newName("/PDF"));
    proc_set.appendItem(QPDFObjectHandle::newName("/Text"));
    proc_set.appendItem(QPDFObjectHandle::newName("/ImageB"));
    proc_set.appendItem(QPDFObjectHandle::newName("/ImageC"));
    proc_set.appendItem(QPDFObjectHandle::newName("/ImageI"));
    resources.replaceKey("/ProcSet", proc_set);
    page.replaceKey("/Resources", resources);

    return page;
}

void PdfEditor::m_set_outline_destination(QPDFObjectHandle &outline,
                                          const FlatOutline &flat_outline)
{
    PageInfo &pi = m_page_infos[flat_outline.file_id][flat_outline.page];
    QPDFPageObjectHelper page = m_output_pdf.getAllPages()[pi.dest];
    Point dest_point = pi.get_point_in_dest(
                flat_outline.left, flat_outline.top);

    // set destination
    QPDFObjectHandle dest = QPDFObjectHandle::newArray();
    dest.appendItem(page.getObjectHandle());
    dest.appendItem(QPDFObjectHandle::newName("/XYZ"));
    dest.appendItem(QPDFObjectHandle::newReal(dest_point.first));
    dest.appendItem(QPDFObjectHandle::newReal(dest_point.second));
    dest.appendItem(QPDFObjectHandle::newNull());
    outline.replaceKey("/Dest", dest);
}

void PdfEditor::m_impose_page(QPDFObjectHandle &outer_page_obj,
                              int file_id,
                              int page_id,
                              const Page &page_layout)
{
    QPDFPageObjectHelper &page = m_pages[file_id][page_id];

    QPDFPageObjectHelper outer_page(outer_page_obj);

    // handle alignment
    Point page_size = m_get_page_size(page);
    double scale_factor = std::min(
                page_layout.width / page_size.first,
                page_layout.height / page_size.second
                );

    double page_width = page_size.first * scale_factor;
    double page_height = page_size.second * scale_factor;

    double delta_x = 0, delta_y = 0;

    if (page_layout.h_alignment == Multipage::Center)
        delta_x = (page_layout.width - page_width) / 2;
    else if (page_layout.h_alignment == Multipage::Right)
        delta_x = page_layout.width - page_width;

    if (page_layout.v_alignment == Multipage::Center)
        delta_y = (page_layout.height - page_height) / 2;
    else if (page_layout.v_alignment == Multipage::Top)
        delta_y = page_layout.height - page_height;

    double x = page_layout.x + delta_x;
    double y = page_layout.y + delta_y;

    // Get form xobject for input page
    QPDFObjectHandle page_xobject = m_output_pdf.copyForeignObject(page.getFormXObjectForPage());

    // Find a unique resource name for the new form XObject
    QPDFObjectHandle resources = outer_page.getAttribute("/Resources", true);

    int min_suffix = 0;
    std::string name = resources.getUniqueResourceName("/Fx", min_suffix);

    std::string content = outer_page.placeFormXObject(
                page_xobject,
                name,
                QPDFObjectHandle::Rectangle(
                    x, y,
                    x + page_width, y + page_height),
                false,
                true,
                true);

    if (!content.empty())
    {
        // Append the content to the page's content.
        // Surround the original content with q...Q to the
        // new content from the page's original content.

        if (!resources.hasKey("/XObject"))
            resources.replaceKey("/XObject", QPDFObjectHandle::newDictionary());

        resources.getKey("/XObject").replaceKey(name, page_xobject);

        outer_page.addPageContents(QPDFObjectHandle::newStream(&m_output_pdf, "q\n"),
                                   true);
        outer_page.addPageContents(QPDFObjectHandle::newStream(&m_output_pdf, "\nQ\n" + content),
                                   false);
    }

    // update pageinfo
    PageInfo &pi = m_page_infos[file_id][page_id];
    pi.scale = scale_factor;
    pi.rot = page.getAttribute("/Rotate", true).getIntValue();
    pi.x0 = x;
    pi.y0 = y;
    if (pi.rot % 360 == 90)
    {
        pi.y0 += page_size.second * scale_factor;
    }
    else if (pi.rot % 360 == 180)
    {
        pi.x0 += page_size.first * scale_factor;
        pi.y0 += page_size.second * scale_factor;
    }
    else if (pi.rot % 360 == 90)
    {
        pi.x0 += page_size.first * scale_factor;
    }
}

PdfEditor::PageLayout::PageLayout(const Multipage &mp) :
    width{mp.page_width * cm}, height{mp.page_height * cm}
{
    double margin_top = mp.margin_top * cm;
    double margin_bottom = mp.margin_bottom * cm;
    double margin_left = mp.margin_left * cm;
    double margin_right = mp.margin_right * cm;
    double spacing = mp.spacing * cm;

    double subpage_width = (width -
                            margin_left - margin_right -
                            spacing * (mp.columns - 1)
                            ) / mp.columns;
    double subpage_height = (height -
                             margin_top - margin_bottom -
                             spacing * (mp.rows - 1)
                             ) / mp.rows;

    for (auto i = 0; i < mp.rows; ++i)
        for (auto j = 0; j < mp.columns; ++j)
        {
            PdfEditor::Page page;
            page.y = margin_bottom \
                    + (mp.rows - 1 - i) * (subpage_height + spacing);
            if (mp.rtl)
                page.x = margin_left \
                        + (mp.columns - 1 - j) * (subpage_width + spacing);
            else
                page.x = margin_left + j * (subpage_width + spacing);
            page.width = subpage_width;
            page.height = subpage_height;
            page.h_alignment = mp.h_alignment;
            page.v_alignment = mp.v_alignment;

            pages.push_back(page);
        }
}

QPDFObjectHandle PdfEditor::m_get_key_in_name_tree(QPDFObjectHandle &node,
                                                   const std::string &key)
{
    if (node.hasKey("/Kids"))
    {
        for (auto obj : node.getKey("/Kids").getArrayAsVector())
        {
            QPDFObjectHandle res = m_get_key_in_name_tree(obj, key);
            if (!res.isNull())
                return res;
        }
    }
    else
    {
        auto names = node.getKey("/Names").getArrayAsVector();
        for (size_t i{0}; i < names.size(); i += 2)
        {
            if (names[i].getUTF8Value() == key)
                return names[i + 1];
        }
    }

    return QPDFObjectHandle::newNull();
}

PdfEditor::Point PdfEditor::m_get_page_size(QPDFPageObjectHelper &page)
{
    // get rotation of the page
    long long page_rotation = 0;
    if (page.getAttribute("/Rotate", true).isInteger())
        page_rotation = page.getAttribute("/Rotate", true).getIntValue();

    // get size of the page
    QPDFObjectHandle::Rectangle rect =
            page.getAttribute("/MediaBox", true).getArrayAsRectangle();

    Point size;
    if (page_rotation % 180 == 90)
    {
        size.first = std::abs(rect.lly - rect.ury);
        size.second = std::abs(rect.llx - rect.urx);
    }
    else
    {
        size.first = std::abs(rect.llx - rect.urx);
        size.second = std::abs(rect.lly - rect.ury);
    }

    return size;
}
