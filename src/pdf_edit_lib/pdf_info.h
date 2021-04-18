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

#ifndef PDF_INFO_H
#define PDF_INFO_H

#include <string>
#include <ctime>
#include <qpdf/QUtil.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageLabelDocumentHelper.hh>
#include <qpdf/QPDFAcroFormDocumentHelper.hh>

class PdfInfo
{
public:
    PdfInfo();

    PdfInfo(const std::string &filename);

    const std::string &filename() const;

    double width() const;

    double height() const;

    const std::string &paper_size() const;

    bool is_portrait() const;

    int n_pages() const;

    const std::string &title() const;
    const std::string &author() const;
    const std::string &subject() const;
    const std::string &keywords() const;
    const std::string &creator() const;
    const std::string &producer() const;
    const std::tm &creation_date() const;
    bool has_creation_date() const;
    const std::tm &mod_date() const;
    bool has_mod_date() const;

    static std::tm string_to_datetime(const std::string &str);

private:
    std::string m_filename;
    double m_page_width, m_page_height;
    int m_n_pages;
    std::string m_paper_size;
    bool m_is_portrait;

    std::string m_title;
    std::string m_author;
    std::string m_subject;
    std::string m_keywords;
    std::string m_creator;
    std::string m_producer;
    std::tm m_creation_date;
    bool m_has_creation_date;
    std::tm m_mod_date;
    bool m_has_mod_date;
};

// 1900/01/01 00:00
static const std::tm default_datetime{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, ""};

#endif // PDF_INFO_H
