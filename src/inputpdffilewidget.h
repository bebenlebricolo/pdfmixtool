/* Copyright (C) 2017-2020 Marco Scarpetta
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

#ifndef INPUTPDFFILEWIDGET_H
#define INPUTPDFFILEWIDGET_H

#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QMouseEvent>
#include <QStandardItem>

#include "multipageprofilesmanager.h"

class InputPdfFileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InputPdfFileWidget(const QModelIndex &index,
                                const QMap<int, Multipage> &custom_multipages,
                                MultipageProfilesManager *mp_manager,
                                int preview_size,
                                bool alternate_mix,
                                QWidget *parent = nullptr);

    void set_editor_data(const QModelIndex &index);

    void set_model_data(QStandardItem *item);

signals:
    void focus_out(QWidget *editor) const;

public slots:
    void mouse_button_pressed(QMouseEvent *event);

    void update_preview();

    void multipage_activated(int index);

    void profile_created(int index);

private:
    double m_page_width;
    double m_page_height;
    const QMap<int, Multipage> &m_multipages;
    MultipageProfilesManager *m_mp_manager;
    bool m_alternate_mix;
    QStandardItem *m_last_item;
    int m_preview_size;
    QLabel *m_preview_label;
    QLineEdit *m_pages_filter_lineedit;
    QComboBox *m_multipage_combobox;
    QComboBox *m_rotation_combobox;
    QLineEdit *m_outline_entry_lineedit;
    QCheckBox *m_reverse_order_checkbox;
};

#endif // INPUTPDFFILEWIDGET_H
