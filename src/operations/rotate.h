/* Copyright (C) 2021 Marco Scarpetta
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

#ifndef ROTATE_H
#define ROTATE_H

#include "abstract_operation.h"
#include "../widgets/pages_selector.h"
#include "../widgets/output_preview.h"

class Rotate : public AbstractOperation
{
    Q_OBJECT
public:
    explicit Rotate(QWidget *parent = nullptr);

    void set_pdf_info(const PdfInfo &pdf_info) override;

    int output_pages_count() override;

private:
    void rotate(bool clockwise);

    void save(bool save_as);

    int m_rotation;
    OutputPreview *m_output_preview;
    PagesSelector *m_pages_selector;
};

#endif // ROTATE_H
