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
    // FIXME not necessary if input files are keep until write
    m_input_files[file_id].setImmediateCopyFrom(true);

    m_pages.push_back(
                QPDFPageDocumentHelper(m_input_files[file_id]).getAllPages());

    QPDFOutlineDocumentHelper outline_helper(m_input_files[file_id]);
    QPDFObjectHandle root = m_input_files[file_id].getRoot();

    // flatten outlines tree
    m_flat_outlines.push_back(std::vector<FlatOutline>());
    m_add_flatten_outlines(root,
                           m_pages[file_id],
                           outline_helper.getTopLevelOutlines(),
                           m_flat_outlines[file_id]);

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
        FlatOutline outline;
        outline.title = outline_entry;
        outline.top = 0;
        outline.left = 0;
        outline.page = m_last_page + 1;

        parent_outline_index = m_output_outlines.size();
        m_output_outlines.push_back(outline);
    }

    int num_pages_per_paper;
    int sub_page_count;
    QPDFObjectHandle outer_page_obj;

    if (layout)
    {
        num_pages_per_paper = layout->pages.size();
        sub_page_count = 0;
        outer_page_obj = m_create_blank_page(layout->width, layout->height);

        output_helper.addPage(QPDFPageObjectHelper(outer_page_obj), false);
    }

    int page_count = 0;

    // add pages from each interval and outlines poinintg to those pages
    for (unsigned int i = 0; i < intervals.size(); ++i)
    {
        // add pages
        int initial_page_count = page_count;
        int initial_sub_page_count = sub_page_count;

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

                    ++page_count;
                }

                if (j > -1)
                {
                    const Page &page_pos = layout->pages[sub_page_count++];
                    QPDFPageObjectHelper page = m_pages[file_id][j];
                    m_impose_page(outer_page_obj, page, page_pos.relative_rotation,
                                  page_pos.x, page_pos.y, page_pos.width, page_pos.height);
                }

                QPDFPageObjectHelper(outer_page_obj).rotatePage(relative_rotation,
                                                                true);
            }
            else
            {
                QPDFPageObjectHelper page = m_pages[file_id][j].shallowCopyPage();

                if (relative_rotation != 0)
                    page.rotatePage(relative_rotation, true);

                output_helper.addPage(page, false);

                ++page_count;
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

                    // update destination page number
                    if (outline.page != -1)
                    {
                        if (layout)
                            outline.page = m_last_page + 1 + initial_page_count +
                                    (outline.page - intervals[i].first +
                                     initial_sub_page_count) / num_pages_per_paper;
                        else
                            outline.page = m_last_page + 1 + outline.page -
                                    intervals[i].first;
                    }

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

    m_last_page += page_count;

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

void PdfEditor::m_add_flatten_outlines(
        QPDFObjectHandle &root,
        const std::vector<QPDFPageObjectHelper> &pages,
        const std::vector<QPDFOutlineObjectHelper> &outlines,
        std::vector<FlatOutline> &flat_outlines)
{
    int count = 0;

    for (QPDFOutlineObjectHelper outline : outlines)
    {
        FlatOutline flat_outline;
        flat_outline.title = outline.getTitle();

        // FIXME compute correct top-left point
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
                                 .getKey(dest.getStringValue());
                    }
                    // PDF 1.2
                    else if (root.hasKey("/Names"))
                    {
                        QPDFObjectHandle dests_root = root.getKey("/Names")
                                .getKey("/Dests");

                        // traverse nodes tree to find the named destination
                        tmp = m_get_key_in_name_tree(dests_root,
                                                     dest.getStringValue());
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

        m_add_flatten_outlines(root, pages, outline.getKids(), flat_outlines);

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
        if (flat_outlines[i].page != -1)
            m_set_outline_destination(outline, flat_outlines[i].page);

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
                                          unsigned int page_index)
{
    QPDFPageObjectHelper page = m_output_pdf.getAllPages()[page_index];

    // get rotation of the page
    long long page_rotation = 0;
    if (page.getAttribute("/Rotate", true).isInteger())
        page_rotation = page.getAttribute("/Rotate", true).getIntValue();

    // get the coordinates
    QPDFObjectHandle::Rectangle rect =
            page.getAttribute("/MediaBox", true).getArrayAsRectangle();

    double height = rect.ury - rect.lly;
    double width = rect.urx - rect.llx;
    double top;
    double left;

    switch (page_rotation)
    {
    case 90:
        top = 0;
        left = 0;
        break;
    case 180:
        top = 0;
        left = width;
        break;
    case 270:
        top = height;
        left = width;
        break;
    default:
        top = height;
        left = 0;
        break;
    }

    // set destination
    QPDFObjectHandle dest = QPDFObjectHandle::newArray();
    dest.appendItem(page.getObjectHandle());
    dest.appendItem(QPDFObjectHandle::newName("/XYZ"));
    dest.appendItem(QPDFObjectHandle::newReal(left));
    dest.appendItem(QPDFObjectHandle::newReal(top));
    dest.appendItem(QPDFObjectHandle::newNull());
    outline.replaceKey("/Dest", dest);
}

void PdfEditor::m_impose_page(QPDFObjectHandle &outer_page_obj,
                              QPDFPageObjectHelper &page,
                              int relative_rotation,
                              double x,
                              double y,
                              double width,
                              double height)
{
    QPDFPageObjectHelper outer_page(outer_page_obj);

    page.rotatePage(relative_rotation, true);

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
                    x + width, y + height),
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
}

PdfEditor::PageLayout::PageLayout(const Multipage &mp) :
    width{mp.page_width}, height{mp.page_height}
{
    double subpage_width = width / mp.columns;
    double subpage_height = height / mp.rows;

    for (auto i = 0; i < mp.rows; ++i)
        for (auto j = 0; j < mp.columns; ++j)
        {
            PdfEditor::Page page;
            page.y = (mp.rows - 1 - i) * subpage_height;
            page.x = j * subpage_width;
            page.width = subpage_width;
            page.height = subpage_height;
            page.relative_rotation = mp.rotation;

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
            if (names[i].getStringValue() == key)
                return names[i + 1];
        }
    }

    return QPDFObjectHandle::newNull();
}
