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
#include <QSplitter>

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

    void set_input_files(const QStringList &files);

signals:

public slots:
    void current_tab_changed(int index);

    // multiple files
    bool load_json_files_list(const QString &filename);

    void load_files_list_pressed();

    void save_files_list_pressed();

    void add_pdf_files(const QStringList &files);

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

    void write_started();

    void write_finished(const QString &filename);

    // close event
    void closeEvent(QCloseEvent *event);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    const QList<int> selected_indexes();

    QTabWidget *m_tab_widget;
    QStatusBar *m_status_bar;

    QCheckBox *m_alternate_mix;

    QLabel *m_output_page_count;
    int m_output_pages_error_index;

    QProgressBar *m_progress_bar;
    QLabel m_saved_file;
    QPushButton *m_save_files_list_button;
    QPushButton *m_generate_pdf_button;

    QListView *m_files_list_view;
    QStandardItemModel *m_files_list_model;
    InputPdfFileDelegate *m_delegate;

    QMenu *m_edit_menu;

    QSplitter m_operations_splitter;
    QPushButton *m_view_opened_pdf_button;
    PdfInfo m_opened_pdf_info;
    PdfInfoLabel *m_opened_file_label;
    QVector<AbstractOperation *> m_operations;

    MultipageProfilesManager *m_multipage_profiles_manager;
};

#endif // MAINWINDOW_H
