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

#include <QButtonGroup>
#include <QLineEdit>

#include "abstract_operation.h"

class DeletePages : public AbstractOperation
{
    Q_OBJECT
public:
    explicit DeletePages(const PdfInfo &pdf_info,
                         QProgressBar *progress_bar,
                         QWidget *parent = nullptr);

private:
    QButtonGroup m_selection_type;
    QLineEdit m_selection;

    bool check_selection();

    void save();
};

#endif // DELETEPAGES_H
