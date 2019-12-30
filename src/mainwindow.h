/* Copyright (C) 2017-2019 Marco Scarpetta
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

#include "mouseeventfilter.h"
#include "inputpdffiledelegate.h"
#include "pdf_edit_lib/definitions.h"

Q_DECLARE_METATYPE(Multipage)

QDataStream &operator<<(QDataStream &out, const Multipage &maltipage);
QDataStream &operator>>(QDataStream &in, Multipage &maltipage);

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(MouseEventFilter *filter, QWidget *parent = nullptr);

signals:

public slots:
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

    void closeEvent(QCloseEvent *event);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    const QList<int> selected_indexes();

    QSettings *m_settings;

    QTabWidget *m_tab_widget;

    QCheckBox *m_alternate_mix;

    QLabel *m_output_page_count;
    int m_output_pages_error_index;

    QProgressBar *m_progress_bar;

    QListView *m_files_list_view;
    QStandardItemModel *m_files_list_model;
    InputPdfFileDelegate *m_delegate;


    QMenu *m_edit_menu;

    QMap<int, Multipage> m_multipages;
};

#endif // MAINWINDOW_H
