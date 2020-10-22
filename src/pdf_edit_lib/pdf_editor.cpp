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

PdfEditor::PdfEditor()
{

}

unsigned int PdfEditor::add_file(const std::string &filename)
{
    for (unsigned int i = 0; i < m_input_filenames.size(); ++i)
        if (m_input_filenames[i] == filename)
            return i;

    m_input_filenames.push_back(filename);
    return m_input_filenames.size() - 1;
}

void PdfEditor::add_pages(unsigned int file_id,
               const std::vector<std::pair<int, int> > &intervals,
               int relative_rotation,
               const std::string &outline_entry,
               const std::vector<PageLocation> &layout)
{
    FromFile *block = new FromFile;

    block->id = file_id;
    block->intervals = intervals;
    block->relative_rotation = relative_rotation;
    block->outline_entry = outline_entry;
    block->layout = layout;

    BlockPointer p = {BlockPointer::FromFile, block};

    m_blocks.push_back(p);
}

void PdfEditor::add_blank_pages(double width, double height, int count)
{
    BlankPages *block = new BlankPages;

    block->width = width;
    block->height = height;
    block->count = count;

    BlockPointer p = {BlockPointer::BlankPages, block};

    m_blocks.push_back(p);
}

void PdfEditor::write(const std::string &output_filename,
                      std::function<void (int)> &progress)
{
    std::locale old_locale = std::locale::global(std::locale::classic());

    // load input files
    std::vector<QPDF> input_files;
    std::vector<std::vector<QPDFPageObjectHelper>> pages;
    std::vector<QPDFOutlineDocumentHelper> outline_helpers;
    std::vector<std::vector<FlatOutline>> flat_outlines;

    for (unsigned int i = 0; i < m_input_filenames.size(); ++i)
    {
        input_files.push_back(QPDF());
        input_files[i].processFile(m_input_filenames[i].c_str());
        input_files[i].setImmediateCopyFrom(true);

        pages.push_back(QPDFPageDocumentHelper(input_files[i]).getAllPages());

        outline_helpers.push_back(QPDFOutlineDocumentHelper(input_files[i]));

        // flatten outlines tree
        flat_outlines.push_back(std::vector<FlatOutline>());
        add_flatten_outlines(pages[i], outline_helpers[i].getTopLevelOutlines(), flat_outlines[i]);
    }

    // create empty PDF
    m_output_pdf = QPDF();
    m_output_pdf.emptyPDF();

    // add document's info from the first input file
    if (input_files[0].getTrailer().hasKey("/Info"))
    {
        QPDFObjectHandle info = m_output_pdf.makeIndirectObject(
                    input_files[0].getTrailer().getKey("/Info"));
        m_output_pdf.getTrailer().replaceKey("/Info", info);
    }

    // outlines of the output file will be added to this vector (in flattened version)
    std::vector<FlatOutline> output_outlines;

    // add pages from each block
    QPDFPageDocumentHelper output_helper(m_output_pdf);

    int last_page = -1;

    for (BlockPointer block_pointer : m_blocks)
    {
        switch (block_pointer.type)
        {
        case BlockPointer::BlankPages: {
            BlankPages *block =
                    static_cast<BlankPages *>(block_pointer.p);

            for (int i = 0; i < block->count; ++i)
                output_helper.addPage(
                            m_create_blank_page(block->width, block->height),
                            false);

            last_page += block->count;

            break;
        }
        case BlockPointer::FromFile: {
            FromFile *block = static_cast<FromFile *>(block_pointer.p);

            // add pages to the output file and add outlines to output_outlines
            int page_count = 0;

            switch (block->layout.size())
            {
            case 0: // simple case: only rotation
            {
                // add parent outline for the block, if any
                unsigned int parent_outline_index;
                if (!block->outline_entry.empty())
                {
                    FlatOutline outline;
                    outline.title = block->outline_entry;
                    outline.top = 0;
                    outline.left = 0;
                    outline.page = last_page + 1;

                    parent_outline_index = output_outlines.size();
                    output_outlines.push_back(outline);
                }

                // add pages from each interval and outlines poinintg to those pages
                for (unsigned int i = 0; i < block->intervals.size(); i++)
                {
                    // add pages
                    for (int j = block->intervals[i].first; j <= block->intervals[i].second; ++j)
                    {
                        QPDFPageObjectHelper page = pages[block->id][j].shallowCopyPage();

                        if (block->relative_rotation != 0)
                            page.rotatePage(block->relative_rotation, true);

                        output_helper.addPage(page, false);

                        ++page_count;
                    }

                    // add outlines
                    unsigned int j = 0;
                    while (j < flat_outlines[block->id].size())
                    {
                        if (flat_outlines[block->id][j].page >= block->intervals[i].first)
                        {
                            int depth = 0;

                            while (j < flat_outlines[block->id].size() &&
                                   (flat_outlines[block->id][j].page <= block->intervals[i].second
                                    || flat_outlines[block->id][j].page == -1))
                            {
                                FlatOutline outline = flat_outlines[block->id][j++];

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
                                    outline.page = last_page + 1 + outline.page -
                                            block->intervals[i].first;

                                output_outlines.push_back(outline);
                            }

                            // go back to the starting level
                            while (depth > 0)
                            {
                                FlatOutline dummy_outline;
                                dummy_outline.page = -1;
                                dummy_outline.next_move = Move::up;
                                output_outlines.push_back(dummy_outline);

                                --depth;
                            }
                        }

                        ++j;
                    }
                }

                // set parent outline next_move based on whether any child outline was added
                if (!block->outline_entry.empty())
                {
                    if (parent_outline_index == output_outlines.size() - 1)
                        output_outlines[parent_outline_index].next_move = Move::next;
                    else
                    {
                        output_outlines[parent_outline_index].next_move = Move::down;

                        FlatOutline dummy_outline;
                        dummy_outline.page = -1;
                        dummy_outline.next_move = Move::up;
                        output_outlines.push_back(dummy_outline);
                    }
                }

                break;
            }
            case 1: // more complex case: alter crop box and resize
            {
                break;
            }
            default: // page composition
            {
                break;
            }
            }

            last_page += page_count;

            break;
        }
        }
    }

    // add outlines to the output file
    m_build_outlines(output_outlines);

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

    progress(100);

    std::locale::global(old_locale);
}

void PdfEditor::add_flatten_outlines(const std::vector<QPDFPageObjectHelper> &pages,
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

        // find destination page number
        flat_outline.page = -1;
        QPDFObjectHandle dest_page = outline.getDestPage();
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

        add_flatten_outlines(pages, outline.getKids(), flat_outlines);

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

int PdfEditor::build_outline_level(const std::vector<FlatOutline> &flat_outlines,
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
            i = build_outline_level(flat_outlines, outline, i + 1);
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

    build_outline_level(flat_outlines, outlines_root, 0);

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
    proc_set.appendItem(QPDFObjectHandle::newName("PDF"));
    proc_set.appendItem(QPDFObjectHandle::newName("Text"));
    proc_set.appendItem(QPDFObjectHandle::newName("ImageB"));
    proc_set.appendItem(QPDFObjectHandle::newName("ImageC"));
    proc_set.appendItem(QPDFObjectHandle::newName("ImageI"));
    resources.replaceKey("ProcSet", proc_set);
    page.replaceKey("Resources", resources);

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
