/* Copyright (C) 2020-2021 Marco Scarpetta
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

#ifndef ABSTRACTOPERATION_H
#define ABSTRACTOPERATION_H

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QDir>

#include "../pdf_edit_lib/pdf_info.h"
#include "../pdf_edit_lib/pdf_editor.h"

class AbstractOperation : public QWidget
{
    Q_OBJECT

public:
    explicit AbstractOperation(QWidget *parent = nullptr);

    const QString &name();

    const QIcon &icon();

    virtual bool is_single_file_operation();

    void set_active(bool active);

    virtual void set_pdf_info(const PdfInfo &pdf_info);

    virtual int output_pages_count();

public slots:
    virtual void update_multipage_profiles();

    virtual void profile_created(int index);

signals:
    void trigger_new_profile();

    void output_pages_count_changed(int);

    void write_started();

    void progress_changed(int progress);

    void write_finished(const QString &filename);

    void write_error(const QString &error);

protected:
    QString m_name;
    QIcon m_icon;
    bool m_is_single_file_operation;

    bool m_active;

    QPushButton m_save_button;
    QString m_save_filename;
    bool show_overwrite_dialog();
    bool show_save_as_dialog();

    PdfInfo m_pdf_info;
};

#endif // ABSTRACTOPERATION_H
