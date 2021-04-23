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

#ifndef MULTIPAGEEDITOR_H
#define MULTIPAGEEDITOR_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QButtonGroup>

#include "../pdf_edit_lib/definitions.h"
#include "../gui_utils.h"

class MultipageEditor : public QWidget
{
    Q_OBJECT
public:
    explicit MultipageEditor(QWidget *parent = nullptr);

    void set_multipage(const Multipage &multipage);

    Multipage get_multipage() const;

signals:
    void multipage_changed(const Multipage &multipage);

    void subpages_number_changed(int subpages);

private slots:
    void update_multipage();

    void update_custom_page_size();

    void on_page_width_changed(double value);

    void on_page_height_changed(double value);

private:
    Multipage m_multipage;

    QButtonGroup m_standard_custom;

    QWidget m_standard_size_chooser;
    QComboBox m_page_size;
    QComboBox m_orientation;

    QWidget m_custom_size_chooser;
    QDoubleSpinBox m_page_width;
    QDoubleSpinBox m_page_height;

    QSpinBox m_rows;
    QSpinBox m_columns;
    QDoubleSpinBox m_spacing;
    QCheckBox m_rtl;

    QComboBox m_h_alignment;
    QComboBox m_v_alignment;

    QDoubleSpinBox m_margin_left;
    QDoubleSpinBox m_margin_right;
    QDoubleSpinBox m_margin_top;
    QDoubleSpinBox m_margin_bottom;
};

#endif // MULTIPAGEEDITOR_H
