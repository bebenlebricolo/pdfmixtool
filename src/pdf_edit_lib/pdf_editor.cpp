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
#include "qpdf_version.h"

QPDFOutlineDocumentHelper::~QPDFOutlineDocumentHelper()
{

}

PdfEditor::PdfEditor() :
    m_old_locale{std::locale::global(std::locale::classic())},
    m_last_page{-1}
{
    // create empty PDF
    m_output_pdf = new QPDF{};
    m_output_pdf->emptyPDF();
}

PdfEditor::~PdfEditor()
{
    m_clear();
}

unsigned int PdfEditor::add_file(const std::string &filename)
{
    for (unsigned int i = 0; i < m_input_filenames.size(); ++i)
        if (m_input_filenames[i] == filename)
            return i;

    m_input_filenames.push_back(filename);

    unsigned int file_id = m_input_files.size();

    m_input_files.push_back(new QPDF{});
    m_input_files[file_id]->processFile(filename.c_str());

    m_pages.push_back(
                QPDFPageDocumentHelper(*m_input_files[file_id]).getAllPages());
    m_page_infos.push_back(std::map<int, PageInfo>{});

    QPDFOutlineDocumentHelper outline_helper(*m_input_files[file_id]);

    // flatten outlines tree
    m_flat_outlines.push_back(std::vector<FlatOutline>());
    m_add_flat_outlines(file_id, outline_helper.getTopLevelOutlines());

    // add document's info from the first input file
    if (file_id == 0 && m_input_files[file_id]->getTrailer().hasKey("/Info"))
    {
        QPDFObjectHandle info = m_output_pdf->copyForeignObject(
                    m_input_files[file_id]->getTrailer().getKey("/Info"));
        m_output_pdf->getTrailer().replaceKey("/Info", info);
    }

    return file_id;
}

void PdfEditor::add_blank_pages(double width, double height, int count)
{
    QPDFPageDocumentHelper output_helper(*m_output_pdf);

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

    QPDFPageDocumentHelper output_helper(*m_output_pdf);

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
        outline.dest.y = std::abs(rect.lly - rect.ury);
        outline.dest.x = 0;
        outline.dest.file_id = file_id;
        outline.dest.page_id = intervals[0].first;

        parent_outline_index = m_output_outlines.size();
        m_output_outlines.push_back(outline);
    }

    int num_pages_per_paper{0};
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
            // create a new blank page if the current one is full
            if (layout == nullptr)
            {
                ++m_last_page;
            }
            else if (sub_page_count >= num_pages_per_paper)
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
                // add pageinfo
                PageInfo pi;
                pi.dest = m_last_page;
                m_page_infos[file_id][j] = pi;

                // save annotations in m_annots and remove them from the page
                QPDFPageObjectHelper &page = m_pages[file_id][j];
                auto annotations = page.getObjectHandle().getKey("/Annots");
                if (!annotations.isNull())
                {
                    if (m_annots.find(m_last_page) == m_annots.end())
                        m_annots[m_last_page] = {};

                    for (auto k{annotations.getArrayNItems() - 1}; k > -1; k--)
                    {
                        auto ann_obj = annotations.getArrayItem(k);
                        Annotation ann;
                        ann.orig_page_id = j;
                        ann.ann_obj = ann_obj;
                        ann.dest = m_find_destination(file_id, ann.ann_obj);
                        m_annots[m_last_page].push_back(ann);
                        annotations.eraseItem(k);
                    }
                }
            }
            if (layout)
            {
                if (j > -1)
                {
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
            }
        }

        // add outlines
        unsigned int j = 0;
        while (j < m_flat_outlines[file_id].size())
        {
            if (m_flat_outlines[file_id][j].dest.page_id >= intervals[i].first)
            {
                int depth = 0;

                while (j < m_flat_outlines[file_id].size() &&
                       (m_flat_outlines[file_id][j].dest.page_id <= intervals[i].second
                        || m_flat_outlines[file_id][j].dest.page_id == -1))
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
            dummy_outline.next_move = Move::up;
            m_output_outlines.push_back(dummy_outline);
        }
    }

    delete layout;
}

void PdfEditor::write(const std::string &output_filename)
{
    // add outlines to the output file
    m_build_outlines();

    // add links to inner pages
    m_build_annotations();

    // write the PDF file to memory first to prevent problems when saving to
    // one of the input files
    QPDFWriter writer(*m_output_pdf);
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

    // clear object and make it ready for a new run
    m_clear();
    m_output_pdf = new QPDF{};
    m_output_pdf->emptyPDF();
}

PdfEditor::Dest PdfEditor::m_find_destination(int file_id,
                                              QPDFObjectHandle &obj)
{
    Dest dest;
    dest.file_id = file_id;
    QPDFObjectHandle root = m_input_files[file_id]->getRoot();

    // find destination page object and coordinates
    // FIXME submit a patch for QPDFOutlineObjectHelper::getDestPage()
    QPDFObjectHandle dest_page = QPDFObjectHandle::newNull();
    QPDFObjectHandle dest_obj = QPDFObjectHandle::newNull();
    if (obj.hasKey("/Dest"))
    {
        dest_obj = obj.getKey("/Dest");
    }
    else if (obj.hasKey("/A"))
    {
        QPDFObjectHandle action = obj.getKey("/A");
        if (action.getKey("/S").getName() == "/GoTo" && action.hasKey("/D"))
        {
            dest_obj = action.getKey("/D");

            // destination is a name: retrieve real destination
            if (dest_obj.isString())
            {
                QPDFObjectHandle tmp;
                // PDF 1.1
                if (root.hasKey("/Dests"))
                {
                    tmp = root.getKey("/Dests")
                            .getKey(dest_obj.getUTF8Value());
                }
                // PDF 1.2
                else if (root.hasKey("/Names"))
                {
                    QPDFObjectHandle dests_root = root.getKey("/Names")
                            .getKey("/Dests");

                    // traverse nodes tree to find the named destination
                    tmp = m_get_key_in_name_tree(dests_root,
                                                 dest_obj.getUTF8Value());
                }
                if (tmp.isArray())
                    dest_obj = tmp;
                else if (tmp.isDictionary())
                    dest_obj = tmp.getKey("/D");
            }
        }
    }
    if (dest_obj.isArray())
    {
        dest_page = dest_obj.getArrayItem(0);

        // find coordinates
        if (dest_obj.getArrayItem(1).getName() == "/XYZ")
        {
            if (!dest_obj.getArrayItem(2).isNull())
                dest.x = dest_obj.getArrayItem(2).getNumericValue();
            if (!dest_obj.getArrayItem(3).isNull())
                dest.y = dest_obj.getArrayItem(3).getNumericValue();
        }
        else if (dest_obj.getArrayItem(1).getName() == "/FitH")
        {
            if (!dest_obj.getArrayItem(2).isNull())
                dest.y = dest_obj.getArrayItem(2).getNumericValue();
        }
        else if (dest_obj.getArrayItem(1).getName() == "/FitV")
        {
            if (!dest_obj.getArrayItem(2).isNull())
                dest.x = dest_obj.getArrayItem(2).getNumericValue();
        }
        else if (dest_obj.getArrayItem(1).getName() == "/FitR")
        {
            if (!dest_obj.getArrayItem(2).isNull())
                dest.x = dest_obj.getArrayItem(2).getNumericValue();
            if (!dest_obj.getArrayItem(5).isNull())
                dest.y = dest_obj.getArrayItem(5).getNumericValue();
        }
        else if (dest_obj.getArrayItem(1).getName() == "/FitBH")
        {
            if (!dest_obj.getArrayItem(2).isNull())
                dest.y = dest_obj.getArrayItem(2).getNumericValue();
        }
    }

    // find destination page number
    if (!dest_page.isNull())
    {
        QPDFObjGen og = dest_page.getObjGen();
        for (unsigned int j = 0; j < m_pages[file_id].size(); ++j)
        {
            if (m_pages[file_id][j].getObjectHandle().getObjGen() == og)
            {
                dest.page_id = j;
                break;
            }
        }
    }

    return dest;
}

void PdfEditor::m_add_flat_outlines(int file_id,
        const std::vector<QPDFOutlineObjectHelper> &outlines)
{
    std::vector<FlatOutline> &flat_outlines = m_flat_outlines[file_id];

    int count = 0;

    for (QPDFOutlineObjectHelper outline : outlines)
    {
        FlatOutline flat_outline;
        flat_outline.dest.file_id = file_id;
        flat_outline.title = outline.getTitle();
        QPDFObjectHandle obj = outline.getObjectHandle();
        flat_outline.dest = m_find_destination(file_id, obj);

        if (outline.getKids().empty())
            flat_outline.next_move = Move::next;
        else
            flat_outline.next_move = Move::down;

        flat_outlines.push_back(flat_outline);

        m_add_flat_outlines(file_id, outline.getKids());

        ++count;
    }

    if (count > 0)
    {
        // add a dummy outline to mark the end of the level
        FlatOutline dummy_outline;
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
        outline = m_output_pdf->makeIndirectObject(outline);
        outline.replaceKey("/Title", QPDFObjectHandle::newUnicodeString(flat_outlines[i].title));
        outline.replaceKey("/Parent", parent);
        m_set_outline_destination(outline, flat_outlines[i].dest);

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

void PdfEditor::m_build_outlines()
{
    // initialize outlines dictionary
    QPDFObjectHandle outlines_root = QPDFObjectHandle::newDictionary();
    outlines_root.replaceKey("/Type", QPDFObjectHandle::newName("/Outlines"));
    outlines_root = m_output_pdf->makeIndirectObject(outlines_root);

    m_build_outline_level(m_output_outlines, outlines_root, 0);

    if (outlines_root.hasKey("/First"))
        m_output_pdf->getRoot().replaceKey("/Outlines", outlines_root);
}

void PdfEditor::m_build_annotations()
{
    auto pages = m_output_pdf->getAllPages();

    for(auto it = m_annots.begin(); it != m_annots.end(); ++it)
    {
        int output_page = it->first;
        auto &annots = it->second;

        for (Annotation &ann : annots)
        {
            QPDFObjectHandle ann_obj =
                    m_output_pdf->copyForeignObject(ann.ann_obj);

            // update rect
            QPDFObjectHandle rect = ann_obj.getKey("/Rect");
            if (rect.isArray())
            {
                double x1 = rect.getArrayItem(0).getNumericValue();
                double y1 = rect.getArrayItem(1).getNumericValue();
                double x2 = rect.getArrayItem(2).getNumericValue();
                double y2 = rect.getArrayItem(3).getNumericValue();
                PageInfo &pi =
                        m_page_infos[ann.dest.file_id][ann.orig_page_id];
                Point p1 = pi.get_point_in_dest(x1, y1);
                Point p2 = pi.get_point_in_dest(x2, y2);
                QPDFObjectHandle new_rect{QPDFObjectHandle::newArray()};
                new_rect.appendItem(QPDFObjectHandle::newReal(p1.first));
                new_rect.appendItem(QPDFObjectHandle::newReal(p1.second));
                new_rect.appendItem(QPDFObjectHandle::newReal(p2.first));
                new_rect.appendItem(QPDFObjectHandle::newReal(p2.second));
                ann_obj.replaceKey("/Rect", new_rect);
            }

            // update page
            QPDFObjectHandle p = ann_obj.getKey("/P");
            if (p.isDictionary())
            {
                ann_obj.replaceKey("/P", pages[output_page]);
            }

            // update destination if it points to a page in the document
            if (ann.dest.page_id > -1)
            {
                auto pi_it =
                        m_page_infos[ann.dest.file_id].find(ann.dest.page_id);
                if (pi_it != m_page_infos[ann.dest.file_id].end())
                {
                    ann_obj.removeKey("/Dest");
                    ann_obj.removeKey("/A");
                    m_set_outline_destination(ann_obj, ann.dest);
                }
                else
                {
                    ann_obj.removeKey("/Dest");
                    ann_obj.removeKey("/A");
                }
            }
            else
                ann_obj.removeKey("/Dest");

            //add annotation to the page
            QPDFObjectHandle annots = pages[output_page].getKey("/Annots");
            if (!annots.isArray())
            {
                QPDFObjectHandle new_annots{QPDFObjectHandle::newArray()};
                pages[output_page].replaceKey("/Annots", new_annots);
                annots = pages[output_page].getKey("/Annots");
            }
            annots.appendItem(ann_obj);
        }
    }
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

void PdfEditor::m_set_outline_destination(QPDFObjectHandle &obj,
                                          const Dest &dest)
{
    if (dest.page_id > -1)
    {
        PageInfo &pi = m_page_infos[dest.file_id][dest.page_id];
        QPDFPageObjectHelper page = m_output_pdf->getAllPages()[pi.dest];
        Point dest_point = pi.get_point_in_dest(dest.x, dest.y);

        // set destination
        QPDFObjectHandle dest_obj = QPDFObjectHandle::newArray();
        dest_obj.appendItem(page.getObjectHandle());
        dest_obj.appendItem(QPDFObjectHandle::newName("/XYZ"));
        dest_obj.appendItem(QPDFObjectHandle::newReal(dest_point.first));
        dest_obj.appendItem(QPDFObjectHandle::newReal(dest_point.second));
        dest_obj.appendItem(QPDFObjectHandle::newNull());
        obj.replaceKey("/Dest", dest_obj);
    }
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
    QPDFObjectHandle page_xobject = m_output_pdf->copyForeignObject(page.getFormXObjectForPage());

    // Find a unique resource name for the new form XObject
    QPDFObjectHandle resources = outer_page.getAttribute("/Resources", true);

    int min_suffix = 0;
    std::string name = resources.getUniqueResourceName("/Fx", min_suffix);

#if QPDF_VERSION_MAJOR < 10
    std::string content = outer_page.placeFormXObject(
                page_xobject,
                name,
                QPDFObjectHandle::Rectangle(
                    x, y,
                    x + page_width, y + page_height),
                false);
#else
    std::string content = outer_page.placeFormXObject(
                page_xobject,
                name,
                QPDFObjectHandle::Rectangle(
                    x, y,
                    x + page_width, y + page_height),
                false,
                true,
                true);
#endif

    if (!content.empty())
    {
        // Append the content to the page's content.
        // Surround the original content with q...Q to the
        // new content from the page's original content.

        if (!resources.hasKey("/XObject"))
            resources.replaceKey("/XObject", QPDFObjectHandle::newDictionary());

        resources.getKey("/XObject").replaceKey(name, page_xobject);

        outer_page.addPageContents(QPDFObjectHandle::newStream(m_output_pdf, "q\n"),
                                   true);
        outer_page.addPageContents(QPDFObjectHandle::newStream(m_output_pdf, "\nQ\n" + content),
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

void PdfEditor::m_clear()
{
    m_last_page = -1;
    m_input_filenames.clear();
    m_pages.clear();
    m_flat_outlines.clear();
    m_annots.clear();
    m_page_infos.clear();
    m_output_outlines.clear();

    for (auto input_file : m_input_files)
        delete input_file;
    m_input_files.clear();

    delete m_output_pdf;
}
