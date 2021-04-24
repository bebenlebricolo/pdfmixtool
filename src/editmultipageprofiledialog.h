/* Copyright (C) 2017-2021 Marco Scarpetta
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

#ifndef EDITMULTIPAGEPROFILEDIALOG_H
#define EDITMULTIPAGEPROFILEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>

#include "pdf_edit_lib/definitions.h"
#include "widgets/multipage_editor.h"

class EditMultipageProfileDialog : public QDialog
{
    Q_OBJECT
public:
    EditMultipageProfileDialog(QWidget *parent = nullptr);

    void set_multipage(const Multipage &multipage);

    Multipage get_multipage();

    void set_index(int index);

    int get_index();

private:
    int m_index;
    QLineEdit m_name;
    MultipageEditor m_multipage_editor;
};

#endif // EDITMULTIPAGEPROFILEDIALOG_H
