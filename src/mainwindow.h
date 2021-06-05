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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QListView>
#include <QStandardItemModel>
#include <QSettings>
#include <QCheckBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QListWidget>

#include "mouseeventfilter.h"
#include "inputpdffiledelegate.h"
#include "pdf_edit_lib/definitions.h"
#include "widgets/pdfinfolabel.h"

#include "operations/add_empty_pages.h"
#include "operations/alternate_mix.h"
#include "operations/booklet.h"
#include "operations/delete_pages.h"
#include "operations/edit_document_info.h"
#include "operations/edit_page_layout.h"
#include "operations/extract_pages.h"
#include "operations/merge.h"
#include "operations/rotate.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void set_input_files(const QStringList &files);

signals:

public slots:
    // single file
    void operation_changed(int index);

    void open_file_pressed();

    void update_opened_file_label(const QString &filename);

    void update_output_pages_count(int count);

    void write_started();

    void update_progress(int progress);

    void write_finished(const QString &filename);

    void write_error(const QString &error);

    // close event
    void closeEvent(QCloseEvent *event);

private:
    QStatusBar *m_status_bar;

    QLabel *m_output_page_count;

    QProgressBar *m_progress_bar;
    QLabel m_saved_file;

    QListWidget *m_operations_list;
    QStackedWidget m_operations_widget;
    QWidget m_open_pdf_widget;
    QPushButton *m_view_opened_pdf_button;
    PdfInfo m_opened_pdf_info;
    PdfInfoLabel *m_opened_file_label;
    QVector<AbstractOperation *> m_operations;
    AbstractOperation *m_active_operation;

    MultipageProfilesManager *m_multipage_profiles_manager;
};

#endif // MAINWINDOW_H
