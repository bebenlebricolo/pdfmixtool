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

#ifndef ROTATEMULTIPAGE_H
#define ROTATEMULTIPAGE_H

#include <QComboBox>
#include <QLabel>
#include <QSpinBox>

#include "abstract_operation.h"
#include "../pdf_edit_lib/pdf_info.h"
#include "../widgets/multipage_editor.h"
#include "../widgets/output_preview.h"
#include "../widgets/pages_selector.h"

class EditPageLayout : public AbstractOperation
{
    Q_OBJECT
public:
    explicit EditPageLayout(QWidget *parent = nullptr);

    void set_pdf_info(const PdfInfo &pdf_info) override;

    int output_pages_count() override;

signals:
    void save_button_pressed();

    void save_as_button_pressed();

private:
    void save();

    MultipageEditor *m_multipage_editor;
    PagesSelector *m_pages_selector;
    OutputPreview *m_output_preview;
};

#endif // ROTATEMULTIPAGE_H
