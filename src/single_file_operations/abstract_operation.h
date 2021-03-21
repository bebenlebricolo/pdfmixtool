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

#ifndef ABSTRACTOPERATION_H
#define ABSTRACTOPERATION_H

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>

#include "../pdf_edit_lib/pdf_info.h"
#include "../pdf_edit_lib/pdf_editor.h"

//class WriterThread : public QThread
//{
//    Q_OBJECT

//    void run() override;

//public:
//    WriterThread(const PdfEditor &editor, const QString &filename);

//signals:
//    void progress(int progress);

//    void done(const QString &filename);

//private:
//    PdfEditor editor;
//    QString filename;
//};

class AbstractOperation : public QWidget
{
    Q_OBJECT
    QThread worker_thread;

public:
    explicit AbstractOperation(const PdfInfo &pdf_info,
                               QWidget *parent = nullptr);

    const QString &name();

public slots:
    virtual void pdf_info_changed();

signals:
    void write_started();
    void progress_changed(int progress);
    void write_finished(const QString &filename);

protected:
    QString m_name;

    QPushButton m_save_button;
    QString m_save_filename;
    bool show_overwrite_dialog();
    bool show_save_as_dialog();

    PdfInfo const *m_pdf_info;

private:
    void update_progress_bar(std::atomic_int *progress);
};

#endif // ABSTRACTOPERATION_H
