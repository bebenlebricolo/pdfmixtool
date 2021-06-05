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

#ifndef MERGE_H
#define MERGE_H

#include <QListView>
#include <QStandardItemModel>
#include <QMenu>

#include "../inputpdffiledelegate.h"
#include "abstract_operation.h"

class Merge : public AbstractOperation
{
    Q_OBJECT
public:
    explicit Merge(QWidget *parent = nullptr);

    int output_pages_count() override;

    void add_pdf_files(const QStringList &files);

public slots:
    void update_multipage_profiles() override;

    void profile_created(int index) override;

protected slots:
    void new_profile_triggered(int index);

    bool load_json_files_list(const QString &filename);

    void load_files_list_pressed();

    void save_files_list_pressed();

    void move_up();

    void move_down();

    void remove_pdf_file();

    virtual void edit_menu_activated();

    void view_menu_activated();

    void update_output_pages_count();

    void generate_pdf_button_pressed();

protected:
    const QList<int> selected_indexes();

    bool eventFilter(QObject *obj, QEvent *ev) override;

    virtual void write(const QString &filename);

    InputPdfFileDelegate *m_delegate;

    int m_triggered_new_profile;

    int m_output_pages_error_index;

    QAction *m_save_files_list_action;
    QPushButton *m_generate_pdf_button;

    QListView *m_files_list_view;
    QStandardItemModel *m_files_list_model;

    QMenu *m_edit_menu;
};

#endif // MERGE_H
