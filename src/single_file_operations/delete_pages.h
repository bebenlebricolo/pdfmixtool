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

#ifndef DELETEPAGES_H
#define DELETEPAGES_H

#include "abstract_operation.h"
#include "../widgets/pages_selector.h"

class DeletePages : public AbstractOperation
{
    Q_OBJECT
public:
    explicit DeletePages(const PdfInfo &pdf_info,
                         QWidget *parent = nullptr);

private:
    PagesSelector *m_pages_selector;

    void save(bool save_as);
};

#endif // DELETEPAGES_H
