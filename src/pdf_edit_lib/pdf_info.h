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

#ifndef PDF_INFO_H
#define PDF_INFO_H

#include <string>
#include <qpdf/QUtil.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageLabelDocumentHelper.hh>
#include <qpdf/QPDFAcroFormDocumentHelper.hh>

class PdfInfo
{
public:
    PdfInfo(const std::string &path);

    double width();

    double height();

    const std::string &paper_size();

    bool is_portrait();

    int n_pages();

    QPDF qpdf;

private:
    double m_page_width, m_page_height;
    int m_n_pages;
    std::string m_paper_size;
    bool m_is_portrait;
};

#endif // PDF_INFO_H
