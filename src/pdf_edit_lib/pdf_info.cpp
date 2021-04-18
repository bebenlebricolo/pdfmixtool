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

#include <iostream>
#include <cmath>

#include "pdf_info.h"
#include "definitions.h"

PdfInfo::PdfInfo() :
    m_filename(""),
    m_page_width(0),
    m_page_height(0),
    m_n_pages(0),
    m_paper_size(""),
    m_is_portrait(true)
{

}

PdfInfo::PdfInfo(const std::string &filename) :
    m_creation_date{default_datetime},
    m_mod_date{default_datetime}
{
    m_filename = filename;

    QPDF pdf_file;
    pdf_file.processFile(filename.c_str());

    std::vector<QPDFPageObjectHelper> pages =
            QPDFPageDocumentHelper(pdf_file).getAllPages();

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
            break;
        }
        else if (std::abs(m_page_width - size.height) < 0.05 &&
                 std::abs(m_page_height - size.width) < 0.05)
        {
            m_paper_size = size.name;
            m_is_portrait = false;
            break;
        }
    }

    // get document info
    QPDFObjectHandle doc_info = pdf_file.getTrailer().getKey("/Info");
    if (doc_info.isDictionary())
    {
        if (doc_info.hasKey("/Title"))
            m_title = doc_info.getKey(("/Title")).getUTF8Value();

        if (doc_info.hasKey("/Author"))
            m_author = doc_info.getKey(("/Author")).getUTF8Value();

        if (doc_info.hasKey("/Subject"))
            m_subject = doc_info.getKey(("/Subject")).getUTF8Value();

        if (doc_info.hasKey("/Keywords"))
            m_keywords = doc_info.getKey(("/Keywords")).getUTF8Value();

        if (doc_info.hasKey("/Creator"))
            m_creator = doc_info.getKey(("/Creator")).getUTF8Value();

        if (doc_info.hasKey("/Producer"))
            m_producer = doc_info.getKey(("/Producer")).getUTF8Value();

        if (doc_info.hasKey("/CreationDate"))
        {
            m_creation_date = PdfInfo::string_to_datetime(
                        doc_info.getKey(("/CreationDate")).getUTF8Value());
            m_has_creation_date = true;
        }
        else
        {
            m_has_creation_date = false;
        }

        if (doc_info.hasKey("/ModDate"))
        {
            m_mod_date = PdfInfo::string_to_datetime(
                        doc_info.getKey(("/ModDate")).getUTF8Value());
            m_has_mod_date = true;
        }
        else
        {
            m_has_mod_date = false;
        }
    }
}

const std::string &PdfInfo::filename() const
{
    return m_filename;
}

double PdfInfo::width() const
{
    return m_page_width;
}

double PdfInfo::height() const
{
    return  m_page_height;
}

const std::string &PdfInfo::paper_size() const
{
    return m_paper_size;
}

bool PdfInfo::is_portrait() const
{
    return m_is_portrait;
}

int PdfInfo::n_pages() const
{
    return m_n_pages;
}

const std::string &PdfInfo::title() const
{
    return m_title;
}
const std::string &PdfInfo::author() const
{
    return m_author;
}

const std::string &PdfInfo::subject() const
{
    return m_subject;
}

const std::string &PdfInfo::keywords() const
{
    return m_keywords;
}

const std::string &PdfInfo::creator() const
{
    return m_creator;
}

const std::string &PdfInfo::producer() const
{
    return m_producer;
}

const std::tm &PdfInfo::creation_date() const
{
    return m_creation_date;
}

bool PdfInfo::has_creation_date() const
{
    return m_has_creation_date;
}

const std::tm &PdfInfo::mod_date() const
{
    return m_mod_date;
}

bool PdfInfo::has_mod_date() const
{
    return m_has_mod_date;
}

std::tm PdfInfo::string_to_datetime(const std::string &str)
{
    std::istringstream ss{str};

    std::tm datetime{};

    try
    {
        datetime.tm_zone = "UTC";
        datetime.tm_year = std::stoi(str.substr(2, 4)) - 1900;
        datetime.tm_mon = std::stoi(str.substr(6, 2)) - 1;
        datetime.tm_mday = std::stoi(str.substr(8, 2));
        datetime.tm_hour = std::stoi(str.substr(10, 2));
        datetime.tm_min = std::stoi(str.substr(12, 2));
        datetime.tm_sec = std::stoi(str.substr(14, 2));

        if (str.at(16) != 'Z')
        {
            datetime.tm_hour -= std::stoi(str.substr(16, 3));
        }
    }
    catch (std::exception &e)
    {
        datetime = default_datetime;
    }

    return datetime;
}
