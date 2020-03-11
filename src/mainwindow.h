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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QListView>
#include <QStandardItemModel>
#include <QSettings>
#include <QTabWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QButtonGroup>

#include "mouseeventfilter.h"
#include "inputpdffiledelegate.h"
#include "pdf_edit_lib/definitions.h"
#include "pdfinfolabel.h"

#include "single_file_operations/booklet.h"
#include "single_file_operations/edit_page_layout.h"
#include "single_file_operations/add_empty_pages.h"
#include "single_file_operations/delete_pages.h"
#include "single_file_operations/extract_pages.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(MouseEventFilter *filter, QWidget *parent = nullptr);

signals:

public slots:
    void current_tab_changed(int index);

    // multiple files
    void add_pdf_files();

    void move_up();

    void move_down();

    void remove_pdf_file();

    void edit_menu_activated();

    void view_menu_activated();

    void item_mouse_pressed(const QModelIndex &index);

    void alternate_mix_checked(bool checked);

    void update_output_pages_count();

    void generate_pdf_button_pressed();

    // single file
    void open_file_pressed();

    void update_opened_file_label(const QString &filename);

    void generate_booklet_pressed();

    void save_button_pressed(int from_page);

    void save_as_button_pressed(int from_page);

    void do_save(int from_page, const QString &filename);

    void extract_individual_button_pressed();

    void extract_single_button_pressed();

    // close event
    void closeEvent(QCloseEvent *event);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    const QList<int> selected_indexes();

    QSettings *m_settings;

    QTabWidget *m_tab_widget;
    QStatusBar *m_status_bar;

    QCheckBox *m_alternate_mix;

    QLabel *m_output_page_count;
    int m_output_pages_error_index;

    QProgressBar *m_progress_bar;
    QPushButton *m_generate_pdf_button;

    QListView *m_files_list_view;
    QStandardItemModel *m_files_list_model;
    InputPdfFileDelegate *m_delegate;

    QMenu *m_edit_menu;

    QWidget *m_operations_widget;
    PdfInfo m_opened_pdf_info;
    PdfInfoLabel *m_opened_file_label;

    Booklet m_booklet_tab;
    EditPageLayout m_edit_page_layout_tab;
    AddEmptyPages m_add_empty_pages_tab;
    DeletePages m_delete_pages_tab;
    ExtractPages m_extract_pages_tab;

    MultipageProfilesManager *m_multipage_profiles_manager;
};

#endif // MAINWINDOW_H
