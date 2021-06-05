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

#ifndef ADDEMPTYPAGES_H
#define ADDEMPTYPAGES_H

#include <QSpinBox>
#include <QButtonGroup>
#include <QComboBox>

#include "abstract_operation.h"

class AddEmptyPages : public AbstractOperation
{
    Q_OBJECT
public:
    explicit AddEmptyPages(QWidget *parent = nullptr);

    void set_pdf_info(const PdfInfo &pdf_info) override;

    int output_pages_count() override;

signals:
    void save_button_pressed();
    void save_as_button_pressed();

private:
    enum class m_pages_size_type {same, custom, standard};
    enum class m_position {before, after};

    void m_save();

    QSpinBox m_count;
    QButtonGroup m_page_size;
    QDoubleSpinBox m_page_width;
    QDoubleSpinBox m_page_height;
    QComboBox m_standard_page_size;
    QButtonGroup m_orientation;
    QButtonGroup m_before_after;
    QSpinBox m_page;
};

#endif // ADDEMPTYPAGES_H
