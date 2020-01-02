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

#include <iostream>
#include <cmath>

#include "pdf_info.h"
#include "definitions.h"

PdfInfo::PdfInfo(const std::string &path)
{
    qpdf.processFile(path.c_str());

    std::vector<QPDFPageObjectHelper> pages =
            QPDFPageDocumentHelper(qpdf).getAllPages();

    m_n_pages = pages.size();

    // Get rotation of the first page
    long long page_rotation = 0;
    if (pages.at(0).getAttribute("/Rotate", true).isInteger())
        page_rotation = pages.at(0).getAttribute("/Rotate", true).getIntValue();

    // Get size of the first page
    QPDFObjectHandle::Rectangle rect =
            pages.at(0).getAttribute("/MediaBox", true).getArrayAsRectangle();

    if (page_rotation % 180 == 90)
    {
        m_page_width = std::round(std::abs(rect.lly - rect.ury) / mm);
        m_page_height = std::round(std::abs(rect.llx - rect.urx) / mm);
    }
    else
    {
        m_page_width = std::round(std::abs(rect.llx - rect.urx) / mm);
        m_page_height = std::round(std::abs(rect.lly - rect.ury) / mm);
    }

    m_page_width = m_page_width / 10;
    m_page_height = m_page_height / 10;

    for (const PaperSize &size : paper_sizes)
    {
        if (std::abs(m_page_width - size.width) < 0.05 &&
                std::abs(m_page_height - size.height) < 0.05)
        {
            m_paper_size = size.name;
            m_is_portrait = true;
            return;
        }
        else if (std::abs(m_page_width - size.height) < 0.05 &&
                 std::abs(m_page_height - size.width) < 0.05)
        {
            m_paper_size = size.name;
            m_is_portrait = false;
            return;
        }
    }
}

double PdfInfo::width()
{
    return m_page_width;
}

double PdfInfo::height()
{
    return  m_page_height;
}

const std::string &PdfInfo::paper_size()
{
    return m_paper_size;
}

bool PdfInfo::is_portrait()
{
    return m_is_portrait;
}

int PdfInfo::n_pages()
{
    return m_n_pages;
}
