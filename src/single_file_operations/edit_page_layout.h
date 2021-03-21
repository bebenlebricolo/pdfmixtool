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

#ifndef ROTATEMULTIPAGE_H
#define ROTATEMULTIPAGE_H

#include <QComboBox>
#include <QLabel>
#include <QSpinBox>

#include "abstract_operation.h"
#include "../pdf_edit_lib/pdf_info.h"

class EditPageLayout : public AbstractOperation
{
    Q_OBJECT
public:
    explicit EditPageLayout(const PdfInfo &pdf_info,
                            QWidget *parent = nullptr);

public slots:
    void pdf_info_changed();

    void update_multipage_profiles();

    void profile_created(int index);

signals:
    void trigger_new_profile();
    void save_button_pressed();
    void save_as_button_pressed();

private slots:
    void update_preview_image();

    void multipage_activated(int index);

private:
    void save();

    QComboBox m_rotation;
    QComboBox m_multipage;
    QSpinBox m_scale;
    QLabel m_preview_image;
    QLabel m_paper_size_label;
    bool m_new_profile_triggered;

};

#endif // ROTATEMULTIPAGE_H
