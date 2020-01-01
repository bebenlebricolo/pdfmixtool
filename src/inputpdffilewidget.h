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

#ifndef INPUTPDFFILEWIDGET_H
#define INPUTPDFFILEWIDGET_H

#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QMouseEvent>
#include <QStandardItem>

#include "pdf_edit_lib/definitions.h"
#include "multipageprofilesmanager.h"

#define FILE_PATH_ROLE Qt::UserRole + 1 // QString
#define PAGE_WIDTH_ROLE Qt::UserRole + 2 // int
#define PAGE_HEIGHT_ROLE Qt::UserRole + 3 // int
#define PAPER_SIZE_ROLE Qt::UserRole + 4 // QString
#define IS_PORTRAIT_ROLE Qt::UserRole + 5 // bool
#define N_PAGES_ROLE Qt::UserRole + 6 // int
#define OUTPUT_PAGES_ROLE Qt::UserRole + 7 // QString
#define MULTIPAGE_ROLE Qt::UserRole + 8 // int
#define ROTATION_ROLE Qt::UserRole + 9 // int
#define OUTLINE_ENTRY_ROLE Qt::UserRole + 10 // QString
#define REVERSE_ORDER_ROLE Qt::UserRole + 11 // bool

double draw_preview_page(QPainter *painter,
                         int max_width,
                         int max_height,
                         double page_width,
                         double page_height,
                         Multipage::Alignment h_alignment,
                         Multipage::Alignment v_alignment,
                         const QString &text);

void draw_preview(QPainter *painter,
                  const QRect &rect,
                  double source_width,
                  double source_height,
                  int rotation,
                  bool multipage_enabled,
                  const Multipage &multipage);

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
