/* Copyright (C) 2017-2022 Marco Scarpetta
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

#ifndef EDITPDFENTRYDIALOG_H
#define EDITPDFENTRYDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QStandardItemModel>

#include "pdf_edit_lib/definitions.h"

class EditPdfEntryDialog : public QDialog
{
    Q_OBJECT
public:
    EditPdfEntryDialog(QStandardItemModel *model,
                       const QModelIndexList &indexes,
                       QWidget *parent = nullptr);

private slots:
    void accepted();

private:
    QStandardItemModel *m_model;
    const QModelIndexList &m_indexes;
    QComboBox m_multipage_combobox;
    QComboBox m_rotation_combobox;
};

#endif // EDITPDFENTRYDIALOG_H
