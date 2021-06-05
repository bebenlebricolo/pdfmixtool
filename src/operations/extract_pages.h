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

#ifndef EXTRACTPAGES_H
#define EXTRACTPAGES_H

#include "abstract_operation.h"
#include "../pdf_edit_lib/pdf_info.h"
#include "../widgets/pages_selector.h"

class ExtractPages : public AbstractOperation
{
    Q_OBJECT
public:
    explicit ExtractPages(QWidget *parent = nullptr);

    void set_pdf_info(const PdfInfo &pdf_info) override;

    int output_pages_count() override;

private:
    PagesSelector *m_pages_selector;
    QLineEdit m_base_name;

    void extract_to_individual();

    void extract_to_single();
};

#endif // EXTRACTPAGES_H
