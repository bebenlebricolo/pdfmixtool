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

#ifndef EXTRACTPAGES_H
#define EXTRACTPAGES_H

#include <QWidget>
#include <QButtonGroup>
#include <QLineEdit>

#include "../pdf_edit_lib/pdf_info.h"

class ExtractPages : public QWidget
{
    Q_OBJECT
public:
    explicit ExtractPages(QWidget *parent = nullptr);

    void set_pdf_info(const PdfInfo &pdf_info);

    QString get_selection();

    QString get_base_name();

signals:
    void extract_individual_button_pressed();
    void extract_single_button_pressed();

private:
    QButtonGroup m_extraction_type;
    QLineEdit m_selection;
    QLineEdit m_base_name;

    PdfInfo m_pdf_info;

    bool check_selection();
};

#endif // EXTRACTPAGES_H
